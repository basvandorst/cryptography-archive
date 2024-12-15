/* ====================================================================
 * Copyright (c) 1995, 1996, 1997 Ben Laurie.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * 4. The name "Apache-SSL Server" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * THIS SOFTWARE IS PROVIDED BY BEN LAURIE ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL BEN LAURIE OR
 * HIS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of patches to the Apache HTTP server interfacing it
 * to SSLeay.
 * For more information on Apache-SSL, contact Ben Laurie <ben@algroup.co.uk>.
 *
 * For more information on Apache see http://www.apache.org.
 *
 * For more information on SSLeay see http://www.psy.uq.oz.au/~ftp/Crypto/.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include <ssl.h>
#include <err.h>
#include <x509.h>
#include <pem.h>
#include <crypto.h>

#include "httpd.h"
#include "http_config.h"
#include "http_log.h"

/*
Useful bits for dbx (useful for Ben, at least):

r -d /u/ben/apache/apache_1.2.5-ssl/SSLconf -X
use /u/ben/work/scuzzy-ssleay8/ssl /u/ben/work/scuzzy-ssleay8/crypto/bio
*/

#define SSL_IS_OPTIONAL		FALSE	/* Set to TRUE if you want to allow SSL to be disabled on a per-request basis (useful for subrequests) */

#if SSL_IS_OPTIONAL
# define OPTIONAL_SSL	(!r->connection->client->ssl)
#else
# define OPTIONAL_SSL	FALSE
#endif

#if SSLEAY_VERSION_NUMBER < 0x0800
# error "SSLeay versions below 0.8.x are no longer supported"
/* But note, they nearly are, if anyone cares */
#endif

#if SSLEAY_VERSION_NUMBER >= 0x0800
/* Do session caching - only available with SSLeay 0.8.x */
# define CACHE_SESSIONS		TRUE
#endif

/* define this true when SSLeay can do an SSL_free without falling over */
#define FREE_SESSION	FALSE

/* define this to debug the verification stuff */
#define DEBUG_VERIFY	FALSE

#if CACHE_SESSIONS
# include "gcache.h"
static const char *szCacheServerPort;
static char *szCacheServerPath;

#define DEBUG_SESSIONS		0

#endif

#if SSLEAY_VERSION_NUMBER < 0x900
/* correct for a typo */
# define SSL3_TXT_ADH_DES_192_CBC_SHA SSL3_TXT_ADH_DES_196_CBC_SHA
#endif

#if SSLEAY_VERSION_NUMBER < 0x600
# define ERR_print_errors_fp	ERR_print_errors
#endif

#if SSLEAY_VERSION_NUMBER < 0x0800
# define X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY \
		VERIFY_ERR_UNABLE_TO_GET_ISSUER
# define X509_V_OK VERIFY_OK
# define SSL_CTX_set_default_verify_paths(ctx)	\
		SSL_set_default_verify_paths(ctx)
# define SSL_CTX_load_verify_locations(ctx,cafile,capath) \
		 SSL_load_verify_locations(ctx,cafile,capath)
# define X509_verify_cert_error_string(error) X509_cert_verify_error_string(error)
#endif

typedef enum
    {
    VERIFY_NONE=0,
    VERIFY_OPTIONAL=1,
    VERIFY_REQUIRE=2,
    VERIFY_OPTIONAL_NO_CA=3
    } VerifyType;

typedef struct
    {
    BOOL bDisabled;

    char *szCertificateFile;
    char *szKeyFile;
    char *szCACertificatePath;
    char *szCACertificateFile;
    char *szLogFile;
    char *szReqCiphers;
    FILE *fileLogFile;
    int nVerifyDepth;
    VerifyType nVerifyClient;

    X509 *px509Certificate;
    RSA *prsaKey;

    SSL_CTX *pSSLCtx;

    BOOL bFakeBasicAuth;

    time_t tSessionCacheTimeout;
    } SSLConfigRec;

typedef struct
    {
    /* 1) If cipher is banned, refuse */
    /* 2) If RequiredCiphers is NULL, accept */
    /* 3) If the cipher isn't required, refuse */

    table *tbRequiredCiphers;
    table *tbBannedCiphers;
    } SSLDirConfigRec;

extern module ssl_module;

static conn_rec *g_pCurrentConnection;

static const char *SSLRequireCipher(cmd_parms *cmd,SSLDirConfigRec *rec,char *cipher)
    {
    table_set(rec->tbRequiredCiphers,cipher,"Required");
    return NULL;
    }

static const char *SSLBanCipher(cmd_parms *cmd,SSLDirConfigRec *rec,char *cipher)
    {
    table_set(rec->tbBannedCiphers,cipher,"Banned");
    return NULL;
    }

static int SSLCheckCipher(request_rec *r)
    {
    char *cipher;
    SSLDirConfigRec *rec=(SSLDirConfigRec *)
      get_module_config(r->per_dir_config,&ssl_module);
    SSLConfigRec *pConfig=(SSLConfigRec *)
      get_module_config(r->server->module_config,&ssl_module);

    /* Check to see if SSL is on */
    if(pConfig->bDisabled || OPTIONAL_SSL)
	return DECLINED;

    cipher=SSL_get_cipher(r->connection->client->ssl);

    if(table_get(rec->tbBannedCiphers,cipher))
	{
	char *buf;

	buf=pstrcat(r->pool,"Cipher ",cipher," is forbidden",NULL);
	log_reason(buf,r->filename,r);
	return FORBIDDEN;
	}

    if(table_get(rec->tbRequiredCiphers,cipher))
	return OK;

    if(rec->tbRequiredCiphers->nelts == 0)
	return OK;
    else
	{
	char *buf;
	
	buf=pstrcat(r->pool,"Cipher ",cipher," is not on the permitted list",
		    NULL);
	log_reason(buf,r->filename,r);
	return FORBIDDEN;
	}
    }

static void StrUpper(char *s)
    {
    for( ; *s ; ++s)
	*s=toupper(*s);
    }

void ExpandCert(table *pEnv,char *szPrefix,char *szCert)
    {
    /* Use ap_pstrintf when we get to 1.3 */
    char var[HUGE_STRING_LEN];
    char buf[HUGE_STRING_LEN];
    char *s;
    /* Expand a X509_oneline entry into it's base components and register
       them as environment variables. Needed if you want to pass certificate
       information to CGI's. The naming convention SHOULD be fairly compatible
       with CGI's written for stronghold's certificate info  - Q */
    /* FIXME - strtok() and strcspn() may cause problems on some systems - Q */

    sprintf(var,"%sDN",szPrefix);
    table_set(pEnv,var,szCert);
    
    strcpy(buf,szCert);
    for(s=strtok(buf,"/") ; s != NULL ; s=strtok(NULL,"/"))
	{
	int n=strcspn(s,"=");
	s[n]='\0';
	StrUpper(s);
	sprintf(var,"%s%s",szPrefix,s);
	table_set(pEnv,var,s+n+1);
	}
    }

static struct
    {
    char *szName;
    int nKeySize;
    int nSecretKeySize;
    } aCipherspecs[]=
    {
#if SSLEAY_VERSION_NUMBER >= 0x0900
    { SSL3_TXT_RSA_IDEA_128_SHA		/*"IDEA-CBC-SHA"*/,	128, 128 },
#endif
    { SSL3_TXT_RSA_NULL_MD5		/*"NULL-MD5"*/, 	0, 0 },
    { SSL3_TXT_RSA_NULL_SHA	 	/*"NULL-SHA"*/, 	0, 0 },
    { SSL3_TXT_RSA_RC4_40_MD5		/*"EXP-RC4-MD5"*/,	128, 40 },
    { SSL3_TXT_RSA_RC4_128_MD5		/*"RC4-MD5"*/,		128, 128 },
    { SSL3_TXT_RSA_RC4_128_SHA		/*"RC4-SHA"*/,		128, 128 },
    { SSL3_TXT_RSA_RC2_40_MD5		/*"EXP-RC2-CBC-MD5"*/,	128, 40 },
    { SSL3_TXT_RSA_IDEA_128_SHA		/*"IDEA-CBC-MD5"*/,	128, 128 },
    { SSL3_TXT_RSA_DES_40_CBC_SHA	/*"EXP-DES-CBC-SHA"*/,	56, 40 },
    { SSL3_TXT_RSA_DES_64_CBC_SHA	/*"DES-CBC-SHA"*/,	56, 56 },
    { SSL3_TXT_RSA_DES_192_CBC3_SHA	/*"DES-CBC3-SHA"*/,	168, 168 },
    { SSL3_TXT_DH_DSS_DES_40_CBC_SHA	/*"EXP-DH-DSS-DES-CBC-SHA"*/, 56, 40 },
    { SSL3_TXT_DH_DSS_DES_64_CBC_SHA	/*"DH-DSS-DES-CBC-SHA"*/, 56, 56 },
    { SSL3_TXT_DH_DSS_DES_192_CBC3_SHA 	/*"DH-DSS-DES-CBC3-SHA"*/, 168, 168 },
    { SSL3_TXT_DH_RSA_DES_40_CBC_SHA	/*"EXP-DH-RSA-DES-CBC-SHA"*/, 56, 40 },
    { SSL3_TXT_DH_RSA_DES_64_CBC_SHA	/*"DH-RSA-DES-CBC-SHA"*/, 56, 56 },
    { SSL3_TXT_DH_RSA_DES_192_CBC3_SHA 	/*"DH-RSA-DES-CBC3-SHA"*/, 168, 168 },
    { SSL3_TXT_EDH_DSS_DES_40_CBC_SHA	/*"EXP-EDH-DSS-DES-CBC-SHA"*/, 56, 40 },
    { SSL3_TXT_EDH_DSS_DES_64_CBC_SHA	/*"EDH-DSS-DES-CBC-SHA"*/, 56, 56 },
    { SSL3_TXT_EDH_DSS_DES_192_CBC3_SHA	/*"EDH-DSS-DES-CBC3-SHA"*/, 168, 168 },
    { SSL3_TXT_EDH_RSA_DES_40_CBC_SHA	/*"EXP-EDH-RSA-DES-CBC"*/, 56, 40 },
    { SSL3_TXT_EDH_RSA_DES_64_CBC_SHA	/*"EDH-RSA-DES-CBC-SHA"*/, 56, 56 },
    { SSL3_TXT_EDH_RSA_DES_192_CBC3_SHA	/*"EDH-RSA-DES-CBC3-SHA"*/, 168, 168 },
    { SSL3_TXT_ADH_RC4_40_MD5		/*"EXP-ADH-RC4-MD5"*/,	128, 40 },
    { SSL3_TXT_ADH_RC4_128_MD5		/*"ADH-RC4-MD5"*/,	128, 128 },
    { SSL3_TXT_ADH_DES_40_CBC_SHA	/*"EXP-ADH-DES-CBC-SHA"*/, 128, 40 },
    { SSL3_TXT_ADH_DES_64_CBC_SHA	/*"ADH-DES-CBC-SHA"*/,	56, 56 },
    { SSL3_TXT_ADH_DES_192_CBC_SHA	/*"ADH-DES-CBC3-SHA"*/, 168, 168 },
    { SSL3_TXT_FZA_DMS_NULL_SHA		/*"FZA-NULL-SHA"*/,	0, 0 },
    { SSL3_TXT_FZA_DMS_FZA_SHA		/*"FZA-FZA-CBC-SHA"*/,	-1, -1 },
    { SSL3_TXT_FZA_DMS_RC4_SHA		/*"FZA-RC4-SHA"*/,	 128, 128 },
    { SSL2_TXT_DES_64_CFB64_WITH_MD5_1	/*"DES-CFB-M1"*/,	56, 56 },
    { SSL2_TXT_RC2_128_CBC_WITH_MD5	/*"RC2-CBC-MD5"*/,	128, 128 },
    { SSL2_TXT_DES_64_CBC_WITH_MD5	/*"DES-CBC-MD5"*/,	56, 56 },
    { SSL2_TXT_DES_192_EDE3_CBC_WITH_MD5/*"DES-CBC3-MD5"*/,	168, 168 },
    { SSL2_TXT_RC4_64_WITH_MD5		/*"RC4-64-MD5"*/,	64, 64 },
    { SSL2_TXT_NULL			/*"NULL"*/,		0, 0 },
    { NULL, 0, 0 }
    };

int SSLFixups(request_rec *r)
    {
    table *e=r->subprocess_env;
    int keysize=0;
    int secretkeysize=0;
    char buf[MAX_STRING_LEN];
    char *cipher;
    int n;
    X509 *xs;

    SSLConfigRec *pConfig=(SSLConfigRec *)
      get_module_config(r->server->module_config,&ssl_module);

    /* Check to see if SSL is on */
    if(pConfig->bDisabled || OPTIONAL_SSL)
	return DECLINED;

    cipher=SSL_get_cipher(r->connection->client->ssl);
    table_set(e,"HTTPS","on");
    table_set(e,"HTTPS_CIPHER",cipher);
    table_set(e,"SSL_CIPHER",cipher);
    table_set(e,"SSL_PROTOCOL_VERSION",
	      SSL_get_version(r->connection->client->ssl));
    table_set(e,"SSL_SSLEAY_VERSION", SSLeay_version(SSLEAY_VERSION));

    for(n=0 ; aCipherspecs[n].szName ; ++n)
	if(!strcmp(cipher,aCipherspecs[n].szName))
	    {
	    keysize=aCipherspecs[n].nKeySize;
	    secretkeysize=aCipherspecs[n].nSecretKeySize;
	    break;
	    }

    sprintf(buf,"%d",keysize);
    table_set(e,"HTTPS_KEYSIZE",buf);

    sprintf(buf,"%d",secretkeysize);
    table_set(e,"HTTPS_SECRETKEYSIZE",buf);

    if(r->connection->client->szClientX509)
	{
	ExpandCert(e,"SSL_CLIENT_",r->connection->client->szClientX509);
	xs=SSL_get_peer_certificate(r->connection->client->ssl);
	ExpandCert(e,"SSL_CLIENT_I",X509_NAME_oneline(X509_get_issuer_name(xs),
						      NULL,0));
	}

    xs=SSL_get_certificate(r->connection->client->ssl);
    ExpandCert(e,"SSL_SERVER_",X509_NAME_oneline(X509_get_subject_name(xs),
						 NULL,0));
    ExpandCert(e,"SSL_SERVER_I",X509_NAME_oneline(X509_get_issuer_name(xs),
						  NULL,0));

    return DECLINED;
    }

static void SSLLogError(server_rec *s)
    {
    unsigned long l;
    char buf[MAX_STRING_LEN];

    /* Print out error messages */

    while((l=ERR_get_error()))
      {
	ERR_error_string(l,buf);
	log_error(buf,s);
      }
    }

/* for debugging */
static void SSLLogErrorToFile(FILE *f)
    {
    unsigned long l;
    char buf[MAX_STRING_LEN];

    /* Print out error messages */

    while((l=ERR_get_error()))
      {
	ERR_error_string(l,buf);
	fputs(buf,f);
	fputc('\n',f);
      }
    }

static void *CreateSSLServerConfig(pool *p,server_rec *s)
    {
    SSLConfigRec *rec=pcalloc(p,sizeof(SSLConfigRec));
    
    rec->bDisabled=FALSE;
    rec->szCertificateFile=rec->szKeyFile=rec->szLogFile=NULL;
    rec->nVerifyDepth=0;
    rec->nVerifyClient=VERIFY_NONE;
    rec->px509Certificate=NULL;
    rec->prsaKey=NULL;
    rec->bFakeBasicAuth=FALSE;
    rec->tSessionCacheTimeout=0;
    return rec;
    }

static void *CreateSSLDirConfig(pool *p,char *dummy)
    {
    SSLDirConfigRec *rec=pcalloc(p,sizeof(SSLDirConfigRec));

    rec->tbRequiredCiphers=make_table(p,4);
    rec->tbBannedCiphers=make_table(p,4);

    return rec;
    }

static void *MergeSSLDirConfig(pool *p,void *basev,void *addv)
    {
    SSLDirConfigRec *base=(SSLDirConfigRec *)basev;
    SSLDirConfigRec *add=(SSLDirConfigRec *)addv;
    SSLDirConfigRec *new=(SSLDirConfigRec *)palloc(p,sizeof(SSLDirConfigRec));

    new->tbRequiredCiphers=overlay_tables(p,add->tbRequiredCiphers,
					    base->tbRequiredCiphers);
    new->tbBannedCiphers=overlay_tables(p,add->tbBannedCiphers,
					  base->tbBannedCiphers);
    return new;
    }

static void InitSSL()
    {
    char *CAfile=NULL,*CApath=NULL;

#if CACHE_SESSIONS
    if(!szCacheServerPort)
	{
	fprintf(stderr,"Required SSLCacheServerPort missing\n");
	exit(1);
	}
    if(!szCacheServerPath)
	{
	fprintf(stderr,"Required SSLCacheServerPath missing\n");
	exit(1);
	}
#endif

    SSL_load_error_strings();
    ERR_load_crypto_strings();
#if SSLEAY_VERSION_NUMBER >= 0x0800
    SSLeay_add_ssl_algorithms();
#else
    /* only enable this with care: it can be used for /tmp symlink attacks.
    SSL_debug("/tmp/ssldebug");*/
#endif
    }

#if SSLEAY_VERSION_NUMBER >= 0x0800
/* FIXME: This is an expensive operation which should probably be done before
forking */
static RSA *TmpRSACallback(SSL *pSSL,int nExport)
    {
    static RSA *pRSA=NULL;

    if (pRSA == NULL)
#if SSLEAY_VERSION_NUMBER >= 0x0900
	pRSA=RSA_generate_key(512,RSA_F4,NULL,NULL);
#else
	pRSA=RSA_generate_key(512,RSA_F4,NULL);
#endif
    return pRSA;
    }
#endif

#if SSLEAY_VERSION_NUMBER >= 0x0800
int ApacheSSLVerifyCallback(int ok,X509_STORE_CTX *ctx)
    {
    X509 *xs=X509_STORE_CTX_get_current_cert(ctx);
    int depth=X509_STORE_CTX_get_error_depth(ctx);
    int error=X509_STORE_CTX_get_error(ctx);
#else
int ApacheSSLVerifyCallback(int ok,X509 *xs,X509 *xi,int depth,int error)
    {
#endif
    SSLConfigRec *pConfig=get_module_config(g_pCurrentConnection->server->module_config,&ssl_module);

#if DEBUG_VERIFY
      {
      char *s;

#if SSLEAY_VERSION_NUMBER < 0x0800
      s=X509_NAME_oneline(X509_get_subject_name(xs));
#else
      s=X509_NAME_oneline(X509_get_subject_name(xs),NULL,0);
#endif

      if(s == NULL)
	  {
	  ERR_print_errors_fp(pConfig->fileLogFile);
	  return(0);
	  }

      fprintf(pConfig->fileLogFile,"depth=%d %s\n",depth,s);
      free(s);
      }
#endif

    if(error == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY || error == X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE)
	{
	char *s;

	if(pConfig->nVerifyClient == VERIFY_OPTIONAL_NO_CA)
	    {
	    fprintf(pConfig->fileLogFile,"no issuer, returning OK\n");
	    return TRUE;
	    }
#if SSLEAY_VERSION_NUMBER < 0x0800
	s=X509_NAME_oneline(X509_get_issuer_name(xs));
#else
	s=X509_NAME_oneline(X509_get_issuer_name(xs),NULL,0);
#endif
	if(s == NULL)
	    {
	    fprintf(pConfig->fileLogFile,"verify error\n");
	    ERR_print_errors_fp(pConfig->fileLogFile);
	    SSLLogError(g_pCurrentConnection->server);
	    return(0);
	    }
	fprintf(pConfig->fileLogFile,"issuer= %s\n",s);
	free(s);
	}
    if(!ok)
	{
	fprintf(pConfig->fileLogFile,"verify error:num=%d:%s\n",error,
		X509_verify_cert_error_string(error));
	SSLLogError(g_pCurrentConnection->server);
	g_pCurrentConnection->client->szClientX509=NULL;
	}
    if(depth >= pConfig->nVerifyDepth)
	{
	fprintf(pConfig->fileLogFile,"Verify depth exceeded\n");
	log_error("Verify depth exceeded",g_pCurrentConnection->server);
	ok=0;
	}
    fprintf(pConfig->fileLogFile,"verify return:%d\n",ok);
    return(ok);
    }

static int VerifyFlags(SSLConfigRec *pConfig)
    {
    int nVerify=0;

    switch(pConfig->nVerifyClient)
	{
    case VERIFY_REQUIRE:
	nVerify|=SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
	break;

    case VERIFY_OPTIONAL:
    case VERIFY_OPTIONAL_NO_CA:
	nVerify|=SSL_VERIFY_PEER;
	break;
	}
    return nVerify;
    }

#if CACHE_SESSIONS

#define MAX_SESSION_DER	100000

static void SendSessionToServer(SSL_SESSION *pSession)
    {
    uchar buf[MAX_SESSION_DER];
    Cache *p;
    uchar *t;
    int nLength;
    SSLConfigRec *pConfig;
    time_t tExpiresAt;

    if(LocalCacheFind(pSession->session_id,pSession->session_id_length))
	return;

    pConfig=get_module_config(g_pCurrentConnection->server->module_config,&ssl_module);
    tExpiresAt=time(NULL)+pConfig->tSessionCacheTimeout;

    t=malloc(pSession->session_id_length);
    memcpy(t,pSession->session_id,pSession->session_id_length);
    p=LocalCacheAdd(t,pSession->session_id_length,tExpiresAt);

    t=buf;
    nLength=i2d_SSL_SESSION(pSession,&t);
    assert(nLength > 0);
    assert(nLength <= MAX_SESSION_DER);

    p->aucData=malloc(nLength);
    memcpy(p->aucData,buf,nLength);
    p->nData=nLength;

    GlobalCacheAdd(p->aucKey,p->nKey,p->aucData,p->nData,tExpiresAt);
    }

static SSL_SESSION *GetSessionFromServer(uchar *aucSessionID,int nLength)
    {
    int n;
    SSL_SESSION *pSession;
    Cache *p;
    uchar *t,*t2;
    int nData;

    p=LocalCacheFind(aucSessionID,nLength);
    if(!p)
	{
	time_t tExpiresAt;

	t=GlobalCacheGet(aucSessionID,nLength,&nData,&tExpiresAt);
	if(!t)
	    return NULL;
	t2=malloc(nLength);
	memcpy(t2,aucSessionID,nLength);
	p=LocalCacheAdd(t2,nLength,tExpiresAt);
	p->aucData=t;
	p->nData=nData;
	}
    else
	{
	t=p->aucData;
	nData=p->nData;
	}
    pSession=d2i_SSL_SESSION(NULL,&t,nData);
    if(!pSession)
	SSLLogErrorToFile(stderr);
    assert(pSession);
    return pSession;
    }

static int NewSessionCacheCallback(SSL *pSSL,SSL_SESSION *pNew)
    {
#if DEBUG_SESSIONS
    int n;

    fprintf(stderr,"NewSessionCacheCallback(");
    for(n=0 ; n < pNew->session_id_length ; ++n)
	fprintf(stderr,"%02x",pNew->session_id[n]);
    fprintf(stderr,")\n");
#endif

    SendSessionToServer(pNew);
    return 0;
    }

static SSL_SESSION *GetSessionCacheCallback(SSL *pSSL,unsigned char *aucSessionID,int nLength,int *pCopy)
    {
    SSL_SESSION *pSession,*p2;

#if DEBUG_SESSIONS
    int n;

    fprintf(stderr,"GetSessionCacheCallback(");
    for(n=0 ; n < nLength ; ++n)
	fprintf(stderr,"%02x",aucSessionID[n]);
    fprintf(stderr,")\n");
#endif
    *pCopy=0;
    pSession=GetSessionFromServer(aucSessionID,nLength);

    return pSession;
    }

#endif /* CACHE_SESSIONS */

static void GetCertificateAndKey(server_rec *s,SSLConfigRec *pConfig)
    {
    char buf[1024],prompt[1024];
    int n;
    FILE *f;
    char szPath[MAX_STRING_LEN];


    if(pConfig->px509Certificate)
	{
	fprintf(stderr,"Attempt to reinitialise SSL for server %s\n",
		s->server_hostname);
	return;
	}

    fprintf(stderr,"Reading certificate and key for server %s:%d\n",
	    s->server_hostname,s->port);

#if SSLEAY_VERSION_NUMBER < 0x0800
    pConfig->pSSLCtx=SSL_CTX_new();
#else
    pConfig->pSSLCtx=SSL_CTX_new(SSLv23_server_method());
    SSL_CTX_set_verify(pConfig->pSSLCtx,VerifyFlags(pConfig),ApacheSSLVerifyCallback);
#endif

#if CACHE_SESSIONS
    SSL_CTX_sess_set_new_cb(pConfig->pSSLCtx,NewSessionCacheCallback);
    SSL_CTX_sess_set_get_cb(pConfig->pSSLCtx,GetSessionCacheCallback);
#endif

    if(pConfig->szReqCiphers != NULL)
	{
	if(!SSL_CTX_set_cipher_list(pConfig->pSSLCtx,pConfig->szReqCiphers))
	    {
	    fprintf(stderr,"unable to set ciphers\n");
	    ERR_print_errors_fp(stderr);
	    SSLLogError(s);
	    }
	}

    if(((pConfig->szCACertificateFile || pConfig->szCACertificatePath)
	&& !SSL_CTX_load_verify_locations(pConfig->pSSLCtx,
					   pConfig->szCACertificateFile,
					   pConfig->szCACertificatePath)
	)
       || !SSL_CTX_set_default_verify_paths(pConfig->pSSLCtx))
	{
	fprintf(stderr,"error setting verify locations\n");
	ERR_print_errors_fp(stderr);
	exit(1);
	}

    f=fopen(pConfig->szCertificateFile,"r");
    if(!f)
	{
	sprintf(szPath,"%s/%s",X509_get_default_cert_dir(),
		pConfig->szCertificateFile);
	f=fopen(szPath,"r");
	if(!f)
	    {
	    fprintf(stderr,"Can't open certificate file %s, nor %s\n",
		    pConfig->szCertificateFile,szPath);
	    exit(1);
	    }
	}
    else
	strcpy(szPath,pConfig->szCertificateFile);  /* in case it also contains the key */

    pConfig->px509Certificate=X509_new();

    if(!PEM_read_X509(f,&pConfig->px509Certificate,NULL))
	{
	fprintf(stderr,"Error reading server certificate file %s: ",szPath);
	ERR_print_errors_fp(stderr);
	exit(1);
	}
    fclose(f);

    if(pConfig->szKeyFile)
	if(*pConfig->szKeyFile == '/')
	    strcpy(szPath,pConfig->szKeyFile);
	else
	    sprintf(szPath,"%s/private/%s",X509_get_default_cert_area(),
		    pConfig->szKeyFile);

/* Otherwise the path already contains the name of the certificate file */
    f=fopen(szPath,"r");
    if(!f)
	{
	fprintf(stderr,"Can't open key file ");
	perror(szPath);
	exit(1);
	}

    pConfig->prsaKey=RSA_new();
    if(!PEM_read_RSAPrivateKey(f,&pConfig->prsaKey,NULL))
	{
	fprintf(stderr,"Error reading private key file %s: ",szPath);
	ERR_print_errors_fp(stderr);
	exit(1);
	}

#if SSLEAY_VERSION_NUMBER >= 0x0800
    SSL_CTX_set_tmp_rsa_callback(pConfig->pSSLCtx,TmpRSACallback);
    /* Really this should have its own directive and list, but I haven't got
       the time at the moment.
     */
    SSL_CTX_set_client_CA_list(pConfig->pSSLCtx, 
	    SSL_load_client_CA_file(pConfig->szCACertificateFile)); 
#endif
    }

#if CACHE_SESSIONS
static void CacheLauncher(void *cmd)
    {
    char a1[1024];
    extern uid_t user_id;
    extern server_rec *server_conf;	/* Since the cache is global, use the main server's log files */

    error_log2stderr(server_conf);
    dup2(STDERR_FILENO,STDOUT_FILENO);
    sprintf(a1,"%d",user_id);
    execl(cmd,"gcache",a1,szCacheServerPort,NULL);
    assert(0);
    }

static void LaunchCache(pool *pPool)
    {
    int pid;

    printf("Launching... %s\n",szCacheServerPath);
    pid=spawn_child_err(pPool,CacheLauncher,szCacheServerPath,kill_after_timeout,NULL,NULL,NULL);
    printf("pid=%d\n",pid);
    }
#endif

void LaunchGlobalCache(pool *pPool)
    {
#if CACHE_SESSIONS
    LaunchCache(pPool);
#endif
    }

static void InitSSLServer(server_rec *s,pool *p)
    {
    char *szLogFile;

    InitSSL();
    for ( ; s ; s=s->next)
	{
	SSLConfigRec *pConfig=get_module_config(s->module_config,&ssl_module);

	if(pConfig->bDisabled)
	    {
	    if(!s->port)
		s->port=HTTP_PORT;
	    fprintf(stderr,"SSL disabled for server %s:%d\n",
		    s->server_hostname,s->port);
	    continue;
	    }
#if CACHE_SESSIONS
	if(!pConfig->tSessionCacheTimeout)
	    {
	    fprintf(stderr,"SSLSessionCacheTimeout not set\n");
	    exit(1);
	    }
#endif
	if(!s->port)
	    s->port=HTTPS_PORT;

	if(!pConfig->szCertificateFile)
	    {
	    fprintf(stderr,"No SSL Certificate set for server %s:%d\n",
		    s->server_hostname,s->port);
	    exit(1);
	    }
	if(pConfig->nVerifyClient < 0 || pConfig->nVerifyClient > VERIFY_OPTIONAL_NO_CA)
	    {
	    fprintf(stderr,"Bad value for SSLVerifyClient (%d)\n",pConfig->nVerifyClient);
	    exit(1);
	    }
	if(!pConfig->szLogFile)
	    {
	    fprintf(stderr,"Required SSLLogFile missing\n");
	    exit(1);
	    }

	szLogFile=server_root_relative(p,pConfig->szLogFile);
	pConfig->fileLogFile=fopen(szLogFile,"a");
	if(!pConfig->fileLogFile)
	    {
	    perror(pConfig->szLogFile);
	    exit(1);
	    }
	setbuf(pConfig->fileLogFile,NULL);
    
	GetCertificateAndKey(s,pConfig);
	}
    }

#if CACHE_SESSIONS
static const char *SSLCacheServerPort(cmd_parms *cmd,char *struct_ptr,char *arg)
    {
    szCacheServerPort=server_root_relative(cmd->pool,arg);
    InitGlobalCache(szCacheServerPort);

    return NULL;
    }

static const char *SSLCacheServerPath(cmd_parms *cmd,char *struct_ptr,char *arg)
    {
    szCacheServerPath=server_root_relative(cmd->pool,arg);

    return NULL;
    }
#endif
    

static const char *set_server_string_slot (cmd_parms *cmd,char *struct_ptr,char *arg)
    {
    /* This one should be pretty generic... */
    char *pConfig=get_module_config(cmd->server->module_config,&ssl_module);
  
    int offset=(int)cmd->info; 
    *(char **)(pConfig+offset)=arg;
    return NULL;
    }

static const char *set_server_int_slot (cmd_parms *cmd,char *struct_ptr,char *arg)
    {
  /* This one should be pretty generic... */
    char *pConfig=get_module_config(cmd->server->module_config,&ssl_module);
  
    int offset=(int)cmd->info; 
    *(int *)(pConfig+offset)=atoi(arg);
    return NULL;
    }

static const char *set_server_bool_slot (cmd_parms *cmd,char *struct_ptr)
    {
  /* This one should be pretty generic... */
    char *pConfig=get_module_config(cmd->server->module_config,&ssl_module);
  
    int offset=(int)cmd->info; 
    *(BOOL *)(pConfig+offset)=TRUE;
    return NULL;
    }

static const char *set_server_bool_slot_false (cmd_parms *cmd,char *struct_ptr)
    {
  /* This one should be pretty generic... */
    char *pConfig=get_module_config(cmd->server->module_config,&ssl_module);
  
    int offset=(int)cmd->info; 
    *(BOOL *)(pConfig+offset)=FALSE;
    return NULL;
    }

command_rec ssl_cmds[]=
    {
    { "SSLDisable",set_server_bool_slot,
      (void *)XtOffsetOf(SSLConfigRec,bDisabled),RSRC_CONF,NO_ARGS,
      "Disable SSL" },
    { "SSLEnable",set_server_bool_slot_false,
      (void *)XtOffsetOf(SSLConfigRec,bDisabled),RSRC_CONF,NO_ARGS,
      "Enable SSL" },
    { "SSLCertificateFile",set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szCertificateFile),RSRC_CONF,TAKE1,
      "PEM certificate file" },
    { "SSLCertificateKeyFile",set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szKeyFile),RSRC_CONF,TAKE1,
      "Certificate private key file (assumed to be SSLCertificateFile if absent)" },
    { "SSLCACertificatePath",set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szCACertificatePath),RSRC_CONF,TAKE1,
      "CA Certificate path (taken from SSL_CERT_DIR if absent)" },
    { "SSLCACertificateFile",set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szCACertificateFile),RSRC_CONF,TAKE1,
      "CA Certificate file (taken from SSL_CERT_FILE if absent)" },
    { "SSLVerifyDepth",set_server_int_slot,
      (void *)XtOffsetOf(SSLConfigRec,nVerifyDepth),RSRC_CONF,TAKE1,
      "Verify depth (default 0)" },
    { "SSLVerifyClient",set_server_int_slot,
      (void *)XtOffsetOf(SSLConfigRec,nVerifyClient),RSRC_CONF,TAKE1,
      "Verify client (0=no,1=optional,2=required" },
    { "SSLFakeBasicAuth",set_server_bool_slot,
      (void *)XtOffsetOf(SSLConfigRec,bFakeBasicAuth),RSRC_CONF,NO_ARGS,
      "Translate client X509 into a user name" },
    { "SSLLogFile",set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szLogFile),RSRC_CONF,TAKE1,
      "Place to dump all SSL messages that have no better home" },
    { "SSLRequiredCiphers",set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szReqCiphers),RSRC_CONF,TAKE1,
      "Colon-delimited list of required ciphers" },
    /* Per Directory */
    { "SSLRequireCipher",SSLRequireCipher,NULL,OR_FILEINFO,ITERATE,
      "Add a cipher to the per directory list of required ciphers" },
    { "SSLBanCipher",SSLBanCipher,NULL,OR_FILEINFO,ITERATE,
      "Add a cipher to the per directory list of banned ciphers" },
#if CACHE_SESSIONS
    { "SSLCacheServerPort",SSLCacheServerPort,NULL,RSRC_CONF,TAKE1,
      "Cache server TCP port number" },
    { "SSLSessionCacheTimeout",set_server_int_slot,
      (void *)XtOffsetOf(SSLConfigRec,tSessionCacheTimeout),RSRC_CONF,TAKE1,
      "Lifetime in seconds of session cache objects" },
    { "SSLCacheServerPath",SSLCacheServerPath,NULL,RSRC_CONF,TAKE1,
      "Path of the global cache server" },
#endif
    { NULL },
    };

static const char six2pr[64+1]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void uuencode(char *szTo,const char *szFrom)
    {
    const unsigned char *s;

    for(s=(const unsigned char *)szFrom ; *s ; s+=3)
	{
	*szTo++=six2pr[s[0] >> 2];
	*szTo++=six2pr[(s[0] << 4 | s[1] >> 4)&0x3f];
	if(!s[0])
	    break;
	*szTo++=six2pr[(s[1] << 2 | s[2] >> 6)&0x3f];
	if(!s[1])
	    break;
	*szTo++=six2pr[s[2]&0x3f];
	if(!s[2])
	    break;
	}
    *szTo++='\0';
    }

/*
Fake a Basic authentication from the X509 client certificate.

This must be run fairly early on to prevent a real authentication from
occuring, in particular it must be run before anything else that authenticates
a user.

This means that the Module statement for this module should be LAST in the
Configuration file.
*/

static int FakeBasicAuth(request_rec *r)
    {
    SSLConfigRec *pConfig=get_module_config(r->server->module_config,&ssl_module);
    char b1[MAX_STRING_LEN],b2[MAX_STRING_LEN];
  
    if(OPTIONAL_SSL)
	return DECLINED;
    if(!pConfig->bFakeBasicAuth)
	return DECLINED;
    if(r->connection->user)
	return DECLINED;
    if(!r->connection->client->szClientX509)
	return DECLINED;
    /*
       Fake a password - which one would be immaterial, as, it seems, an empty
       password in the users file would match ALL incoming passwords, if only we
       were using the standard crypt library routine. Unfortunately, SSLeay
       "fixes" a "bug" in crypt and thus prevents blank passwords from working.
       (IMHO what they really fix is a bug in the users of the code - failing to
       program correctly for shadow passwords).
       We need, therefore, to provide a password. This password can be matched by
       adding the string "xxj31ZMTZzkVA" as the password in the user file.
*/
    sprintf(b1,"%s:password",r->connection->client->szClientX509);
    uuencode(b2,b1);
    sprintf(b1,"Basic %s",b2);
    table_set(r->headers_in,"Authorization",b1);
    return DECLINED;
    }

static BOOL ApacheSSLSetCertStuff(conn_rec *conn)
    {
    SSLConfigRec *pConfig=get_module_config(conn->server->module_config,
					    &ssl_module);
    SSL *con=conn->client->ssl;
    char *cert_file=pConfig->szCertificateFile;
    char *key_file=pConfig->szKeyFile;

/*  PEM_set_getkey_callback(ApacheSSLKeyCallback); */
    if(cert_file != NULL)
	{
	if(SSL_use_certificate(con,pConfig->px509Certificate) <= 0)
	    {
	    fprintf(pConfig->fileLogFile,"unable to set certificate\n");
	    ERR_print_errors_fp(pConfig->fileLogFile);
	    SSLLogError(conn->server);
	    return FALSE;
	    }
	if(SSL_use_RSAPrivateKey(con,pConfig->prsaKey) <= 0)
	    {
	    fprintf(pConfig->fileLogFile,"unable to set private key\n");
	    ERR_print_errors_fp(pConfig->fileLogFile);
	    SSLLogError(conn->server);
	    return FALSE;
	    }
	}
    return TRUE;
    }

void ApacheSSLSetupVerify(conn_rec *conn)
    {
    SSLConfigRec *pConfig=get_module_config(conn->server->module_config,&ssl_module);
    int nVerify=VerifyFlags(pConfig);

    conn->client->szClientX509=NULL;

    g_pCurrentConnection=conn;

    conn->client->nVerifyError=X509_V_OK;

#if SSLEAY_VERSION_NUMBER < 0x0800
/* Why call this twice?? Ben */  
    SSL_set_verify(conn->client->ssl,nVerify,ApacheSSLVerifyCallback);
#endif
    if(!ApacheSSLSetCertStuff(conn))
	{
	fprintf(pConfig->fileLogFile,"ApacheSSLSetCertStuff failed\n");
	log_error("ApacheSSLSetCertStuff failed",conn->server);
	exit(1);
	}
#if SSLEAY_VERSION_NUMBER < 0x0800
    SSL_set_verify(conn->client->ssl,nVerify,ApacheSSLVerifyCallback);
#endif
    }

#if FREE_SESSION
/*
To make this work, SSL_free needs to be patched. Remove from SSL_free:

	if (s->ctx) SSL_CTX_free(s->ctx);

but there's more, which I haven't found.
*/

static void CloseSSL(void *_ssl)
    {
    SSL *ssl=_ssl;
printf("Closing\n");
    SSL_free(_ssl);
printf("Closed\n");
    }
#endif

int ApacheSSLSetupConnection(conn_rec * conn)
    {
    server_rec *srvr=conn->server;
    BUFF *fb=conn->client;
    SSLConfigRec *pConfig=get_module_config(srvr->module_config,
					      &ssl_module);
    char *cert_file=pConfig->szCertificateFile;
    char *key_file=pConfig->szKeyFile;

    if(pConfig->bDisabled)
	{
	fb->ssl=NULL;
	return TRUE;
	}
    
    g_pCurrentConnection=conn;

    fb->ssl=SSL_new(pConfig->pSSLCtx);
    SSL_set_fd(fb->ssl,fb->fd);
#if FREE_SESSION
    register_cleanup(conn->pool,fb->ssl,CloseSSL,CloseSSL);
#endif

    ApacheSSLSetupVerify(conn);

    while(!SSL_is_init_finished(fb->ssl))
	{
	int ret=SSL_accept(fb->ssl);
	if (ret <= 0)
	    {
	    log_error("SSL_Accept failed",srvr);
	    SSLLogError(srvr);
	    fb->flags |= B_EOF | B_EOUT;
	    return FALSE;
	    }

	if(conn->client->nVerifyError != X509_V_OK)
	    {
	    log_error("Verification failed",conn->server);
	    SSLLogError(srvr);
	    fb->flags |= B_EOF | B_EOUT;
	    return FALSE;
	    }

	if(pConfig->nVerifyClient != VERIFY_NONE)
	    {
	    char *s;
	    X509 *xs;
	    xs=SSL_get_peer_certificate(fb->ssl);
	    if(xs)
		{
		s=X509_NAME_oneline(X509_get_subject_name(xs),NULL,0);
		conn->client->szClientX509=pstrdup(conn->pool,s);
		free(s);
		}
	    }

	if(pConfig->nVerifyClient == VERIFY_REQUIRE && !conn->client->szClientX509)
	    {
	    log_error("No client certificate",conn->server);
	    SSLLogError(conn->server);
	    return 0;
	    }
	fprintf(pConfig->fileLogFile,"CIPHER is %s\n",SSL_get_cipher(conn->client->ssl));
	fflush(pConfig->fileLogFile);
	}

    /* This should be safe.... so I'll use it */
    SSL_set_read_ahead(fb->ssl,1);

    g_pCurrentConnection=NULL;

    return TRUE;
    }

module ssl_module =
    {
    STANDARD_MODULE_STUFF,
    InitSSLServer,		/* initializer */
    CreateSSLDirConfig,		/* dir config creater */
    MergeSSLDirConfig,		/* dir merger --- default is to override */
    CreateSSLServerConfig,	/* server config */
    NULL,			/* merge server config */
    ssl_cmds,			/* command table */
    NULL,			/* handlers */
    NULL,			/* filename translation */
    FakeBasicAuth,		/* check_user_id */
    NULL,			/* check auth */
    SSLCheckCipher,		/* check access */
    NULL,			/* type_checker */
    SSLFixups,			/* fixups */
    NULL,			/* logger */
    }
;

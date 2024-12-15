/* ====================================================================
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 Ben Laurie.  All rights reserved.
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
 * For more information on Apache-SSL see http://www.apache-ssl.org.
 *
 * For more information on Apache see http://www.apache.org.
 *
 * For more information on SSLeay see http://www.psy.uq.oz.au/~ftp/Crypto/.
 *
 * Ben Laurie <ben@algroup.co.uk>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>

#include "httpd.h"
#include "http_config.h"
#include "http_log.h"
#include "http_main.h"


/* Note that APACHE_SSL_EXPORT_CERTS is defined in buff.h */

/* Temp */
#define RENEG		FALSE
#define DEBUG_RENEG	FALSE
#define DEBUG_SSL_STATE FALSE

#define USE_OLD_ENVIRONMENT	FALSE	/* Set to TRUE to use the old SSL_{CLIENT,SERVER}_I* instead of SSL_{CLIENT,SERVER}_I_* */

#define SSL_IS_OPTIONAL		FALSE	/* Set to TRUE if you want to allow SSL to be disabled on a per-request basis (useful for subrequests) */

#if SSLEAY_VERSION_NUMBER >= 0x0800
/* Do session caching - only available with SSLeay 0.8.x */
# define CACHE_SESSIONS		TRUE
#endif

#if SSL_IS_OPTIONAL
# define OPTIONAL_SSL	(!r->connection->client->ssl)
#else
# define OPTIONAL_SSL	FALSE
#endif

#if SSLEAY_VERSION_NUMBER < 0x0800
# error "SSLeay versions below 0.8.x are no longer supported"
/* But note, they nearly are, if anyone cares */
#endif

#if SSLEAY_VERSION_NUMBER < 0x0900 && APACHE_SSL_EXPORT_CERTS
# error "Can't export certificates with SSLeay before 0.9.0"
#endif

#define FREE_SESSION	TRUE	/* Don't set this FALSE! */

/* define this to debug the verification stuff */
#define DEBUG_VERIFY	FALSE

#if CACHE_SESSIONS
# include "gcache.h"
static const char *s_szCacheServerPort;
static const char *s_szCacheServerDir;
static char *s_szCacheServerPath;

# define DEBUG_SESSIONS	FALSE

#endif

#if APACHE_SSL_EXPORT_CERTS && !CACHE_SESSIONS
# error "CACHE_SESSIONS must be TRUE if APACHE_SSL_EXPORT_CERTS is"
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

#if OPENSSL_VERSION_NUMBER >= 0x00904000L
# define localPEM_read_RSAPrivateKey(f,k,x) PEM_read_RSAPrivateKey(f,k,x,NULL)
# define localPEM_read_X509(f,c,x)	    PEM_read_X509(f,c,x,NULL)
#else
# define localPEM_read_RSAPrivateKey(f,k,x) PEM_read_RSAPrivateKey(f,k,x)
# define localPEM_read_X509(f,c,x)	    PEM_read_X509(f,c,x)
#endif

#define ENV_CERT_CHAIN		"SSL_CLIENT_CERT_CHAIN_"

#define UNSET	-1

/* Offsets for data stored in SSL structures */
#define EX_DATA_CONN_REC	0
#define EX_DATA_SERVER_REC	1
#if RENEG
# define EX_DATA_DIR_CONFIG	2
#endif

typedef enum
    {
    VERIFY_UNSET=-1,
    VERIFY_NONE=0,
    VERIFY_OPTIONAL=1,
    VERIFY_REQUIRE=2,
    VERIFY_OPTIONAL_NO_CA=3
    } VerifyType;

typedef enum
    {
    SSLREQ_UNSET=-1,
    SSLREQ_REQUIRED=0,
    SSLREQ_DENIED=1
    } RequireType;

typedef struct
    {
    BOOL bDisabled;

    char *szCertificateFile;
    char *szKeyFile;
    char *szCACertificatePath;
    char *szCACertificateFile;
    char *szReqCiphers;
#if !RENEG
    int nVerifyDepth;
    VerifyType nVerifyClient;
#endif

    X509 *px509Certificate;
    RSA *prsaKey;

    SSL_CTX *pSSLCtx;

    BOOL bFakeBasicAuth;
    BOOL bNoCAList;

    time_t tSessionCacheTimeout;

    array_header *aRandomFiles;
    } SSLConfigRec;

typedef struct
    {
    /* 1) If cipher is banned, refuse */
    /* 2) If RequiredCiphers is NULL, accept */
    /* 3) If the cipher isn't required, refuse */

    table *tbRequiredCiphers;
    table *tbBannedCiphers;
    RequireType nSSLRequired;
#if APACHE_SSL_EXPORT_CERTS
    BOOL bExportCerts;
#endif
#if RENEG
    int nVerifyDepth;
    VerifyType nVerifyClient;
#endif
    } SSLDirConfigRec;

MODULE_VAR_EXPORT module apache_ssl_module;

static pool *s_pPermanentPool;

static const char six2pr[64+1]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void uuencoden(unsigned char *szTo,const unsigned char *szFrom,
                     int nLength,BOOL bPad)
    {
    const unsigned char *s;
    int nPad=0;

    for(s=(const unsigned char *)szFrom ; nLength > 0 ; s+=3)
	{
	*szTo++=six2pr[s[0] >> 2];
	*szTo++=six2pr[(s[0] << 4 | s[1] >> 4)&0x3f];
	if(!--nLength)
	    {
	    nPad=2;
	    break;
	    }
	*szTo++=six2pr[(s[1] << 2 | s[2] >> 6)&0x3f];
	if(!--nLength)
	    {
	    nPad=1;
	    break;
	    }
	*szTo++=six2pr[s[2]&0x3f];
	--nLength;
	}
    while(bPad && nPad--)
	*szTo++='=';
    *szTo++='\0';
    }

static void uuencode(char *szTo,const char *szFrom,BOOL bPad)
    {
    uuencoden((unsigned char *)szTo,(const unsigned char *)szFrom,
	      strlen(szFrom),bPad);
    }

static void SSLLogError(server_rec *s,int nLevel,const char *szErr)
    {
    unsigned long l;
    char buf[MAX_STRING_LEN];

    if(szErr)
	ap_log_error(APLOG_MARK,nLevel|APLOG_NOERRNO,s,"%s",szErr);
    while((l=ERR_get_error()))
	{
	ERR_error_string(l,buf);
	ap_log_error(APLOG_MARK,nLevel|APLOG_NOERRNO,s,"%s",buf);
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

#if DEBUG_SSL_STATE

static void InfoCallback(SSL *s,int where,int ret)
    {
    char *str;
    int w;
    server_rec *pServer=SSL_get_ex_data(s,EX_DATA_SERVER_REC);

    w=where&~SSL_ST_MASK;

    if(w & SSL_ST_CONNECT)
	str="SSL_connect";
    else if(w & SSL_ST_ACCEPT)
	str="SSL_accept";
    else
	str="undefined";

    if(where & SSL_CB_LOOP)
	ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,"%s:%s",
		     str,SSL_state_string_long(s));
    else if(where & SSL_CB_ALERT)
	{
	str=(where & SSL_CB_READ)?"read":"write";
	ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,
		     "SSL3 alert %s:%s:%s",
		     str,
		     SSL_alert_type_string_long(ret),
		     SSL_alert_desc_string_long(ret));
	}
    else if(where & SSL_CB_EXIT)
	{
	if(ret == 0)
	    ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,
			 "%s:failed in %s",
			 str,SSL_state_string_long(s));
	else if (ret < 0)
	    {
	    ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,
			 "%s:error in %s",
			 str,SSL_state_string_long(s));
	    }
	}
    }

#endif /* DEBUG_SSL_STATE */

typedef struct
    {
    const char *szFile;
    int nBytes;
    } RandomFile;

static void ReadRandomFiles(server_rec *pServer,RandomFile *pRF,int nRF)
    {
    int n;

    for(n=0 ; n < nRF ; ++n,++pRF)
	{
	int r=RAND_load_file(pRF->szFile,pRF->nBytes);
	ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,
		     "Random input %s(%d) -> %d",pRF->szFile,pRF->nBytes,r);
	}
    }

#if APACHE_SSL_EXPORT_CERTS
/* szEnv must have been pool allocated, or be static */
static void AddCertToEnv(pool *p,table *e,char *szEnv,X509 *xs)
    {
    int n;
    unsigned char *s,*d;

    n=i2d_X509(xs,NULL);
    d=s=ap_palloc(p,n);
    i2d_X509(xs,&d);
    d=ap_palloc(p,(n*4)/3+2+2);
    uuencoden(d,s,n,TRUE);
    ap_table_setn(e,szEnv,(char *)d);
    }
#endif

#if SSLEAY_VERSION_NUMBER >= 0x0800
static int ApacheSSLVerifyCallback(int ok,X509_STORE_CTX *ctx)
    {
    X509 *xs=X509_STORE_CTX_get_current_cert(ctx);
    int depth=X509_STORE_CTX_get_error_depth(ctx);
    int error=X509_STORE_CTX_get_error(ctx);
    SSL *pSSL=X509_STORE_CTX_get_ex_data(ctx,
					 SSL_get_ex_data_X509_STORE_CTX_idx());
    conn_rec *pCurrentConnection=SSL_get_ex_data(pSSL,EX_DATA_CONN_REC);
#else
static int ApacheSSLVerifyCallback(int ok,X509 *xs,X509 *xi,int depth,
				   int error)
    {
#endif

#if RENEG
    SSLDirConfigRec *pConfig=SSL_get_ex_data(pSSL,EX_DATA_DIR_CONFIG);
#else
    SSLConfigRec *pConfig=
      ap_get_module_config(pCurrentConnection->server->module_config,
			   &apache_ssl_module);
#endif
    server_rec *pServer=pCurrentConnection->server;

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
	  SSLLogError(pServer,APLOG_ERR,"can't get subject name");
	  return(0);
	  }

      ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,"depth=%d %s",
		   depth,s);
      free(s);
      }
#endif
    
#if APACHE_SSL_KEEP_CERTS
    if(depth)
	{
	char *t;
	pool *p=pCurrentConnection->client->pool;

	if(!pCurrentConnection->client->tableCertChain)
	    pCurrentConnection->client->tableCertChain=ap_make_table(p,5);

	t=ap_psprintf(p,ENV_CERT_CHAIN "%d",depth);
	AddCertToEnv(p,pCurrentConnection->client->tableCertChain,t,xs);
	}
#endif

    if(error == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY
       || error == X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE
       || error == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT
       || error == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN)
	{
	char *s;

	if(pConfig->nVerifyClient == VERIFY_OPTIONAL_NO_CA)
	    {
	    ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,
			 "no issuer, returning OK");
	    return TRUE;
	    }
#if SSLEAY_VERSION_NUMBER < 0x0800
	s=X509_NAME_oneline(X509_get_issuer_name(xs));
#else
	s=X509_NAME_oneline(X509_get_issuer_name(xs),NULL,0);
#endif
	if(s == NULL)
	    {
	    SSLLogError(pServer,APLOG_ERR,"verify error");
	    return FALSE;
	    }
	ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,"issuer=%s",
		     s);
	free(s);
	}
    if(!ok)
	{
	ap_log_error(APLOG_MARK,APLOG_ERR|APLOG_NOERRNO,pServer,
		      "verify error:num=%d:%s",error,
		      X509_verify_cert_error_string(error));
	SSLLogError(pCurrentConnection->server,APLOG_ERR,NULL);
	pCurrentConnection->client->szClientX509=NULL;
	}
    if(depth >= pConfig->nVerifyDepth)
	{
	ap_log_error(APLOG_MARK,APLOG_ERR|APLOG_NOERRNO,
		     pServer,"Verify depth exceeded");
	ok=0;
	}
    ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,pServer,
		 "verify return:%d",ok);
    return ok;
    }

#if RENEG
static int VerifyFlags(SSLDirConfigRec *pConfig)
#else
static int VerifyFlags(SSLConfigRec *pConfig)
#endif
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

    default: break; /* avoid warnings from gcc */
	}

    return nVerify;
    }

static int SSLCheckCipher(request_rec *r)
    {
    const char *cipher;
    SSLDirConfigRec *rec=(SSLDirConfigRec *)
      ap_get_module_config(r->per_dir_config,&apache_ssl_module);
    SSLConfigRec *pConfig=(SSLConfigRec *)
      ap_get_module_config(r->server->module_config,&apache_ssl_module);

    if(rec->nSSLRequired == SSLREQ_DENIED && r->connection->client->ssl)
	{
	ap_log_reason("SSL denied",r->filename,r);
	return FORBIDDEN;
	}

    if(rec->nSSLRequired == SSLREQ_REQUIRED && !r->connection->client->ssl)
	{
	ap_log_reason("SSL required",r->filename,r);
	return FORBIDDEN;
	}

    /* Check to see if SSL is on */
    if(pConfig->bDisabled || OPTIONAL_SSL)
	return DECLINED;

#if RENEG
# if DEBUG_RENEG
    fprintf(stderr,"Renegotiated for %s (%x)!\n",r->filename,VerifyFlags(rec));
# endif
    if(1)
	{
	SSL_set_ex_data(r->connection->client->ssl,EX_DATA_DIR_CONFIG,rec);
	SSL_set_verify(r->connection->client->ssl,VerifyFlags(rec),
		       ApacheSSLVerifyCallback);
	/* Set the context. This is probably not optimal (but nor is the whole,
	   of renegotiation. But it certainly should avoid inappropriate
	   session reuse. */
	SSL_set_session_id_context(r->connection->client->ssl,
				   (unsigned char *)&rec,sizeof rec);
	SSL_renegotiate(r->connection->client->ssl);
	SSL_do_handshake(r->connection->client->ssl);

	r->connection->client->ssl->state=SSL_ST_ACCEPT;
	SSL_do_handshake(r->connection->client->ssl);
	}

    if(rec->nVerifyClient != VERIFY_NONE && rec->nVerifyClient != VERIFY_UNSET)
	{
	char *s;
	X509 *xs;
	xs=SSL_get_peer_certificate(r->connection->client->ssl);
	if(xs)
	    {
	    s=X509_NAME_oneline(X509_get_subject_name(xs),NULL,0);
	    r->connection->client->szClientX509=ap_pstrdup(r->connection->pool,
							   s);
	    free(s);
	    }
	}

    if(rec->nVerifyClient == VERIFY_REQUIRE
       && !r->connection->client->szClientX509)
	{
	SSLLogError(r->connection->server,APLOG_ERR,"No client certificate");
	return FORBIDDEN;
	}
#endif

    cipher=SSL_get_cipher(r->connection->client->ssl);

    if(ap_table_get(rec->tbBannedCiphers,cipher))
	{
	char *buf;

	buf=ap_pstrcat(r->pool,"Cipher ",cipher," is forbidden",NULL);
	ap_log_reason(buf,r->filename,r);
	return FORBIDDEN;
	}

    if(ap_table_get(rec->tbRequiredCiphers,cipher))
	return OK;

    if(ap_is_empty_table(rec->tbRequiredCiphers))
	return OK;
    else
	{
	char *buf;
	
	buf=ap_pstrcat(r->pool,"Cipher ",cipher,
		       " is not on the permitted list",NULL);
	ap_log_reason(buf,r->filename,r);
	return FORBIDDEN;
	}
    }

static void StrUpper(char *s)
    {
    for( ; *s ; ++s)
	*s=toupper(*s);
    }

static void ExpandCert(pool *p,table *pEnv,char *szPrefix,char *szCert)
    {
    char buf[HUGE_STRING_LEN];
    char *s,*t;
    /* Expand a X509_oneline entry into it's base components and register
       them as environment variables. Needed if you want to pass certificate
       information to CGI's. The naming convention SHOULD be fairly compatible
       with CGI's written for stronghold's certificate info  - Q */
    /* FIXME - strtok() and strcspn() may cause problems on some systems - Q */

    t=ap_psprintf(p,"%sDN",szPrefix);
    ap_table_setn(pEnv,t,ap_pstrdup(p,szCert));
    
    ap_cpystrn(buf,szCert,sizeof buf);
    for(s=strtok(buf,"/") ; s != NULL ; s=strtok(NULL,"/"))
	{
	int n=strcspn(s,"=");
	s[n]='\0';
	StrUpper(s);
	t=ap_pstrcat(p,szPrefix,s,NULL);
	ap_table_setn(pEnv,t,ap_pstrdup(p,s+n+1));
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

void ApacheSSLAddCommonVars(request_rec *r)
    {
    table *e=r->subprocess_env;
    int keysize=0;
    int secretkeysize=0;
    char *t;
    const char *cipher;
    int n;
    X509 *xs;
#if APACHE_SSL_EXPORT_CERTS && !APACHE_SSL_KEEP_CERTS
    STACK *pCertStack;
#endif

    SSLConfigRec *pConfig=(SSLConfigRec *)
      ap_get_module_config(r->server->module_config,&apache_ssl_module);
#if APACHE_SSL_EXPORT_CERTS
    SSLDirConfigRec *pDirConfig=(SSLDirConfigRec *)
      ap_get_module_config(r->per_dir_config,&apache_ssl_module);
#endif

    /* Check to see if SSL is on */
    if(pConfig->bDisabled || OPTIONAL_SSL)
	return;

    cipher=SSL_get_cipher(r->connection->client->ssl);
    ap_table_set(e,"HTTPS","on");
    ap_table_set(e,"HTTPS_CIPHER",cipher);
    ap_table_set(e,"SSL_CIPHER",cipher);
    ap_table_set(e,"SSL_PROTOCOL_VERSION",
		 SSL_get_version(r->connection->client->ssl));
    ap_table_set(e,"SSL_SSLEAY_VERSION", SSLeay_version(SSLEAY_VERSION));

    for(n=0 ; aCipherspecs[n].szName ; ++n)
	if(!strcmp(cipher,aCipherspecs[n].szName))
	    {
	    keysize=aCipherspecs[n].nKeySize;
	    secretkeysize=aCipherspecs[n].nSecretKeySize;
	    break;
	    }

    t=ap_psprintf(r->pool,"%d",keysize);
    ap_table_setn(e,"HTTPS_KEYSIZE",t);

    t=ap_psprintf(r->pool,"%d",secretkeysize);
    ap_table_setn(e,"HTTPS_SECRETKEYSIZE",t);

    if(r->connection->client->szClientX509)
	{
	ExpandCert(r->pool,e,"SSL_CLIENT_",
		   r->connection->client->szClientX509);
	xs=SSL_get_peer_certificate(r->connection->client->ssl);
	ExpandCert(r->pool,e,
#if USE_OLD_ENVIRONMENT
		   "SSL_CLIENT_I",
#else
		   "SSL_CLIENT_I_",
#endif
		   X509_NAME_oneline(X509_get_issuer_name(xs),NULL,0));
#if APACHE_SSL_EXPORT_CERTS
	if(pDirConfig->bExportCerts)
	    AddCertToEnv(r->pool,e,"SSL_CLIENT_CERT",xs);
#endif
	}

    xs=SSL_get_certificate(r->connection->client->ssl);
    ExpandCert(r->pool,e,"SSL_SERVER_",
	       X509_NAME_oneline(X509_get_subject_name(xs),NULL,0));
    ExpandCert(r->pool,e,
#if USE_OLD_ENVIRONMENT
	       "SSL_SERVER_I",
#else
	       "SSL_SERVER_I_",
#endif
	       X509_NAME_oneline(X509_get_issuer_name(xs),NULL,0));

#if APACHE_SSL_EXPORT_CERTS
# if !APACHE_SSL_KEEP_CERTS
    pCertStack=SSL_get_peer_cert_chain(r->connection->client->ssl);
    for(n=0 ; pCertStack && n < sk_num(pCertStack) ; ++n)
	{
	t=ap_psprintf(r->pool,ENV_CERT_CHAIN "%d",n+1);
	AddCertToEnv(r->connection->client->pool,e,t,(X509 *)sk_value(pCertStack,n));
	}
# else
    if(r->connection->client->tableCertChain && pDirConfig->bExportCerts)
	r->subprocess_env=
	  ap_overlay_tables(r->pool,r->subprocess_env,
			    r->connection->client->tableCertChain);
# endif
#endif

    return;
    }

/*
static int SSLFixups(request_rec *r)
    {
    SSLAddCommonVars(r);
    return DECLINED;
    }
*/

static void *CreateSSLServerConfig(pool *p,server_rec *s)
    {
    SSLConfigRec *rec=ap_pcalloc(p,sizeof(SSLConfigRec));
    
    rec->bDisabled=UNSET;	/* Will become FALSE */
    rec->szCertificateFile=NULL;
    rec->szKeyFile=NULL;
    rec->szCACertificatePath=NULL;
    rec->szCACertificateFile=NULL;
    rec->szReqCiphers=NULL;
#if !RENEG
    rec->nVerifyDepth=0;
    rec->nVerifyClient=VERIFY_UNSET;	/* Will become VERIFY_NONE */
#endif
    rec->px509Certificate=NULL;
    rec->prsaKey=NULL;
    rec->pSSLCtx=NULL;
    rec->bFakeBasicAuth=UNSET;		/* Will become FALSE */
    rec->bNoCAList=UNSET;
    rec->tSessionCacheTimeout=0;
    rec->aRandomFiles=ap_make_array(p,1,sizeof(RandomFile));

    return rec;
    }

#define Merge(el,unset)		new->el=(add->el == unset ? base->el : add->el)
#define MergeString(el)		Merge(el,NULL)
#define MergeBool(el)		Merge(el,UNSET)
#define MergeInt(el)		Merge(el,0)

static void *MergeSSLServerConfig(pool *p,void *basev,void *addv)
    {
    SSLConfigRec *base=(SSLConfigRec *)basev;
    SSLConfigRec *add=(SSLConfigRec *)addv;
    SSLConfigRec *new=(SSLConfigRec *)ap_pcalloc(p,sizeof(SSLConfigRec));

    MergeBool(bDisabled);
    MergeString(szCertificateFile);
    MergeString(szKeyFile);
    MergeString(szCACertificatePath);
    MergeString(szCACertificateFile);
#if !RENEG
    MergeInt(nVerifyDepth);
    Merge(nVerifyClient,VERIFY_UNSET);
#endif
    MergeBool(bFakeBasicAuth);
    MergeBool(bNoCAList);
    MergeInt(tSessionCacheTimeout);
    new->aRandomFiles=ap_append_arrays(p,base->aRandomFiles,add->aRandomFiles);

    return new;
    }

static void *CreateSSLDirConfig(pool *p,char *dummy)
    {
    SSLDirConfigRec *rec=ap_pcalloc(p,sizeof(SSLDirConfigRec));

    rec->tbRequiredCiphers=ap_make_table(p,4);
    rec->tbBannedCiphers=ap_make_table(p,4);
    rec->nSSLRequired=SSLREQ_UNSET;
#if APACHE_SSL_EXPORT_CERTS
    rec->bExportCerts=FALSE;
#endif
#if RENEG
    rec->nVerifyDepth=0;
    rec->nVerifyClient=VERIFY_UNSET;	/* Won't become VERIFY_NONE */
#endif

    return rec;
    }

static void *MergeSSLDirConfig(pool *p,void *basev,void *addv)
    {
    SSLDirConfigRec *base=(SSLDirConfigRec *)basev;
    SSLDirConfigRec *add=(SSLDirConfigRec *)addv;
    SSLDirConfigRec *new=(SSLDirConfigRec *)
      ap_palloc(p,sizeof(SSLDirConfigRec));

    new->tbRequiredCiphers=ap_overlay_tables(p,add->tbRequiredCiphers,
					     base->tbRequiredCiphers);
    new->tbBannedCiphers=ap_overlay_tables(p,add->tbBannedCiphers,
					   base->tbBannedCiphers);
    Merge(nSSLRequired,SSLREQ_UNSET);
#if APACHE_SSL_EXPORT_CERTS
    new->bExportCerts=base->bExportCerts || add->bExportCerts;
#endif
#if RENEG
    MergeInt(nVerifyDepth);
    Merge(nVerifyClient,VERIFY_UNSET);
#endif

    return new;
    }

static void InitSSL(server_rec *s)
    {
#if CACHE_SESSIONS
    if(!s_szCacheServerPort)
	{
	ap_log_error(APLOG_MARK,APLOG_CRIT|APLOG_NOERRNO,s,
		"Required SSLCacheServerPort missing");
	exit(1);
	}
    if(!s_szCacheServerPath)
	{
	ap_log_error(APLOG_MARK,APLOG_CRIT|APLOG_NOERRNO,s,
		     "Required SSLCacheServerPath missing");
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

#if SSLEAY_VERSION_NUMBER >= 0x0800 && SSLEAY_VERSION_NUMBER < 0x0920
/* FIXME: This ought to generate a new key from time to time - why make the
Feds' life easy? */
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

#if SSLEAY_VERSION_NUMBER >= 0x0920
/* FIXME: This ought to generate a new key from time to time - why make the
Feds' life easy? */
static RSA *TmpRSACallback(SSL *pSSL,int nExport,int nKeyLength)
    {
    static RSA *pRSA512=NULL;
    static RSA *pRSA1024=NULL;

    ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,
		 SSL_get_ex_data(pSSL,EX_DATA_SERVER_REC),
		 "Generating %d bit key",nKeyLength);
    assert(nExport);  /* Otherwise it was a sign-only key and we should set our own keylength */
    assert(nKeyLength == 512 || nKeyLength == 1024);
    if (pRSA512 == NULL && nKeyLength == 512)
	pRSA512=RSA_generate_key(512,RSA_F4,NULL,NULL);
    if (pRSA1024 == NULL && nKeyLength == 1024)
	pRSA1024=RSA_generate_key(1024,RSA_F4,NULL,NULL);
    
    return nKeyLength == 512 ? pRSA512 : pRSA1024;
    }
#endif

#if CACHE_SESSIONS

#define MAX_SESSION_DER	100000

/*
  If *pbLocal is TRUE, only do a local search. Also, set *pbLocal if the key
  was found locally.
  */
static uchar *CacheFind(uchar *aucKey,int nKey,int *pnData,BOOL *pbLocal)
    {
    Cache *p;
    uchar *t;

    p=LocalCacheFind(aucKey,nKey);

    if(!p)
	{
	uchar *t2;
	time_t tExpiresAt;

	if(*pbLocal)
	    return NULL;

	t=GlobalCacheGet(aucKey,nKey,pnData,&tExpiresAt);
	if(!t)
	    return NULL;

	t2=malloc(nKey);
	memcpy(t2,aucKey,nKey);
	p=LocalCacheAdd(t2,nKey,tExpiresAt);
	p->aucData=t;
	p->nData=*pnData;
	}
    else
	{
	t=p->aucData;
	*pnData=p->nData;
	*pbLocal=TRUE;
	}
    return t;
    }

#if APACHE_SSL_KEEP_CERTS
static uchar *MakeCertKey(Cache *pCache,const char *szKey,int *pnLength)
    {
    uchar *buf=malloc(*pnLength=pCache->nKey+strlen(szKey));
    memcpy(buf,pCache->aucKey,pCache->nKey);
    memcpy(buf+pCache->nKey,szKey,strlen(szKey));

    return buf;
    }

static int SendCertToServer(void *extra,const char *szKey,const char *szValue)
    {
    Cache *pCache=extra;
    int nKey;
    uchar *aucKey;
    Cache *pCertCache;

    aucKey=MakeCertKey(pCache,szKey,&nKey);
    pCertCache=LocalCacheAdd(aucKey,nKey,pCache->tExpiresAt);
    pCertCache->aucData=(uchar *)strdup(szValue);
    pCertCache->nData=strlen(szValue)+1;
    GlobalCacheAdd(pCertCache->aucKey,pCertCache->nKey,pCertCache->aucData,
		   pCertCache->nData,pCertCache->tExpiresAt);
    return 1;
    }

static uchar *CacheFindS(char *aucKey,int nKey,int *pnData,BOOL *pbLocal)
    { return CacheFind((uchar *)aucKey,nKey,pnData,pbLocal); }

static void SendCertChainToServer(Cache *pCache,conn_rec *pCurrentConnection)
    {
    if(!pCurrentConnection->client->tableCertChain)
	return;
    ap_table_do(SendCertToServer,pCache,
		pCurrentConnection->client->tableCertChain,NULL);
    }

static void GetCertChainFromServer(uchar *aucSessionID,int nLength,
				   BOOL bLocal,conn_rec *pCurrentConnection)
    {
    int n;
    char buf[MAX_SESSION_DER];

    ap_assert(nLength < MAX_SESSION_DER);
    memcpy(buf,aucSessionID,nLength);
    for(n=1 ; ; ++n)
	{
	uchar *t;
	int nData;
	int l;

	ap_snprintf(buf+nLength,sizeof buf-nLength,ENV_CERT_CHAIN "%d",n);
	l=nLength+strlen(buf+nLength);

	t=CacheFindS(buf,l,&nData,&bLocal);
	if(!t)
	    break;

	if(!pCurrentConnection->client->tableCertChain)
	    pCurrentConnection->client->tableCertChain=
	      ap_make_table(pCurrentConnection->client->pool,5);

	ap_table_set(pCurrentConnection->client->tableCertChain,
		     buf+nLength,(char *)t);
	}
    }
#endif /* APACHE_SSL_KEEP_CERTS */

static void SendSessionToServer(SSL_SESSION *pSession,
				conn_rec *pCurrentConnection)
    {
    uchar buf[MAX_SESSION_DER];
    Cache *p;
    uchar *t;
    int nLength;
    SSLConfigRec *pConfig;
    time_t tExpiresAt;

    if(LocalCacheFind(pSession->session_id,pSession->session_id_length))
	return;

    pConfig=ap_get_module_config(pCurrentConnection->server->module_config,
				 &apache_ssl_module);
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
#if APACHE_SSL_KEEP_CERTS
    SendCertChainToServer(p,pCurrentConnection);
#endif
    }

static SSL_SESSION *GetSessionFromServer(uchar *aucSessionID,int nLength,
					 conn_rec *pCurrentConnection)
    {
    SSL_SESSION *pSession;
    uchar *t;
    int nData;
    BOOL bLocal=FALSE;

#if APACHE_SSL_KEEP_CERTS
    GetCertChainFromServer(aucSessionID,nLength,bLocal,pCurrentConnection);
#endif

    t=CacheFind(aucSessionID,nLength,&nData,&bLocal);
    if(!t)
	return NULL;

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

    SendSessionToServer(pNew,SSL_get_ex_data(pSSL,EX_DATA_CONN_REC));
    return 0;
    }

static SSL_SESSION *GetSessionCacheCallback(SSL *pSSL,
					    unsigned char *aucSessionID,
					    int nLength,int *pCopy)
    {
    SSL_SESSION *pSession;

#if DEBUG_SESSIONS
    int n;

    fprintf(stderr,"GetSessionCacheCallback(");
    for(n=0 ; n < nLength ; ++n)
	fprintf(stderr,"%02x",aucSessionID[n]);
    fprintf(stderr,")\n");
#endif
    *pCopy=0;
    pSession=GetSessionFromServer(aucSessionID,nLength,
				  SSL_get_ex_data(pSSL,EX_DATA_CONN_REC));

    return pSession;
    }

#endif /* CACHE_SESSIONS */

static void GetPrivateKey(server_rec *s,SSLConfigRec *pConfig,
			  const char *szKeyFile)
    {
    static table *s_pKeyTable;
    RSA *prsaKey;
    FILE *f;
    int nCount;

    if(!s_pKeyTable)
	{
	s_pKeyTable=ap_make_table(s_pPermanentPool,10);
	assert(s_pKeyTable);
	}
    prsaKey=(RSA *)ap_table_get(s_pKeyTable,szKeyFile);
    if(prsaKey)
	{
	pConfig->prsaKey=prsaKey;
	return;
	}

    fprintf(stderr,"Reading key for server %s:%d\n",s->server_hostname,
	    s->port);

    f=fopen(szKeyFile,"r");
    if(!f)
	{
	ap_log_error(APLOG_MARK,APLOG_CRIT,s,"Can't open key file %s",
		     szKeyFile);
	exit(1);
	}

    pConfig->prsaKey=RSA_new();
    for(nCount=0 ; !localPEM_read_RSAPrivateKey(f,&pConfig->prsaKey,NULL)
	  ; ++nCount)
	{
	if(nCount < 3 && ERR_peek_error() == ERR_PACK(ERR_LIB_EVP,
						      EVP_F_EVP_DECRYPTFINAL,
						      EVP_R_BAD_DECRYPT))
	    {
	    fprintf(stderr,"Bad passphrase - try again\n");
	    rewind(f);
	    continue;
	    }
	ap_log_error(APLOG_MARK,APLOG_CRIT,s,
		     "Error reading private key file %s: ",szKeyFile);
	SSLLogError(s,APLOG_CRIT,NULL);
	exit(1);
	}
    fclose(f);

    ap_table_setn(s_pKeyTable,ap_pstrdup(s_pPermanentPool,szKeyFile),
		  (const char *)pConfig->prsaKey);
    }

#if SSLEAY_VERSION_NUMBER < 0x0920
static int X509NameCompare(X509_NAME **a,X509_NAME **b)
    {
    return X509_NAME_cmp(*a,*b);
    }
#endif

static void GetCertificateAndKey(server_rec *s,pool *p,SSLConfigRec *pConfig)
    {
    FILE *f;
    char szPath[MAX_STRING_LEN];
#if SSLEAY_VERSION_NUMBER >= 0x0920
    STACK_OF(X509_NAME) *stackCAs=NULL;
#endif

    if(pConfig->px509Certificate)
	{
	ap_log_error(APLOG_MARK,APLOG_NOERRNO|APLOG_CRIT,s,
		     "Attempt to reinitialise SSL for server %s",
		     s->server_hostname);
	exit(1);
	}

#if SSLEAY_VERSION_NUMBER < 0x0800
    pConfig->pSSLCtx=SSL_CTX_new();
#else
    pConfig->pSSLCtx=SSL_CTX_new(SSLv23_server_method());
# if RENEG
//    s_nVerifyClient=VERIFY_OPTIONAL_NO_CA;
    SSL_CTX_set_verify(pConfig->pSSLCtx,0,ApacheSSLVerifyCallback);
# else
    SSL_CTX_set_verify(pConfig->pSSLCtx,VerifyFlags(pConfig),ApacheSSLVerifyCallback);
# endif
#endif

#if CACHE_SESSIONS
    SSL_CTX_sess_set_new_cb(pConfig->pSSLCtx,NewSessionCacheCallback);
    SSL_CTX_sess_set_get_cb(pConfig->pSSLCtx,GetSessionCacheCallback);
# if APACHE_SSL_EXPORT_CERTS
    SSL_CTX_set_session_cache_mode(pConfig->pSSLCtx,
				   SSL_CTX_get_session_cache_mode(pConfig
								  ->pSSLCtx)
				   |SSL_SESS_CACHE_NO_INTERNAL_LOOKUP);
# endif
#endif

    if(pConfig->szReqCiphers != NULL)
	{
	if(!SSL_CTX_set_cipher_list(pConfig->pSSLCtx,pConfig->szReqCiphers))
	    {
	    SSLLogError(s,APLOG_CRIT,"unable to set ciphers");
	    exit(1);
	    }
	}

    if(((pConfig->szCACertificateFile || pConfig->szCACertificatePath)
	&& !SSL_CTX_load_verify_locations(pConfig->pSSLCtx,
					   pConfig->szCACertificateFile,
					   pConfig->szCACertificatePath)
	)
       || !SSL_CTX_set_default_verify_paths(pConfig->pSSLCtx))
	{
	SSLLogError(s,APLOG_CRIT,"error setting verify locations");
	exit(1);
	}

    f=fopen(pConfig->szCertificateFile,"r");
    if(!f)
	{
	ap_snprintf(szPath,sizeof szPath,"%s/%s",X509_get_default_cert_dir(),
		pConfig->szCertificateFile);
	f=fopen(szPath,"r");
	if(!f)
	    {
	    ap_log_error(APLOG_MARK,APLOG_CRIT|APLOG_NOERRNO,s,
			 "Can't open certificate file %s, nor %s",
			 pConfig->szCertificateFile,szPath);
	    exit(1);
	    }
	}
    else
	strcpy(szPath,pConfig->szCertificateFile);  /* in case it also contains the key */

    pConfig->px509Certificate=X509_new();

    if(!localPEM_read_X509(f,&pConfig->px509Certificate,NULL))
	{
	ap_log_error(APLOG_MARK,APLOG_CRIT|APLOG_NOERRNO,s,
		     "Error reading server certificate file %s",szPath);
	SSLLogError(s,APLOG_CRIT,NULL);
	exit(1);
	}
    fclose(f);

    if(pConfig->szKeyFile)
	if(*pConfig->szKeyFile == '/')
	    strcpy(szPath,pConfig->szKeyFile);
	else
	    ap_snprintf(szPath,sizeof szPath,"%s/private/%s",
			X509_get_default_cert_area(),pConfig->szKeyFile);

/* Otherwise the path already contains the name of the certificate file */
    GetPrivateKey(s,pConfig,szPath);

#if SSLEAY_VERSION_NUMBER >= 0x0800
    SSL_CTX_set_tmp_rsa_callback(pConfig->pSSLCtx,TmpRSACallback);
#endif

#if SSLEAY_VERSION_NUMBER >= 0x0920
#if RENEG
    if(!pConfig->bNoCAList)
#else
    if(pConfig->nVerifyClient != VERIFY_OPTIONAL_NO_CA && !pConfig->bNoCAList)
#endif
	{
	if(pConfig->szCACertificateFile || pConfig->szCACertificatePath)
	    stackCAs=sk_X509_NAME_new(NULL);

	if(pConfig->szCACertificateFile
	   && !SSL_add_file_cert_subjects_to_stack(stackCAs,
					      pConfig->szCACertificateFile))
	    {
	    SSLLogError(s,APLOG_CRIT,"error reading CA certs");
	    exit(1);
	    }

	if(pConfig->szCACertificatePath
	   && !SSL_add_dir_cert_subjects_to_stack(stackCAs,
					     pConfig->szCACertificatePath))
	    {
	    SSLLogError(s,APLOG_CRIT,"error reading CA certs");
	    exit(1);
	    }

	if(stackCAs && sk_X509_NAME_num(stackCAs) > 0)
	    SSL_CTX_set_client_CA_list(pConfig->pSSLCtx,stackCAs);
	}
#elif SSLEAY_VERSION_NUMBER >= 0x0800
    if(pConfig->szCACertificateFile)
	SSL_CTX_set_client_CA_list(pConfig->pSSLCtx,
		SSL_load_client_CA_file(pConfig->szCACertificateFile));

    if(pConfig->szCACertificatePath)
	{
	pool *p2=ap_make_sub_pool(p);
	DIR *d=ap_popendir(p2,pConfig->szCACertificatePath);
	struct DIR_TYPE *dstruct;
	STACK *stackCAs;

	/* Note that a side effect is that the CAs will be sorted by name */
	stackCAs=sk_new(X509NameCompare);
	if(!d)
	    {
	    ap_log_error(APLOG_MARK,APLOG_ERR,s,"Can't open directory %s",
			 pConfig->szCACertificatePath);
	    exit(1);
	    }

	while((dstruct=readdir(d)))
	    {
	    STACK *s;
	    int n;
	    char *szFile;
	    
	    szFile=ap_pstrcat(p2,pConfig->szCACertificatePath,"/",
			      dstruct->d_name,NULL);
	    s=SSL_load_client_CA_file(szFile);
	    for(n=0 ; s && n < sk_num(s) ; ++n)
		{
		if(sk_find(stackCAs,sk_value(s,n)) >= 0)
		    continue;
	        sk_push(stackCAs,sk_value(s,n));
		}
	    }

	sk_set_cmp_func(stackCAs,NULL);
	SSL_CTX_set_client_CA_list(pConfig->pSSLCtx,stackCAs);
	ap_pclosedir(p2,d);
	ap_destroy_pool(p2);
	}
#endif
    }

#if CACHE_SESSIONS
static server_rec *s_pServer;
extern uid_t ap_user_id;

static int CacheLauncher(void *cmd,child_info *pInfo)
    {
    char a1[1024];

    ap_error_log2stderr(s_pServer);
    dup2(STDERR_FILENO,STDOUT_FILENO);
    ap_snprintf(a1,sizeof a1,"%d",ap_user_id);
    ap_cleanup_for_exec();
    if(s_szCacheServerDir)
	{
	if(chdir(s_szCacheServerDir) < 0)
	    perror(s_szCacheServerDir);
	}
    ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,s_pServer,
		 "gcache pid is %d",getpid());
    execl(cmd,"gcache",a1,s_szCacheServerPort,NULL);
    perror(cmd);
    assert(0);
    return 1;
    }

static void LaunchCache(server_rec *s,pool *pPool)
    {
    int pid;

    s_pServer=s;
    printf("Launching... %s\n",s_szCacheServerPath);
    pid=ap_spawn_child(pPool,CacheLauncher,s_szCacheServerPath,kill_always,
		       NULL,NULL,NULL);
    printf("pid=%d\n",pid);
    }
#endif

static void InitSSLServer(server_rec *s,pool *p)
    {
    int n;
    server_rec *pMainServer=s;
    int nSaveStderr;

    /* If you were going to make Apache-SSL add a version string by the new
       method, here's where to do it. However, it doesn't seem right to me to
       do that, since if APACHE_SSL is defined, the server is modified,
       regardless of whether this module is included. - Ben 3/8/98

       ap_add_version_component(APACHE_SSL_VERSION);

       */

    if(!s_pPermanentPool)
	{
	s_pPermanentPool=ap_make_sub_pool(NULL);
	assert(s_pPermanentPool);
	}

    nSaveStderr=dup(2);
    close(2);
    n=dup(1);
    assert(n == 2);

    InitSSL(s);
    for ( ; s ; s=s->next)
	{
	SSLConfigRec *pConfig=ap_get_module_config(s->module_config,
						   &apache_ssl_module);

	/* Fix up stuff that may not have been set */

	if(pConfig->bDisabled == UNSET)
	    pConfig->bDisabled=FALSE;
#if !RENEG
	if(pConfig->nVerifyClient == VERIFY_UNSET)
	    pConfig->nVerifyClient=VERIFY_NONE;
#endif
	if(pConfig->bFakeBasicAuth == UNSET)
	    pConfig->bFakeBasicAuth=FALSE;
	if(pConfig->bNoCAList == UNSET)
	    pConfig->bNoCAList=FALSE;

	/* End of fixups */

	if(pConfig->bDisabled)
	    {
	    if(!s->port)
		s->port=DEFAULT_HTTP_PORT;
	    ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,s,
			 "SSL disabled for server %s:%d\n",
			 s->server_hostname,s->port);
	    continue;
	    }
#if CACHE_SESSIONS
	if(!pConfig->tSessionCacheTimeout)
	    {
	    ap_log_error(APLOG_MARK,APLOG_CRIT|APLOG_NOERRNO,s,
			 "SSLSessionCacheTimeout not set");
	    exit(1);
	    }
#endif
	if(!s->port)
	    s->port=DEFAULT_HTTPS_PORT;

	if(!pConfig->szCertificateFile)
	    {
	    ap_log_error(APLOG_MARK,APLOG_CRIT|APLOG_NOERRNO,s,
			 "No SSL Certificate set for server %s:%d",
			 s->server_hostname,s->port);
	    exit(1);
	    }
#if !RENEG
	if(pConfig->nVerifyClient < 0
	   || pConfig->nVerifyClient > VERIFY_OPTIONAL_NO_CA)
	    {
	    ap_log_error(APLOG_MARK,APLOG_CRIT|APLOG_NOERRNO,s,
			 "Bad value for SSLVerifyClient (%d)",
			 pConfig->nVerifyClient);
	    exit(1);
	    }
#endif
	GetCertificateAndKey(s,p,pConfig);
	}
#if CACHE_SESSIONS
    LaunchCache(pMainServer,p);
#endif
    close(2);
    n=dup(nSaveStderr);
    assert(n == 2);
    close(nSaveStderr);
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
    SSLConfigRec *pConfig=ap_get_module_config(r->server->module_config,
					       &apache_ssl_module);
    char b1[MAX_STRING_LEN],b2[MAX_STRING_LEN];
    char *t;
  
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
      We need, therefore, to provide a password. This password can be matched
      by adding the string "xxj31ZMTZzkVA" as the password in the user file.
      */
    ap_snprintf(b1,sizeof b1,"%s:password",
		r->connection->client->szClientX509);
    uuencode(b2,b1,FALSE);
    t=ap_psprintf(r->pool,"Basic %s",b2);
    ap_table_setn(r->headers_in,"Authorization",t);
    return DECLINED;
    }

static BOOL ApacheSSLSetCertStuff(conn_rec *conn)
    {
    SSLConfigRec *pConfig=ap_get_module_config(conn->server->module_config,
					       &apache_ssl_module);
    SSL *con=conn->client->ssl;
    char *cert_file=pConfig->szCertificateFile;

/*  PEM_set_getkey_callback(ApacheSSLKeyCallback); */
    if(cert_file != NULL)
	{
	if(SSL_use_certificate(con,pConfig->px509Certificate) <= 0)
	    {
	    SSLLogError(conn->server,APLOG_CRIT,"unable to set certificate");
	    return FALSE;
	    }
	if(SSL_use_RSAPrivateKey(con,pConfig->prsaKey) <= 0)
	    {
	    SSLLogError(conn->server,APLOG_CRIT,"unable to set private key");
	    return FALSE;
	    }
	}
    return TRUE;
    }

static void ApacheSSLSetupVerify(conn_rec *conn)
    {
#if SSLEAY_VERSION_NUMBER < 0x0800
    SSLConfigRec *pConfig=ap_get_module_config(conn->server->module_config,
					       &apache_ssl_module);
    int nVerify=VerifyFlags(pConfig);
#endif

    conn->client->szClientX509=NULL;

    conn->client->nVerifyError=X509_V_OK;

#if SSLEAY_VERSION_NUMBER < 0x0800
/* Why call this twice?? Ben */  
    SSL_set_verify(conn->client->ssl,nVerify,ApacheSSLVerifyCallback);
#endif
    if(!ApacheSSLSetCertStuff(conn))
	{
	ap_log_error(APLOG_MARK,APLOG_ERR|APLOG_NOERRNO,conn->server,
		     "ApacheSSLSetCertStuff failed");
	exit(1);
	}
#if SSLEAY_VERSION_NUMBER < 0x0800
    SSL_set_verify(conn->client->ssl,nVerify,ApacheSSLVerifyCallback);
#endif
    }

#if FREE_SESSION
static void CloseSSL(void *_conn)
    {
    conn_rec *conn=_conn;
    SSL *ssl=conn->client->ssl;

    SSL_free(ssl);
    /*    ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,conn->server,
		 "Closed");*/
    }
#endif

int ApacheSSLSetupConnection(conn_rec *conn)
    {
    server_rec *srvr=conn->server;
    BUFF *fb=conn->client;
    SSLConfigRec *pConfig=ap_get_module_config(srvr->module_config,
					       &apache_ssl_module);
    int r;

    if(pConfig->bDisabled)
	{
	fb->ssl=NULL;
	return TRUE;
	}

    ap_conn_timeout("SSL accept timeout",conn);

    ReadRandomFiles(srvr,(RandomFile *)pConfig->aRandomFiles->elts,
		    pConfig->aRandomFiles->nelts);

#if APACHE_SSL_KEEP_CERTS
    fb->tableCertChain=NULL;
#endif
#if SSLEAY_VERSION_NUMBER >= 0x0920
    /* Use our current configuration as the session ID context. Do this by
       actually using its address, which should be invariant */
    fb->ssl=SSL_new(pConfig->pSSLCtx);
    r=SSL_set_session_id_context(fb->ssl,(unsigned char *)&pConfig,
				 sizeof pConfig);
    ap_assert(r);
#else
    fb->ssl=SSL_new(pConfig->pSSLCtx);
#endif
    SSL_set_fd(fb->ssl,fb->fd);
#if FREE_SESSION
    ap_register_cleanup(conn->pool,conn,CloseSSL,CloseSSL);
#endif
#if DEBUG_SSL_STATE
    SSL_set_info_callback(fb->ssl,InfoCallback);
    SSL_set_ex_data(fb->ssl,EX_DATA_SERVER_REC,srvr);
#endif
    SSL_set_ex_data(fb->ssl,EX_DATA_CONN_REC,conn);

    ApacheSSLSetupVerify(conn);

    while(!SSL_is_init_finished(fb->ssl))
	{
	int ret=SSL_accept(fb->ssl);
	if(ret == 0)
	    {
	    /* This appears to happen when a connection is closed without any
	       data being sent. Since Netscape does this all the time, don't
	       log as an error.
	       */
	    SSLLogError(srvr,APLOG_DEBUG,"SSL_accept returned 0");
	    fb->flags |= B_EOF | B_EOUT;
	    return FALSE;
	    }
	if (ret < 0)
	    {
	    SSLLogError(srvr,APLOG_ERR,"SSL_accept failed");
	    fb->flags |= B_EOF | B_EOUT;
	    return FALSE;
	    }

	if(conn->client->nVerifyError != X509_V_OK)
	    {
	    SSLLogError(srvr,APLOG_ERR,"Verification failed");
	    fb->flags |= B_EOF | B_EOUT;
	    return FALSE;
	    }

#if !RENEG
	if(pConfig->nVerifyClient != VERIFY_NONE)
	    {
	    char *s;
	    X509 *xs;
	    xs=SSL_get_peer_certificate(fb->ssl);
	    if(xs)
		{
		s=X509_NAME_oneline(X509_get_subject_name(xs),NULL,0);
		conn->client->szClientX509=ap_pstrdup(conn->pool,s);
		free(s);
		}
	    }

	if(pConfig->nVerifyClient == VERIFY_REQUIRE
	   && !conn->client->szClientX509)
	    {
	    SSLLogError(conn->server,APLOG_ERR,"No client certificate");
	    return 0;
	    }
#endif
	ap_log_error(APLOG_MARK,APLOG_DEBUG|APLOG_NOERRNO,srvr,"CIPHER is %s",
		     SSL_get_cipher(conn->client->ssl));
	}

#if !RENEG
    /* This should be safe.... so I'll use it */
    SSL_set_read_ahead(fb->ssl,1);
#endif

    return TRUE;
    }

static const char *SSLLogFile(cmd_parms *cmd,char *struct_ptr,char *arg)
    {
    return "The SSLLogFile directive is no longer used. All logging information now goes to the error log.";
    }

#if RENEG
static const char *SSLVerifyClient(cmd_parms *cmd,SSLDirConfigRec *rec,
				   char *level)
    {
    rec->nVerifyClient=atoi(level);
    if(rec->nVerifyClient < 0 || rec->nVerifyClient > 3)
	return "Bad value for SSLVerifyClient";
    return NULL;
    }
#endif

static const char *SSLRequireCipher(cmd_parms *cmd,SSLDirConfigRec *rec,
				    char *cipher)
    {
    ap_table_set(rec->tbRequiredCiphers,cipher,"Required");
    return NULL;
    }

static const char *SSLBanCipher(cmd_parms *cmd,SSLDirConfigRec *rec,
				char *cipher)
    {
    ap_table_set(rec->tbBannedCiphers,cipher,"Banned");
    return NULL;
    }

static const char *SSLRequireSSL(cmd_parms *cmd,SSLDirConfigRec *rec,
				 char *cipher)
    {
    rec->nSSLRequired=SSLREQ_REQUIRED;
    return NULL;
    }

static const char *SSLDenySSL(cmd_parms *cmd,SSLDirConfigRec *rec,
				 char *cipher)
    {
    rec->nSSLRequired=SSLREQ_DENIED;
    return NULL;
    }

#if APACHE_SSL_EXPORT_CERTS
static const char *SSLExportCerts(cmd_parms *cmd,SSLDirConfigRec *rec,
				  char *cipher)
    {
    rec->bExportCerts=TRUE;
    return NULL;
    }
#endif

#if CACHE_SESSIONS
static const char *SSLCacheServerPort(cmd_parms *cmd,char *struct_ptr,
				      char *arg)
    {
    if(strspn(arg,"0123456789") == strlen(arg))
	s_szCacheServerPort=arg;
    else
	s_szCacheServerPort=ap_server_root_relative(cmd->pool,arg);
    InitGlobalCache(s_szCacheServerPort);

    return NULL;
    }

static const char *SSLCacheServerPath(cmd_parms *cmd,char *struct_ptr,
				      char *arg)
    {
    s_szCacheServerPath=ap_server_root_relative(cmd->pool,arg);

    return NULL;
    }

static const char *SSLCacheServerDir(cmd_parms *cmd,char *struct_ptr,
				     char *arg)
    {
    s_szCacheServerDir=ap_server_root_relative(cmd->pool,arg);

    return NULL;
    }
#endif

static const char *SSLRandomFile(cmd_parms *cmd,char *struct_ptr,char *arg1,
				 char *arg2)
    {
    RandomFile rf;

    rf.szFile=arg1;
    rf.nBytes=atoi(arg2);
    ReadRandomFiles(cmd->server,&rf,1);

    return NULL;
    }

static const char *SSLRandomFilePerConnection(cmd_parms *cmd,char *struct_ptr,
					      char *arg1,char *arg2)
    {
    SSLConfigRec *pConfig=ap_get_module_config(cmd->server->module_config,
					       &apache_ssl_module);
    RandomFile *prf=ap_push_array(pConfig->aRandomFiles);

    prf->szFile=arg1;
    prf->nBytes=atoi(arg2);

    return NULL;
    }

static const char *set_server_string_slot (cmd_parms *cmd,char *struct_ptr,
					   char *arg)
    {
    char *pConfig=ap_get_module_config(cmd->server->module_config,
				       &apache_ssl_module);
  
    int offset=(int)(long)cmd->info; 
    *(char **)(pConfig+offset)=arg;
    return NULL;
    }

#if RENEG
static const char *set_dir_int_slot(cmd_parms *cmd,char *pStruct,char *szValue)
    {
    int offset=(int)(long)cmd->info;
    *(int *)(pStruct+offset)=atoi(szValue);
    return NULL;
    }
#endif

static const char *set_server_int_slot (cmd_parms *cmd,char *struct_ptr,
					char *arg)
    {
    char *pConfig=ap_get_module_config(cmd->server->module_config,
				       &apache_ssl_module);
  
    int offset=(int)(long)cmd->info; 
    *(int *)(pConfig+offset)=atoi(arg);
    return NULL;
    }

static const char *set_server_bool_slot (cmd_parms *cmd,char *struct_ptr)
    {
    char *pConfig=ap_get_module_config(cmd->server->module_config,
				       &apache_ssl_module);
  
    int offset=(int)(long)cmd->info; 
    *(BOOL *)(pConfig+offset)=TRUE;
    return NULL;
    }

static const char *set_server_bool_slot_false (cmd_parms *cmd,char *struct_ptr)
    {
    char *pConfig=ap_get_module_config(cmd->server->module_config,
				       &apache_ssl_module);
  
    int offset=(int)(long)cmd->info; 
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
    { "SSLNoCAList",set_server_bool_slot,
      (void *)XtOffsetOf(SSLConfigRec,bNoCAList),RSRC_CONF,NO_ARGS,
      "Don't send the CA list to the client" },
#if RENEG
    { "SSLVerifyDepth",set_dir_int_slot,
      (void *)XtOffsetOf(SSLDirConfigRec,nVerifyDepth),OR_FILEINFO,TAKE1,
      "Verify depth (default 0)" },
    { "SSLVerifyClient",SSLVerifyClient,NULL,OR_FILEINFO,TAKE1,
      "Verify client (0=no,1=optional,2=required,3=required (CA optional)" },
#else
    { "SSLVerifyDepth",set_server_int_slot,
      (void *)XtOffsetOf(SSLConfigRec,nVerifyDepth),RSRC_CONF,TAKE1,
      "Verify depth (default 0)" },
    { "SSLVerifyClient",set_server_int_slot,
      (void *)XtOffsetOf(SSLConfigRec,nVerifyClient),RSRC_CONF,TAKE1,
      "Verify client (0=no,1=optional,2=required" },
#endif
    { "SSLFakeBasicAuth",set_server_bool_slot,
      (void *)XtOffsetOf(SSLConfigRec,bFakeBasicAuth),RSRC_CONF,NO_ARGS,
      "Translate client X509 into a user name" },
    { "SSLRequiredCiphers",set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szReqCiphers),RSRC_CONF,TAKE1,
      "Colon-delimited list of required ciphers" },
    /* Per Directory */
    { "SSLRequireCipher",SSLRequireCipher,NULL,OR_FILEINFO,ITERATE,
      "Add a cipher to the per directory list of required ciphers" },
    { "SSLBanCipher",SSLBanCipher,NULL,OR_FILEINFO,ITERATE,
      "Add a cipher to the per directory list of banned ciphers" },
    { "SSLRequireSSL",SSLRequireSSL,NULL,OR_FILEINFO,NO_ARGS,
      "Require SSL (use to protect directories that should not be served unencrypted)" },
    { "SSLDenySSL",SSLDenySSL,NULL,OR_FILEINFO,NO_ARGS,
      "Deny SSL (use to constrain directories that should be served unencrypted)" },
#if APACHE_SSL_EXPORT_CERTS
    { "SSLExportClientCertificates",SSLExportCerts,NULL,OR_FILEINFO,NO_ARGS,
      "Export client certificates and their certificate chain" },
#endif
#if CACHE_SESSIONS
    { "SSLCacheServerPort",SSLCacheServerPort,NULL,RSRC_CONF,TAKE1,
      "Cache server TCP port number" },
    { "SSLSessionCacheTimeout",set_server_int_slot,
      (void *)XtOffsetOf(SSLConfigRec,tSessionCacheTimeout),RSRC_CONF,TAKE1,
      "Lifetime in seconds of session cache objects" },
    { "SSLCacheServerPath",SSLCacheServerPath,NULL,RSRC_CONF,TAKE1,
      "Path of the global cache server" },
    { "SSLCacheServerRunDir",SSLCacheServerDir,NULL,RSRC_CONF,TAKE1,
      "Path for the server to run in (in case of coredumps)" },
#endif
    { "SSLRandomFile",SSLRandomFile,NULL,RSRC_CONF,TAKE2,
      "A file to read randomness from and a maximum number of bytes to read" },
    { "SSLRandomFilePerConnection", SSLRandomFilePerConnection,NULL,RSRC_CONF,
      TAKE2,
      "A file to read randomness from and a maximim number of bytes to read on each request" },
    { "SSLLogFile",SSLLogFile,NULL,RSRC_CONF,TAKE1,
      "Defunct SSL log file - DO NOT USE" },
    { NULL }
    };

module MODULE_VAR_EXPORT apache_ssl_module =
    {
    STANDARD_MODULE_STUFF,
    InitSSLServer,		/* initializer */
    CreateSSLDirConfig,		/* dir config creater */
    MergeSSLDirConfig,		/* dir merger --- default is to override */
    CreateSSLServerConfig,	/* server config */
    MergeSSLServerConfig,	/* merge server config */
    ssl_cmds,			/* command table */
    NULL,			/* handlers */
    NULL,			/* filename translation */
    FakeBasicAuth,		/* check_user_id */
    NULL,			/* check auth */
    SSLCheckCipher,		/* check access */
    NULL,			/* type_checker */
    /*    SSLFixups,			fixups */
    NULL,			/* fixups */
    NULL,			/* logger */
    };

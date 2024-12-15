
/* ====================================================================
 * Copyright (c) 1995 Ben Laurie.  All rights reserved.
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

#include <err.h>
#include <pem.h>
#include <rsa.h>
#include <x509.h>

#include "httpd.h"
#include "http_config.h"

/*
Apache SSL Support
*/

/*

This version only uses the default/environment settings for CA certificates,
as SSLeay doesn't (currently) have a method to reset them. This means that
a single instance of Apache can only have one set of CA certificates. This
limitation cannot easily be overcome, as there would be no way to synchronise
accept()s with the correct run of the server if multiple servers are run,
unless they use different ports, or Apache is modded to run individual accepts
for each virtual host.

*/

typedef enum
    {
    VERIFY_NONE=0, /* At present, NONE and OPTIONAL are the same! */
    VERIFY_OPTIONAL=1,
    VERIFY_REQUIRE=2,
    } VerifyType;

typedef struct
    {
    char *szCertificateFile;
    char *szKeyFile;
    char *szLogFile;
    FILE *fileLogFile;
    int nVerifyDepth;
    VerifyType nVerifyClient;

    X509 *px509Certificate;
    RSA *prsaKey;

    SSL_CTX *ctx;

    BOOL bFakeBasicAuth;
    } SSLConfigRec;

extern module ssl_module;

static void SSLLogError(server_rec *s)
    {
    unsigned long l;
    char str[HUGE_STRING_LEN];

    /* Print out error messages */

    while(l=ERR_get_error())
      {
	ERR_error_string(l, str);
	log_error(str,s);
      }
    }

int SSLLogger(request_rec *orig)
{
  SSLConfigRec *pConfig= get_module_config(orig->server->module_config,
				       &ssl_module);
  conn_rec *conn = orig->connection;
  char str[HUGE_STRING_LEN];

  sprintf(str, "%s", SSL_get_cipher(conn->client.ssl));
  if(conn->client.nVerifyError)
    sprintf(str, "%s %d %s", str, conn->client.nVerifyError,
	    X509_cert_verify_error_string(conn->client.nVerifyError));
  else
    strcat(str, " - -");

  if(conn->client.szClientX509)
    sprintf(str, "%s \"%s\"",str,conn->client.szClientX509);
  else
    strcat(str, " -");

  strcat(str, "\n");
  fputs(str, pConfig->fileLogFile);
  fflush(pConfig->fileLogFile);
  return OK;
}

static void *CreateSSLServerConfig(pool *p,server_rec *s)
    {
    SSLConfigRec *rec=pcalloc(p,sizeof(SSLConfigRec));
    
    rec->szCertificateFile=rec->szKeyFile=rec->szLogFile=NULL;
    rec->nVerifyDepth=0;
    rec->nVerifyClient=VERIFY_NONE;
    rec->px509Certificate=NULL;
    rec->prsaKey=NULL;
    rec->bFakeBasicAuth=FALSE;
    rec->ctx=NULL;
    return rec;
    }

static void InitSSL()
    {
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    SSL_debug("/tmp/ssldebug");
    }

static void GetCertificateAndKey(server_rec *s,SSLConfigRec *pConfig)
    {
    char buf[1024],prompt[1024];
    int n;
    FILE *f;
    char szPath[MAX_STRING_LEN];

    if(pConfig->px509Certificate)
	{
	fprintf(stderr,"Attempt to reinitialise SSL for server %s\n",s->server_hostname);
	return;
	}

    fprintf(stderr,"Reading certificate and key for server %s\n",s->server_hostname);
  
    f=fopen(pConfig->szCertificateFile,"r");
    if(!f)
	{
	sprintf(szPath,"%s/%s",X509_get_default_cert_dir(),pConfig->szCertificateFile);
	f=fopen(szPath,"r");
	if(!f)
	    {
	    fprintf(stderr,"Can't open certificate file %s, nor %s\n",pConfig->szCertificateFile,szPath);
	    exit(1);
	    }
	}
    else
	strcpy(szPath,pConfig->szCertificateFile);  /* in case it also contains the key */

    if(!(pConfig->px509Certificate = PEM_read_X509(f,NULL,NULL)))
      {
	fprintf(stderr,"Error reading server certificate file %s: ",szPath);
	ERR_print_errors(stderr);
	exit(1);
	}
    fclose(f);

    if(pConfig->szKeyFile)
	sprintf(szPath,"%s/private/%s",X509_get_default_cert_area(),pConfig->szKeyFile);
/* Otherwise the path already contains the name of the certificate file */
    f=fopen(szPath,"r");
    if(!f)
	{
	fprintf(stderr,"Can't open key file ");
	perror(szPath);
	exit(1);
	}
    
    if(!(pConfig->prsaKey = PEM_read_RSAPrivateKey(f,NULL,NULL)))
 	{
	fprintf(stderr,"Error reading private key file %s: ",szPath);
	ERR_print_errors(stderr);
	exit(1);
	}
    }

static void InitSSLServer(server_rec *s,pool *p)
    {
    InitSSL();
    for ( ; s ; s = s->next)
	{
	SSLConfigRec *pConfig=get_module_config(s->module_config,&ssl_module);

	if(!pConfig->szCertificateFile)
	    {
	    fprintf(stderr,"No SSL Certificate set for server %s\n",s->server_hostname);
	    exit(1);
	    }
	if(pConfig->nVerifyClient < 0 || pConfig->nVerifyClient > VERIFY_REQUIRE)
	    {
	    fprintf(stderr,"Bad value for SSLVerifyClient (%d)\n",pConfig->nVerifyClient);
	    exit(1);
	    }
	if(!pConfig->szLogFile)
	    {
	    fprintf(stderr,"Required SSLLogFile missing\n");
	    exit(1);
	    }
	
	pConfig->fileLogFile=fopen(pConfig->szLogFile,"a");
	if(!pConfig->fileLogFile)
	    {
	    perror(pConfig->szLogFile);
	    exit(1);
	    }

	/* Setup CTX for the first tiem*/
	if(pConfig->ctx == NULL)
	  {
	    char *CAfile=NULL,*CApath=NULL;

	    pConfig->ctx = SSL_CTX_new();
	    if ((!X509_load_verify_locations(pConfig->ctx->cert, CAfile,CApath)) ||
		(!X509_set_default_verify_paths(pConfig->ctx->cert)))
	      {
		fprintf(stderr,"X509_load_verify_locations\n");
		ERR_print_errors(stderr);
		exit(1);
	      }
	  }

	GetCertificateAndKey(s,pConfig);
      }
}

static char *set_server_string_slot (cmd_parms *cmd, char *struct_ptr, char *arg)
    {
    /* This one should be pretty generic... */
    char *pConfig=get_module_config(cmd->server->module_config,&ssl_module);
  
    int offset=(int)cmd->info; 
    *(char **)(pConfig+offset)=arg;
    return NULL;
    }

static char *set_server_int_slot (cmd_parms *cmd, char *struct_ptr, char *arg)
    {
  /* This one should be pretty generic... */
    char *pConfig=get_module_config(cmd->server->module_config,&ssl_module);
  
    int offset=(int)cmd->info; 
    *(int *)(pConfig+offset) = atoi(arg);
    return NULL;
    }

static char *set_server_bool_slot (cmd_parms *cmd, char *struct_ptr)
    {
  /* This one should be pretty generic... */
    char *pConfig=get_module_config(cmd->server->module_config,&ssl_module);
  
    int offset=(int)cmd->info; 
    *(BOOL *)(pConfig+offset)=TRUE;
    return NULL;
    }

command_rec ssl_cmds[]=
{
  { "SSLCertificateFile", set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szCertificateFile),RSRC_CONF,TAKE1,
      "PEM certificate file" },
  { "SSLCertificateKeyFile",set_server_string_slot,
      (void *)XtOffsetOf(SSLConfigRec,szKeyFile),RSRC_CONF,TAKE1,
      "Certificate private key file (assumed to be SSLCertificateFile if absent)" },
  { "SSLVerifyDepth",set_server_int_slot,
      (void *)XtOffsetOf(SSLConfigRec,nVerifyDepth),RSRC_CONF,TAKE1,
      "Verify depth (default 0)" },
  { "SSLVerifyClient",set_server_int_slot,
      (void *)XtOffsetOf(SSLConfigRec,nVerifyClient),RSRC_CONF,TAKE1,
      "Verify client (0=no, 1=optional, 2=required" },
  { "SSLFakeBasicAuth",set_server_bool_slot,
      (void *)XtOffsetOf(SSLConfigRec,bFakeBasicAuth),RSRC_CONF,NO_ARGS,
      "Translate client X509 into a user name" },
  { "SSLLogFile",set_server_string_slot,
	(void *)XtOffsetOf(SSLConfigRec,szLogFile),RSRC_CONF,TAKE1,
	"Place to dump all SSL messages that have no better home" },
  { NULL },
};

static conn_rec *SSLVerifyConn;

static const char six2pr[64+1]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void uuencode(char *szTo,const char *szFrom)
    {
    const unsigned char *s;

    for(s=szFrom ; *s ; s+=3)
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

/* Do an SSL_new with the CTX stuff */
SSL *ApacheSSLNew(server_rec *s)
{
  SSLConfigRec *pConfig = get_module_config(s->module_config, &ssl_module);

  return(SSL_new(pConfig->ctx));
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
  
    if(!pConfig->bFakeBasicAuth)
	return DECLINED;
    if(r->connection->user)
	return DECLINED;
    if(!r->connection->client.szClientX509)
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
    sprintf(b1,"%s:password",r->connection->client.szClientX509);
    uuencode(b2,b1);
    sprintf(b1,"Basic %s",b2);
    table_set(r->headers_in,"Authorization",b1);
    return DECLINED;
    }

int ApacheSSLVerifyCallback(int ok,X509 *xs,X509 *xi,int depth,int error)
    {
    char *s;
    SSLConfigRec *pConfig=get_module_config(SSLVerifyConn->server->module_config,&ssl_module);
  
    s=(char *)X509_NAME_oneline(X509_get_subject_name(xs));
    if(s == NULL)
	{
	ERR_print_errors(pConfig->fileLogFile);
	return(0);
	}
    if(depth == 0)
	SSLVerifyConn->client.szClientX509=pstrdup(SSLVerifyConn->pool,s);

    fprintf(pConfig->fileLogFile,"depth=%d %s\n",depth,s);
    free(s);
    if(error == VERIFY_ERR_UNABLE_TO_GET_ISSUER)
	{
	s=(char *)X509_NAME_oneline(X509_get_issuer_name(xs));
	if(s == NULL)
	    {
	    fprintf(pConfig->fileLogFile,"verify error\n");
	    ERR_print_errors(pConfig->fileLogFile);
	    SSLLogError(SSLVerifyConn->server);
	    return(0);
	    }
	fprintf(pConfig->fileLogFile,"issuer= %s\n",s);
	free(s);
	}
    if(!ok)
	{
	fprintf(pConfig->fileLogFile,"verify error:num=%d:%s\n",error,
		X509_cert_verify_error_string(error));
	SSLLogError(SSLVerifyConn->server);
	SSLVerifyConn->client.szClientX509=NULL;
	}
    if(depth >= pConfig->nVerifyDepth)
	{
	fprintf(pConfig->fileLogFile,"Verify depth exceeded\n");
	log_error("Verify depth exceeded",SSLVerifyConn->server);
	ok=0;
	}
    fprintf(pConfig->fileLogFile,"verify return:%d\n",ok);
    return(ok);
    }

/*
static int ApacheSSLKeyCallback(char *buf,int num,int w)
{
  SSLConfigRec *pConfig=get_module_config(SSLVerifyConn->server->module_config,&ssl_module);

  assert(w == 0);
  assert(pConfig->szPassPhrase);

  strncpy(buf,pConfig->szPassPhrase,num-1);
  buf[num-1]='\0';
  return strlen(buf);
}
*/

static BOOL ApacheSSLSetCertStuff(conn_rec *conn)
    {
    SSLConfigRec *pConfig=get_module_config(conn->server->module_config,&ssl_module);
    SSL *con=conn->client.ssl;
    char *cert_file=pConfig->szCertificateFile;
    char *key_file=pConfig->szKeyFile;

/*  PEM_set_getkey_callback(ApacheSSLKeyCallback); */
    if(SSL_use_certificate(con,pConfig->px509Certificate) <= 0)
      {
	fprintf(pConfig->fileLogFile,"unable to set certificate\n");
	ERR_print_errors(pConfig->fileLogFile);
	SSLLogError(conn->server);
	return FALSE;
      }
    if(SSL_use_RSAPrivateKey(con,pConfig->prsaKey) <= 0)
      {
	fprintf(pConfig->fileLogFile,"unable to set private key\n");
	ERR_print_errors(pConfig->fileLogFile);
	SSLLogError(conn->server);
	return FALSE;
      }
    return TRUE;
  }

void ApacheSSLSetupVerify(conn_rec *conn)
    {
    SSLConfigRec *pConfig=get_module_config(conn->server->module_config,&ssl_module);
    int nVerify=0;

    conn->client.szClientX509=NULL;
    SSLVerifyConn=conn;

    switch(pConfig->nVerifyClient)
	{
    case VERIFY_REQUIRE:
	nVerify|=SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
	break;

    case VERIFY_OPTIONAL:
	nVerify|=SSL_VERIFY_PEER;
	break;
	}

    conn->client.nVerifyError=VERIFY_OK;

/* Why call this twice?? Ben */  
    SSL_set_verify(conn->client.ssl,nVerify,ApacheSSLVerifyCallback);
    if(!ApacheSSLSetCertStuff(conn))
	{
	fprintf(pConfig->fileLogFile,"ApacheSSLSetCertStuff failed\n");
	log_error("ApacheSSLSetCertStuff failed",conn->server);
	exit(1);
	}
    SSL_set_verify(conn->client.ssl,nVerify,ApacheSSLVerifyCallback);
    }

int os_conn_vprintf(conn_rec *conn,const char *fmt,va_list args)
    {
    static char buf[MAX_STRING_LEN];
    int ret,len;

    vsprintf(buf,fmt,args);
    len=strlen(buf);
    ret=SSL_write(conn->client.ssl,buf,len);
    if(ret < len)
	{
	log_error("SSL_write failed",conn->server);
	SSLLogError(conn->server);
	exit(1);
	}
    return ret;
    }

int _os_conn_printf(conn_rec *conn,const char *fmt,...)
    {
    va_list args;
    int ret;
    
    va_start(args,fmt);
    ret=os_conn_vprintf(conn,fmt,args);
    va_end(args);

    return ret;
    }

int os_conn_puts(char *buf,conn_rec *conn)
    {
    return SSL_write(conn->client.ssl,buf,strlen(buf));
    }

int os_conn_putc(int c,conn_rec *conn)
    {
    char buf[1];

    buf[0]=c;
    return SSL_write(conn->client.ssl,buf,1);
    }

int os_conn_read(void *buf,int unit,int len,conn_rec *conn)
    {
    int ret;
    X509 *peer;
    char *str;

/* Dodgy - but SSLeay knows no better */
    SSLVerifyConn=conn;

    while(!SSL_is_init_finished(conn->client.ssl))
	{
	SSLConfigRec *pConfig=get_module_config(conn->server->module_config,&ssl_module);
      
	ret=SSL_accept(conn->client.ssl);
	if(ret <= 0)
	    {
	    log_error("SSL_accept failed",conn->server);
	    SSLLogError(conn->server);
/* treat as EOF for now... */
	    return 0;
	    }
	if(conn->client.nVerifyError != VERIFY_OK)
	    {
	    log_error("Verification failed",conn->server);
	    SSLLogError(conn->server);
/* treat as EOF for now... */
	    return 0;
	    }
/*
This does not seem to work in a server!!
      peer=SSL_get_peer_certificate(conn->client.ssl);
      if (peer != NULL)
	{
	  fprintf("Client certificate\n");
	  PEM_write_X509(stdout,peer);
	  str=X509_NAME_oneline(X509_get_subject_name(peer));
	  fprintf(pConfig->fileLogFile,"subject=%s\n",str);
	  free(str);
	  str=X509_NAME_oneline(X509_get_issuer_name(peer));
	  fprintf(pConfig->fileLogFile,"issuer=%s\n",str);
	  free(str);
	  X509_free(peer);
	}
*/
	if(pConfig->nVerifyClient == VERIFY_REQUIRE && !conn->client.szClientX509)
	    {
	    log_error("No client certificate",conn->server);
	    SSLLogError(conn->server);
	    return 0;
	    }
	}

    ret=SSL_read(conn->client.ssl,buf,unit*len);

    SSLVerifyConn=NULL;

    if(ret < 0)
	{
	log_error("SSL_read failed",conn->server);
	SSLLogError(conn->server);
	}
    if(ret <= 0)
	return ret;
    return ret/unit;
    }

char *os_conn_gets(char *buf,int len,conn_rec *conn)
    {
    int n,ret;
    char locbuf[1];
    
    for(n=0 ; n < len-1 ; ++n)
	{
	ret=os_conn_read(locbuf,1,1,conn);
	if(ret == 0)
	    return NULL;
	if(ret < 0)
	    {
	    log_error("SSL_read failed",conn->server);
	    exit(1);
	    }
	if(locbuf[0] == '\n')
	    break;
	buf[n]=locbuf[0];
	}
    buf[n]='\0';
    return buf;
    }
  
module ssl_module = {
   STANDARD_MODULE_STUFF,
   InitSSLServer,		/* initializer */
   NULL,			/* dir config creater */
   NULL,			/* dir merger --- default is to override */
   CreateSSLServerConfig,	/* server config */
   NULL,			/* merge server config */
   ssl_cmds,			/* command table */
   NULL,			/* handlers */
   NULL,			/* filename translation */
   FakeBasicAuth,		/* check_user_id */
   NULL,			/* check auth */
   NULL,			/* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   SSLLogger,			/* logger */
};

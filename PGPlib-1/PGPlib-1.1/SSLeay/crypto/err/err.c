/* crypto/err/err.c */
/* Copyright (C) 1995-1997 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include "lhash.h"
#include "crypto.h"
#include "cryptlib.h"
#include "buffer.h"
#include "err.h"
#include "crypto.h"


static LHASH *error_hash=NULL;
static LHASH *thread_hash=NULL;

#ifndef NOPROTO
static unsigned long err_hash(ERR_STRING_DATA *a);
static int err_cmp(ERR_STRING_DATA *a, ERR_STRING_DATA *b);
static unsigned long pid_hash(ERR_STATE *pid);
static int pid_cmp(ERR_STATE *a,ERR_STATE *pid);
#else
static unsigned long err_hash();
static int err_cmp();
static unsigned long pid_hash();
static int pid_cmp();
#endif

#ifndef NO_ERR
static ERR_STRING_DATA ERR_str_libraries[]=
	{
{ERR_PACK(ERR_LIB_NONE,0,0)		,"unknown library"},
{ERR_PACK(ERR_LIB_SYS,0,0)		,"system library"},
{ERR_PACK(ERR_LIB_BN,0,0)		,"bignum routines"},
{ERR_PACK(ERR_LIB_RSA,0,0)		,"rsa routines"},
{ERR_PACK(ERR_LIB_DH,0,0)		,"Diffie-Hellman routines"},
{ERR_PACK(ERR_LIB_EVP,0,0)		,"digital envelope routines"},
{ERR_PACK(ERR_LIB_BUF,0,0)		,"memory buffer routines"},
{ERR_PACK(ERR_LIB_BIO,0,0)		,"BIO routines"},
{ERR_PACK(ERR_LIB_OBJ,0,0)		,"object identifier routines"},
{ERR_PACK(ERR_LIB_PEM,0,0)		,"PEM routines"},
{ERR_PACK(ERR_LIB_ASN1,0,0)		,"asn1 encoding routines"},
{ERR_PACK(ERR_LIB_X509,0,0)		,"x509 certificate routines"},
{ERR_PACK(ERR_LIB_CONF,0,0)		,"configuation file routines"},
{ERR_PACK(ERR_LIB_METH,0,0)		,"X509 lookup 'method' routines"},
{ERR_PACK(ERR_LIB_SSL,0,0)		,"SSL routines"},
{ERR_PACK(ERR_LIB_RSAREF,0,0)		,"RSAref routines"},
{ERR_PACK(ERR_LIB_PROXY,0,0)		,"Proxy routines"},
{ERR_PACK(ERR_LIB_BIO,0,0)		,"BIO routines"},
{ERR_PACK(ERR_LIB_PKCS7,0,0)		,"PKCS7 routines"},
{0,NULL},
	};

static ERR_STRING_DATA ERR_str_functs[]=
	{
	{ERR_PACK(0,SYS_F_FOPEN,0),     	"fopen"},
	{ERR_PACK(0,SYS_F_CONNECT,0),		"connect"},
	{ERR_PACK(0,SYS_F_GETSERVBYNAME,0),	"getservbyname"},
	{ERR_PACK(0,SYS_F_SOCKET,0),		"socket"}, 
	{ERR_PACK(0,SYS_F_IOCTLSOCKET,0),	"ioctlsocket"},
	{ERR_PACK(0,SYS_F_BIND,0),		"bind"},
	{ERR_PACK(0,SYS_F_LISTEN,0),		"listen"},
	{ERR_PACK(0,SYS_F_ACCEPT,0),		"accept"},
#ifdef WINDOWS
	{ERR_PACK(0,SYS_F_WSASTARTUP,0),	"WSAstartup"},
#endif
	{0,NULL},
	};

static ERR_STRING_DATA ERR_str_reasons[]=
	{
{ERR_R_FATAL                             ,"fatal"},
{ERR_R_SYS_LIB				,"system lib"},
{ERR_R_BN_LIB				,"BN lib"},
{ERR_R_RSA_LIB				,"RSA lib"},
{ERR_R_DH_LIB				,"DH lib"},
{ERR_R_EVP_LIB				,"EVP lib"},
{ERR_R_BUF_LIB				,"BUF lib"},
{ERR_R_BIO_LIB				,"BIO lib"},
{ERR_R_OBJ_LIB				,"OBJ lib"},
{ERR_R_PEM_LIB				,"PEM lib"},
{ERR_R_X509_LIB				,"X509 lib"},
{ERR_R_METH_LIB				,"METH lib"},
{ERR_R_ASN1_LIB				,"ASN1 lib"},
{ERR_R_CONF_LIB				,"CONF lib"},
{ERR_R_SSL_LIB				,"SSL lib"},
{ERR_R_PROXY_LIB			,"PROXY lib"},
{ERR_R_BIO_LIB				,"BIO lib"},
{ERR_R_PKCS7_LIB			,"PKCS7 lib"},
{ERR_R_MALLOC_FAILURE			,"Malloc failure"},
{ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED	,"called a fuction you should not call"},
{0,NULL},
	};
#endif

void ERR_load_ERR_strings()
	{
	static int init=1;

	if (init)
		{
		CRYPTO_w_lock(CRYPTO_LOCK_ERR);
		if (init == 0)
			{
			CRYPTO_w_unlock(CRYPTO_LOCK_ERR);
			return;
			}
		init=0;
		CRYPTO_w_unlock(CRYPTO_LOCK_ERR);

#ifndef NO_ERR
		ERR_load_strings(0,ERR_str_libraries);
		ERR_load_strings(0,ERR_str_reasons);
		ERR_load_strings(ERR_LIB_SYS,ERR_str_functs);
#endif
		}
	}

void ERR_load_strings(lib,str)
int lib;
ERR_STRING_DATA *str;
	{
	if (error_hash == NULL)
		{
		CRYPTO_w_lock(CRYPTO_LOCK_ERR_HASH);
		error_hash=lh_new(err_hash,err_cmp);
		if (error_hash == NULL)
			{
			CRYPTO_w_unlock(CRYPTO_LOCK_ERR_HASH);
			return;
			}
		CRYPTO_w_unlock(CRYPTO_LOCK_ERR_HASH);

		ERR_load_ERR_strings();
		}

	CRYPTO_w_lock(CRYPTO_LOCK_ERR_HASH);
	while (str->error)
		{
		str->error|=ERR_PACK(lib,0,0);
		lh_insert(error_hash,(char *)str);
		str++;
		}
	CRYPTO_w_unlock(CRYPTO_LOCK_ERR_HASH);
	}

void ERR_free_strings()
	{
	CRYPTO_w_lock(CRYPTO_LOCK_ERR);

	if (error_hash != NULL)
		{
		lh_free(error_hash);
		error_hash=NULL;
		}

	CRYPTO_w_unlock(CRYPTO_LOCK_ERR);
	}

void ERR_put_error(lib,func,reason,file,line)
int lib,func,reason;
char *file;
int line;
	{
	ERR_STATE *es;

	es=ERR_get_state();

	es->top=(es->top+1)%ERR_NUM_ERRORS;
	if (es->top == es->bottom)
		es->bottom=(es->bottom+1)%ERR_NUM_ERRORS;
	es->err_buffer[es->top]=ERR_PACK(lib,func,reason);
	es->err_file[es->top]=file;
	es->err_line[es->top]=line;
	}

void ERR_clear_error()
	{
	int i;
	ERR_STATE *es;

	es=ERR_get_state();

	for (i=0; i<ERR_NUM_ERRORS; i++)
		{
		es->err_buffer[i]=0;
		es->err_file[i]=NULL;
		es->err_line[i]= -1;
		}
	es->top=es->bottom=0;
	}

unsigned long ERR_peek_error()
	{	
	int i;
	ERR_STATE *es;

	/* should be fine since only one thread should ever be playing
	 * with the value returned from this call */
	es=ERR_get_state();

	if (es->bottom == es->top) return(0);
	i=(es->bottom+1)%ERR_NUM_ERRORS;


	return(es->err_buffer[i]);
	}

unsigned long ERR_get_error()
	{
	int i;
	unsigned long ret;
	ERR_STATE *es;

	es=ERR_get_state();

	if (es->bottom == es->top) return(0);
	i=(es->bottom+1)%ERR_NUM_ERRORS;
	es->bottom=i;
	ret=es->err_buffer[i];
	es->err_buffer[i]=0;


	return(ret);
	}

unsigned long ERR_peek_error_line(file,line)
char **file;
int *line;
	{	
	int i=0;
	ERR_STATE *es;

	es=ERR_get_state();

	if (es->bottom == es->top)
		{
		return(0);
		}

	i=(es->bottom+1)%ERR_NUM_ERRORS;
	if (es->err_file[i] == NULL)
		{
		*file="NA";
		*line=0;
		}
	else
		{
		*file=es->err_file[i];
		*line=es->err_line[i];
		}

	return(es->err_buffer[i]);
	}

unsigned long ERR_get_error_line(file,line)
char **file;
int *line;
	{
	int i;
	unsigned long ret;
	ERR_STATE *es;

	es=ERR_get_state();

	if (es->bottom == es->top)
		return(0);

	i=(es->bottom+1)%ERR_NUM_ERRORS;
	es->bottom=i;
	ret=  es->err_buffer[i];
	if (es->err_file[i] == NULL)
		{
		*file="NA";
		*line=0;
		}
	else
		{
		*file=es->err_file[i];
		*line=es->err_line[i];
		}
	es->err_buffer[i]=0;
	es->err_file[i]=NULL;
	es->err_line[i]= -1;

	return(ret);
	}

/* BAD for multi-threaded, uses a local buffer if ret == NULL */
char *ERR_error_string(e,ret)
unsigned long e;
char *ret;
	{
	static char buf[256];
	char *ls,*fs,*rs;
	unsigned long l,f,r;
	int i;

	l=ERR_GET_LIB(e);
	f=ERR_GET_FUNC(e);
	r=ERR_GET_REASON(e);

	ls=ERR_lib_error_string(e);
	fs=ERR_func_error_string(e);
	rs=ERR_reason_error_string(e);

	if (ret == NULL) ret=buf;

	sprintf(&(ret[0]),"error:%08lX:",e);
	i=strlen(ret);
	if (ls == NULL)
		sprintf(&(ret[i]),":lib(%lu) ",l);
	else	sprintf(&(ret[i]),"%s",ls);
	i=strlen(ret);
	if (fs == NULL)
		sprintf(&(ret[i]),":func(%lu) ",f);
	else	sprintf(&(ret[i]),":%s",fs);
	i=strlen(ret);
	if (rs == NULL)
		sprintf(&(ret[i]),":reason(%lu)",r);
	else	sprintf(&(ret[i]),":%s",rs);

	return(ret);
	}

LHASH *ERR_get_string_table()
	{
	return(error_hash);
	}

LHASH *ERR_get_err_state_table()
	{
	return(thread_hash);
	}

char *ERR_lib_error_string(e)
unsigned long e;
	{
	ERR_STRING_DATA d,*p=NULL;
	unsigned long l;

	l=ERR_GET_LIB(e);

	CRYPTO_r_lock(CRYPTO_LOCK_ERR_HASH);

	if (error_hash != NULL)
		{
		d.error=ERR_PACK(l,0,0);
		p=(ERR_STRING_DATA *)lh_retrieve(error_hash,(char *)&d);
		}

	CRYPTO_r_unlock(CRYPTO_LOCK_ERR_HASH);

	return((p == NULL)?NULL:p->string);
	}

char *ERR_func_error_string(e)
unsigned long e;
	{
	ERR_STRING_DATA d,*p=NULL;
	unsigned long l,f;

	l=ERR_GET_LIB(e);
	f=ERR_GET_FUNC(e);

	CRYPTO_r_lock(CRYPTO_LOCK_ERR_HASH);

	if (error_hash != NULL)
		{
		d.error=ERR_PACK(l,f,0);
		p=(ERR_STRING_DATA *)lh_retrieve(error_hash,(char *)&d);
		}

	CRYPTO_r_unlock(CRYPTO_LOCK_ERR_HASH);

	return((p == NULL)?NULL:p->string);
	}

char *ERR_reason_error_string(e)
unsigned long e;
	{
	ERR_STRING_DATA d,*p=NULL;
	unsigned long l,r;

	l=ERR_GET_LIB(e);
	r=ERR_GET_REASON(e);

	CRYPTO_r_lock(CRYPTO_LOCK_ERR_HASH);

	if (error_hash != NULL)
		{
		d.error=ERR_PACK(l,0,r);
		p=(ERR_STRING_DATA *)lh_retrieve(error_hash,(char *)&d);
		if (p == NULL)
			{
			d.error=ERR_PACK(0,0,r);
			p=(ERR_STRING_DATA *)lh_retrieve(error_hash,
				(char *)&d);
			}
		}

	CRYPTO_r_unlock(CRYPTO_LOCK_ERR_HASH);

	return((p == NULL)?NULL:p->string);
	}

static unsigned long err_hash(a)
ERR_STRING_DATA *a;
	{
	unsigned long ret,l;

	l=a->error;
	ret=l^ERR_GET_LIB(l)^ERR_GET_FUNC(l);
	return(ret^ret%19*13);
	}

static int err_cmp(a,b)
ERR_STRING_DATA *a,*b;
	{
	return((int)(a->error-b->error));
	}

static unsigned long pid_hash(a)
ERR_STATE *a;
	{
	return(a->pid*13);
	}

static int pid_cmp(a,b)
ERR_STATE *a,*b;
	{
	return((int)((long)a->pid - (long)b->pid));
	}

void ERR_remove_state(pid)
unsigned long pid;
	{
	ERR_STATE *p,tmp;

	if (thread_hash == NULL)
		return;
	if (pid == 0)
		pid=(unsigned long)CRYPTO_thread_id();
	tmp.pid=pid;
	CRYPTO_w_lock(CRYPTO_LOCK_ERR);
	p=(ERR_STATE *)lh_delete(thread_hash,(char *)&tmp);
	if (p != NULL) Free(p);
	CRYPTO_w_unlock(CRYPTO_LOCK_ERR);
	}

ERR_STATE *ERR_get_state()
	{
	static ERR_STATE fallback;
	ERR_STATE *ret=NULL,tmp,*tmpp;
	unsigned long pid;

	pid=(unsigned long)CRYPTO_thread_id();

	CRYPTO_r_lock(CRYPTO_LOCK_ERR);
	if (thread_hash == NULL)
		{
		CRYPTO_r_unlock(CRYPTO_LOCK_ERR);
		CRYPTO_w_lock(CRYPTO_LOCK_ERR);
		if (thread_hash == NULL)
			{
			thread_hash=lh_new(pid_hash,pid_cmp);
			CRYPTO_w_unlock(CRYPTO_LOCK_ERR);
			if (thread_hash == NULL) return(&fallback);
			}
		else
			CRYPTO_w_unlock(CRYPTO_LOCK_ERR);
		}
	else
		{
		tmp.pid=pid;
		ret=(ERR_STATE *)lh_retrieve(thread_hash,(char *)&tmp);
		CRYPTO_r_unlock(CRYPTO_LOCK_ERR);
		}

	/* ret == the error state, if NULL, make a new one */
	if (ret == NULL)
		{
		ret=(ERR_STATE *)Malloc(sizeof(ERR_STATE));
		if (ret == NULL) return(&fallback);
		ret->pid=pid;
		ret->top=0;
		ret->bottom=0;
		CRYPTO_w_lock(CRYPTO_LOCK_ERR);
		tmpp=(ERR_STATE *)lh_insert(thread_hash,(char *)ret);
		CRYPTO_w_unlock(CRYPTO_LOCK_ERR);
		if (tmpp != NULL) /* old entry - should not happen */
			Free(tmpp);
		}
	return(ret);
	}


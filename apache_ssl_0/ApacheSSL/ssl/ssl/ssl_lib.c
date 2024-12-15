/* lib/ssl/ssl_lib.c */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
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
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
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
#include "ssl_locl.h"
#include "ssl_des.h"
#include "ssl_md5.h"
#include "ssl_sha.h"
#include "ssl_rc4.h"
#include "ssl_idea.h"
#include "ssl_rsa.h"
#include "ssl_null.h"

char *SSL_version="\0SSLeay v 0.5.1 21/12/95";

/* THIS ARRAY MUST BE KEPT ORDERED BY c1, c2 and c3.
 * basically the second last 'value' which is a #define for these 3
 * numbers */
CIPHER ssl_ciphers[NUM_CIPHERS]={
/* NULL_WITH_MD5 v3 */
	{	1,SSL_TXT_NULL_WITH_MD5,
	ssl_enc_null_init,ssl_enc_null,ssl_compute_md5_mac,
	8,MD5_MAC_SIZE,1,0,SSL_CK_NULL_WITH_MD5,0},

#ifndef NORC4
/* RC4_128_WITH_MD5 */
	{	1,SSL_TXT_RC4_128_WITH_MD5,
	ssl_enc_rc4_init,ssl_enc_rc4,ssl_compute_md5_mac,
	16,MD5_MAC_SIZE,1,0,SSL_CK_RC4_128_WITH_MD5,0},

/* RC4_128_EXPORT40_WITH_MD5 */
	{	1,SSL_TXT_RC4_128_EXPORT40_WITH_MD5,
	ssl_enc_rc4_init,ssl_enc_rc4,ssl_compute_md5_mac,
	16,MD5_MAC_SIZE,1,0,
	SSL_CK_RC4_128_EXPORT40_WITH_MD5,40},
#endif

/* RC2_128_CBC_WITH_MD5 */
	{	0,SSL_TXT_RC2_128_CBC_WITH_MD5,
	NULL,NULL,NULL,
	0,0,0,0 ,SSL_CK_RC2_128_CBC_WITH_MD5,0},

/* RC2_128_CBC_EXPORT40_WITH_MD5 */
	{	0,SSL_TXT_RC2_128_CBC_EXPORT40_WITH_MD5,
	NULL,NULL,NULL,
	0,0,0,0,
	SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5,40},

#ifndef NOIDEA
/* IDEA_128_CBC_WITH_MD5 */
	{	1,SSL_TXT_IDEA_128_CBC_WITH_MD5,
	ssl_enc_idea_cbc_init,ssl_enc_idea_cbc,ssl_compute_md5_mac,
	16,MD5_MAC_SIZE,8,8,
	SSL_CK_IDEA_128_CBC_WITH_MD5,0},
#endif

/* DES_64_CBC_WITH_MD5 */
	{	1,SSL_TXT_DES_64_CBC_WITH_MD5,
	ssl_enc_des_cbc_init,ssl_enc_des_cbc,ssl_compute_md5_mac,
	8,MD5_MAC_SIZE,8,8,
	SSL_CK_DES_64_CBC_WITH_MD5,0},

/* DES_64_CBC_WITH_SHA */
	{	1,SSL_TXT_DES_64_CBC_WITH_SHA,
	ssl_enc_des_cbc_init,ssl_enc_des_cbc,ssl_compute_sha_mac,
	8,SHA_MAC_SIZE,8,8,
	SSL_CK_DES_64_CBC_WITH_SHA,0},

/* DES_192_EDE3_CBC_WITH_MD5 */
	{	1,SSL_TXT_DES_192_EDE3_CBC_WITH_MD5,
	ssl_enc_des_ede3_cbc_init,ssl_enc_des_ede3_cbc,ssl_compute_md5_mac,
	24,MD5_MAC_SIZE,8,8,
	SSL_CK_DES_192_EDE3_CBC_WITH_MD5,0},

/* DES_192_EDE3_CBC_WITH_SHA */
	{	1,SSL_TXT_DES_192_EDE3_CBC_WITH_SHA,
	ssl_enc_des_ede3_cbc_init,ssl_enc_des_ede3_cbc,ssl_compute_sha_mac,
	24,SHA_MAC_SIZE,8,8,
	SSL_CK_DES_192_EDE3_CBC_WITH_SHA,0},

/* DES_64_CFB64_WITH_MD5_1 SSLeay */
	{	1,SSL_TXT_DES_64_CFB64_WITH_MD5_1,
	ssl_enc_des_cfb_init,ssl_enc_des_cfb,ssl_compute_md5_mac_1,
	8,1,1,8,
	SSL_CK_DES_64_CFB64_WITH_MD5_1,0},

/* NULL SSLeay (testing) */
	{	1,SSL_TXT_NULL,
	ssl_enc_null_init,ssl_enc_null,ssl_compute_null_mac,
	0,0,1,0,SSL_CK_NULL,0},

/* end of list :-) */
	};

static int num_pref_cipher=8;
static char *pref_cipher[]={
	SSL_TXT_DES_64_CBC_WITH_MD5,		/* Small key */
	SSL_TXT_DES_64_CBC_WITH_SHA,		/* Small key, SHA */
#ifndef NORC4
	SSL_TXT_RC4_128_WITH_MD5,		/* Stream cipher but fast */
#endif
	SSL_TXT_DES_192_EDE3_CBC_WITH_MD5,	/* paranoid but slow */
	SSL_TXT_DES_192_EDE3_CBC_WITH_SHA,	/* paranoid but slow, SHA */
#ifndef NOIDEA
	SSL_TXT_IDEA_128_CBC_WITH_MD5,		/* best? */
#endif
/*	SSL_TXT_DES_64_CFB64_WITH_MD5_1,*/	/* 1 byte MAC, small key */
#ifndef NORC4
	SSL_TXT_RC4_128_EXPORT40_WITH_MD5,	/* Smaller key easy to break */
#endif
	NULL
	};

#ifndef _Windows
FILE *SSL_LOG=stderr;
FILE *SSL_ERR=stderr;
#else
FILE *SSL_LOG=NULL;
FILE *SSL_ERR=NULL;
#endif

#ifdef PKT_DEBUG
void SSL_debug(file)
char *file;
	{
	SSL_LOG=fopen(file,"w");
	if (SSL_LOG == NULL)
		{
		perror(file);
		abort();
		}
	/* this is naughty ... each *character* goes out as a individual
	 * system call --tjh
	setbuf(SSL_LOG,NULL);
	 */
	}
#else
void SSL_debug(file)
char *file;
	{
	SSL_LOG=fopen(file,"w");
	}
#endif

void SSL_clear(s)
SSL *s;
	{
	s->version=SSL_SERVER_VERSION;
	s->state=ST_BEFORE;
	s->rstate=ST_READ_HEADER;
	if (s->init_buf != NULL) free(s->init_buf);
	s->init_buf=NULL;

	if (s->state_ccl != NULL) free(s->state_ccl);
	s->state_ccl=NULL;
	s->read_ahead=0;
	s->wpend_tot=0;
	s->wpend_off=0;
	s->wpend_len=0;
	s->rpend_off=0;
	s->rpend_len=0;

	s->rbuf_left=0;
	s->rbuf_offs=0;

	s->packet=s->rbuf;
	s->packet_length=0;

	s->escape=0;
	s->length=0;
	s->padding=0;
	s->ract_data_length=0;
	s->wact_data_length=0;
	s->ract_data=NULL;
	s->wact_data=NULL;
	s->mac_data=NULL;
	s->pad_data=NULL;

	if (s->crypt_state != NULL) free(s->crypt_state);
	s->crypt_state=NULL;
	s->read_key=NULL;
	s->write_key=NULL;

	s->challenge_length=0;
	if (s->challenge != NULL) free(s->challenge);
	s->challenge=NULL;
	s->conn_id_length=0;
	if (s->conn_id != NULL) free(s->conn_id);
	s->conn_id=NULL;

	s->send=0;
	s->clear_text=1;
	s->hit=0;
	s->write_ptr=NULL;

	s->read_sequence=0;
	s->write_sequence=0;
	s->trust_level=0;

	s->ctx->references++;
	}

SSL *SSL_new(ctx)
SSL_CTX *ctx;
	{
	SSL *s;

	if (ctx == NULL)
		{
		SSLerr(SSL_F_SSL_NEW,SSL_R_NULL_SSL_CTX);
		return(NULL);
		}

	s=(SSL *)malloc(sizeof(SSL));
	if (s == NULL) goto err;

	s->rfd=-1;
	s->wfd=-1;
	s->init_buf=NULL;
	s->rbuf=(unsigned char *)malloc(SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER+2);
	if (s->rbuf == NULL) goto err;
	s->wbuf=(unsigned char *)malloc(SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER+2);
	if (s->wbuf == NULL) goto err;
	s->num_pref_cipher=0;
	s->pref_cipher=NULL;
	s->crypt_state=NULL;
	s->challenge=NULL;
	s->conn_id=NULL;
	s->conn=NULL;
	s->cert=NULL;
	s->peer=NULL;
	s->peer_status=SSL_PEER_NOT_SET;
	s->verify_mode=SSL_VERIFY_NONE;
	s->verify_callback=NULL;
	s->ctx=ctx;
	ctx->references++;
	s->state_ccl=NULL;

	SSL_clear(s);
	return(s);
err:
	SSLerr(SSL_F_SSL_NEW,ERR_R_MALLOC_FAILURE);
	return(NULL);
	}

void SSL_free(s)
SSL *s;
	{
	/* add extra stuff */
	free(s->rbuf);
	free(s->wbuf);
	if (s->init_buf != NULL) free(s->init_buf);
	if (s->crypt_state != NULL) free(s->crypt_state);
	/* again, just let them linger around */
	if (s->conn != NULL) ssl_conn_free(s,s->conn);
	if (s->cert != NULL) ssl_cert_free(s->cert);
	if (s->challenge != NULL) free(s->challenge);
	if (s->conn_id != NULL) free(s->conn_id);
	if (s->state_ccl != NULL) free(s->state_ccl);
	/* free up if allocated */
	if ((s->peer_status == SSL_PEER_IN_SSL) && (s->peer != NULL))
		X509_free(s->peer);
	if (s->ctx) SSL_CTX_free(s->ctx);
	free((char *)s);
	}

void ssl_print_bytes(f, n, b)
FILE *f;
int n;
char *b;
	{
	int i;
	static char *h="0123456789abcdef";

	/* NULL means don't trace/log */
	if (f == NULL) return;

	fflush(f);
#if 0
	ssl_ddt_dump_fd(fileno(f),b,n);
#else

	for (i=0; i<n; i++)
		{
		Fputc(h[(b[i]>>4)&0x0f],f);
		Fputc(h[(b[i]   )&0x0f],f);
		Fputc(' ',f);
		}
#endif

	}

void ssl_return_error(s)
SSL *s;
	{
	static unsigned char buf[1]={SSL_MT_ERROR};

	SSL_write(s,(char *)buf,1);
#ifdef ABORT_DEBUG
	abort(1);
#endif
	}


void SSL_set_fd(s, fd)
SSL *s;
int fd;
	{
	s->rfd=fd;
	s->wfd=fd;
	}

int SSL_get_fd(s)
SSL *s;
	{
	return(s->rfd);
	}

SSL_CTX *SSL_get_SSL_CTX(s)
SSL *s;
	{
	return(s->ctx);
	}

int SSL_set_pref_cipher(s, str)
SSL *s;
char *str;
	{
	int i,j;
	char *p,**pp;

	if (str == NULL) return(1);
	if (s->pref_cipher != NULL)
		{
		free(s->pref_cipher[0]);
		free(s->pref_cipher);
		}

	p=str;
	i=(*p == '\0')?0:1;
	for (; *p; p++)
		if (*p == ':') i++;
	pp=(char **)malloc(sizeof(char *)*(i+1));
	if (pp == NULL) goto err;
	pp[0]=(char *)malloc(strlen(str)+1);
	if (pp == NULL) goto err;
	strcpy(pp[0],str);
	p=pp[0];
	j=1;
	for (;;)
		{
		while ((*p != ':') && (*p != '\0'))
			p++;
		if (*p == '\0') break;
		*p='\0';
		p++;
		pp[j++]=p;
		}
	pp[j]=NULL;
	s->num_pref_cipher=i;
	s->pref_cipher=pp;
	return(1);
err:
	SSLerr(SSL_F_SSL_NEW,ERR_R_MALLOC_FAILURE);
	return(0);
	}

char *SSL_get_pref_cipher(s, n)
SSL *s;
int n;
	{
	if (n < s->num_pref_cipher)
		return(s->pref_cipher[n]);

	if (s->num_pref_cipher != 0) return(NULL);

	n-=s->num_pref_cipher;
	if (n < num_pref_cipher)
		return(pref_cipher[n]);
	return(NULL);
	}

char *SSL_get_cipher(s)
SSL *s;
	{
	if ((s->conn != NULL) && (s->conn->cipher != NULL))
		return(s->conn->cipher->name);
	return(NULL);
	}

char *SSL_get_shared_ciphers(s,buf,len)
SSL *s;
char *buf;
int len;
	{
	char *p,*cp;
	CIPHER **c;
	int i;

	if ((s->conn == NULL) || (s->conn->ciphers == NULL) ||
		(len < 2))
		return(NULL);

	p=buf;
	c= &(s->conn->ciphers[0]);
	len--;
	i=0;
	for (i=0; i<s->conn->num_ciphers; i++)
		{
		for (cp=c[i]->name; *cp; )
			{
			if (--len == 0)
				{
				*p='\0';
				return(buf);
				}
			else
				*(p++)= *(cp++);
			}
		*(p++)=':';
		}
	p[-1]='\0';
	return(buf);
	}

int SSL_copy_session_id(to, from)
SSL *to;
SSL *from;
	{
	if (from->conn == NULL) return(0);
	to->conn=from->conn;
	from->conn->references++;
	return(1);
	}

void SSL_set_verify(s, mode, callback)
SSL *s;
int mode;
int (*callback)();
	{
	s->verify_mode=mode;
	s->verify_callback=callback;
	}

void SSL_set_read_ahead(s, yes)
SSL *s;
int yes;
	{
	s->read_ahead=yes;
	}

int SSL_get_read_ahead(s)
SSL *s;
	{
	return(s->read_ahead);
	}

int SSL_pending(s)
SSL *s;
	{
	return(s->rbuf_left);
	}

int SSL_set_timeout(s, t)
SSL *s;
int t;
	{
	if (s->conn == NULL) return(0);
	s->conn->timeout=t;
	return(1);
	}

long SSL_get_timeout(s)
SSL *s;
	{
	if (s->conn == NULL) return(0);
	return(s->conn->timeout);
	}

long SSL_get_time(s)
SSL *s;
	{
	if (s->conn == NULL) return(0);
	return(s->conn->time);
	}

void SSL_flush_connections()
	{
	/* not implemented yet */
	}

int SSL_is_init_finished(s)
SSL *s;
	{
	return(s->state == ST_OK);
	}

X509 *SSL_get_peer_certificate(s)
SSL *s;
	{
	X509 *r=NULL;
	
	if (s->peer_status == SSL_PEER_NOT_SET)
		r=NULL;
	else if (s->peer_status == SSL_PEER_IN_SSL)
		r=s->peer;
	else if ((s->peer_status == SSL_PEER_IN_CERT) &&
		(s->conn != NULL) &&
		(s->conn->cert != NULL))
		r=s->conn->cert->x509;

	if (r == NULL) return(0);
	r->references++;
	return(r);
	}

int ssl_cipher_cmp(a,b)
CIPHER *a,*b;
	{
	int i;

	i=a->c1-b->c1;
	if (i) return(i);
	i=a->c2-b->c2;
	if (i) return(i);
	return(a->c3-b->c3);
	}

int ssl_cipher_ptr_cmp(ap,bp)
CIPHER **ap,**bp;
	{
	int i;
	CIPHER *a= *ap,*b= *bp;

	i=a->c1-b->c1;
	if (i) return(i);
	i=a->c2-b->c2;
	if (i) return(i);
	return(a->c3-b->c3);
	}

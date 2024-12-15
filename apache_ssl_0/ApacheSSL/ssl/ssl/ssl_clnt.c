/* ssl/ssl_clnt.c */
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
#include "md5.h"
#include "rand.h"
#include "buffer.h"
#include "objects.h"
#include "ssl_locl.h"
#include "envelope.h"

#ifndef NOPROTO
static int get_server_finish(SSL *s);
static int get_server_verify(SSL *s);
static int get_server_hello(SSL *s);
static int client_hello(SSL *s); 
static int client_master_key(SSL *s);
static int client_finish(SSL *s);
static int client_certificate(SSL *s);
static int choose_cipher(SSL *s);
#else
static int get_server_finish();
static int get_server_verify();
static int get_server_hello();
static int client_hello(); 
static int client_master_key();
static int client_finish();
static int client_certificate();
static int choose_cipher();
#endif

#define BREAK	break

int SSL_connect(s)
SSL *s;
	{
	unsigned long l=time(NULL);
	BUFFER *buf=NULL;
	int ret;

	RAND_seed((unsigned char *)&l,sizeof(l));
	ERR_clear_error();

	/* init things to blank */
	if (!SSL_ST_INIT(s->state)) SSL_clear(s);

	errno=0;
	for (;;)
		{
#ifdef SSL_DEBUG
		SSL_TRACE(SSL_ERR,"state=%02X %d %s\n",
			s->state,s->init_num,ssl_state_string(s));
#endif
		switch (s->state)
			{
		case ST_BEFORE:
			if ((buf=BUF_new()) == NULL) return(-1);
			if (!BUF_grow(buf,(unsigned long)
				SSL_MAX_RECORD_LENGTH_3_BYTE_HEADER))
				return(-1);
			if (s->init_buf) free(s->init_buf);
			s->init_buf=(unsigned char *)buf->data;
			free(buf);
			s->init_num=0;
			s->state=ST_SEND_CLIENT_HELLO_A;
			BREAK;

		case ST_SEND_CLIENT_HELLO_A:
		case ST_SEND_CLIENT_HELLO_B:
			ret=client_hello(s);
			if (ret <= 0) return(ret);
			s->init_num=0;
			s->state=ST_GET_SERVER_HELLO_A;
			BREAK;
		
		case ST_GET_SERVER_HELLO_A:
		case ST_GET_SERVER_HELLO_B:
			ret=get_server_hello(s);
			if (ret <= 0) return(ret);
			s->init_num=0;
			if (!s->hit) /* new connection */
				{
				s->state=ST_SEND_CLIENT_MASTER_KEY_A;
				BREAK; 
				}
			else
				{
				s->state=ST_START_ENCRYPTION;
				break;
				}
	
		case ST_SEND_CLIENT_MASTER_KEY_A:
		case ST_SEND_CLIENT_MASTER_KEY_B:
			ret=client_master_key(s);
			if (ret <= 0) return(ret);
			s->init_num=0;
			s->state=ST_START_ENCRYPTION;
			BREAK;

		case ST_START_ENCRYPTION:
			/* Ok, we now have all the stuff needed to
			 * start encrypting, so lets fire it up :-) */
			if (!s->conn->cipher->crypt_init(s,1)) return(-1);
			s->clear_text=0;
			s->state=ST_SEND_CLIENT_FINISHED_A;
			BREAK;

		case ST_SEND_CLIENT_FINISHED_A:
		case ST_SEND_CLIENT_FINISHED_B:
			ret=client_finish(s);
			if (ret <= 0) return(ret);
			s->init_num=0;
			s->state=ST_GET_SERVER_VERIFY_A;
			BREAK;

		case ST_GET_SERVER_VERIFY_A:
		case ST_GET_SERVER_VERIFY_B:
			ret=get_server_verify(s);
			if (ret <= 0) return(ret);
			s->init_num=0;
			s->state=ST_GET_SERVER_FINISHED_A;
			BREAK;

		case ST_GET_SERVER_FINISHED_A:
		case ST_GET_SERVER_FINISHED_B:
			ret=get_server_finish(s);
			if (ret <= 0) return(ret);
			break;

		case ST_SEND_CLIENT_CERTIFICATE_A:
		case ST_SEND_CLIENT_CERTIFICATE_B:
		case ST_SEND_CLIENT_CERTIFICATE_C:
		case ST_SEND_CLIENT_CERTIFICATE_D:
			ret=client_certificate(s);
			if (ret <= 0) return(ret);
			s->init_num=0;
			s->state=ST_GET_SERVER_FINISHED_A;
			break;

		case ST_OK:
			free(s->init_buf);
			s->init_buf=NULL;
			s->init_num=0;
			ERR_clear_error();
			return(1);
		default:
			SSLerr(SSL_F_SSL_CONNECT,SSL_R_UNKNOWN_STATE);
			return(-1);
			break;
			}
		}
	}

static int get_server_hello(s)
SSL *s;
	{
	unsigned char *buf;
	unsigned char *p;
	CIPHER **cipher;
	CIPHER *tmp_c[NUM_CIPHERS];
	int i,j,num_c,nnum_c;

	buf=s->init_buf;
	p=buf;
	if (s->state == ST_GET_SERVER_HELLO_A)
		{
		i=SSL_read(s,(char *)&(buf[s->init_num]),11-s->init_num);
		if (i < (11-s->init_num)) 
			return(ssl_part_read(s,SSL_F_GET_SERVER_HELLO,i));

		if (*(p++) != SSL_MT_SERVER_HELLO)
			{
			if (p[-1] != SSL_MT_ERROR)
				{
				ssl_return_error(s);
				SSLerr(SSL_F_GET_SERVER_HELLO,
					SSL_R_READ_WRONG_PACKET_TYPE);
				}
			else
				SSLerr(SSL_F_GET_SERVER_HELLO,
					SSL_R_PEER_ERROR);
			return(-1);
			}
		s->hit=(*(p++))?1:0;
		s->state_cert_type= *(p++);
		n2s(p,i);
		if (i < s->version) s->version=i;
		n2s(p,i); s->state_cert_length=i;
		n2s(p,i); s->state_csl=i;
		n2s(p,i); s->state_conn_id_length=i;
		s->state=ST_GET_SERVER_HELLO_B;
		s->init_num=0;
		}

	/* ST_GET_SERVER_HELLO_B */
	j=s->state_cert_length+s->state_csl+s->state_conn_id_length
		- s->init_num;
	i=SSL_read(s,(char *)&(buf[s->init_num]),j);
	if (i != j) return(ssl_part_read(s,SSL_F_GET_SERVER_HELLO,i));

	/* things are looking good */

	p=buf;
	if (s->hit)
		{
		if ((s->state_cert_length != 0) ||
			/*(s->state_cert_type != 0) || */ (s->state_csl != 0))
			{
			/*return(-1);*/
			/* ignore these errors :-(
			 * SSLref does return a non-zero cert_type */
			}
		}
	else
		{
		if (s->conn->session_id != NULL) free(s->conn->session_id);
		s->conn->session_id=NULL;
		s->conn->session_id_length=0;

		if (ssl_set_certificate(s,s->state_cert_type,
			s->state_cert_length,p) <= 0)
			{
			ssl_return_error(s);
			return(-1);
			}
		p+=s->state_cert_length;

		if (s->state_csl == 0)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_GET_SERVER_HELLO,SSL_R_NO_CIPHER_LIST);
			return(-1);
			}

		/* what we will do is copy the ciphers we are happy to
		 * use to a temp area and then check the server ones
		 * are put those that are in the conn structure again. */
		cipher= &(s->conn->ciphers[0]);
		memcpy(&(tmp_c[0]),cipher,sizeof(CIPHER *)*NUM_CIPHERS);
		num_c=s->conn->num_ciphers;
		nnum_c=0;
		for (i=0; i<s->state_csl; i+=3)
			{
			CIPHER c,*cp= &c,**cpp;

			c.c1=p[i];
			c.c2=p[i+1];
			c.c3=p[i+2];
			
			cpp=(CIPHER **)OBJ_bsearch((char *)&cp,
				(char *)&(cipher[0]),num_c,sizeof(CIPHER *),
				(int (*)())ssl_cipher_ptr_cmp);
			if ((cpp != NULL) && ((*cpp)->valid))
				{
				s->conn->ciphers[nnum_c++]= *cpp;
				}
			if (nnum_c == NUM_CIPHERS) break;
			}
		p+=s->state_csl;
		s->conn->num_ciphers=nnum_c;

		if (nnum_c == 0)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_GET_SERVER_HELLO,SSL_R_NO_CIPHER_MATCH);
			return(-1);
			}
		qsort((char *)&(cipher[0]),nnum_c,sizeof(CIPHER *),
			(int (*)())ssl_cipher_ptr_cmp);

		/* we now have the sorted cipher in s->conn->ciphers; */

		/* pick a cipher - which one? */
		if (!choose_cipher(s))
			{
			ssl_return_error(s);
			SSLerr(SSL_F_GET_SERVER_HELLO,SSL_R_NO_CIPHER_WE_TRUST);
			return(-1);
			}
		}

	if ((s->peer_status == SSL_PEER_IN_SSL) && (s->peer != NULL))
		X509_free(s->peer);
	s->peer=NULL;
	s->peer_status=SSL_PEER_IN_CERT;
	/* get conn_id */
	if (s->conn_id_length != s->state_conn_id_length)
		{
		if (s->conn_id) free(s->conn_id);
		s->conn_id=(unsigned char *)malloc(s->state_conn_id_length);
		if (s->conn_id == NULL)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_GET_SERVER_HELLO,ERR_R_MALLOC_FAILURE);
			return(-1);
			}
		}
	s->conn_id_length=s->state_conn_id_length;
	memcpy(s->conn_id,p,s->state_conn_id_length);
	return(1);
	}

static int client_hello(s)
SSL *s;
	{
	unsigned char *buf;
	unsigned char *p,*d;
	CIPHER **cipher;
	int i,n,j;

	buf=s->init_buf;
	if (s->state == ST_SEND_CLIENT_HELLO_A)
		{
		if (s->conn == NULL)
			if (!ssl_new_conn(s,0))
				{
				ssl_return_error(s);
				return(-1);
				}
		/* else use the pre-loaded session */

		cipher= &(s->conn->ciphers[0]);
		p=buf;					/* header */
		d=p+9;					/* data section */
		*(p++)=SSL_MT_CLIENT_HELLO;		/* type */
		s2n(SSL_CLIENT_VERSION,p);		/* version */
		i=n=j=0;
		for (i=0; i<NUM_CIPHERS; i++)
			{
			if (ssl_ciphers[i].valid)
				{
				cipher[j++]= &(ssl_ciphers[i]);
				*(d++)=ssl_ciphers[i].c1;
				*(d++)=ssl_ciphers[i].c2;
				*(d++)=ssl_ciphers[i].c3;
				n+=3;
				}
			}
		s->conn->num_ciphers=j;
		s2n(n,p);			/* cipher spec num bytes */

		if (s->conn->session_id != NULL)
			{
			i=s->conn->session_id_length;
			s2n(i,p);		/* session id length */
			memcpy(d,s->conn->session_id,(unsigned int)i);
			d+=i;
			}
		else
			{
			s2n(0,p);
			}

		s->challenge_length=CHALLENGE_LENGTH;
		s2n(CHALLENGE_LENGTH,p);		/* challenge length */
		s->challenge=(unsigned char *)malloc(CHALLENGE_LENGTH);
		if (s->challenge == NULL)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_CLIENT_HELLO,ERR_R_MALLOC_FAILURE);
			return(-1);
			}
		if (s->ctx->reverse)
			{
			memcpy(s->challenge,s->ctx->challenge,
				s->ctx->challenge_length);
			}
		else
			/*challenge id data*/
			RAND_bytes(s->challenge,CHALLENGE_LENGTH);
		memcpy(d,s->challenge,CHALLENGE_LENGTH);
		d+=CHALLENGE_LENGTH;

		s->state=ST_SEND_CLIENT_HELLO_B;
		s->init_num=d-buf;
		s->init_off=0;
		}
	/* ST_SEND_CLIENT_HELLO_B */
	return(ssl_do_write(s));
	}

static int client_master_key(s)
SSL *s;
	{
	unsigned char *buf;
	unsigned char *p,*d;
	int clear,enc,karg,i;

	buf=s->init_buf;
	if (s->state == ST_SEND_CLIENT_MASTER_KEY_A)
		{
		p=buf;
		d=p+10;
		*(p++)=SSL_MT_CLIENT_MASTER_KEY;/* type */
		*(p++)=s->conn->cipher->c1;	/* cipher type - byte 1 */
		*(p++)=s->conn->cipher->c2;	/* cipher type - byte 2 */
		*(p++)=s->conn->cipher->c3;	/* cipher type - byte 3 */

		/* make a master key */
		i=s->conn->master_key_length=(unsigned char)
			s->conn->cipher->key_size;
		s->conn->master_key=(unsigned char *)malloc((unsigned int)i+1);
		if (s->conn->master_key == NULL)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_CLIENT_MASTER_KEY,ERR_R_MALLOC_FAILURE);
			return(-1);
			}
		if (s->ctx->reverse)
			{
			if (s->ctx->master_key_length != i)
				{
				SSLerr(SSL_F_CLIENT_MASTER_KEY,SSL_R_REVERSE_MASTER_KEY_LENGTH_IS_WRONG);
				return(-1);
				}
			memcpy(s->conn->master_key,s->ctx->master_key,i);
			}
		else
			RAND_bytes(s->conn->master_key,i);

		/* make key_arg data */
		i=s->conn->key_arg_length=s->conn->cipher->key_arg_size;
		if (i == 0)
			s->conn->key_arg=NULL;
		else
			{
			s->conn->key_arg=(unsigned char *)
				malloc((unsigned int)i);
			if (s->conn->key_arg == NULL)
				{
				ssl_return_error(s);
				SSLerr(SSL_F_CLIENT_MASTER_KEY,ERR_R_MALLOC_FAILURE);
				return(-1);
				}
			if (s->ctx->reverse)
				{
				if (s->ctx->key_arg_length != i)
					{
					SSLerr(SSL_F_CLIENT_MASTER_KEY,SSL_R_REVERSE_KEY_ARG_LENGTH_IS_WRONG);
					return(-1);
					}
				memcpy(s->conn->key_arg,s->ctx->key_arg,i);
				}
			else
				RAND_bytes(s->conn->key_arg,i);
			}

		i=s->conn->cipher->key_size*8;
		enc=s->conn->cipher->enc_bits;
		if (enc == 0) enc=i;
		if (i < enc)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_CLIENT_MASTER_KEY,SSL_R_CIPHER_TABLE_SRC_ERROR);
			return(-1);
			}
		clear=i-enc;
		/* bytes or bits? */
		clear/=8; /* clear */
		s2n(clear,p);
		memcpy(d,s->conn->master_key,(unsigned int)clear);
		d+=clear;

		enc/=8;
		enc=s->conn->cert->public_encrypt(s->conn->cert,enc,
			&(s->conn->master_key[clear]),d);
		if (enc <= 0)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_CLIENT_MASTER_KEY,SSL_R_PUBLIC_KEY_ENCRYPT_ERROR);
			return(-1);
			}
		s2n(enc,p);
		d+=enc;
		karg=s->conn->cipher->key_arg_size;	
		s2n(karg,p); /* key arg size */
		memcpy(d,s->conn->key_arg,(unsigned int)karg);
		d+=karg;

		s->state=ST_SEND_CLIENT_MASTER_KEY_B;
		s->init_num=d-buf;
		s->init_off=0;
		}

	/* ST_SEND_CLIENT_MASTER_KEY_B */
	return(ssl_do_write(s));
	}

static int client_finish(s)
SSL *s;
	{
	unsigned char *p;

	if (s->state == ST_SEND_CLIENT_FINISHED_A)
		{
		p=s->init_buf;
		*(p++)=SSL_MT_CLIENT_FINISHED;
		memcpy(p,s->conn_id,(unsigned int)s->conn_id_length);

		s->state=ST_SEND_CLIENT_FINISHED_B;
		s->init_num=s->conn_id_length+1;
		s->init_off=0;
		}
	return(ssl_do_write(s));
	}

/* read the data and then respond */
static int client_certificate(s)
SSL *s;
	{
	unsigned char *buf;
	unsigned char *p,*d;
	int i;
	unsigned int n;
	int cert_ch_len=0;
	unsigned char *cert_ch;

	buf=s->init_buf;
	cert_ch= &(buf[2]);
	if (s->state == ST_SEND_CLIENT_CERTIFICATE_A)
		{
		i=SSL_read(s,(char *)&(buf[s->init_num]),
			MAX_CERT_CHALLENGE_LENGTH+1-s->init_num);
		if (i<(MIN_CERT_CHALLENGE_LENGTH+1-s->init_num))
			return(ssl_part_read(s,SSL_F_CLIENT_CERTIFICATE,i));

		/* type=buf[0]; */
		/* type eq x509 */
		if (buf[1] != SSL_AT_MD5_WITH_RSA_ENCRYPTION)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_CLIENT_CERTIFICATE,SSL_R_BAD_AUTHENTICATION_TYPE);
			return(-1);
			}
		cert_ch_len=i-1;

		if ((s->cert == NULL) || (s->cert->x509 == NULL) ||
			(s->cert->privatekey == NULL))
			{
			p=buf;
			*(p++)=SSL_MT_ERROR;
			s2n(SSL_PE_NO_CERTIFICATE,p);
			s->init_off=0;
			s->init_num=3;
			s->state=ST_SEND_CLIENT_CERTIFICATE_B;
			}
		else
			s->state=ST_SEND_CLIENT_CERTIFICATE_C;
		}

	if (s->state == ST_SEND_CLIENT_CERTIFICATE_B)
		{
		return(ssl_do_write(s));
		}

	if (s->state == ST_SEND_CLIENT_CERTIFICATE_C)
		{
		EVP_MD_CTX ctx;

		/* ok, now we calculate the checksum
		 * do it first so we can reuse buf :-) */
		p=buf;
		EVP_SignInit(&ctx,EVP_md5);
		EVP_SignUpdate(&ctx,s->conn->key_material,
			(unsigned int)s->conn->key_material_length);
		EVP_SignUpdate(&ctx,cert_ch,(unsigned int)cert_ch_len);
		n=i2d_X509(s->conn->cert->x509,&p);
		EVP_SignUpdate(&ctx,buf,(unsigned int)n);

		p=buf;
		d=p+6;
		*(p++)=SSL_MT_CLIENT_CERTIFICATE;
		*(p++)=SSL_CT_X509_CERTIFICATE;
		n=i2d_X509(s->cert->x509,&d);
		s2n(n,p);

		if (!EVP_SignFinal(&ctx,d,&n,s->cert->privatekey))
			{
			/* this is not good.  If things have failed it
			 * means there so something wrong with the key.
			 * We will contiune with a 0 length signature
			 */
			}
		s2n(n,p);
		d+=n;

		s->state=ST_SEND_CLIENT_CERTIFICATE_D;
		s->init_num=d-buf;
		s->init_off=0;
		}
	/* if (s->state == ST_SEND_CLIENT_CERTIFICATE_D) */
	return(ssl_do_write(s));
	}

static int get_server_verify(s)
SSL *s;
	{
	unsigned char *p;
	int i;

	p=s->init_buf;
	if (s->state == ST_GET_SERVER_VERIFY_A)
		{
		i=SSL_read(s,(char *)&(p[s->init_num]),1-s->init_num);
		if (i < (1-s->init_num)) 
			return(ssl_part_read(s,SSL_F_GET_SERVER_VERIFY,i));

		s->state= ST_GET_SERVER_VERIFY_B;
		s->init_num=0;
		if (*p != SSL_MT_SERVER_VERIFY)
			{
			if (p[0] != SSL_MT_ERROR)
				{
				ssl_return_error(s);
				SSLerr(SSL_F_GET_SERVER_VERIFY,
					SSL_R_READ_WRONG_PACKET_TYPE);
				}
			else
				SSLerr(SSL_F_GET_SERVER_VERIFY,
					SSL_R_PEER_ERROR);
			return(-1);
			}
		}
	
	p=s->init_buf;
	i=SSL_read(s,(char *)&(p[s->init_num]),
		(unsigned int)s->challenge_length-s->init_num);
	if (i < (s->challenge_length-s->init_num))
		return(ssl_part_read(s,SSL_F_GET_SERVER_VERIFY,i));
	if (memcmp(p,s->challenge,(unsigned int)s->challenge_length) != 0)
		{
		ssl_return_error(s);
		SSLerr(SSL_F_GET_SERVER_VERIFY,SSL_R_CHALLENGE_IS_DIFFERENT);
		return(-1);
		}
	return(1);
	}

static int get_server_finish(s)
SSL *s;
	{
	unsigned char *buf;
	unsigned char *p;
	int i;

	buf=s->init_buf;
	p=buf;
	if (s->state == ST_GET_SERVER_FINISHED_A)
		{
		i=SSL_read(s,(char *)&(buf[s->init_num]),1-s->init_num);
		if (i < (1-s->init_num))
			return(ssl_part_read(s,SSL_F_GET_SERVER_FINISHED,i));
		s->init_num=i;
		if (*p == SSL_MT_REQUEST_CERTIFICATE)
			{
			s->state=ST_SEND_CLIENT_CERTIFICATE_A;
			return(1);
			}
		else if (*p != SSL_MT_SERVER_FINISHED)
			{
			if (p[0] != SSL_MT_ERROR)
				{
				ssl_return_error(s);
				SSLerr(SSL_F_GET_SERVER_FINISHED,SSL_R_READ_WRONG_PACKET_TYPE);
				}
			else
				SSLerr(SSL_F_GET_SERVER_FINISHED,SSL_R_PEER_ERROR);
			return(-1);
			}
		s->state=ST_OK;
		s->init_num=0;
		}

	i=SSL_read(s,(char *)&(buf[s->init_num]),
		SESSION_ID_LENGTH-s->init_num);
	if (i < (SESSION_ID_LENGTH-s->init_num))
		return(ssl_part_read(s,SSL_F_GET_SERVER_FINISHED,i));

	if (!s->hit) /* new connection */
		{
		if ((s->conn->session_id == NULL) ||
			(s->conn->session_id_length <
				SESSION_ID_LENGTH))
			{
			if (s->conn->session_id)
				free(s->conn->session_id);
			s->conn->session_id=(unsigned char *)
				malloc(SESSION_ID_LENGTH);
			if (s->conn->session_id == NULL)
				{
				ssl_return_error(s);
				SSLerr(SSL_F_GET_SERVER_FINISHED,
					ERR_R_MALLOC_FAILURE);
				return(-1);
				}
			}
		s->conn->session_id_length=SESSION_ID_LENGTH;
		memcpy(s->conn->session_id,p,SESSION_ID_LENGTH);
		ssl_add_hash_conn(s,s->conn); /* should return error? */
		}
	else
		{
		if (memcmp(buf,s->conn->session_id,
			(unsigned int)s->conn->session_id_length) != 0)
			{
			ssl_return_error(s);
			SSLerr(SSL_F_GET_SERVER_FINISHED,SSL_R_SESSION_ID_IS_DIFFERENT);
			return(-1);
			}
		}
	return(1);
	}

static int choose_cipher(s)
SSL *s;
	{
	int j,i=0;
	char *p;

	if (s->ctx->reverse)
		{
		for (;;)
			{
			for (j=0; s->conn->ciphers[j] != NULL; j++)
				{
				if (strcmp(s->ctx->cipher,
					s->conn->ciphers[j]->name) == 0)
					{
					s->conn->cipher=s->conn->ciphers[j];
					return(1);
					}
				}
			return(0);
			}
		}

	for (;;)
		{
		p=SSL_get_pref_cipher(s,i++);
		if (p == NULL)
			{
			s->conn->cipher=NULL;
			return(0);
			}
		for (j=0; s->conn->ciphers[j] != NULL; j++)
			{
			if (strcmp(p,s->conn->ciphers[j]->name) == 0)
				{
				s->conn->cipher=s->conn->ciphers[j];
				return(1);
				}
			}
		}
	}


/* ssl/ssl_rsa.c */
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
#include "rsa.h"
#include "x509.h"
#include "pem.h"
#include "ssl_locl.h"
#include "ssl_rsa.h"

int ssl_rsa_public_encrypt(c, len, from, to)
CERT *c;
int len;
unsigned char *from;
unsigned char *to;
	{
	RSA *rsa;
	int i;

	if ((c == NULL) || (c->publickey == NULL))
		{
		SSLerr(SSL_F_SSL_CERT_RSA_PUBLIC_ENCRYPT,SSL_R_NO_PUBLICKEY);
		return(-1);
		}
	rsa=c->publickey;
	/* we have the public key */
#ifdef RSA_DEBUG
	SSL_TRACE(SSL_ERR,"PUBLIC RSA INPUT %d %d\n",len,to);
	for (j=0; j<len; j++) SSL_TRACE(SSL_ERR,"%02X ",from[j]);
	SSL_TRACE(SSL_ERR,"\n");
	for (j=0; j<64; j++) SSL_TRACE(SSL_ERR,"%02X ",to[j]);
	SSL_TRACE(SSL_ERR,"\n");
#endif
	i=RSA_public_encrypt(len,from,to,rsa);
	if (i < 0)
		{
		SSLerr(SSL_F_SSL_CERT_RSA_PUBLIC_ENCRYPT,ERR_R_RSA_LIB);
		}
#ifdef RSA_DEBUG
	SSL_TRACE(SSL_ERR,"PUBLIC RSA OUTPUT %d %08x\n",i,to);
	for (j=0; j<i; j++) SSL_TRACE(SSL_ERR,"%02X ",to[j]);
	SSL_TRACE(SSL_ERR,"\n");
#endif
	return(i);
	}

#ifdef undef
int ssl_rsa_private_encrypt(c, len, from, to)
CERT *c;
int len;
unsigned char *from;
unsigned char *to;
	{
	RSA *rsa;
	int i;

	if ((c == NULL) || (c->privatekey == NULL))
		{
		SSLerr(SSL_F_SSL_CERT_RSA_PRIVATE_ENCRYPT,SSL_R_NO_PRIVATEKEY);
		return(-1);
		}
	rsa=c->privatekey;
	/* we have the private key */
#ifdef RSA_DEBUG
	SSL_TRACE(SSL_ERR,"PRIVATE RSA INPUT %d %d\n",len,to);
	for (j=0; j<len; j++) SSL_TRACE(SSL_ERR,"%02X ",from[j]);
	SSL_TRACE(SSL_ERR,"\n");
#endif
	i=RSA_private_encrypt(len,from,to,rsa);
	if (i < 0)
		{
		SSLerr(SSL_F_SSL_CERT_RSA_PRIVATE_ENCRYPT,ERR_R_RSA_LIB);
		}
#ifdef RSA_DEBUG
	SSL_TRACE(SSL_ERR,"PRIVATE RSA OUTPUT %d %d\n",i,to);
	for (j=0; j<i; j++) SSL_TRACE(SSL_ERR,"%02X ",to[j]);
	SSL_TRACE(SSL_ERR,"\n");
#endif
	return(i);
	}

#endif

int ssl_rsa_private_decrypt(c, len, from, to)
CERT *c;
int len;
unsigned char *from;
unsigned char *to;
	{
	RSA *rsa;
	int i;

	if ((c == NULL) || (c->privatekey == NULL))
		{
		SSLerr(SSL_F_SSL_CERT_RSA_PRIVATE_DECRYPT,SSL_R_NO_PRIVATEKEY);
		return(-1);
		}
	rsa=c->privatekey;

	/* we have the public key */
#ifdef RSA_DEBUG
	SSL_TRACE(SSL_ERR,"PRIVATE RSA INPUT %d\n",len);
	for (j=0; j<len; j++) SSL_TRACE(SSL_ERR,"%02X ",from[j]);
	SSL_TRACE(SSL_ERR,"\n");
#endif
	i=RSA_private_decrypt(len,from,to,rsa);
	if (i < 0)
		SSLerr(SSL_F_SSL_CERT_RSA_PRIVATE_DECRYPT,ERR_R_RSA_LIB);
#ifdef RSA_DEBUG
	SSL_TRACE(SSL_ERR,"PRIVATE RSA OUTPUT %d\n",i);
	for (j=0; j<i; j++) SSL_TRACE(SSL_ERR,"%02X ",to[j]);
	SSL_TRACE(SSL_ERR,"\n");
#endif
	return(i);
	}

int SSL_use_certificate(ssl, x)
SSL *ssl;
X509 *x;
	{
	CERT *c;

	if (ssl->cert == NULL)
		{
		c=ssl_cert_new();
		if (c == NULL) return(0);
		ssl->cert=c;
		}
	c=ssl->cert;

	if (!ssl_set_cert_type(c,SSL_CT_X509_CERTIFICATE)) return(0);
	if (c->x509 != NULL) X509_free(x);
	c->x509=x;
	x->references++;
	if (c->publickey != NULL) RSA_free(c->publickey);
	c->publickey=X509_extract_key(x);
	if (c->publickey == NULL) return(0);
	return(1);
	}

int SSL_use_certificate_file(ssl, file, type)
SSL *ssl;
char *file;
int type;
	{
	int j;
	FILE *in;
	X509 *x;

	in=fopen(file,"r");
	if (in == NULL)
		{
		SYSerr(ERR_F_FOPEN,errno);
		SSLerr(SSL_F_SSL_USE_CERTIFICATE_FILE,ERR_R_SYS_LIB);
		return(0);
		}
	if (type == SSL_FILETYPE_ASN1)
		{
		j=ERR_R_ASN1_LIB;
		x=d2i_X509_fp(in,NULL);
		}
#ifdef F2I
	else if (type == SSL_FILETYPE_TEXT)
		{
		j=ERR_R_ASN1_LIB;
		i=f2i_X509(in,x);
		}
#endif
	else if (type == SSL_FILETYPE_PEM)
		{
		j=ERR_R_PEM_LIB;
		x=PEM_read_X509(in,NULL,NULL);
		}
	else
		{
		SSLerr(SSL_F_SSL_USE_CERTIFICATE_FILE,SSL_R_BAD_SSL_FILETYPE);
		fclose(in);
		return(0);
		}
	fclose(in);

	if (x == NULL)
		{
		SSLerr(SSL_F_SSL_USE_CERTIFICATE_FILE,j);
		return(0);
		}

	return(SSL_use_certificate(ssl,x));
	}

int SSL_use_certificate_ASN1(ssl, len, d)
SSL *ssl;
int len;
unsigned char *d;
	{
	X509 *x;

	x=d2i_X509(NULL,&d,(long)len);
	if (x == NULL)
		{
		SSLerr(SSL_F_SSL_USE_CERTIFICATE_ASN1,ERR_R_ASN1_LIB);
		return(0);
		}

	return(SSL_use_certificate(ssl,x));
	}

int SSL_use_RSAPrivateKey(ssl, rsa)
SSL *ssl;
RSA *rsa;
	{
	CERT *c;

	if (ssl->cert == NULL)
		{
		c=ssl_cert_new();
		if (c == NULL)
			{
			SSLerr(SSL_F_SSL_USE_RSAPRIVATEKEY,ERR_R_MALLOC_FAILURE);
			return(0);
			}
		ssl->cert=c;
		}
	c=ssl->cert;

	if (!ssl_set_cert_type(c,SSL_CT_X509_CERTIFICATE)) return(0);
	if (c->privatekey) RSA_free(c->privatekey);
	c->privatekey=rsa;
	rsa->references++;
	return(1);
	}

int SSL_use_RSAPrivateKey_file(ssl, file, type)
SSL *ssl;
char *file;
int type;
	{
	int j;
	FILE *in;
	RSA *rsa;

	in=fopen(file,"r");
	if (in == NULL)
		{
		SYSerr(ERR_F_FOPEN,errno);
		SSLerr(SSL_F_SSL_USE_RSAPRIVATEKEY_FILE,ERR_R_SYS_LIB);
		return(0);
		}
	if	(type == SSL_FILETYPE_ASN1)
		{
		j=ERR_R_ASN1_LIB;
		rsa=d2i_RSAPrivateKey_fp(in,NULL);
		}
#ifdef F2I
	else if (type == SSL_FILETYPE_TEXT)
		{
		j=ERR_R_ASN1_LIB;
		i=f2i_RSAPrivateKey(in,rsa);
		}
#endif
	else if (type == SSL_FILETYPE_PEM)
		{
		j=ERR_R_PEM_LIB;
		rsa=PEM_read_RSAPrivateKey(in,NULL,NULL);
		}
	else
		{
		SSLerr(SSL_F_SSL_USE_RSAPRIVATEKEY_FILE,
			SSL_R_BAD_SSL_FILETYPE);
		fclose(in);
		return(0);
		}
	fclose(in);
	if (rsa == NULL)
		{
		SSLerr(SSL_F_SSL_USE_RSAPRIVATEKEY_FILE,j);
		return(0);
		}

	return(SSL_use_RSAPrivateKey(ssl,rsa));
	}

int SSL_use_RSAPrivateKey_ASN1(ssl,d,len)
SSL *ssl;
unsigned char *d;
long len;
	{
	unsigned char *p;
	RSA *rsa;

	p=d;
	if ((rsa=d2i_RSAPrivateKey(&rsa,&p,(long)len)) == NULL)
		{
		SSLerr(SSL_F_SSL_USE_RSAPRIVATEKEY_ASN1,ERR_R_ASN1_LIB);
		return(0);
		}

	return(SSL_use_RSAPrivateKey(ssl,rsa));
	}

#ifdef undef
int ssl_rsa_public_decrypt(c, len, from, to)
CERT *c;
int len;
unsigned char *from;
unsigned char *to;
	{
	RSA *rsa;
	int i;

	if ((c == NULL) || (c->publickey == NULL))
		{
		SSLerr(SSL_F_SSL_CERT_RSA_PUBLIC_DECRYPT,SSL_R_NO_PUBLICKEY);
		return(-1);
		}
	rsa=c->publickey;
	/* we have the public key */
#ifdef RSA_DEBUG
	SSL_TRACE(SSL_ERR,"PUBLIC RSA INPUT %d\n",len);
	for (j=0; j<len; j++) SSL_TRACE(SSL_ERR,"%02X ",from[j]);
	SSL_TRACE(SSL_ERR,"\n");
#endif
	i=RSA_public_decrypt(len,from,to,rsa);
	if (i < 0)
		{
		SSLerr(SSL_F_SSL_CERT_RSA_PUBLIC_DECRYPT,ERR_R_RSA_LIB);
		}
#ifdef RSA_DEBUG
	SSL_TRACE(SSL_ERR,"PUBLIC RSA OUTPUT %d\n",i);
	for (j=0; j<i; j++) SSL_TRACE(SSL_ERR,"%02X ",to[j]);
	SSL_TRACE(SSL_ERR,"\n");
#endif
	return(i);
	}
#endif


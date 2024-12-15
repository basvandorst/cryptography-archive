/* ssl/ssl_cert.c */
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
#include "ssl_rsa.h"

CERT *ssl_cert_new()
	{
	CERT *ret;

	ret=(CERT *)malloc(sizeof(CERT));
	if (ret == NULL)
		{
		SSLerr(SSL_F_SSL_CERT_NEW,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	ret->cert_type=0;
	ret->public_encrypt=NULL;
	ret->private_decrypt=NULL;
	ret->x509=NULL;
	ret->publickey=NULL;
	ret->privatekey=NULL;
	ret->references=1;
	return(ret);
	}

void ssl_cert_free(c)
CERT *c;
	{
	if (--c->references > 0) return;
	if (c->x509 != NULL) X509_free(c->x509);
	if (c->privatekey != NULL) RSA_free(c->privatekey);
	if (c->publickey != NULL) RSA_free(c->publickey);
	free(c);
	}

/* loads in the certificate from the server */
int ssl_set_certificate(s, type, len, data)
SSL *s;
int type;
int len;
unsigned char *data;
	{
	CERT *c;
	int i;
	X509 *x509; /* will don't want to include the headers */
	
	x509=d2i_X509(NULL,&data,(long)len);
	if (x509 == NULL)
		{
		SSLerr(SSL_F_SSL_SET_CERTIFICATE,ERR_R_X509_LIB);
		return(0);
		}

	i=X509_cert_verify(s->ctx->cert,x509,s->verify_callback);
	if ((s->verify_mode & SSL_VERIFY_PEER) && (!i))
		{
		SSLerr(SSL_F_SSL_SET_CERTIFICATE,ERR_R_X509_LIB);
		return(0);
		}

	/* cert for ssl */
	c=ssl_cert_new();
	if (c == NULL) return(-1);

	/* cert for session */
	if (s->conn->cert) ssl_cert_free(s->conn->cert);
	s->conn->cert=c;
	c->references++;

	c->cert_type=type;
	c->x509=x509;
	if (c->publickey != NULL) RSA_free(c->publickey);
	c->publickey=(RSA *)X509_extract_key(x509);
	if (c->publickey == NULL) return(0);
	if (!ssl_set_cert_type(c,SSL_CT_X509_CERTIFICATE)) return(0);
	return(1);
	}

int ssl_set_cert_type(c, type)
CERT *c;
int type;
	{
	c->cert_type=type;
	c->public_encrypt=ssl_rsa_public_encrypt;
	c->private_decrypt=ssl_rsa_private_decrypt;
	return(1);
	}

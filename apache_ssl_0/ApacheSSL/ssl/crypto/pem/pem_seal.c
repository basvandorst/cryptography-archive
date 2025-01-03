/* lib/pem/pem_seal.c */
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
#include "crypto.h"
#include "rsa.h"
#include "envelope.h"
#include "rand.h"
#include "objects.h"
#include "x509.h"
#include "pem.h"

int PEM_SealInit(ctx,type,md_type,ek,ekl,iv,pubk,npubk)
PEM_ENCODE_SEAL_CTX *ctx;
EVP_CIPHER *type;
EVP_MD *md_type;
unsigned char **ek;
int *ekl;
unsigned char *iv;
RSA **pubk;
int npubk;
	{
	unsigned char key[EVP_MAX_KEY_LENGTH];
	int ret=-1;
	int i,j=0,max=0;
	char *s;

	for (i=0; i<npubk; i++)
		{
		j=RSA_size(pubk[i]);
		if (j > max) max=i;
		}
	s=(char *)malloc(max*2);
	if (s == NULL)
		{
		PEMerr(PEM_F_PEM_SEALINIT,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	EVP_EncodeInit(&(ctx->encode));
	EVP_SignInit(&(ctx->md),md_type);

	ret=EVP_SealInit(&(ctx->cipher),type,ek,ekl,iv,pubk,npubk);
	if (!ret) goto err;

	/* base64 encode the keys */
	for (i=0; i<npubk; i++)
		{
		j=EVP_EncodeBlock((unsigned char *)s,ek[i],RSA_size(pubk[i]));
		ekl[i]=j;
		memcpy(ek[i],s,j+1);
		}

	ret=npubk;
err:
	if (s != NULL) free(s);
	memset(key,0,EVP_MAX_KEY_LENGTH);
	return(ret);
	}

void PEM_SealUpdate(ctx,out,outl,in,inl)
PEM_ENCODE_SEAL_CTX *ctx;
unsigned char *out;
int *outl;
unsigned char *in;
int inl;
	{
	unsigned char buffer[1600];
	int i,j,r;

	r=0;
	*outl=0;
	EVP_SignUpdate(&(ctx->md),in,inl);
	for (;;)
		{
		if (inl <= 0) break;
		if (inl > 1200)
			i=1200;
		else
			i=inl;
		EVP_EncryptUpdate(&(ctx->cipher),buffer,&j,in,i);
		EVP_EncodeUpdate(&(ctx->encode),out,&j,buffer,j);
		*outl+=j;
		out+=j;
		in+=i;
		inl-=i;
		}
	}

int PEM_SealFinal(ctx,sig,sigl,out,outl,priv)
PEM_ENCODE_SEAL_CTX *ctx;
unsigned char *sig;
int *sigl;
unsigned char *out;
int *outl;
RSA *priv;
	{
	unsigned char *s;
	int ret=0,j;
	unsigned int i;

	i=RSA_size(priv);
	if (i < 100) i=100;
	s=(unsigned char *)malloc(i*2);
	if (s == NULL)
		{
		PEMerr(PEM_F_PEM_SEALFINAL,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	EVP_EncryptFinal(&(ctx->cipher),s,(int *)&i);
	EVP_EncodeUpdate(&(ctx->encode),out,&j,s,i);
	*outl=j;
	out+=j;
	EVP_EncodeFinal(&(ctx->encode),out,&j);
	*outl+=j;

	if (!EVP_SignFinal(&(ctx->md),s,&i,priv)) goto err;
	*sigl=EVP_EncodeBlock(sig,s,i);
	
	ret=1;
err:
	if (s != NULL) free(s);
	return(ret);
	}

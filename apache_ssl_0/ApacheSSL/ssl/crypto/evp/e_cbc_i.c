/* lib/evp/e_cbc_i.c */
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

#ifndef NOIDEA

#include <stdio.h>
#include "crypto.h"
#include "envelope.h"
#include "objects.h"

#ifndef NOPROTO
static void idea_cbc_enc_init_key(EVP_CIPHER_CTX *ctx, unsigned char *key,
	unsigned char *iv);
static void idea_cbc_dec_init_key(EVP_CIPHER_CTX *ctx, unsigned char *key,
	unsigned char *iv);
static void idea_cbc_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
	unsigned char *in, unsigned int inl);
#else
static void idea_cbc_enc_init_key();
static void idea_cbc_dec_init_key();
static void idea_cbc_cipher();
#endif

static EVP_CIPHER cipher=
	{
	NID_idea_cbc,
	8,16,8,
	idea_cbc_enc_init_key,
	idea_cbc_dec_init_key,
	idea_cbc_cipher,
	};

EVP_CIPHER *EVP_idea_cbc= &cipher;
	
static void idea_cbc_enc_init_key(ctx,key,iv)
EVP_CIPHER_CTX *ctx;
unsigned char *key;
unsigned char *iv;
	{
	if (iv != NULL)
		memcpy(&(ctx->c.idea_cbc.oiv[0]),iv,8);
	memcpy(&(ctx->c.idea_cbc.iv[0]),&(ctx->c.idea_cbc.oiv[0]),8);
	if (key != NULL)
		idea_set_encrypt_key(key,&(ctx->c.idea_cbc.ks));
	}

static void idea_cbc_dec_init_key(ctx,key,iv)
EVP_CIPHER_CTX *ctx;
unsigned char *key;
unsigned char *iv;
	{
	if (iv != NULL)
		memcpy(&(ctx->c.idea_cbc.oiv[0]),iv,8);
	memcpy(&(ctx->c.idea_cbc.iv[0]),&(ctx->c.idea_cbc.oiv[0]),8);
	if (key != NULL)
		{
		IDEA_KEY_SCHEDULE tmp;

		if (key != NULL)
			{
			idea_set_encrypt_key(key,&tmp);
			idea_set_decrypt_key(&tmp,&(ctx->c.idea_cbc.ks));
			}
		else if (ctx->encrypt)
			{
			IDEA_KEY_SCHEDULE tmp;

			memcpy((unsigned char *)&tmp,&(ctx->c.idea_cbc.ks),
				 sizeof(IDEA_KEY_SCHEDULE));
			idea_set_decrypt_key(&tmp,&(ctx->c.idea_cbc.ks));
			}
		memset((unsigned char *)&tmp,0,sizeof(IDEA_KEY_SCHEDULE));
		}
	}

static void idea_cbc_cipher(ctx,out,in,inl)
EVP_CIPHER_CTX *ctx;
unsigned char *out;
unsigned char *in;
unsigned int inl;
	{
	idea_cbc_encrypt(
		in,out,(long)inl,
		&(ctx->c.idea_cbc.ks),&(ctx->c.idea_cbc.iv[0]),
		ctx->encrypt);
	}

#endif

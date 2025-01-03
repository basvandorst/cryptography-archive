/* lib/evp/e_ecb_3d.c */
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
#include "envelope.h"
#include "objects.h"

#ifndef NOPROTO
static void des_ede_init_key(EVP_CIPHER_CTX *ctx, unsigned char *key,
	unsigned char *iv);
static void des_ede3_init_enc_key(EVP_CIPHER_CTX *ctx, unsigned char *key,
	unsigned char *iv);
static void des_ede3_init_dec_key(EVP_CIPHER_CTX *ctx, unsigned char *key,
	unsigned char *iv);
static void des_ede_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
	unsigned char *in, unsigned int inl);
#else
static void des_ede_init_key();
static void des_ede3_init_enc_key();
static void des_ede3_init_dec_key();
static void des_ede_cipher();
#endif

static EVP_CIPHER cipher2=
	{
	NID_des_ede,
	8,16,0,
	des_ede_init_key,
	des_ede_init_key,
	des_ede_cipher,
	};

static EVP_CIPHER cipher3=
	{
	NID_des_ede3,
	8,24,0,
	des_ede3_init_enc_key,
	des_ede3_init_dec_key,
	des_ede_cipher,
	};

EVP_CIPHER *EVP_des_ede= &cipher2;
EVP_CIPHER *EVP_des_ede3= &cipher3;
	
static void des_ede_init_key(ctx,key,iv)
EVP_CIPHER_CTX *ctx;
unsigned char *key;
unsigned char *iv;
	{
	if (key != NULL)
		{
		des_set_key((des_cblock *)key,ctx->c.des_ede.ks1);
		des_set_key((des_cblock *)&(key[8]),ctx->c.des_ede.ks2);
		memcpy( (char *)ctx->c.des_ede.ks3,
			(char *)ctx->c.des_ede.ks1,
			sizeof(ctx->c.des_ede.ks1));
		}
	}

static void des_ede3_init_enc_key(ctx,key,iv)
EVP_CIPHER_CTX *ctx;
unsigned char *key;
unsigned char *iv;
	{
	if (key != NULL)
		{
		des_set_key((des_cblock *)key,ctx->c.des_ede.ks1);
		des_set_key((des_cblock *)&(key[8]),ctx->c.des_ede.ks2);
		des_set_key((des_cblock *)&(key[16]),ctx->c.des_ede.ks3);
		}
	}

static void des_ede3_init_dec_key(ctx,key,iv)
EVP_CIPHER_CTX *ctx;
unsigned char *key;
unsigned char *iv;
	{
	if (key != NULL)
		{
		des_set_key((des_cblock *)key,ctx->c.des_ede.ks3);
		des_set_key((des_cblock *)&(key[8]),ctx->c.des_ede.ks2);
		des_set_key((des_cblock *)&(key[16]),ctx->c.des_ede.ks1);
		}
	}

static void des_ede_cipher(ctx,out,in,inl)
EVP_CIPHER_CTX *ctx;
unsigned char *out;
unsigned char *in;
unsigned int inl;
	{
	int i;

	if (inl < 8) return;
	inl-=8;
	for (i=0; i<=inl; i+=8)
		{
		des_ecb3_encrypt(
			(des_cblock *)&(in[i]),(des_cblock *)&(out[i]),
			ctx->c.des_ede.ks1,
			ctx->c.des_ede.ks2,
			ctx->c.des_ede.ks3,
			ctx->encrypt);
		}
	}

/* lib/evp/e_cfb_3d.c */
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
static void des_ede_cfb_init_key(EVP_CIPHER_CTX *ctx, unsigned char *key,
	unsigned char *iv);
static void des_ede3_cfb_init_key(EVP_CIPHER_CTX *ctx, unsigned char *key,
	unsigned char *iv);
static void des_ede_cfb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
	unsigned char *in, unsigned int inl);
#else
static void des_ede_cfb_init_key();
static void des_ede3_cfb_init_key();
static void des_ede_cfb_cipher();
#endif

static EVP_CIPHER cipher2=
	{
	NID_des_ede_cfb,
	1,16,8,
	des_ede_cfb_init_key,
	des_ede_cfb_init_key,
	des_ede_cfb_cipher,
	};

static EVP_CIPHER cipher3=
	{
	NID_des_ede3_cfb,
	1,24,8,
	des_ede3_cfb_init_key,
	des_ede3_cfb_init_key,
	des_ede_cfb_cipher,
	};

EVP_CIPHER *EVP_des_ede_cfb= &cipher2;
EVP_CIPHER *EVP_des_ede3_cfb= &cipher3;
	
static void des_ede_cfb_init_key(ctx,key,iv)
EVP_CIPHER_CTX *ctx;
unsigned char *key;
unsigned char *iv;
	{
	ctx->c.des_cfb.num=0;

	if (iv != NULL)
		memcpy(&(ctx->c.des_cfb.oiv[0]),iv,8);
	memcpy(&(ctx->c.des_cfb.iv[0]),&(ctx->c.des_cfb.oiv[0]),8);
	if (key != NULL)
		{
		des_set_key((des_cblock *)key,ctx->c.des_cfb.ks);
		des_set_key((des_cblock *)&(key[8]),ctx->c.des_cfb.ks2);
		memcpy( (char *)ctx->c.des_cfb.ks3,
			(char *)ctx->c.des_cfb.ks,
			sizeof(ctx->c.des_cfb.ks));
		}
	}

static void des_ede3_cfb_init_key(ctx,key,iv)
EVP_CIPHER_CTX *ctx;
unsigned char *key;
unsigned char *iv;
	{
	ctx->c.des_cfb.num=0;

	if (iv != NULL)
		memcpy(&(ctx->c.des_cfb.oiv[0]),iv,8);
	memcpy(&(ctx->c.des_cfb.iv[0]),&(ctx->c.des_cfb.oiv[0]),8);
	if (key != NULL)
		{
		des_set_key((des_cblock *)key,ctx->c.des_cfb.ks);
		des_set_key((des_cblock *)&(key[8]),ctx->c.des_cfb.ks2);
		des_set_key((des_cblock *)&(key[16]),ctx->c.des_cfb.ks3);
		}
	}

static void des_ede_cfb_cipher(ctx,out,in,inl)
EVP_CIPHER_CTX *ctx;
unsigned char *out;
unsigned char *in;
unsigned int inl;
	{
	des_ede3_cfb64_encrypt(
		in,out,(long)inl,
		ctx->c.des_cfb.ks,
		ctx->c.des_cfb.ks2,
		ctx->c.des_cfb.ks3,
		(des_cblock *)&(ctx->c.des_cfb.iv[0]),
		&ctx->c.des_cfb.num,ctx->encrypt);
	}

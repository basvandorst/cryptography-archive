/* crypto/evp/p_seal.c */
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
#include "cryptlib.h"
#include "rand.h"
#include "rsa.h"
#include "evp.h"
#include "objects.h"
#include "x509.h"

int EVP_SealInit(ctx,type,ek,ekl,iv,pubk,npubk)
EVP_CIPHER_CTX *ctx;
EVP_CIPHER *type;
unsigned char **ek;
int *ekl;
unsigned char *iv;
EVP_PKEY **pubk;
int npubk;
	{
	unsigned char key[EVP_MAX_KEY_LENGTH];
	int i,ret=0,n;
	
	if (npubk <= 0) return(0);
	RAND_bytes(key,EVP_MAX_KEY_LENGTH);
	if (type->iv_len > 0)
		RAND_bytes(iv,type->iv_len);

	EVP_EncryptInit(ctx,type,key,iv);
	for (i=0; i<npubk; i++)
		{
		if (pubk[i]->type != EVP_PKEY_RSA)
			{
			EVPerr(EVP_F_EVP_SEALINIT,EVP_R_PUBLIC_KEY_NOT_RSA);
			goto err;
			}
		n=RSA_public_encrypt(type->key_len,key,ek[i],pubk[i]->pkey.rsa,
			RSA_PKCS1_PADDING);
		if (n <= 0) goto err;
		ekl[i]=n;
		}
	ret=npubk;
err:
	memset(key,0,EVP_MAX_KEY_LENGTH);
	return(ret);
	}

/* MACRO
void EVP_SealUpdate(ctx,out,outl,in,inl)
EVP_CIPHER_CTX *ctx;
unsigned char *out;
int *outl;
unsigned char *in;
int inl;
	{
	EVP_EncryptUpdate(ctx,out,outl,in,inl);
	}
*/

void EVP_SealFinal(ctx,out,outl)
EVP_CIPHER_CTX *ctx;
unsigned char *out;
int *outl;
	{
	EVP_EncryptFinal(ctx,out,outl);
	EVP_EncryptInit(ctx,NULL,NULL,NULL);
	}

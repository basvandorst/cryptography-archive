/* lib/rsa/rsa.h */
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

#ifndef HEADER_RSA_H
#define HEADER_RSA_H

#include "bn.h"

typedef struct rsa_st
	{
	int version;
	BIGNUM *n;
	BIGNUM *e;
	BIGNUM *d;
	BIGNUM *p;
	BIGNUM *q;
	BIGNUM *dmp1;
	BIGNUM *dmq1;
	BIGNUM *iqmp;
	int references;
	} RSA;

#define RSA_PKCS1_PADDING	11

#ifndef NOPROTO
RSA *	RSA_new(void);
int	RSA_size(RSA *);
RSA *	RSA_generate_key(int bits, unsigned long e,void
	(*callback)());
	/* next 4 return -1 on error */
int	RSA_public_encrypt(int flen, unsigned char *from,
		unsigned char *to, RSA *rsa);
int	RSA_private_encrypt(int flen, unsigned char *from,
		unsigned char *to, RSA *rsa);
int	RSA_public_decrypt(int flen, unsigned char *from, 
		unsigned char *to, RSA *rsa);
int	RSA_private_decrypt(int flen, unsigned char *from, 
		unsigned char *to, RSA *rsa);
int	RSA_mod_exp(BIGNUM *r0, BIGNUM *I, RSA *rsa);
void	RSA_free (RSA *r);
BIGNUM *RSA_generate_prime(int bits,void (*callback)());

void	ERR_load_RSA_strings(void );

RSA *	d2i_RSAPublicKey(RSA **a, unsigned char **pp, long length);
int	i2d_RSAPublicKey(RSA *a, unsigned char **pp);
RSA *	d2i_RSAPrivateKey(RSA **a, unsigned char **pp, long length);
int 	i2d_RSAPrivateKey(RSA *a, unsigned char **pp);
int	RSA_print(FILE *fp, RSA *r);

int i2d_Netscape_RSA(RSA *a, unsigned char **pp, int (*cb)());
RSA *d2i_Netscape_RSA(RSA **a, unsigned char **pp, long length, int (*cb)());

#else

RSA *	RSA_new();
int	RSA_size();
RSA *	RSA_generate_key();
	/* next 4 return -1 on error */
int	RSA_public_encrypt();
int	RSA_private_encrypt();
int	RSA_public_decrypt();
int	RSA_private_decrypt();
int	RSA_mod_exp();
void	RSA_free();
BIGNUM *RSA_generate_prime();

void	ERR_load_RSA_strings();

RSA *	d2i_RSAPublicKey();
int	i2d_RSAPublicKey();
RSA *	d2i_RSAPrivateKey();
int 	i2d_RSAPrivateKey();
int	RSA_print();

int i2d_Netscape_RSA();
RSA *d2i_Netscape_RSA();

#endif

/* BEGIN ERROR CODES */
/* Error codes for the RSA functions. */

/* Function codes. */
#define RSA_F_RSA_GENERATE_KEY				 100
#define RSA_F_RSA_NEW					 101
#define RSA_F_RSA_PRINT					 102
#define RSA_F_RSA_PRIVATE_DECRYPT			 103
#define RSA_F_RSA_PRIVATE_ENCRYPT			 104
#define RSA_F_RSA_PUBLIC_DECRYPT			 105
#define RSA_F_RSA_PUBLIC_ENCRYPT			 106

/* Reason codes. */
#define RSA_R_BAD_E_VALUE				 100
#define RSA_R_BAD_FF_HEADER				 101
#define RSA_R_BAD_PAD_BYTE_COUNT			 102
#define RSA_R_BLOCK_TYPE_IS_NOT_01			 103
#define RSA_R_BLOCK_TYPE_IS_NOT_02			 104
#define RSA_R_DATA_NOT_EQ_TO_MOD_LEN			 105
#define RSA_R_DATA_TO_LARGE_FOR_KEY_SIZE		 106
#define RSA_R_NULL_BEFORE_BLOCK_MISSING			 107

#endif

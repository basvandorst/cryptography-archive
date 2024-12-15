/* lib/bn/bn_gcd.c */
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
#include "bn.h"

#ifndef NOPROTO
static BIGNUM *euclid(BIGNUM *a, BIGNUM *b);
#else
static BIGNUM *euclid();
#endif

int BN_gcd(r,in_a,in_b,ctx)
BIGNUM *r,*in_a,*in_b;
BN_CTX *ctx;
	{
	BIGNUM *a,*b,*t;
	int ret=0;

	a=ctx->bn[ctx->tos];
	b=ctx->bn[ctx->tos+1];

	if (!BN_copy(a,in_a)) goto err;
	if (!BN_copy(b,in_b)) goto err;

	if (BN_cmp(a,b) < 0) { t=a; a=b; b=t; }
	t=euclid(a,b);
	if (t == NULL) goto err;

	r=BN_copy(r,t);
	ret=1;
err:
	return(ret);
	}

static BIGNUM *euclid(a,b)
BIGNUM *a,*b;
	{
	BIGNUM *t;
	int shifts=0;

	for (;;)
		{
		if (BN_is_zero(b))
			break;

		if ((a->d[0]&1))	/* a is odd */
			{
			if (b->d[0]&1)  /* b is odd */
				{
				if (!BN_sub(a,a,b)) goto err;
				if (!BN_rshift1(a,a)) goto err;
				if (BN_cmp(a,b) < 0)
					{ t=a; a=b; b=t; }
				}
			else		/* a odd - b even */
				{
				if (!BN_rshift1(b,b)) goto err;
				if (BN_cmp(a,b) < 0)
					{ t=a; a=b; b=t; }
				}
			}
		else			/* a is even */
			{
			if (b->d[0]&1)	/* b id odd */
				{
				if (!BN_rshift1(a,a)) goto err;
				if (BN_cmp(a,b) < 0)
					{ t=a; a=b; b=t; }
				}
			else		/* a even - b even */
				{
				if (!BN_rshift1(a,a)) goto err;
				if (!BN_rshift1(b,b)) goto err;
				shifts++;
				}
			}
		}
	if (shifts)
		{
		if (!BN_lshift(a,a,shifts)) goto err;
		}
	return(a);
err:
	return(NULL);
	}

/* solves ax == 1 (mod n) */
BIGNUM *BN_mod_inverse(a, n, ctx)
BIGNUM *a;
BIGNUM *n;
BN_CTX *ctx;
	{
	BIGNUM *A,*B,*X,*Y,*M,*D,*R;
	BIGNUM *ret=NULL,*T;
	int sign;

	A=ctx->bn[ctx->tos];
	B=ctx->bn[ctx->tos+1];
	X=ctx->bn[ctx->tos+2];
	D=ctx->bn[ctx->tos+3];
	M=ctx->bn[ctx->tos+4];
	Y=ctx->bn[ctx->tos+5];
	ctx->tos+=6;
	R=BN_new();
	if (R == NULL) goto err;

	BN_zero(X);
	BN_one(Y);
	BN_copy(A,a);
	BN_copy(B,n);
	sign=1;

	while (!BN_is_zero(B))
		{
		if (!BN_div(D,M,A,B,ctx)) goto err;
		T=A;
		A=B;
		B=M;
		/* T has a struct, M does not */

		if (!BN_mul(T,D,X)) goto err;
		if (!BN_add(T,T,Y)) goto err;
		M=Y;
		Y=X;
		X=T;
		sign= -sign;
		}
	if (sign < 0)
		{
		if (!BN_sub(Y,n,Y)) goto err;
		}

	if (BN_is_one(A))
		{ if (!BN_mod(R,Y,n,ctx)) goto err; }
	else
		{
		BNerr(BN_F_BN_MOD_INVERSE,BN_R_NO_INVERSE);
		goto err;
		}
	ret=R;
err:
	if ((ret == NULL) && (R != NULL)) BN_free(R);
	ctx->tos-=6;
	return(ret);
	}


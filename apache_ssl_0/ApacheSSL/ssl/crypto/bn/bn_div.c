/* lib/bn/bn_div.c */
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

int BN_div(dv, rem, m, d,ctx)
BIGNUM *dv;
BIGNUM *rem;
BIGNUM *m;
BIGNUM *d;
BN_CTX *ctx;
	{
	int i,nm,nd;
	BIGNUM *D;

	if (BN_is_zero(d))
		{
		BNerr(BN_F_BN_DIV,BN_R_DIV_BY_ZERO);
		return(0);
		}

	if (BN_cmp(m,d) < 0)
		{
		if (rem != NULL)
			{ if (BN_copy(rem,m) == NULL) return(0); }
		if (dv != NULL) BN_zero(dv);
		return(1);
		}

	D=ctx->bn[ctx->tos];
	if (dv == NULL) dv=ctx->bn[ctx->tos+1];
	if (rem == NULL) rem=ctx->bn[ctx->tos+2];

	nd=BN_num_bits(d);
	nm=BN_num_bits(m);
	if (BN_copy(D,d) == NULL) return(0);
	if (BN_copy(rem,m) == NULL) return(0);

	/* The next 2 are needed so we can do a dv->d[0]|=1 later
	 * since BN_lshift1 will only work once there is a value :-) */
	BN_zero(dv);
	dv->top=1;

	if (!BN_lshift(D,D,nm-nd)) return(0);
	for (i=nm-nd; i>=0; i--)
		{
		if (!BN_lshift1(dv,dv)) return(0);
		if (BN_cmp(rem,D) >= 0)
			{
			dv->d[0]|=1;
			if (!BN_sub(rem,rem,D)) return(0);
			}
/* CAN IMPROVE */
		if (!BN_rshift1(D,D)) return(0);
		}
	dv->neg=m->neg^d->neg;
	return(1);
	}


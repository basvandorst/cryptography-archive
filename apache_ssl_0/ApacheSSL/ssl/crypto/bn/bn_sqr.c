/* lib/bn/bn_sqr.c */
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
#include "bn_lcl.h"

#define			LBITS(a)	((a)&BN_MASK2l)
#define			HBITS(a)	(((a)>>BN_BITS4)&BN_MASK2l)
#define			L2HBITS(a)	(((a)&BN_MASK2l)<<BN_BITS4)

/* r must not be a */
int BN_sqr(r, a, ctx)
BIGNUM *r;
BIGNUM *a;
BN_CTX *ctx;
	{
	int i,j,max,al;
	BIGNUM *tmp;
	BN_ULONG *ap,*rp;

	tmp=ctx->bn[ctx->tos];

	al=a->top;
	if (al == 0)
		{
		r->top=0;
		return(1);
		}

	max=(al*2+1);
	BN_clear(r);
	BN_clear(tmp);
	if (bn_expand(r,(max+1)*BN_BITS2) == NULL) return(0);
	if (bn_expand(tmp,(max+1)*BN_BITS2) == NULL) return(0);
	r->top=max;
	r->neg=a->neg;
	ap=a->d;
	rp=r->d;

	rp++;
	j=al;
	for (i=1; i<al; i++)
		{
		BN_ULONG r;

		j--;
		r= *(ap++);
		rp[j]+=bn_mul_add_word(rp,ap,j,r);
		rp+=2;
		}
	bn_fix_top(r);
	if (!BN_lshift1(r,r)) return(0);

	bn_sqr_words(tmp->d,a->d,a->top);
	tmp->top=al*2+1;
	bn_fix_top(tmp);
	if (!BN_add(r,r,tmp)) return(0);
	return(1);
	}

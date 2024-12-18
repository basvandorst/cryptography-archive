/* lib/dh/dh_check.c */
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
#include "dh.h"

/* Check that p is a strong prime and
 * if g is 2, 3 or 5, check that is is a suitable generator
 * where
 * for 2, p mod 24 == 11
 * for 3, p mod 12 == 5
 * for 5, p mod 10 == 3 or 7
 * should hold.
 */

int DH_check(dh,ret)
DH *dh;
int *ret;
	{
	int ok=0;
	BN_CTX *ctx=NULL;
	BN_ULONG l;
	BIGNUM *q=NULL;

	*ret=0;
	ctx=BN_CTX_new();
	if (ctx == NULL) goto err;
	q=BN_new();
	if (q == NULL) goto err;

	if (BN_is_word(dh->g,DH_GENERATOR_2))
		{
		l=BN_mod_word(dh->p,24);
		if (l != 11) *ret|=DH_NOT_SUITABLE_GENERATOR;
		}
/*	else if (BN_is_word(dh->g,DH_GENERATOR_3))
		{
		l=BN_mod_word(dh->p,12);
		if (l != 5) *ret|=DH_NOT_SUITABLE_GENERATOR;
		}*/
	else if (BN_is_word(dh->g,DH_GENERATOR_5))
		{
		l=BN_mod_word(dh->p,10);
		if ((l != 3) && (l != 7))
			*ret|=DH_NOT_SUITABLE_GENERATOR;
		}
	else
		*ret|=DH_UNABLE_TO_CHECK_GENERATOR;

	if (!BN_is_prime(dh->p,BN_prime_checks,NULL,ctx))
		*ret|=DH_CHECK_P_NOT_PRIME;
	else
		{
		if (!BN_rshift1(q,dh->p)) goto err;
		if (!BN_is_prime(q,BN_prime_checks,NULL,ctx))
			*ret|=DH_CHECK_P_NOT_STRONG_PRIME;
		}
	ok=1;
err:
	if (ctx != NULL) BN_CTX_free(ctx);
	if (q != NULL) BN_free(q);
	return(ok);
	}

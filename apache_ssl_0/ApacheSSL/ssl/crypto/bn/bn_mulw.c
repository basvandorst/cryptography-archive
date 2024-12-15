/* lib/bn/bn_mulw.c */
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

#ifdef BN_LLONG 

BN_ULONG bn_mul_add_word(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG c1=0;

	for (;;)
		{
		mul_add(rp[0],ap[0],w,c1);
		if (--num == 0) break;
		mul_add(rp[1],ap[1],w,c1);
		if (--num == 0) break;
		mul_add(rp[2],ap[2],w,c1);
		if (--num == 0) break;
		mul_add(rp[3],ap[3],w,c1);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}
	return(c1);
	} 

BN_ULONG bn_mul_word(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG c1=0;

	for (;;)
		{
		mul(rp[0],ap[0],w,c1);
		if (--num == 0) break;
		mul(rp[1],ap[1],w,c1);
		if (--num == 0) break;
		mul(rp[2],ap[2],w,c1);
		if (--num == 0) break;
		mul(rp[3],ap[3],w,c1);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}
	return(c1);
	} 

void bn_sqr_words(r,a,n)
BN_ULONG *r,*a;
int n;
        {
	BN_ULONG w;

	for (;;)
		{
		BN_ULLONG t;

		w=a[0];
		t=(BN_ULLONG)(w)*(w);
		r[0]=L(t); r[1]=H(t);
		if (--n == 0) break;

		w=a[1];
		t=(BN_ULLONG)(w)*(w);
		r[2]=L(t); r[3]=H(t);
		if (--n == 0) break;

		w=a[2];
		t=(BN_ULLONG)(w)*(w);
		r[4]=L(t); r[5]=H(t);
		if (--n == 0) break;

		w=a[3];
		t=(BN_ULLONG)(w)*(w);
		r[6]=L(t); r[7]=H(t);
		if (--n == 0) break;

		a+=4;
		r+=8;
		}
	}


#else

BN_ULONG bn_mul_add_word(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG c=0;
	BN_ULONG bl,bh;

	bl=LBITS(w);
	bh=HBITS(w);

	for (;;)
		{
		mul_add(rp[0],ap[0],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[1],ap[1],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[2],ap[2],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[3],ap[3],bl,bh,c);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}
	return(c);
	} 

BN_ULONG bn_mul_word(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG c=0;
	BN_ULONG bl,bh;

	bl=LBITS(w);
	bh=HBITS(w);

	for (;;)
		{
		mul(rp[0],ap[0],bl,bh,c);
		if (--num) break;
		mul(rp[0],ap[0],bl,bh,c);
		if (--num) break;
		mul(rp[0],ap[0],bl,bh,c);
		if (--num) break;
		mul(rp[0],ap[0],bl,bh,c);
		if (--num) break;
		ap+=4;
		rp+=4;
		}
	return(c);
	} 

void bn_sqr_words(r,a,n)
BN_ULONG *r,*a;
int n;
        {
	BN_ULONG w;

	for (;;)
		{
		sqr64(r[0],r[1],a[0]);
		if (--n == 0) break;

		sqr64(r[2],r[3],a[1]);
		if (--n == 0) break;

		sqr64(r[4],r[5],a[2]);
		if (--n == 0) break;

		sqr64(r[6],r[7],a[3]);
		if (--n == 0) break;

		a+=4;
		r+=8;
		}
	}

#endif


/* lib/idea/i_ecb.c */
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

#include "idea.h"
#include "idea_lcl.h"

char *IDEA_version="\0IDEA part of SSLeay v 0.5.1 21/12/95";

void idea_ecb_encrypt(in, out, ks)
unsigned char *in;
unsigned char *out;
IDEA_KEY_SCHEDULE *ks;
	{
	unsigned long l0,l1,d[2];

	n2l(in,l0); d[0]=l0;
	n2l(in,l1); d[1]=l1;
	idea_encrypt(d,ks);
	l0=d[0]; l2n(l0,out);
	l1=d[1]; l2n(l1,out);
	l0=l1=d[0]=d[1]=0;
	}

void idea_encrypt(d,key)
unsigned long *d;
IDEA_KEY_SCHEDULE *key;
	{
	int i;
	register IDEA_INT *p;
	register unsigned long x1,x2,x3,x4,t0,t1,ul;

	x2=d[0];
	x1=(x2>>16);
	x4=d[1];
	x3=(x4>>16);

	p=&(key->data[0][0]);
	for (i=0; i<8; i++)
		{
		x1&=0xffff;
		idea_mul(x1,x1,*p,ul); p++;

		x2+= *(p++);
		x3+= *(p++);

		x4&=0xffff;
		idea_mul(x4,x4,*p,ul); p++;

		t0=(x1^x3)&0xffff;
		idea_mul(t0,t0,*p,ul); p++;

		t1=(t0+(x2^x4))&0xffff;
		idea_mul(t1,t1,*p,ul); p++;

		t0+=t1;

		x1^=t1;
		x4^=t0;
		ul=x2^t0;		/* do the swap to x3 */
		x2=x3^t1;
		x3=ul;
		}

	x1&=0xffff;
	idea_mul(x1,x1,*p,ul); p++;

	t0= x3+ *(p++);
	t1= x2+ *(p++);

	x4&=0xffff;
	idea_mul(x4,x4,*p,ul);

	d[0]=(t0&0xffff)|((x1&0xffff)<<16);
	d[1]=(x4&0xffff)|((t1&0xffff)<<16);
	}

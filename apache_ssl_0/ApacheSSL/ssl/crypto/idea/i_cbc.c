/* lib/idea/i_cbc.c */
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

void idea_cbc_encrypt(in, out, length, ks, iv, encrypt)
unsigned char *in;
unsigned char *out;
long length;
IDEA_KEY_SCHEDULE *ks;
unsigned char *iv;
int encrypt;
	{
	register unsigned long tin0,tin1;
	register unsigned long tout0,tout1,xor0,xor1;
	register long l=length;
	unsigned long tin[2];

	if (encrypt)
		{
		n2l(iv,tout0);
		n2l(iv,tout1);
		iv-=8;
		for (; l>0; l-=8)
			{
			if (l >= 8)
				{
				n2l(in,tin0);
				n2l(in,tin1);
				}
			else
				n2ln(in,tin0,tin1,l);
			tin0^=tout0;
			tin1^=tout1;
			tin[0]=tin0;
			tin[1]=tin1;
			idea_encrypt(tin,ks);
			tout0=tin[0]; l2n(tout0,out);
			tout1=tin[1]; l2n(tout1,out);
			}
		l2n(tout0,iv);
		l2n(tout1,iv);
		}
	else
		{
		n2l(iv,xor0);
		n2l(iv,xor1);
		iv-=8;
		for (; l>0; l-=8)
			{
			n2l(in,tin0); tin[0]=tin0;
			n2l(in,tin1); tin[1]=tin1;
			idea_encrypt(tin,ks);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			if (l >= 8)
				{
				l2n(tout0,out);
				l2n(tout1,out);
				}
			else
				l2nn(tout0,tout1,out,l);
			xor0=tin0;
			xor1=tin1;
			}
		l2n(xor0,iv);
		l2n(xor1,iv);
		}
	tin0=tin1=tout0=tout1=xor0=xor1=0;
	tin[0]=tin[1]=0;
	}


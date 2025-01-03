/* lib/des/cfb64ede.c */
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

#include "des_locl.h"

/* The input and output encrypted as though 64bit cfb mode is being
 * used.  The extra state information to record how much of the
 * 64bit block we have used is contained in *num;
 */

void des_ede3_cfb64_encrypt(in, out, length, ks1,ks2,ks3, ivec, num, encrypt)
unsigned char *in;
unsigned char *out;
long length;
des_key_schedule ks1,ks2,ks3;
des_cblock (*ivec);
int *num;
int encrypt;
	{
	register unsigned long v0,v1;
	register long l=length,n=*num;
	unsigned long ti[2];
	unsigned char *iv,c,cc;

	iv=(unsigned char *)ivec;
	if (encrypt)
		{
		while (l--)
			{
			if (n == 0)
				{
				c2l(iv,v0);
				c2l(iv,v1);

				IP(v0,v1);

				ti[0]=v0;
				ti[1]=v1;
				des_encrypt2((unsigned long *)ti,ks1,DES_ENCRYPT);
				des_encrypt2((unsigned long *)ti,ks2,DES_DECRYPT);
				des_encrypt2((unsigned long *)ti,ks3,DES_ENCRYPT);
				v0=ti[0];
				v1=ti[1];

				FP(v1,v0);

				iv=(unsigned char *)ivec;
				l2c(v0,iv);
				l2c(v1,iv);
				iv=(unsigned char *)ivec;
				}
			c= *(in++)^iv[n];
			*(out++)=c;
			iv[n]=c;
			n=(n+1)&0x07;
			}
		}
	else
		{
		while (l--)
			{
			if (n == 0)
				{
				c2l(iv,v0);
				c2l(iv,v1);

				IP(v0,v1);

				ti[0]=v0;
				ti[1]=v1;
				des_encrypt2((unsigned long *)ti,ks1,DES_ENCRYPT);
				des_encrypt2((unsigned long *)ti,ks2,DES_DECRYPT);
				des_encrypt2((unsigned long *)ti,ks3,DES_ENCRYPT);

				v0=ti[0];
				v1=ti[1];

				FP(v1,v0);

				iv=(unsigned char *)ivec;
				l2c(v0,iv);
				l2c(v1,iv);
				iv=(unsigned char *)ivec;
				}
			cc= *(in++);
			c=iv[n];
			iv[n]=cc;
			*(out++)=c^cc;
			n=(n+1)&0x07;
			}
		}
	v0=v1=ti[0]=ti[1]=c=cc=0;
	*num=n;
	}

#ifdef undef /* MACRO */
void des_ede2_cfb64_encrypt(in, out, length, ks1,ks2, ivec, num, encrypt)
unsigned char *in;
unsigned char *out;
long length;
des_key_schedule ks1,ks2;
des_cblock (*ivec);
int *num;
int encrypt;
	{
	des_ede3_cfb64_encrypt(in,out,length,ks1,ks2,ks1,ivec,num,encrypt);
	}
#endif

/* crypto/des/des_enc.c */
/* Copyright (C) 1995-1997 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@mincom.oz.au).
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
 *     Eric Young (eay@mincom.oz.au)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an
acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@mincom.oz.au)"
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

#define register
#include "des_locl.h"
#include "spr.h"
#include "arch.h"

void des_encrypt(DES_LONG *data,des_key_schedule ks,int encrypt)
	{
	register DES_LONG l,r,t,u;
#ifdef DES_PTR
	register unsigned char *des_SP=(unsigned char *)des_SPtrans;
#endif
#ifndef DES_UNROLL
	register int i;
#endif
	register DES_LONG *s;

	r=data[0];
	l=data[1];

	IP(r,l);
	/* Things have been modified so that the initial rotate is
	 * done outside the loop.  This required the
	 * des_SPtrans values in sp.h to be rotated 1 bit to the right.
	 * One perl script later and things have a 5% speed up on a sparc2.
	 * Thanks to Richard Outerbridge <71755.204@CompuServe.COM>
	 * for pointing this out. */
	/* clear the top bits on machines with 8byte longs */
	/* shift left by 2 */
	r=ROTATE(r,29)&0xffffffffL;
	l=ROTATE(l,29)&0xffffffffL;

	s=(DES_LONG *)ks;
	/* I don't know if it is worth the effort of loop unrolling the
	 * inner loop */
	if (encrypt)
		{
#ifdef DES_UNROLL
		D_ENCRYPT(l,r, 0); /*  1 */
		D_ENCRYPT(r,l, 2); /*  2 */
		D_ENCRYPT(l,r, 4); /*  3 */
		D_ENCRYPT(r,l, 6); /*  4 */
		D_ENCRYPT(l,r, 8); /*  5 */
		D_ENCRYPT(r,l,10); /*  6 */
		D_ENCRYPT(l,r,12); /*  7 */
		D_ENCRYPT(r,l,14); /*  8 */
		D_ENCRYPT(l,r,16); /*  9 */
		D_ENCRYPT(r,l,18); /*  10 */
		D_ENCRYPT(l,r,20); /*  11 */
		D_ENCRYPT(r,l,22); /*  12 */
		D_ENCRYPT(l,r,24); /*  13 */
		D_ENCRYPT(r,l,26); /*  14 */
		D_ENCRYPT(l,r,28); /*  15 */
		D_ENCRYPT(r,l,30); /*  16 */
#else
		for (i=0; i<32; i+=8)
			{
			D_ENCRYPT(l,r,i+0); /*  1 */
			D_ENCRYPT(r,l,i+2); /*  2 */
			D_ENCRYPT(l,r,i+4); /*  3 */
			D_ENCRYPT(r,l,i+6); /*  4 */
			}
#endif
		}
	else
		{
#ifdef DES_UNROLL
		D_ENCRYPT(l,r,30); /* 16 */
		D_ENCRYPT(r,l,28); /* 15 */
		D_ENCRYPT(l,r,26); /* 14 */
		D_ENCRYPT(r,l,24); /* 13 */
		D_ENCRYPT(l,r,22); /* 12 */
		D_ENCRYPT(r,l,20); /* 11 */
		D_ENCRYPT(l,r,18); /* 10 */
		D_ENCRYPT(r,l,16); /*  9 */
		D_ENCRYPT(l,r,14); /*  8 */
		D_ENCRYPT(r,l,12); /*  7 */
		D_ENCRYPT(l,r,10); /*  6 */
		D_ENCRYPT(r,l, 8); /*  5 */
		D_ENCRYPT(l,r, 6); /*  4 */
		D_ENCRYPT(r,l, 4); /*  3 */
		D_ENCRYPT(l,r, 2); /*  2 */
		D_ENCRYPT(r,l, 0); /*  1 */
#else
		for (i=30; i>0; i-=8)
			{
			D_ENCRYPT(l,r,i-0); /* 16 */
			D_ENCRYPT(r,l,i-2); /* 15 */
			D_ENCRYPT(l,r,i-4); /* 14 */
			D_ENCRYPT(r,l,i-6); /* 13 */
			}
#endif
		}

	/* rotate and clear the top bits on machines with 8byte longs */
	l=ROTATE(l,3)&0xffffffffL;
	r=ROTATE(r,3)&0xffffffffL;

	FP(r,l);
	data[0]=l;
	data[1]=r;
	l=r=t=u=0;
	}

void des_encrypt2(DES_LONG *data,des_key_schedule ks,int encrypt)
	{
	register DES_LONG l,r,t,u;
#ifdef DES_PTR
	register unsigned char *des_SP=(unsigned char *)des_SPtrans;
#endif
#ifndef DES_UNROLL
	register int i;
#endif
	register DES_LONG *s;

	r=data[0];
	l=data[1];

	/* Things have been modified so that the initial rotate is
	 * done outside the loop.  This required the
	 * des_SPtrans values in sp.h to be rotated 1 bit to the right.
	 * One perl script later and things have a 5% speed up on a sparc2.
	 * Thanks to Richard Outerbridge <71755.204@CompuServe.COM>
	 * for pointing this out. */
	/* clear the top bits on machines with 8byte longs */
	r=ROTATE(r,29);
	l=ROTATE(l,29);

	s=(DES_LONG *)ks;
	/* I don't know if it is worth the effort of loop unrolling the
	 * inner loop */
	if (encrypt)
		{
#ifdef DES_UNROLL
		D_ENCRYPT(l,r, 0); /*  1 */
		D_ENCRYPT(r,l, 2); /*  2 */
		D_ENCRYPT(l,r, 4); /*  3 */
		D_ENCRYPT(r,l, 6); /*  4 */
		D_ENCRYPT(l,r, 8); /*  5 */
		D_ENCRYPT(r,l,10); /*  6 */
		D_ENCRYPT(l,r,12); /*  7 */
		D_ENCRYPT(r,l,14); /*  8 */
		D_ENCRYPT(l,r,16); /*  9 */
		D_ENCRYPT(r,l,18); /*  10 */
		D_ENCRYPT(l,r,20); /*  11 */
		D_ENCRYPT(r,l,22); /*  12 */
		D_ENCRYPT(l,r,24); /*  13 */
		D_ENCRYPT(r,l,26); /*  14 */
		D_ENCRYPT(l,r,28); /*  15 */
		D_ENCRYPT(r,l,30); /*  16 */
#else
		for (i=0; i<32; i+=8)
			{
			D_ENCRYPT(l,r,i+0); /*  1 */
			D_ENCRYPT(r,l,i+2); /*  2 */
			D_ENCRYPT(l,r,i+4); /*  3 */
			D_ENCRYPT(r,l,i+6); /*  4 */
			}
#endif
		}
	else
		{
#ifdef DES_UNROLL
		D_ENCRYPT(l,r,30); /* 16 */
		D_ENCRYPT(r,l,28); /* 15 */
		D_ENCRYPT(l,r,26); /* 14 */
		D_ENCRYPT(r,l,24); /* 13 */
		D_ENCRYPT(l,r,22); /* 12 */
		D_ENCRYPT(r,l,20); /* 11 */
		D_ENCRYPT(l,r,18); /* 10 */
		D_ENCRYPT(r,l,16); /*  9 */
		D_ENCRYPT(l,r,14); /*  8 */
		D_ENCRYPT(r,l,12); /*  7 */
		D_ENCRYPT(l,r,10); /*  6 */
		D_ENCRYPT(r,l, 8); /*  5 */
		D_ENCRYPT(l,r, 6); /*  4 */
		D_ENCRYPT(r,l, 4); /*  3 */
		D_ENCRYPT(l,r, 2); /*  2 */
		D_ENCRYPT(r,l, 0); /*  1 */
#else
		for (i=30; i>0; i-=8)
			{
			D_ENCRYPT(l,r,i-0); /* 16 */
			D_ENCRYPT(r,l,i-2); /* 15 */
			D_ENCRYPT(l,r,i-4); /* 14 */
			D_ENCRYPT(r,l,i-6); /* 13 */
			}
#endif
		}
	/* rotate and clear the top bits on machines with 8byte longs */
	data[0]=ROTATE(l,3);
	data[1]=ROTATE(r,3);
	l=r=t=u=0;
	}

void des_encrypt3(DES_LONG *data,des_key_schedule ks1,des_key_schedule
ks2,des_key_schedule ks3)
	{
	register DES_LONG l,r;

	l=data[0];
	r=data[1];
	IP(l,r);
	data[0]=l;
	data[1]=r;
	des_encrypt2((DES_LONG *)data,ks1,DES_ENCRYPT);
	des_encrypt2((DES_LONG *)data,ks2,DES_DECRYPT);
	des_encrypt2((DES_LONG *)data,ks3,DES_ENCRYPT);
	l=data[0];
	r=data[1];
	FP(r,l);
	data[0]=l;
	data[1]=r;
	}

void des_decrypt3(DES_LONG *data,des_key_schedule ks1,des_key_schedule
ks2,des_key_schedule ks3)
	{
	register DES_LONG l,r;

	l=data[0];
	r=data[1];
	IP(l,r);
	data[0]=l;
	data[1]=r;
	des_encrypt2((DES_LONG *)data,ks3,DES_DECRYPT);
	des_encrypt2((DES_LONG *)data,ks2,DES_ENCRYPT);
	des_encrypt2((DES_LONG *)data,ks1,DES_DECRYPT);
	l=data[0];
	r=data[1];
	FP(r,l);
	data[0]=l;
	data[1]=r;
	}


void des_cbc_encrypt8(des_cblock (*input), des_cblock
(*output),des_key_schedule schedule, des_cblock (*ivec))
	{
	register DES_LONG tin0,tin1;
	register DES_LONG tout0,tout1,xor0,xor1;
	register unsigned char *in,*out;
	register long l=8;
	DES_LONG tin[2];
	unsigned char *iv;

	in=(unsigned char *)input;
	out=(unsigned char *)output;
	iv=(unsigned char *)ivec;

	c2l(iv,xor0);
	c2l(iv,xor1);
		{
		c2l(in,tin0); tin[0]=tin0;
		c2l(in,tin1); tin[1]=tin1;
		des_encrypt((DES_LONG *)tin,schedule,DES_DECRYPT);
		tout0=tin[0]^xor0;
		tout1=tin[1]^xor1;
		l2c(tout0,out);
		l2c(tout1,out);
		xor0=tin0;
		xor1=tin1;
		}
	tin0=tin1=tout0=tout1=xor0=xor1=0;
	tin[0]=tin[1]=0;
	}

int checkkey(des_key_schedule ks, unsigned char *cipher, unsigned char
*iv,unsigned char *known, int len)
{
  unsigned char result[256];
  int i;

    des_cbc_encrypt8((des_cblock *)cipher,(des_cblock
*)result,ks,(des_cblock *)iv);

  	if (result[0] != 'T') return 0;
  	if (result[1] != 'h') return 0;
  	if (result[2] != 'e') return 0;
  	if (result[3] != ' ') return 0;
    for(i=0;i<len;i++)
    {
  	  if (result[i] != known[i]) return 0;
     }
  return 1;
}


void crack_key0(des_cblock (*input),des_cblock (*output),des_cblock (*ivec),DES_LONG tin[2],DES_LONG tcmp[2]) {
  register DES_LONG tin0,tin1;
  register DES_LONG tout0,tout1,xor0,xor1;
  register unsigned char *in,*out;
  register DES_LONG l,r,t,u;
  register unsigned char *des_SP=(unsigned char *)des_SPtrans;
  register DES_LONG *s;
  unsigned char *iv;
  
  in=(unsigned char *)input;
  out=(unsigned char *)output;
  iv=(unsigned char *)ivec;
  
  c2l(iv,xor0);
  c2l(iv,xor1);
  c2l(in,tin0);
  c2l(in,tin1);
  c2l(out,tout0);
  c2l(out,tout1);
  tout0^=xor0;
  tout1^=xor1;
  IP(tout0,tout1);
  tout1=ROTATE(tout1,29);
  tout0=ROTATE(tout0,29);
  tcmp[0]=tout0;
  tcmp[1]=tout1;
  
  
  IP(tin0,tin1);
  /* Things have been modified so that the initial rotate is
   * done outside the loop.  This required the
   * des_SPtrans values in sp.h to be rotated 1 bit to the right.
   * One perl script later and things have a 5% speed up on a sparc2.
   * Thanks to Richard Outerbridge <71755.204@CompuServe.COM>
   * for pointing this out. */
  /* clear the top bits on machines with 8byte longs */
  /* shift left by 2 */
  tin1=ROTATE(tin1,29);
  tin0=ROTATE(tin0,29);
  
  tin[0]=tin0;
  tin[1]=tin1;
}

void crack_key1(des_key_schedule schedule,DES_LONG tin[2],DES_LONG tout[2]) {
  register DES_LONG tin0,tin1;
  register DES_LONG l,r,t,u;
  register unsigned char *des_SP=(unsigned char *)des_SPtrans;
  register DES_LONG *s;

  tin1=tin[1];
  tin0=tin[0];
  s=(DES_LONG *)schedule;
  D_ENCRYPT(tin1,tin0,30); /* 16 */
  tout[0]=tin0;
  tout[1]=tin1;
  
}

#ifndef USE_ASM
void crack_key15(des_key_schedule schedule,DES_LONG tin[2],DES_LONG tout[2]) {
  /* register DES_LONG tin0,tin1; */
  register DES_LONG l,r,t,u;
  register unsigned char *des_SP=(unsigned char *)des_SPtrans;
  register DES_LONG *s= (DES_LONG *) schedule;
#if defined(INTEL) || defined(SUN4U)
  register unsigned int u1,u2,u3;
#endif 

  r=tin[0];
  l=tin[1];
  
#if defined(INTEL) || defined(SUN4U)
  t = l ^s[  28  +1];
  t=(((t)>>(4))|((t)<<(32-(4))));
  u2=(int)t>>8L;
  u1=(int)t&0xfc;
  u2&=0xfc;
  t>>=16L;
  r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
  r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
  u3=(int)t>>8L;
  u1=(int)t&0xfc;
  u3&=0xfc;
  r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
  r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);
#else 
  t=l^s[28+1];
  t=(((t)>>(4))|((t)<<(32-(4))));
  r^= *(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc));
  r^= *(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc));
  r^= *(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc));
  r^= *(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
#endif
  
  tout[0]=r;
  tout[1]=l;
}
#endif /* USE_ASM */

void crack_key16(des_key_schedule schedule,DES_LONG tin[2],DES_LONG tout[2],DES_LONG load[2]) {
  register DES_LONG tin0,tin1;
  register DES_LONG l,r,t,u;
  register unsigned char *des_SP=(unsigned char *)des_SPtrans;
  register DES_LONG *s= (DES_LONG *) schedule;
#if defined(INTEL) || defined(SUN4U)
  register unsigned int u1,u2,u3;
#endif 

  tin1=tin[1];
  tin0=tin[0];

#if defined(INTEL) || defined(SUN4U)
  u =  tin0  ^s[  30    ];
  t =  tin0  ^s[  30  +1];
  u2=(int)u>>8L;
  u1=(int)u&0xfc;
  u2&=0xfc;
  t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
  u>>=16L;
  tin1 ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
  tin1 ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
  u3=(int)(u>>8L);
  u1=(int)u&0xfc;
  u3&=0xfc;
  tin1 ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
  tin1 ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
  u2=(int)t>>8L;
  u1=(int)t&0xfc;
  u2&=0xfc;
  t>>=16L;
  tin1 ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
  tin1 ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
  u3=(int)t>>8L;
  u1=(int)t&0xfc;
  u3&=0xfc;
  tin1 ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
  tin1 ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);
#else
  D_ENCRYPT(tin1,tin0,30);
#endif

  r=load[0]=tin0;
  l=load[1]=tin1;  

#if defined(INTEL) || defined(SUN4U)
  t = l ^s[  28  +1];
  t=(((t)>>(4))|((t)<<(32-(4))));
  u2=(int)t>>8L;
  u1=(int)t&0xfc;
  u2&=0xfc;
  t>>=16L;
  r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
  r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
  u3=(int)t>>8L;
  u1=(int)t&0xfc;
  u3&=0xfc;
  r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
  r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);
#else /* Power-PC mm */
  t=l^s[28+1];
  t=(((t)>>(4))|((t)<<(32-(4))));
  r^= *(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc));
  r^= *(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc));
  r^= *(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc));
  r^= *(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
#endif 

  tout[0]=r;
  tout[1]=l;
}

#ifndef USE_ASM
int crack_key25(DES_LONG tin[2], DES_LONG tcmp, des_key_schedule schedule) {
	register DES_LONG l,r,t,u;
	register unsigned char *des_SP=(unsigned char *)des_SPtrans;
	register DES_LONG *s= (DES_LONG *) schedule;
#if defined(INTEL) || defined(SUN4U)
	register unsigned int u1,u2,u3;
#endif
#if defined(ALPHA) || defined(MIPS) || defined(m68k)
	unsigned int u1,u2,s1,s2;
#endif
#ifdef GENERIC
	register DES_LONG u2,t2;
#endif
#if defined(SUN4M) || defined(PARISC) || defined(SUN4C)
	unsigned int u1,u2,u3;
#endif

	r=tin[0];
	l=tin[1];
		
	/*	
        D_ENCRYPT(r,l,28);
	D_ENCRYPT(l,r,26);
	D_ENCRYPT(r,l,24);
	D_ENCRYPT(l,r,22);
	D_ENCRYPT(r,l,20);
	D_ENCRYPT(l,r,18);
	D_ENCRYPT(r,l,16);
	D_ENCRYPT(l,r,14);
	D_ENCRYPT(r,l,12);
	D_ENCRYPT(l,r,10);
	D_ENCRYPT(r,l, 8);
	D_ENCRYPT(l,r, 6);
	D_ENCRYPT(r,l, 4);
	D_ENCRYPT(l,r, 2);
	D_ENCRYPT(r,l, 0); 
	*/

#if defined(INTEL) || defined(SUN4U)

       	u =  l  ^s[  28    ];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	u>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);

#ifdef DEBUG2
	printf("r:%08x l:%08x\n",r,l);
#endif

	/* Round 14 */  
	u =  r  ^s[  26    ];
	t =  r  ^s[  26  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);

	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	t>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);


	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

#ifdef DEBUG2
	printf("r:%08x l:%08x\n",r,l);
#endif

	/* Round 13 */  
	u =  l  ^s[  24    ];
	t =  l  ^s[  24  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	t>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 12 */
	u =  r  ^s[  22    ];
	t =  r  ^s[  22  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc; t>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 11 */
	u =  l  ^s[  20    ];
	t =  l  ^s[  20  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc; t>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 10 */
	u =  r  ^s[  18    ];
	t =  r  ^s[  18  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	t>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3); 

	/* Round 9 */
	u =  l  ^s[  16    ];
	t =  l  ^s[  16  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	t>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 8 */
	u =  r  ^s[  14    ];
	t =  r  ^s[  14  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	t>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 7 */
	u =  l  ^s[  12    ];
	t =  l  ^s[  12  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc; t>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 6 */
	u =  r  ^s[  10    ];
	t =  r  ^s[  10  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	t>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 5 */
	u =  l  ^s[   8    ];
	t =  l  ^s[   8  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc; t>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 4 */
	u =  r  ^s[   6    ];
	t =  r  ^s[   6  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc; t>>=16L; 
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 3 */
	u =  l  ^s[   4    ];
	t =  l  ^s[   4  +1];
	l^= tcmp; /* added */
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	t>>=16L;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);

	/* Round 2 with early termination */
	u =  r  ^s[   2    ];
		u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	u>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	if (l & 0x02080802L) return 0; /* added */
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	if (l & (0x04040101L | 0x02080802L)) return 0; /* added */
	u3=(int)(u>>8L);
	u1=(int)u&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+u3);
	
	t =  r  ^s[   2  +1];
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc; t>>=16L;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	u3=(int)t>>8L;
	u1=(int)t&0xfc;
	u3&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+u3);
#endif  /* Intel */

#if defined(SUN4M) || defined(PARISC) || defined(SUN4C)

	/* (Half) Round 15 */
	u  =   l   ^s[   28     ]; 
	r ^= des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ 
	des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]; 

	/* All the rounds... */
	/* Round 14 */
	u  =   r   ^s[   26     ];
	t  =   r   ^s[   26   +1];
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	l ^= des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ 
	des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ 
	des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ 
	des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];

	/* Round 13 */
	u  =   l   ^s[   24     ];
	t  =   l   ^s[   24   +1];
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	r ^= des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ 
	des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ 
	des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ 
	des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f]; 

	/* Round 12 */
	u  =   r   ^s[   22     ];
	t  =   r   ^s[   22   +1];
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	l ^= des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ 
	des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ 
	des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ 
	des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f]; 

	/* Round 11 */
	u  =   l   ^s[   20     ];
	t  =   l   ^s[   20   +1];
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	r ^= des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ 
	des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ 
	des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ 
	des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];

	/* Round 10 */
	u  =   r   ^s[   18     ];
	t  =   r   ^s[   18   +1]; 
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	l ^= des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ 
	des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ 
	des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ 
	des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f]; 


	u  =   l   ^s[   16     ];   t  =   l   ^s[   16   +1]  ; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  r ^=	des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];
 
  	
	u  =   r   ^s[   14     ];   t  =   r   ^s[   14   +1]  ; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  l ^=	des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];
  	
	u  =   l   ^s[   12     ];   t  =   l   ^s[   12   +1]  ; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  r ^=	des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];

	u  =   r   ^s[   10     ];   t  =   r   ^s[   10   +1]  ; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  l ^=	des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];

	u  =   l   ^s[    8     ];   t  =   l   ^s[    8   +1]  ; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  r ^=	des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];

	u  =   r   ^s[    6     ];   t  =   r   ^s[    6   +1]  ; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  l ^=	des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];

	/* Round 3 */
	u  =   l   ^s[    4     ];
	t  =   l   ^s[    4   +1];
	l ^= tcmp;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	r ^= des_SPtrans[0][(u>> 2L)&0x3f]^ des_SPtrans[2][(u>>10L)&0x3f]^ 
	des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f]^ 
	des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ 
	des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];

	

	/* Round 2 with early termination */
	u = r   ^s[    2     ];
	l ^= des_SPtrans[0][(u>> 2L)&0x3f];
	if (l & 0x02080802L) return 0;
	l ^= des_SPtrans[2][(u>>10L)&0x3f];
	if (l & (0x04040101L | 0x02080802L)) return 0;
	l ^= des_SPtrans[4][(u>>18L)&0x3f]^ des_SPtrans[6][(u>>26L)&0x3f];

	t = r^s[    2   +1];
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
 	l ^= des_SPtrans[1][(t>> 2L)&0x3f]^ des_SPtrans[3][(t>>10L)&0x3f]^ 
	des_SPtrans[5][(t>>18L)&0x3f]^ des_SPtrans[7][(t>>26L)&0x3f];

	
#endif /* SUNm and PA-RISC and SUN4c*/

#if defined(ALPHA) || defined(MIPS) || defined(m68k)
	u =  l  ^s[  28    ];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	s1=(int)(u>>16L);
	s2=(int)(u>>24L);
	s1&=0xfc;
	s2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2);
	
	/* Round 14 */
	u =  r  ^s[  26    ];
	t =  r  ^s[  26  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	s1=(int)(u>>16L);
	s2=(int)(u>>24L);
	s1&=0xfc; s2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	s1=(int)(t>>16L);
	s2=(int)(t>>24L);
	s1&=0xfc;
	s2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 13 */
	u =  l  ^s[  24    ];
	t =  l  ^s[  24  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	s1=(int)(u>>16L);
	s2=(int)(u>>24L);
	s1&=0xfc;
	s2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	s1=(int)(t>>16L);
	s2=(int)(t>>24L);
	s1&=0xfc;
	s2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 12 */
	u =  r  ^s[  22    ];
	t =  r  ^s[  22  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	s1=(int)(u>>16L);
	s2=(int)(u>>24L);
	s1&=0xfc;
	s2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	s1=(int)(t>>16L);
	s2=(int)(t>>24L);
	s1&=0xfc;
	s2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2); 

	/* Round 11 */
	u =  l  ^s[  20    ];
	t =  l  ^s[  20  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	s1=(int)(u>>16L);
	s2=(int)(u>>24L);
	s1&=0xfc;
	s2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	s1=(int)(t>>16L);
	s2=(int)(t>>24L);
	s1&=0xfc;
	s2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 10 */
	u =  r  ^s[  18    ];
	t =  r  ^s[  18  +1];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	s1=(int)(u>>16L);
	s2=(int)(u>>24L);
	s1&=0xfc;
	s2&=0xfc; 
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	s1=(int)(t>>16L);
	s2=(int)(t>>24L);
	s1&=0xfc;
	s2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 9 */
	u =  l  ^s[  16    ];  t =  l  ^s[  16  +1] ; u2=(int)u>>8L; u1=(int)u&0xfc; u2&=0xfc; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2); s1=(int)(u>>16L); s2=(int)(u>>24L); s1&=0xfc; s2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2); u2=(int)t>>8L; u1=(int)t&0xfc; u2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2); s1=(int)(t>>16L); s2=(int)(t>>24L); s1&=0xfc; s2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2); 

	/* Round 8 */
	u =  r  ^s[  14    ];  t =  r  ^s[  14  +1] ; u2=(int)u>>8L; u1=(int)u&0xfc; u2&=0xfc; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2); s1=(int)(u>>16L); s2=(int)(u>>24L); s1&=0xfc; s2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2); u2=(int)t>>8L; u1=(int)t&0xfc; u2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2); s1=(int)(t>>16L); s2=(int)(t>>24L); s1&=0xfc; s2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 7 */  
	u =  l  ^s[  12    ];  t =  l  ^s[  12  +1] ; u2=(int)u>>8L; u1=(int)u&0xfc; u2&=0xfc; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2); s1=(int)(u>>16L); s2=(int)(u>>24L); s1&=0xfc; s2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2); u2=(int)t>>8L; u1=(int)t&0xfc; u2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2); s1=(int)(t>>16L); s2=(int)(t>>24L); s1&=0xfc; s2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 6 */
	u =  r  ^s[  10    ];  t =  r  ^s[  10  +1] ; u2=(int)u>>8L; u1=(int)u&0xfc; u2&=0xfc; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2); s1=(int)(u>>16L); s2=(int)(u>>24L); s1&=0xfc; s2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2); u2=(int)t>>8L; u1=(int)t&0xfc; u2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2); s1=(int)(t>>16L); s2=(int)(t>>24L); s1&=0xfc; s2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 5 */
	u =  l  ^s[   8    ];  t =  l  ^s[   8  +1] ; u2=(int)u>>8L; u1=(int)u&0xfc; u2&=0xfc; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2); s1=(int)(u>>16L); s2=(int)(u>>24L); s1&=0xfc; s2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2); u2=(int)t>>8L; u1=(int)t&0xfc; u2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2); s1=(int)(t>>16L); s2=(int)(t>>24L); s1&=0xfc; s2&=0xfc;  r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);  r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 4 */ 
	u =  r  ^s[   6    ];  t =  r  ^s[   6  +1] ; u2=(int)u>>8L; u1=(int)u&0xfc; u2&=0xfc; t= ((( t )>>( 4 ))+(( t )<<(32-( 4 )))) ;  l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2); s1=(int)(u>>16L); s2=(int)(u>>24L); s1&=0xfc; s2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2); u2=(int)t>>8L; u1=(int)t&0xfc; u2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2); s1=(int)(t>>16L); s2=(int)(t>>24L); s1&=0xfc; s2&=0xfc;  l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);  l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);

	/* Round 3 */  
	u =  l  ^s[   4    ];
	t =  l  ^s[   4  +1];
	l^= tcmp; /* added */
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	r ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	s1=(int)(u>>16L);
	s2=(int)(u>>24L);
	s1&=0xfc; s2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2);
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	s1=(int)(t>>16L);
	s2=(int)(t>>24L);
	s1&=0xfc;
	s2&=0xfc;
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);
	r ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2); 
  
  	/* Round 2 with early termination */
	u =  r  ^s[   2    ];
	u2=(int)u>>8L;
	u1=(int)u&0xfc;
	u2&=0xfc;
	t= ((( t )>>( 4 ))+(( t )<<(32-( 4 ))));
	l ^= *(UINT4  *)((unsigned char *)des_SP      +u1);
	if (l & 0x02080802L) return 0; /* from Pierce */
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x200+u2);
	if (l & (0x04040101L | 0x02080802L)) return 0; /* from Pierce */
	s1=(int)(u>>16L);
	s2=(int)(u>>24L);
	s1&=0xfc;
	s2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x400+s1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x600+s2);
	
	t =  r  ^s[   2  +1];
 	t=(((t)>>(4))+((t)<<(32-(4))));
	u2=(int)t>>8L;
	u1=(int)t&0xfc;
	u2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x100+u1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x300+u2);
	s1=(int)(t>>16L);
	s2=(int)(t>>24L);
	s1&=0xfc; s2&=0xfc;
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x500+s1);
	l ^= *(UINT4  *)((unsigned char *)des_SP+0x700+s2);
#endif /* Alpha or MIPS or m68k */

#ifdef GENERIC
	u=l^s[28 ];
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x200+((u>>8L)&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc));

	u=r^s[26 ]; t=r^s[26+1];
	t=(((t)>>(4))|((t)<<(32-(4))));
	l^=*(UINT4 *)((unsigned char *)des_SP +((u )&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x200+((u>>8L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));

	u=l^s[24 ]; t=l^s[24+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));

	u=r^s[22 ]; t=r^s[22+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[20 ]; t=l^s[20+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=r^s[18 ]; t=r^s[18+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[16 ]; t=l^s[16+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=r^s[14 ]; t=r^s[14+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[12 ]; t=l^s[12+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
    
	u=r^s[10 ]; t=r^s[10+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[8 ]; t=l^s[8+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));

	u=r^s[6 ]; t=r^s[6+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))^
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[4 ]; t=l^s[4+1];
	l^= tcmp;
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));

	u=r^s[2 ];
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc));
	if (l & 0x02080802L) return 0;
	l ^= *(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc));
	if (l & (0x04040101L | 0x02080802L)) return 0;
	l ^= *(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc)) ;
	l ^= *(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc));

 	t=r^s[2+1];
 	t=(((t)>>(4))+((t)<<(32-(4))));
	l ^=
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
#endif /* GENERIC */

	/*
	l=ROTATE(l,3);
	r=ROTATE(r,3);
	FP(r,l);
	*/

	if (l ) return 0;
	return 1;
}
#endif /* USE_ASM */

#if 0
int crack_key2(DES_LONG tin[2], DES_LONG tcmp[2], des_key_schedule schedule) {
	register DES_LONG l,r,t,u,u2,t2;
	register unsigned char *des_SP=(unsigned char *)des_SPtrans;
	register DES_LONG *s= (DES_LONG *) schedule;

	r=tin[0];
	l=tin[1];

#if 0
		D_ENCRYPT(r,l,28); /* 15 */
		D_ENCRYPT(l,r,26); /* 14 */
		D_ENCRYPT(r,l,24); /* 13 */
		D_ENCRYPT(l,r,22); /* 12 */
		D_ENCRYPT(r,l,20); /* 11 */
		D_ENCRYPT(l,r,18); /* 10 */
		D_ENCRYPT(r,l,16); /*  9 */
		D_ENCRYPT(l,r,14); /*  8 */
		D_ENCRYPT(r,l,12); /*  7 */
		D_ENCRYPT(l,r,10); /*  6 */
		D_ENCRYPT(r,l, 8); /*  5 */
		D_ENCRYPT(l,r, 6); /*  4 */
		D_ENCRYPT(r,l, 4); /*  3 */
		D_ENCRYPT(l,r, 2); /*  2 */
		//D_ENCRYPT(r,l, 0); /*  1 */
#else
	u=l^s[28 ]; t=l^s[28+1];
	t=(((t)>>(4))|((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x200+((u>>8L)&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc));
	r^= *(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));

	u=r^s[26 ]; t=r^s[26+1];
	t=(((t)>>(4))|((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x200+((u>>8L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc));
	l^=*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));

	u=l^s[24 ]; t=l^s[24+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));

	u=r^s[22 ]; t=r^s[22+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[20 ]; t=l^s[20+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=r^s[18 ]; t=r^s[18+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[16 ]; t=l^s[16+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=r^s[14 ]; t=r^s[14+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[12 ]; t=l^s[12+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
    
	u=r^s[10 ]; t=r^s[10+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[8 ]; t=l^s[8+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=r^s[6 ]; t=r^s[6+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))^
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=l^s[4 ]; t=l^s[4+1];
    t=(((t)>>(4))+((t)<<(32-(4))));
	r^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
	
	u=r^s[2 ]; t=r^s[2+1];
	t=(((t)>>(4))+((t)<<(32-(4))));
	l^= *(UINT4 *)((unsigned char *)des_SP +((u )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x200+((u>> 8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x400+((u>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x600+((u>>24L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x100+((t )&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x300+((t>>8L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x500+((t>>16L)&0xfc))|
	*(UINT4 *)((unsigned char *)des_SP+0x700+((t>>24L)&0xfc));
#endif

	/* l=ROTATE(l,3); */
	/* r=ROTATE(r,3); */

	/* FP(r,l); */
	if (l != tcmp) return 0;
	return 1;
}

#endif

/* lib/sha/sha_dgst.c */
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
#include "sha.h"
#include "sha_locl.h"

char *SHA_version="\0SHA part of SSLeay v 0.5.1 21/12/95";

/* Implemented from SHA document - The Secure Hash Algorithm
 */

#define INIT_DATA_h0 (unsigned long)0x67452301L
#define INIT_DATA_h1 (unsigned long)0xefcdab89L
#define INIT_DATA_h2 (unsigned long)0x98badcfeL
#define INIT_DATA_h3 (unsigned long)0x10325476L
#define INIT_DATA_h4 (unsigned long)0xc3d2e1f0L

#define K_00_19	0x5a827999L
#define K_20_39 0x6ed9eba1L
#define K_40_59 0x8f1bbcdcL
#define K_60_79 0xca62c1d6L

#ifndef NOPROTO
static void do_block(SHA_CTX *c, register unsigned long *p);
#else
static void do_block();
#endif

unsigned char *SHA(d, n, md)
unsigned char *d;
unsigned long n;
unsigned char *md;
	{
	SHA_CTX c;
	static unsigned char m[SHA_DIGEST_LENGTH];

	if (md == NULL) md=m;
	SHA_Init(&c);
	SHA_Update(&c,d,n);
	SHA_Final(md,&c);
	return(md);
	}

void SHA_Init(c)
SHA_CTX *c;
	{
	c->h0=INIT_DATA_h0;
	c->h1=INIT_DATA_h1;
	c->h2=INIT_DATA_h2;
	c->h3=INIT_DATA_h3;
	c->h4=INIT_DATA_h4;
	c->Nl=0;
	c->Nh=0;
	c->num=0;
	}

void SHA_Update(c, data, len)
SHA_CTX *c;
register unsigned char *data;
unsigned long len;
	{
	register ULONG *p;
	int ew,ec,sw,sc;
	ULONG l;

	if (len == 0) return;

	l=(c->Nl+(len<<3))&0xffffffff;
	if (l < c->Nl) /* overflow */
		c->Nh++;
	c->Nh+=(len>>29);
	c->Nl=l;

	if (c->num != 0)
		{
		p=c->data;
		sw=c->num>>2;
		sc=c->num&0x03;

		if ((c->num+len) >= SHA_CBLOCK)
			{
			l= p[sw];
			p_c2nl(data,l,sc);
			p[sw++]=l;
			for (; sw<SHA_LBLOCK; sw++)
				{
				c2nl(data,l);
				p[sw]=l;
				}
			len-=(SHA_CBLOCK-c->num);

			do_block(c,p);
			c->num=0;
			/* drop through and do the rest */
			}
		else
			{
			int ew,ec;

			c->num+=len;
			if ((sc+len) < 4) /* ugly, add char's to a word */
				{
				l= p[sw];
				p_c2nl_p(data,l,sc,len);
				p[sw]=l;
				}
			else
				{
				ew=(c->num>>2);
				ec=(c->num&0x03);
				l= p[sw];
				p_c2nl(data,l,sc);
				p[sw++]=l;
				for (; sw < ew; sw++)
					{ c2nl(data,l); p[sw]=l; }
				if (ec)
					{
					c2nl_p(data,l,ec);
					p[sw]=l;
					}
				}
			return;
			}
		}
	/* we now can process the input data in blocks of SHA_CBLOCK
	 * chars and save the leftovers to c->data. */
	p=c->data;
	while (len >= SHA_CBLOCK)
		{
		for (sw=(SHA_BLOCK/4); sw; sw--)
			{
			c2nl(data,l); *(p++)=l;
			c2nl(data,l); *(p++)=l;
			c2nl(data,l); *(p++)=l;
			c2nl(data,l); *(p++)=l;
			}
		p=c->data;
		do_block(c,p);
		len-=SHA_CBLOCK;
		}
	ew=(len>>2);
	ec=(len&0x03);

	for (sw=0; sw < ew; sw++)
		{ c2nl(data,l); p[sw]=l; }
	c2nl_p(data,l,ec);
	p[sw]=l;
	c->num=len;
	}

static void do_block(c, X)
SHA_CTX *c;
register unsigned long *X;
	{
	register ULONG A,B,C,D,E,T;

	A=c->h0;
	B=c->h1;
	C=c->h2;
	D=c->h3;
	E=c->h4;

	BODY_00_15( 0,A,B,C,D,E,T);
	BODY_00_15( 1,T,A,B,C,D,E);
	BODY_00_15( 2,E,T,A,B,C,D);
	BODY_00_15( 3,D,E,T,A,B,C);
	BODY_00_15( 4,C,D,E,T,A,B);
	BODY_00_15( 5,B,C,D,E,T,A);
	BODY_00_15( 6,A,B,C,D,E,T);
	BODY_00_15( 7,T,A,B,C,D,E);
	BODY_00_15( 8,E,T,A,B,C,D);
	BODY_00_15( 9,D,E,T,A,B,C);
	BODY_00_15(10,C,D,E,T,A,B);
	BODY_00_15(11,B,C,D,E,T,A);
	BODY_00_15(12,A,B,C,D,E,T);
	BODY_00_15(13,T,A,B,C,D,E);
	BODY_00_15(14,E,T,A,B,C,D);
	BODY_00_15(15,D,E,T,A,B,C);
	BODY_16_19(16,C,D,E,T,A,B);
	BODY_16_19(17,B,C,D,E,T,A);
	BODY_16_19(18,A,B,C,D,E,T);
	BODY_16_19(19,T,A,B,C,D,E);

	BODY_20_39(20,E,T,A,B,C,D);
	BODY_20_39(21,D,E,T,A,B,C);
	BODY_20_39(22,C,D,E,T,A,B);
	BODY_20_39(23,B,C,D,E,T,A);
	BODY_20_39(24,A,B,C,D,E,T);
	BODY_20_39(25,T,A,B,C,D,E);
	BODY_20_39(26,E,T,A,B,C,D);
	BODY_20_39(27,D,E,T,A,B,C);
	BODY_20_39(28,C,D,E,T,A,B);
	BODY_20_39(29,B,C,D,E,T,A);
	BODY_20_39(30,A,B,C,D,E,T);
	BODY_20_39(31,T,A,B,C,D,E);
	BODY_20_39(32,E,T,A,B,C,D);
	BODY_20_39(33,D,E,T,A,B,C);
	BODY_20_39(34,C,D,E,T,A,B);
	BODY_20_39(35,B,C,D,E,T,A);
	BODY_20_39(36,A,B,C,D,E,T);
	BODY_20_39(37,T,A,B,C,D,E);
	BODY_20_39(38,E,T,A,B,C,D);
	BODY_20_39(39,D,E,T,A,B,C);

	BODY_40_59(40,C,D,E,T,A,B);
	BODY_40_59(41,B,C,D,E,T,A);
	BODY_40_59(42,A,B,C,D,E,T);
	BODY_40_59(43,T,A,B,C,D,E);
	BODY_40_59(44,E,T,A,B,C,D);
	BODY_40_59(45,D,E,T,A,B,C);
	BODY_40_59(46,C,D,E,T,A,B);
	BODY_40_59(47,B,C,D,E,T,A);
	BODY_40_59(48,A,B,C,D,E,T);
	BODY_40_59(49,T,A,B,C,D,E);
	BODY_40_59(50,E,T,A,B,C,D);
	BODY_40_59(51,D,E,T,A,B,C);
	BODY_40_59(52,C,D,E,T,A,B);
	BODY_40_59(53,B,C,D,E,T,A);
	BODY_40_59(54,A,B,C,D,E,T);
	BODY_40_59(55,T,A,B,C,D,E);
	BODY_40_59(56,E,T,A,B,C,D);
	BODY_40_59(57,D,E,T,A,B,C);
	BODY_40_59(58,C,D,E,T,A,B);
	BODY_40_59(59,B,C,D,E,T,A);

	BODY_60_79(60,A,B,C,D,E,T);
	BODY_60_79(61,T,A,B,C,D,E);
	BODY_60_79(62,E,T,A,B,C,D);
	BODY_60_79(63,D,E,T,A,B,C);
	BODY_60_79(64,C,D,E,T,A,B);
	BODY_60_79(65,B,C,D,E,T,A);
	BODY_60_79(66,A,B,C,D,E,T);
	BODY_60_79(67,T,A,B,C,D,E);
	BODY_60_79(68,E,T,A,B,C,D);
	BODY_60_79(69,D,E,T,A,B,C);
	BODY_60_79(70,C,D,E,T,A,B);
	BODY_60_79(71,B,C,D,E,T,A);
	BODY_60_79(72,A,B,C,D,E,T);
	BODY_60_79(73,T,A,B,C,D,E);
	BODY_60_79(74,E,T,A,B,C,D);
	BODY_60_79(75,D,E,T,A,B,C);
	BODY_60_79(76,C,D,E,T,A,B);
	BODY_60_79(77,B,C,D,E,T,A);
	BODY_60_79(78,A,B,C,D,E,T);
	BODY_60_79(79,T,A,B,C,D,E);

	c->h0=(c->h0+E)&0xffffffff; 
	c->h1=(c->h1+T)&0xffffffff;
	c->h2=(c->h2+A)&0xffffffff;
	c->h3=(c->h3+B)&0xffffffff;
	c->h4=(c->h4+C)&0xffffffff;
	}

void SHA_Final(md, c)
unsigned char *md;
SHA_CTX *c;
	{
	register int i,j;
	register ULONG l;
	register ULONG *p;
	static unsigned char end[4]={0x80,0x00,0x00,0x00};
	unsigned char *cp=end;

	/* c->num should definitly have room for at least one more byte. */
	p=c->data;
	j=c->num;
	i=j>>2;
	l=p[i];
	p_c2nl(cp,l,j&0x03);
	p[i]=l;
	i++;
	/* i is the next 'undefined word' */
	if (c->num >= SHA_LAST_BLOCK)
		{
		for (; i<SHA_LBLOCK; i++)
			p[i]=0;
		do_block(c,p);
		i=0;
		}
	for (; i<(SHA_LBLOCK-2); i++)
		p[i]=0;
	p[SHA_LBLOCK-2]=c->Nh;
	p[SHA_LBLOCK-1]=c->Nl;
	do_block(c,p);
	cp=md;
	l=c->h0; nl2c(l,cp);
	l=c->h1; nl2c(l,cp);
	l=c->h2; nl2c(l,cp);
	l=c->h3; nl2c(l,cp);
	l=c->h4; nl2c(l,cp);

	/* clear stuff, do_block may be leaving some stuff on the stack
	 * but I'm not worried :-) */
	c->num=0;
/*	memset((char *)&c,0,sizeof(c));*/
	}

#ifdef undef
int printit(l)
unsigned long *l;
	{
	int i,ii;

	for (i=0; i<2; i++)
		{
		for (ii=0; ii<8; ii++)
			{
			fprintf(stderr,"%08lx ",l[i*8+ii]);
			}
		fprintf(stderr,"\n");
		}
	}
#endif

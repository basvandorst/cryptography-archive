/* lib/asn1/f_int.c */
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
#include "x509.h"

#ifndef NOPROTO
static int xfgets(char *buf,int size,FILE *fp);
#else
static int xfgets();
#endif

int i2f_ASN1_INTEGER(fp, a)
FILE *fp;
ASN1_INTEGER *a;
	{
	int i,n=0;
	static char *h="0123456789ABCDEF";

	if (a->length == 0)
		{
		Fprintf(fp,"00");
		n=2;
		}
	else
		{
		for (i=0; i<a->length; i++)
			{
			if ((i != 0) && (i%35 == 0))
					Fprintf(fp,"\\\n");
			Fputc((h[((unsigned char)a->data[i]>>4)&0x0f]),fp);
			Fputc((h[((unsigned char)a->data[i]   )&0x0f]),fp);
			n+=2;
			}
		}
	return(n);
	}

int f2i_ASN1_INTEGER(fp,bs,buf,size)
FILE *fp;
ASN1_INTEGER *bs;
char *buf;
int size;
	{
	int i,j,k,m,n,again,bufsize;
	unsigned char *s=NULL;
	unsigned char *bufp;
	int num=0,slen=0,first=1;

	bs->type=V_ASN1_INTEGER;

	bufsize=xfgets(buf,size,fp);
	for (;;)
		{
		if (bufsize < 3) return(0);
		i=strlen(buf);
		if (buf[i-1] == '\n')
			buf[--i]='\0';
		again=(buf[i-1] == '\\');
		bufp=(unsigned char *)buf;
		if (first)
			{
			first=0;
			if ((bufp[0] == '0') && (buf[1] == '0'))
				{
				bufp+=2;
				i-=2;
				}
			}
		k=0;
		i-=again;
		if (i%2 != 0)
			{
			ASN1err(ASN1_F_F2I_ASN1_INTEGER,ASN1_R_ODD_NUMBER_OF_CHARS);
			return(0);
			}
		i/=2;
		if (num+i > slen)
			{
			if (s == NULL)
				s=(unsigned char *)malloc(
					(unsigned int)num+i*2);
			else
				s=(unsigned char *)realloc(s,
					(unsigned int)num+i*2);
			if (s == NULL)
				{
				ASN1err(ASN1_F_F2I_ASN1_INTEGER,ERR_R_MALLOC_FAILURE);
				return(0);
				}
			slen=num+i*2;
			}
		for (j=0; j<i; j++,k+=2)
			{
			for (n=0; n<2; n++)
				{
				m=bufp[k+n];
				if ((m >= '0') && (m <= '9'))
					m-='0';
				else if ((m >= 'a') && (m <= 'f'))
					m=m-'a'+10;
				else if ((m >= 'A') && (m <= 'F'))
					m=m-'A'+10;
				else
					{
					ASN1err(ASN1_F_F2I_ASN1_INTEGER,ASN1_R_NON_HEX_CHARACTERS);
					return(0);
					}
				s[num+j]<<=4;
				s[num+j]|=m;
				}
			}
		num+=i;
		if (again)
			bufsize=xfgets(buf,size,fp);
		else
			break;
		}
	bs->length=num;
	bs->data=s;
	return(1);
	}

static int xfgets(buf, size, fp)
char *buf;
int size;
FILE *fp;
	{
	int ret;

	buf[0]='\0';
	Fgets(buf,size,fp);
	if (buf[0] == '\0') return(0);
	ret=strlen(buf);
	return(ret);
	}


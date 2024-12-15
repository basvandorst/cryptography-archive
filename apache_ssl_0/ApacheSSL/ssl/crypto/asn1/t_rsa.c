/* lib/asn1/t_rsa.c */
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
#include "rsa.h"

#ifndef NOPROTO
static void print(FILE *fp,char *str,BIGNUM *num,unsigned char *buf);
#else
static void print();
#endif

int RSA_print(fp,x)
FILE *fp;
RSA *x;
	{
	unsigned char *m=NULL;
	int i,ret=0;

	i=RSA_size(x);
	m=(unsigned char *)malloc((unsigned int)i+10);
	if (m == NULL)
		{
		RSAerr(RSA_F_RSA_PRINT,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	Fprintf(fp,"Private-Key: (%d bit)\n",BN_num_bits(x->n));
	print(fp,"modulus:",x->n,m);
	print(fp,"publicExponent:",x->e,m);
	print(fp,"privateExponent:",x->d,m);
	print(fp,"prime1:",x->p,m);
	print(fp,"prime2:",x->q,m);
	print(fp,"exponent1:",x->dmp1,m);
	print(fp,"exponent2:",x->dmq1,m);
	print(fp,"coefficient:",x->iqmp,m);
	ret=1;
err:
	if (m != NULL) free(m);
	return(ret);
	}

static void print(fp,str,num,buf)
FILE *fp;
char *str;
BIGNUM *num;
unsigned char *buf;
	{
	int n,i;
	char *neg;

	if (BN_num_bytes(num) <= BN_BYTES)
		{
		neg=(num->neg)?"-":"";
		Fprintf(fp,"%4s%s %s%lu (%s0x%lx)\n"," ",str,neg,num->d[0],
			neg,num->d[0]);
		}
	else
		{
		neg=(num->neg)?" (Negative)":"";
		buf[0]=0;
		Fprintf(fp,"%4s%s%s"," ",str,neg);
		n=BN_bn2bin(num,&buf[1]);
	
		if (buf[1] & 0x80)
			n++;
		else	buf++;

		for (i=0; i<n; i++)
			{
			if ((i%15) == 0) Fprintf(fp,"\n%8s","");
			Fprintf(fp,"%02x%s",buf[i],((i+1) == n)?"":":");
			}
		Fprintf(fp,"\n");
		}
	return;
	}

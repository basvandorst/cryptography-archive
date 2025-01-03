/* lib/bn/bntest.c */
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
#include "bn.h"
#include "rand.h"
#include "x509.h"
#include "err.h"

#ifndef NOPROTO
int test_add (FILE *fp);
int test_sub (FILE *fp);
int test_lshift1 (FILE *fp);
int test_lshift (FILE *fp);
int test_rshift1 (FILE *fp);
int test_rshift (FILE *fp);
int test_div (FILE *fp,BN_CTX *ctx);
int test_mul (FILE *fp);
int test_sqr (FILE *fp,BN_CTX *ctx);
int test_mont (FILE *fp,BN_CTX *ctx);
int test_mod (FILE *fp,BN_CTX *ctx);
int test_mod_mul (FILE *fp,BN_CTX *ctx);
int test_mod_exp (FILE *fp,BN_CTX *ctx);
#else
int test_add ();
int test_sub ();
int test_lshift1 ();
int test_lshift ();
int test_rshift1 ();
int test_rshift ();
int test_div ();
int test_mul ();
int test_sqr ();
int test_mont ();
int test_mod ();
int test_mod_mul ();
int test_mod_exp ();
#endif

static int results=0;

int main(argc,argv)
int argc;
char *argv[];
	{
	BN_CTX *ctx;

	argc--;
	argv++;
	while (argc >= 1)
		{
		if (strcmp(*argv,"-results") == 0)
			results=1;
		argc--;
		argv++;
		}

	if (!results)
		fprintf(stdout,"obase=16\nibase=16\n");

	ctx=BN_CTX_new();
	if (ctx == NULL) exit(1);

	fprintf(stderr,"test BN_add\n");
	if (!test_add(stdout)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_sub\n");
	if (!test_sub(stdout)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_lshift1\n");
	if (!test_lshift1(stdout)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_lshift\n");
	if (!test_lshift(stdout)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_rshift1\n");
	if (!test_rshift1(stdout)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_rshift\n");
	if (!test_rshift(stdout)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_div\n");
	if (!test_div(stdout,ctx)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_mod\n");
	if (!test_mod(stdout,ctx)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_mul\n");
	if (!test_mul(stdout)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_sqr\n");
	if (!test_sqr(stdout,ctx)) goto err;
	fflush(stdout);

/*
	fprintf(stderr,"test BN_mont\n");
	if (!test_mont(stdout,ctx)) goto err;
	fflush(stdout);
*/

	fprintf(stderr,"test BN_mol_mul\n");
	if (!test_mod_mul(stdout,ctx)) goto err;
	fflush(stdout);

	fprintf(stderr,"test BN_mod_exp\n");
	if (!test_mod_exp(stdout,ctx)) goto err;
	fflush(stdout);
/**/
	exit(0);
err:
	ERR_load_crypto_strings();
	ERR_print_errors(stderr);
	exit(1);
#ifdef LINT
	return(0);
#endif
	}

int test_add(fp)
FILE *fp;
	{
	BIGNUM *a,*b,*c;
	int i;
	int j;

	a=BN_new();
	b=BN_new();
	c=BN_new();

	BN_rand(a,512,0,0);
	for (i=0; i<100; i++)
		{
		BN_rand(b,450+i,0,0);
		if (fp == NULL)
			for (j=0; j<10000; j++)
				BN_add(c,a,b);
		BN_add(c,a,b);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," + ");
				BN_print(fp,b);
				fprintf(fp," - ");
				}
			BN_print(fp,c);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	return(1);
	}

int test_sub(fp)
FILE *fp;
	{
	BIGNUM *a,*b,*c;
	int i;
	int j;

	a=BN_new();
	b=BN_new();
	c=BN_new();

	BN_rand(a,512,0,0);
	for (i=0; i<100; i++)
		{
		BN_rand(b,400+i,0,0);
		if (fp == NULL)
			for (j=0; j<10000; j++)
				BN_sub(c,a,b);
		BN_sub(c,a,b);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," - ");
				BN_print(fp,b);
				fprintf(fp," - ");
				}
			BN_print(fp,c);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	return(1);
	}

int test_div(fp,ctx)
FILE *fp;
BN_CTX *ctx;
	{
	BIGNUM *a,*b,*c,*d;
	int i;
	int j;

	a=BN_new();
	b=BN_new();
	c=BN_new();
	d=BN_new();

	BN_rand(a,400,0,0);
	for (i=0; i<100; i++)
		{
		BN_rand(b,50+i,0,0);
		if (fp == NULL)
			for (j=0; j<100; j++)
				BN_div(d,c,a,b,ctx);
		BN_div(d,c,a,b,ctx);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," / ");
				BN_print(fp,b);
				fprintf(fp," - ");
				}
			BN_print(fp,d);
			fprintf(fp,"\n");

			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," %% ");
				BN_print(fp,b);
				fprintf(fp," - ");
				}
			BN_print(fp,c);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	BN_free(d);
	return(1);
	}

int test_mul(fp)
FILE *fp;
	{
	BIGNUM *a,*b,*c;
	int i;
	int j;

	a=BN_new();
	b=BN_new();
	c=BN_new();

	BN_rand(a,200,0,0);
	for (i=0; i<100; i++)
		{
		BN_rand(b,250+i,0,0);
		if (fp == NULL)
			for (j=0; j<100; j++)
				BN_mul(c,a,b);
		BN_mul(c,a,b);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," * ");
				BN_print(fp,b);
				fprintf(fp," - ");
				}
			BN_print(fp,c);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	return(1);
	}

int test_sqr(fp,ctx)
FILE *fp;
BN_CTX *ctx;
	{
	BIGNUM *a,*c;
	int i;
	int j;

	a=BN_new();
	c=BN_new();

	for (i=0; i<40; i++)
		{
		BN_rand(a,40+i*10,0,0);
		if (fp == NULL)
			for (j=0; j<100; j++)
				BN_sqr(c,a,ctx);
		BN_sqr(c,a,ctx);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," * ");
				BN_print(fp,a);
				fprintf(fp," - ");
				}
			BN_print(fp,c);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(c);
	return(1);
	}

#ifdef undef
int test_mont(fp,ctx)
FILE *fp;
BN_CTX *ctx;
	{
	BIGNUM *a,*b,*c;
	int i;
	int j;

	a=BN_new();
	b=BN_new();
	c=BN_new();

	BN_rand(a,100,0,0); /**/
	for (i=0; i<1; i++)
		{
		BN_rand(b,70+i*10,0,0); /**/
		if (fp == NULL)
			for (j=0; j<100; j++)
				BN_mont(c,a,b,NULL,ctx);/**/
		BN_mont(c,a,b,NULL,ctx);/**/
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," %% ");
				BN_print(fp,b);
				fprintf(fp," - ");
				}
			BN_print(fp,c);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	return(1);
	}
#endif

int test_mod(fp,ctx)
FILE *fp;
BN_CTX *ctx;
	{
	BIGNUM *a,*b,*c;
	int i;
	int j;

	a=BN_new();
	b=BN_new();
	c=BN_new();

	BN_rand(a,1024,0,0); /**/
	for (i=0; i<20; i++)
		{
		BN_rand(b,450+i*10,0,0); /**/
		if (fp == NULL)
			for (j=0; j<100; j++)
				BN_mod(c,a,b,ctx);/**/
		BN_mod(c,a,b,ctx);/**/
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," %% ");
				BN_print(fp,b);
				fprintf(fp," - ");
				}
			BN_print(fp,c);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	return(1);
	}

int test_mod_mul(fp,ctx)
FILE *fp;
BN_CTX *ctx;
	{
	BIGNUM *a,*b,*c,*d,*e;
	int i;

	a=BN_new();
	b=BN_new();
	c=BN_new();
	d=BN_new();
	e=BN_new();

	BN_rand(c,1024,0,0); /**/
	for (i=0; i<10; i++)
		{
		BN_rand(a,475+i*10,0,0); /**/
		BN_rand(b,425+i*10,0,0); /**/
	/*	if (fp == NULL)
			for (j=0; j<100; j++)
				BN_mod_mul(d,a,b,c,ctx);*/ /**/

		if (!BN_mod_mul(e,a,b,c,ctx))
			{
			unsigned long l;

			while ((l=ERR_get_error()))
				fprintf(stderr,"ERROR:%s\n",
					ERR_error_string(l,NULL));
			exit(1);
			}
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," * ");
				BN_print(fp,b);
				fprintf(fp," %% ");
				BN_print(fp,c);
				fprintf(fp," - ");
				}
			BN_print(fp,e);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	BN_free(d);
	BN_free(e);
	return(1);
	}

int test_mod_exp(fp,ctx)
FILE *fp;
BN_CTX *ctx;
	{
	BIGNUM *a,*b,*c,*d,*e;
	int i;

	a=BN_new();
	b=BN_new();
	c=BN_new();
	d=BN_new();
	e=BN_new();

	BN_rand(c,30,0,0); /**/
	for (i=0; i<6; i++)
		{
		BN_rand(a,20+i*5,0,0); /**/
		BN_rand(b,2+i,0,0); /**/

		if (!BN_mod_exp(d,a,b,c,ctx))
			return(00);

		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," ^ ");
				BN_print(fp,b);
				fprintf(fp," %% ");
				BN_print(fp,c);
				fprintf(fp," - ");
				}
			BN_print(fp,d);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	BN_free(d);
	BN_free(e);
	return(1);
	}

int test_lshift(fp)
FILE *fp;
	{
	BIGNUM *a,*b,*c;
	int i;

	a=BN_new();
	b=BN_new();
	c=BN_new();
	BN_one(c);

	BN_rand(a,200,0,0); /**/
	for (i=0; i<70; i++)
		{
		BN_lshift(b,a,i+1);
		BN_add(c,c,c);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," * ");
				BN_print(fp,c);
				fprintf(fp," - ");
				}
			BN_print(fp,b);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	return(1);
	}

int test_lshift1(fp)
FILE *fp;
	{
	BIGNUM *a,*b;
	int i;

	a=BN_new();
	b=BN_new();

	BN_rand(a,200,0,0); /**/
	for (i=0; i<70; i++)
		{
		BN_lshift1(b,a);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," * 2");
				fprintf(fp," - ");
				}
			BN_print(fp,b);
			fprintf(fp,"\n");
			}
		BN_copy(a,b);
		}
	BN_free(a);
	BN_free(b);
	return(1);
	}

int test_rshift(fp)
FILE *fp;
	{
	BIGNUM *a,*b,*c;
	int i;

	a=BN_new();
	b=BN_new();
	c=BN_new();
	BN_one(c);

	BN_rand(a,200,0,0); /**/
	for (i=0; i<70; i++)
		{
		BN_rshift(b,a,i+1);
		BN_add(c,c,c);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," / ");
				BN_print(fp,c);
				fprintf(fp," - ");
				}
			BN_print(fp,b);
			fprintf(fp,"\n");
			}
		}
	BN_free(a);
	BN_free(b);
	BN_free(c);
	return(1);
	}

int test_rshift1(fp)
FILE *fp;
	{
	BIGNUM *a,*b;
	int i;

	a=BN_new();
	b=BN_new();

	BN_rand(a,200,0,0); /**/
	for (i=0; i<70; i++)
		{
		BN_rshift1(b,a);
		if (fp != NULL)
			{
			if (!results)
				{
				BN_print(fp,a);
				fprintf(fp," / 2");
				fprintf(fp," - ");
				}
			BN_print(fp,b);
			fprintf(fp,"\n");
			}
		BN_copy(a,b);
		}
	BN_free(a);
	BN_free(b);
	return(1);
	}

/* lib/dh/dhtest.c */
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
#include <stdlib.h>
#include "bn.h"
#include "dh.h"

#ifndef NOPROTO
static void cb(int p, int n);
#else
static void cb();
#endif

int main(argc,argv)
int argc;
char *argv[];
	{
	DH *a,*b;
	unsigned char *abuf,*bbuf;
	int i,alen,blen,aout,bout,ret=1;

	a=DH_generate_parameters(64,DH_GENERATOR_5,cb);
	if (a == NULL) goto err;

	printf("\np    =");
	BN_print(stdout,a->p);
	printf("\ng    =");
	BN_print(stdout,a->g);
	printf("\n");

	b=DH_new();
	if (b == NULL) goto err;

	b->p=BN_dup(a->p);
	b->g=BN_dup(a->g);
	if ((b->p == NULL) || (b->g == NULL)) goto err;

	if (!DH_generate_key(a)) goto err;
	printf("pri 1=");
	BN_print(stdout,a->private);
	printf("\npub 1=");
	BN_print(stdout,a->public);
	printf("\n");

	if (!DH_generate_key(b)) goto err;
	printf("pri 2=");
	BN_print(stdout,b->private);
	printf("\npub 2=");
	BN_print(stdout,b->public);
	printf("\n");

	alen=DH_size(a);
	abuf=(unsigned char *)malloc(alen);
	aout=DH_compute_key(abuf,b->public,a);

	printf("key1 =");
	for (i=0; i<aout; i++)
		printf("%02X",abuf[i]);
	printf("\n");

	blen=DH_size(b);
	bbuf=(unsigned char *)malloc(blen);
	bout=DH_compute_key(bbuf,a->public,b);

	printf("key2 =");
	for (i=0; i<bout; i++)
		printf("%02X",bbuf[i]);
	printf("\n");
	if ((aout < 4) || (bout != aout) || (memcmp(abuf,bbuf,aout) != 0))
		{
		fprintf(stderr,"Error in DH routines\n");
		ret=1;
		}
	else
		ret=0;
err:
	exit(ret);
	}

static void cb(p, n)
int p;
int n;
	{
	int c='*';

	if (p == 0) c='.';
	if (p == 1) c='+';
	if (p == 2) c='*';
	if (p == 3) c='\n';
	fputc(c,stderr);
	fflush(stderr);
#ifdef LINT
	p=n;
#endif
	}

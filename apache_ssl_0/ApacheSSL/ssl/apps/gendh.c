/* apps/gendh.c */
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "apps.h"
#include "rand.h"
#include "err.h"
#include "bn.h"
#include "dh.h"
#include "x509.h"
#include "pem.h"

#define DEFBITS	512
#define PROG gendh_main

#ifndef NOPROTO
static void cb(int p, int n);
static long load_rand(char *names);
#else
static void cb();
static long load_rand();
#endif

int MAIN(argc, argv)
int argc;
char **argv;
	{
	char buffer[200];
	DH *dh=NULL;
	int ret=1,num=DEFBITS;
	int g=2;
	char *outfile=NULL;
	char *inrand=NULL,*randfile;
	FILE *out=NULL;

	argv++;
	argc--;
	for (;;)
		{
		if (argc <= 0) break;
		if (strcmp(*argv,"-out") == 0)
			{
			if (--argc < 1) goto bad;
			outfile= *(++argv);
			}
		else if (strcmp(*argv,"-2") == 0)
			g=2;
	/*	else if (strcmp(*argv,"-3") == 0)
			g=3; */
		else if (strcmp(*argv,"-5") == 0)
			g=5;
		else if (strcmp(*argv,"-rand") == 0)
			{
			if (--argc < 1) goto bad;
			inrand= *(++argv);
			}
		else
			break;
		argv++;
		argc--;
		}
	if ((argc >= 1) && ((sscanf(*argv,"%d",&num) == 0) || (num < 0)))
		{
bad:
		fprintf(stderr,"usage: gendh [args] [numbits]\n");
		fprintf(stderr," -out file - output the key to 'file\n");
		fprintf(stderr," -2    use 2 as the generator value\n");
	/*	fprintf(stderr," -3    use 3 as the generator value\n"); */
		fprintf(stderr," -5    use 5 as the generator value\n");
		fprintf(stderr," -rand file:file:...\n");
		fprintf(stderr,"           - load the file (or the files in the directory) into\n");
		fprintf(stderr,"             the random number generator\n");
		goto end;
		}
		
	if (outfile == NULL)
		out=stdout;
	else
		{ 
		out=fopen(outfile,"w");
		if (out == NULL)
			{
			perror("open");
			goto end;
			}
		}

	randfile=RAND_file_name(buffer,200);
	if ((randfile == NULL)|| !RAND_load_file(randfile,1024L*1024L))
		fprintf(stderr,"unable to load 'random state'\n");

	if (inrand == NULL)
		fprintf(stderr,"warning, not much extra random data, consider using the -rand option\n");
	else
		{
		fprintf(stderr,"%ld semi-random bytes loaded\n",
			load_rand(inrand));
		}

	fprintf(stderr,"Generating DH parameters, %d bit long strong prime, generator of %d\n",num,g);
	fprintf(stderr,"This is going to take a long time\n");
	dh=DH_generate_parameters(num,g,cb);
		
	if (dh == NULL) goto end;

	if (randfile == NULL)
		fprintf(stderr,"unable to write 'random state'\n");
	else
		RAND_write_file(randfile);

	if (!PEM_write_DHparams(out,dh))
		goto end;
	ret=0;
end:
	if (ret != 0)
		ERR_print_errors(stderr);
	if ((out != NULL) && (out != stdout)) fclose(out);
	if (dh != NULL) DH_free(dh);
	EXIT(ret);
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

static long load_rand(name)
char *name;
	{
	char *p,*n;
	int last;
	long tot=0;

	for (;;)
		{
		last=0;
		for (p=name; ((*p != '\0') && (*p != LIST_SEPERATOR_CHAR)); p++);
		if (*p == '\0') last=1;
		*p='\0';
		n=name;
		name=p+1;
		if (*n == '\0') break;

		tot+=RAND_load_file(n,1);
		if (last) break;
		}
	return(tot);
	}



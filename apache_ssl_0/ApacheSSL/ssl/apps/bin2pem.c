/* apps/bin2pem.c */
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
#include <time.h>
#include "apps.h"
#include "x509.h"
#include "pem.h"
#include "err.h"

/* -in arg	- input file - default stdin
 * -out arg	- output file -default stdout
 */
#define PROG	bin2pem_main

int MAIN(argc, argv)
int argc;
char **argv;
	{
	int ret=1,i,badops=0;
	FILE *in=NULL,*out=NULL;
	char *infile,*outfile,*prog;
	char buf[1024*100];

	infile=NULL;
	outfile=NULL;

	prog=argv[0];
	argc--;
	argv++;
	while (argc >= 1)
		{
		if	(strcmp(*argv,"-in") == 0)
			{
			if (--argc < 1) goto bad;
			infile= *(++argv);
			}
		else if (strcmp(*argv,"-out") == 0)
			{
			if (--argc < 1) goto bad;
			outfile= *(++argv);
			}
		else
			{
			fprintf(stderr,"unknown option %s\n",*argv);
			badops=1;
			break;
			}
		argc--;
		argv++;
		}

	if (badops)
		{
bad:
		fprintf(stderr,"%s [options] <infile >outfile\n",prog);
		fprintf(stderr,"where options are\n");
		fprintf(stderr," -in arg       inout file\n");
		fprintf(stderr," -out arg      output file\n");
		goto end;
		}

	if (infile == NULL)
		in=stdin;
	else
		{
		in=fopen(infile,"r");
		if (in == NULL)
			{
			perror(infile);
			goto end;
			}
		}

	ERR_load_crypto_strings();
	i=fread(buf,1,1024*100,in);
	if (i <= 0)
		{
		fprintf(stderr,"unable to load data\n");
		goto end;
		}

	if (outfile == NULL)
		out=stdout;
	else
		{
		out=fopen(outfile,"w");
		if (out == NULL)
			{
			perror(outfile);
			goto end;
			}
		}

	i=PEM_write(out,"","",(unsigned char *)buf,(long)i);
	if (i <= 0)
		{
		perror("fwrite");
		goto end;
		}
	ret=0;
end:
	if ((in != NULL) && (in != stdin)) fclose(in);
	if ((out != NULL) && (out != stdout)) fclose(out);
	EXIT(ret);
	}


/* apps/pkcs7.c */
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
#include <time.h>
#include "apps.h"
#include "err.h"
#include "envelope.h"
#include "x509.h"
#include "pkcs7.h"
#include "pem.h"

#define PROG	pkcs7_main

/* -inform arg	- input format - default PEM (one of DER, TXT or PEM)
 * -outform arg - output format - default PEM
 * -in arg	- input file - default stdin
 * -out arg	- output file - default stdout
 * -des		- encrypt output if PEM format with DES in cbc mode
 * -des3	- encrypt output if PEM format
 * -idea	- encrypt output if PEM format
 */

int MAIN(argc, argv)
int argc;
char **argv;
	{
	PKCS7 *p7;
	int i,badops=0;
	EVP_CIPHER *enc=NULL;
	FILE *in,*out;
	int informat,outformat;
	char *infile,*outfile,*prog;

	infile=NULL;
	outfile=NULL;
	informat=FORMAT_PEM;
	outformat=FORMAT_PEM;

	prog=argv[0];
	argc--;
	argv++;
	while (argc >= 1)
		{
		if 	(strcmp(*argv,"-inform") == 0)
			{
			if (--argc < 1) goto bad;
			informat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-outform") == 0)
			{
			if (--argc < 1) goto bad;
			outformat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-in") == 0)
			{
			if (--argc < 1) goto bad;
			infile= *(++argv);
			}
		else if (strcmp(*argv,"-out") == 0)
			{
			if (--argc < 1) goto bad;
			outfile= *(++argv);
			}
		else if (strcmp(*argv,"-des") == 0)
			enc=EVP_des_cbc;
		else if (strcmp(*argv,"-des3") == 0)
			enc=EVP_des_cbc_ede3;
#ifndef NOIDEA
		else if (strcmp(*argv,"-idea") == 0)
			enc=EVP_idea_cbc;
#endif
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
		fprintf(stderr," -inform arg   input format - one of DER TXT PEM\n");
		fprintf(stderr," -outform arg  output format - one of DER TXT PEM\n");
		fprintf(stderr," -in arg       inout file\n");
		fprintf(stderr," -out arg      output file\n");
		fprintf(stderr," -des          encrypt PEM output with cbc des\n");
		fprintf(stderr," -des3         encrypt PEM output with ede cbc des using 168 bit key\n");
#ifndef NOIDEA
		fprintf(stderr," -idea         encrypt PEM output with cbc idea\n");
#endif
		EXIT(1);
		}

	ERR_load_crypto_strings();

	if (infile == NULL)
		in=stdin;
	else
		{
		in=fopen(infile,"r");
		if (in == NULL)
			{
			perror(infile);
			EXIT(1);
			}
		}

	if	(informat == FORMAT_ASN1)
		p7=d2i_PKCS7_fp(in,NULL);
	else if (informat == FORMAT_PEM)
		p7=PEM_read_PKCS7(in,NULL,NULL);
	else
		{
		fprintf(stderr,"bad input format specified for pkcs7 object\n");
		EXIT(1);
		}
	if (p7 == NULL)
		{
		fprintf(stderr,"unable to load PKCS7 object\n");
		ERR_print_errors(stderr);
		EXIT(1);
		}

	if (outfile == NULL)
		out=stdout;
	else
		{
		out=fopen(outfile,"w");
		if (out == NULL)
			{
			perror(outfile);
			EXIT(1);
			}
		}

	if 	(outformat == FORMAT_ASN1)
		i=i2d_PKCS7_fp(out,p7);
	else if (outformat == FORMAT_PEM)
		i=PEM_write_PKCS7(out,p7);
	else	{
		fprintf(stderr,"bad output format specified for outfile\n");
		EXIT(1);
		}
	if (!i)
		{
		fprintf(stderr,"unable to write pkcs7 object\n");
		ERR_print_errors(stderr);
		EXIT(1);
		}
	if (in != stdin) fclose(in);
	if (out != stdout) fclose(out);
	EXIT(0);
	}

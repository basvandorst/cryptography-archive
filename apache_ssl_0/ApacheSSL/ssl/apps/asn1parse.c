/* apps/asn1parse.c */
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
#include <string.h>
#include "apps.h"
#include "err.h"
#include "buffer.h"
#include "x509.h"
#include "pem.h"

#define FORMAT_UNDEF	0
#define FORMAT_ASN1	1
#define FORMAT_TEXT	2
#define FORMAT_PEM	3

/* -inform arg	- input format - default PEM (DER or PEM)
 * -in arg	- input file - default stdin
 */

#define PROG	asn1parse_main

int MAIN(argc, argv)
int argc;
char **argv;
	{
	int i,badops=0,offset=0,length=0,ret=1;
	long num;
	FILE *in=NULL;
	int informat;
	char *infile,*str=NULL,*prog;
	BUFFER *buf=NULL;

	infile=NULL;
	informat=FORMAT_PEM;

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
		else if (strcmp(*argv,"-in") == 0)
			{
			if (--argc < 1) goto bad;
			infile= *(++argv);
			}
		else if (strcmp(*argv,"-offset") == 0)
			{
			if (--argc < 1) goto bad;
			offset= atoi(*(++argv));
			}
		else if (strcmp(*argv,"-length") == 0)
			{
			if (--argc < 1) goto bad;
			length= atoi(*(++argv));
			if (length == 0) goto bad;
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
		fprintf(stderr,"%s [options] <infile\n",prog);
		fprintf(stderr,"where options are\n");
		fprintf(stderr," -inform arg   input format - one of DER TXT PEM\n");
		fprintf(stderr," -in arg       inout file\n");
		fprintf(stderr," -offset arg   offset into file\n");
		fprintf(stderr," -length arg   lenth of section in file\n");
		goto end;
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
			goto end;
			}
		}
	if (informat == FORMAT_ASN1)
		{
		num=0;
		buf=BUF_new();
		if (buf == NULL)
			{
			fprintf(stderr,"memory error\n");
			goto end;
			}
		for (;;)
			{
			if (!BUF_grow(buf,num+BUFSIZ)) goto end;
			i=fread(&(buf->data[num]),1,BUFSIZ,in);
			if (i <= 0) break;
			num+=i;
			}
		str=buf->data;
		buf->data=NULL;
		i=1;
		}
	else
		{
		char *name=NULL,*header=NULL;
		EVP_CIPHER_INFO cipher;

		if (!PEM_read(in,&name,&header,(unsigned char **)&str,&num) ||
		    !PEM_get_EVP_CIPHER_INFO(header,&cipher) ||
		    !PEM_do_header(&cipher,(unsigned char *)str,&num,NULL))
			i=0;
		else
			i=1;
		if (name != NULL) free(name);
		if (header != NULL) free(header);
		if (!i)
			{
			ERR_print_errors(stderr);
			goto end;
			}
		}
	if (i <= 0)
		{
		perror("error reading input");
		ERR_print_errors(stderr);
		goto end;
		}
	if (length == 0) length=num;
	ASN1_parse(stdout,(unsigned char *)&(str[offset]),(unsigned int)length);
	ret=0;
end:
	if ((in != NULL) && (in != stdin)) fclose(in);
	if (ret != 0)
		ERR_print_errors(stderr);
	if (buf != NULL) BUF_free(buf);
	if (str != NULL) free(str);
	EXIT(ret);
	}


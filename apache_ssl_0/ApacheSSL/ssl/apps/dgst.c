/* apps/dgst.c */
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
#include <unistd.h>
#include <string.h>
#include "envelope.h"
#include "objects.h"
#include "x509.h"
#include "pem.h"

#define BUFSIZE	10240

#define PROG	dgst_main

#if defined(MSDOS) || defined(_Windows)
#define EXIT(n)         return(n);
#define LIST_SEPERATOR_CHAR ';'
#include <io.h>
#else
#ifndef MONOLITH
#define EXIT(n)         exit(n); return(n)
#else
#define EXIT(n)         return(n)
#endif
#endif

#ifndef MONOLITH
#define MAIN(a,v)       main(a,v)
#else
#define MAIN(a,v)       PROG(a,v)
#endif


#ifndef NOPROTO
void do_fp(EVP_MD *t, FILE *f);
#else
void do_fp();
#endif

unsigned char buf[BUFSIZE];

int MAIN(argc,argv)
int argc;
char **argv;
	{
	int i,err=0;
	EVP_MD *md=NULL,*m;
	FILE *in=NULL;
	char *name;

	md=EVP_get_MDbyname(argv[0]);


	if ((argc >= 2) && (argv[1][0] == '-') &&
		((m=EVP_get_MDbyname(&(argv[1][1]))) != NULL))
		{
		argc--;
		argv++;
		md=m;
		}
	else if (md == NULL)
		md=EVP_md5;

	argc--;
	argv++;

	if ((argc > 0) && (argv[0][0] == '-')) /* bad option */
		{
		fprintf(stderr,"unknown option '%s'\n",*argv);
		fprintf(stderr,"options are\n");
		fprintf(stderr,"-%3s to use the %s message digest alogorithm (default)\n",
			LN_md5,LN_md5);
		fprintf(stderr,"-%3s to use the %s message digest alogorithm\n",
			LN_md2,LN_md2);
		fprintf(stderr,"-%3s to use the %s message digest alogorithm\n",
			LN_sha,LN_sha);
		fprintf(stderr,"-%3s to use the %s message digest alogorithm\n",
			LN_sha1,LN_sha1);
		err=1;
		goto end;
		}
	
	if (argc == 0)
		{
		do_fp(md,stdin);
		}
	else
		{
		name=OBJ_nid2sn(md->type);
		for (i=0; i<argc; i++)
			{
			in=fopen(argv[i],"r");
			if (in == NULL)
				{
				perror(argv[i]);
				err++;
				continue;
				}
			printf("%s(%s)= ",name,argv[i]);
			do_fp(md,in);
			fclose(in);
			}
		}
end:
	if ((in != NULL) && (in != stdin)) fclose(in);
	EXIT(err);
	}

void do_fp(md,f)
EVP_MD *md;
FILE *f;
	{
	EVP_MD_CTX c;
	int fd,i;
	unsigned int len;

	fd=fileno(f);
	EVP_DigestInit(&c,md);
	for (;;)
		{
		i=read(fd,buf,BUFSIZE);
		if (i <= 0) break;
		EVP_DigestUpdate(&c,buf,(unsigned long)i);
		}
	EVP_DigestFinal(&c,buf,&len);

	for (i=0; i<len; i++)
		printf("%02x",buf[i]);
	printf("\n");
	}

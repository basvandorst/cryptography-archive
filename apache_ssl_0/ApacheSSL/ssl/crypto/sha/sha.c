/* lib/sha/sha.c */
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
#include "sha.h"

#define BUFSIZE	1024*16

#ifndef NOPROTO
void do_fp(FILE *f);
void pt(unsigned char *md);
int read(int, void *, unsigned int);
#else
void do_fp();
void pt();
int read();
#endif

int main(argc, argv)
int argc;
char **argv;
	{
	int i,err=0;
	FILE *IN;

	if (argc == 1)
		{
		do_fp(stdin);
		}
	else
		{
		for (i=1; i<argc; i++)
			{
			IN=fopen(argv[i],"r");
			if (IN == NULL)
				{
				perror(argv[i]);
				err++;
				continue;
				}
			printf("SHA(%s)= ",argv[i]);
			do_fp(IN);
			fclose(IN);
			}
		}
	exit(err);
	}

void do_fp(f)
FILE *f;
	{
	SHA_CTX c;
	unsigned char md[SHA_DIGEST_LENGTH];
	int fd;
	int i;
	unsigned char buf[BUFSIZE];

	fd=fileno(f);
	SHA_Init(&c);
	for (;;)
		{
		i=read(fd,buf,BUFSIZE);
		if (i <= 0) break;
		SHA_Update(&c,buf,(unsigned long)i);
		}
	SHA_Final(&(md[0]),&c);
	pt(md);
	}

void pt(md)
unsigned char *md;
	{
	int i;

	for (i=0; i<SHA_DIGEST_LENGTH; i++)
		printf("%02x",md[i]);
	printf("\n");
	}


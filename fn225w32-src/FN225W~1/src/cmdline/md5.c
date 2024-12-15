/* crypto/md/md5.c */
/* Copyright (C) 1995-1996 Eric Young (eay@mincom.oz.au)
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
#include "md5_calc.h"

#ifdef macintosh
#include <console.h>
#include <unix.h>
#endif

static void usage()
	{
	fprintf(stderr, "Usage: md5 [file ...]\n");
	fprintf(stderr, "   or: md5 -r {range} {file ...}\n");
	fprintf(stderr, "range = start[-stop|:len][,range]\n");
	exit(1);
	}

int main(argc, argv)
int argc;
char **argv;
	{
	int i,err=0;
	FILE *IN;
	char *p;
	char *r = NULL;

#ifdef macintosh
	argc = ccommand(&argv);
#endif

	if (argc == 1)
		{
		puts(md5_calc(0, ""));
		}
	else if (argc > 1)
		{
		if (strcmp(argv[1], "/?") == 0)
			usage();
		if (argv[1][0] == '-')
			{
			if (strcmp(argv[1], "-r") != 0)
				usage();
			if (argc < 4)
				usage();

			r = argv[2];
			argv += 2;
			argc -= 2;
			}

		for (i=1; i<argc; i++)
			{
			IN=fopen(argv[i],"rb");
			if (IN == NULL)
				{
				perror(argv[i]);
				err++;
				continue;
				}
			p = md5_calc(fileno(IN), r);
			if (r)
				printf("MD5(%s %s)= %s\n", argv[i], r, p);
			else
				printf("MD5(%s)= %s\n", argv[i], p);
			fclose(IN);
			}
		}
	exit(err);
	}

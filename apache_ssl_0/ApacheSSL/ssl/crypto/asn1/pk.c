/* lib/asn1/pk.c */
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
#include "../error/err.h"
#include "./asn1.h"
#include "rsa.h"
#include "../x509/x509.h"
#include "x509.h"
#include "pkcs7.h"

main()
	{
	PKCS7 *x;
	FILE *in;
	unsigned char buf[10240],buf2[10240],*p;
	int num,i;

	PKCS7 *nx=NULL,*mx=NULL;

	in=fopen("pkcs7.der","r");
	if (in == NULL)
		{
		perror("pkcs7.der");
		exit(1);
		}
	num=fread(buf,1,10240,in);
	fclose(in);


		p=buf;
		if (d2i_PKCS7(&nx,&p,num) == NULL) goto err;
		printf("num=%d p-buf=%d\n",num,p-buf);

exit(0);
		p=buf2;
		num=i2d_PKCS7(nx,&p);
		printf("num=%d p-buf=%d\n",num,p-buf2);

		if (memcmp(buf,buf2,num) != 0)
			{
			fprintf(stderr,"data difference\n");
			for (i=0; i<num; i++)
				fprintf(stderr,"%c%03d <%02X-%02X>\n",
					(buf[i] == buf2[i])?' ':'*',i,
					buf[i],buf2[i]);
			fprintf(stderr,"\n");
			exit(1);
			}

		p=buf2;
		if (d2i_PKCS7(&mx,&p,num) == NULL) goto err;
		printf("num=%d p-buf=%d\n",num,p-buf2);

/*		X509_print(stdout,mx);*/

	exit(0);
err:
	ERR_load_crypto_strings();
	ERR_print_errors(stderr);
	exit(1);
	}


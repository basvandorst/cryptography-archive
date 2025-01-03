/* test/methtest.c */
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
#include "rsa.h"
#include "x509.h"
#include "meth.h"
#include "err.h"

int main(argc,argv)
int argc;
char *argv[];
	{
	METHOD_CTX *top,*tmp1,*tmp2;

	top=METH_new(x509_lookup); /* get a top level context */
	if (top == NULL) goto err;

	tmp1=METH_new(x509_by_file);
	if (top == NULL) goto err;
	METH_arg(tmp1,METH_TYPE_FILE,"cafile1");
	METH_arg(tmp1,METH_TYPE_FILE,"cafile2");
	METH_push(top,METH_X509_CA_BY_SUBJECT,tmp1);

	tmp2=METH_new(x509_by_dir);
	METH_arg(tmp2,METH_TYPE_DIR,"/home/eay/.CAcerts");
	METH_arg(tmp2,METH_TYPE_DIR,"/home/eay/SSLeay/certs");
	METH_arg(tmp2,METH_TYPE_DIR,"/usr/local/ssl/certs");
	METH_push(top,METH_X509_CA_BY_SUBJECT,tmp2);

/*	tmp=METH_new(x509_by_issuer_dir);
	METH_arg(tmp,METH_TYPE_DIR,"/home/eay/.mycerts");
	METH_push(top,METH_X509_BY_ISSUER,tmp);

	tmp=METH_new(x509_by_issuer_primary);
	METH_arg(tmp,METH_TYPE_FILE,"/home/eay/.mycerts/primary.pem");
	METH_push(top,METH_X509_BY_ISSUER,tmp);
*/

	METH_init(top);
	METH_control(tmp1,METH_CONTROL_DUMP,stdout);
	METH_control(tmp2,METH_CONTROL_DUMP,stdout);
	exit(0);
err:
	ERR_load_crypto_strings();
	ERR_print_errors(stderr);
	exit(1);
	return(0);
	}

/* lib/pem/pem_sign.c */
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
#include "crypto.h"
#include "rand.h"
#include "rsa.h"
#include "envelope.h"
#include "objects.h"
#include "x509.h"
#include "pem.h"

int PEM_SignFinal(ctx,sigret,siglen,rsa)
EVP_MD_CTX *ctx;
unsigned char *sigret;
unsigned int *siglen;
RSA *rsa;
	{
	unsigned char *m=NULL;
	int i,ret=0;
	unsigned int m_len;

	m=(unsigned char *)malloc(RSA_size(rsa)+2);
	if (m == NULL)
		{
		PEMerr(PEM_F_PEM_SIGNFINAL,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	if (!EVP_SignFinal(ctx,m,&m_len,rsa)) goto err;

	i=EVP_EncodeBlock(sigret,m,m_len);
	*siglen=i;
	ret=1;
err:
	if (m != NULL) free(m);
	return(ret);
	}
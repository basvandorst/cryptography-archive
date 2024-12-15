/* lib/evp/p_verify.c */
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
#include "rsa.h"
#include "envelope.h"
#include "objects.h"
#include "x509.h"

int EVP_VerifyFinal(ctx,sigbuf,siglen,rsa)
EVP_MD_CTX *ctx;
unsigned char *sigbuf;
unsigned int siglen;
RSA *rsa;
	{
	unsigned char m[EVP_MAX_MD_SIZE];
	unsigned int m_len;
	int i,ret=0,sigtype;
	X509_SIG *sig=NULL;
	unsigned char *p,*s;

	EVP_DigestFinal(ctx,&(m[0]),&m_len);

	if (siglen != RSA_size(rsa))
		{
		EVPerr(EVP_F_EVP_VERIFYFINAL,EVP_R_WRONG_SIGNATURE_LENGTH);
		return(0);
		}

	s=(unsigned char *)malloc((unsigned int)siglen);
	if (s == NULL)
		{
		EVPerr(EVP_F_EVP_VERIFYFINAL,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	i=RSA_public_decrypt((int)siglen,sigbuf,s,rsa);

	if (i <= 0) goto err;

	p=s;
	sig=d2i_X509_SIG(NULL,&p,(long)i);
	if (sig == NULL) goto err;
	sigtype=OBJ_obj2nid(sig->algor->algorithm);

#ifdef EVP_DEBUG
	/* put a backward compatability flag in EAY */
	fprintf(stderr,"in(%s) expect(%s)\n",OBJ_nid2ln(sigtype),
		OBJ_nid2ln(ctx->digest->type));
#endif
	if (sigtype != ctx->digest->type)
		{
		if (((ctx->digest->type == NID_md5) &&
			(sigtype == NID_md5withRSAEncryption)) ||
			((ctx->digest->type == NID_md2) &&
			(sigtype == NID_md2withRSAEncryption)))
			{
			/* ok, we will let it through */
			fprintf(stderr,"signature has problems, re-make with post SSLeay045\n");
			}
		else
			{
			EVPerr(EVP_F_EVP_VERIFYFINAL,EVP_R_ALGORITHM_MISMATCH);
			goto err;
			}
		}
	if (	(sig->digest->length != m_len) ||
		(memcmp(&(m[0]),sig->digest->data,m_len) != 0))
		{
		EVPerr(EVP_F_EVP_VERIFYFINAL,EVP_R_BAD_SIGNATURE);
		}
	else
		ret=1;
err:
	if (sig != NULL) X509_SIG_free(sig);
	memset(s,0,(unsigned int)siglen);
	free(s);
	return(ret);
	}

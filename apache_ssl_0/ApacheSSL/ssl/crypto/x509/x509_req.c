/* lib/x509/x509_req.c */
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
#include "bn.h"
#include "rsa.h"
#include "envelope.h"
#include "asn1.h"
#include "x509.h"
#include "objects.h"
#include "buffer.h"
#include "pem.h"

X509_REQ *X509_X509_TO_req(x,rsa)
X509 *x;
RSA *rsa;
	{
	X509_REQ *ret;
	X509_REQ_INFO *ri;
	X509_NAME *n;
	ASN1_OBJECT *obj;
	int i;
	unsigned char *s,*p;

	ret=X509_REQ_new();
	if (ret == NULL)
		{
		X509err(X509_F_X509_X509_TO_REQ,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	ri=ret->req_info;

	ri->version->length=1;
	ri->version->data=(unsigned char *)malloc(1);
	if (ri->version->data == NULL) goto err;
	ri->version->data[0]=0; /* version == 0 */

	n=X509_NAME_dup(x->cert_info->subject);
	if (n == NULL) goto err;

	X509_NAME_free(ri->subject); 
	ri->subject=n;

	obj=OBJ_nid2obj(NID_rsaEncryption);
	if (obj == NULL) goto err;
	ASN1_OBJECT_free(ri->pubkey->algor->algorithm);
	ri->pubkey->algor->algorithm=obj;

	i=i2d_RSAPublicKey(rsa,NULL);
	s=(unsigned char *)malloc((unsigned int)i+1);
	if (s == NULL)
		{
		X509err(X509_F_X509_X509_TO_REQ,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	p=s;
	i2d_RSAPublicKey(rsa,&p);
	ri->pubkey->public_key->length=i;
	ri->pubkey->public_key->data=s;

	if (!X509_REQ_sign(ret,rsa,EVP_md5))
		goto err;
	return(ret);
err:
	X509_REQ_free(ret);
	return(NULL);
	}

#ifdef F2I
void i2f_X509_REQ_INFO(fp, a)
FILE *fp;
X509_REQ_INFO *a;
	{
	Fprintf(fp,"%s\n",STRING_REQ_INFO_BEGIN);
	i2f_ASN1_BIT_STRING(fp,a->version);
	i2f_X509_NAME(fp,a->subject);
	i2f_X509_PUBKEY(fp,a->pubkey);
	Fprintf(fp,"%s\n",STRING_REQ_INFO_END);
	}

void i2f_X509_REQ(fp, a)
FILE *fp;
X509_REQ *a;
	{
	Fprintf(fp,"%s\n",STRING_REQ_BEGIN);
	i2f_X509_REQ_INFO(fp,a->req_info);
	i2f_X509_ALGOR(fp,a->sig_alg);
	i2f_ASN1_BIT_STRING(fp,a->signature);
	Fprintf(fp,"%s\n",STRING_REQ_END);
	}
#endif

RSA *X509_REQ_extract_key(req)
X509_REQ *req;
	{
	X509_REQ_INFO *ri;
	RSA *rsa=NULL;
	long j;
	unsigned char *p;

	ri=req->req_info;

	p=ri->pubkey->public_key->data;
	j=ri->pubkey->public_key->length;
	rsa=d2i_RSAPublicKey(NULL,&p,(long)j);
	return(rsa);
	}

/* replaced by a macro front end to ASN1_verify */
#ifdef undef 
int X509_REQ_verify(req)
X509_REQ *req;
	{
	EVP_MD_CTX ctx;
	X509_REQ_INFO *ri;
	int type,j,ret;
	EVP_MD *ptype;
	unsigned char *s,*p;
	RSA *rsa;

	ri=req->req_info;

	type=OBJ_obj2nid(req->sig_alg->algorithm);
	if (type == NID_undef)
		{
		X509err(X509_F_X509_REQ_VERIFY,X509_R_UNKNOWN_OBJECT_TYPE);
		return(-1);
		}
	if	((type == NID_md5) || (type == NID_md5withRSAEncryption))
		ptype=EVP_md5;
	else if ((type == NID_md2) || (type == NID_md2withRSAEncryption))
		ptype=EVP_md2;
	else if ((type == NID_sha) || (type == NID_shaWithRSAEncryption))
		ptype=EVP_sha;
	else
		{
		X509err(X509_F_X509_REQ_VERIFY,X509_R_UNKNOWN_DIGEST_TYPE);
		return(-1);
		}

	p=ri->pubkey->public_key->data;
	j=ri->pubkey->public_key->length;
	rsa=d2i_RSAPublicKey(NULL,&p,(long)j);
	if (rsa == NULL) return(-1);

	j=i2d_X509_REQ_INFO(ri,NULL);
	s=malloc((unsigned int)j);
	if (s == NULL)
		{
		RSA_free(rsa);
		X509err(X509_F_X509_REQ_VERIFY,ERR_R_MALLOC_FAILURE);
		return(-1);
		}
	p=s;
	i2d_X509_REQ_INFO(ri,&p);
	EVP_VerifyInit(&ctx,ptype);
	EVP_VerifyUpdate(&ctx,(unsigned char *)s,j);
	if (!EVP_VerifyFinal(&ctx,(unsigned char *)req->signature->data,
			req->signature->length,rsa))
		ret=0;
	else
		ret=1;

	RSA_free(rsa);
	free(s);
	return(ret);
	}
#endif

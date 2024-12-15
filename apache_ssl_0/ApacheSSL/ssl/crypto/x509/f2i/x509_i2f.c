#ifdef F2I
/* lib/x509/x509_i2f.c */
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
#include "buffer.h"
#include "x509.h"

void i2f_X509_ALGOR(fp, a)
FILE *fp;
X509_ALGOR *a;
	{
	Fprintf(fp,"%s\n",STRING_ALGORITHMIDENTIFIER_BEGIN);
	i2f_DER_OBJECT(fp,a->algorithm);
	i2f_DER_BIT_STRING(fp,a->parameters);
	Fprintf(fp,"%s\n",STRING_ALGORITHMIDENTIFIER_END);
	}

void i2f_X509_NAME(fp, a)
FILE *fp;
X509_NAME *a;
	{
	int i;

	Fprintf(fp,"%s\n",STRING_NAME_BEGIN);
	for (i=0; i<a->num; i++)
		{
		i2f_DER_OBJECT(fp,a->objects[i]);
		Fprintf(fp,"%c %s\n",(((i+1) == a->num)?'-':'='),
			a->values[i]->data);
		}
	Fprintf(fp,"%s\n",STRING_NAME_END);
	}

void i2f_X509_VAL(fp, a)
FILE *fp;
X509_VAL *a;
	{
	Fprintf(fp,"%s\n",STRING_VALIDITY_BEGIN);
	Fprintf(fp,"= %s\n",(a->notBefore != NULL)?a->notBefore:"");
	Fprintf(fp,"= %s\n",(a->notAfter != NULL)?a->notAfter:"");
	Fprintf(fp,"%s\n",STRING_VALIDITY_END);
	}

void i2f_X509_PUBKEY(fp, a)
FILE *fp;
X509_PUBKEY *a;
	{
	Fprintf(fp,"%s\n",STRING_SUBJECTPUBLICKEYINFO_BEGIN);
	i2f_X509_ALGOR(fp,a->algor);
	i2f_DER_BIT_STRING(fp,a->public_key);
	Fprintf(fp,"%s\n",STRING_SUBJECTPUBLICKEYINFO_END);
	}

void i2f_X509_SIG(fp, a)
FILE *fp;
X509_SIG *a;
	{
	Fprintf(fp,"%s\n",STRING_SIGNATURE_BEGIN);
	i2f_X509_ALGOR(fp,a->algor);
	i2f_DER_BIT_STRING(fp,a->digest);
	Fprintf(fp,"%s\n",STRING_SIGNATURE_END);
	}

void i2f_X509_CINF(fp, a)
FILE *fp;
X509_CINF *a;
	{
	Fprintf(fp,"%s\n",STRING_CERTIFICATE_BEGIN);
/*	Fprintf(fp,"= %ld\n",a->version); */
	i2f_DER_BIT_STRING(fp,a->serialNumber);
	i2f_X509_ALGOR(fp,a->signature);
	i2f_X509_NAME(fp,a->issuer);
	i2f_X509_VAL(fp,a->validity);
	i2f_X509_NAME(fp,a->subject);
	i2f_X509_PUBKEY(fp,a->key);
	Fprintf(fp,"%s\n",STRING_CERTIFICATE_END);
	}

void i2f_X509_CRL_INFO(fp, a)
FILE *fp;
X509_CRL_INFO *a;
	{
	int i;

	Fprintf(fp,"%s\n",STRING_CRL_INFO_BEGIN);
	i2f_X509_ALGOR(fp,a->sig_alg);
	i2f_X509_NAME(fp,a->issuer);
	Fprintf(fp,"= %s\n",(a->lastUpdate != NULL)?a->lastUpdate:"");
	Fprintf(fp,"%c %s\n",
		(a->num)?'=':'-',
		(a->nextUpdate != NULL)?a->nextUpdate:"");
	for (i=0; i<a->num; i++)
		{
		i2f_DER_BIT_STRING(fp,a->revoked[i]->serialNumber);
		Fprintf(fp,"%c %s\n",
			(i+1 == a->num)?'-':'=',
			(a->revoked[i]->revocationDate != NULL)?
			 a->revoked[i]->revocationDate:"");
		}
	Fprintf(fp,"%s\n",STRING_CRL_INFO_END);
	}

void i2f_X509_CRL(fp, a)
FILE *fp;
X509_CRL *a;
	{
	Fprintf(fp,"%s\n",STRING_CRL_BEGIN);
	i2f_X509_CRL_INFO(fp,a->crl);
	i2f_X509_ALGOR(fp,a->sig_alg);
	i2f_DER_BIT_STRING(fp,a->signature);
	Fprintf(fp,"%s\n",STRING_CRL_END);
	}

void i2f_X509(fp, a)
FILE *fp;
X509 *a;
	{
	Fprintf(fp,"%s\n",STRING_X509_CERTIFICATE_BEGIN);
	i2f_X509_CINF(fp,a->cert_info);
	i2f_X509_ALGOR(fp,a->sig_alg);
	i2f_DER_BIT_STRING(fp,a->signature);
	Fprintf(fp,"%s\n",STRING_X509_CERTIFICATE_END);
	}

int i2f_RSAPrivateKey(fp, rsa)
FILE *fp;
RSA *rsa;
	{
	int n=RSA_size(rsa)+2,i,ret=0;
	BUFFER *b;
	DER_BIT_STRING bs;
	BIGNUM *nums[9];

	b=BUF_new();
	if (b == NULL)
		{
		DERerr(DER_F_I2F_RSAPRIVATEKEY,ERR_R_MALLOC_FAILURE);
		return(0);
		}
	if (!BUF_grow(b,n))
		{
		DERerr(DER_F_I2F_RSAPRIVATEKEY,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	
	nums[0]=NULL;
	nums[1]=rsa->n;
	nums[2]=rsa->e;
	nums[3]=rsa->d;
	nums[4]=rsa->p;
	nums[5]=rsa->q;
	nums[6]=rsa->dmp1;
	nums[7]=rsa->dmq1;
	nums[8]=rsa->iqmp;
	bs.data=(unsigned char *)b->data;
	Fprintf(fp,"%s\n",STRING_RSAPRIVATEKEY_BEGIN);
	for (i=0; i<=8; i++)
		{
		if (nums[i] == NULL)
			{
			bs.length=1;
			b->data[0]=0;
			}
		else
			bs.length=BN_bn2bin(nums[i],
				(unsigned char *)b->data);
		i2f_DER_BIT_STRING(fp,&bs);
		}
	Fprintf(fp,"%s\n",STRING_RSAPRIVATEKEY_END);
	ret=1;
err:
	BUF_free(b);
	return(ret);
	}

int i2f_RSAPublicKey(fp, rsa)
FILE *fp;
RSA *rsa;
	{
	int n=RSA_size(rsa)+2,i,ret=0;
	BUFFER *b;
	DER_BIT_STRING bs;
	BIGNUM *nums[2];

	b=BUF_new();
	if (b == NULL)
		{
		DERerr(DER_F_I2F_RSAPUBLICKEY,ERR_R_MALLOC_FAILURE);
		return(0);
		}
	if (!BUF_grow(b,n*2))
		{
		DERerr(DER_F_I2F_RSAPUBLICKEY,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	nums[0]=rsa->n;
	nums[1]=rsa->e;
	bs.data=(unsigned char *)b->data;
	Fprintf(fp,"%s\n",STRING_RSAPUBLICKEY_BEGIN);
	for (i=0; i<=1; i++)
		{
		if (nums[i] == NULL)
			{
			bs.length=1;
			b->data[0]=0;
			}
		else
			bs.length=BN_bn2bin(nums[i],
				(unsigned char *)b->data);
		i2f_DER_BIT_STRING(fp,&bs);
		}
	Fprintf(fp,"%s\n",STRING_RSAPUBLICKEY_END);
	ret=1;
err:
	BUF_free(b);
	return(ret);
	}
#endif

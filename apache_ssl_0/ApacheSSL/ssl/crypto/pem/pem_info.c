/* lib/pem/pem_info.c */
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
#include "objects.h"
#include "rsa.h"
#include "envelope.h"
#include "x509.h"
#include "pem.h"

STACK *PEM_X509_INFO_read(fp,sk,cb)
FILE *fp;
STACK *sk;
int (*cb)();
	{
	X509_INFO *xi=NULL;
	char *name=NULL,*header=NULL,**pp;
	unsigned char *data=NULL,*p;
	long len,error=0;
	int ok=0;
	STACK *ret=NULL;
	int i,raw;
	char *(*d2i)();

	if (sk == NULL)
		{
		if ((ret=sk_new_null()) == NULL)
			{
			PEMerr(PEM_F_PEM_X509_INFO_READ,ERR_R_MALLOC_FAILURE);
			goto err;
			}
		}
	else
		ret=sk;

	if ((xi=X509_INFO_new()) == NULL) goto err;
	for (;;)
		{
		raw=0;
		i=PEM_read(fp,&name,&header,&data,&len);
		if (i == 0)
			{
			error=ERR_GET_REASON(ERR_peek_error());
			if (error == PEM_R_NO_START_LINE)
				{
				ERR_clear_error();
				break;
				}
			goto err;
			}
start:
		if (strcmp(name,PEM_STRING_X509) == 0)
			{
			d2i=(char *(*)())d2i_X509;
			pp=(char **)&(xi->x509);
			}
		else if (strcmp(name,PEM_STRING_X509_CRL) == 0)
			{
			d2i=(char *(*)())d2i_X509_CRL;
			pp=(char **)&(xi->crl);
			}
		else if (strcmp(name,PEM_STRING_RSA) == 0)
			{
			d2i=(char *(*)())d2i_RSAPrivateKey;
			pp=(char **)&(xi->rsa_data);
			if ((int)strlen(header) > 10) /* assume encrypted */
				raw=1;
			}
		else
			{
			d2i=NULL;
			pp=NULL;
			}

		if (d2i != NULL)
			{
			if ((*pp != NULL) || ((xi->rsa_data != NULL)
				&& (xi->rsa != NULL)))
				{
				if (!sk_push(sk,(char *)xi)) goto err;
				if ((xi=X509_INFO_new()) == NULL) goto err;
				goto start;
				}
			if (!raw)
				{
				EVP_CIPHER_INFO cipher;

				if (!PEM_get_EVP_CIPHER_INFO(header,&cipher))
					goto err;
				if (!PEM_do_header(&cipher,data,&len,cb))
					goto err;
				p=data;
				if (d2i(pp,&p,len) == NULL)
					{
					PEMerr(PEM_F_PEM_X509_INFO_READ,ERR_R_ASN1_LIB);
					goto err;
					}
				}
			else
				{ /* encrypted RSA data */
				if (!PEM_get_EVP_CIPHER_INFO(header,
					&xi->rsa_cipher)) goto err;
				xi->rsa_data=data;
				xi->rsa_len=len;
				data=NULL;
				}
			}
		else	{
			/* unknown */
			}
		if (name != NULL) free(name);
		if (header != NULL) free(header);
		if (data != NULL) free(data);
		name=NULL;
		header=NULL;
		data=NULL;
		}
	if ((xi->x509 != NULL) || (xi->crl != NULL) ||
		(xi->rsa != NULL) || (xi->rsa_data != NULL))
		{
		if (!sk_push(sk,(char *)xi)) goto err;
		xi=NULL;
		}
	ok=1;
err:
	if (xi != NULL) X509_INFO_free(xi);
	if (!ok && (ret != sk))
		{
		for (i=0; i<sk_num(sk); i++)
			{
			xi=(X509_INFO *)sk_value(sk,i);
			X509_INFO_free(xi);
			}
		sk_free(ret);
		}
		
	if (name != NULL) free(name);
	if (header != NULL) free(header);
	if (data != NULL) free(data);
	return(ret);
	}

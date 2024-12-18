/* lib/asn1/a_bitstr.c */
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
#include "asn1.h"

/* ASN1err(ASN1_F_ASN1_BIT_STRING_NEW,ASN1_R_BIT_STRING_TOO_SHORT);
 * ASN1err(ASN1_F_ASN1_BIT_STRING_NEW,ASN1_R_EXPECTING_A_BIT_STRING);
 */

int i2d_ASN1_BIT_STRING(a,pp)
ASN1_BIT_STRING *a;
unsigned char **pp;
	{
	int ret,j,r,bits;
	unsigned char *p,*d;

	if (a == NULL) return(0);

	/* our bit strings are always a multiple of 8 :-) */
	bits=0;
	ret=1+a->length;
	r=ASN1_object_size(0,ret,V_ASN1_BIT_STRING);
	if (pp == NULL) return(r);
	p= *pp;

	ASN1_put_object(&p,0,ret,V_ASN1_BIT_STRING,V_ASN1_UNIVERSAL);
	if (bits == 0)
		j=0;
	else	j=8-bits;
	*(p++)=(unsigned char)j;
	d=a->data;
	memcpy(p,d,a->length);
	p+=a->length;
	if (a->length > 0) p[-1]&=(0xff<<j);
	*pp=p;
	return(r);
	}

ASN1_BIT_STRING *d2i_ASN1_BIT_STRING(a, pp, length)
ASN1_BIT_STRING **a;
unsigned char **pp;
long length;
	{
	ASN1_BIT_STRING *ret=NULL;
	unsigned char *p,*s,*max;
	long len;
	int inf,tag,class;
	int i;

	if ((a == NULL) || ((*a) == NULL))
		{
		if ((ret=ASN1_BIT_STRING_new()) == NULL) return(NULL);
		}
	else
		ret=(*a);

	p= *pp;
	max=(p+length);
	inf=ASN1_get_object(&p,&len,&tag,&class,length);
	if (inf == 0xff)
		{
		i=ASN1_R_BAD_OBJECT_HEADER;
		goto err;
		}

	if (tag != V_ASN1_BIT_STRING)
		{
		i=ASN1_R_EXPECTING_A_BIT_STRING;
		goto err;
		}
	if (len < 1) { i=ASN1_R_BIT_STRING_TOO_SHORT; goto err; }

	i= *(p++);
	if (len-- > 1) /* using one because of the bits left byte */
		{
		s=(unsigned char *)malloc(len);
		if (s == NULL)
			{
			i=ERR_R_MALLOC_FAILURE;
			goto err;
			}
		memcpy(s,p,len);
		s[len-1]&=(0xff<<i);
		p+=len;
		}
	else
		s=NULL;

	ret->length=len;
	if (ret->data != NULL) free(ret->data);
	ret->data=s;
	ret->type=V_ASN1_BIT_STRING;
	if (a != NULL) (*a)=ret;
	*pp=p;
	return(ret);
err:
	ASN1err(ASN1_F_D2I_ASN1_BIT_STRING,i);
	if ((ret != NULL) && (a != NULL) && (*a != ret))
		ASN1_BIT_STRING_free(ret);
	return(NULL);
	}

ASN1_BIT_STRING *ASN1_BIT_STRING_type_new(type)
int type;
	{
	ASN1_BIT_STRING *ret;

	ret=(ASN1_BIT_STRING *)malloc(sizeof(ASN1_BIT_STRING));
	if (ret == NULL)
		{
		ASN1err(ASN1_F_ASN1_BIT_STRING_NEW,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	ret->length=0;
	ret->type=type;
	ret->data=NULL;
	return(ret);
	}

void ASN1_BIT_STRING_free(a)
ASN1_BIT_STRING *a;
	{
	if (a == NULL) return;
	if (a->data != NULL) free(a->data);
	free(a);
	}

int ASN1_BIT_STRING_cmp(a,b)
ASN1_BIT_STRING *a,*b;
	{
	int i;

	i=(a->length-b->length);
	if (i == 0)
		{
		i=memcmp(a->data,b->data,a->length);
		if (i == 0)
			return(a->type-b->type);
		else
			return(i);
		}
	else
		return(i);
	}

/* v3_genn.c */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 1999.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */


#include <stdio.h>
#include "cryptlib.h"
#include "asn1.h"
#include "asn1_mac.h"
#include "conf.h"
#include "x509v3.h"

/*
 * ASN1err(ASN1_F_GENERAL_NAME_NEW,ERR_R_MALLOC_FAILURE);
 * ASN1err(ASN1_F_D2I_GENERAL_NAME,ERR_R_MALLOC_FAILURE);
 */

int i2d_GENERAL_NAME(a,pp)
GENERAL_NAME *a;
unsigned char **pp;
{
	unsigned char *p;
	int ret;

	ret = 0;

	/* Save the location of initial TAG */
	if(pp) p = *pp;
	else p = NULL;

	/* GEN_DNAME needs special treatment because of EXPLICIT tag */

	if(a->type == GEN_DIRNAME) {
		int v = 0;
		M_ASN1_I2D_len_EXP_opt(a->d.dirn, i2d_X509_NAME, 4, v);
		if(!p) return ret;
		M_ASN1_I2D_put_EXP_opt(a->d.dirn, i2d_X509_NAME, 4, v);
		*pp = p;
		return ret;
	}

	switch(a->type) {

		case GEN_OTHERNAME:
		case GEN_X400:
		case GEN_EDIPARTY:
		ret = i2d_ASN1_TYPE(a->d.other, pp);
		break;

		case GEN_EMAIL:
		case GEN_DNS:
		case GEN_URI:
		ret = i2d_ASN1_IA5STRING(a->d.ia5, pp);
		break;

		case GEN_IPADD:
		ret = i2d_ASN1_OCTET_STRING(a->d.ip, pp);
		break;
	
		case GEN_RID:
		ret = i2d_ASN1_OBJECT(a->d.rid, pp);
		break;
	}
	/* Replace TAG with IMPLICIT value */
	if(p) *p = (*p & V_ASN1_CONSTRUCTED) | a->type;
	return ret;
}

GENERAL_NAME *GENERAL_NAME_new()
{
	GENERAL_NAME *ret=NULL;
	ASN1_CTX c;
	M_ASN1_New_Malloc(ret, GENERAL_NAME);
	ret->type = -1;
	ret->d.ptr = NULL;
	return (ret);
	M_ASN1_New_Error(ASN1_F_GENERAL_NAME_NEW);
}

GENERAL_NAME *d2i_GENERAL_NAME(a,pp,length)
GENERAL_NAME **a;
unsigned char **pp;
long length;
{
	unsigned char _tmp;
	M_ASN1_D2I_vars(a,GENERAL_NAME *,GENERAL_NAME_new);
	M_ASN1_D2I_Init();
	c.slen = length;

	_tmp = M_ASN1_next;
	ret->type = _tmp & ~V_ASN1_CONSTRUCTED;

	switch(ret->type) {
		/* Just put these in a "blob" for now */
		case GEN_OTHERNAME:
		case GEN_X400:
		case GEN_EDIPARTY:
		M_ASN1_D2I_get_imp(ret->d.other, d2i_ASN1_TYPE,V_ASN1_SEQUENCE);
		break;

		case GEN_EMAIL:
		case GEN_DNS:
		case GEN_URI:
		M_ASN1_D2I_get_imp(ret->d.ia5, d2i_ASN1_IA5STRING,
							V_ASN1_IA5STRING);
		break;

		case GEN_DIRNAME:
		M_ASN1_D2I_get_EXP_opt(ret->d.dirn, d2i_X509_NAME, 4);
		break;

		case GEN_IPADD:
		M_ASN1_D2I_get_imp(ret->d.ip, d2i_ASN1_OCTET_STRING,
							V_ASN1_OCTET_STRING);
		break;
	
		case GEN_RID:
		M_ASN1_D2I_get_imp(ret->d.rid, d2i_ASN1_OBJECT,V_ASN1_OBJECT);
		break;

		default:
		c.error = ASN1_R_BAD_TAG;
		goto err;
	}

	c.slen = 0;
	M_ASN1_D2I_Finish(a, GENERAL_NAME_free, ASN1_F_D2I_GENERAL_NAME);
}

void GENERAL_NAME_free(a)
GENERAL_NAME *a;
{
	if (a == NULL) return;
	switch(a->type) {
		case GEN_OTHERNAME:
		case GEN_X400:
		case GEN_EDIPARTY:
		ASN1_TYPE_free(a->d.other);
		break;

		case GEN_EMAIL:
		case GEN_DNS:
		case GEN_URI:

		ASN1_IA5STRING_free(a->d.ia5);
		break;

		case GEN_DIRNAME:
		X509_NAME_free(a->d.dirn);
		break;

		case GEN_IPADD:
		ASN1_OCTET_STRING_free(a->d.ip);
		break;
	
		case GEN_RID:
		ASN1_OBJECT_free(a->d.rid);
		break;

	}
	Free ((char *)a);
}

/* Now the GeneralNames versions: a SEQUENCE OF GeneralName These are needed as
 * an explicit functions.
 */

STACK *GENERAL_NAMES_new()
{
	return sk_new(NULL);
}

void GENERAL_NAMES_free(a)
STACK *a;
{
	sk_pop_free(a, GENERAL_NAME_free);
}

STACK *d2i_GENERAL_NAMES(a,pp,length)
STACK **a;
unsigned char **pp;
long length;
{
return d2i_ASN1_SET(a, pp, length, (char *(*)())d2i_GENERAL_NAME,
			 GENERAL_NAME_free, V_ASN1_SEQUENCE, V_ASN1_UNIVERSAL);
}

int i2d_GENERAL_NAMES(a,pp)
STACK *a;
unsigned char **pp;
{
return i2d_ASN1_SET(a, pp, i2d_GENERAL_NAME, V_ASN1_SEQUENCE,
						 V_ASN1_UNIVERSAL, IS_SEQUENCE);
}

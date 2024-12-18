/* lib/asn1/x_cinf.c */
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
#include "asn1_mac.h"

/*
 * ASN1err(ASN1_F_D2I_X509_CINF,ASN1_R_LENGTH_MISMATCH);
 * ASN1err(ASN1_F_X509_CINF_NEW,ASN1_R_LENGTH_MISMATCH);
 */

int i2d_X509_CINF(a,pp)
X509_CINF *a;
unsigned char **pp;
	{
	int v=0;
	M_ASN1_I2D_vars(a);

	M_ASN1_I2D_len_opt(a->version,		i2d_ASN1_INTEGER);
	M_ASN1_I2D_len(a->serialNumber,		i2d_ASN1_INTEGER);
	M_ASN1_I2D_len(a->signature,		i2d_X509_ALGOR);
	M_ASN1_I2D_len(a->issuer,		i2d_X509_NAME);
	M_ASN1_I2D_len(a->validity,		i2d_X509_VAL);
	M_ASN1_I2D_len(a->subject,		i2d_X509_NAME);
	M_ASN1_I2D_len(a->key,			i2d_X509_PUBKEY);
	M_ASN1_I2D_len_opt(a->issuerUID,	i2d_ASN1_BIT_STRING);
	M_ASN1_I2D_len_opt(a->subjectUID,	i2d_ASN1_BIT_STRING);
	M_ASN1_I2D_len_EXP_set_opt(a->attributes,i2d_X509_ATTRIBUTE,3,
		V_ASN1_SEQUENCE,v);

	M_ASN1_I2D_seq_total();

	M_ASN1_I2D_put_opt(a->version,		i2d_ASN1_INTEGER,0);
	M_ASN1_I2D_put(a->serialNumber,		i2d_ASN1_INTEGER);
	M_ASN1_I2D_put(a->signature,		i2d_X509_ALGOR);
	M_ASN1_I2D_put(a->issuer,		i2d_X509_NAME);
	M_ASN1_I2D_put(a->validity,		i2d_X509_VAL);
	M_ASN1_I2D_put(a->subject,		i2d_X509_NAME);
	M_ASN1_I2D_put(a->key,			i2d_X509_PUBKEY);
	M_ASN1_I2D_put_opt(a->issuerUID,	i2d_ASN1_INTEGER,1);
	M_ASN1_I2D_put_opt(a->subjectUID,	i2d_ASN1_INTEGER,2);
	M_ASN1_I2D_put_EXP_set_opt(a->attributes,i2d_X509_ATTRIBUTE,3,
		V_ASN1_SEQUENCE,v);

	M_ASN1_I2D_finish();
	}

X509_CINF *d2i_X509_CINF(a,pp,length)
X509_CINF **a;
unsigned char **pp;
long length;
	{
	int ver=0;
	M_ASN1_D2I_vars(a,X509_CINF *,X509_CINF_new);

	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	/* we have the optional version field */
	if (M_ASN1_next == (V_ASN1_CONTEXT_SPECIFIC | 0))
		{
		M_ASN1_next=(V_ASN1_UNIVERSAL|V_ASN1_INTEGER);
		M_ASN1_D2I_get(ret->version,d2i_ASN1_INTEGER);
		M_ASN1_next_prev=(V_ASN1_CONTEXT_SPECIFIC|0);
		if (ret->version->data != NULL)
			ver=ret->version->data[0];
		}
	M_ASN1_D2I_get(ret->serialNumber,d2i_ASN1_INTEGER);
	M_ASN1_D2I_get(ret->signature,d2i_X509_ALGOR);
	M_ASN1_D2I_get(ret->issuer,d2i_X509_NAME);
	M_ASN1_D2I_get(ret->validity,d2i_X509_VAL);
	M_ASN1_D2I_get(ret->subject,d2i_X509_NAME);
	M_ASN1_D2I_get(ret->key,d2i_X509_PUBKEY);
	if (ver+1 > 0) /* version 2 extensions */
		{
		M_ASN1_D2I_get_opt(ret->issuerUID,d2i_ASN1_BIT_STRING,  1,
			V_ASN1_BIT_STRING);
		M_ASN1_D2I_get_opt(ret->subjectUID,d2i_ASN1_BIT_STRING, 2,
			V_ASN1_BIT_STRING);
		}
	if (ver+2 > 1) /* version 3 extensions */
		{
		M_ASN1_D2I_get_set_exp_opt(
			ret->attributes,d2i_X509_ATTRIBUTE,3,V_ASN1_SEQUENCE);
		}
	M_ASN1_D2I_Finish(a,X509_CINF_free,ASN1_F_D2I_X509_CINF);
	}

X509_CINF *X509_CINF_new()
	{
	X509_CINF *ret=NULL;

	M_ASN1_New_Malloc(ret,X509_CINF);
	ret->version=NULL;
	M_ASN1_New(ret->serialNumber,ASN1_INTEGER_new);
	M_ASN1_New(ret->signature,X509_ALGOR_new);
	M_ASN1_New(ret->issuer,X509_NAME_new);
	M_ASN1_New(ret->validity,X509_VAL_new);
	M_ASN1_New(ret->subject,X509_NAME_new);
	M_ASN1_New(ret->key,X509_PUBKEY_new);
	ret->issuerUID=NULL;
	ret->subjectUID=NULL;
	ret->attributes=NULL;
	return(ret);
	M_ASN1_New_Error(ASN1_F_X509_CINF_NEW);
	}

void X509_CINF_free(a)
X509_CINF *a;
	{
	if (a == NULL) return;
	ASN1_INTEGER_free(a->version);
	ASN1_INTEGER_free(a->serialNumber);
	X509_ALGOR_free(a->signature);
	X509_NAME_free(a->issuer);
	X509_VAL_free(a->validity);
	X509_NAME_free(a->subject);
	X509_PUBKEY_free(a->key);
	ASN1_BIT_STRING_free(a->issuerUID);
	ASN1_BIT_STRING_free(a->subjectUID);
	sk_pop_free(a->attributes,	X509_ATTRIBUTE_free);
	free(a);
	}


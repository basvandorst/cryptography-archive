/* lib/asn1/x_req.c */
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
#include "x509.h"

/*
 * ASN1err(ASN1_F_D2I_X509_REQ,ASN1_R_LENGTH_MISMATCH);
 * ASN1err(ASN1_F_D2I_X509_REQ_INFO,ASN1_R_LENGTH_MISMATCH);
 * ASN1err(ASN1_F_X509_REQ_NEW,ASN1_R_LENGTH_MISMATCH);
 * ASN1err(ASN1_F_X509_REQ_INFO_NEW,ASN1_R_LENGTH_MISMATCH);
 */

int i2d_X509_REQ_INFO(a,pp)
X509_REQ_INFO *a;
unsigned char **pp;
	{
	M_ASN1_I2D_vars(a);

	M_ASN1_I2D_len(a->version,		i2d_ASN1_INTEGER);
	M_ASN1_I2D_len(a->subject,		i2d_X509_NAME);
	M_ASN1_I2D_len(a->pubkey,		i2d_X509_PUBKEY);
	M_ASN1_I2D_len_IMP_set(a->attributes,	i2d_X509_ATTRIBUTE,0);
	
	M_ASN1_I2D_seq_total();
	M_ASN1_I2D_put(a->version,		i2d_ASN1_INTEGER);
	M_ASN1_I2D_put(a->subject,		i2d_X509_NAME);
	M_ASN1_I2D_put(a->pubkey,		i2d_X509_PUBKEY);
	M_ASN1_I2D_put_IMP_set(a->attributes,	i2d_X509_ATTRIBUTE,0);

	M_ASN1_I2D_finish();
	}

X509_REQ_INFO *d2i_X509_REQ_INFO(a,pp,length)
X509_REQ_INFO **a;
unsigned char **pp;
long length;
	{
	M_ASN1_D2I_vars(a,X509_REQ_INFO *,X509_REQ_INFO_new);

	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get(ret->version,d2i_ASN1_INTEGER);
	M_ASN1_D2I_get(ret->subject,d2i_X509_NAME);
	M_ASN1_D2I_get(ret->pubkey,d2i_X509_PUBKEY);
	M_ASN1_D2I_get_IMP_set(ret->attributes,d2i_X509_ATTRIBUTE,0);
	M_ASN1_D2I_Finish(a,X509_REQ_INFO_free,ASN1_F_D2I_X509_REQ_INFO);
	}

X509_REQ_INFO *X509_REQ_INFO_new()
	{
	X509_REQ_INFO *ret=NULL;

	M_ASN1_New_Malloc(ret,X509_REQ_INFO);
	M_ASN1_New(ret->version,ASN1_INTEGER_new);
	M_ASN1_New(ret->subject,X509_NAME_new);
	M_ASN1_New(ret->pubkey,X509_PUBKEY_new);
	M_ASN1_New(ret->attributes,sk_new_null);
	return(ret);
	M_ASN1_New_Error(ASN1_F_X509_REQ_INFO_NEW);
	}
	
void X509_REQ_INFO_free(a)
X509_REQ_INFO *a;
	{
	if (a == NULL) return;
	ASN1_INTEGER_free(a->version);
	X509_NAME_free(a->subject);
	X509_PUBKEY_free(a->pubkey);
	sk_pop_free(a->attributes,X509_ATTRIBUTE_free);
	free(a);
	}

int i2d_X509_REQ(a,pp)
X509_REQ *a;
unsigned char **pp;
	{
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len(a->req_info,	i2d_X509_REQ_INFO);
	M_ASN1_I2D_len(a->sig_alg,	i2d_X509_ALGOR);
	M_ASN1_I2D_len(a->signature,	i2d_ASN1_BIT_STRING);

	M_ASN1_I2D_seq_total();

	M_ASN1_I2D_put(a->req_info,	i2d_X509_REQ_INFO);
	M_ASN1_I2D_put(a->sig_alg,	i2d_X509_ALGOR);
	M_ASN1_I2D_put(a->signature,	i2d_ASN1_BIT_STRING);

	M_ASN1_I2D_finish();
	}

X509_REQ *d2i_X509_REQ(a,pp,length)
X509_REQ **a;
unsigned char **pp;
long length;
	{
	M_ASN1_D2I_vars(a,X509_REQ *,X509_REQ_new);

	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get(ret->req_info,d2i_X509_REQ_INFO);
	M_ASN1_D2I_get(ret->sig_alg,d2i_X509_ALGOR);
	M_ASN1_D2I_get(ret->signature,d2i_ASN1_BIT_STRING);
	M_ASN1_D2I_Finish(a,X509_REQ_free,ASN1_F_D2I_X509_REQ);
	}

X509_REQ *X509_REQ_new()
	{
	X509_REQ *ret=NULL;

	M_ASN1_New_Malloc(ret,X509_REQ);
	ret->references=1;
	M_ASN1_New(ret->req_info,X509_REQ_INFO_new);
	M_ASN1_New(ret->sig_alg,X509_ALGOR_new);
	M_ASN1_New(ret->signature,ASN1_BIT_STRING_new);
	return(ret);
	M_ASN1_New_Error(ASN1_F_X509_REQ_NEW);
	}

void X509_REQ_free(a)
X509_REQ *a;
	{
	if (a == NULL) return;
	if (--a->references > 0) return;
	X509_REQ_INFO_free(a->req_info);
	X509_ALGOR_free(a->sig_alg);
	ASN1_BIT_STRING_free(a->signature);
	free(a);
	}



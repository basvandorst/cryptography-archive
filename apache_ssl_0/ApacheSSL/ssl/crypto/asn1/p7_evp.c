/* lib/asn1/p7_evp.c */
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
 * ASN1err(ASN1_F_PKCS7_ENVELOPE_NEW,ASN1_R_LENGTH_MISMATCH);
 * ASN1err(ASN1_F_D2I_PKCS7_ENVELOPE,ASN1_R_LENGTH_MISMATCH);
 */

int i2d_PKCS7_ENVELOPE(a,pp)
PKCS7_ENVELOPE *a;
unsigned char **pp;
	{
	M_ASN1_I2D_vars(a);

	M_ASN1_I2D_len(a->version,i2d_ASN1_INTEGER);
	M_ASN1_I2D_len_SET(a->recipientinfo,i2d_PKCS7_RECIP_INFO);
	M_ASN1_I2D_len(a->enc_data,i2d_PKCS7_ENC_CONTENT);

	M_ASN1_I2D_seq_total();

	M_ASN1_I2D_put(a->version,i2d_ASN1_INTEGER);
	M_ASN1_I2D_put_SET(a->recipientinfo,i2d_PKCS7_RECIP_INFO);
	M_ASN1_I2D_put(a->enc_data,i2d_PKCS7_ENC_CONTENT);

	M_ASN1_I2D_finish();
	}

PKCS7_ENVELOPE *d2i_PKCS7_ENVELOPE(a,pp,length)
PKCS7_ENVELOPE **a;
unsigned char **pp;
long length;
	{
	M_ASN1_D2I_vars(a,PKCS7_ENVELOPE *,PKCS7_ENVELOPE_new);

	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get(ret->version,d2i_ASN1_INTEGER);
	M_ASN1_D2I_get_set(ret->recipientinfo,d2i_PKCS7_RECIP_INFO);
	M_ASN1_D2I_get(ret->enc_data,d2i_PKCS7_ENC_CONTENT);

	M_ASN1_D2I_Finish(a,PKCS7_ENVELOPE_free,ASN1_F_D2I_PKCS7_ENVELOPE);
	}

PKCS7_ENVELOPE *PKCS7_ENVELOPE_new()
	{
	PKCS7_ENVELOPE *ret=NULL;

	M_ASN1_New_Malloc(ret,PKCS7_ENVELOPE);
	M_ASN1_New(ret->version,ASN1_INTEGER_new);
	M_ASN1_New(ret->recipientinfo,sk_new_null);
	M_ASN1_New(ret->enc_data,PKCS7_ENC_CONTENT_new);
	return(ret);
	M_ASN1_New_Error(ASN1_F_PKCS7_ENVELOPE_NEW);
	}

void PKCS7_ENVELOPE_free(a)
PKCS7_ENVELOPE *a;
	{
	if (a == NULL) return;
	ASN1_INTEGER_free(a->version);
	sk_pop_free(a->recipientinfo,PKCS7_RECIP_INFO_free);
	PKCS7_ENC_CONTENT_free(a->enc_data);
	free(a);
	}


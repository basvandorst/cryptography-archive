/* lib/asn1/x_pubkey.c */
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
 * ASN1err(ASN1_F_D2I_X509_PUBKEY,ASN1_R_LENGTH_MISMATCH);
 * ASN1err(ASN1_F_X509_PUBKEY_NEW,ASN1_R_LENGTH_MISMATCH);
 */

int i2d_X509_PUBKEY(a,pp)
X509_PUBKEY *a;
unsigned char **pp;
	{
	M_ASN1_I2D_vars(a);


	M_ASN1_I2D_len(a->algor,	i2d_X509_ALGOR);
	M_ASN1_I2D_len(a->public_key,	i2d_ASN1_BIT_STRING);

	M_ASN1_I2D_seq_total();

	M_ASN1_I2D_put(a->algor,	i2d_X509_ALGOR);
	M_ASN1_I2D_put(a->public_key,	i2d_ASN1_BIT_STRING);

	M_ASN1_I2D_finish();
	}

X509_PUBKEY *d2i_X509_PUBKEY(a,pp,length)
X509_PUBKEY **a;
unsigned char **pp;
long length;
	{
	M_ASN1_D2I_vars(a,X509_PUBKEY *,X509_PUBKEY_new);

	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get(ret->algor,d2i_X509_ALGOR);
	M_ASN1_D2I_get(ret->public_key,d2i_ASN1_BIT_STRING);
	M_ASN1_D2I_Finish(a,X509_PUBKEY_free,ASN1_F_D2I_X509_PUBKEY);
	}

X509_PUBKEY *X509_PUBKEY_new()
	{
	X509_PUBKEY *ret=NULL;

	M_ASN1_New_Malloc(ret,X509_PUBKEY);
	M_ASN1_New(ret->algor,X509_ALGOR_new);
	M_ASN1_New(ret->public_key,ASN1_BIT_STRING_new);
	return(ret);
	M_ASN1_New_Error(ASN1_F_X509_PUBKEY_NEW);
	}

void X509_PUBKEY_free(a)
X509_PUBKEY *a;
	{
	if (a == NULL) return;
	X509_ALGOR_free(a->algor);
	ASN1_BIT_STRING_free(a->public_key);
	free(a);
	}



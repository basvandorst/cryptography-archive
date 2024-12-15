/* Copyright Dr Stephen Henson 1997. All rights reserved.
 * The conditions of use are identical to those of SSLeay except that my
 * name (in addition to Eric Young or Tim Hudson) must be mentioned
 * if this software is used in a product and the notice "This product
 * includes software written by Dr Stephen Henson (shenson@bigfoot.com)"
 * used.
 */
#include <stdio.h>
#include <stdlib.h>
#include <objects.h>
#include <asn1.h>
#include <asn1_mac.h>
#include <x509.h>
#include <pkcs7.h>
#include <err.h>
#include <stack.h>
#include <crypto.h>
#include "pfx.h"

int i2d_PFX(a,pp)
PFX *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);

	M_ASN1_I2D_len_IMP_opt (a->mac, i2d_MAC_DATA);
	M_ASN1_I2D_len_IMP_opt (a->authsafe, i2d_PKCS7);

	M_ASN1_I2D_seq_total();

	M_ASN1_I2D_put_IMP_opt (a->mac, i2d_MAC_DATA, 0 | V_ASN1_CONSTRUCTED);
	M_ASN1_I2D_put_IMP_opt (a->authsafe, i2d_PKCS7, 1 | V_ASN1_CONSTRUCTED);

	M_ASN1_I2D_finish();
}

PFX *d2i_PFX(a,pp,length)
PFX **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,PFX *,PFX_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get_IMP_opt (ret->mac, d2i_MAC_DATA, 0, V_ASN1_SEQUENCE);
	M_ASN1_D2I_get_IMP_opt (ret->authsafe, d2i_PKCS7, 1, V_ASN1_SEQUENCE);
	M_ASN1_D2I_Finish(a, PFX_free, 500);
}

PFX *PFX_new()
{
	PFX *ret=NULL;
	M_ASN1_New_Malloc(ret, PFX);
	ret->mac=NULL;
	ret->authsafe=NULL;
	ret->bags = NULL;
	ret->safe_cont = NULL;
	return (ret);
	M_ASN1_New_Error(501);
}

void PFX_free (a)
PFX *a;
{
	if (a == NULL) return;
	MAC_DATA_free (a->mac);
	PKCS7_free (a->authsafe);
	Free ((char *)a);
}

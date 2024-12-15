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

int i2d_PFX_AUTHSAFE(a,pp)
AUTHSAFE *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len (a->version, i2d_ASN1_INTEGER);
	M_ASN1_I2D_len (a->mode, i2d_ASN1_OBJECT);
	M_ASN1_I2D_len (a->psalt, i2d_ASN1_BIT_STRING);
	M_ASN1_I2D_len_SET (a->bagitms, i2d_BAGITEM);
	M_ASN1_I2D_len (a->safe, i2d_PKCS7);

	M_ASN1_I2D_seq_total ();	

	M_ASN1_I2D_put (a->version, i2d_ASN1_INTEGER);
	M_ASN1_I2D_put (a->mode, i2d_ASN1_OBJECT);
	M_ASN1_I2D_put (a->psalt, i2d_ASN1_BIT_STRING);
	M_ASN1_I2D_put_SET (a->bagitms, i2d_BAGITEM);
	M_ASN1_I2D_put (a->safe, i2d_PKCS7);
	M_ASN1_I2D_finish();
}

AUTHSAFE *d2i_PFX_AUTHSAFE(a,pp,length)
AUTHSAFE **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,AUTHSAFE *,PFX_AUTHSAFE_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get (ret->version, d2i_ASN1_INTEGER);
	M_ASN1_D2I_get (ret->mode, d2i_ASN1_OBJECT);
	M_ASN1_D2I_get (ret->psalt, d2i_ASN1_BIT_STRING);
	M_ASN1_D2I_get_set (ret->bagitms, d2i_BAGITEM);
	M_ASN1_D2I_get (ret->safe, d2i_PKCS7);
	M_ASN1_D2I_Finish(a, PFX_AUTHSAFE_free, 500);
}

AUTHSAFE *PFX_AUTHSAFE_new()
{
	AUTHSAFE *ret=NULL;
	M_ASN1_New_Malloc(ret, AUTHSAFE);
	M_ASN1_New(ret->version,ASN1_INTEGER_new);
	ret->mode = NULL;
	ret->psalt = NULL;
	ret->safe = NULL;
	ret->bagitms = NULL;
	return (ret);
	M_ASN1_New_Error(500);
}

void PFX_AUTHSAFE_free (a)
AUTHSAFE *a;
{
	if (a == NULL) return;
	ASN1_INTEGER_free (a->version);
	ASN1_OBJECT_free (a->mode);
	ASN1_BIT_STRING_free (a->psalt);
	sk_pop_free (a->bagitms, BAGITEM_free);
	PKCS7_free (a->safe);
	Free ((char *)a);
}

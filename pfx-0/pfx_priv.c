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

int i2d_PRIV_KEY_INFO(a,pp)
PRIV_KEY_INFO *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);

	M_ASN1_I2D_len (a->version, i2d_ASN1_INTEGER);
	M_ASN1_I2D_len (a->pkeyalg, i2d_X509_ALGOR);
	M_ASN1_I2D_len (a->pkey, i2d_ASN1_OCTET_STRING);
	M_ASN1_I2D_len_IMP_set_opt (a->attributes, i2d_ASN1_TYPE, 0);
	
	M_ASN1_I2D_seq_total ();

	M_ASN1_I2D_put (a->version, i2d_ASN1_INTEGER);
	M_ASN1_I2D_put (a->pkeyalg, i2d_X509_ALGOR);
	M_ASN1_I2D_put (a->pkey, i2d_ASN1_OCTET_STRING);
	M_ASN1_I2D_put_IMP_set_opt (a->attributes, i2d_ASN1_TYPE, 0);

	M_ASN1_I2D_finish();
}

PRIV_KEY_INFO *PRIV_KEY_INFO_new()
{
	PRIV_KEY_INFO *ret=NULL;
	M_ASN1_New_Malloc(ret, PRIV_KEY_INFO);
	M_ASN1_New (ret->version, ASN1_INTEGER_new);
	M_ASN1_New (ret->pkeyalg, X509_ALGOR_new);
	M_ASN1_New (ret->pkey, ASN1_OCTET_STRING_new);
	ret->attributes = NULL;
	return (ret);
	M_ASN1_New_Error(502);
}

PRIV_KEY_INFO *d2i_PRIV_KEY_INFO(a,pp,length)
PRIV_KEY_INFO **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,PRIV_KEY_INFO *,PRIV_KEY_INFO_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get (ret->version, d2i_ASN1_INTEGER);
	M_ASN1_D2I_get (ret->pkeyalg, d2i_X509_ALGOR);
	M_ASN1_D2I_get (ret->pkey, d2i_ASN1_OCTET_STRING);
	M_ASN1_D2I_get_IMP_set_opt (ret->attributes, d2i_ASN1_TYPE, 0);
	M_ASN1_D2I_Finish(a, PRIV_KEY_INFO_free, 503);
}

void PRIV_KEY_INFO_free (a)
PRIV_KEY_INFO *a;
{
	if (a == NULL) return;
	ASN1_INTEGER_free (a->version);
	X509_ALGOR_free (a->pkeyalg);
	ASN1_OCTET_STRING_free (a->pkey);
	sk_pop_free (a->attributes, ASN1_TYPE_free);
	Free ((char *)a);
}

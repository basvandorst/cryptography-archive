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

int i2d_ESPVK(a,pp)
ESPVK *a;
unsigned char **pp;
{
	int v1 = 0;
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len (a->espvkoid, i2d_ASN1_OBJECT);
	M_ASN1_I2D_len (a->supdata, i2d_SUP_DATA);
	M_ASN1_I2D_len_EXP_opt (a->cipher, i2d_ENCPKEY, 0, v1);

	M_ASN1_I2D_seq_total ();
	
	M_ASN1_I2D_put (a->espvkoid, i2d_ASN1_OBJECT);
	M_ASN1_I2D_put (a->supdata, i2d_SUP_DATA);
	M_ASN1_I2D_put_EXP_opt (a->cipher, i2d_ENCPKEY, 0, v1);

	M_ASN1_I2D_finish();
}

ESPVK *d2i_ESPVK(a,pp,length)
ESPVK **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,ESPVK *,ESPVK_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get (ret->espvkoid, d2i_ASN1_OBJECT);
	M_ASN1_D2I_get (ret->supdata, d2i_SUP_DATA);
	M_ASN1_D2I_get_EXP_opt (ret->cipher, d2i_ENCPKEY, 0);
	M_ASN1_D2I_Finish(a, ESPVK_free, 504);
}

ESPVK *ESPVK_new ()
{
	ESPVK *ret=NULL;
	M_ASN1_New_Malloc(ret, ESPVK);
	ret->espvkoid = NULL;
	ret->supdata = NULL;
	M_ASN1_New (ret->cipher, ENCPKEY_new);
	ret->pkey = NULL;
	return (ret);
	M_ASN1_New_Error(504);
}

void ESPVK_free (a)
ESPVK *a;
{
	if(a==NULL) return;
	ASN1_OBJECT_free (a->espvkoid);
	SUP_DATA_free (a->supdata);
	ENCPKEY_free (a->cipher);
	PRIV_KEY_INFO_free (a->pkey);
	Free ((char *)a);
}

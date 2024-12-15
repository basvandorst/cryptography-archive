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

int i2d_BAGITEM(a,pp)
BAGITEM *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len_SET (a->espvks, i2d_ESPVK);
	M_ASN1_I2D_len_SET (a->safebags, i2d_SAFEBAG);

	M_ASN1_I2D_seq_total ();
	
	M_ASN1_I2D_put_SET (a->espvks, i2d_ESPVK);
	M_ASN1_I2D_put_SET (a->safebags, i2d_SAFEBAG);
	M_ASN1_I2D_finish();
}

BAGITEM *d2i_BAGITEM(a,pp,length)
BAGITEM **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,BAGITEM *,BAGITEM_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get_set (ret->espvks, d2i_ESPVK);
	M_ASN1_D2I_get_set (ret->safebags, d2i_SAFEBAG);
	M_ASN1_D2I_Finish(a, BAGITEM_free, 504);
}

BAGITEM *BAGITEM_new()
{
	BAGITEM *ret=NULL;
	M_ASN1_New_Malloc(ret, BAGITEM);
	ret->espvks = NULL;
	ret->safebags = NULL;
	return (ret);
	M_ASN1_New_Error(504);
}

void BAGITEM_free (a)
BAGITEM *a;
{
	if (a==NULL) return;
	sk_pop_free (a->espvks, ESPVK_free);
	sk_pop_free (a->safebags, SAFEBAG_free);
	Free ((char *)a);
}

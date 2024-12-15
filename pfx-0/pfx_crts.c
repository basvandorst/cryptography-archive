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

int i2d_CERTS(a,pp)
CERTS *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len (a->cchain, i2d_PKCS7);

	M_ASN1_I2D_seq_total ();

	M_ASN1_I2D_put (a->cchain, i2d_PKCS7);
	M_ASN1_I2D_finish();
}

CERTS *CERTS_new()
{
	CERTS *ret=NULL;
	M_ASN1_New_Malloc(ret, CERTS);
	ret->cchain = NULL;
	return (ret);
	M_ASN1_New_Error(502);
}

CERTS *d2i_CERTS(a,pp,length)
CERTS **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,CERTS *,CERTS_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get (ret->cchain, d2i_PKCS7);
	M_ASN1_D2I_Finish(a, CERTS_free, 503);
}

void CERTS_free (a)
CERTS *a;
{
	if (a == NULL) return;
	PKCS7_free (a->cchain);
	Free ((char *)a);
}

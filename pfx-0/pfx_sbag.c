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

int i2d_SAFEBAG(a,pp)
SAFEBAG *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len (a->type, i2d_ASN1_OBJECT);
	/* Need to check OID in here */
	M_ASN1_I2D_len_SET (a->certs, i2d_CERTCRL);

	M_ASN1_I2D_seq_total ();
	
	M_ASN1_I2D_put (a->type, i2d_ASN1_OBJECT);
	/* Need to check OID in here */
	M_ASN1_I2D_put_SET (a->certs, i2d_CERTCRL);
	M_ASN1_I2D_finish();
}

SAFEBAG *SAFEBAG_new()
{
	SAFEBAG *ret=NULL;
	M_ASN1_New_Malloc(ret, SAFEBAG);
	ret->type=NULL;
	ret->certs=NULL;
	ret->other=NULL;
	return (ret);
	M_ASN1_New_Error(502);
}

SAFEBAG *d2i_SAFEBAG(a,pp,length)
SAFEBAG **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,SAFEBAG *,SAFEBAG_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get (ret->type, d2i_ASN1_OBJECT);
	/* Need to check OID in here */
	M_ASN1_D2I_get_set (ret->certs, d2i_CERTCRL);
	M_ASN1_D2I_Finish(a, SAFEBAG_free, 503);
}

void SAFEBAG_free (a)
SAFEBAG *a;
{
	if (a == NULL) return;
	ASN1_OBJECT_free (a->type);
	sk_pop_free (a->certs, CERTCRL_free);
	Free ((char *)a);
}

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

int i2d_PBEPARAM(a, pp)
PBEPARAM *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len (a->salt, i2d_ASN1_OCTET_STRING);
	M_ASN1_I2D_len (a->iter, i2d_ASN1_INTEGER);

	M_ASN1_I2D_seq_total ();

	M_ASN1_I2D_put (a->salt, i2d_ASN1_OCTET_STRING);
	M_ASN1_I2D_put (a->iter, i2d_ASN1_INTEGER);
	M_ASN1_I2D_finish();
}

PBEPARAM *PBEPARAM_new()
{
	PBEPARAM *ret=NULL;
	M_ASN1_New_Malloc(ret, PBEPARAM);
	ret->salt = NULL;
	M_ASN1_New(ret->iter,ASN1_INTEGER_new);
	M_ASN1_New(ret->salt,ASN1_OCTET_STRING_new);
	return (ret);
	M_ASN1_New_Error(502);
}

PBEPARAM *d2i_PBEPARAM(a,pp,length)
PBEPARAM **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,PBEPARAM *,PBEPARAM_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get (ret->salt, d2i_ASN1_OCTET_STRING);
	M_ASN1_D2I_get (ret->iter, d2i_ASN1_INTEGER);
	M_ASN1_D2I_Finish(a, PBEPARAM_free, 503);
}

void PBEPARAM_free (a)
PBEPARAM *a;
{
	if(a==NULL) return;
	ASN1_OCTET_STRING_free (a->salt);
	ASN1_INTEGER_free (a->iter);
	Free ((char *)a);
}

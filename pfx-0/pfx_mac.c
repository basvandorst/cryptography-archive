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

int i2d_MAC_DATA(a,pp)
MAC_DATA *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len (a->dinfo, i2d_X509_SIG);
	M_ASN1_I2D_len (a->salt, i2d_ASN1_BIT_STRING);

	M_ASN1_I2D_seq_total();

	M_ASN1_I2D_put (a->dinfo, i2d_X509_SIG);
	M_ASN1_I2D_put (a->salt, i2d_ASN1_BIT_STRING);
	M_ASN1_I2D_finish();
}

MAC_DATA *MAC_DATA_new()
{
	MAC_DATA *ret=NULL;
	M_ASN1_New_Malloc(ret, MAC_DATA);
	ret->dinfo = X509_SIG_new();
	ret->salt = NULL;
	return (ret);
	M_ASN1_New_Error(502);
}

MAC_DATA *d2i_MAC_DATA(a,pp,length)
MAC_DATA **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,MAC_DATA *,MAC_DATA_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get (ret->dinfo, d2i_X509_SIG);
	M_ASN1_D2I_get (ret->salt, d2i_ASN1_BIT_STRING);
	M_ASN1_D2I_Finish(a, MAC_DATA_free, 503);
}

void MAC_DATA_free (a)
MAC_DATA *a;
{
if (a == NULL) return;
X509_SIG_free (a->dinfo);
ASN1_BIT_STRING_free (a->salt);
Free ((char *)a);
}

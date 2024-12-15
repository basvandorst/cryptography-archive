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

int i2d_CERTCRL(a,pp)
CERTCRL *a;
unsigned char **pp;
{
	int v1 = 0;
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len (a->bagid, i2d_ASN1_OBJECT);
	M_ASN1_I2D_len_EXP_opt (a->lcerts, i2d_CERTS, 0, v1);

	M_ASN1_I2D_seq_total ();

	M_ASN1_I2D_put (a->bagid, i2d_ASN1_OBJECT);
	M_ASN1_I2D_put_EXP_opt (a->lcerts, i2d_CERTS, 0, v1);
	M_ASN1_I2D_finish();
}

CERTCRL *CERTCRL_new()
{
	CERTCRL *ret=NULL;
	M_ASN1_New_Malloc(ret, CERTCRL);
	ret->bagid = NULL;
	ret->lcerts = NULL;
	return (ret);
	M_ASN1_New_Error(500);
}

CERTCRL *d2i_CERTCRL(a,pp,length)
CERTCRL **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,CERTCRL *,CERTCRL_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get (ret->bagid, d2i_ASN1_OBJECT);
	M_ASN1_D2I_get_EXP_opt (ret->lcerts, d2i_CERTS, 0);
	M_ASN1_D2I_Finish(a, CERTCRL_free, 500);
}

void CERTCRL_free (a)
CERTCRL *a;
{
	if (a == NULL) return;
	ASN1_OBJECT_free (a->bagid);
	CERTS_free (a->lcerts);
	Free ((char *)a);
}

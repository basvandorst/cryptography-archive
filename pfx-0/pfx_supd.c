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

int i2d_SUP_DATA(a,pp)
SUP_DATA *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);
	M_ASN1_I2D_len_SET (a->prints, i2d_X509_SIG);
	/*M_ASN1_I2D_get (a->regen, d2i_ASN1_TYPE);*/
	ret += i2d_ASN1_bytes (a->nickname, NULL, V_ASN1_BMPSTRING,
							 V_ASN1_UNIVERSAL);
	M_ASN1_I2D_len_SEQ_opt (a->additional, i2d_ASN1_TYPE);

	M_ASN1_I2D_seq_total();

	M_ASN1_I2D_put_SET (a->prints, i2d_X509_SIG);
	/*M_ASN1_I2D_put (a->regen, d2i_ASN1_TYPE);*/
	i2d_ASN1_bytes (a->nickname, &p, V_ASN1_BMPSTRING, V_ASN1_UNIVERSAL);
	M_ASN1_I2D_put_SEQ_opt (a->additional, i2d_ASN1_TYPE);
	M_ASN1_I2D_finish();
}

SUP_DATA *SUP_DATA_new()
{
	SUP_DATA *ret=NULL;
	M_ASN1_New_Malloc(ret, SUP_DATA);
	ret->prints = NULL;
	ret->regen = NULL;
	M_ASN1_New(ret->nickname, ASN1_STRING_new);
	ret->additional = NULL;
	return (ret);
	M_ASN1_New_Error(504);
}

SUP_DATA *d2i_SUP_DATA(a,pp,length)
SUP_DATA **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,SUP_DATA *,SUP_DATA_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get_set (ret->prints, d2i_X509_SIG);
	/*M_ASN1_D2I_get (ret->regen, d2i_ASN1_TYPE);*/
	c.q=c.p;
	if(!d2i_ASN1_bytes (&ret->nickname, &c.p, c.slen, V_ASN1_BMPSTRING,
		V_ASN1_UNIVERSAL)) goto err;
	c.slen-=(c.p-c.q);
	M_ASN1_D2I_get_seq_opt (ret->additional, d2i_ASN1_TYPE);
	M_ASN1_D2I_Finish(a, SUP_DATA_free, 503);
}

void SUP_DATA_free (a)
SUP_DATA *a;
{
	if(a==NULL) return;
	sk_pop_free (a->prints, X509_SIG_free);
	sk_pop_free (a->additional, ASN1_TYPE_free);
	Free ((char *)a);
}

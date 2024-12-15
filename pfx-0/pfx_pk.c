/* Copyright Dr Stephen Henson 1997. All rights reserved.
 * The conditions of use are identical to those of SSLeay except that my
 * name (in addition to Eric Young or Tim Hudson) must be mentioned
 * if this software is used in a product and the notice "This product
 * includes software written by Dr Stephen Henson (shenson@bigfoot.com)"
 * used.
 */
#include <stdio.h>
#include <stdlib.h>
#include <bio.h>
#include <objects.h>
#include <asn1.h>
#include <asn1_mac.h>
#include <x509.h>
#include <pkcs7.h>
#include <err.h>
#include <crypto.h>
#include <sha.h>
#include <rc2.h>
#include <stack.h>
#include "pfx.h"

/* Pack and unpack the AUTHSAFE */

AUTHSAFE *pfx_safe_unpack (pfx)
PFX *pfx;
{
    unsigned char *pbuf;
    pbuf =  pfx->authsafe->d.data->data;
    d2i_PFX_AUTHSAFE (&pfx->safe_cont, &pbuf, pfx->authsafe->d.data->length);
    return pfx->safe_cont;
}

int pfx_safe_pack (pfx)
PFX *pfx;
{
	int safelen;
	unsigned char *safe, *p;
	safelen = i2d_PFX_AUTHSAFE (pfx->safe_cont, NULL);
	safe = Malloc (safelen);
	if (!safe) return 1;
	p = safe;
	i2d_PFX_AUTHSAFE (pfx->safe_cont, &p);
	pfx->authsafe->d.data->length = safelen;	
	pfx->authsafe->d.data->data = safe;
	return 0;
}


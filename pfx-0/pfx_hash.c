/* Copyright Dr Stephen Henson 1997. All rights reserved.
 * The conditions of use are identical to those of SSLeay except that my
 * name (in addition to Eric Young or Tim Hudson) must be mentioned
 * if this software is used in a product and the notice "This product
 * includes software written by Dr Stephen Henson (shenson@bigfoot.com)"
 * used.
 */
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <crypto.h>
#include <evp.h>
#include <string.h>
#include <pkcs7.h>
#include "hmac.h"
#include "pfx.h"

/* Implementation of p_hash function described in TLS spec */
/* Note this is a "broken" version for compatability with Netscape */

void pfx_P_HASH (secret, secretlen, seed, seedlen, key, count, md_type)
unsigned char *secret, *seed, *key;
int seedlen, secretlen, count;
EVP_MD *md_type;
{
	int diglen, i;
	unsigned char an[EVP_MAX_MD_SIZE];
	HMAC_CTX hmac;
	diglen = EVP_MD_size (md_type);
	memset (an, 0, diglen);
	memcpy (an, seed, seedlen);
	for (i = 0; i < count; i++) {
		HMAC_Init (&hmac, secret, secretlen, md_type);
		HMAC_Update (&hmac, an, diglen);
		HMAC_Update (&hmac, seed, seedlen);
		HMAC_Final (&hmac, key, NULL);
		key += diglen;
		HMAC_Init (&hmac, secret, secretlen, md_type);
		HMAC_Update (&hmac, an, diglen);
		HMAC_Final (&hmac, an, NULL);
	}
}

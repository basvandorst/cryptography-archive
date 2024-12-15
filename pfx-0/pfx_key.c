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

void pfx_keyiv_gen (pass, passlen, salt, saltlen, iter, iv, ivlen,
						 key, keylen, md_type)
unsigned char *pass, *salt, *iv, *key;
int passlen, saltlen, ivlen, keylen;
EVP_MD *md_type;
{
	int diglen, count;
	unsigned char *keyiv, digtmp[EVP_MAX_MD_SIZE];
	diglen = EVP_MD_size (md_type);
	count = (keylen + ivlen + diglen - 1) / diglen;
	keyiv = Malloc (count * diglen);
	pfx_pkcs5 (pass, passlen, salt, saltlen, iter, digtmp, md_type);
	if (count > 1) 
	      pfx_P_HASH (digtmp, diglen, salt, saltlen, keyiv, count, md_type);
	else memcpy (keyiv, digtmp, diglen);
	if (keylen) memcpy (key, keyiv, keylen);
	if (ivlen) memcpy (iv, keyiv + (count * diglen - ivlen), ivlen);
	Free (keyiv);
}


void pfx_pkcs5 (pass, passlen, salt, saltlen, iter, md, md_type)
unsigned char *pass;
int passlen;
unsigned char *salt;
int saltlen;
int iter;
unsigned char *md;
EVP_MD *md_type;
{
	int i, diglen;
	unsigned char zeropad[20];
	EVP_MD_CTX ctx;
	EVP_DigestInit (&ctx, md_type);
	EVP_DigestUpdate (&ctx, pass, passlen);
	if (saltlen) EVP_DigestUpdate (&ctx, salt, saltlen);
	if (saltlen + passlen < 20 ) {
		memset (zeropad, 0, 20);
		EVP_DigestUpdate (&ctx, zeropad, 20 - saltlen - passlen);
	}
	EVP_DigestFinal (&ctx, md, &diglen);
	for (i = 1; i < iter; i++) {
		EVP_DigestInit (&ctx, md_type);
		EVP_DigestUpdate (&ctx, md, diglen);
		EVP_DigestFinal (&ctx, md, NULL);
	}
}

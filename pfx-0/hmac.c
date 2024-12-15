/* Written by Dr. S N Henson and released into the public domain */
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <crypto.h>
#include <evp.h>
#include <string.h>
#include "hmac.h"

/* HMAC routines: these implement the HMAC function as described in RFC2104 */

void HMAC_Init (hctx, key, keylen, md_type)
HMAC_CTX *hctx;
unsigned char *key;
int keylen;
EVP_MD *md_type;
{
	int blocklen, i;
	unsigned char keytmp[EVP_MAX_MD_BLOCK];
	blocklen = EVP_MD_blocksize (md_type);
	if (keylen > blocklen) {
		hctx->keylen = EVP_MD_size (md_type);
		EVP_DigestInit (&hctx->ctx, hctx->type);
		EVP_DigestUpdate (&hctx->ctx, key, keylen);
		EVP_DigestFinal (&hctx->ctx, hctx->key, NULL);
	} else {
		hctx->keylen = keylen;
		memcpy (hctx->key, key, keylen);
	}
	hctx->type = md_type;
	EVP_DigestInit (&hctx->ctx, hctx->type);
	memset (keytmp, 0, blocklen);
	memcpy (keytmp, hctx->key, hctx->keylen);
	for (i = 0; i < blocklen; i++) keytmp[i] ^=0x36;
	EVP_DigestUpdate (&hctx->ctx, keytmp, blocklen);
}

void HMAC_Update (hctx, data, len)
HMAC_CTX *hctx;
unsigned char *data;
unsigned int len;
{
	EVP_DigestUpdate (&hctx->ctx, data, len);
}

void HMAC_Final (hctx, md, len)
HMAC_CTX *hctx;
unsigned char *md;
unsigned int *len;
{
	int blocklen, i, diglen;
	unsigned char digtmp[EVP_MAX_MD_SIZE], keytmp[EVP_MAX_MD_BLOCK];
	blocklen = EVP_MD_blocksize (md_type);
	EVP_DigestFinal (&hctx->ctx, digtmp, &diglen);
	memset (keytmp, 0, blocklen);
	memcpy (keytmp, hctx->key, hctx->keylen);
	for (i = 0; i < blocklen; i++) keytmp[i] ^=0x5c;
	EVP_DigestInit (&hctx->ctx, hctx->type);
	EVP_DigestUpdate (&hctx->ctx, keytmp, blocklen);
	EVP_DigestUpdate (&hctx->ctx, digtmp, diglen);
	EVP_DigestFinal (&hctx->ctx, md, len);
}


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
#include <pkcs7.h>
#include <err.h>
#include <crypto.h>
#include <sha.h>
#include <stack.h>
#include <evp.h>
#include <rand.h>
#include <string.h>
#include <evp.h>
#include <rc2.h>
#include <rc4.h>
#include "hmac.h"
#include "pfx.h"

/* PBE control routine. Given relevant parameters,
 * initialise encryption/decryption routine properly.
 */

int pfx_crypt_init (pbe_nid, pass, passlen, salt, saltlen, iter, ctx, en_de)
int pbe_nid, passlen, saltlen, iter, en_de;
unsigned char *pass, *salt;
EVP_CIPHER_CTX *ctx;
{
	EVP_MD *md_type;
	EVP_CIPHER *cipher;
	unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];
	int keylen, ivlen;
	md_type = EVP_sha1(); /* They all use SHA-1 at present */
	keylen = 0;
	ivlen = 0;

	if (pbe_nid == NID_pbe_sha1_rc4_128) cipher = EVP_rc4();
	else if (pbe_nid == NID_pbe_sha1_rc4_40) {
		cipher = EVP_rc4();
		keylen = 5;
	} else if (pbe_nid == NID_pbe_sha1_tripdes) cipher = EVP_des_ede3_cbc();
	else if (pbe_nid == NID_pbe_sha1_rc2_128) cipher = EVP_rc2_cbc();
	else if (pbe_nid == NID_pbe_sha1_rc2_40) {
		cipher = EVP_rc2_cbc();
		keylen = 5;
	} else if (pbe_nid == NID_pbe_sha1_des) cipher = EVP_des_cbc();
	else {
		/*PFXerr(PFX_F_PFX_CRYPT_INIT,PFX_R_UNKNOWN_PBE_ALGORITHM);*/
		return 1;
	}
	if (!keylen) keylen = EVP_CIPHER_key_length (cipher);
	if (!ivlen) ivlen = EVP_CIPHER_iv_length (cipher);
	/* OK generate the key and iv */
	pfx_keyiv_gen (pass, passlen, salt, saltlen, iter, iv, ivlen,
						 key, keylen, md_type);
	if (en_de) EVP_EncryptInit (ctx, cipher, key, iv);
	else EVP_DecryptInit (ctx, cipher, key, iv);
	/* And finally the kludge: for RC2/RC4 + 40, reset key */
	if (pbe_nid == NID_pbe_sha1_rc4_40)
		 RC4_set_key (&ctx->c.rc4.ks, keylen, key);
	else if (pbe_nid == NID_pbe_sha1_rc2_40)
		 RC2_set_key (&ctx->c.rc2_cbc.ks, keylen, key, keylen * 8);
	return 0;
}

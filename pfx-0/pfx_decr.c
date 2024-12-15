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
#include <string.h>
#include "hmac.h"
#include "pfx.h"

/* Verify the mac */
int pfx_verify_mac (pass, p)
char *pass;
PFX *p;
{
    unsigned char dig[EVP_MAX_MD_SIZE], *vpass;
    int vplen, diglen;
    ASN1_OCTET_STRING *safe, *macsalt;
    HMAC_CTX hmac;
    EVP_MD *md_type;
    safe = p->authsafe->d.data;
    macsalt = p->mac->salt;
    md_type = EVP_get_digestbyobj (p->mac->dinfo->algor->algorithm);
    diglen = EVP_MD_size (md_type);
    vpass = vpasswd(pass, macsalt->data, macsalt->length, &vplen);

/* No this is not a bug, it simulates the salt prepended twice */
    pfx_keyiv_gen (macsalt->data, macsalt->length, vpass, vplen, 1,
						dig, 16, NULL, 0, md_type);
    free (vpass);
    HMAC_Init (&hmac, dig, 16, md_type);
    HMAC_Update (&hmac, safe->data, safe->length);
    HMAC_Final (&hmac, dig, NULL);

    if ((p->mac->dinfo->digest->length != diglen) ||
	memcmp (dig, p->mac->dinfo->digest->data, diglen)) return 1;
    return 0;
}

/* Decrypt private keys and add them to ESPVK struct */
int pfx_decrypt_pkeys (pfx, pass)
PFX *pfx;
char *pass;
{
int i, j;
STACK *bags;
BAGITEM *tmpbag;
ESPVK *tmpsbag;
PRIV_KEY_INFO *pinf;
PBEPARAM *pbe;
EVP_CIPHER_CTX ctx;
unsigned char *tbuf;
unsigned char *debuf, *p;
int delen, ok, tmplen;
int pbe_nid, saltlen, iter;
unsigned char *salt;

bags = pfx->safe_cont->bagitms;

/* Get all private keys */
for (i = 0; i < sk_num (bags); i++)
	for (j = 0, tmpbag = (BAGITEM *) sk_value (bags, i) ;
				 j < sk_num (tmpbag->espvks); j++) {

	/* Bug: need to check ESPVKOID in here */
    tmpsbag = (ESPVK *) sk_value (tmpbag->espvks, j);

        /* Extract pbe parameters */
    tbuf = tmpsbag->cipher->algor->parameter->value.sequence->data;
    pbe = d2i_PBEPARAM (NULL, &tbuf, 
		tmpsbag->cipher->algor->parameter->value.sequence->length);

    pbe_nid = OBJ_obj2nid (tmpsbag->cipher->algor->algorithm);
    iter = ASN1_INTEGER_get (pbe->iter);
    salt = pbe->salt->data;
    saltlen = pbe->salt->length;

    /* Initialise decryption */
    debuf = malloc (tmpsbag->cipher->epkey->length + 8);
    
    /* Decrypt private key */
    ok = pfx_crypt_init (pbe_nid, pass, strlen(pass), salt, saltlen, iter,
								 &ctx, 0);
    if (ok) return 1;
    EVP_DecryptUpdate (&ctx, debuf, &delen, tmpsbag->cipher->epkey->data,
					 tmpsbag->cipher->epkey->length);
    ok = EVP_DecryptFinal (&ctx, debuf + delen, &tmplen);
    delen += tmplen;
    p = debuf;
    if (!ok) return 1;

    /* Parse structure and add to safebag */
    pinf = d2i_PRIV_KEY_INFO (NULL, &p, delen);
    if (!pinf) return 1;
    free (debuf);
    PBEPARAM_free (pbe);
    /* Add to ESPVK structure */
    tmpsbag->pkey = pinf;

  }
  return 0;
} 

STACK *pfx_decrypt_safe (pfx, pass)
PFX *pfx;
char *pass;
{
	X509_ALGOR *algor;
	ASN1_OCTET_STRING *safe_enc;
	PBEPARAM *pbe;
	STACK *tstk;
	unsigned char *pbuf, *vpass;
	unsigned char *out;
	int vplen, outlen, i, ok;
	EVP_CIPHER_CTX ctx;
	int pbe_nid, saltlen, iter;
	unsigned char *salt;

	/* Extract useful info from authentisafe */
	algor = pfx->safe_cont->safe->d.encrypted->enc_data->algorithm;
	safe_enc = pfx->safe_cont->safe->d.encrypted->enc_data->enc_data;
	pbuf = (char *) algor->parameter->value.sequence->data;
	pbe = d2i_PBEPARAM (NULL, &pbuf,
				 algor->parameter->value.sequence->length);
	if (!pbe) return NULL;

	out = malloc (safe_enc->length + 8);

	pbe_nid = OBJ_obj2nid (algor->algorithm);
	iter = ASN1_INTEGER_get (pbe->iter);
	salt = pbe->salt->data;
	saltlen = pbe->salt->length;

	vpass = vpasswd (pass, pfx->safe_cont->psalt->data,
					 pfx->safe_cont->psalt->length, &vplen);
	/* Decrypt safe */
        ok = pfx_crypt_init (pbe_nid, vpass, vplen, salt, saltlen, iter,
								 &ctx, 0);
	if (ok) return NULL;
	EVP_DecryptUpdate (&ctx, out, &i, safe_enc->data, safe_enc->length);
	outlen = i;
	ok = EVP_DecryptFinal (&ctx, out + i, &i);

	if (!ok) {
		free (out);
		free (vpass);
		return NULL;
	}

	outlen += i;
	pbuf = out;
	/* Parse safe */
	tstk = d2i_ASN1_SET (&pfx->bags, &pbuf, outlen,
		 (char *(*)())d2i_SAFEBAG, V_ASN1_SET, V_ASN1_UNIVERSAL);

	free (out);
	free (vpass);
	return (tstk);

}

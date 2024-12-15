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
#include "hmac.h"
#include "pfx.h"

/* Set the mac */
int pfx_set_mac (pass, p, md_type)
char *pass;
PFX *p;
EVP_MD *md_type;
{
    unsigned char dig[EVP_MAX_MD_SIZE], *vpass;
    int vplen, saltlen, diglen;
    unsigned char *salt;
    ASN1_OCTET_STRING *safe;
    HMAC_CTX hmac;
    diglen = EVP_MD_size (md_type); 

    safe = p->authsafe->d.data;

    RAND_seed (safe->data, safe->length);

    saltlen = 16;
    salt = Malloc (saltlen);
    RAND_bytes (salt, saltlen);

    /* Set the algorithm identifier */
    p->mac->dinfo->algor->algorithm = OBJ_nid2obj (EVP_MD_type(md_type));
    p->mac->dinfo->algor->parameter = ASN1_TYPE_new ();
    p->mac->dinfo->algor->parameter->type = V_ASN1_NULL;

    if (!p->mac->salt) p->mac->salt = ASN1_BIT_STRING_new ();	
    p->mac->salt->length = saltlen;
    p->mac->salt->data = salt;

    vpass = vpasswd(pass, salt, saltlen, &vplen);

/* No this is not a bug, it simulates the salt prepended twice */
    pfx_keyiv_gen (salt, saltlen, vpass, vplen, 1, dig, 16, NULL, 0, md_type);
    free (vpass);

    HMAC_Init (&hmac, dig, 16, md_type);
    HMAC_Update (&hmac, safe->data, safe->length);
    p->mac->dinfo->digest->data = Malloc (diglen);
    p->mac->dinfo->digest->length = diglen;
    HMAC_Final (&hmac, p->mac->dinfo->digest->data, NULL);

    return 1;

}

/* Encrypt private keys and add them to ESPVK struct */
int pfx_encrypt_pkeys (pfx, pass, pbe_nid)
PFX *pfx;
char *pass;
int pbe_nid;
{
	int i, j, k;
	BAGITEM *tmpbag;
	ESPVK *tmpsbag;
	EVP_CIPHER_CTX ctx;
	STACK *bags;
	unsigned char *pkeybuf, *cryptbuf, *p;
	int cryptlen, tmplen;
	int iter, saltlen;
	unsigned char *salt;
	

	bags = pfx->safe_cont->bagitms;

	iter = 1;
	saltlen = 8;

	/* Get all private keys */
	for (i = 0; i < sk_num (bags); i++)
		for (j = 0, tmpbag = (BAGITEM *) sk_value (bags, i) ;
					 j < sk_num (tmpbag->espvks); j++) {

	    tmpsbag = (ESPVK *) sk_value (tmpbag->espvks, j);


	    /* Generate private key DER encoding */
	    k = i2d_PRIV_KEY_INFO (tmpsbag->pkey, NULL);
	    pkeybuf = Malloc (k);
	    p = pkeybuf;
	    i2d_PRIV_KEY_INFO (tmpsbag->pkey, &p);
	    cryptbuf = Malloc (k + 8);

	    /* Hmm, private key DER encoded, looks like a good seed ... */ 
	    RAND_seed (pkeybuf, k);

	    salt = Malloc (saltlen);
	    RAND_bytes (salt, saltlen);

	    tmpsbag->cipher->algor = pbe_set (pbe_nid, iter, salt, saltlen);

	    pfx_crypt_init (pbe_nid, pass, strlen(pass), salt, saltlen, iter,
								 &ctx, 1);
	    Free (salt);

	    /* Encrypt private key */
	    EVP_EncryptUpdate (&ctx, cryptbuf, &cryptlen, pkeybuf, k);
	    EVP_EncryptFinal (&ctx, cryptbuf + cryptlen, &tmplen);
	    cryptlen += tmplen;
	    Free (pkeybuf);

	    /* Add encrypted key to Safebag */

	    tmpsbag->cipher->epkey->data = cryptbuf;
	    tmpsbag->cipher->epkey->length = cryptlen;

	  }
	  return 0;
} 

int pfx_encrypt_safe (pfx, pass, pbe_nid)
PFX *pfx;
char *pass;
int pbe_nid;
{
	ASN1_OCTET_STRING *safe_enc;
	unsigned char *vpass;
	int vplen, i;
	EVP_CIPHER_CTX ctx;

	int iter, saltlen, psaltlen, safelen, crslen;
	unsigned char *salt, *psalt, *safe, *crsafe, *p;
	AUTHSAFE *asafe;

	asafe = pfx->safe_cont;

	iter = 1;
	saltlen = 8;
	salt = Malloc (saltlen);
	RAND_bytes (salt, saltlen);

	psaltlen = 16;
	psalt = Malloc (psaltlen);
	RAND_bytes (psalt, psaltlen);

	/* Set encryption algorithm */
	asafe->safe->d.encrypted->enc_data->algorithm =
	    		pbe_set (pbe_nid, iter, salt, saltlen);
	/* Set privacy salt */
	asafe->psalt = ASN1_BIT_STRING_new();
	asafe->psalt->data = psalt;
	asafe->psalt->length = psaltlen;

	/* Generate DER encoding of safe */
	
	safelen = i2d_ASN1_SET (pfx->bags, NULL, i2d_SAFEBAG, V_ASN1_SET,
							 V_ASN1_UNIVERSAL);
	safe = Malloc (safelen);	
	p = safe;
	i2d_ASN1_SET (pfx->bags, &p, i2d_SAFEBAG, V_ASN1_SET, V_ASN1_UNIVERSAL);

	crsafe = Malloc (safelen + 8);
	vpass = vpasswd (pass, psalt, psaltlen, &vplen);
	pfx_crypt_init (pbe_nid, vpass, vplen, salt, saltlen, iter, &ctx, 1);
	EVP_EncryptUpdate (&ctx, crsafe, &i, safe, safelen);
	crslen = i;
	EVP_EncryptFinal (&ctx, crsafe + i, &i);
	crslen += i;
	Free (safe);

	/* Place encrypted content in safe */
	safe_enc = ASN1_OCTET_STRING_new ();
	asafe->safe->d.encrypted->enc_data->enc_data = safe_enc;
	safe_enc->length = crslen;
	safe_enc->data = crsafe;

	RAND_seed (crsafe, crslen);
	Free (salt);

	return 0;
}

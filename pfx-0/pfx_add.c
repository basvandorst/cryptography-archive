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
#include <string.h>
#include <rsa.h>
#include <sha.h>
#include <pem.h>
#include <rand.h>
#include "pfx.h"

/* Add a certificate chain to SafeContents */

int pfx_add_cert_chain (pfx, chain)
PFX *pfx;
STACK *chain;
{

SAFEBAG *sbg = NULL, *sbgtmp;
CERTCRL *crt;
PKCS7 *p7;
X509 *cert;
int i;

/* Add a SafeContents if none there already */
if (!pfx->bags) pfx->bags = sk_new (NULL);

/* Search for CertCRLBag */
for (i = 0; i < sk_num (pfx->bags); i++) {
	sbgtmp = (SAFEBAG *) sk_value (pfx->bags, i);
	if (OBJ_obj2nid (sbgtmp->type) == NID_CertAndCRLBagID) {
		sbg = sbgtmp;
		break;
	}
}

if (!sbg) { /* None found, create one */
	sbg = SAFEBAG_new ();
	sbg->type = OBJ_nid2obj (NID_CertAndCRLBagID);
	sk_push (pfx->bags, (char *)sbg);
}

/* OK at this point sbg points to a CertAndCRLBag */
if (!sbg->certs) sbg->certs = sk_new (NULL);
crt = CERTCRL_new ();
sk_push (sbg->certs, (char *) crt);

crt->bagid = OBJ_nid2obj (NID_x509CertCRLBag);
crt->lcerts = CERTS_new ();

p7 = PKCS7_new();
crt->lcerts->cchain = p7;

PKCS7_set_type (p7, NID_pkcs7_signed);

/* Add certificate chain */
for (i = 0; i < sk_num (chain); i++) {
	cert = (X509 *) sk_value (chain, i);
	PKCS7_add_certificate (p7,cert);
}

/* Set dummy contents to data */
p7 = PKCS7_new ();
PKCS7_set_type (p7, NID_pkcs7_data);
PKCS7_set_content (crt->lcerts->cchain, p7);

/* Set detached because we have no data */
PKCS7_set_detached (p7, 1);
if (p7->d.data) {
	ASN1_OCTET_STRING_free (p7->d.data);
	p7->d.data = NULL;
}

return 0;
}

/* Add a private key to Baggage.
 * Note: we need the associated certificate for the thumbprint
 * and a name for its nickname. Also pass message digest type.
 * NB: specs recommend SHA-1.
 */

int pfx_add_private (pfx, rsa, cert, name, md_type)
PFX *pfx;
RSA *rsa;
X509 *cert;
char *name;
int md_type;
{

BAGITEM *bitm;
AUTHSAFE *safe;
ESPVK *skey;
PRIV_KEY_INFO *pkey;
X509_SIG *print;
EVP_MD *mdtype;
unsigned char cdig[EVP_MAX_MD_SIZE];
unsigned char *p, *q;
int i, nlen, diglen;

safe = pfx->safe_cont;

/* Add baggage if none there already */
if (!safe->bagitms) safe->bagitms = sk_new (NULL);

/* Always add a new bagitem */
bitm = BAGITEM_new ();
sk_push (safe->bagitms, (char *)bitm);
bitm->espvks = sk_new (NULL);
bitm->safebags = sk_new (NULL); /* Empty but we may need it */

/* Add new private key struct */
skey = ESPVK_new ();
sk_push (bitm->espvks, (char *)skey);
skey->espvkoid = OBJ_nid2obj (NID_pkcs8_key_shrouding);

/* Add supporting data */
skey->supdata = SUP_DATA_new ();
skey->supdata->prints = sk_new (NULL);
print = X509_SIG_new ();
sk_push (skey->supdata->prints, (char *)print);
print->algor->algorithm = OBJ_nid2obj (md_type);
print->algor->parameter = ASN1_TYPE_new ();
print->algor->parameter->type = V_ASN1_NULL;

/* Generate digest of the certificate */
mdtype = EVP_get_digestbynid(md_type);
X509_digest (cert, mdtype, cdig, &diglen);
/* Add digest to thumbprint */
p = Malloc (diglen);
memcpy (p, cdig, diglen);
print->digest->length = diglen;
print->digest->data = p;

/* Set the nickname */
nlen = (strlen(name)<<1) + 2;
p = Malloc (nlen);
memset (p, 0, nlen);

/* Quick and nasty unicode converter */
for (i = 0; i < strlen(name); i++) p[(i<<1) + 1] = name[i];

skey->supdata->nickname->length = nlen;
skey->supdata->nickname->data = p;

/* Now insert private key */
pkey = PRIV_KEY_INFO_new ();
ASN1_INTEGER_set (pkey->version, 0);
pkey->pkeyalg->algorithm = OBJ_nid2obj (NID_rsaEncryption);
pkey->pkeyalg->parameter = ASN1_TYPE_new ();
pkey->pkeyalg->parameter->type = V_ASN1_NULL;

i = i2d_RSAPrivateKey (rsa, NULL);
p = Malloc (i);
q = p;
i2d_RSAPrivateKey (rsa, &q);
pkey->pkey->length = i;
pkey->pkey->data = p;

skey->pkey = pkey;

RAND_seed (p, i);

return 0;
}

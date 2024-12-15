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
#include <crypto.h>
#include <stack.h>
#include <evp.h>
#include "pfx.h"

/* Get a stack of all certificates */

STACK *pfx_get_certs (p)
PFX *p;
{
	int i, j, k;
	SAFEBAG *sbag;
	CERTCRL *ccrl;
	STACK *certs, *p7certs;
	if (!p->bags) return NULL;
	certs = sk_new (NULL);
	for (i = 0; i < sk_num(p->bags); i++) {
  	    sbag = (SAFEBAG *) sk_value (p->bags, i); 
	    if (sbag->certs) for (j = 0; j < sk_num (sbag->certs); j++) {
		ccrl = (CERTCRL *) sk_value (sbag->certs, j);
		if (ccrl->lcerts && ccrl->lcerts->cchain) {
		    p7certs = ccrl->lcerts->cchain->d.sign->cert;
		    for (k = 0; k < sk_num(p7certs); k++) 
				sk_push (certs, sk_value(p7certs, k));
		}
 	    }
        }
	return certs;
}

/* Return certificate matching thumprint */

X509 *pfx_get_cert_by_thumbprint (thprints, certs)
STACK *thprints;
STACK *certs;
{
	X509_SIG *thprint;
	X509 *cert;
	EVP_MD *digtype;
	int md_type, i, diglen;
	unsigned char tmpdig[EVP_MAX_MD_SIZE];
	if (!thprints || !sk_num(thprints)) return NULL;
	thprint = (X509_SIG *)sk_value(thprints, 0);
	if (!thprint) return NULL;
	md_type = OBJ_obj2nid(thprint->algor->algorithm);
	digtype = EVP_get_digestbynid(md_type);
	for (i = 0; i < sk_num(certs); i++) {
	    cert = (X509 *) sk_value(certs, i);
	    X509_digest(cert, digtype, tmpdig, &diglen);
	    if ((diglen == thprint->digest->length) &&
		!memcmp(thprint->digest->data, tmpdig, diglen)) return cert;
	}
	return NULL;
}

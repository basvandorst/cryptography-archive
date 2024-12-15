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

/* Given a single certificate return a verified chain or NULL if error */

/* Hope this is OK .... */

int pfx_get_chain (cert, chain)
X509 *cert;
STACK **chain;
{
	X509_STORE *store;
	X509_STORE_CTX store_ctx;
	store = X509_STORE_new ();
	X509_STORE_set_default_paths (store);
	X509_STORE_CTX_init(&store_ctx, store, cert, NULL);
	if (X509_verify_cert(&store_ctx) <= 0) 
			return X509_STORE_CTX_get_error (&store_ctx);
	*chain =  X509_STORE_CTX_get_chain (&store_ctx);
	return 0;
}	
	


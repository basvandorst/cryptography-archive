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
#include "pfx.h"

/* Initialise a PFX/AUTHSAFE structure to take data */

void pfx_init (p)
PFX **p;
{
	PFX *pfx;
	AUTHSAFE *safe;
	ASN1_OBJECT *obj;
	/* Initial Setup */
	if (!*p) *p = PFX_new ();
	pfx = *p;
	if (!pfx->mac) pfx->mac = MAC_DATA_new ();
	if (!pfx->safe_cont) pfx->safe_cont = PFX_AUTHSAFE_new ();
	safe = pfx->safe_cont;
	/* Set up AuthenticatedSafe */
	ASN1_INTEGER_set (safe->version, 1); /* v1 */
	obj = OBJ_nid2obj (NID_off_line);    /* off line mode */
	safe->mode = obj;
	/* Set up safe */
	safe->safe = PKCS7_new ();

	/* The next bit may end up in PKCS7_set_type eventually */
	obj = OBJ_nid2obj (NID_pkcs7_encrypted);
	safe->safe->type = obj;
	safe->safe->d.encrypted = PKCS7_ENCRYPT_new ();
	ASN1_INTEGER_set (safe->safe->d.encrypted->version, 0);
	obj = OBJ_nid2obj (NID_pkcs7_data);
	safe->safe->d.encrypted->enc_data->content_type = obj;

	/* Set up authsafe */
	pfx->authsafe = PKCS7_new ();
	PKCS7_set_type (pfx->authsafe, NID_pkcs7_data);

}

/* Return an algorithm identifier for a PBE algorithm */

X509_ALGOR *pbe_set(alg, iter, salt, saltlen)
int alg;
int iter;
unsigned char *salt;
int saltlen;
{
	unsigned char *pdata, *ptmp;
	int plen;
	PBEPARAM *pbe;
	ASN1_OBJECT *al;
	X509_ALGOR *algor;
	ASN1_TYPE *astype;

	pbe = PBEPARAM_new ();
	ASN1_INTEGER_set (pbe->iter, iter);
	pbe->salt->data = Malloc (saltlen);
	pbe->salt->length = saltlen;
	memcpy (pbe->salt->data, salt, saltlen);
	plen = i2d_PBEPARAM (pbe, NULL);
	pdata = Malloc (plen);
	ptmp = pdata;
	i2d_PBEPARAM (pbe, &ptmp);
	PBEPARAM_free (pbe);

	astype = ASN1_TYPE_new();

	astype->type = V_ASN1_SEQUENCE;
	astype->value.sequence=ASN1_STRING_new();
	ASN1_STRING_set (astype->value.sequence, pdata, plen);
	Free (pdata);
	
	al = OBJ_nid2obj (alg);
	algor = X509_ALGOR_new();
	algor->algorithm = al;
	algor->parameter = astype;

	return (algor);
}	

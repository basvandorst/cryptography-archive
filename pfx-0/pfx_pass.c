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

unsigned char *vpasswd (pass, salt, saltlen, oplen)
unsigned char *pass;
unsigned char *salt;
int saltlen;
int *oplen;
{
	char *uni, *unipass;
	int plen, i;
	plen = strlen (pass);
	uni = malloc ((plen<<1) + saltlen + 2);
	if (!uni) return NULL;
	memset (uni, 0, (plen<<1) + saltlen + 2);
	unipass = uni + saltlen + 1;
	memcpy (uni, salt, saltlen);
	for (i = 0; i < plen; i++) unipass[i<<1] = pass[i];
	if (oplen) *oplen = (plen<<1) + saltlen + 2;
	return uni;
}

void PKCS5_KEY_GEN (pass, passlen, salt, saltlen, iter, md)
unsigned char *pass;
int passlen;
unsigned char *salt;
int saltlen;
int iter;
unsigned char *md;
{
	pfx_pkcs5 (pass, passlen, salt, saltlen, iter, md, EVP_sha1());
}

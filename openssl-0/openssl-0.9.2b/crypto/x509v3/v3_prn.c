/* v3_prn.c */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 1999.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* X509 v3 extension utilities */

#include <stdio.h>
#include "cryptlib.h"
#include "conf.h"
#include "x509v3.h"

/* Extension printing routines */

/* Print out a name+value stack */

void X509V3_EXT_val_prn(out, val)
BIO *out;
STACK *val;
{
	int i;
	CONF_VALUE *nval;
	if(!val) return;
	for(i = 0; i < sk_num(val); i++) {
		if(i > 0) BIO_printf(out, ", ");
		nval = (CONF_VALUE *)sk_value(val, i);
		if(!nval->name) BIO_printf(out, "%s", nval->value);
		else if(!nval->value) BIO_printf(out, "%s", nval->name);
		else BIO_printf(out, "%s:%s", nval->name, nval->value);
	}
}

/* Main routine: print out a general extension */

int X509V3_EXT_print(out, ext, flag)
BIO *out;
X509_EXTENSION *ext;
int flag;
{
	char *ext_str = NULL, *p, *value = NULL;
	X509V3_EXT_METHOD *method;	
	STACK *nval = NULL;
	int ok = 1;
	if(!(method = X509V3_EXT_get(ext))) return 0;
	p = ext->value->data;
	if(!(ext_str = method->d2i(NULL, &p, ext->value->length))) return 0;
	if(method->i2s) {
		if(!(value = method->i2s(method, ext_str))) {
			ok = 0;
			goto err;
		}
		BIO_printf(out, value);
	} else if(method->i2v) {
		if(!(nval = method->i2v(method, ext_str, NULL))) {
			ok = 0;
			goto err;
		}
		X509V3_EXT_val_prn(out, nval);
	} else if(method->i2r) {
		if(!method->i2r(method, ext_str, out)) ok = 0;
	} else ok = 0;

	err:
		sk_pop_free(nval, X509V3_conf_free);
		if(value) Free(value);
		method->ext_free(ext_str);
		return ok;
}

int X509V3_EXT_print_fp(fp, ext, flag)
FILE *fp;
X509_EXTENSION *ext;
int flag;
{
	BIO *bio_tmp;
	int ret;
	if(!(bio_tmp = BIO_new_fp(fp, BIO_NOCLOSE))) return 0;
	ret = X509V3_EXT_print(bio_tmp, ext, flag);
	BIO_free(bio_tmp);
	return ret;
}

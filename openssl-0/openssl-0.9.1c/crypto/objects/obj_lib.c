/* crypto/objects/obj_lib.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include "cryptlib.h"
#include "lhash.h"
#include "objects.h"
#include "buffer.h"

ASN1_OBJECT *OBJ_dup(o)
ASN1_OBJECT *o;
	{
	ASN1_OBJECT *r;
	int i;

	if (o == NULL) return(NULL);
	if (!(o->flags & ASN1_OBJECT_FLAG_DYNAMIC))
		return(o);

	r=(ASN1_OBJECT *)ASN1_OBJECT_new();
	if (r == NULL)
		{
		OBJerr(OBJ_F_OBJ_DUP,ERR_R_ASN1_LIB);
		return(NULL);
		}
	r->data=(unsigned char *)Malloc(o->length);
	if (r->data == NULL)
		goto err;
	memcpy(r->data,o->data,o->length);
	r->length=o->length;
	r->nid=o->nid;
	r->ln=r->sn=NULL;
	if (o->ln != NULL)
		{
		i=strlen(o->ln)+1;
		r->ln=(char *)Malloc(i);
		if (r->ln == NULL) goto err;
		memcpy(r->ln,o->ln,i);
		}

	if (o->sn != NULL)
		{
		i=strlen(o->sn)+1;
		r->sn=(char *)Malloc(i);
		if (r->sn == NULL) goto err;
		memcpy(r->sn,o->sn,i);
		}
	r->flags=o->flags|(ASN1_OBJECT_FLAG_DYNAMIC|
		ASN1_OBJECT_FLAG_DYNAMIC_STRINGS|ASN1_OBJECT_FLAG_DYNAMIC_DATA);
	return(r);
err:
	OBJerr(OBJ_F_OBJ_DUP,ERR_R_MALLOC_FAILURE);
	if (r != NULL)
		{
		if (r->ln != NULL) Free(r->ln);
		if (r->data != NULL) Free(r->data);
		Free(r);
		}
	return(NULL);
	}

int OBJ_cmp(a,b)
ASN1_OBJECT *a;
ASN1_OBJECT *b;
	{
	int ret;

	ret=(a->length-b->length);
	if (ret) return(ret);
	return(memcmp(a->data,b->data,a->length));
	}
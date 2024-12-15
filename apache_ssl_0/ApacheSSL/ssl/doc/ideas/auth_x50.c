/* doc/ideas/auth_x50.c */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
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
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
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

int x509_get_by_alias(ctx,arg,ret)
AUTH_CTX *ctx;
METHOD_ARG *arg;
METHOD_RET *ret;
	{
	/* Lookup in cache?  Generally do it however :-) */
	}

/* Lookup by our name, could be more than one row returned */
int x509_get_by_subject(ctx,arg,ret)
AUTH_CTX *ctx;
METHOD_ARG *arg;
METHOD_RET *ret;
	{
	/* Look in cache, if present, return it.
	 * else for each 'active' method in turn, call the 'get by subject'
	 * methods.
	 */
	}

/* Lookup by our issuer and serial, one row returned. */
int x509_get_by_issuer_and_serial(ctx,x,ret)
AUTH *ctx;
METHOD_RET *ret;
METHOD_ARG *arg;
	{
	/* Look in cache, if present, return it.
	 * else for each 'active' method in turn, call the 'get by
	 * subject'.
	 */
	}
	
int X509_verify_certificate(ctx,x,list,cb)
AUTH_CTX *ctx;
X509 *x;
LIST list;	/* X509_OBJ chain - they are cached */
int (*cb)();
	{
	x509_get_by_subject(ctx,arg,ret)
	}

int X509_verify_signature(ctx,x,data,list,cb);
AUTH_CTX *ctx;
	{
	}

int X509_get_cert(ctx,name,ret)

int X509_get_cert_by_alias(ctx,alias,ret)

AUTH_CTX	*AUTH_new();

METHOD_CTX *	AUTH_push_method(ctx,type,auth_meth)
AUTH_CTX *ctx;
int type;	/* type of method */
AUTH_METHOD *ath_meth;

int		AUTH_push_arg(auth_meth,arg,len);
AUTH_METHOD *auth_meth;
char *arg;
int len;
	{
	}
	
int 		AUTH_init(ctx)
AUTH_CTX *ctx;
	{
	}

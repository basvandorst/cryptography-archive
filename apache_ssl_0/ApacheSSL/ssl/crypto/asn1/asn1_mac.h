/* lib/asn1/asn1_mac.h */
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

#include "asn1.h"
#include "x509.h"
#include "./pkcs7.h"

#define M_ASN1_D2I_vars(a,type,func) \
	ASN1_CTX c; \
	type ret=NULL; \
	\
	c.pp=pp; \
	c.error=ASN1_R_ERROR_STACK; \
	if ((a == NULL) || ((*a) == NULL)) \
		{ if ((ret=(type)func()) == NULL) goto err; } \
	else	ret=(*a);

#define M_ASN1_D2I_Init() \
	c.p= *pp; \
	c.max=(length == 0)?0:(c.p+length);

#define M_ASN1_D2I_Finish_2(a) \
	if (!asn1_Finish(&c)) goto err; \
	*pp=c.p; \
	if (a != NULL) (*a)=ret; \
	return(ret);

#define M_ASN1_D2I_Finish(a,func,e) \
	if (!asn1_Finish(&c)) goto err; \
	*pp=c.p; \
	if (a != NULL) (*a)=ret; \
	return(ret); \
err:\
	ASN1err((e),c.error); \
	if (ret != NULL) func(ret); \
	if ((a != NULL) && (*a == ret)) *a=NULL; \
	return(NULL)


#define M_ASN1_D2I_start_sequence() \
	if (!asn1_GetSequence(&c,&length)) goto err;

#define M_ASN1_D2I_end_sequence() \
	(((c.inf&1) == 0)?(c.slen <= 0): \
		(c.eos=ASN1_check_infinite_end(&c.p,c.slen)))

#define M_ASN1_D2I_get(b,func) \
	c.q=c.p; \
	if (func(&(b),&c.p,c.slen) == NULL) goto err; \
	c.slen-=(c.p-c.q);

#define M_ASN1_D2I_get_opt(b,func,tag,type) \
	if (M_ASN1_next == (V_ASN1_CONTEXT_SPECIFIC|(tag))) \
		{ \
		M_ASN1_next=(V_ASN1_UNIVERSAL|type); \
		M_ASN1_D2I_get(b,func); \
		M_ASN1_next_prev=(V_ASN1_CONTEXT_SPECIFIC|tag); \
		}

#define M_ASN1_D2I_get_set(r,func) \
		M_ASN1_D2I_get_set_imp(r,func,V_ASN1_SET,V_ASN1_UNIVERSAL);

#define M_ASN1_D2I_get_set_opt(b,func,tag) \
	if (M_ASN1_next == (V_ASN1_CONTEXT_SPECIFIC|V_ASN1_CONSTRUCTED|(tag)))\
		{ \
		M_ASN1_D2I_get_set_imp(b,func,tag,V_ASN1_CONTEXT_SPECIFIC); \
		}

#define M_ASN1_D2I_get_seq(r,func) \
		M_ASN1_D2I_get_set_imp(r,func,V_ASN1_SEQUENCE,V_ASN1_UNIVERSAL);

#define M_ASN1_D2I_get_IMP_set(r,func,x) \
		M_ASN1_D2I_get_set_imp(r,func,x,V_ASN1_CONTEXT_SPECIFIC);

#define M_ASN1_D2I_get_set_imp(r,func,a,b) \
	c.q=c.p; \
	if (d2i_ASN1_SET(&(r),&c.p,c.slen,(char *(*)())func,a,b) == NULL) \
		goto err; \
	c.slen-=(c.p-c.q);

#define M_ASN1_D2I_get_set_strings(r,func,a,b) \
	c.q=c.p; \
	if (d2i_ASN1_STRING_SET(&(r),&c.p,c.slen,a,b) == NULL) \
		goto err; \
	c.slen-=(c.p-c.q);

#define M_ASN1_D2I_get_set_exp_opt(r,func,tag,b) \
	if (M_ASN1_next == (V_ASN1_CONSTRUCTED|V_ASN1_CONTEXT_SPECIFIC|tag)) \
		{ \
		int Tinf,Ttag,Tclass; \
		long Tlen; \
		\
		c.q=c.p; \
		Tinf=ASN1_get_object(&c.p,&Tlen,&Ttag,&Tclass,c.slen); \
		if (Tinf == 0xff) \
			{ c.error=ASN1_R_BAD_OBJECT_HEADER; goto err; } \
		if (d2i_ASN1_SET(&(r),&c.p,Tlen,(char *(*)())func, \
			b,V_ASN1_UNIVERSAL) == NULL) \
			goto err; \
		c.slen-=(c.p-c.q); \
		}

/* New macros */
#define M_ASN1_New_Malloc(ret,type) \
	if ((ret=(type *)malloc(sizeof(type))) == NULL) goto err2;

#define M_ASN1_New(arg,func) \
	if (((arg)=func()) == NULL) return(NULL)

#define M_ASN1_New_Error(a) \
/*	err:	ASN1err((a),ASN1_R_ERROR_STACK); \
		return(NULL);*/ \
	err2:	ASN1err((a),ERR_R_MALLOC_FAILURE); \
		return(NULL)


#define M_ASN1_next		(*c.p)
#define M_ASN1_next_prev	(*c.q)

/*************************************************/

#define M_ASN1_I2D_vars(a)	int r,ret=0; \
				unsigned char *p; \
				if (a == NULL) return(0)

/* Length Macros */
#define M_ASN1_I2D_len(a,f)	ret+=f(a,NULL)
#define M_ASN1_I2D_len_opt(a,f)	if (a != NULL) M_ASN1_I2D_len(a,f)

#define M_ASN1_I2D_len_SET(a,f) \
		ret+=i2d_ASN1_SET(a,NULL,f,V_ASN1_SET,V_ASN1_UNIVERSAL);

#define M_ASN1_I2D_len_SEQ(a,f) \
		ret+=i2d_ASN1_SET(a,NULL,f,V_ASN1_SEQUENCE,V_ASN1_UNIVERSAL);

#define M_ASN1_I2D_len_IMP_set(a,f,x) \
		ret+=i2d_ASN1_SET(a,NULL,f,x,V_ASN1_CONTEXT_SPECIFIC);

#define M_ASN1_I2D_len_set_opt(a,f,x) \
		if ((a != NULL) && (sk_num(a) != 0)) \
			ret+=i2d_ASN1_SET(a,NULL,f,x,V_ASN1_CONTEXT_SPECIFIC);

#define M_ASN1_I2D_len_EXP_set_opt(a,f,mtag,tag,v) \
		if ((a != NULL) && (sk_num(a) != 0))\
			{ \
			v=i2d_ASN1_SET(a,NULL,f,tag,V_ASN1_UNIVERSAL); \
			ret+=ASN1_object_size(1,v,mtag); \
			}

/* Put Macros */
#define M_ASN1_I2D_put(a,f)	f(a,&p)

#define M_ASN1_I2D_put_opt(a,f,t)	\
		if (a != NULL) \
			{ \
			unsigned char *q=p; \
			f(a,&p); \
			*q=(V_ASN1_CONTEXT_SPECIFIC|t); \
			}

#define M_ASN1_I2D_put_SET(a,f) i2d_ASN1_SET(a,&p,f,V_ASN1_SET,\
			V_ASN1_UNIVERSAL)
#define M_ASN1_I2D_put_IMP_set(a,f,x) i2d_ASN1_SET(a,&p,f,x,\
			V_ASN1_CONTEXT_SPECIFIC)

#define M_ASN1_I2D_put_SEQ(a,f) i2d_ASN1_SET(a,&p,f,V_ASN1_SEQUENCE,\
			V_ASN1_UNIVERSAL)

#define M_ASN1_I2D_put_set_opt(a,f,x) \
		if ((a != NULL) && (sk_num(a) != 0)) \
			{ i2d_ASN1_SET(a,&p,f,x,V_ASN1_CONTEXT_SPECIFIC); }

#define M_ASN1_I2D_put_EXP_set_opt(a,f,mtag,tag,v) \
		if ((a != NULL) && (sk_num(a) != 0)) \
			{ \
			ASN1_put_object(&p,1,v,mtag,V_ASN1_CONTEXT_SPECIFIC); \
			i2d_ASN1_SET(a,&p,f,tag,V_ASN1_UNIVERSAL); \
			}

#define M_ASN1_I2D_seq_total() \
		r=ASN1_object_size(1,ret,V_ASN1_SEQUENCE); \
		if (pp == NULL) return(r); \
		p= *pp; \
		ASN1_put_object(&p,1,ret,V_ASN1_SEQUENCE,V_ASN1_UNIVERSAL)

#define M_ASN1_I2D_INF_seq_start(tag,ctx) \
		*(p++)=(V_ASN1_CONSTRUCTED|(tag)|(ctx)); \
		*(p++)=0x80

#define M_ASN1_I2D_INF_seq_end() *(p++)=0x00; *(p++)=0x00

#define M_ASN1_I2D_finish()	*pp=p; \
				return(r);

#ifndef NOPROTO

int asn1_GetSequence(ASN1_CTX *c, long *length);

#else 

int asn1_GetSequence();

#endif


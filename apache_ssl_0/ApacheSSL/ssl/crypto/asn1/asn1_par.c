/* lib/asn1/asn1_par.c */
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

#include <stdio.h>
#include "crypto.h"
#include "objects.h"
#include "x509.h"

#ifndef NOPROTO
static void asn1_print_info(FILE *file, int tag, int class,int constructed);
static int asn1_parse2(FILE *fp, unsigned char **pp, long length,
	int offset, int depth);
#else
static void asn1_print_info();
static int asn1_parse2();
#endif

static void asn1_print_info(file, tag, class, constructed)
FILE *file;
int tag;
int class;
int constructed;
	{
	static char *fmt="%-18s";

	if (constructed & V_ASN1_CONSTRUCTED)
		Fprintf(file,"cons: ");
	else
		Fprintf(file,"prim: ");
	if ((class & V_ASN1_PRIVATE) == V_ASN1_PRIVATE)
		{
		Fprintf(file,"priv [ %d ] ",tag);
		return;
		}
	else if ((class & V_ASN1_CONTEXT_SPECIFIC) == V_ASN1_CONTEXT_SPECIFIC)
		{
		Fprintf(file,"cont [ %d ]",tag);
		return;
		}
	else if ((class & V_ASN1_APPLICATION) == V_ASN1_APPLICATION)
		{
		Fprintf(file,"appl [ %d ]",tag);
		return;
		}
	else if ((tag == V_ASN1_EOC) /* && (class == V_ASN1_UNIVERSAL) */)
		Fprintf(file,fmt,"EOC");
	else if (tag == V_ASN1_INTEGER)
		Fprintf(file,fmt,"INTEGER");
	else if (tag == V_ASN1_BIT_STRING)
		Fprintf(file,fmt,"BIT STRING");
	else if (tag == V_ASN1_OCTET_STRING)
		Fprintf(file,fmt,"OCTET STRING");
	else if (tag == V_ASN1_NULL)
		Fprintf(file,fmt,"NULL");
	else if (tag == V_ASN1_OBJECT)
		Fprintf(file,fmt,"OBJECT");
	else if (tag == V_ASN1_SEQUENCE)
		Fprintf(file,fmt,"SEQUENCE");
	else if (tag == V_ASN1_SET)
		Fprintf(file,fmt,"SET");
	else if (tag == V_ASN1_PRINTABLESTRING)
		Fprintf(file,fmt,"PRINTABLESTRING");
	else if (tag == V_ASN1_T61STRING)
		Fprintf(file,fmt,"T61STRING");
	else if (tag == V_ASN1_IA5STRING)
		Fprintf(file,fmt,"IA5STRING");
	else if (tag == V_ASN1_UTCTIME)
		Fprintf(file,fmt,"UTCTIME");
	else	Fprintf(file,fmt,"unknown");
	}

void ASN1_parse(fp, pp, len)
FILE *fp;
unsigned char *pp;
long len;
	{
	asn1_parse2(fp,&pp,len,0,0);
	}

static int asn1_parse2(fp, pp, length, offset, depth)
FILE *fp;
unsigned char **pp;
long length;
int offset;
int depth;
	{
	unsigned char *p,*ep,*tot,*op,*opp;
	long len;
	int tag,class,ret=1;
	int nl,i,hl,j;
	ASN1_OBJECT *o=NULL;

	p= *pp;
	tot=p+length;
	op=p-1;
	while ((p < tot) && (op < p))
		{
		op=p;
		j=ASN1_get_object(&p,&len,&tag,&class,length);
#ifdef LINT
		j=j;
#endif
		hl=(p-op);
		/* if j == 0x21 it is a constructed indefinite length object */
		Fprintf(fp,"%5ld:",(long)offset+(long)(op- *pp));
		if (j != 0x21)
			Fprintf(fp,"d=%d hl=%ld l=%3ld ",
				depth,(long)hl,len);
		else
			Fprintf(fp,"d=%d hl=%ld l=inf ",depth,(long)hl);
		asn1_print_info(fp,tag,class,j);
		if (j & V_ASN1_CONSTRUCTED)
			{
			ep=p+len;
			Fputc('\n',fp);
			if (len > length)
				{
				ret=0;
				Fprintf(fp,"length is greater than %ld\n",length);
				goto end;
				}
			if ((j == 0x21) && (len == 0))
				{
				for (;;)
					{
					if (!asn1_parse2(fp,&p,
						(long)(tot-p),
						offset+(p - *pp),depth+1))
						{
						ret=0;
						goto end;
						}
					if (p >= tot) break;
					}
				}
			else
				while (p < ep)
					{
					if (!asn1_parse2(fp,&p,(long)len,
						offset+(p - *pp),depth+1))
						{
						ret=0;
						goto end;
						}
					}
			}
		else
			{
			nl=0;
			if (	(tag == V_ASN1_PRINTABLESTRING) ||
				(tag == V_ASN1_T61STRING) ||
				(tag == V_ASN1_IA5STRING) ||
				(tag == V_ASN1_UTCTIME))
				{
				Fputc(':',fp);
				for (i=0; i<len; i++)
					Fputc(p[i],fp);
				}
			else if (tag == V_ASN1_OBJECT)
				{
				opp=op;
				if (d2i_ASN1_OBJECT(&o,&opp,len+hl) != NULL)
					{
					Fprintf(fp,":");
					i2f_ASN1_OBJECT(fp,o);
					}
				else
					Fprintf(fp,":BAD OBJECT");
				}
			else if (tag == V_ASN1_INTEGER)
				{
				ASN1_INTEGER *bs;
				int i;

				opp=op;
				bs=d2i_ASN1_INTEGER(NULL,&opp,len+hl);
				if (bs != NULL)
					{
					Fputc(':',fp);
					if (bs->type == V_ASN1_NEG_INTEGER)
						Fputc('-',fp);
					for (i=0; i<bs->length; i++)
						Fprintf(fp,"%02X",bs->data[i]);
					if (bs->length == 0)
						Fprintf(fp,"00");
					}
				else
					Fprintf(fp,"BAD INTEGER");
				ASN1_INTEGER_free(bs);
				}

			if (!nl) Fputc('\n',fp);
			p+=len;
			if ((tag == V_ASN1_EOC) && (class == 0))
				{
				ret=0;
				goto end;
				}
			}
		}
end:
	if (o != NULL) ASN1_OBJECT_free(o);
	*pp=p;
	return(ret);
	}

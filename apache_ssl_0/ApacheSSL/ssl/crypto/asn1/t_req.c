/* lib/asn1/t_req.c */
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
#include "bn.h"
#include "rsa.h"
#include "objects.h"
#include "x509.h"

void X509_REQ_print(fp,x)
FILE *fp;
X509_REQ *x;
	{
	unsigned long l;
	int i,n;
	char *m=NULL,*s,*neg;
	X509_REQ_INFO *ri;
	STACK *sk;
	RSA *rsa;

	ri=x->req_info;
	Fprintf(fp,"Certificate Request:\n");
	Fprintf(fp,"%4sData:\n","");
	neg=(ri->version->type == V_ASN1_NEG_INTEGER)?"-":"";
	for (l=i=0; i<ri->version->length; i++)
		{ l<<=8; l+=ri->version->data[i]; }
	Fprintf(fp,"%8sVersion: %s%lu (%s0x%lx)\n","",neg,l,neg,l);
	Fprintf(fp,"%8sSubject: ","");
	X509_NAME_print(fp,ri->subject,16);
	Fprintf(fp,"\n%8sSubject Public Key Info:\n","");
	i=OBJ_obj2nid(ri->pubkey->algor->algorithm);
	Fprintf(fp,"%12sPublic Key Algorithm: %s\n","",
		(i == NID_undef)?"UNKNOWN":OBJ_nid2ln(i));

	rsa=RSA_new();
	s=(char *)ri->pubkey->public_key->data;
	i=ri->pubkey->public_key->length;
	d2i_RSAPublicKey(&rsa,(unsigned char **)&s,(long)i);
	i=RSA_size(rsa);
	m=s=(char *)malloc((unsigned int)i+10);
	s[0]=0;
	n=BN_bn2bin(rsa->n,(unsigned char *)&(s[1]));
	if (s[1] & 0x80)
		n++;
	else	s++;
	
	Fprintf(fp,"%12sPublic Key: (%d bit)\n","",BN_num_bits(rsa->n));
	neg=(rsa->n->neg)?" (Negative)":"";
	Fprintf(fp,"%16sModulus:%s","",neg);

	for (i=0; i<n; i++)
		{
		if ((i%15) == 0) Fprintf(fp,"\n%20s","");
		Fprintf(fp,"%02x%s",(unsigned char)s[i],((i+1) == n)?"":":");
		}
	free(m);
	Fputc('\n',fp);
	neg=(rsa->e->neg)?"-":"";
	l=rsa->e->d[0];
	Fprintf(fp,"%16sExponent: %s%lu (%s0x%lx)\n","",neg,l,neg,l);
	RSA_free(rsa);

	/* may not be */
	Fprintf(fp,"%8sAttributes:\n","");
	sk=x->req_info->attributes;
	if ((sk == NULL) || (sk_num(sk) == 0))
		Fprintf(fp,"%12sa0:00\n","");
	else
		{
		for (i=0; i<sk_num(sk); i++)
			{
			X509_ATTRIBUTE *a;
			ASN1_BIT_STRING *bs;
			ASN1_TYPE *t;
			int j,k;

			a=(X509_ATTRIBUTE *)sk_value(sk,i);
			Fprintf(fp,"%12s","");
			j=i2f_ASN1_OBJECT(fp,a->object);
			if (a->set)
				{
				Fprintf(fp,"\n");
				continue;
				}
			t=a->value.single;
			bs=t->value.bit_string;
			for (j=25-j; j>0; j--)
				Fputc(' ',fp);
			k=t->type;
			Fprintf(fp,":");
			if (	(k == V_ASN1_PRINTABLESTRING) ||
				(k == V_ASN1_T61STRING) ||
				(k == V_ASN1_IA5STRING))
				{
				for (j=0; j<bs->length; j++)
					Fputc(bs->data[j],fp);
				Fputc('\n',fp);
				}
			else
				{
				Fprintf(fp,"unable to print attribute\n");
				}
			}
		}

	i=OBJ_obj2nid(x->sig_alg->algorithm);
	Fprintf(fp,"%4sSignature Algorithm: %s","",
		(i == NID_undef)?"UNKNOWN":OBJ_nid2ln(i));

	n=x->signature->length;
	s=(char *)x->signature->data;
	for (i=0; i<n; i++)
		{
		if ((i%18) == 0) Fprintf(fp,"\n%8s","");
		Fprintf(fp,"%02x%s",(unsigned char)s[i],((i+1) == n)?"":":");
		}
	Fputc('\n',fp);
	}

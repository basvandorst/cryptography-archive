/* lib/asn1/t_x509.c */
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

int X509_print(fp,x)
FILE *fp;
X509 *x;
	{
	unsigned long l;
	int i,n;
	char *m=NULL,*s;
	X509_CINF *ci;
	ASN1_BIT_STRING *bs;
	RSA *rsa=NULL;
	char *neg;

	ci=x->cert_info;
	Fprintf(fp,"Certificate:\n");
	Fprintf(fp,"%4sData:\n","");
	bs=ci->version;
	if ((bs == NULL) || (bs->data == NULL))
		l=0;
	else
		for (l=i=0; i<bs->length; i++)
			{ l<<=8; l+=bs->data[i]; }
	Fprintf(fp,"%8sVersion: %lu (0x%lx)\n","",l,l);
	Fprintf(fp,"%8sSerial Number:","");
	bs=ci->serialNumber;
	if (bs->length <= 4)
		{
		neg=(ci->serialNumber->type == V_ASN1_NEG_INTEGER)?"-":"";
		for (l=i=0; i<bs->length; i++)
			{ l<<=8; l+=bs->data[i]; }
		Fprintf(fp," %s%lu (%s0x%lx)\n",neg,l,neg,l);
		}
	else
		{
		neg=(ci->serialNumber->type == V_ASN1_NEG_INTEGER)?" (Negative)":"";
		Fprintf(fp,"\n%12s%s","",neg);
		for (i=0; i<bs->length; i++)
			Fprintf(fp,"%02x%c",bs->data[i],
				((i+1 == bs->length)?'\n':':'));
		}

	i=OBJ_obj2nid(ci->signature->algorithm);
	Fprintf(fp,"%8sSignature Algorithm: %s\n","",
		(i == NID_undef)?"UNKNOWN":OBJ_nid2ln(i));

	Fprintf(fp,"%8sIssuer: "," ");
	X509_NAME_print(fp,ci->issuer,16);
	Fprintf(fp,"\n%8sValidity\n","");
	Fprintf(fp,"%12sNot Before: ","");
	ASN1_UTCTIME_print(fp,ci->validity->notBefore);
	Fprintf(fp,"\n%12sNot After : ","");
	ASN1_UTCTIME_print(fp,ci->validity->notAfter);
	Fprintf(fp,"\n%8sSubject:"," ");
	X509_NAME_print(fp,ci->subject,16);

	Fprintf(fp,"\n%8sSubject Public Key Info:\n","");
	i=OBJ_obj2nid(ci->key->algor->algorithm);
	Fprintf(fp,"%12sPublic Key Algorithm: %s\n","",
		(i == NID_undef)?"UNKNOWN":OBJ_nid2ln(i));

	s=(char *)ci->key->public_key->data;
	if (d2i_RSAPublicKey(&rsa,(unsigned char **)&s,
		ci->key->public_key->length) == NULL) goto err;
	i=RSA_size(rsa);
	s=m=(char *)malloc((unsigned int)i+10);
	if (m == NULL)
		{
		X509err(X509_F_X509_PRINT,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	Fprintf(fp,"%12sPublic Key: (%d bit)\n","",BN_num_bits(rsa->n));
	Fprintf(fp,"%16sModulus:","");

	n=BN_bn2bin(rsa->n,(unsigned char *)&(s[1]));
	
	m[0]='\0';
	if (m[1] & 0x80)
		n++;
	else	s++;
	for (i=0; i<n; i++)
		{
		if ((i%15) == 0) Fprintf(fp,"\n%20s","");
		Fprintf(fp,"%02x%s",(unsigned char)s[i],((i+1) == n)?"":":");
		}
	free(m);
	Fputc('\n',fp);
	l=rsa->e->d[0];
	Fprintf(fp,"%16sExponent: %lu (0x%lx)\n","",l,l);
	RSA_free(rsa);

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
	return(1);
err:
	if (rsa != NULL) RSA_free(rsa);
	if (m != NULL) free(m);
	return(0);
	}

void ASN1_UTCTIME_print(fp,v)
FILE *fp;
ASN1_UTCTIME *v;
	{
	static char *mon[12]={
		"Jan","Feb","Mar","Apr","May","Jun",
		"Jul","Aug","Sep","Oct","Nov","Dec"};
	int i;
	int y=0,M=0,d=0,h=0,m=0,s=0;

	i=strlen(v);
	if (i < 10) goto err;
	for (i=0; i<10; i++)
		if ((v[i] > '9') || (v[i] < '0')) goto err;
	y= (v[0]-'0')*10+(v[1]-'0');
	if (y < 70) y+=100;
	M= (v[2]-'0')*10+(v[3]-'0');
	if ((M > 12) || (M < 1)) goto err;
	d= (v[4]-'0')*10+(v[5]-'0');
	h= (v[6]-'0')*10+(v[7]-'0');
	m=  (v[8]-'0')*10+(v[9]-'0');
	if (	(v[10] >= '0') && (v[10] <= '9') &&
		(v[11] >= '0') && (v[11] <= '9'))
		s=  (v[10]-'0')*10+(v[11]-'0');

	Fprintf(fp,"%s %2d %02d:%02d:%02d %d",
		mon[M-1],d,h,m,s,y+1900);
	return;
err:
	Fprintf(fp,"Bad time value");
	}

void X509_NAME_print(fp,name,obase)
FILE *fp;
X509_NAME *name;
int obase;
	{
	char *m=NULL,*s,*c;
	int l,ll,i;


	ll=80-2-obase;

	s=m=X509_NAME_oneline(name);
	if (s == NULL) return;
	s++; /* skip the first slash */

	l=ll;
	c=s;
	for (;;)
		{
		if (l <= 0)
			{
			Fputc('\n',fp);
			for (i=0; i<obase; i++) Fputc(' ',fp);
			l=ll;
			}
		if ((*s == '/') || (*s == '\0'))
			{
			while (c != s)
				Fputc(*(c++),fp);
			c++;
			if (*s != '\0')
				{
				Fputc(',',fp);
				Fputc(' ',fp);
				}
			l--;
			}
		if (*s == '\0') break;
		s++;
		l--;
		}
	
	free(m);
	}

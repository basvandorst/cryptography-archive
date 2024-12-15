#ifdef F2I
/* lib/x509/x509_f2i.c */
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
#include "x509.h"
#include "objects.h"
#include "buffer.h"

#ifndef NOPROTO
static int xfgets(char *buf, int size, FILE *fp);
static int f2i_get_token(FILE *fp, int size, char *buf);
#else
static int xfgets();
static int f2i_get_token();
#endif

X509_ALGOR *f2i_X509_ALGOR(fp,a,buf,size)
FILE *fp;
X509_ALGOR **a;
char *buf;
int size;
	{
	M_XFILE_F2I_Init(X509_ALGOR *,X509_ALGOR_new);
	M_XFILE_F2I_Get_Token(a,TOKEN_ALGORITHMIDENTIFIER_BEGIN);
	M_XFILE_F2I_Get(a->algorithm,f2i_ASN1_OBJECT);
	M_XFILE_F2I_Get(a->parameter,f2i_ASN1_OBJECT);

	if (!f2i_ASN1_OBJECT(fp,a->algorithm,size,buf)) return(0);
	if (!f2i_ASN1_BIT_STRING(fp,a->parameter,size,buf)) return(0);
	if (f2i_get_token(fp,size,buf) != TOKEN_ALGORITHMIDENTIFIER_END)
		{
		DERerr(DER_F_F2I_X509_ALGOR,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	return(1);
	}

int f2i_X509_NAME(fp, a, size, buf)
FILE *fp;
X509_NAME *a;
int size;
char *buf;
	{
	int i=0,j;
	DER_OBJECT **objs,*obj;
	DER_BIT_STRING **vals;
	int *types;
	int set=0;
	unsigned char *s;
	int n=0;

	if (f2i_get_token(fp,size,buf) != TOKEN_NAME_BEGIN)
		{
		DERerr(DER_F_F2I_X509_NAME,DER_R_EXPECTING_A_BEGIN_TOKEN);
		return(0);
		}
	objs=a->objects;
	types=a->types;
	vals=a->values;
	n=a->num;
	for (;;)
		{
		if (i >= n)
			{
			n+=10;
			if (objs == NULL)
				{
				objs=(DER_OBJECT **)malloc(
					sizeof(DER_OBJECT *)*n);
				types=(int *)malloc(sizeof(int *)*n);
				vals=(DER_BIT_STRING **)malloc(
					sizeof(DER_BIT_STRING *)*n);
				}
			else	
				{
				objs=(DER_OBJECT **)realloc(objs,
					sizeof(DER_OBJECT *)*n);
				types=(int *)realloc(types,sizeof(int *)*n);
				vals=(DER_BIT_STRING **)realloc(vals,
					sizeof(DER_BIT_STRING *)*n);
				}
			if ((objs == NULL) || (vals == NULL) || (types == NULL))
				{
				DERerr(DER_F_F2I_X509_NAME,
					ERR_R_MALLOC_FAILURE);
				return(0);
				}
			for (j=i; j<n; j++)
				{
				objs[j]=NULL;
				types[j]=0;
				vals[j]=DER_BIT_STRING_new();
				if (vals[j] == NULL) return(0);
				}
			a->objects=objs;
			a->types=types;
			a->values=vals;
			}
		obj=a->objects[i];
		if (obj == NULL)
			{
			obj=(DER_OBJECT *)DER_OBJECT_new();
			if (obj == NULL)
				{
				DERerr(DER_F_F2I_X509_NAME,
					ERR_R_MALLOC_FAILURE);
				return(0);
				}
			}
		if (!f2i_ASN1_OBJECT(fp,obj,size,buf))
			return(0);
		a->objects[i]=obj;

/* EAY FIX - change to print any type */
		if (!f2i_PRINTABLESTRING(fp,&s,size,buf)) return(0);
		a->values[i]->length=strlen((char *)s);
		set++;
		a->types[i]=(DER_PRINTABLESTRING|(set<<8));
		a->values[i]->data=s;
		a->num++;
		i++;
		if (buf[0] == '-') break;
		}
	if (f2i_get_token(fp,size,buf) != TOKEN_NAME_END)
		{
		DERerr(DER_F_F2I_X509_NAME,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	return(1);
	}

int f2i_X509_VAL(fp, a, size, buf)
FILE *fp;
X509_VAL *a;
int size;
char *buf;
	{
	if (f2i_get_token(fp,size,buf) != TOKEN_VALIDITY_BEGIN)
		{
		DERerr(DER_F_F2I_X509_VAL,DER_R_EXPECTING_A_BEGIN_TOKEN);
		return(0);
		}
	if (!f2i_UTCTime(fp,(unsigned char **)&a->notBefore,size,buf))
		return(0);
	if (!f2i_UTCTime(fp,(unsigned char **)&a->notAfter,size,buf))
		return(0);
	if (f2i_get_token(fp,size,buf) != TOKEN_VALIDITY_END)
		{
		DERerr(DER_F_F2I_X509_VAL,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	return(1);
	}

int f2i_X509_PUBKEY(fp, a, size, buf)
FILE *fp;
X509_PUBKEY *a;
int size;
char *buf;
	{
	if (f2i_get_token(fp,size,buf) != TOKEN_SUBJECTPUBLICKEYINFO_BEGIN)
		{
		DERerr(DER_F_F2I_X509_PUBKEY,DER_R_EXPECTING_A_BEGIN_TOKEN);
		return(0);
		}
	if (!f2i_X509_ALGOR(fp,a->algor,size,buf)) return(0);
	if (!f2i_ASN1_BIT_STRING(fp,a->public_key,size,buf)) return(0);
	if (f2i_get_token(fp,size,buf) != TOKEN_SUBJECTPUBLICKEYINFO_END)
		{
		DERerr(DER_F_F2I_X509_PUBKEY,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	return(1);
	}

int f2i_X509_SIG(fp, a, size, buf)
FILE *fp;
X509_SIG *a;
int size;
char *buf;
	{
	if (f2i_get_token(fp,size,buf) != TOKEN_SIGNATURE_BEGIN)
		{
		DERerr(DER_F_F2I_X509_SIG,DER_R_EXPECTING_A_BEGIN_TOKEN);
		return(0);
		}
	if (!f2i_X509_ALGOR(fp,a->algor,size,buf)) return(0);
	if (!f2i_ASN1_BIT_STRING(fp,a->digest,size,buf)) return(0);
	if (f2i_get_token(fp,size,buf) != TOKEN_SIGNATURE_END)
		{
		DERerr(DER_F_F2I_X509_SIG,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	return(1);
	}

int f2i_X509_CINF(fp, a, size, buf)
FILE *fp;
X509_CINF *a;
int size;
char *buf;
	{
	if (f2i_get_token(fp,size,buf) != TOKEN_CERTIFICATE_BEGIN)
		{
		DERerr(DER_F_F2I_X509_CINF,DER_R_EXPECTING_A_BEGIN_TOKEN);
		return(0);
		}
/*	Fprintf(fp,"= %ld\n",a->version); */
	if (!f2i_ASN1_BIT_STRING(fp,a->serialNumber,size,buf)) return(0);
	if (!f2i_X509_ALGOR(fp,a->signature,size,buf)) return(0);
	if (!f2i_X509_NAME(fp,a->issuer,size,buf)) return(0);
	if (!f2i_X509_VAL(fp,a->validity,size,buf)) return(0);
	if (!f2i_X509_NAME(fp,a->subject,size,buf)) return(0);
	if (!f2i_X509_PUBKEY(fp,a->key,size,buf)) return(0);
	if (f2i_get_token(fp,size,buf) != TOKEN_CERTIFICATE_END)
		{
		DERerr(DER_F_F2I_X509_CINF,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	return(1);
	}

int f2i_X509_REVOKED(fp,a,size,buf)
FILE *fp;
X509_REVOKED **a;
int size;
char *buf;
	{
	X509_REVOKED *p;

	p=X509_REVOKED_new();
	if (p == NULL)
		{
		DERerr(DER_F_F2I_X509_REVOKED,ERR_R_MALLOC_FAILURE);
		return(0);
		}
	if (!f2i_ASN1_BIT_STRING(fp,p->serialNumber,size,buf)) return(0);
	if (!f2i_UTCTime(fp,(unsigned char **)&p->revocationDate,size,buf))
		return(0);
	*a=p;
	return(1);
	}

int f2i_X509_REVOKED_list(fp,a,size,buf)
FILE *fp;
X509_CRL_INFO *a;
int size;
char *buf;
	{
	X509_REVOKED *p,**pp;
	int i,numa;

	numa=a->num;
	pp=a->revoked;
	if (pp == NULL)
		numa=0;
	else
		for (i=0; i<a->num; i++)
			{
			X509_REVOKED_free(a->revoked[i]);
			a->revoked[i]=NULL;
			}
	i=0;
	for (;;)
		{
		if (!f2i_X509_REVOKED(fp,&p,size,buf)) return(0);
		if (numa <= i)
			{
			numa+=10;
			if (pp != NULL)
				{
				pp=(X509_REVOKED **)realloc(pp,
					sizeof(X509_REVOKED *)*numa);
				}
			else
				{
				pp=(X509_REVOKED **)malloc(
					sizeof(X509_REVOKED *)*numa);
				}
			if (pp == NULL)
				{
				DERerr(DER_F_F2I_X509_REVOKED_LIST,ERR_R_MALLOC_FAILURE);
				return(0);
				}
			}
		pp[i]=p;
		i++;
		if (buf[0] == '-') break;
		}
	a->revoked=pp;
	a->num=i;
	return(1);
	}

int f2i_X509_CRL_INFO(fp, a, size, buf)
FILE *fp;
X509_CRL_INFO *a;
int size;
char *buf;
	{
	if (f2i_get_token(fp,size,buf) != TOKEN_CRL_INFO_BEGIN)
		{
		DERerr(DER_F_F2I_X509_CRL_INFO,DER_R_EXPECTING_A_BEGIN_TOKEN);
		return(0);
		}
	if (!f2i_X509_ALGOR(fp,a->sig_alg,size,buf)) return(0);
	if (!f2i_X509_NAME(fp,a->issuer,size,buf)) return(0);
	if (!f2i_UTCTime(fp,(unsigned char **)&a->lastUpdate,size,buf))
		return(0);
	if (!f2i_UTCTime(fp,(unsigned char **)&a->nextUpdate,size,buf))
		return(0);
	
	if (buf[0] == '=')
		{
		if (!f2i_X509_REVOKED_list(fp,a,size,buf)) return(0);
		}
	if (f2i_get_token(fp,size,buf) != TOKEN_CRL_INFO_END)
		{
		DERerr(DER_F_F2I_X509_CRL_INFO,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	return(1);
	}

int f2i_X509_CRL(fp, a)
FILE *fp;
X509_CRL *a;
	{
	BUFFER *b;

	b=BUF_new();
	if ((b == NULL) || !BUF_grow(b,1024*4))
		{
		DERerr(DER_F_F2I_X509_CRL,ERR_R_MALLOC_FAILURE);
		if (b != NULL) goto err;
		return(0);
		}

	if (f2i_get_token(fp,b->length,b->data) != TOKEN_CRL_BEGIN)
		{
		DERerr(DER_F_F2I_X509_CRL,DER_R_EXPECTING_A_BEGIN_TOKEN);
		goto err;
		}
	
	if (!f2i_X509_CRL_INFO(fp,a->crl,b->length,b->data)) goto err;
	if (!f2i_X509_ALGOR(fp,a->sig_alg,b->length,b->data)) goto err;
	if (!f2i_ASN1_BIT_STRING(fp,a->signature,b->length,b->data)) goto err;
	if (f2i_get_token(fp,b->length,b->data) != TOKEN_CRL_END)
		{
		DERerr(DER_F_F2I_X509_CRL,DER_R_EXPECTING_A_END_TOKEN);
		goto err;
		}
	BUF_free(b);
	return(1);
err:
	BUF_free(b);
	return(0);
	}

int f2i_X509(fp, a)
FILE *fp;
X509 *a;
	{
	BUFFER *b;

	b=BUF_new();
	if ((b == NULL) || !BUF_grow(b,1024*4))
		{
		DERerr(DER_F_F2I_X509,ERR_R_MALLOC_FAILURE);
		if (b != NULL) goto err;
		return(0);
		}

	if (f2i_get_token(fp,b->length,b->data) != TOKEN_X509_CERTIFICATE_BEGIN)
		{
		DERerr(DER_F_F2I_X509,DER_R_EXPECTING_A_BEGIN_TOKEN);
		goto err;
		}
	if (!f2i_X509_CINF(fp,a->cert_info,b->length,b->data)) goto err;
	if (!f2i_X509_ALGOR(fp,a->sig_alg,b->length,b->data)) goto err;
	if (!f2i_ASN1_BIT_STRING(fp,a->signature,b->length,b->data)) goto err;
	if (f2i_get_token(fp,b->length,b->data) != TOKEN_X509_CERTIFICATE_END)
		{
		DERerr(DER_F_F2I_X509,DER_R_EXPECTING_A_END_TOKEN);
		goto err;
		}
	BUF_free(b);
	return(1);
err:
	BUF_free(b);
	return(0);
	}

static int f2i_get_token(fp, size, buf)
FILE *fp;
int size;
char *buf;
	{
	int len;

	if ((len=xfgets(buf,size,fp)) == 0) return(TOKEN_ERROR);
	buf[len-1]='\0';
	/* shoud do this with a hash table to make it fast */
	if (strcmp(buf,STRING_X509_CERTIFICATE_BEGIN) == 0)
		return(TOKEN_X509_CERTIFICATE_BEGIN);
	if (strcmp(buf,STRING_X509_CERTIFICATE_END) == 0)
		return(TOKEN_X509_CERTIFICATE_END);
	if (strcmp(buf,STRING_CERTIFICATE_BEGIN) == 0)
		return(TOKEN_CERTIFICATE_BEGIN);
	if (strcmp(buf,STRING_CERTIFICATE_END) == 0)
		return(TOKEN_CERTIFICATE_END);
	if (strcmp(buf,STRING_ALGORITHMIDENTIFIER_BEGIN) == 0)
		return(TOKEN_ALGORITHMIDENTIFIER_BEGIN);
	if (strcmp(buf,STRING_ALGORITHMIDENTIFIER_END) == 0)
		return(TOKEN_ALGORITHMIDENTIFIER_END);
	if (strcmp(buf,STRING_NAME_BEGIN) == 0)
		return(TOKEN_NAME_BEGIN);
	if (strcmp(buf,STRING_NAME_END) == 0)
		return(TOKEN_NAME_END);
	if (strcmp(buf,STRING_VALIDITY_BEGIN) == 0)
		return(TOKEN_VALIDITY_BEGIN);
	if (strcmp(buf,STRING_VALIDITY_END) == 0)
		return(TOKEN_VALIDITY_END);
	if (strcmp(buf,STRING_SUBJECTPUBLICKEYINFO_BEGIN) == 0)
		return(TOKEN_SUBJECTPUBLICKEYINFO_BEGIN);
	if (strcmp(buf,STRING_SUBJECTPUBLICKEYINFO_END) == 0)
		return(TOKEN_SUBJECTPUBLICKEYINFO_END);
	if (strcmp(buf,STRING_RSAPUBLICKEY_BEGIN) == 0)
		return(TOKEN_RSAPUBLICKEY_BEGIN);
	if (strcmp(buf,STRING_RSAPUBLICKEY_END) == 0)
		return(TOKEN_RSAPUBLICKEY_END);
	if (strcmp(buf,STRING_RSAPRIVATEKEY_BEGIN) == 0)
		return(TOKEN_RSAPRIVATEKEY_BEGIN);
	if (strcmp(buf,STRING_RSAPRIVATEKEY_END) == 0)
		return(TOKEN_RSAPRIVATEKEY_END);
	if (strcmp(buf,STRING_CRL_BEGIN) == 0)
		return(TOKEN_CRL_BEGIN);
	if (strcmp(buf,STRING_CRL_END) == 0)
		return(TOKEN_CRL_END);
	if (strcmp(buf,STRING_CRL_INFO_BEGIN) == 0)
		return(TOKEN_CRL_INFO_BEGIN);
	if (strcmp(buf,STRING_CRL_INFO_END) == 0)
		return(TOKEN_CRL_INFO_END);
	if (strcmp(buf,STRING_REQ_INFO_BEGIN) == 0)
		return(TOKEN_REQ_INFO_BEGIN);
	if (strcmp(buf,STRING_REQ_INFO_END) == 0)
		return(TOKEN_REQ_INFO_END);
	if (strcmp(buf,STRING_REQ_BEGIN) == 0)
		return(TOKEN_REQ_BEGIN);
	if (strcmp(buf,STRING_REQ_END) == 0)
		return(TOKEN_REQ_END);
	if (strcmp(buf,STRING_OPT) == 0)
		return(TOKEN_OPT);
	return(TOKEN_ERROR);
	}

static int xfgets(buf, size, fp)
char *buf;
int size;
FILE *fp;
	{
	int ret;

	buf[0]='\0';
	Fgets(buf,size,fp);
	if (buf[0] == '\0') return(0);
	ret=strlen(buf);
	return(ret);
	}

int f2i_PRINTABLESTRING(fp, s, size, buf)
FILE *fp;
unsigned char **s;
int size;
char *buf;
	{
	int i;

	size=xfgets(buf,size,fp);
	if (size < 3)
		{
		DERerr(DER_F_F2I_PRINTABLESTRING,DER_R_SHORT_LINE);
		return(0);
		}
	if (	!((buf[0] == '=') || (buf[0] == '-')) ||
		(buf[1] != ' '))
		{
		DERerr(DER_F_F2I_PRINTABLESTRING,DER_R_BAD_SOL_CHARACTER);
		return(0);
		}
	i=strlen(buf);
	if (buf[i-1] != '\n')
		{
		DERerr(DER_F_F2I_PRINTABLESTRING,DER_R_BAD_EOL_CHARACTER);
		return(0);
		}
	buf[i-1]='\0';
	buf+=2;
	i-=3;
	*s=(unsigned char *)malloc((unsigned int)i+1);
	if (*s == NULL)
		{
		DERerr(DER_F_F2I_PRINTABLESTRING,ERR_R_MALLOC_FAILURE);
		return(0);
		}
	strcpy((char *)*s,buf);
	return(1);
	}

int f2i_ASN1_BIT_STRING(fp, bs, size, buf)
FILE *fp;
DER_BIT_STRING *bs;
int size;
char *buf;
	{
	int i,j,k,m,n,again,bufsize;
	unsigned char *s=NULL;
	int num=0,slen=0;

	bufsize=xfgets(buf,size,fp);
	if ((bufsize > 6) && (strncmp("= NULL",buf,6) == 0))
			return(1); /* null entry */
	
	for (;;)
		{
		if (bufsize < 4) return(0);
		if (!((buf[0] == '=') || (buf[0] == '-')) ||
			(buf[1] != ' '))
			{
			DERerr(DER_F_F2I_ASN1_BIT_STRING,DER_R_BAD_SOL_CHARACTER);
			return(0);
			}
		i=strlen(buf);
		if (buf[i-1] != '\n')
			{
			DERerr(DER_F_F2I_ASN1_BIT_STRING,DER_R_BAD_EOL_CHARACTER);
			return(0);
			}
		buf[i-1]='\0';
		again=(buf[0] == '=');
		k=2;
		i-=3;
		if (i%2 != 0)
			{
			DERerr(DER_F_F2I_ASN1_BIT_STRING,DER_R_ODD_NUMBER_OF_CHARS);
			return(0);
			}
		i/=2;
		if (num+i > slen)
			{
			if (s == NULL)
				s=(unsigned char *)malloc(
					(unsigned int)num+i*2);
			else
				s=(unsigned char *)realloc(s,
					(unsigned int)num+i*2);
			if (s == NULL)
				{
				DERerr(DER_F_F2I_ASN1_BIT_STRING,ERR_R_MALLOC_FAILURE);
				return(0);
				}
			slen=num+i*2;
			}
		for (j=0; j<i; j++,k+=2)
			{
			for (n=0; n<2; n++)
				{
				m=buf[k+n];
				if ((m >= '0') && (m <= '9'))
					m-='0';
				else if ((m >= 'a') && (m <= 'f'))
					m=m-'a'+10;
				else if ((m >= 'A') && (m <= 'F'))
					m=m-'A'+10;
				else
					{
					DERerr(DER_F_F2I_ASN1_BIT_STRING,DER_R_NON_HEX_CHARACTERS);
					return(0);
					}
				s[num+j]<<=4;
				s[num+j]|=m;
				}
			}
		num+=i;
		if (again)
			bufsize=xfgets(buf,size,fp);
		else
			break;
		}
	bs->length=num;
	bs->data=s;
	return(1);
	}

int f2i_UTCTime(fp, s, size, buf)
FILE *fp;
unsigned char **s;
int size;
char *buf;
	{
	return(f2i_PRINTABLESTRING(fp,s,size,buf));
	}

int f2i_ASN1_OBJECT(fp, o, size, buf)
FILE *fp;
DER_OBJECT *o;
int size;
char *buf;
	{
	int nid,i,ok,j,num=0;
	char *b;
	unsigned long l;
	DER_OBJECT *no;

	size=xfgets(buf,size,fp);
	if (size < 3)
		{
		DERerr(DER_F_F2I_ASN1_OBJECT,DER_R_SHORT_LINE);
		return(0);
		}
	if (	!((buf[0] == '=') || (buf[0] == '-')) ||
		(buf[1] != ' '))
		{
		DERerr(DER_F_F2I_ASN1_OBJECT,DER_R_BAD_SOL_CHARACTER);
		return(0);
		}
	i=strlen(buf);
	if (buf[i-1] != '\n')
		{
		DERerr(DER_F_F2I_ASN1_OBJECT,DER_R_BAD_EOL_CHARACTER);
		return(0);
		}
	buf[i-1]='\0';
	buf+=2;
	i-=3;
	b=buf;

	/* first check for text object names */
	while (*b == ' ') b++; /* skip white space */
	nid=OBJ_ln2nid(b);
	if (nid == NID_undef) nid=OBJ_sn2nid(b);
	if (nid != NID_undef)
		{
		no=OBJ_dup(OBJ_nid2obj(nid));
		if (no == NULL) return(0);
		o->num=no->num;
		o->values=no->values;
		o->nid=no->nid;
		o->sn=no->sn;
		o->ln=no->ln;
		free(no);
		return(1);
		}
		
	/* else, look for numbers*/
	b=buf;
	for (j=0; j<i; j++)
		{
		/* skip white space */
		while (*b == ' ') b++;
		/* a number? */
		if ((*b >= '0') && (*b <= '9')) num++;
		/* skip number */
		while ((*b >= '0') && (*b <= '9')) b++;
		}
	if (o->num < num)
		{
		if (o->values != NULL) free(o->values);
		o->values=(unsigned long *)malloc(sizeof(unsigned long)*num);
		if (o->values == NULL)
			{
			DERerr(DER_F_F2I_ASN1_OBJECT,ERR_R_MALLOC_FAILURE);
			return(0);
			}
		}
	o->num=num;

	b=buf;
	num=0;
	for (j=0; j<i; j++)
		{
		l=0;
		ok=0;
		while (*b == ' ') b++;
		while ((*b >= '0') && (*b <= '9'))
			{
			l=l*10+(*b-'0');
			ok++;
			b++;
			}
		if (ok)
			{
			o->values[num]=l;
			num++;
			}
		}
	return(1);
	}

int f2i_RSAPrivateKey(fp, rsa)
FILE *fp;
RSA *rsa;
	{
	DER_BIT_STRING bs1,bs2;
	BUFFER *buf;

	buf=BUF_new();
	if (buf == NULL)
		{
		DERerr(DER_F_F2I_RSAPRIVATEKEY,ERR_R_MALLOC_FAILURE);
		goto err2;
		}

	if (!BUF_grow(buf,1024*4))
		{
		DERerr(DER_F_F2I_RSAPRIVATEKEY,ERR_R_MALLOC_FAILURE);
		goto err2;
		}
	bs1.length=0; bs1.data=NULL;
	bs2.length=0; bs2.data=NULL;
	if (f2i_get_token(fp,buf->length,buf->data) != TOKEN_RSAPRIVATEKEY_BEGIN)
		{
		DERerr(DER_F_F2I_RSAPRIVATEKEY,DER_R_EXPECTING_A_BEGIN_TOKEN);
		goto err2;
		}
	/* read and throw away version number */
	if (!f2i_ASN1_BIT_STRING(fp,&bs1,buf->length,buf->data)) goto err;
	rsa->n=BN_bin2bn(bs1.data,bs1.length,rsa->n);
	if (rsa->n == NULL) goto err;

	if (!f2i_ASN1_BIT_STRING(fp,&bs1,buf->length,buf->data)) goto err;
	rsa->n=BN_bin2bn(bs1.data,bs1.length,rsa->n);
	if (rsa->n == NULL) goto err;

	if (!f2i_ASN1_BIT_STRING(fp,&bs2,buf->length,buf->data)) goto err;
	rsa->e=BN_bin2bn(bs2.data,bs2.length,rsa->e);
	if (rsa->e == NULL) goto err;

	if (!f2i_ASN1_BIT_STRING(fp,&bs1,buf->length,buf->data)) goto err;
	rsa->d=BN_bin2bn(bs1.data,bs1.length,rsa->d);
	if (rsa->d == NULL) goto err;

	if (!f2i_ASN1_BIT_STRING(fp,&bs1,buf->length,buf->data)) goto err;
	rsa->p=BN_bin2bn(bs1.data,bs1.length,rsa->p);
	if (rsa->p == NULL) goto err;

	if (!f2i_ASN1_BIT_STRING(fp,&bs1,buf->length,buf->data)) goto err;
	rsa->q=BN_bin2bn(bs1.data,bs1.length,rsa->q);
	if (rsa->q == NULL) goto err;

	if (!f2i_ASN1_BIT_STRING(fp,&bs1,buf->length,buf->data)) goto err;
	rsa->dmp1=BN_bin2bn(bs1.data,bs1.length,rsa->dmp1);
	if (rsa->dmp1 == NULL) goto err;

	if (!f2i_ASN1_BIT_STRING(fp,&bs1,buf->length,buf->data)) goto err;
	rsa->dmq1=BN_bin2bn(bs1.data,bs1.length,rsa->dmq1);
	if (rsa->dmq1 == NULL) goto err;

	if (!f2i_ASN1_BIT_STRING(fp,&bs1,buf->length,buf->data)) goto err;
	rsa->iqmp=BN_bin2bn(bs1.data,bs1.length,rsa->iqmp);
	if (rsa->iqmp == NULL) goto err;

	free(bs1.data);
	free(bs2.data);
	if (f2i_get_token(fp,buf->length,buf->data) != TOKEN_RSAPRIVATEKEY_END)
		{
		DERerr(DER_F_F2I_RSAPRIVATEKEY,DER_R_EXPECTING_A_END_TOKEN);
		goto err2;
		}
	BUF_free(buf);
	return(1);
err:
	DERerr(DER_F_F2I_RSAPRIVATEKEY,ERR_R_RSA_LIB);
err2:
	BUF_free(buf);
	return(0);
	}

int f2i_RSAPublicKey(fp, rsa, size, buf)
FILE *fp;
RSA *rsa;
int size;
char *buf;
	{
	DER_BIT_STRING bs1;

	bs1.length=0; bs1.data=NULL;
	if (f2i_get_token(fp,size,buf) != TOKEN_RSAPUBLICKEY_BEGIN)
		{
		DERerr(DER_F_F2I_RSAPUBLICKEY,DER_R_EXPECTING_A_BEGIN_TOKEN);
		return(0);
		}
	if (!f2i_ASN1_BIT_STRING(fp,&bs1,size,buf)) return(0);
	rsa->n=BN_bin2bn(bs1.data,bs1.length,rsa->n);
	if (rsa->n == NULL)
		{ DERerr(DER_F_F2I_RSAPUBLICKEY,ERR_R_RSA_LIB); return(0); }

	if (!f2i_ASN1_BIT_STRING(fp,&bs1,size,buf)) return(0);
	rsa->e=BN_bin2bn(bs1.data,bs1.length,rsa->e);
	if (rsa->e == NULL)
		{ DERerr(DER_F_F2I_RSAPUBLICKEY,ERR_R_RSA_LIB); return(0); }

	free(bs1.data);
	if (f2i_get_token(fp,size,buf) != TOKEN_RSAPUBLICKEY_END)
		{
		DERerr(DER_F_F2I_RSAPUBLICKEY,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	if (rsa->d != NULL) BN_free(rsa->d);
	if (rsa->p != NULL) BN_free(rsa->d);
	if (rsa->q != NULL) BN_free(rsa->d);
	if (rsa->dmp1 != NULL) BN_free(rsa->d);
	if (rsa->dmq1 != NULL) BN_free(rsa->d);
	if (rsa->iqmp != NULL) BN_free(rsa->d);
	return(1);
	}

int f2i_X509_REQ_INFO(fp,a,size,buf)
FILE *fp;
X509_REQ_INFO *a;
int size;
char *buf;
	{
	if (f2i_get_token(fp,size,buf) != TOKEN_REQ_INFO_BEGIN)
		{
		DERerr(DER_F_F2I_X509_REQ_INFO,DER_R_EXPECTING_A_BEGIN_TOKEN);
		return(0);
		}
	if (!f2i_ASN1_BIT_STRING(fp,a->version,size,buf)) return(0);
	if (!f2i_X509_NAME(fp,a->subject,size,buf)) return(0);
	if (!f2i_X509_PUBKEY(fp,a->pubkey,size,buf)) return(0);
	if (f2i_get_token(fp,size,buf) != TOKEN_REQ_INFO_END)
		{
		DERerr(DER_F_F2I_X509_REQ_INFO,DER_R_EXPECTING_A_END_TOKEN);
		return(0);
		}
	return(1);
	}

int f2i_X509_REQ(fp,a)
FILE *fp;
X509_REQ *a;
	{
	BUFFER *b;

	b=BUF_new();
	if ((b == NULL) || !BUF_grow(b,1024*4))
		{
		DERerr(DER_F_F2I_X509_REQ,ERR_R_MALLOC_FAILURE);
		if (b != NULL) goto err;
		return(0);
		}
	if (f2i_get_token(fp,b->length,b->data) != TOKEN_REQ_BEGIN)
		{
		DERerr(DER_F_F2I_X509_REQ,DER_R_EXPECTING_A_BEGIN_TOKEN);
		goto err;
		}
	if (!f2i_X509_REQ_INFO(fp,a->req_info,b->length,b->data)) goto err;
	if (!f2i_X509_ALGOR(fp,a->sig_alg,b->length,b->data)) goto err;
	if (!f2i_ASN1_BIT_STRING(fp,a->signature,b->length,b->data)) goto err;
	if (f2i_get_token(fp,b->length,b->data) != TOKEN_REQ_END)
		{
		DERerr(DER_F_F2I_X509_REQ,DER_R_EXPECTING_A_END_TOKEN);
		goto err;
		}
	BUF_free(b);
	return(1);
err:
	BUF_free(b);
	return(0);
	}
#endif

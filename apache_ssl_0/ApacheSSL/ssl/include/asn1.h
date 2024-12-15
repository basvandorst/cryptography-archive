/* lib/asn1/asn1.h */
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

#ifndef HEADER_ASN1_H
#define HEADER_ASN1_H

#include "bn.h"
#include "stack.h"

#define V_ASN1_UNIVERSAL		0x00
#define	V_ASN1_APPLICATION		0x40
#define V_ASN1_CONTEXT_SPECIFIC		0x80
#define V_ASN1_PRIVATE			0xc0

#define V_ASN1_CONSTRUCTED		0x20
#define V_ASN1_PRIMATIVE_TAG		0x1f

#define V_ASN1_EOC			0
#define V_ASN1_INTEGER			2
#define V_ASN1_NEG_INTEGER		(2+0x100)
#define V_ASN1_BIT_STRING		3
#define V_ASN1_OCTET_STRING		4
#define V_ASN1_NULL			5
#define V_ASN1_OBJECT			6
#define V_ASN1_SEQUENCE			16
#define V_ASN1_SET			17
#define V_ASN1_PRINTABLESTRING		19
#define V_ASN1_T61STRING		20
#define V_ASN1_IA5STRING		22
#define V_ASN1_UTCTIME			23

#define ASN1_INTEGER		ASN1_BIT_STRING
#define ASN1_OCTET_STRING	ASN1_BIT_STRING
#define ASN1_PRINTABLESTRING	ASN1_BIT_STRING
#define ASN1_T61STRING		ASN1_BIT_STRING
#define ASN1_IA5STRING		ASN1_BIT_STRING
#define ASN1_UTCTIME		char

typedef struct asn1_ctx_st
	{
	unsigned char *p;/* work char pointer */
	int eos;	/* end of sequence read for indefinite encoding */
	int error;	/* error code to use when returning an error */
	int inf;	/* constructed if 0x20, indefinite is 0x21 */
	int tag;	/* tag from last 'get object' */
	int class;	/* class from last 'get object' */
	long slen;	/* length of last 'get object' */
	unsigned char *max; /* largest value of p alowed */
	unsigned char *q;/* temporary variable */
	unsigned char **pp;/* variable */
	} ASN1_CTX;

typedef struct asn1_object_st
	{
	/* both null if a dynamic ASN1_OBJECT, one is
	 * defined if a 'static' ASN1_OBJECT */
	char *sn,*ln;
	int nid;
	int length;
	unsigned char *data;
	} ASN1_OBJECT;

/*
typedef struct asn1_object_st
	{
	int num;
	unsigned long *values;
	} ASN1_OBJECT_EXPANDED;
*/

typedef struct asn1_bit_string_st
	{
	int length;
	int type;
	unsigned char *data;
	} ASN1_BIT_STRING;

typedef struct asn1_type_st
	{
	int type;
	union	{
		char *ptr;
		ASN1_INTEGER *		integer;
		ASN1_BIT_STRING *	bit_string;
		ASN1_OCTET_STRING *	octet_string;
		ASN1_OBJECT *		object;
		ASN1_PRINTABLESTRING *	printablestring;
		ASN1_T61STRING *	t61string;
		ASN1_IA5STRING *	ia5string;
		ASN1_UTCTIME *		utctime;
		ASN1_BIT_STRING *	set;
		ASN1_BIT_STRING *	sequence;
		} value;
	} ASN1_TYPE;

#define ASN1_get_type(t)	((t)&0xff)
#define ASN1_get_set(t)		(((t)>>8)&0xff)
#define ASN1_set_type(t,v)	((t)=((t)&0xff00)|(v))
#define ASN1_set_set(t,v)	((t)=((t)&0x00ff)|(((v)<<8)&0xff00))

#define ASN1_BIT_STRING_new()	ASN1_BIT_STRING_type_new(V_ASN1_BIT_STRING)
#define ASN1_INTEGER_new()	ASN1_BIT_STRING_type_new(V_ASN1_INTEGER)
#define ASN1_INTEGER_free(a)		ASN1_BIT_STRING_free(a)
#define ASN1_OCTET_STRING_new()	ASN1_BIT_STRING_type_new(V_ASN1_OCTET_STRING)
#define ASN1_OCTET_STRING_free(a)	ASN1_BIT_STRING_free(a)
#define ASN1_PRINTABLE_new()		ASN1_BIT_STRING_type_new(SN1_T61STRING)
#define ASN1_PRINTABLE_free(a)		ASN1_BIT_STRING_free(a)
#define ASN1_PRINTABLESTRING_new() \
		ASN1_BIT_STRING_type_new(V_ASN1_PRINTABLESTRING)
#define ASN1_PRINTABLESTRING_free(a)	ASN1_BIT_STRING_free(a)
#define i2d_ASN1_PRINTABLESTRING(a,pp) \
		i2d_ASN1_bytes(a,pp,V_ASN1_PRINTABLESTRING,V_ASN1_UNIVERSAL)
#define ASN1_T61STRING_new()	ASN1_BIT_STRING_type_new(V_ASN1_T61STRING)
#define ASN1_T61STRING_free(a)	ASN1_BIT_STRING_free(a)
#define i2d_ASN1_T61STRING(a,pp) \
		i2d_ASN1_bytes(a,pp,V_ASN1_T61STRING,V_ASN1_UNIVERSAL)
#define ASN1_IA5STRING_new()	ASN1_BIT_STRING_type_new(V_ASN1_IA5STRING)
#define ASN1_IA5STRING_free(a)	ASN1_BIT_STRING_free(a)

#define M_d2i_ASN1_PRINTABLESTRING(a,pp,l) \
		d2i_asn1_print_type(a,pp,l,V_ASN1_PRINTABLESTRING)
#define M_d2i_ASN1_T61STRING(a,pp,l) \
		d2i_asn1_print_type(a,pp,l,V_ASN1_T61STRING)
#define M_i2d_ASN1_IA5STRING(a,pp) \
		i2d_ASN1_bytes(a,pp,V_ASN1_IA5STRING,V_ASN1_UNIVERSAL)
#define M_d2i_ASN1_IA5STRING(a,pp,l) \
		d2i_asn1_print_type(a,pp,l,V_ASN1_IA5STRING)
#define M_i2d_ASN1_PRINTABLE(a,pp) i2d_ASN1_bytes(a,pp,a->type,V_ASN1_UNIVERSAL)
#define M_d2i_ASN1_PRINTABLE(a,pp,l)	d2i_asn1_print_type(a,pp,l,0)

#ifndef NOPROTO
ASN1_TYPE *	ASN1_TYPE_new(void );
void		ASN1_TYPE_free(ASN1_TYPE *a);
int		i2d_ASN1_TYPE(ASN1_TYPE *a,unsigned char **pp);
ASN1_TYPE *	d2i_ASN1_TYPE(ASN1_TYPE **a,unsigned char **pp,long length);

ASN1_OBJECT *	ASN1_OBJECT_new(void );
void		ASN1_OBJECT_free(ASN1_OBJECT *a);
int		i2d_ASN1_OBJECT(ASN1_OBJECT *a,unsigned char **pp);
ASN1_OBJECT *	d2i_ASN1_OBJECT(ASN1_OBJECT **a,unsigned char **pp,
			long length);

int ASN1_BIT_STRING_cmp(ASN1_BIT_STRING *a, ASN1_BIT_STRING *b);

ASN1_BIT_STRING *ASN1_BIT_STRING_type_new(int type );
void		ASN1_BIT_STRING_free(ASN1_BIT_STRING *a);
int		i2d_ASN1_BIT_STRING(ASN1_BIT_STRING *a,unsigned char **pp);
ASN1_BIT_STRING *d2i_ASN1_BIT_STRING(ASN1_BIT_STRING **a,unsigned char **pp,
			long length);

int		i2d_ASN1_INTEGER(ASN1_BIT_STRING *a,unsigned char **pp);
ASN1_BIT_STRING *d2i_ASN1_INTEGER(ASN1_BIT_STRING **a,unsigned char **pp,
			long length);
int ASN1_UTCTIME_check(ASN1_UTCTIME *a);
void ASN1_UTCTIME_print(FILE *fp,ASN1_UTCTIME *a);
ASN1_UTCTIME *ASN1_UTCTIME_dup(ASN1_UTCTIME *a);

int		i2d_ASN1_OCTET_STRING(ASN1_BIT_STRING *a,unsigned char **pp);
ASN1_BIT_STRING *d2i_ASN1_OCTET_STRING(ASN1_BIT_STRING **a,unsigned char **pp,
			long length);

ASN1_BIT_STRING *d2i_asn1_print_type(ASN1_BIT_STRING **a,unsigned char **pp,
		long length,int type);

int i2d_ASN1_PRINTABLE(ASN1_BIT_STRING *a,unsigned char **pp);
ASN1_BIT_STRING *d2i_ASN1_PRINTABLE(ASN1_BIT_STRING **a,
	unsigned char **pp, long l);

ASN1_PRINTABLESTRING *d2i_ASN1_PRINTABLESTRING(ASN1_PRINTABLESTRING **a,
	unsigned char **pp, long l);

ASN1_T61STRING *d2i_ASN1_T61STRING(ASN1_T61STRING **a,
	unsigned char **pp, long l);

int i2d_ASN1_IA5STRING(ASN1_IA5STRING *a,unsigned char **pp);
ASN1_PRINTABLESTRING *d2i_ASN1_IA5STRING(ASN1_IA5STRING **a,
	unsigned char **pp, long l);

ASN1_UTCTIME *	ASN1_UTCTIME_new(void );
void		ASN1_UTCTIME_free(ASN1_UTCTIME *a);
int		i2d_ASN1_UTCTIME(ASN1_UTCTIME *a,unsigned char **pp);
ASN1_UTCTIME *	d2i_ASN1_UTCTIME(ASN1_UTCTIME **a,unsigned char **pp,
			long length);

int		i2d_ASN1_SET(STACK *a, unsigned char **pp,
			int (*func)(), int ex_tag, int ex_class);
STACK *		d2i_ASN1_SET(STACK **a, unsigned char **pp, long length,
			char *(*func)(), int ex_tag, int ex_class);

int i2f_ASN1_INTEGER(FILE *fp, ASN1_INTEGER *a);
int f2i_ASN1_INTEGER(FILE *fp,ASN1_INTEGER *bs,char *buf,int size);
int ASN1_INTEGER_set(ASN1_INTEGER *a, long v);
ASN1_INTEGER *BIGNUM_to_ASN1_INTEGER(BIGNUM *bn, ASN1_INTEGER *ai);
BIGNUM *ASN1_INTEGER_to_BIGNUM(ASN1_INTEGER *ai,BIGNUM *bn);

/* General */
/* given a string, return the correct type */
int ASN1_PRINTABLE_type(unsigned char *s);

void ASN1_parse(FILE *fp,unsigned char *pp,long len);

int i2d_ASN1_bytes(ASN1_BIT_STRING *a, unsigned char **pp, int tag, int class);
ASN1_BIT_STRING *d2i_ASN1_bytes(ASN1_OCTET_STRING **a, unsigned char **pp,
	long length, int Ptag, int Pclass);

/* PARSING */
int asn1_Finish(ASN1_CTX *c);

/* SPECIALS */
int ASN1_get_object(unsigned char **pp, long *plength, int *ptag,
	int *pclass, long omax);
int ASN1_check_infinite_end(unsigned char **p,long len);
void ASN1_put_object(unsigned char **pp, int constructed, int length,
	int tag, int class);
int ASN1_object_size(int constructed, int length, int tag);

/* Used to implement other functions */
char *ASN1_dup(int (*i2d)(),char *(*d2i)(),char *x);
char *ASN1_d2i_fp(char *(*new)(),char *(*d2i)(),FILE *fp,unsigned char **x);
int ASN1_i2d_fp(int (*i2d)(),FILE *out,unsigned char *x);

void ERR_load_ASN1_strings(void);

#else

ASN1_TYPE *	ASN1_TYPE_new();
void		ASN1_TYPE_free();
int		i2d_ASN1_TYPE();
ASN1_TYPE *	d2i_ASN1_TYPE();
ASN1_OBJECT *	ASN1_OBJECT_new();
void		ASN1_OBJECT_free();
int		i2d_ASN1_OBJECT();
ASN1_OBJECT *	d2i_ASN1_OBJECT();
int ASN1_BIT_STRING_cmp();
ASN1_BIT_STRING *ASN1_BIT_STRING_type_new();
void		ASN1_BIT_STRING_free();
int		i2d_ASN1_BIT_STRING();
ASN1_BIT_STRING *d2i_ASN1_BIT_STRING();
int		i2d_ASN1_INTEGER();
ASN1_BIT_STRING *d2i_ASN1_INTEGER();
int ASN1_UTCTIME_check();
void ASN1_UTCTIME_print();
ASN1_UTCTIME *ASN1_UTCTIME_dup();
int		i2d_ASN1_OCTET_STRING();
ASN1_BIT_STRING *d2i_ASN1_OCTET_STRING();
ASN1_BIT_STRING *d2i_asn1_print_type();
int		i2d_ASN1_PRINTABLE();
ASN1_BIT_STRING *d2i_ASN1_PRINTABLE();
ASN1_PRINTABLESTRING *d2i_ASN1_PRINTABLESTRING();
ASN1_T61STRING *d2i_ASN1_T61STRING();
int i2d_ASN1_IA5STRING();
ASN1_PRINTABLESTRING *d2i_ASN1_IA5STRING();
ASN1_UTCTIME *	ASN1_UTCTIME_new();
void		ASN1_UTCTIME_free();
int		i2d_ASN1_UTCTIME();
ASN1_UTCTIME *	d2i_ASN1_UTCTIME();
int		i2d_ASN1_SET();
STACK *		d2i_ASN1_SET();
int i2f_ASN1_INTEGER();
int f2i_ASN1_INTEGER();
int ASN1_INTEGER_set();
ASN1_INTEGER *BIGNUM_to_ASN1_INTEGER();
BIGNUM *ASN1_INTEGER_to_BIGNUM();
int ASN1_PRINTABLE_type();
void ASN1_parse();
int i2d_ASN1_bytes();
ASN1_BIT_STRING *d2i_ASN1_bytes();
int asn1_Finish();
int ASN1_get_object();
int ASN1_check_infinite_end();
void ASN1_put_object();
int ASN1_object_size();
char *ASN1_dup();
char *ASN1_d2i_fp();
int ASN1_i2d_fp();
void ERR_load_ASN1_strings();

#endif

/* BEGIN ERROR CODES */
/* Error codes for the ASN1 functions. */

/* Function codes. */
#define ASN1_F_ASN1_BIT_STRING_NEW			 100
#define ASN1_F_ASN1_D2I_FP				 101
#define ASN1_F_ASN1_DUP					 102
#define ASN1_F_ASN1_GET_OBJECT				 103
#define ASN1_F_ASN1_I2D_FP				 104
#define ASN1_F_ASN1_INTEGER_SET				 105
#define ASN1_F_ASN1_OBJECT_NEW				 106
#define ASN1_F_ASN1_SET					 107
#define ASN1_F_ASN1_SIGN				 108
#define ASN1_F_ASN1_TYPE_NEW				 109
#define ASN1_F_ASN1_UTCTIME_DUP				 110
#define ASN1_F_ASN1_UTCTIME_NEW				 111
#define ASN1_F_ASN1_VERIFY				 112
#define ASN1_F_BIGNUM_TO_ASN1_INTEGER			 113
#define ASN1_F_D2I_ASN1_BIT_STRING			 114
#define ASN1_F_D2I_ASN1_BYTES				 115
#define ASN1_F_D2I_ASN1_INTEGER				 116
#define ASN1_F_D2I_ASN1_OBJECT				 117
#define ASN1_F_D2I_ASN1_OCTET_STRING			 118
#define ASN1_F_D2I_ASN1_TYPE				 119
#define ASN1_F_D2I_ASN1_UTCTIME				 120
#define ASN1_F_D2I_DHPARAMS				 121
#define ASN1_F_D2I_NETSCAPE_PKEY			 122
#define ASN1_F_D2I_NETSCAPE_RSA				 123
#define ASN1_F_D2I_NETSCAPE_RSA_2			 124
#define ASN1_F_D2I_PKCS7				 125
#define ASN1_F_D2I_PKCS7_DIGEST				 126
#define ASN1_F_D2I_PKCS7_ENCRYPT			 127
#define ASN1_F_D2I_PKCS7_ENC_CONTENT			 128
#define ASN1_F_D2I_PKCS7_ENVELOPE			 129
#define ASN1_F_D2I_PKCS7_ISSUER_AND_SERIAL		 130
#define ASN1_F_D2I_PKCS7_RECIP_INFO			 131
#define ASN1_F_D2I_PKCS7_SIGNED				 132
#define ASN1_F_D2I_PKCS7_SIGNER_INFO			 133
#define ASN1_F_D2I_PKCS7_SIGN_ENVELOPE			 134
#define ASN1_F_D2I_RSAPRIVATEKEY			 135
#define ASN1_F_D2I_RSAPUBLICKEY				 136
#define ASN1_F_D2I_X509					 137
#define ASN1_F_D2I_X509_ALGOR				 138
#define ASN1_F_D2I_X509_ATTRIBUTE			 139
#define ASN1_F_D2I_X509_CINF				 140
#define ASN1_F_D2I_X509_CRL				 141
#define ASN1_F_D2I_X509_CRL_INFO			 142
#define ASN1_F_D2I_X509_KEYU				 143
#define ASN1_F_D2I_X509_NAME				 144
#define ASN1_F_D2I_X509_NAME_ENTRY			 145
#define ASN1_F_D2I_X509_PUBKEY				 146
#define ASN1_F_D2I_X509_REQ				 147
#define ASN1_F_D2I_X509_REQ_INFO			 148
#define ASN1_F_D2I_X509_REVOKED				 149
#define ASN1_F_D2I_X509_SIG				 150
#define ASN1_F_D2I_X509_VAL				 151
#define ASN1_F_F2I_ASN1_INTEGER				 152
#define ASN1_F_I2D_DHPARAMS				 153
#define ASN1_F_I2D_NETSCAPE_RSA_2			 154
#define ASN1_F_I2D_PKCS7				 155
#define ASN1_F_I2D_RSAPRIVATEKEY			 156
#define ASN1_F_I2D_RSAPUBLICKEY				 157
#define ASN1_F_I2D_X509_ATTRIBUTE			 158
#define ASN1_F_NETSCAPE_PKEY_NEW			 159
#define ASN1_F_PKCS7_DIGEST_NEW				 160
#define ASN1_F_PKCS7_ENCRYPT_NEW			 161
#define ASN1_F_PKCS7_ENC_CONTENT_NEW			 162
#define ASN1_F_PKCS7_ENVELOPE_NEW			 163
#define ASN1_F_PKCS7_ISSUER_AND_SERIAL_NEW		 164
#define ASN1_F_PKCS7_NEW				 165
#define ASN1_F_PKCS7_RECIP_INFO_NEW			 166
#define ASN1_F_PKCS7_SIGNED_NEW				 167
#define ASN1_F_PKCS7_SIGNER_INFO_NEW			 168
#define ASN1_F_PKCS7_SIGN_ENVELOPE_NEW			 169
#define ASN1_F_X509_ALGOR_NEW				 170
#define ASN1_F_X509_ATTRIBUTE_NEW			 171
#define ASN1_F_X509_CINF_NEW				 172
#define ASN1_F_X509_CRL_INFO_NEW			 173
#define ASN1_F_X509_CRL_NEW				 174
#define ASN1_F_X509_DHPARAMS_NEW			 175
#define ASN1_F_X509_INFO_NEW				 176
#define ASN1_F_X509_KEY_NEW				 177
#define ASN1_F_X509_NAME_ENTRY_NEW			 178
#define ASN1_F_X509_NAME_NEW				 179
#define ASN1_F_X509_NEW					 180
#define ASN1_F_X509_PUBKEY_NEW				 181
#define ASN1_F_X509_REQ_INFO_NEW			 182
#define ASN1_F_X509_REQ_NEW				 183
#define ASN1_F_X509_REVOKED_NEW				 184
#define ASN1_F_X509_SIG_NEW				 185
#define ASN1_F_X509_VAL_FREE				 186
#define ASN1_F_X509_VAL_NEW				 187

/* Reason codes. */
#define ASN1_R_BAD_CLASS				 100
#define ASN1_R_BAD_GET_OBJECT				 101
#define ASN1_R_BAD_OBJECT_HEADER			 102
#define ASN1_R_BAD_PASSWD_READ				 103
#define ASN1_R_BAD_PASSWORD_READ			 104
#define ASN1_R_BAD_PKCS7_CONTENT			 105
#define ASN1_R_BAD_PKCS7_TYPE				 106
#define ASN1_R_BAD_TAG					 107
#define ASN1_R_BAD_TYPE					 108
#define ASN1_R_BIT_STRING_TOO_SHORT			 109
#define ASN1_R_BN_LIB					 110
#define ASN1_R_DECODING_ERROR				 111
#define ASN1_R_ERROR_STACK				 112
#define ASN1_R_EXPECTING_AN_INTEGER			 113
#define ASN1_R_EXPECTING_AN_OBJECT			 114
#define ASN1_R_EXPECTING_AN_OCTET_STRING		 115
#define ASN1_R_EXPECTING_A_BIT_STRING			 116
#define ASN1_R_EXPECTING_A_PRINTABLE			 117
#define ASN1_R_EXPECTING_A_SEQUENCE			 118
#define ASN1_R_EXPECTING_A_UTCTIME			 119
#define ASN1_R_HEADER_TOO_LONG				 120
#define ASN1_R_LENGTH_ERROR				 121
#define ASN1_R_LENGTH_MISMATCH				 122
#define ASN1_R_MISSING_EOS				 123
#define ASN1_R_NON_HEX_CHARACTERS			 124
#define ASN1_R_NOT_ENOUGH_DATA				 125
#define ASN1_R_NOT_EOC_IN_INDEFINITLE_LENGTH_SEQUENCE	 126
#define ASN1_R_ODD_NUMBER_OF_CHARS			 127
#define ASN1_R_PARSING					 128
#define ASN1_R_PRIVATE_KEY_HEADER_MISSING		 129
#define ASN1_R_THE_ASN1_OBJECT_IDENTIFIER_IS_NOT_KNOWN_FOR_THIS_MD 130
#define ASN1_R_TOO_LONG					 131
#define ASN1_R_UNABLE_TO_DECODE_RSA_KEY			 132
#define ASN1_R_UNABLE_TO_DECODE_RSA_PRIVATE_KEY		 133
#define ASN1_R_UNKNOWN_ATTRIBUTE_TYPE			 134
#define ASN1_R_UNKNOWN_MESSAGE_DIGEST_ALGORITHM		 135
#define ASN1_R_UNKNOWN_OBJECT_TYPE			 136
#define ASN1_R_UNSUPORTED_ENCRYPTION_ALGORITHM		 137
#define ASN1_R_UTCTIME_TOO_LONG				 138
#define ASN1_R_WRONG_EXPLICIT_TAG			 139
#define ASN1_R_WRONG_TAG				 140

#endif

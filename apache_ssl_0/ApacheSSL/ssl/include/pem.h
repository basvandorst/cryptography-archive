/* lib/pem/pem.h */
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

#ifndef HEADER_PEM_H
#define HEADER_PEM_H

#ifndef HEADER_RSA_H
#include "rsa.h"
#endif

#ifndef HEADER_X509_H
#include "x509.h"
#endif

#ifndef HEADER_ENVELOPE_H
#include "envelope.h"
#endif

#define PEM_OBJ_UNDEF		0
#define PEM_OBJ_X509		1
#define PEM_OBJ_CRL		5
#define PEM_OBJ_PRIV_KEY	10
#define PEM_OBJ_PRIV_RSA	11
#define PEM_OBJ_PUB_RSA		12
#define PEM_OBJ_PRIV_DH		15
#define PEM_OBJ_PUB_DH		16

#define PEM_ERROR		30
#define PEM_DEK_DES_CBC         40
#define PEM_DEK_IDEA_CBC        45
#define PEM_DEK_DES_EDE         50
#define PEM_DEK_DES_ECB         60
#define PEM_DEK_RSA             70
#define PEM_DEK_RSA_MD2         80
#define PEM_DEK_RSA_MD5         90

#define PEM_MD_MD2		NID_md2
#define PEM_MD_MD5		NID_md5
#define PEM_MD_SHA		NID_sha
#define PEM_MD_MD2_RSA		NID_md2withRSAEncryption
#define PEM_MD_MD5_RSA		NID_md5withRSAEncryption
#define PEM_MD_SHA_RSA		NID_shawithRSAEncryption

#define PEM_STRING_X509		"X509 CERTIFICATE"
#define PEM_STRING_X509_REQ	"NEW CERTIFICATE REQUEST"
#define PEM_STRING_X509_CRL	"X509 CRL"
#define PEM_STRING_RSA		"RSA PRIVATE KEY"
#define PEM_STRING_PKCS7	"PKCS7"
#define PEM_STRING_DHPARAMS	"DH PARAMETERS"

#define PEM_SignInit(a,b)	EVP_DigestInit(a,b)
#define PEM_SignUpdate(a,b,c)	EVP_DigestUpdate(a,b,c)

#ifndef HEADER_ENVELOPE_H

#define EVP_ENCODE_CTX_SIZE 88
#define EVP_MD_SIZE 24
#define EVP_MD_CTX_SIZE 152
#define EVP_CIPHER_SIZE 28
#define EVP_CIPHER_CTX_SIZE 424
#define EVP_MAX_MD_SIZE 20

typedef struct evp_encode_ctx_st
	{
	char data[EVP_ENCODE_CTX_SIZE];
	} EVP_ENCODE_CTX;

typedef struct env_md_ctx_st
	{
	char data[EVP_MD_CTX_SIZE];
	} EVP_MD_CTX;

typedef struct evp_cipher_st
	{
	char data[EVP_CIPHER_SIZE];
	} EVP_CIPHER;

typedef struct evp_cipher_ctx_st
	{
	char data[EVP_CIPHER_CTX_SIZE];
	} EVP_CIPHER_CTX;
#endif


typedef struct PEM_Encode_Seal_st
	{
	EVP_ENCODE_CTX encode;
	EVP_MD_CTX md;
	EVP_CIPHER_CTX cipher;
	} PEM_ENCODE_SEAL_CTX;

/* enc_type is one off */
#define PEM_TYPE_ENCRYPTED      10
#define PEM_TYPE_MIC_ONLY       20
#define PEM_TYPE_MIC_CLEAR      30
#define PEM_TYPE_CLEAR		40

typedef struct pem_recip_st
	{
	char *name;
	X509_NAME *dn;

	int cipher;
	int key_enc;
	char iv[8];
	} PEM_USER;

typedef struct pem_ctx_st
	{
	int type;		/* what type of object */

	struct	{
		int version;	
		int mode;		
		} proc_type;

	char *domain;

	struct	{
		int cipher;
		unsigned char iv[8];
		} DEK_info;
		
	PEM_USER *originator;

	int num_recipient;
	PEM_USER **recipient;

#ifdef HEADER_STACK_H
	STACK *x509_chain;	/* certificate chain */
#else
	char *x509_chain;	/* certificate chain */
#endif
	EVP_MD *md;		/* signature type */

	int md_enc;		/* is the md encrypted or not? */
	int md_len;		/* length of md_data */
	char *md_data;		/* message digest, could be rsa encrypted */

	EVP_CIPHER *dec;	/* date encryption cipher */
	int key_len;		/* key length */
	unsigned char *key;	/* key */
	unsigned char iv[8];	/* the iv */

	
	int  data_enc;		/* is the data encrypted */
	int data_len;
	unsigned char *data;
	} PEM_CTX;

#ifndef NOPROTO
int	PEM_get_EVP_CIPHER_INFO(char *header, EVP_CIPHER_INFO *cipher);

int	PEM_do_header (EVP_CIPHER_INFO *cipher, unsigned char *data,long *len,
		int (*callback)());
int	PEM_read(FILE *fp, char **name, char **header,
		unsigned char **data,long *len);

int	PEM_write(FILE *fp,char *name,char *hdr,unsigned char *data,long len);

STACK *	PEM_X509_INFO_read(FILE *fp, STACK *sk, int (*cb)());
char *	PEM_ASN1_read(char *(*d2i)(),char *name,FILE *fp,char **x,int (*cb)());
int	PEM_ASN1_write(int (*i2d)(),char *name,FILE *fp,char *x,EVP_CIPHER *enc,
		unsigned char *kstr,int klen,int (*callback)());

int	PEM_SealInit(PEM_ENCODE_SEAL_CTX *ctx, EVP_CIPHER *type,
		EVP_MD *md_type, unsigned char **ek, int *ekl,
		unsigned char *iv, RSA **pubk, int npubk);
void	PEM_SealUpdate(PEM_ENCODE_SEAL_CTX *ctx, unsigned char *out, int *outl,
		unsigned char *in, int inl);
int	PEM_SealFinal(PEM_ENCODE_SEAL_CTX *ctx, unsigned char *sig,int *sigl,
		unsigned char *out, int *outl, RSA *priv);

int	PEM_SignFinal(EVP_MD_CTX *ctx, unsigned char *sigret,
		unsigned int *siglen, RSA *rsa);

void	ERR_load_PEM_strings(void);

#else

int	PEM_get_EVP_CIPHER_INFO();
int	PEM_do_header();
int	PEM_read();
int	PEM_write();
STACK *	PEM_X509_INFO_read();
char *	PEM_ASN1_read();
int	PEM_ASN1_write();
int	PEM_SealInit();
void	PEM_SealUpdate();
int	PEM_SealFinal();
int	PEM_SignFinal();

void	ERR_load_PEM_strings();

#endif


#define PEM_write_X509(fp,x) \
		PEM_ASN1_write(i2d_X509,PEM_STRING_X509,fp,(char *)x, \
			NULL,NULL,0,NULL)
#define PEM_write_X509_REQ(fp,x) PEM_ASN1_write( \
		i2d_X509_REQ,PEM_STRING_X509_REQ,fp,(char *)x, \
			NULL,NULL,0,NULL)
#define PEM_write_X509_CRL(fp,x) \
		PEM_ASN1_write(i2d_X509_CRL,PEM_STRING_X509_CRL,fp,(char *)x, \
			NULL,NULL,0,NULL)
#define	PEM_write_RSAPrivateKey(fp,x,enc,kstr,klen,cb) \
		PEM_ASN1_write(i2d_RSAPrivateKey,PEM_STRING_RSA,fp,\
			(char *)x,enc,kstr,klen,cb)
#define PEM_write_PKCS7(fp,x) \
		PEM_ASN1_write(i2d_PKCS7,PEM_STRING_PKCS7,fp,(char *)x, \
			NULL,NULL,0,NULL)
#define PEM_write_DHparams(fp,x) \
		PEM_ASN1_write(i2d_DHparams,PEM_STRING_DHPARAMS,fp,(char *)x, \
			NULL,NULL,0,NULL)

#define	PEM_read_X509(fp,x,cb) (X509 *)PEM_ASN1_read( \
	(char *(*)())d2i_X509,PEM_STRING_X509,fp,(char **)x,cb)
#define	PEM_read_X509_REQ(fp,x,cb) (X509_REQ *)PEM_ASN1_read( \
	(char *(*)())d2i_X509_REQ,PEM_STRING_X509_REQ,fp,(char **)x,cb)
#define	PEM_read_X509_CRL(fp,x,cb) (X509_CRL *)PEM_ASN1_read( \
	(char *(*)())d2i_X509_CRL,PEM_STRING_X509_CRL,fp,(char **)x,cb)
#define	PEM_read_RSAPrivateKey(fp,x,cb) (RSA *)PEM_ASN1_read( \
	(char *(*)())d2i_RSAPrivateKey,PEM_STRING_RSA,fp,(char **)x,cb)
#define	PEM_read_PKCS7(fp,x,cb) (PKCS7 *)PEM_ASN1_read( \
	(char *(*)())d2i_PKCS7,PEM_STRING_PKCS7,fp,(char **)x,cb)
#define	PEM_read_DHparams(fp,x,cb) (DH *)PEM_ASN1_read( \
	(char *(*)())d2i_DHparams,PEM_STRING_DHPARAMS,fp,(char **)x,cb)

/* BEGIN ERROR CODES */
/* Error codes for the PEM functions. */

/* Function codes. */
#define PEM_F_DEF_CALLBACK				 100
#define PEM_F_LOAD_IV					 101
#define PEM_F_PEM_ASN1_READ				 102
#define PEM_F_PEM_DO_HEADER				 103
#define PEM_F_PEM_EVP_CIPHER_INFO			 104
#define PEM_F_PEM_READ					 105
#define PEM_F_PEM_SEALFINAL				 106
#define PEM_F_PEM_SEALINIT				 107
#define PEM_F_PEM_SIGNFINAL				 108
#define PEM_F_PEM_WRITE					 109
#define PEM_F_PEM_WRITE_RSA				 110
#define PEM_F_PEM_X509_INFO_READ			 111

/* Reason codes. */
#define PEM_R_BAD_BASE64_DECODE				 100
#define PEM_R_BAD_DECRYPT				 101
#define PEM_R_BAD_END_LINE				 102
#define PEM_R_BAD_IV_CHARS				 103
#define PEM_R_BAD_PASSWORD_READ				 104
#define PEM_R_NOT_DEK_INFO				 105
#define PEM_R_NOT_ENCRYPTED				 106
#define PEM_R_NOT_PROC_TYPE				 107
#define PEM_R_NO_START_LINE				 108
#define PEM_R_READ_KEY					 109
#define PEM_R_SHORT_HEADER				 110
#define PEM_R_UNSUPPORTED_CIPHER			 111
#define PEM_R_UNSUPPORTED_ENCRYPTION			 112

#endif

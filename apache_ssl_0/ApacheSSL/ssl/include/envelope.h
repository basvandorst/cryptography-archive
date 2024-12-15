/* lib/evp/envelope.h */
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

#ifndef HEADER_ENVELOPE_H
#define HEADER_ENVELOPE_H

#include "md2.h"
#include "md5.h"
#include "sha.h"
#include "des.h"
#ifndef NORC4
#include "rc4.h"
#endif
#ifndef NOIDEA
#include "idea.h"
#endif

#define EVP_RC4_KEY_SIZE	16
#define EVP_MAX_MD_SIZE		20
#define EVP_MAX_KEY_LENGTH	24
#define EVP_MAX_IV_LENGTH	8

#ifndef HEADER_RSA_H
#include "rsa.h"
#endif

#ifndef EVP_MD
typedef struct env_md_st
	{
	int type;
	int rsa_type;
	int md_size;
	void (*init)();
	void (*update)();
	void (*final)();
	} EVP_MD;
#endif

typedef struct env_md_ctx_st
	{
	EVP_MD *digest;
	union	{
		unsigned char base[4];
		MD2_CTX md2;
		MD5_CTX md5;
		SHA_CTX sha;
		} md;
	} EVP_MD_CTX;

typedef struct evp_cipher_st
	{
	int type;
	int block_size;
	int key_len;
	int iv_len;
	void (*enc_init)();	/* init for encryption */
	void (*dec_init)();	/* init for decryption */
	void (*do_cipher)();	/* encrypt data */
	} EVP_CIPHER;

typedef struct evp_cipher_info_st
	{
	EVP_CIPHER *cipher;
	unsigned char iv[8];
	} EVP_CIPHER_INFO;

typedef struct evp_cipher_ctx_st
	{
	EVP_CIPHER *cipher;
	int encrypt;		/* encrypt or decrypt */
	int buf_len;		/* number we have left */
	unsigned char buf[8];
	union	{
#ifndef NORC4
		struct
			{
			unsigned char key[EVP_RC4_KEY_SIZE];
			RC4_KEY ks;	/* working key */
			} rc4;
#endif
		struct
			{
			des_key_schedule ks;/* key schedule */
			} des_ecb;

		struct
			{
			C_Block oiv;	/* original iv */
			C_Block iv;	/* working iv */
			des_key_schedule ks;/* key schedule */
			} des_cbc;

		struct
			{
			C_Block oiv;	/* original iv */
			C_Block iv;	/* working iv */
			des_key_schedule ks;/* key schedule */
			des_key_schedule ks2;/* key schedule (for ede) */
			des_key_schedule ks3;/* key schedule (for ede3) */
			int num;	/* used by cfb mode */
			} des_cfb;

		struct
			{
			C_Block oiv;	/* original iv */
			C_Block iv;	/* working iv */
			des_key_schedule ks1;/* ksched 1 */
			des_key_schedule ks2;/* ksched 2 */
			des_key_schedule ks3;/* ksched 3 */
			} des_ede;
#ifndef NOIDEA
		struct
			{
			IDEA_KEY_SCHEDULE ks;/* key schedule */
			} idea_ecb;
		struct
			{
			unsigned char oiv[8];/* original iv */
			unsigned char iv[8]; /* working iv */
			IDEA_KEY_SCHEDULE ks;/* key schedule */
			} idea_cbc;
		struct
			{
			unsigned char oiv[8];/* original iv */
			unsigned char iv[8]; /* working iv */
			IDEA_KEY_SCHEDULE ks;/* key schedule */
			int num;	/* used by cfb mode */
			} idea_cfb;
#endif
		} c;
	} EVP_CIPHER_CTX;

typedef struct evp_Encode_Ctx_st
	{
	int num;        /* number saved in a partial encode/decode */
	int length;
	unsigned char enc_data[80];     /* data to encode */
	} EVP_ENCODE_CTX;

extern EVP_MD *EVP_md2;
extern EVP_MD *EVP_md5;
extern EVP_MD *EVP_sha;
extern EVP_MD *EVP_sha1;

extern EVP_CIPHER *EVP_des_ecb;
extern EVP_CIPHER *EVP_des_ede;
extern EVP_CIPHER *EVP_des_ede3;
extern EVP_CIPHER *EVP_des_cfb;
extern EVP_CIPHER *EVP_des_ede_cfb;
extern EVP_CIPHER *EVP_des_ede3_cfb;
extern EVP_CIPHER *EVP_des_ofb;
extern EVP_CIPHER *EVP_des_ede_ofb;
extern EVP_CIPHER *EVP_des_ede3_ofb;
extern EVP_CIPHER *EVP_des_cbc;
extern EVP_CIPHER *EVP_des_ede_cbc;
extern EVP_CIPHER *EVP_des_ede3_cbc;
extern EVP_CIPHER *EVP_rc4;
extern EVP_CIPHER *EVP_idea_ecb;
extern EVP_CIPHER *EVP_idea_cfb;
extern EVP_CIPHER *EVP_idea_ofb;
extern EVP_CIPHER *EVP_idea_cbc;

#define EVP_ENCODE_LENGTH(l)    (((l+2)/3*4)+(l/48+1)*2)
#define EVP_DECODE_LENGTH(l)    ((l+3)/4*3)

#define EVP_SignInit(a,b)		EVP_DigestInit(a,b)
#define EVP_SignUpdate(a,b,c)		EVP_DigestUpdate(a,b,c)
#define	EVP_VerifyInit(a,b)		EVP_DigestInit(a,b)
#define	EVP_VerifyUpdate(a,b,c)		EVP_DigestUpdate(a,b,c)
#define EVP_OpenUpdate(a,b,c,d,e)	EVP_DecryptUpdate(a,b,c,d,e)
#define EVP_SealUpdate(a,b,c,d,e)	EVP_EncryptUpdate(a,b,c,d,e)	

#ifndef NOPROTO

EVP_MD *EVP_get_MDbyname(char *name);

void	EVP_DigestInit(EVP_MD_CTX *ctx, EVP_MD *type);
void	EVP_DigestUpdate(EVP_MD_CTX *ctx,unsigned char *d,unsigned int cnt);
void	EVP_DigestFinal(EVP_MD_CTX *ctx,unsigned char *md,unsigned int *s);

int	EVP_read_pw_string(char *buf,int length,char *prompt,int verify);
int	EVP_BytesToKey(EVP_CIPHER *type,EVP_MD *md,unsigned char *salt,
		unsigned char *data, int datal, int count,
		unsigned char *key,unsigned char *iv);

EVP_CIPHER *EVP_get_cipherbyname(char *name);

void	EVP_EncryptInit(EVP_CIPHER_CTX *ctx,EVP_CIPHER *type,
		unsigned char *key, unsigned char *iv);
void	EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out,
		int *outl, unsigned char *in, int inl);
void	EVP_EncryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);

void	EVP_DecryptInit(EVP_CIPHER_CTX *ctx,EVP_CIPHER *type,
		unsigned char *key, unsigned char *iv);
void	EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out,
		int *outl, unsigned char *in, int inl);
int	EVP_DecryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl);

void	EVP_CipherInit(EVP_CIPHER_CTX *ctx,EVP_CIPHER *type, unsigned char *key,
		unsigned char *iv,int enc);
void	EVP_CipherUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out,
		int *outl, unsigned char *in, int inl);
int	EVP_CipherFinal(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl);

int     EVP_SignFinal(EVP_MD_CTX *ctx,unsigned char *md,unsigned int *s,
		RSA *rsa);

int	EVP_VerifyFinal(EVP_MD_CTX *ctx,unsigned char *sigbuf,unsigned int siglen,
		RSA *rsa);

int	EVP_OpenInit(EVP_CIPHER_CTX *ctx,EVP_CIPHER *type,unsigned char *ek,
		int ekl,unsigned char *iv,RSA *priv);
int	EVP_OpenFinal(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);



int	EVP_SealInit(EVP_CIPHER_CTX *ctx, EVP_CIPHER *type, unsigned char **ek,
		int *ekl, unsigned char *iv, RSA **pubk, int npubk);
void	EVP_SealFinal(EVP_CIPHER_CTX *ctx,unsigned char *out,int *outl);

void    EVP_EncodeInit(EVP_ENCODE_CTX *ctx);
void    EVP_EncodeUpdate(EVP_ENCODE_CTX *ctx,unsigned char *out,
		int *outl,unsigned char *in,int inl);
void    EVP_EncodeFinal(EVP_ENCODE_CTX *ctx,unsigned char *out,int *outl);
int     EVP_EncodeBlock(unsigned char *t, unsigned char *f, int n);

void    EVP_DecodeInit(EVP_ENCODE_CTX *ctx);
int     EVP_DecodeUpdate(EVP_ENCODE_CTX *ctx,unsigned char *out,int *outl,
		unsigned char *in, int inl);
int     EVP_DecodeFinal(EVP_ENCODE_CTX *ctx, unsigned
		char *out, int *outl);
int     EVP_DecodeBlock(unsigned char *t, unsigned
		char *f, int n);

void	ERR_load_EVP_strings(void );

#else

EVP_MD *EVP_get_MDbyname();
void	EVP_DigestInit();
void	EVP_DigestUpdate();
void	EVP_DigestFinal();
int	EVP_read_pw_string();
int	EVP_BytesToKey();
EVP_CIPHER *EVP_get_cipherbyname();
void	EVP_EncryptInit();
void	EVP_EncryptUpdate();
void	EVP_EncryptFinal();
void	EVP_DecryptInit();
void	EVP_DecryptUpdate();
int	EVP_DecryptFinal();
void	EVP_CipherInit();
void	EVP_CipherUpdate();
int	EVP_CipherFinal();
int     EVP_SignFinal();
int	EVP_VerifyFinal();
int	EVP_OpenInit();
int	EVP_OpenFinal();
int	EVP_SealInit();
void	EVP_SealFinal();
void    EVP_EncodeInit();
void    EVP_EncodeUpdate();
void    EVP_EncodeFinal();
int     EVP_EncodeBlock();
void    EVP_DecodeInit();
int     EVP_DecodeUpdate();
int     EVP_DecodeFinal();
int     EVP_DecodeBlock();

void	ERR_load_EVP_strings();

#endif

/* BEGIN ERROR CODES */
/* Error codes for the EVP functions. */

/* Function codes. */
#define EVP_F_EVP_DECRYPTFINAL				 100
#define EVP_F_EVP_OPENINIT				 101
#define EVP_F_EVP_SIGNFINAL				 102
#define EVP_F_EVP_VERIFYFINAL				 103

/* Reason codes. */
#define EVP_R_ALGORITHM_MISMATCH			 100
#define EVP_R_BAD_DECRYPT				 101
#define EVP_R_BAD_SIGNATURE				 102
#define EVP_R_DIGEST_TOO_BIG_FOR_RSA_KEY		 103
#define EVP_R_THE_ASN1_OBJECT_IDENTIFIER_IS_NOT_KNOWN_FOR_THIS_MD 104
#define EVP_R_UNKNOWN_ALGORITHM_TYPE			 105
#define EVP_R_WRONG_FINAL_BLOCK_LENGTH			 106
#define EVP_R_WRONG_SIGNATURE_LENGTH			 107

#endif

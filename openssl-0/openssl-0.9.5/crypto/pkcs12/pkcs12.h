/* pkcs12.h */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 1999.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#ifndef HEADER_PKCS12_H
#define HEADER_PKCS12_H

#ifdef __cplusplus
extern "C" {
#endif

#include <openssl/bio.h>
#include <openssl/x509.h>

#define PKCS12_KEY_ID	1
#define PKCS12_IV_ID	2
#define PKCS12_MAC_ID	3

/* Default iteration count */
#ifndef PKCS12_DEFAULT_ITER
#define PKCS12_DEFAULT_ITER	PKCS5_DEFAULT_ITER
#endif

#define PKCS12_MAC_KEY_LENGTH 20

#define PKCS12_SALT_LEN	8

/* Uncomment out next line for unicode password and names, otherwise ASCII */

/*#define PBE_UNICODE*/

#ifdef PBE_UNICODE
#define PKCS12_key_gen PKCS12_key_gen_uni
#define PKCS12_add_friendlyname PKCS12_add_friendlyname_uni
#else
#define PKCS12_key_gen PKCS12_key_gen_asc
#define PKCS12_add_friendlyname PKCS12_add_friendlyname_asc
#endif

/* MS key usage constants */

#define KEY_EX	0x10
#define KEY_SIG 0x80

typedef struct {
X509_SIG *dinfo;
ASN1_OCTET_STRING *salt;
ASN1_INTEGER *iter;	/* defaults to 1 */
} PKCS12_MAC_DATA;

typedef struct {
ASN1_INTEGER *version;
PKCS12_MAC_DATA *mac;
PKCS7 *authsafes;
} PKCS12;

typedef struct {
ASN1_OBJECT *type;
union {
	struct pkcs12_bag_st *bag; /* secret, crl and certbag */
	struct pkcs8_priv_key_info_st	*keybag; /* keybag */
	X509_SIG *shkeybag; /* shrouded key bag */
	STACK /* PKCS12_SAFEBAG */ *safes;
	ASN1_TYPE *other;
}value;
STACK_OF(X509_ATTRIBUTE) *attrib;
ASN1_TYPE *rest;
} PKCS12_SAFEBAG;

typedef struct pkcs12_bag_st {
ASN1_OBJECT *type;
union {
	ASN1_OCTET_STRING *x509cert;
	ASN1_OCTET_STRING *x509crl;
	ASN1_OCTET_STRING *octet;
	ASN1_IA5STRING *sdsicert;
	ASN1_TYPE *other; /* Secret or other bag */
}value;
} PKCS12_BAGS;

#define PKCS12_ERROR	0
#define PKCS12_OK	1

#define M_PKCS12_bag_type(bag) OBJ_obj2nid(bag->type)
#define M_PKCS12_cert_bag_type(bag) OBJ_obj2nid(bag->value.bag->type)
#define M_PKCS12_crl_bag_type M_PKCS12_cert_bag_type

#define M_PKCS12_x5092certbag(x509) \
PKCS12_pack_safebag ((char *)(x509), i2d_X509, NID_x509Certificate, NID_certBag)

#define M_PKCS12_x509crl2certbag(crl) \
PKCS12_pack_safebag ((char *)(crl), i2d_X509CRL, NID_x509Crl, NID_crlBag)

#define M_PKCS12_certbag2x509(bg) \
(X509 *) ASN1_unpack_string ((bg)->value.bag->value.octet, \
(char *(*)())d2i_X509)

#define M_PKCS12_certbag2x509crl(bg) \
(X509CRL *) ASN1_unpack_string ((bg)->value.bag->value.octet, \
(char *(*)())d2i_X509CRL)

/*#define M_PKCS12_pkcs82rsa(p8) \
(RSA *) ASN1_unpack_string ((p8)->pkey, (char *(*)())d2i_RSAPrivateKey)*/

#define M_PKCS12_unpack_p7data(p7) \
ASN1_seq_unpack ((p7)->d.data->data, p7->d.data->length, \
			 (char *(*)())d2i_PKCS12_SAFEBAG, PKCS12_SAFEBAG_free)

#define M_PKCS12_pack_authsafes(p12, safes) \
ASN1_seq_pack((safes), (int (*)())i2d_PKCS7,\
	&(p12)->authsafes->d.data->data, &(p12)->authsafes->d.data->length)

#define M_PKCS12_unpack_authsafes(p12) \
ASN1_seq_unpack((p12)->authsafes->d.data->data, \
		(p12)->authsafes->d.data->length, (char *(*)())d2i_PKCS7, \
							PKCS7_free)

#define M_PKCS12_unpack_p7encdata(p7, pass, passlen) \
(STACK *) PKCS12_decrypt_d2i ((p7)->d.encrypted->enc_data->algorithm,\
			 (char *(*)())d2i_PKCS12_SAFEBAG, PKCS12_SAFEBAG_free, \
							(pass), (passlen), \
			(p7)->d.encrypted->enc_data->enc_data, 3)

#define M_PKCS12_decrypt_skey(bag, pass, passlen) \
(PKCS8_PRIV_KEY_INFO *) PKCS12_decrypt_d2i ((bag)->value.shkeybag->algor, \
(char *(*)())d2i_PKCS8_PRIV_KEY_INFO, PKCS8_PRIV_KEY_INFO_free, \
						(pass), (passlen), \
			 (bag)->value.shkeybag->digest, 2)

#define M_PKCS8_decrypt(p8, pass, passlen) \
(PKCS8_PRIV_KEY_INFO *) PKCS12_decrypt_d2i ((p8)->algor, \
(char *(*)())d2i_PKCS8_PRIV_KEY_INFO, PKCS8_PRIV_KEY_INFO_free,\
			 (pass), (passlen), (p8)->digest, 2)

#define PKCS12_get_attr(bag, attr_nid) \
			 PKCS12_get_attr_gen(bag->attrib, attr_nid)

#define PKCS8_get_attr(p8, attr_nid) \
		PKCS12_get_attr_gen(p8->attributes, attr_nid)

#define PKCS12_mac_present(p12) ((p12)->mac ? 1 : 0)


PKCS12_SAFEBAG *PKCS12_pack_safebag(char *obj, int (*i2d)(), int nid1, int nid2);
PKCS12_SAFEBAG *PKCS12_MAKE_KEYBAG(PKCS8_PRIV_KEY_INFO *p8);
X509_SIG *PKCS8_encrypt(int pbe_nid, const EVP_CIPHER *cipher, 
			const char *pass, int passlen,
			unsigned char *salt, int saltlen, int iter,
			PKCS8_PRIV_KEY_INFO *p8);
PKCS12_SAFEBAG *PKCS12_MAKE_SHKEYBAG(int pbe_nid, const char *pass,
				     int passlen, unsigned char *salt,
				     int saltlen, int iter,
				     PKCS8_PRIV_KEY_INFO *p8);
PKCS7 *PKCS12_pack_p7data(STACK *sk);
PKCS7 *PKCS12_pack_p7encdata(int pbe_nid, const char *pass, int passlen,
			     unsigned char *salt, int saltlen, int iter,
			     STACK *bags);
int PKCS12_add_localkeyid(PKCS12_SAFEBAG *bag, unsigned char *name, int namelen);
int PKCS12_add_friendlyname_asc(PKCS12_SAFEBAG *bag, const char *name,
				int namelen);
int PKCS12_add_friendlyname_uni(PKCS12_SAFEBAG *bag, const unsigned char *name,
				int namelen);
int PKCS8_add_keyusage(PKCS8_PRIV_KEY_INFO *p8, int usage);
ASN1_TYPE *PKCS12_get_attr_gen(STACK_OF(X509_ATTRIBUTE) *attrs, int attr_nid);
char *PKCS12_get_friendlyname(PKCS12_SAFEBAG *bag);
unsigned char *PKCS12_pbe_crypt(X509_ALGOR *algor, const char *pass,
				int passlen, unsigned char *in, int inlen,
				unsigned char **data, int *datalen, int en_de);
char *PKCS12_decrypt_d2i(X509_ALGOR *algor, char *(*d2i)(),
			 void (*free_func)(), const char *pass, int passlen,
			 ASN1_STRING *oct, int seq);
ASN1_STRING *PKCS12_i2d_encrypt(X509_ALGOR *algor, int (*i2d)(),
				const char *pass, int passlen, char *obj,
				int seq);
PKCS12 *PKCS12_init(int mode);
int PKCS12_key_gen_asc(const char *pass, int passlen, unsigned char *salt,
		       int saltlen, int id, int iter, int n,
		       unsigned char *out, const EVP_MD *md_type);
int PKCS12_key_gen_uni(unsigned char *pass, int passlen, unsigned char *salt, int saltlen, int id, int iter, int n, unsigned char *out, const EVP_MD *md_type);
int PKCS12_PBE_keyivgen(EVP_CIPHER_CTX *ctx, const char *pass, int passlen,
			 ASN1_TYPE *param, EVP_CIPHER *cipher, EVP_MD *md_type,
			 int en_de);
int PKCS12_gen_mac(PKCS12 *p12, const char *pass, int passlen,
			 unsigned char *mac, unsigned int *maclen);
int PKCS12_verify_mac(PKCS12 *p12, const char *pass, int passlen);
int PKCS12_set_mac(PKCS12 *p12, const char *pass, int passlen,
		   unsigned char *salt, int saltlen, int iter,
		   EVP_MD *md_type);
int PKCS12_setup_mac(PKCS12 *p12, int iter, unsigned char *salt,
					 int saltlen, EVP_MD *md_type);
unsigned char *asc2uni(const char *asc, unsigned char **uni, int *unilen);
char *uni2asc(unsigned char *uni, int unilen);
int i2d_PKCS12_BAGS(PKCS12_BAGS *a, unsigned char **pp);
PKCS12_BAGS *PKCS12_BAGS_new(void);
PKCS12_BAGS *d2i_PKCS12_BAGS(PKCS12_BAGS **a, unsigned char **pp, long length);
void PKCS12_BAGS_free(PKCS12_BAGS *a);
int i2d_PKCS12(PKCS12 *a, unsigned char **pp);
PKCS12 *d2i_PKCS12(PKCS12 **a, unsigned char **pp, long length);
PKCS12 *PKCS12_new(void);
void PKCS12_free(PKCS12 *a);
int i2d_PKCS12_MAC_DATA(PKCS12_MAC_DATA *a, unsigned char **pp);
PKCS12_MAC_DATA *PKCS12_MAC_DATA_new(void);
PKCS12_MAC_DATA *d2i_PKCS12_MAC_DATA(PKCS12_MAC_DATA **a, unsigned char **pp,
								 long length);
void PKCS12_MAC_DATA_free(PKCS12_MAC_DATA *a);
int i2d_PKCS12_SAFEBAG(PKCS12_SAFEBAG *a, unsigned char **pp);
PKCS12_SAFEBAG *PKCS12_SAFEBAG_new(void);
PKCS12_SAFEBAG *d2i_PKCS12_SAFEBAG(PKCS12_SAFEBAG **a, unsigned char **pp,
								 long length);
void PKCS12_SAFEBAG_free(PKCS12_SAFEBAG *a);
void ERR_load_PKCS12_strings(void);
void PKCS12_PBE_add(void);
int PKCS12_parse(PKCS12 *p12, const char *pass, EVP_PKEY **pkey, X509 **cert,
		 STACK **ca);
PKCS12 *PKCS12_create(char *pass, char *name, EVP_PKEY *pkey, X509 *cert,
			 STACK *ca, int nid_key, int nid_cert, int iter,
						 int mac_iter, int keytype);
int i2d_PKCS12_bio(BIO *bp, PKCS12 *p12);
int i2d_PKCS12_fp(FILE *fp, PKCS12 *p12);
PKCS12 *d2i_PKCS12_bio(BIO *bp, PKCS12 **p12);
PKCS12 *d2i_PKCS12_fp(FILE *fp, PKCS12 **p12);
int PKCS12_newpass(PKCS12 *p12, char *oldpass, char *newpass);

/* BEGIN ERROR CODES */
/* The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

/* Error codes for the PKCS12 functions. */

/* Function codes. */
#define PKCS12_F_PARSE_BAGS				 103
#define PKCS12_F_PKCS12_ADD_FRIENDLYNAME		 100
#define PKCS12_F_PKCS12_ADD_FRIENDLYNAME_ASC		 127
#define PKCS12_F_PKCS12_ADD_FRIENDLYNAME_UNI		 102
#define PKCS12_F_PKCS12_ADD_LOCALKEYID			 104
#define PKCS12_F_PKCS12_CREATE				 105
#define PKCS12_F_PKCS12_DECRYPT_D2I			 106
#define PKCS12_F_PKCS12_GEN_MAC				 107
#define PKCS12_F_PKCS12_I2D_ENCRYPT			 108
#define PKCS12_F_PKCS12_INIT				 109
#define PKCS12_F_PKCS12_KEY_GEN_ASC			 110
#define PKCS12_F_PKCS12_KEY_GEN_UNI			 111
#define PKCS12_F_PKCS12_MAKE_KEYBAG			 112
#define PKCS12_F_PKCS12_MAKE_SHKEYBAG			 113
#define PKCS12_F_PKCS12_NEWPASS				 128
#define PKCS12_F_PKCS12_PACK_P7DATA			 114
#define PKCS12_F_PKCS12_PACK_P7ENCDATA			 115
#define PKCS12_F_PKCS12_PACK_SAFEBAG			 117
#define PKCS12_F_PKCS12_PARSE				 118
#define PKCS12_F_PKCS12_PBE_CRYPT			 119
#define PKCS12_F_PKCS12_PBE_KEYIVGEN			 120
#define PKCS12_F_PKCS12_SETUP_MAC			 122
#define PKCS12_F_PKCS12_SET_MAC				 123
#define PKCS12_F_PKCS8_ADD_KEYUSAGE			 124
#define PKCS12_F_PKCS8_ENCRYPT				 125
#define PKCS12_F_VERIFY_MAC				 126

/* Reason codes. */
#define PKCS12_R_CANT_PACK_STRUCTURE			 100
#define PKCS12_R_DECODE_ERROR				 101
#define PKCS12_R_ENCODE_ERROR				 102
#define PKCS12_R_ENCRYPT_ERROR				 103
#define PKCS12_R_ERROR_SETTING_ENCRYPTED_DATA_TYPE	 120
#define PKCS12_R_INVALID_NULL_ARGUMENT			 104
#define PKCS12_R_INVALID_NULL_PKCS12_POINTER		 105
#define PKCS12_R_IV_GEN_ERROR				 106
#define PKCS12_R_KEY_GEN_ERROR				 107
#define PKCS12_R_MAC_ABSENT				 108
#define PKCS12_R_MAC_GENERATION_ERROR			 109
#define PKCS12_R_MAC_SETUP_ERROR			 110
#define PKCS12_R_MAC_STRING_SET_ERROR			 111
#define PKCS12_R_MAC_VERIFY_ERROR			 112
#define PKCS12_R_MAC_VERIFY_FAILURE			 113
#define PKCS12_R_PARSE_ERROR				 114
#define PKCS12_R_PKCS12_ALGOR_CIPHERINIT_ERROR		 115
#define PKCS12_R_PKCS12_CIPHERFINAL_ERROR		 116
#define PKCS12_R_PKCS12_PBE_CRYPT_ERROR			 117
#define PKCS12_R_UNKNOWN_DIGEST_ALGORITHM		 118
#define PKCS12_R_UNSUPPORTED_PKCS12_MODE		 119

#ifdef  __cplusplus
}
#endif
#endif


/* Copyright Dr Stephen Henson 1997. All rights reserved.
 * The conditions of use are identical to those of SSLeay except that my
 * name (in addition to Eric Young or Tim Hudson) must be mentioned
 * if this software is used in a product and the notice "This product
 * includes software written by Dr Stephen Henson (shenson@bigfoot.com)"
 * used.
 */
/* Encrypted private key is same structure as X509_SIG */
#define ENCPKEY X509_SIG
#define ENCPKEY_free X509_SIG_free
#define ENCPKEY_new X509_SIG_new
#define epkey digest
#define d2i_ENCPKEY d2i_X509_SIG
#define i2d_ENCPKEY i2d_X509_SIG

#ifndef V_ASN1_BMPSTRING
#define V_ASN1_BMPSTRING	30
#endif

typedef struct {
X509_SIG *dinfo;
ASN1_BIT_STRING *salt;
} MAC_DATA;

typedef struct {
MAC_DATA *mac;
PKCS7 *authsafe;
STACK /* SAFEBAG */ *bags; /* temp for unencrypted SafeContents */
struct asafe_st *safe_cont; /* Contents of authsafe */
} PFX;

typedef struct {
STACK /* ESPVK */ *espvks;
STACK /* SAFEBAG */ *safebags;
} BAGITEM;

typedef struct {
STACK *prints;
int *regen;  
ASN1_STRING *nickname;
STACK *additional; /* Ignored */
} SUP_DATA;

typedef struct {
ASN1_INTEGER *version;
X509_ALGOR *pkeyalg;
ASN1_OCTET_STRING *pkey;
STACK *attributes;
} PRIV_KEY_INFO;

typedef struct {
ASN1_OBJECT *espvkoid;
SUP_DATA *supdata;
ENCPKEY *cipher;
PRIV_KEY_INFO *pkey; /* temp for unencrypted private keys */
} ESPVK;

typedef struct asafe_st {
ASN1_INTEGER *version;
ASN1_OBJECT *mode;
ASN1_BIT_STRING *psalt;
STACK /* BAGITEM */ *bagitms;
PKCS7 *safe;
} AUTHSAFE;

typedef struct {
ASN1_OCTET_STRING *salt;
ASN1_INTEGER *iter;
} PBEPARAM;

typedef struct {
ASN1_OBJECT *type;
STACK /* CERTCRL */ *certs;
ASN1_TYPE *other;
} SAFEBAG;

typedef union {
PKCS7 *cchain;
ASN1_IA5STRING *value;
} CERTS;

typedef struct {
ASN1_OBJECT *bagid;
CERTS *lcerts;
} CERTCRL;

/* New OIDS */
#ifndef ADD_OBJS
#define __NIDS__ extern
#else
#define __NIDS__ /**/
#endif

__NIDS__ int NID_off_line;
__NIDS__ int NID_on_line;
__NIDS__ int NID_pbe_sha1_rc4_128;
__NIDS__ int NID_pbe_sha1_rc4_40;
__NIDS__ int NID_pbe_sha1_tripdes;
__NIDS__ int NID_pbe_sha1_rc2_128;
__NIDS__ int NID_pbe_sha1_rc2_40;
__NIDS__ int NID_pbe_sha1_des;
__NIDS__ int NID_pkcs8_key_shrouding;
__NIDS__ int NID_CertAndCRLBagID;
__NIDS__ int NID_x509CertCRLBag;

#include "pfxproto.h"

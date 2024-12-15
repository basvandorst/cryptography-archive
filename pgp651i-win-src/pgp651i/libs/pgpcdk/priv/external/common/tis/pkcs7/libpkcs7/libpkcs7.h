/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael Elkins <michael@tis.com>
   Last Edit: November 18, 1998 */

#include "pkcs7_asn.h"

enum {
    PKCS7_CONTENT_DATA,
    PKCS7_CONTENT_SIGNED_DATA,
    PKCS7_CONTENT_ENVELOPED_DATA,
    PKCS7_CONTENT_ENCRYPTED_DATA
};

typedef int HashCallback (PKIOCTET_STRING *hashValue /* OUT */,
	const char *hashAlgorithm /* IN */,
	const unsigned char *tbs /* IN */,
	size_t tbsLen /* IN */,
	void *data /* IN */,
	PKICONTEXT *memFunc);

typedef int SignCallback (PKIOCTET_STRING *sigValue /* OUT */,
	const char *hashAlgorithm, /* IN */
	const char *signatureAlgorithm /* IN */,
	PKICertificate *signerPKICertificate /* IN */,
	unsigned char *tbs /* IN */,
	size_t tbsLen /* IN */,
	void *data /* IN */,
	PKICONTEXT *memFunc);

/* the encryption callback function is responsible for setting those fields
   listed as OUT in the following structure */
typedef struct
{
    PKICertificate *certificate;     /* [IN] entity to encrypt to */
    void *data;                   /* [IN] user supplied data (option) */
    const char *algorithm;        /* [OUT] algorithm used to wrap key */
    unsigned char *encryptedKey;  /* [OUT] wrapped key */
    size_t encryptedKeyLen;       /* [OUT] length of wrapped key */
} EncryptRecipient;

typedef struct sm_list_t
{
    void                *data;
    struct sm_list_t	*next;
} List;

typedef int EncryptCallback (
	PKIOCTET_STRING        *encryptedData, /* [OUT] encrypted data */
	PKIANY     *encryptParam,  /* [OUT] data encryption parameters (e.g., initialization vector) */
	const char              *dataEncAlg,    /* [IN] data encryption alg */
	const unsigned char     *tbe,           /* [IN] data to encrypt */
	size_t                  tbelen,         /* [IN] size of data */
	List                    *recips,        /* [IN/OUT] who to encrypt to.
						   The callback also returns
						   the encrypted session key
						   for each recipient in this variable. */
	void                    *data,         /* [IN] user supplied data
						   (optional) */
	PKICONTEXT *memFunc);

typedef int DecryptCallback (
	unsigned char		**msg,	    	/* [OUT] decrypted data */
	size_t			*msglen,    	/* [OUT] decrypted data len */
	const char		*contentEncAlg, /* [IN] data encrypted alg */
	PKIANY *param,				/* [IN] data encryption param
						   (e.g., initialization
						   vector) */
	PKIEncryptedContent	*content,   	/* [IN] encrypted data */
	const char		*keyEncAlg, 	/* [IN] key encryption alg */
	PKIEncryptedKey		*enckey,    	/* [IN] encrypted key */
	PKICertificate	    	*cert,		/* [IN] key to decrypt with */
	void		    	*data,		/* [IN] callback data (optional) */
	PKICONTEXT *memFunc);

typedef int VerifyCallback (
	const unsigned char	*tbs,			/* signed data */
	size_t			tbslen,			/* signed data len */
	const char		*digestAlg,		/* hash alg */
	const char		*digestEncryptionAlg,	/* alg to encrypt sig */
	PKIEncryptedDigest	*signature,		/* encrypted sig */
	PKICertificate		*cert,			/* signer cert */
	void			*data,			/* [IN] callback data
							   (optional */
	PKICONTEXT *memFunc);

int sm_EncryptMessage (
    unsigned char	**msg,			/* [OUT] output message */
    size_t		*msgLen,		/* [OUT] length of `msg' */
    const unsigned char *tbe,			/* [IN] data to be encrypted */
    size_t		tbeLen,			/* [IN] length of `tbe' */
    int			nested,			/* [in] nested op */
    const char		*contentType,		/* [IN] type of `tbe' */
    const char		*encAlg,		/* [IN] encryption alg to use */
    List		*recipCerts,		/* [IN] recipient certs */
    EncryptCallback     *encryptCallback,	/* [IN] encrypt function */
    void	        *encryptCallbackData,	/* [IN] (optional) */
    PKICONTEXT		*memFunc);

int sm_DecryptMessage (
	unsigned char	**msg,		/* [OUT] decrypted message */
	size_t		*msglen,	/* [OUT] decrypted message length */
	PKIEnvelopedData *envelope,	/* [IN] encrypted message */
	PKICertificate   *cert,		/* [IN] cert with key to decrypt for */
	DecryptCallback *callback,	/* [IN] decryption callback function */
	void		*data,		/* [IN] callback data (optional) */
	PKICONTEXT	*context);

int sm_SignMessage (
	unsigned char	**signedMessage,	/* [OUT] */
	size_t		*signedMessageLen,	/* [OUT] */
	unsigned char	*tbs,  			/* [IN] data to sign */
	size_t		tbsLen,	    		/* [IN] length of `tbs' */
	int		nested,			/* [IN] nested crypto protect */
	const char	*contentType,		/* [IN] type of `tbs' */
	const char	*digestAlgorithm,	/* [IN] */
	const unsigned char *digestParam,	/* [IN] */
	size_t		digestParamSize,	/* [IN] */
	const char	*signatureAlgorithm,	/* [IN] */
	const unsigned char *sigParam,		/* [IN] */
	size_t		sigParamSize,		/* [IN] */
	PKICertificate	*signerPKICertificate,	/* [IN] */
	PKIAttributes	*signedAttributes,	/* [IN] (optional) */
	List		*includePKICertificates,/* [IN] (optional) */
	HashCallback	*hashCallback,		/* [IN] */
	void		*hashCallbackData,	/* [IN] (optional) */
	SignCallback	*signCallback,		/* [IN] */
	void		*signCallbackData,	/* [IN] (optional) */
	PKICONTEXT	*context);

int sm_VerifyMessage (
    unsigned char   **signedData,       /* [OUT] */
    size_t          *signedDataLen,     /* [OUT] */
    PKISignedData   *msg,               /* [IN] */
    PKICertificate  *cert,              /* [IN] */
    HashCallback    *hashCallback,	/* [IN] */
    void            *hashCallbackData,	/* [IN] */
    VerifyCallback  *callback,          /* [IN] */
    void            *data,              /* [IN] (optional) */
    PKICONTEXT      *context);

PKIContentInfo *
sm_DecodeMessage (
	const unsigned char *msg,
	size_t msglen,
	PKICONTEXT *memFunc);

PKISignedData *
sm_DecodeSignedData (
	const unsigned char *,
	size_t,
	PKICONTEXT *);

PKIEnvelopedData *
sm_DecodeEnvelopedData (
	const unsigned char *,
	size_t,
	PKICONTEXT *);

/* Converts a printable OID string in dot-notation (eg.  1.2.3.4.5) to the DER
   encoded ASN.1 representation */
PKIOBJECT_ID *sm_StringToOID (const char *str, PKICONTEXT *memFunc);

/* Converts from DER ASN.1 to dot-notation */
char *sm_OIDToString (PKIOBJECT_ID *id, PKICONTEXT *memFunc);

/* compares the DER of two ASN.1 OBJECT IDENTIFIERs
   returns PKCS7_OK if equivalent */
int sm_CompareOID (PKIOBJECT_ID *a, PKIOBJECT_ID *b);

/* Copies the Issuer Distinguished Name and Issuer Serial Number from the
   specified PKICertificate and places them in the IssuerAndSerialNumber struct */
int
sm_CopyIssuerAndSerialNumber (
	PKIIssuerAndSerialNumber *out,
	PKICertificate *in,
	PKICONTEXT *memFunc);

/* Copy a Distinguished Name field */
int
sm_CopyDistinguishedName (
	PKIName		*out,
	PKIName		*in,
	PKICONTEXT	*ctx
);

int
sm_ContentType (
	PKIOBJECT_ID *asn,
	PKICONTEXT *mem);

#define sm_ContentInfoType(x,c) sm_ContentType((x)->contentType,c)

int
sm_MessageType (
	PKIContentInfo *info,
	PKICONTEXT *mem);

/* compare two IssuerAndSerialNumber blocks for equality.  Returns 1 if equal */
int
sm_CompareIssuerAndSerialNumber (
	PKIIssuerAndSerialNumber *,
	PKIIssuerAndSerialNumber *,
	PKICONTEXT *);

/* this is a callback function to be provided by the application.  In certain
   cases the PKCS#7 library needs to be able to follow a certificate chain,
   and using this callback function allows the application to use its own
   certificate database */

int
sm_GetIssuerPKICertificate (
	PKICertificate **issuer /* [OUT] */,
	PKICertificate *subject /* [IN] */);

extern const char *SM_OID_MESSAGE_DIGEST; /* id-messageDigest */
extern const char *SM_OID_CONTENT_DATA;
extern const char *SM_OID_CONTENT_SIGNED_DATA;
extern const char *SM_OID_CONTENT_ENVELOPED_DATA;

#define PKCS7_OK 0
#define PKCS7_ERROR_HASH_ALG -2         /* unsupported hash algorithm */
#define PKCS7_ERROR_HASH_CALLBACK -3    /* generic error occured in hash callback */
#define PKCS7_ERROR_SIGN_CALLBACK -4    /* generic error occured in sign callback */
#define PKCS7_ERROR_CALLBACK -5         /* generic error occured in callback */
#define PKCS7_ERROR_BAD_SIG -6          /* invalid signature */
#define PKCS7_ERROR_DATA_ENC_ALG -7     /* unsupported data encryption algorithm */
#define PKCS7_ERROR_UNKNOWN -8          /* unknown error */
#define PKCS7_ERROR_ATTRIBUTES -9       /* error in user supplied signing attributes */
#define PKCS7_ERROR_NOT_EQUAL -10
#define PKCS7_ERROR_HASH_ASN -11	/* unable to parse ASN.1 hash value */
#define PKCS7_ERROR_MISSING_HASH -12	/* unable to find hash value auth attr */
#define PKCS7_ERROR_BAD_HASH_VAL -13	/* computed hash does not match hash attr */

extern const char *SM_OID_ALG_KEA;
extern const char *SM_OID_ALG_SHA;
extern const char *SM_OID_ALG_SKIPJACK;
extern const char *SM_OID_ALG_DSA_SHA;
extern const char *SM_OID_ALG_DSA;
extern const char *SM_OID_ALG_RSA;
extern const char *SM_OID_ALG_MD5;
extern const char *SM_OID_ALG_MD2;
extern const char *SM_OID_ALG_DES;
extern const char *SM_OID_ALG_RC2;
extern const char *SM_OID_ALG_RSA_MD5;
extern const char *SM_OID_ALG_RSA_MD2;
extern const char *SM_OID_ALG_3DES;

/* support for CryptoAPI 1.0 */

#undef MSCAPI

#ifdef MSCAPI
#include "mscapicb.h"
#endif /* MSCAPI */

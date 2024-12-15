/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: February 16, 1999 */

#include "tc.h"
#include "libpkcs7.h"

#include "pgpPFLErrors.h"
#include "pgpMemoryMgr.h"
#include "pgpPubTypes.h"
#include "pgpOptionList.h"
#include "pgpKeys.h"
#include "pgpUtilities.h"
#include "pgpPublicKey.h"
#include "pgpErrors.h"


/*
** callback specific data for CMS and PKCS-7 lib
*/

typedef struct {
    PGPOptionListRef	passphrase;
    PGPContextRef	context;
    PGPKeyRef		key;
} X509CMSCallbackData;

/*
** ASN.1 memory manager procs
*/

void *
x509CMSAllocProc (
	PKIMemoryMgr    *mem,
	size_t          requestSize);

int
x509CMSDeallocProc (
	PKIMemoryMgr    *mem,
	void            *allocation);

int
x509CMSReallocProc (
	PKIMemoryMgr    *mem,
	void            **allocation,
	size_t          requestSize);

/*
** CMS callback functions
*/

int
x509CMSSignCallback (unsigned char **sig,
        size_t *siglen,
        unsigned char *ber,
        size_t berlen,
        const unsigned char *alg,
        size_t algLen,
        void *data,
        TC_CONTEXT *ctx);

int
x509CMSVerifyCallback (
        unsigned char *data,
        size_t dataLen,
        unsigned char *sigalg,
        size_t algLen,
        unsigned char *params,
        size_t paramLen,
        unsigned char *signature,
        size_t sigLen,
        unsigned char *pubkey,
        size_t pubkeyLen,
        TC_CERT *issuer,
        void *verData,
        TC_CONTEXT *ctx);

extern PKIMemoryMgr X509CMSMemoryMgr;

/*
** PKCS-7 callback functions
*/

int
pkcs7HashCallback (PKIOCTET_STRING *hashValue /* OUT */,
        const char *hashAlgorithm /* IN */,
        const unsigned char *tbs /* IN */,
        size_t tbsLen /* IN */,
        void *data /* IN */,
	PKICONTEXT *mem
);

int
pkcs7SignCallback (PKIOCTET_STRING *sigValue /* OUT */,
        const char *hashAlgorithm, /* IN */
        const char *signatureAlgorithm /* IN */,
        PKICertificate *signerPKICertificate /* IN */,
        unsigned char *tbs /* IN */,
        size_t tbsLen /* IN */,
        void *data /* IN */,
	PKICONTEXT *mem);

int
pkcs7VerifyCallback (
        const unsigned char     *tbs,                   /* signed data */
        size_t                  tbslen,                 /* signed data len */
        const char              *digestAlg,             /* hash alg */
        const char              *digestEncryptionAlg,   /* alg to encrypt sig */
        PKIEncryptedDigest      *signature,             /* encrypted sig */
        PKICertificate          *cert,                  /* signer cert */
        void                    *data,                  /* [IN] callback data
                                                           (optional */
	PKICONTEXT		*mem
);

int
pkcs7EncryptCallback (
        PKIOCTET_STRING		*encryptedData, /* [OUT] encrypted data */
        PKIANY			*encryptParam,  /* [OUT] data encryption
						   parameters (e.g.,
						   initialization vector) */
        const char              *dataEncAlg,    /* [IN] data encryption alg */
        const unsigned char     *tbe,           /* [IN] data to encrypt */
        size_t                  tbelen,         /* [IN] size of data */
        List                    *recips,        /* [IN/OUT] who to encrypt to.
                                                   The callback also returns
                                                   the encrypted session key
                                                   for each recipient in this
						   variable. */
        void                    *data,          /* [IN] user supplied data
                                                   (optional) */
	PKICONTEXT		*mem);

int
pkcs7DecryptCallback (
        unsigned char           **msg,          /* [OUT] decrypted data */
        size_t                  *msglen,        /* [OUT] decrypted data size */
        const char              *contentEncAlg, /* [IN] data encrypted alg */
        PKIANY			*param,		/* [IN] data encryption
						   parameter (e.g.,
						   initialization vector) */
        PKIEncryptedContent     *content,       /* [IN] encrypted data */
        const char              *keyEncAlg,     /* [IN] key encryption alg */
        PKIEncryptedKey         *enckey,        /* [IN] encrypted key */
        PKICertificate		*cert,          /* [IN] key to decrypt with */
        void                    *data,		/* [IN] callback data
						   (optional) */
	PKICONTEXT		*mem
);

/*
** utility functions
*/

PGPError
x509ExtractCertificate (
	PGPContextRef	context,
	PGPKeyRef	key,
	PKICONTEXT	*asnContext,
	PKICertificate	**cert);

/*
** structure used in generation of subjectPublicKeyInfo
*/

typedef struct {
    const PGPByte	*keyAlg;
    PGPSize			keyAlgSize;
    PGPByte			*keyData;
    PGPSize			keyDataSize;
    PGPByte			*keyParm;
    PGPSize			keyParmSize;
    const PGPByte	*sigAlg;
    PGPSize			sigAlgSize;
    PGPByte			*sigParm;
    PGPSize			sigParmSize;
} X509SubjectPublicKeyInfo;

PGPError x509CreateSubjectPublicKeyInfo (
	PGPKeyRef keyref,
	PKICONTEXT *asnContext,
	X509SubjectPublicKeyInfo *info);

void x509FreeSubjectPublicKeyInfo (X509SubjectPublicKeyInfo *);

/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: November 12, 1998 */

#include "x509CMS.h"

#include "pgpHash.h"
#include "pgpErrors.h"
#include "pgpSymmetricCipher.h"
#include "pgpCBC.h"

int
pkcs7HashCallback (
	PKIOCTET_STRING		*hashValue,	/* OUT */
	const char		*hashAlgorithm,
	const unsigned char	*tbs,
	size_t			tbsLen,
	void			*data,
	PKICONTEXT		*asnmem
)
{
    PGPError err;
    PGPHashContextRef hash;
    PGPSize hashLen;
    PGPHashAlgorithm algid;
    X509CMSCallbackData *pgpData = (X509CMSCallbackData *) data;
    PGPMemoryMgrRef mem;

    if (!strcmp (SM_OID_ALG_MD5, hashAlgorithm))
	algid = kPGPHashAlgorithm_MD5;
    else if (!strcmp (SM_OID_ALG_SHA, hashAlgorithm))
	algid = kPGPHashAlgorithm_SHA;
    else
	return PKCS7_ERROR_HASH_ALG;

    mem = PGPGetContextMemoryMgr (pgpData->context);

    err = PGPNewHashContext (mem, algid, &hash);
    if (IsPGPError (err))
	return PKCS7_ERROR_HASH_CALLBACK;
    
    err = PGPContinueHash (hash, tbs, tbsLen);
    if (IsPGPError (err))
    {
	err = PGPFreeHashContext (hash);
	return PKCS7_ERROR_HASH_CALLBACK;
    }

    err = PGPGetHashSize (hash, &hashLen);
    if (IsPGPError (err))
    {
	err = PGPFreeHashContext (hash);
	return PKCS7_ERROR_HASH_CALLBACK;
    }
    
    hashValue->len = hashLen;
    hashValue->val = PKIAlloc (asnmem->memMgr, hashLen);
    
    err = PGPFinalizeHash (hash, hashValue->val);
    if (IsPGPError (err))
    {
	PKIFree (asnmem->memMgr, hashValue->val);
	hashValue->val = NULL;
	hashValue->len = 0;
	err = PGPFreeHashContext (hash);
	return PKCS7_ERROR_HASH_CALLBACK;
    }
    
    err = PGPFreeHashContext (hash);
    
    return PKCS7_OK;
}

int
pkcs7SignCallback (
	PKIOCTET_STRING *sigValue,		/* OUT */
	const char	*hashAlgorithm,
	const char	*signatureAlgorithm,
	PKICertificate	*signerCertificate,
	unsigned char	*tbs,
	size_t		tbsLen,
	void		*data,
	PKICONTEXT	*asnmem)
{
    PGPError err;
    PGPHashContextRef hash;
    PGPHashAlgorithm algid;
    PGPPrivateKeyContextRef privkey;
    X509CMSCallbackData *pgpData = (X509CMSCallbackData *) data;
    PGPMemoryMgrRef	mem;
    PGPOptionListRef	pass;

	(void) signatureAlgorithm;
	(void) signerCertificate;
	
    if (!strcmp (SM_OID_ALG_MD5, hashAlgorithm))
	algid = kPGPHashAlgorithm_MD5;
    else if (!strcmp (SM_OID_ALG_SHA, hashAlgorithm))
	algid = kPGPHashAlgorithm_SHA;
    else
	return PKCS7_ERROR_HASH_ALG;

    mem = PGPGetContextMemoryMgr (pgpData->context);

    err = PGPNewHashContext (mem, algid, &hash);
    if (IsPGPError (err))
	return PKCS7_ERROR_SIGN_CALLBACK;
    
    err = PGPContinueHash (hash, tbs, tbsLen);
    if (IsPGPError (err))
    {
	err = PGPFreeHashContext (hash);
	return PKCS7_ERROR_SIGN_CALLBACK;
    }

    PGPCopyOptionList (pgpData->passphrase, &pass);
    err = PGPNewPrivateKeyContext (pgpData->key,
	    kPGPPublicKeyMessageFormat_X509,
	    &privkey,
	    pass,
	    PGPOLastOption (pgpData->context));
    if (IsPGPError (err))
    {
	err = PGPFreeHashContext (hash);
	return PKCS7_ERROR_SIGN_CALLBACK;
    }
    
    err = PGPGetPrivateKeyOperationSizes (privkey, NULL, NULL, &sigValue->len);
    if (IsPGPError (err))
    {
	err = PGPFreeHashContext (hash);
	err = PGPFreePrivateKeyContext (privkey);
	return PKCS7_ERROR_SIGN_CALLBACK;
    }

    sigValue->val = PKIAlloc (asnmem->memMgr, sigValue->len);

    err = PGPPrivateKeySign (privkey, hash, sigValue->val, &sigValue->len);
    if (IsPGPError (err))
    {
	PKIFree (asnmem->memMgr, sigValue->val);
	sigValue->val = NULL;
	sigValue->len = 0;
	err = PGPFreeHashContext (hash);
	err = PGPFreePrivateKeyContext (privkey);
	return PKCS7_ERROR_HASH_CALLBACK;
    }
    
    err = PGPFreePrivateKeyContext (privkey);
    
    return PKCS7_OK;
}

int
pkcs7VerifyCallback (
 	const unsigned char     *tbs,                   /* signed data */
	size_t                  tbsLen,                 /* signed data len */
	const char              *digestAlg,             /* hash alg */
	const char              *digestEncryptionAlg,   /* alg to decrypt sig */
	PKIEncryptedDigest      *signature,             /* encrypted sig */
	PKICertificate          *cert,                  /* signer cert */
	void                    *data,                  /* [IN] callback data
							   (optional) */
	PKICONTEXT		*asnmem
)
{
    X509CMSCallbackData		*pgpData = (X509CMSCallbackData *) data;
    PGPMemoryMgrRef		mem;
    PGPHashAlgorithm		algid;
    PGPPublicKeyContextRef	pubkey;
    PGPError			err;
    PGPHashContextRef		hash;
    PGPContextRef		context;

	(void) digestEncryptionAlg;
	(void) cert;
	(void) asnmem;
	
    if (!strcmp (SM_OID_ALG_MD5, digestAlg))
	algid = kPGPHashAlgorithm_MD5;
    else if (!strcmp (SM_OID_ALG_SHA, digestAlg))
	algid = kPGPHashAlgorithm_SHA;
    else
	return PKCS7_ERROR_HASH_ALG;

    context = PGPGetKeyContext (pgpData->key);

    mem = PGPGetContextMemoryMgr (context);

    err = PGPNewHashContext (mem, algid, &hash);
    if (IsPGPError (err))
	return PKCS7_ERROR_HASH_CALLBACK;
    
    err = PGPContinueHash (hash, tbs, tbsLen);
    if (IsPGPError (err))
    {
	err = PGPFreeHashContext (hash);
	return PKCS7_ERROR_HASH_CALLBACK;
    }
    
    err = PGPNewPublicKeyContext (
	    pgpData->key,
	    kPGPPublicKeyMessageFormat_X509,
	    &pubkey);
    if (IsPGPError (err))
    {
	err = PGPFreeHashContext (hash);
	return PKCS7_ERROR_HASH_CALLBACK;
    }

    err = PGPPublicKeyVerifySignature (pubkey,
	    hash,
	    signature->val,
	    signature->len);
    if (IsPGPError (err))
    {
	err = PGPFreeHashContext (hash);
	err = PGPFreePublicKeyContext (pubkey);
	return PKCS7_ERROR_HASH_CALLBACK;
    }

    err = PGPFreePublicKeyContext (pubkey);

    return PKCS7_OK;
}

int
pkcs7EncryptCallback (
        PKIOCTET_STRING        *encryptedData, /* [OUT] encrypted data */
        PKIANY                 *encryptParam,  /* [OUT] data encryption
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
        void                    *data,          /* [IN] user supplied data */
	PKICONTEXT		*asnmem
)
{
    PGPSymmetricCipherContextRef	cipherRef;
    PGPCBCContextRef		cbcRef;
    PGPError			err;
    PGPCipherAlgorithm		cipherAlg;
    unsigned char		*randomBuf;
    X509CMSCallbackData		*pgpData = (X509CMSCallbackData *) data;
    PGPMemoryMgr *mem = PGPGetContextMemoryMgr (pgpData->context);
    PGPSize keySize = 24;	/* size for 3des.  this has to be hardcoded
				   for now since PGPsdk doesn't provide a way
				   for the application to query the value
				   in a generic way */
    PGPSize			blockSize;
    PGPSize			plainTextLen;
    PGPPublicKeyContextRef	pubKey;
    unsigned char		*lastBlock;
    PGPSize			lastBlockLen;
    size_t				i;
    PKIOCTET_STRING		*iv;
    int				e = 0;
    List			*pRecip;

    if (!strcmp (dataEncAlg, SM_OID_ALG_3DES))
	cipherAlg = kPGPCipherAlgorithm_3DES;
    else
    {
	/* unsupported algorithm */
	return PKCS7_ERROR_CALLBACK;
    }

    err = PGPNewSymmetricCipherContext (mem, cipherAlg, keySize, &cipherRef);
    if (IsPGPError (err))
    {
	return PKCS7_ERROR_CALLBACK;
    }

    err = PGPGetSymmetricCipherSizes (cipherRef, &keySize, &blockSize);
    if (IsPGPError (err))
    {
	PGPFreeSymmetricCipherContext (cipherRef);
	return PKCS7_ERROR_CALLBACK;
    }

    err = PGPNewCBCContext (cipherRef, &cbcRef);
    if (IsPGPError (err))
    {
	PGPFreeSymmetricCipherContext (cipherRef);
	return PKCS7_ERROR_CALLBACK;
    }

    /* Generate a random session key and IV */
    randomBuf = PGPNewSecureData (mem, keySize, 0);
    err = PGPContextGetRandomBytes (pgpData->context, randomBuf, keySize);
    if (IsPGPError (err))
    {
	PGPFreeData (randomBuf);
	PGPFreeCBCContext (cbcRef);
	return PKCS7_ERROR_CALLBACK;
    }

    iv = PKINewOCTET_STRING (asnmem);
    iv->len = blockSize;
    iv->val = PKIAlloc (asnmem->memMgr, iv->len);
    err = PGPContextGetRandomBytes (pgpData->context, iv->val, iv->len);
    if (IsPGPError (err))
    {
	PKIFreeOCTET_STRING (asnmem, iv);
	PGPFreeData (randomBuf);
	PGPFreeCBCContext (cbcRef);
	return PKCS7_ERROR_CALLBACK;
    }

    err = PGPInitCBC (cbcRef, randomBuf, iv->val);
    if (IsPGPError (err))
    {
	PKIFreeOCTET_STRING (asnmem, iv);
	PGPFreeData (randomBuf);
	PGPFreeCBCContext (cbcRef);
	return PKCS7_ERROR_CALLBACK;
    }

    /* encrypt all full blocks in one pass */
    plainTextLen = tbelen - (tbelen % blockSize);
    encryptedData->len = plainTextLen + blockSize;
    encryptedData->val = PKIAlloc (asnmem->memMgr, encryptedData->len);
    err = PGPCBCEncrypt (cbcRef, tbe, plainTextLen, encryptedData->val);
    if (IsPGPError (err))
    {
	PKIFreeOCTET_STRING (asnmem, iv);
	PGPFreeData (randomBuf);
	PGPFreeCBCContext (cbcRef);
	return PKCS7_ERROR_CALLBACK;
    }

    /* pad the last block according to PKCS-7 */
    lastBlockLen = tbelen - plainTextLen;
    lastBlock = PGPNewSecureData (mem, blockSize, 0);
    memcpy (lastBlock, tbe + plainTextLen, lastBlockLen);
    for (i = lastBlockLen; i < blockSize; i++)
	lastBlock[i] = blockSize - lastBlockLen;

    err = PGPCBCEncrypt (cbcRef, lastBlock, blockSize, encryptedData->val + plainTextLen);
    if (IsPGPError (err))
    {
	PKIFreeOCTET_STRING (asnmem, iv);
	PGPFreeData (randomBuf);
	PGPFreeData (lastBlock);
	PGPFreeCBCContext (cbcRef);
	return PKCS7_ERROR_CALLBACK;
    }

    PGPFreeData (lastBlock);

    err = PGPFreeCBCContext (cbcRef);
    if (IsPGPError (err))
    {
	PKIFreeOCTET_STRING (asnmem, iv);
	PGPFreeData (randomBuf);
	return PKCS7_ERROR_CALLBACK;
    }

    encryptParam->len = PKISizeofOCTET_STRING (asnmem, iv, TRUE);
    encryptParam->val = PKIAlloc (asnmem->memMgr, encryptParam->len);
    PKIPackOCTET_STRING (asnmem, encryptParam->val, encryptParam->len, iv, &e);
    if (e)
    {
	PKIFreeOCTET_STRING (asnmem, iv);
	PGPFreeData (randomBuf);
	return PKCS7_ERROR_CALLBACK;
    }
    PKIFreeOCTET_STRING (asnmem, iv);

    /* encrypt session key for each recipient */
    for (pRecip = recips; pRecip; pRecip = pRecip->next)
    {
	PGPSize decSize;
	PGPSize sigSize;
	EncryptRecipient *info = (EncryptRecipient *) pRecip->data;

	err = PGPNewPublicKeyContext ((PGPKeyRef) info->data,
		kPGPPublicKeyMessageFormat_PKCS1, /* HACK - only for RSA */
		&pubKey);
	if (IsPGPError (err))
	{
	    PGPFreeData (randomBuf);
	    return PKCS7_ERROR_CALLBACK;
	}

	err = PGPGetPublicKeyOperationSizes (pubKey,
		&decSize,
		&info->encryptedKeyLen,
		&sigSize);
	if (IsPGPError (err))
	{
	    PGPFreeData (randomBuf);
	    return PKCS7_ERROR_CALLBACK;
	}

	info->encryptedKey = PKIAlloc (asnmem->memMgr, info->encryptedKeyLen);

	err = PGPPublicKeyEncrypt (pubKey,
		randomBuf,
		keySize,
		info->encryptedKey,
		&info->encryptedKeyLen);
	if (IsPGPError (err))
	{
	    PGPFreeData (randomBuf);
	    return PKCS7_ERROR_CALLBACK;
	}

	err = PGPFreePublicKeyContext (pubKey);
	if (IsPGPError (err))
	{
	    PGPFreeData (randomBuf);
	    return PKCS7_ERROR_CALLBACK;
	}
	/* ??? should this be part of the PKCS7 library rather than the
	   responsibility of the callback? */
	info->algorithm = sm_OIDToString (&info->certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.algorithm, asnmem);
    }

    PGPFreeData (randomBuf);

    return PKCS7_OK;
}

int
pkcs7DecryptCallback (
        unsigned char           **msg,          /* [OUT] decrypted data */
        size_t                  *msgLen,        /* [OUT] decrypted data len */
        const char              *contentEncAlg, /* [IN] data encrypted alg */
        PKIANY			*param,     	/* [IN] data encryption
						   parameter (e.g.,
						   initialization vector) */
        PKIEncryptedContent     *content,       /* [IN] encrypted data */
        const char              *keyEncAlg,     /* [IN] key encryption alg */
        PKIEncryptedKey         *enckey,        /* [IN] encrypted key */
        PKICertificate          *cert,          /* [IN] key to decrypt with */
        void                    *data,          /* [IN] callback data
						   (optional) */
	PKICONTEXT		*asnmem
)
{
    X509CMSCallbackData			*pgpData = (X509CMSCallbackData *) data;
    PGPError				err;
    PGPSize				decmax;
    PGPByte				*encKeyData;
    PGPMemoryMgrRef			mem;
    PGPCipherAlgorithm			symKeyAlg;
    PGPSymmetricCipherContextRef	cipherRef;
    PGPCBCContextRef			cbcRef;
    PGPPrivateKeyContextRef		privKey;
    PGPSize				keySize;
    PGPSize				blockSize;
    PGPOptionListRef			pass;

    PKIOCTET_STRING *iv;
    int e = 0;
    size_t pad;

	(void) keyEncAlg;
	(void) cert;
	
    *msg = NULL;
    *msgLen = 0;

    if (!strcmp (contentEncAlg, SM_OID_ALG_3DES))
	symKeyAlg = kPGPCipherAlgorithm_3DES;
    else
    {
	/* unsupported algorithm */
	return PKCS7_ERROR_CALLBACK;
    }

    /* decrypt the encrypted session key with our private key */
    PGPCopyOptionList (pgpData->passphrase, &pass);
    err = PGPNewPrivateKeyContext (pgpData->key,
	    kPGPPublicKeyMessageFormat_PKCS1,
	    &privKey,
	    pass,
	    PGPOLastOption (pgpData->context));
    if (IsPGPError (err))
	return PKCS7_ERROR_CALLBACK;

    err = PGPGetPrivateKeyOperationSizes (privKey, &decmax, NULL, NULL);
    if (IsPGPError (err))
    {
	err = PGPFreePrivateKeyContext (privKey);
	return PKCS7_ERROR_CALLBACK;
    }

    mem = PGPGetContextMemoryMgr (pgpData->context);

    encKeyData = PGPNewSecureData (mem, decmax, 0);

    err = PGPPrivateKeyDecrypt (privKey,
	    enckey->val,
	    enckey->len,
	    encKeyData,
	    &decmax);
    if (IsPGPError (err))
    {
	PGPFreeData (encKeyData);
	err = PGPFreePrivateKeyContext (privKey);
	return PKCS7_ERROR_CALLBACK;
    }

    err = PGPFreePrivateKeyContext (privKey);

    /* unpack the parameters for the decryption (IV) */
    PKIUnpackOCTET_STRING (asnmem, &iv, param->val, param->len, &e);
    if (e)
    {
	PGPFreeData (encKeyData);
	return PKCS7_ERROR_CALLBACK;
    }

    /* now decrypt the message data with the extracted key */
    err = PGPNewSymmetricCipherContext (mem, symKeyAlg, decmax, &cipherRef);
    if (IsPGPError (err))
    {
	PGPFreeData (encKeyData);
	PKIFreeOCTET_STRING (asnmem, iv);
	return PKCS7_ERROR_CALLBACK;
    }

    err = PGPGetSymmetricCipherSizes (cipherRef, &keySize, &blockSize);
    if (IsPGPError (err))
    {
	PGPFreeData (encKeyData);
	PKIFreeOCTET_STRING (asnmem, iv);
	err = PGPFreeSymmetricCipherContext (cipherRef);
	return PKCS7_ERROR_CALLBACK;
    }

    err = PGPNewCBCContext (cipherRef, &cbcRef);
    if (IsPGPError (err))
    {
	PGPFreeData (encKeyData);
	err = PGPFreeSymmetricCipherContext (cipherRef);
	PKIFreeOCTET_STRING (asnmem, iv);
	return PKCS7_ERROR_CALLBACK;
    }

    err = PGPInitCBC (cbcRef, encKeyData, iv->val);
    if (IsPGPError (err))
    {
	err = PGPFreeCBCContext (cbcRef);
	PKIFreeOCTET_STRING (asnmem, iv);
	PGPFreeData (encKeyData);
	return PKCS7_ERROR_CALLBACK;
    }
    PKIFreeOCTET_STRING (asnmem, iv);
    PGPFreeData (encKeyData);

    *msgLen = content->len;
    *msg = PKIAlloc (asnmem->memMgr, content->len);
    err = PGPCBCDecrypt (cbcRef, content->val, content->len, *msg);
    if (IsPGPError (err))
    {
	PKIFree (asnmem->memMgr, *msg);
	*msg = NULL;
	*msgLen = 0;
	err = PGPFreeCBCContext (cbcRef);
	return PKCS7_ERROR_CALLBACK;
    }

    /* remove PKCS7 padding in last block */
    pad = *(*msg + *msgLen - 1);
    if (pad < 1 || pad > blockSize)
	return PKCS7_ERROR_CALLBACK; /* invalid pad, decryption failed */
    *msgLen -= pad;

    return PKCS7_OK;
}

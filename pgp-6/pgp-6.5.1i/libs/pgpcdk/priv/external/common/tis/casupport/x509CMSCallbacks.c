/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: Dec 4, 1998 */

#include "x509CMS.h"

static PGPHashAlgorithm
x509HashAlgorithm (const PGPByte *alg, PGPSize algLen)
{
	(void) algLen;
	
    if (memcmp (alg, TC_ALG_RSA_MD5, TC_ALG_RSA_MD5_LEN) == 0)
	return kPGPHashAlgorithm_MD5;
    else if (memcmp (alg, TC_ALG_DSA_SHA1, TC_ALG_DSA_SHA1_LEN) == 0
	     || memcmp (alg, TC_ALG_RSA_SHA1, TC_ALG_RSA_SHA1_LEN) == 0)
	return kPGPHashAlgorithm_SHA;
    return kPGPHashAlgorithm_Invalid;
}

/*****
*
* sign_certificate
*
* Call-out routine used by the CMS library for signing provided data
* (a certificate).
*
* Parameters and return values defined in CMS docs.  This uses the
* PGP SDK.  See "PGP Software Developer's Kit, Reference Manual"
* for details on calls used here.
*
*****/
int
x509CMSSignCallback (unsigned char **sig,
	size_t *siglen,
	unsigned char *ber,
	size_t berlen,
	const unsigned char *alg,
	size_t algLen,
	void *data,
	TC_CONTEXT *ctx)
{
    X509CMSCallbackData *pgpData = (X509CMSCallbackData *) data;

    PGPPrivateKeyContextRef privKey = NULL;
    PGPHashContextRef hash;
    PGPMemoryMgrRef mgrRef;
    PGPError err;
    int ret = -1;
    PGPOptionListRef	pass;
    PGPHashAlgorithm	hashAlg;

    (void) alg;
    (void) algLen;

    if (!pgpData || !pgpData->passphrase || !pgpData->context)
	return -1;

    do {
	/* create and calculate the hash, do not finalize hash
	   for PGPPrivateKeySign() call */
	mgrRef = PGPGetContextMemoryMgr (pgpData->context);

	hashAlg = x509HashAlgorithm (alg, algLen);

	err = PGPNewHashContext (mgrRef, hashAlg, &hash);
	if (IsPGPError (err))
	    break;
	err = PGPContinueHash (hash, ber, berlen);
	if (IsPGPError (err))
	    break;

	/* create the private key context */
	PGPCopyOptionList (pgpData->passphrase, &pass);
	err = PGPNewPrivateKeyContext (pgpData->key,
		kPGPPublicKeyMessageFormat_X509,
		&privKey,
		pass,
		PGPOLastOption (pgpData->context));
	if (IsPGPError (err))
	    break;

	/* figure out signature size */
	err = PGPGetPrivateKeyOperationSizes (privKey, NULL, NULL, siglen);
	if (IsPGPError (err))
	    break;
	if (*siglen == 0) /* can't use key to sign */
	    break;
	*sig = TC_Alloc (ctx->memMgr, *siglen);
	if (*sig == NULL)
	    break;
	memset (*sig, 0, *siglen);

	/* calculate signature */
	err = PGPPrivateKeySign (privKey, hash, *sig, siglen);
	hash = NULL; /* dont attempt to free it later */
	if (IsPGPError (err))
	    break;

	ret = 0;
    }
    while (0);

    /* clean-up */
    if (hash)
	err = PGPFreeHashContext (hash);
    err = PGPFreePrivateKeyContext (privKey);

    return ret;

} /* sign */

/*****
*
* verify_signature
*
* Call-out routine used by the CMS library for verifying a signature.
* Parameters and return values defined in CMS docs.  This uses the
* PGP SDK.  See "PGP Software Developer's Kit, Reference Manual"
* for details on calls used here.
*
*****/
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
	TC_CONTEXT *ctx)
{    
    X509CMSCallbackData		*pgpData = (X509CMSCallbackData *) verData;
    PGPMemoryMgrRef		mgrRef;
    PGPPublicKeyContextRef	publicKey;
    PGPHashContextRef		hash;
    PGPKeySetRef		keySet;
    PGPKeyListRef		klist;
    PGPKeyIterRef		kiter;
    PGPKeyRef			mykey;
    PGPError			err;
    PGPByte			*certData;
    PGPSize			certDataLen;
    int				rtn = -1;
    PGPHashAlgorithm		hashAlg;
    
    (void) params;
    (void) paramLen;
    (void) pubkey;
    (void) pubkeyLen;
    
    do {
        /* check that the algorithm ID is allowed */
         /* TODO... */

	/* if we dont have the issuer's certificate, we can't verify the
	   signature. */
	/* PGPsdk needs to have a function that takes raw ASN.1 encoded
	   key material and returns a KeyRef in order to support
	   verification of PKCS10 requests */
	if (!issuer)
	    break;

	/* import the certificate into pgp */
	tc_pack_cert (&certData, &certDataLen, issuer, ctx);

        /* import the key into a PGP key set */
	err = PGPImportKeySet (pgpData->context,
		&keySet,
		PGPOX509Encoding (pgpData->context, 1),
		PGPOInputBuffer (pgpData->context, certData, certDataLen),
		PGPOLastOption (pgpData->context));

	TC_Free (ctx->memMgr, certData);

	if (IsPGPError (err))
	    break;

        /* extract the key into a PGPKeyRef */
        err = PGPOrderKeySet (keySet, kPGPAnyOrdering, &klist);
	if (IsPGPError (err))
	    break;
        err = PGPNewKeyIter (klist, &kiter);
	if (IsPGPError (err))
	    break;
        err = PGPKeyIterNext (kiter, &mykey);
	if (IsPGPError (err))
	    break;

	/* create the public key context */
	err = PGPNewPublicKeyContext (mykey,
		kPGPPublicKeyMessageFormat_X509,
		&publicKey);
	if (IsPGPError (err))
	    break;

	hashAlg = x509HashAlgorithm (sigalg, algLen);

        /* create and calculate the hash, do not finalize hash
           for PGPPublicKeyVerify() call */
        mgrRef = PGPGetContextMemoryMgr (pgpData->context);
	err = PGPNewHashContext (mgrRef, hashAlg, &hash);
	if (IsPGPError (err))
	    break;

	err = PGPContinueHash (hash, data, dataLen);
        if (IsPGPError (err))
            break;

        err = PGPPublicKeyVerifySignature (
            publicKey, hash, (void *)signature, sigLen);
	hash = NULL;
        if ( IsPGPError(err) )
            break;

        rtn = 0;
    } while (0);

    /* clean-up */
    PGPFreeKeyIter(kiter);
    PGPFreeKeyList(klist);
    PGPFreeKeySet(keySet);
    if (hash)
	PGPFreeHashContext(hash);
    PGPFreePublicKeyContext (publicKey);

    return rtn;

} /* verify_signature */


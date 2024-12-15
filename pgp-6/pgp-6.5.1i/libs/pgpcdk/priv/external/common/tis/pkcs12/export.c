/* Copyright (C) 1999 Network Associates, Inc.
   Author: Michael_Elkins@nai.com
   Last Edit: April 9, 1999 */

/* this file contains code to export a PKCS-8 PrivateKeyInfo and X.509
   Certificate as a PKCS-12 message */

#include "pbe12.h"
#include "pkcs7_oid.h"

static PGPError
pbe12ShroudKey (PGPContextRef context,
		PKICONTEXT *pki,
		PGPPBEAlgorithm alg,
		const PGPByte *pass,
		PGPSize passSize,
		int iterations,
		const PGPByte *privKey,
		PGPSize privKeySize,
		PGPByte **out,
		PGPSize *outSize)
{
    PKIEncryptedPrivateKeyInfo *epki = 0;
    PKIpkcs_12PbeParams *pbeParam = 0;
    PGPError err;
    PGPMemoryMgrRef mem = PGPGetContextMemoryMgr (context);
    PGPSize keySize = 24; /* size of 3DES key */
    PGPByte *iv = 0;
    PGPByte *key = 0;
    PGPSymmetricCipherContextRef symRef = 0;
    PGPCBCContextRef cbcRef = 0;
    PGPByte block[8];
    PGPSize extraBytes;
    int asnerr;
    
    (void) alg; /* unused */

    *out = 0;
    *outSize = 0;
    
    epki = PKINewEncryptedPrivateKeyInfo (pki);

    PKIPutOctVal (pki, &epki->encryptionAlgorithm.algorithm,
		  PKIpbeWithSHA1And3_KeyTripleDES_CBC_OID,
		  PKIpbeWithSHA1And3_KeyTripleDES_CBC_OID_LEN);

    /* generate pbe parameters */
    pbeParam = PKINewpkcs_12PbeParams (pki);
    PKIPutIntVal (pki, &pbeParam->iterationCount, iterations);

    /* generate random salt value.  PKCS-12 says that in general the salt
       should be as long as the key used */
    pbeParam->salt.len = keySize;
    pbeParam->salt.val = PGPNewData (mem, keySize, 0);
    err = PGPContextGetRandomBytes (context, pbeParam->salt.val,
				    pbeParam->salt.len);
    if (err)
	goto error;

    /* pack pbe params */
    epki->encryptionAlgorithm.parameters = PKINewANY (pki);
    epki->encryptionAlgorithm.parameters->len = PKISizeofpkcs_12PbeParams (pki, pbeParam, PKITRUE);
    epki->encryptionAlgorithm.parameters->val = PGPNewData (mem, epki->encryptionAlgorithm.parameters->len, 0);
    asnerr = 0;
    PKIPackpkcs_12PbeParams (pki, epki->encryptionAlgorithm.parameters->val,
			     epki->encryptionAlgorithm.parameters->len,
			     pbeParam, &asnerr);
    if (asnerr)
	{
	    err = kPGPError_ASNPackFailure;
	    goto error;    
	}
    
    /* generate IV for symmetric encryption */
    err = pbe12GenerateRandomBits (context,
				   kPGPHashAlgorithm_SHA,
				   kPGPPBEIDIV,
				   iterations,
				   pbeParam->salt.val,
				   pbeParam->salt.len,
				   pass,
				   passSize,
				   keySize,
				   &iv);
    if (err)
	goto error;
    
    /* generate key for symmetric encryption */
    err = pbe12GenerateRandomBits (context,
				   kPGPHashAlgorithm_SHA,
				   kPGPPBEIDEncryptKey,
				   iterations,
				   pbeParam->salt.val,
				   pbeParam->salt.len,
				   pass,
				   passSize,
				   keySize,
				   &key);
    if (err)
	goto error;

    /* create encryption context */
    err = PGPNewSymmetricCipherContext (mem,
					kPGPCipherAlgorithm_3DES,
					24,
					&symRef);
    if (err)
	goto error;

    /* create CBC context */
    err = PGPNewCBCContext (symRef, &cbcRef);
    if (err)
	goto error;
    symRef = 0; /* FreeCBCContext() takes care of destroying symRef */
    
    /* initialiaze */
    err = PGPInitCBC (cbcRef, key, iv);
    if (err)
	goto error;

    extraBytes = privKeySize % 8;

    /* output will be a multiple of the block size, which for 3DES is 8.  we
       add an extra block for the pkcs-5 style padding */
    epki->encryptedData.len = privKeySize - extraBytes + 8;
    epki->encryptedData.val = PGPNewData (mem, epki->encryptedData.len, 0);

    /* encrypt all but last block */
    err = PGPCBCEncrypt (cbcRef, privKey, privKeySize - extraBytes,
			 epki->encryptedData.val);
    if (err)
	goto error;

    /* copy remaining unencrypted bytes into block[] */
    memcpy (block, privKey + privKeySize - extraBytes, extraBytes);

    /* add pkcs-5 pad */
    memset (block + extraBytes, 8 - extraBytes, 8 - extraBytes);

    /* encrypt final block */
    err = PGPCBCEncrypt (cbcRef, block, 8,
			 epki->encryptedData.val + privKeySize - extraBytes);
    if (err)
	goto error;

    /* pack up ASN.1 structure */
    *outSize = PKISizeofEncryptedPrivateKeyInfo (pki, epki, PKITRUE);
    *out = PGPNewData (mem, *outSize, 0);
    asnerr = 0;
    PKIPackEncryptedPrivateKeyInfo (pki, *out, *outSize, epki, &asnerr);
    if (asnerr)
	{
	    err = kPGPError_ASNPackFailure;
	    goto error;    
	}

    /* success */
    err = kPGPError_NoErr;
    
 error:
    
    if (IsPGPError (err))
	{
	    if (*out)
		{
		    PGPFreeData (*out);
		    *out = 0;
		}
	    *outSize = 0;
	}
    
    if (pbeParam)
	PKIFreepkcs_12PbeParams (pki, pbeParam);
    if (epki)
	PKIFreeEncryptedPrivateKeyInfo (pki, epki);
    if (cbcRef)
	PGPFreeCBCContext (cbcRef);
    if (symRef)
	PGPFreeSymmetricCipherContext (symRef);
    if (iv)
	PGPFreeData (iv);
    if (key)
	PGPFreeData (key);
    
    return err;
}

/* 1 3 14 3 2 26 */
const PGPByte PKIsha1_OID[] = { 0x2B, 0x0e, 0x03, 0x02, 0x1a };
#define PKIsha1_OID_LEN 5

static PGPError
pkcs12GenerateMAC (PGPContextRef context,
		   PKICONTEXT *pki,
		   PKIPFX *pfx,
		   PKIOCTET_STRING *tbh,
		   int iterations,
		   const PGPByte *pass,
		   PGPSize passSize,
		   PGPHashAlgorithm alg)
{
    PGPError err;
    PGPMemoryMgrRef mem = PGPGetContextMemoryMgr (context);
    PGPByte *key = 0;
    PGPHMACContextRef hashRef = 0;
    PGPSize keySize;
    
    switch (alg)
	{
	case kPGPHashAlgorithm_SHA:
	    keySize = 20;
	    break;
#if 0
	case kPGPHashAlgorithm_MD5:
	    keySize = 16;
	    break;
#endif
	default:
	    return kPGPError_LazyProgrammer; /* bad hash algorithm */
	}

    /* allocate storage for the mac data */
    pfx->macData = PKINewMacData (pki);
    if (iterations != 1)
	{
	    pfx->macData->macIterationCount = PKINewINTEGER (pki);
	    PKIPutIntVal (pki, pfx->macData->macIterationCount, iterations);
	}
    
    PKIPutOctVal (pki, &pfx->macData->mac.digestAlgorithm.algorithm,
		  PKIsha1_OID, PKIsha1_OID_LEN);

    /* generate salt value */
    pfx->macData->macSalt.len = keySize;
    pfx->macData->macSalt.val = PGPNewData (mem, keySize, 0);
    err = PGPContextGetRandomBytes (context, pfx->macData->macSalt.val, keySize);
    if (IsPGPError (err))
	goto error;

    /* generate MAC key */
    err = pbe12GenerateRandomBits (context,
				   alg,
				   kPGPPBEIDMACKey,
				   iterations,
				   pfx->macData->macSalt.val,
				   pfx->macData->macSalt.len,
				   pass,
				   passSize,
				   keySize,
				   &key);
    /* set up HMAC context */
    err = PGPNewHMACContext (mem, alg, key, keySize, &hashRef);
    if (IsPGPError (err))
	goto error;
    err = PGPContinueHMAC (hashRef, tbh->val, tbh->len);
    if (IsPGPError (err))
	goto error;

    pfx->macData->mac.digest.len = keySize;
    pfx->macData->mac.digest.val = PGPNewData (mem, keySize, 0);
    err = PGPFinalizeHMAC (hashRef, pfx->macData->mac.digest.val);
    if (IsPGPError (err))
	goto error;
    
    err = kPGPError_NoErr;
    
 error:
    if (key)
	PGPFreeData (key);
    if (hashRef)
	PGPFreeHMACContext (hashRef);
    return err;
    
}

static PGPError
pkcs12CreateCertBag (PGPContextRef context,
		     PKICONTEXT *pki,
		     const PGPByte *cert,
		     PGPSize certSize,
		     PKISafeBag **safeBag)
{
    PKISafeBag *sb;
    PKICertBag *certBag = 0;
    int asnerr = 0;
    PGPError err;
    PGPMemoryMgrRef mem = PGPGetContextMemoryMgr (context);
    PKIOCTET_STRING *oct = 0;
    
    oct = PKINewOCTET_STRING (pki);
    PKIPutOctVal (pki, oct, cert, certSize);

    *safeBag = PKINewSafeBag (pki);
    sb = *safeBag;
    
    PKIPutOctVal (pki, &sb->bagType, PKIcertBag_OID, PKIcertBag_OID_LEN);

    certBag = PKINewCertBag (pki);
    PKIPutOctVal (pki, &certBag->certType,
		  PKIx509Certificate_OID,
		  PKIx509Certificate_OID_LEN);
    certBag->cert.len = PKISizeofOCTET_STRING (pki, oct, PKITRUE);
    certBag->cert.val = PGPNewData (mem, certBag->cert.len, 0);
    asnerr = 0;
    PKIPackOCTET_STRING (pki, certBag->cert.val, certBag->cert.len, oct,
	&asnerr);
    if (asnerr)
      {
	err = kPGPError_ASNPackFailure;
	goto error;
      }

    sb->bagContent.len = PKISizeofCertBag (pki, certBag, PKITRUE);
    sb->bagContent.val = PGPNewData (mem, sb->bagContent.len, 0);
    asnerr = 0;
    PKIPackCertBag (pki,
		    sb->bagContent.val,
		    sb->bagContent.len,
		    certBag,
		    &asnerr);
    if (asnerr)
	{
	    err = kPGPError_ASNPackFailure;
	    goto error;
	}
    err = kPGPError_NoErr;
 error:
    if (oct)
	PKIFreeOCTET_STRING (pki, oct);
    if (certBag)
	PKIFreeCertBag (pki, certBag);
    return err;
}

PGPError
PKCS12ExportKey (
    PGPContextRef	context,	/* [IN] PGP context */
    const PGPByte	*privKeyInfo,	/* [IN] DER of PKCS-8 PrivateKeyInfo */
    PGPSize		privKeyInfoSize,/* [IN] size in bytes of privKeyInfo */
    const PGPByte	*password,	/* [IN] password to use for
					   MAC/symmetric encryption */
    PGPSize		passwordSize,	/* [IN] length in bytes of password */
    const PGPByte	*certificate,	/* [IN] DER of X.509 Certificate */
    PGPSize		certificateSize,/* [IN] size in bytes of X.509 cert */
    int			macIterations,	/* [IN] number of iterations in
					   generation of MAC key */
    int			pbeIterations,	/* [IN] number of iterations in
					   generation of PBE key */
    PGPPBEAlgorithm	pbeAlgorithm,	/* [IN] PBE algorithm to use */
    PGPByte		**pkcs12der,	/* [OUT] PKCS-12 DER */
    PGPSize		*pkcs12derSize)/* [OUT] size in bytes of PKCS-12 DER */
{
    PKIPFX *pfx = 0;
    PKICONTEXT pki;
    PKIAuthenticatedSafes *authSafes = 0;
    PKISafeContents *safeContents = 0;
    PKICertBag *certBag = 0;
    PKIOCTET_STRING *oct = 0;
    PGPByte *bmpPass = 0;
    PGPSize bmpPassSize;
    PGPMemoryMgrRef mem;
    PGPError err;
    int asnerr;

    *pkcs12der = 0;
    *pkcs12derSize = 0;
    
    memset (&pki, 0, sizeof (pki));
    pki.memMgr = &X509CMSMemoryMgr;
    mem = pki.memMgr->customValue = PGPGetContextMemoryMgr (context);

    /*----- begin creation of PKCS8-Shrouded-KeyBag ----- */    
    safeContents = PKINewSafeContents (&pki);
    safeContents->n = 1;
    safeContents->elt[0] = PKINewSafeBag (&pki);
    PKIPutOctVal (&pki, &safeContents->elt[0]->bagType,
		  PKIpkcs_8ShroudedKeyBag_OID,
		  PKIpkcs_8ShroudedKeyBag_OID_LEN);

    /* convert plaintext password into BMPString (2 byte per char) */
    pbe12ASCIIToBMPString (&pki, password, passwordSize, &bmpPass,
			   &bmpPassSize);

    err = pbe12ShroudKey (context, &pki, pbeAlgorithm, bmpPass, bmpPassSize,
			  pbeIterations, privKeyInfo, privKeyInfoSize,
			  &safeContents->elt[0]->bagContent.val,
			  &safeContents->elt[0]->bagContent.len);
    if (IsPGPError (err))
	goto error;
    
    /*----- if a certificate is specified, add it to the safeContents -----*/
    if (certificate)
	{
	    safeContents->n++;
	    err = pkcs12CreateCertBag (context, &pki, certificate,
				       certificateSize,
				       &safeContents->elt[safeContents->n - 1]);
	    if (IsPGPError (err))
		goto error;
	}
    
    /*---- pack the safeContents into the authSafe ----- */
    authSafes = PKINewAuthenticatedSafes (&pki);

    authSafes->n = 1;
    authSafes->elt[0] = PKINewContentInfo (&pki);
    PKIPutOctVal (&pki, &authSafes->elt[0]->contentType,
		  PKIdata_OID, PKIdata_OID_LEN);

    authSafes->elt[0]->content = PKINewANY (&pki);
    
    oct = PKINewOCTET_STRING (&pki);
    oct->len = PKISizeofSafeContents (&pki, safeContents, PKITRUE);
    oct->val = PGPNewData (mem, oct->len, 0);
    asnerr = 0;
    PKIPackSafeContents (&pki, oct->val, oct->len, safeContents, &asnerr);
    if (asnerr)
      {
	err = kPGPError_ASNPackFailure;
	goto error;
      }

    authSafes->elt[0]->content->len = PKISizeofOCTET_STRING (&pki, oct, PKITRUE);
    authSafes->elt[0]->content->val = PGPNewData (mem, authSafes->elt[0]->content->len, 0);
    asnerr = 0;
    PKIPackOCTET_STRING (&pki, authSafes->elt[0]->content->val,
	authSafes->elt[0]->content->len, oct, &asnerr);
    if (asnerr)
	{
	    err = kPGPError_ASNPackFailure;
	    goto error;
	}
    PKIFreeOCTET_STRING (&pki, oct);
    oct = 0;

    /*----- create the PFX structure -----*/
    pfx = PKINewPFX (&pki);
    PKIPutIntVal (&pki, &pfx->version, 3); /* our PKCS-12 draft is V3 */

    PKIPutOctVal (&pki, &pfx->authSafes.contentType,
		  PKIdata_OID, PKIdata_OID_LEN);
    pfx->authSafes.content = PKINewANY (&pki);
    
    /*----- pack the authSafes into the PFX ----- */
    oct = PKINewOCTET_STRING (&pki);

    oct->len = PKISizeofAuthenticatedSafes (&pki, authSafes, PKITRUE);
    oct->val = PGPNewData (mem, oct->len, 0);
    asnerr = 0;
    PKIPackAuthenticatedSafes (&pki, oct->val, oct->len, authSafes, &asnerr);
    if (asnerr)
	{
	    err = kPGPError_ASNPackFailure;
	    goto error;
	}
    
    pfx->authSafes.content->len = PKISizeofOCTET_STRING (&pki, oct, PKITRUE);
    pfx->authSafes.content->val = PGPNewData (mem, pfx->authSafes.content->len, 0);
    asnerr = 0;
    PKIPackOCTET_STRING (&pki, pfx->authSafes.content->val,
	pfx->authSafes.content->len, oct, &asnerr);
    if (asnerr)
      {
	err = kPGPError_ASNPackFailure;
	goto error;
      }

    /*----- generate the MAC over the authSafes portion -----*/
    err = pkcs12GenerateMAC (context,
			     &pki,
			     pfx,
			     oct,
			     macIterations,
			     bmpPass,
			     bmpPassSize,
			     kPGPHashAlgorithm_SHA);
    if (err)
	goto error;
    
    /*----- pack up the PFX ----- */
    *pkcs12derSize = PKISizeofPFX (&pki, pfx, PKITRUE);
    *pkcs12der = PGPNewData (mem, *pkcs12derSize, 0);
    asnerr = 0;
    PKIPackPFX (&pki, *pkcs12der, *pkcs12derSize,  pfx, &asnerr);

    if (asnerr)
	{
	    err = kPGPError_ASNPackFailure;
	    goto error;
	}

    err = kPGPError_NoErr;
 error:
    if (IsPGPError (err))
	{
	    if (*pkcs12der)
		{
		    PGPFreeData (*pkcs12der);
		    *pkcs12der = 0;
		}
	    *pkcs12derSize = 0;
	}
    
    if (pfx)
	PKIFreePFX (&pki, pfx);
    if (authSafes)
	PKIFreeAuthenticatedSafes (&pki, authSafes);
    if (certBag)
	PKIFreeCertBag (&pki, certBag);
    if (safeContents)
	PKIFreeSafeContents (&pki, safeContents);
    if (bmpPass)
	PGPFreeData (bmpPass);
    if (oct)
	PKIFreeOCTET_STRING (&pki, oct);
    return err;
}

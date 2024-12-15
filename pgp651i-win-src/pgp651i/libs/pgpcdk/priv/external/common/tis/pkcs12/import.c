/* Copyright (C) 1998-9 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: April 9, 1999 */

/* routines for importing a PKCS-12 */

#include "pbe12.h"
#include "pgpSymmetricCipherPriv.h" /* For RC2 ciphers */

#define kPGPMACSize 20	/* pkcs#12 requires 160-bit MAC keys */

/* compute HMAC-SHA */
/* If it doesn't verify, assume it's because of a bad passphrase */
static PGPError
pkcs12VerifyMAC (
    PKICONTEXT		*context,
    PKIPFX		*pfx,
    const unsigned char	*password,
    size_t		passwordLen)
{
    unsigned char	*mackey=NULL;
    unsigned char	mac[kPGPMACSize];
    size_t		mackeylen = kPGPMACSize;
    int			asnerror=0;

    PKIOCTET_STRING *oct=NULL;
    PGPError err=kPGPError_LazyProgrammer;
    PGPHMACContextRef hmacref;

    if(!context||!pfx)
	return err;
    if(!pfx->macData)
	return kPGPError_NoErr;/*nothing to do*/

#ifdef LOUD
    pbe12DumpBytes("HMAC salt:", pfx->macData->macSalt.val,pfx->macData->macSalt.len);
#endif
    pbe12GenerateRandomBits(context->customValue,
	kPGPHashAlgorithm_SHA,
	kPGPPBEIDMACKey,
	pfx->macData->macIterationCount ? PKIGetIntVal(context,pfx->macData->macIterationCount,&asnerror) : 1,
	pfx->macData->macSalt.val,
	pfx->macData->macSalt.len,
	password,
	passwordLen,
	mackeylen,
	&mackey);

    PKIUnpackOCTET_STRING(context,&oct,pfx->authSafes.content->val,pfx->authSafes.content->len,&asnerror);
    if(asnerror)
	goto error;

    err = PGPNewHMACContext(context->memMgr->customValue,
	kPGPHashAlgorithm_SHA,
	mackey,
	mackeylen,
	&hmacref);
    if (IsPGPError(err))
	goto error;
    err = PGPContinueHMAC(hmacref,oct->val,oct->len);
    if (IsPGPError(err))
	goto error;
    err = PGPFinalizeHMAC(hmacref,mac);
    if (IsPGPError(err))
	goto error;
    err = PGPFreeHMACContext(hmacref);
    if (IsPGPError(err))
	goto error;
    if (pfx->macData->mac.digest.len != kPGPMACSize ||
	memcmp(mac,pfx->macData->mac.digest.val,kPGPMACSize))
    {
	err = kPGPError_BadPassphrase;
	goto error;
    }

    err=kPGPError_NoErr;

error:
    if (mackey)
	PKIFree(context->memMgr,mackey);
    if (oct)
	PKIFreeOCTET_STRING(context,oct);

    return err;
}

static PGPPBEAlgorithm
pkcs12PBEAlgorithm (const unsigned char *oid, size_t oidsize)
{
    if (oidsize == PKIpbeWithSHA1And3_KeyTripleDES_CBC_OID_LEN &&
	!memcmp (oid,PKIpbeWithSHA1And3_KeyTripleDES_CBC_OID,oidsize))
	return kPGPPBEAlgorithm_SHA3DES;
    if (oidsize == PKIpbeWithSHA1And2_KeyTripleDES_CBC_OID_LEN &&
	!memcmp (oid,PKIpbeWithSHA1And2_KeyTripleDES_CBC_OID,oidsize))
	return kPGPPBEAlgorithm_SHADESEDE2;
    if (oidsize == PKIpbeWithSHA1And128BitRC4_OID_LEN &&
	!memcmp (oid, PKIpbeWithSHA1And128BitRC4_OID, oidsize))
	return kPGPPBEAlgorithm_SHARC4_128;
    if (oidsize == PKIpbeWithSHA1And40BitRC4_OID_LEN &&
	!memcmp (oid, PKIpbeWithSHA1And40BitRC4_OID, oidsize))
	return kPGPPBEAlgorithm_SHARC4_40;
    if (oidsize == PKIpbeWithSHA1And128BitRC2_CBC_OID_LEN &&
	!memcmp (oid, PKIpbeWithSHA1And128BitRC2_CBC_OID, oidsize))
	return kPGPPBEAlgorithm_SHARC2_128;
    if (oidsize == PKIpbeWithSHA1And40BitRC2_CBC_OID_LEN &&
	!memcmp (oid, PKIpbeWithSHA1And40BitRC2_CBC_OID, oidsize))
	return kPGPPBEAlgorithm_SHARC2_40;
    return kPGPPBEAlgorithm_Unknown;
}

typedef enum {
    kPGPBagType_Unknown,
    kPGPBagType_KeyBag,
    kPGPBagType_PKCS8ShroudedKeyBag,
    kPGPBagType_CertBag,
    kPGPBagType_CRLBag,
    kPGPBagType_SecretBag,
    kPGPBagType_SafeContentsBag
} PGPBagType;

static PGPBagType
pkcs12BagType (
    const unsigned char	*oid,
    size_t				oidsize)
{
    if(oidsize==PKIkeyBag_OID_LEN && !memcmp(PKIkeyBag_OID,oid,oidsize))
	return kPGPBagType_KeyBag;
    if(oidsize==PKIpkcs_8ShroudedKeyBag_OID_LEN && !memcmp(PKIpkcs_8ShroudedKeyBag_OID,oid,oidsize))
	return kPGPBagType_PKCS8ShroudedKeyBag;
    if(oidsize==PKIcertBag_OID_LEN && !memcmp(PKIcertBag_OID,oid,oidsize))
	return kPGPBagType_CertBag;
    if(oidsize==PKIcrlBag_OID_LEN && !memcmp(PKIcrlBag_OID,oid,oidsize))
	return kPGPBagType_CRLBag;
    if(oidsize==PKIsecretBag_OID_LEN && !memcmp(PKIsecretBag_OID,oid,oidsize))
	return kPGPBagType_SecretBag;
    if(oidsize==PKIsafeContentsBag_OID_LEN && !memcmp(PKIsafeContentsBag_OID,oid,oidsize))
	return kPGPBagType_SafeContentsBag;
    return kPGPBagType_Unknown;
}

#if 0
static unsigned char pbe12OddParity[256] = {
  1,   1,   2,   2,   4,   4,   7,   7,   8,   8,  11,  11,  13,  13,  14,  14, 
 16,  16,  19,  19,  21,  21,  22,  22,  25,  25,  26,  26,  28,  28,  31,  31, 
 32,  32,  35,  35,  37,  37,  38,  38,  41,  41,  42,  42,  44,  44,  47,  47, 
 49,  49,  50,  50,  52,  52,  55,  55,  56,  56,  59,  59,  61,  61,  62,  62, 
 64,  64,  67,  67,  69,  69,  70,  70,  73,  73,  74,  74,  76,  76,  79,  79, 
 81,  81,  82,  82,  84,  84,  87,  87,  88,  88,  91,  91,  93,  93,  94,  94, 
 97,  97,  98,  98, 100, 100, 103, 103, 104, 104, 107, 107, 109, 109, 110, 110, 
112, 112, 115, 115, 117, 117, 118, 118, 121, 121, 122, 122, 124, 124, 127, 127, 
128, 128, 131, 131, 133, 133, 134, 134, 137, 137, 138, 138, 140, 140, 143, 143, 
145, 145, 146, 146, 148, 148, 151, 151, 152, 152, 155, 155, 157, 157, 158, 158, 
161, 161, 162, 162, 164, 164, 167, 167, 168, 168, 171, 171, 173, 173, 174, 174, 
176, 176, 179, 179, 181, 181, 182, 182, 185, 185, 186, 186, 188, 188, 191, 191, 
193, 193, 194, 194, 196, 196, 199, 199, 200, 200, 203, 203, 205, 205, 206, 206, 
208, 208, 211, 211, 213, 213, 214, 214, 217, 217, 218, 218, 220, 220, 223, 223, 
224, 224, 227, 227, 229, 229, 230, 230, 233, 233, 234, 234, 236, 236, 239, 239, 
241, 241, 242, 242, 244, 244, 247, 247, 248, 248, 251, 251, 253, 253, 254, 254
};

static void pbe12SetOddParity (PGPByte *b)
{
    int i;

    for (i = 0; i < 24; i++)
	b[i] = pbe12OddParity[b[i]];
}
#endif

/*
 * Do a PKCS-12 style decryption of a buffer into a dynamically allocated
 * buffer
 */
static PGPError
pkcs12Decrypt(
    PGPContextRef		context,	/* [IN] */
    const PGPByte		*pass,		/* [IN] */
    PGPSize			passsize,	/* [IN] */
    const PGPByte		*encdata,	/* [IN] data to decrypt */
    PGPSize			encdatasize,	/* [IN] size of data to decrypt
						   (bytes) */
    PKIEncryptionAlgorithmIdentifier *alginfo,	/* [IN] alg info */
    PKICONTEXT			*pki,		/* [IN] asn.1 compiler
						   context */
    PGPByte			**outbuf,	/* [OUT] outptu buffer */
    PGPSize			*outbufsize)	/* [OUT] output size */
{
    PKIpkcs_12PbeParams		*param = NULL;
    int				asnerr = 0, itercount;
    PGPPBEAlgorithm		alg;
    PGPError			err;
    PGPByte			*symkey = NULL, *iv = NULL, pad;
    PGPSize			keysize, blocksize, i;
    PGPHashAlgorithm		hashalg;
    PGPCipherAlgorithm		encalg;
    PGPCBCContextRef		cbcref = NULL;
    PGPSymmetricCipherContextRef	symref = NULL;
    PGPMemoryMgrRef		mem = PGPGetContextMemoryMgr (context);


    /* determine what algorithm was used to encrypt */
    alg = pkcs12PBEAlgorithm (alginfo->algorithm.val,alginfo->algorithm.len);
    switch (alg)
    {
	case kPGPPBEAlgorithm_SHA3DES:
	    hashalg = kPGPHashAlgorithm_SHA;
	    encalg = kPGPCipherAlgorithm_3DES;
	    keysize = 24;
	    break;
	case kPGPPBEAlgorithm_SHARC2_40:
	    hashalg = kPGPHashAlgorithm_SHA;
	    encalg = (PGPCipherAlgorithm)kPGPCipherAlgorithm_RC2_40;
	    keysize = 40/8;
	    break;
        case kPGPPBEAlgorithm_SHARC2_128:
	    hashalg = kPGPHashAlgorithm_SHA;
	    encalg = (PGPCipherAlgorithm)kPGPCipherAlgorithm_RC2_128;
	    keysize = 128/8;
	    break;
	default:
	    err = kPGPError_BadHashNumber;
	    goto error;
    }

    /* set up encryption context */
    err = PGPNewSymmetricCipherContext (mem, encalg, keysize, &symref);
    if (IsPGPError (err))
	goto error;
    /* determine key and block sizes */
    err = PGPGetSymmetricCipherSizes (symref, &keysize, &blocksize);
    if (IsPGPError (err))
	goto error;

    /* decode pbe parameters */
    /* this assumes that we aren't using any of the PKCS-5 modes */
    PKIUnpackpkcs_12PbeParams (pki, &param, alginfo->parameters->val,
			       alginfo->parameters->len, &asnerr);
    if (asnerr)
    {
	err = kPGPError_LazyProgrammer;
	goto error;
    }

    itercount = PKIGetIntVal (pki, &param->iterationCount, &asnerr);
    if (asnerr)
    {
	err = kPGPError_LazyProgrammer;
	goto error;
    }

    /* generate the symmetric cipher key */
    err = pbe12GenerateRandomBits (context,
	hashalg,
	kPGPPBEIDEncryptKey,
	itercount,
	param->salt.val,
	param->salt.len,
	pass,
	passsize,
	keysize,
	&symkey);
    if (IsPGPError (err))
	goto error;

    /* generate the IV */
    err = pbe12GenerateRandomBits (context,
	hashalg,
	kPGPPBEIDIV,
	itercount,
	param->salt.val,
	param->salt.len,
	pass,
	passsize,
	blocksize,
	&iv);
    if (IsPGPError (err))
	goto error;

    err = PGPNewCBCContext (symref, &cbcref);
    if (IsPGPError (err))
	goto error;
    symref = NULL; /* dont free this, PGPFreeCBCContext does it */
    err = PGPInitCBC (cbcref, symkey, iv);
    if (IsPGPError (err))
	goto error;
    *outbufsize = encdatasize;
    *outbuf = PGPNewData (mem, *outbufsize, 0);
    err = PGPCBCDecrypt (cbcref, encdata, encdatasize, *outbuf);
    if (IsPGPError (err))
	goto error;
    /* remove padding */
    pad = (*outbuf)[*outbufsize - 1];
    if (pad < 1 || pad > blocksize)
    {
	/* invalid pad size, decryption failed */
	err = kPGPError_LazyProgrammer;
	goto error;
    }
    *outbufsize -= pad;
    /* ensure that we have a valid pad */
    for (i = 0; i < pad; i++)
	if ((*outbuf)[*outbufsize + i] != pad)
	{
	    /* invalid pad, decryption failed */
	    err = kPGPError_LazyProgrammer;
	    goto error;
	}

    err = kPGPError_NoErr;

error:
    if (symref)
	err = PGPFreeSymmetricCipherContext (symref);
    if (cbcref)
	err = PGPFreeCBCContext (cbcref);
    if (param)
	PKIFreepkcs_12PbeParams (pki, param);
    if (symkey)
	PGPFreeData (symkey);
    if (iv)
	PGPFreeData (iv);
    return err;
}


/* processes a PKCS #8 EncryptedPrivateKeyInfo ASN.1 blob */
static PGPError
pkcs8Decrypt (
    PGPContextRef		context,	/* [IN] */
    const PGPByte		*pass,		/* [IN] */
    PGPSize			passsize,	/* [IN] */
    const PGPByte		*encdata,	/* [IN] data to decrypt */
    PGPSize			encdatasize,	/* [IN] size of data to decrypt
						   (bytes) */
    PKICONTEXT			*pki,		/* [IN] asn.1 compiler
						   context */
    PGPByte			**privkey,	/* [OUT] PrivateKeyInfo ASN.1 */
    PGPSize			*privkeysize)	/* [OUT] asn.1 binary size */
{
    int				asnerr = 0;
    PKIEncryptedPrivateKeyInfo	*pkcs8;
#if 0
    PKIPrivateKeyInfo		*privkeyinfo;
#endif
    PGPError			err;

    PKIUnpackEncryptedPrivateKeyInfo (pki, &pkcs8, encdata, encdatasize,&asnerr);
    if (asnerr)
	return kPGPError_NoErr; /* can't process this */

    err = pkcs12Decrypt(context, pass, passsize,
			pkcs8->encryptedData.val, pkcs8->encryptedData.len,
			&pkcs8->encryptionAlgorithm, pki,
			privkey, privkeysize);

    if (IsPGPError (err))
	goto error;

#if 0
    /* test the result of the decryption */
    PKIUnpackPrivateKeyInfo (pki, &privkeyinfo, *privkey, *privkeysize, &asnerr);
    if (asnerr)
    {
	err = kPGPError_LazyProgrammer;
	goto error;
    }
    PKIFreePrivateKeyInfo (pki, privkeyinfo);
#endif

    err = kPGPError_NoErr;

error:
    if (pkcs8)
	PKIFreeEncryptedPrivateKeyInfo(pki,pkcs8);
    return err;
}


static PGPError
pkcs12DecryptData (
    PGPContextRef	context,
    PKICONTEXT		*pki,
    const PGPByte	*data,
    PGPSize		datasize,
    const PGPByte	*pass,
    PGPSize		passsize,
    PGPByte		**plain,	/* [OUT] */
    PGPSize		*plainsize)	/* [OUT] */
{
    PKIEncryptedData	*enc;
    PKIEncryptedContentInfo *cinfo;
    int			asnerror=0;
    PGPError		err=kPGPError_NoErr;

    /* clear outputs */
    *plain = NULL;
    *plainsize = 0;

    PKIUnpackEncryptedData (pki,&enc,(PGPByte *) data,datasize,&asnerror);
    if (asnerror)
	return kPGPError_LazyProgrammer;

    cinfo = &enc->encryptedContentInfo;

    err = pkcs12Decrypt(context, pass, passsize,
			cinfo->encryptedContent->val,
			cinfo->encryptedContent->len,
			&cinfo->contentEncryptionAlgorithm, pki,
			plain, plainsize);

    if (IsPGPError (err))
	goto error;

    err = kPGPError_NoErr;

error:

    if (enc)
	PKIFreeEncryptedData(pki,enc);
    return err;
}




/*
* EXPORTED FUNCTIONS
*/

PGPError
PKCS12InputKey (
    PGPContextRef	context,	/* [IN] pgp context */
    const PGPByte	*der,		/* [IN] pkcs #12 data */
    PGPSize		dersize,	/* [IN] pkcs #12 data size */
    const PGPByte	*pass,		/* [IN] MAC/decrypt password */
    PGPSize		passlen,	/* [IN] password length */
    PGPByte		**privkey,	/* [OUT] ASN.1 PrivateKeyInfo */
    PGPSize		*privkeysize,	/* [OUT] size of PrivateKeyInfo */
    PGPByte		**cert		/* [OUT] certset from pkcs12 bags */,
    PGPSize		*certSize)
{
    PKIPFX			*pfx;
    PKICONTEXT			pki;
    PGPError			err;
    int				asnerr = 0;
    PKIOCTET_STRING		*oct = NULL;
    int				i, j;
    unsigned char		*bmppass = NULL;
    size_t			bmppasslen = 0;
    PKISafeContents		*safe = NULL;
    PKIAuthenticatedSafes	*authsafes = NULL;
    PKIExtendedCertificatesAndCertificates	*certSet;
    PGPMemoryMgrRef		mem = PGPGetContextMemoryMgr (context);

    /* clear outputs */
    *privkey = NULL;
    *privkeysize = 0;
    *cert = 0;
    *certSize = 0;

    /* initialize ASN.1 compiler */
    memset(&pki,0,sizeof pki);
    pki.customValue=context;
    pki.memMgr = &X509CMSMemoryMgr;
    pki.memMgr->customValue = PGPGetContextMemoryMgr (context);
    
    certSet = PKINewExtendedCertificatesAndCertificates (&pki);

    /* convert ascii password to bmpstring format */
    pbe12ASCIIToBMPString(&pki, pass, passlen, &bmppass, &bmppasslen);
#ifdef LOUD
    printf("Password: '%s'\n\n", pass);
    pbe12DumpBytes("BMPString Password", bmppass, bmppasslen);
    puts("\n===== HMAC Verification =====");
#endif

    /* parse high level asn.1 structure */
    PKIUnpackPFX(&pki,&pfx,der,dersize,&asnerr);
    if(asnerr)
	return kPGPError_LazyProgrammer;
    
    err=pkcs12VerifyMAC(&pki,pfx,bmppass,bmppasslen);
    if (IsPGPError(err))
	goto error;
#ifdef LOUD
    puts("\n===== END HMAC Verification =====");
#endif
    PKIUnpackOCTET_STRING(&pki,&oct,pfx->authSafes.content->val,pfx->authSafes.content->len,&asnerr);
    if(asnerr)
    {
	err=kPGPError_LazyProgrammer;
	goto error;
    }
    PKIUnpackAuthenticatedSafes(&pki,&authsafes,oct->val,oct->len,&asnerr);
    if (asnerr)
    {
	err = kPGPError_LazyProgrammer;
	goto error;
    }
    PKIFreeOCTET_STRING (&pki, oct);
    oct = NULL;
    for (i = 0; i < authsafes->n; i++)
    {
	int		msgtype = sm_MessageType(authsafes->elt[i],&pki);
	unsigned char	*input;
	size_t		inputsize;

	/* TODO: this should really be a loop since we can have nested
	   protections.  hopefully nobody will actually do this... */
	if (msgtype == PKCS7_CONTENT_DATA)
	{
	    /* unencrypted data */
	    input = authsafes->elt[i]->content->val;
	    inputsize = authsafes->elt[i]->content->len;
	    PKIUnpackOCTET_STRING (&pki,&oct,input,inputsize,&asnerr);
	    if (asnerr)
	    {
		err = kPGPError_LazyProgrammer;
		goto error;
	    }
	    input = oct->val;
	    inputsize = oct->len;
	    oct->val = 0;
	    oct->len = 0;
	    PKIFreeOCTET_STRING (&pki,oct);
	}
	else if (msgtype == PKCS7_CONTENT_ENCRYPTED_DATA)
	{
	    /* password-encrypted data */
	    pkcs12DecryptData (context, &pki,
		authsafes->elt[i]->content->val,
		authsafes->elt[i]->content->len,
		bmppass,
		bmppasslen,
		&input,
		&inputsize);
	}
	else
	{
	    err = kPGPError_InvalidPKCS7Encoding;
	    goto error;
	}
	PKIUnpackSafeContents (&pki,&safe,input,inputsize,&asnerr);
	PKIFree (pki.memMgr, input);
	if (asnerr)
	{
	    err = kPGPError_LazyProgrammer;
	    goto error;
	}

	/* Microsoft Internet Explorer puts "our" cert last */
	for (j=0; j<safe->n; j++)
	{
	    PGPBagType bagid = pkcs12BagType (safe->elt[j]->bagType.val,safe->elt[j]->bagType.len);

	    if (bagid == kPGPBagType_CertBag)
	    {
		PKICertBag *certBag;

		asnerr = 0;
		PKIUnpackCertBag (&pki,
		    &certBag,
		    safe->elt[j]->bagContent.val,
		    safe->elt[j]->bagContent.len,
		    &asnerr);
		/* TODO: should we bail out here on error? */
		if (certBag)
		  {
		    if (certBag->certType.len == PKIx509Certificate_OID_LEN &&
			memcmp (certBag->certType.val,
			    PKIx509Certificate_OID,
			    PKIx509Certificate_OID_LEN) == 0)
		      {
			 PKIOCTET_STRING *certoct;
			 PKICertificate *bagcert;

			 asnerr = 0;
			 PKIUnpackOCTET_STRING (&pki, &certoct,
			     certBag->cert.val,
			     certBag->cert.len,
			     &asnerr);
			 if (certoct)
			   {
			     PKIUnpackCertificate (&pki, &bagcert,
						     certoct->val,
						     certoct->len,
						     &asnerr);
			     if (bagcert) {
				 PKIExtendedCertificateOrCertificate *extcert;
				 extcert = PKINewExtendedCertificateOrCertificate (&pki);
				 if( extcert ) {
				     certSet->elt[certSet->n++] = extcert;
				     extcert->data = bagcert;
				     extcert->CHOICE_field_type = PKIID_Certificate;
				 }
			     }
			     PKIFreeOCTET_STRING (&pki, certoct);
			   }
		       }
		    PKIFreeCertBag (&pki, certBag);
		  }
	    }
	    else if (bagid == kPGPBagType_PKCS8ShroudedKeyBag)
	      {
		err = pkcs8Decrypt (context,
		    bmppass,
		    bmppasslen,
		    safe->elt[j]->bagContent.val,
		    safe->elt[j]->bagContent.len,
		    &pki,
		    privkey,
		    privkeysize);
		/* TODO: bail out on error? */
	      }
	    else if (bagid == kPGPBagType_KeyBag)
	      {
		  /* Steal data for caller */
		  *privkey = safe->elt[j]->bagContent.val;
		  *privkeysize = safe->elt[j]->bagContent.len;
		  safe->elt[j]->bagContent.val = 0;
		  safe->elt[j]->bagContent.len = 0;
	      }
	}
	PKIFreeSafeContents (&pki,safe);
	safe = NULL;
    }

    if (certSet->n)
    {
	    *certSize = PKISizeofExtendedCertificatesAndCertificates (
		    &pki,
		    certSet,
		    1);
	    *cert = PGPNewData (mem, *certSize, 0);
	    PKIPackExtendedCertificatesAndCertificates (&pki,
		    *cert,
		    *certSize,
		    certSet,
		    &asnerr);
	    if (asnerr)
	    {
		    PGPFreeData (*cert);
		    *cert = NULL;
		    *certSize = 0;
		    err = kPGPError_ASNPackFailure;
		    goto error;
	    }
    }


    err = kPGPError_NoErr;

error:
    if (IsPGPError (err))
      {
	if (*cert)
	  {
	    PKIFree (pki.memMgr, *cert);
	    *cert = 0;
	  }
	*certSize = 0;
	if (*privkey)
	  {
	    PKIFree (pki.memMgr, *privkey);
	    *privkey = 0;
	  }
	*privkeysize = 0;
      }

    if (safe)
	PKIFreeSafeContents (&pki,safe);
    if (authsafes)
	PKIFreeAuthenticatedSafes (&pki,authsafes);
    if (pfx)
	PKIFreePFX (&pki,pfx);
    if (bmppass)
	PKIFree (pki.memMgr,bmppass);
    PKIFreeExtendedCertificatesAndCertificates (&pki, certSet);
    return err;
}
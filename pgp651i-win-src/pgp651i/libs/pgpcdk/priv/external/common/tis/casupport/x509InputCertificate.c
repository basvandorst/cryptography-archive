/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: January 15, 1999 */

#include "x509CMS.h"
#include "pgpX509Priv.h"
#include "pkcsreq_asn.h"

static void
x509VerifyPKCS7Signatures (
	PGPContextRef	context,
	PGPKeySetRef	keyset,		/* [IN] where to find keys */
	PKICONTEXT	*asnContext,	/* [IN] */
	PKISignedData	*sigData,	/* [IN] */
	PGPBoolean	*sigPresent,	/* [OUT] */
	PGPBoolean	*sigChecked,	/* [OUT] */
	PGPBoolean	*sigVerified,	/* [OUT] */
	PGPKeyRef	*signKey	/* [OUT] */
)
{
    X509CMSCallbackData	pgpData;
    int			p7err;
    int			asnError = 0;
    PGPError		err;
    PGPFilterRef	filter = NULL;
    PGPKeySetRef	targetKeySet = NULL;
    PGPKeyListRef	keyList = NULL;
    PGPKeyIterRef	keyIter = NULL;
    PGPKeyRef		keyRef = NULL;
    PGPByte		*isnData = NULL;
    PGPSize		isnDataLen;
    PKICertificate	*cert = NULL;
    PGPMemoryMgrRef	mem = PGPGetContextMemoryMgr (context);

	if (!sigData || sigData->signerInfos.n < 1 || !sigData->signerInfos.elt[0])
		return; /* nothing to check */

	*sigPresent = 1;

	if (!keyset)
		return; /* Can't check without keys to check with */

	/* find certificate for this user */
	/* use the Issuer And Serial Number as a filter */
	isnDataLen = PKISizeofIssuerAndSerialNumber (asnContext,
		&sigData->signerInfos.elt[0]->issuerAndSerialNumber,
		1);
	isnData = PGPNewData (mem, isnDataLen, 0);
	PKIPackIssuerAndSerialNumber (asnContext,
		isnData,
		isnDataLen,
		&sigData->signerInfos.elt[0]->issuerAndSerialNumber,
		&asnError);
	if (asnError)
		goto error_exit;

	err = PGPNewSigPropertyBufferFilter( context, kPGPSigPropX509IASN, 
				isnData, isnDataLen, kPGPMatchEqual, &filter );
	if (IsPGPError (err))
		goto error_exit;
	err = PGPFilterKeySet (keyset, filter, &targetKeySet);
	if (IsPGPError (err))
		goto error_exit;
	err = PGPOrderKeySet (targetKeySet,
		kPGPAnyOrdering,
		&keyList);
	if (IsPGPError (err))
		goto error_exit;
	err = PGPNewKeyIter (keyList, &keyIter);
	if (IsPGPError (err))
		goto error_exit;
	err = PGPKeyIterNext (keyIter, &keyRef);
	if (IsPGPError (err))
		goto error_exit;

	*signKey = keyRef;

	err = x509ExtractCertificate (context,
		keyRef,
		asnContext,
		&cert);
	if (IsPGPError (err))
		goto error_exit;

	*sigChecked = 1;

	/* set up callback data */
	memset (&pgpData, 0, sizeof (pgpData));
	pgpData.context = context;
	pgpData.key = keyRef;

	/* verify signature */
	p7err = sm_VerifyMessage (NULL,	/* dont return the inner data */
		0,
		sigData,
		cert,
		pkcs7HashCallback,
		(void *) &pgpData,
		pkcs7VerifyCallback,
		(void *) &pgpData,
		asnContext);
	if (p7err == PKCS7_OK)
		*sigVerified = 1;

error_exit:

	if (keyIter)
		PGPFreeKeyIter (keyIter);
	if (keyList)
		PGPFreeKeyList (keyList);
	if (targetKeySet)
		PGPFreeKeySet (targetKeySet);
	if (filter)
		PGPFreeFilter (filter);
	if (cert)
		PKIFreeCertificate (asnContext, cert);
	if (isnData)
		PGPFreeData (isnData);
}

static void
x509ExtractCertificatesFromSignedData (
	PKISignedData				*sigData,
	PKIExtendedCertificatesAndCertificates	*certSet)
{
	int i;

	if (sigData->certificates)
	{
		for (i = 0; i < sigData->certificates->n; i++)
		{
			certSet->elt[certSet->n++] = sigData->certificates->elt[i];
			sigData->certificates->elt[i] = NULL;
		}
		/* don't free this data when free'ing sigData */
		sigData->certificates->n = 0;
		PGPFreeData (sigData->certificates);
		sigData->certificates = NULL;
	}
}

static void
x509ExtractCRLsFromSignedData (
	PKISignedData			*sig,
	PKICertificateRevocationLists	*crls)
{
	int i;

	if (sig->crls)
	{
		for (i = 0; i < sig->crls->n; i++)
		{
			crls->elt[crls->n++] = sig->crls->elt[i];
			sig->crls->elt[i] = NULL;
		}
		sig->crls->n = 0;
		PGPFreeData (sig->crls);
		sig->crls = NULL;
	}
}

static PGPError
x509ParseAsCertificate(
	PKICONTEXT *asnContext,
	/*const*/ PGPByte *input,
	PGPSize inputSize,
	PKIExtendedCertificatesAndCertificates *certSet)
{
	/* first check to see if this is a vanilla X.509 certificate */
	int asnError = 0;
	PKICertificate *cert;
	PKIUnpackCertificate (asnContext, &cert, input, inputSize, &asnError);
	if(asnError)
		return kPGPError_LazyProgrammer;
	certSet->n = 1;
	certSet->elt[0] = PKINewExtendedCertificateOrCertificate (asnContext);
	certSet->elt[0]->CHOICE_field_type = PKIID_Certificate;
	certSet->elt[0]->data = (void *) cert; /* NOTE: don't free `cert' */
	return kPGPError_NoErr;
}

static PGPError
x509ParseAsCRL(
	PKICONTEXT *asnContext,
	/*const*/ PGPByte *input,
	PGPSize inputSize,
	PKICertificateRevocationLists *crls)
{
	/* check for x.509 crl */
	int asnError=0;
	PKICertificateList *crl;
	PKIUnpackCertificateList(asnContext,&crl,input,inputSize,&asnError);
	if(asnError)
		return kPGPError_LazyProgrammer;
	crls->n=1;
	crls->elt[0]=crl;/* don't free crl */
	return kPGPError_NoErr;
}

PGPError
X509InputCertificate (
	PGPContextRef	context,
	PGPByte		*input,		/* input buffer */
	PGPSize		inputSize,
	PGPKeySetRef	keyset,		/* where to find keys */
	PGPInputFormat	format,		/* Import format (CA) */
	PGPOptionListRef    passphrase,/* Passphrase for decrypting */
	PGPKeyRef	*decryptKey,	/* Output decryption key */
	PGPKeyRef	*signKey,	/* Output signing key */
	PGPBoolean	*sigPresent,	/* output flag for signed message */
	PGPBoolean	*sigChecked,	/* output that we have sig key */
	PGPBoolean	*sigVerified,	/* output flag for sig ok */
	PGPAttributeValue	**extraData,
	PGPUInt32	*extraDataCount,
	PGPByte		**certOut,
	PGPSize		*certOutSize,
	PGPByte		**crlOut,
	PGPSize		*crlOutSize
)
{
	/* The calling function expects to be returned SET OF Certificate, but
	   the PKCS-7 definition ExtendedCertificatesAndCertificates produces the
	   same encoding when no ExtendedCertificate's are present, so just use
	   that defintion to reduce code size */
	PKIExtendedCertificatesAndCertificates	*certSet = NULL;

	PKICONTEXT					asnContext;
	PKIContentInfo				*contentInfo = NULL;
	PKISignedData				*sigData = NULL;
	PKICertificateRevocationLists		*crls = NULL;
	PGPError					err;
	PGPMemoryMgrRef				mem = PGPGetContextMemoryMgr (context);
	PGPByte					*contentData = NULL;
	PGPSize					contentDataLen;
	int						messageType;
	int						asnError = 0;

	(void) keyset;	/* Unused argument */
	(void) passphrase;	/* Unused argument */
	(void) format;	/* Unused argument */

	/* clear outputs */
	*sigPresent = 0;
	*sigChecked = 0;
	*sigVerified = 0;
	*extraData = NULL;
	*extraDataCount = 0;
	*decryptKey = NULL;
	*signKey = NULL;
	*crlOut = NULL;
	*crlOutSize = 0;

	memset (&asnContext, 0, sizeof (PKICONTEXT));
	asnContext.memMgr = &X509CMSMemoryMgr;
	asnContext.memMgr->customValue = (void *) mem;

	certSet = PKINewExtendedCertificatesAndCertificates (&asnContext);
	crls = PKINewCertificateRevocationLists(&asnContext);

	/* first check to see if input is a plain x.509 certificate.  if not, check
	   to see if input is a plain CRL.  failing that, attempt to decode as
	   a PKCS-7 message */
	if (x509ParseAsCertificate(&asnContext,input,inputSize,certSet)!=kPGPError_NoErr &&
		x509ParseAsCRL(&asnContext,input,inputSize,crls)!=kPGPError_NoErr)
	{
		/* either this is not a certificate/crl or it has a bad encoding, so
		   try to decode it as a PKCS-7 message */

		contentInfo = sm_DecodeMessage (input, inputSize, &asnContext);
		if (!contentInfo)
		{
			/* failed to decode message */
			err = kPGPError_InvalidPKCS7Encoding;
			goto error_exit;
		}

		messageType = sm_MessageType (contentInfo, &asnContext);
		if (!contentInfo->content)
		{
			PKIFreeContentInfo (&asnContext, contentInfo);
			err = kPGPError_InvalidPKCS7Encoding;
			goto error_exit;
		}

		/* save the internal content */
		contentData = contentInfo->content->val;
		contentDataLen = contentInfo->content->len;

		contentInfo->content->val = NULL;
		contentInfo->content->len = 0;
		PGPFreeData (contentInfo->content);
		contentInfo->content = NULL;

		PKIFreeContentInfo (&asnContext, contentInfo);

		while (messageType == PKCS7_CONTENT_SIGNED_DATA)
		{
			sigData = sm_DecodeSignedData (contentData,
				contentDataLen,
				&asnContext);
			if (!sigData)
			{
				err = kPGPError_InvalidPKCS7Encoding; /* parse error */
				goto error_exit;
			}

			/* check for signature.  since the returned certificate is
			   itself signed, we don't exit upon failure during this
			   step */
			x509VerifyPKCS7Signatures (
				context,
				keyset,
				&asnContext,
				sigData,
				sigPresent,
				sigChecked,
				sigVerified,
				signKey);

			/* if there are certificates present, return them */
			x509ExtractCertificatesFromSignedData (sigData, certSet);

			/* if there are CRLs present, return them */
			x509ExtractCRLsFromSignedData (sigData, crls);

			PGPFreeData (contentData); /* free existing data */

			/* get the content data */
			sm_VerifyMessage (&contentData,
				&contentDataLen,
				sigData,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				&asnContext);

			/* peek at the internal content type */
			messageType = sm_ContentType (&sigData->contentInfo.contentType, &asnContext);

			PKIFreeSignedData (&asnContext, sigData);
			sigData = NULL;
		}

		if (messageType != PKCS7_CONTENT_DATA)
		{
			/* unknown PKCS7 content type */
			err = kPGPError_InvalidPKCS7Encoding;
			goto error_exit;
		}

		/* if we have something from Verisign, the certificates are located
		   in the body of the signed message */
		if (contentData)
		{
			PKICertRep *certRep;

			/* Try to parse with VeriSign structure (extra reginfo) */
			asnError = 0;
			PKIUnpackCertRep (&asnContext,
				&certRep,
				contentData,
				contentDataLen,
				&asnError);

			if (certRep) {
				/* Verisign parse succeeded, extract contentInfo portion */
				contentInfo = certRep->certs;
				certRep->certs = NULL;
				PKIFreeCertRep (&asnContext, certRep);
			} else {
				/* Try to parse as a plain CMS message */
				asnError = 0;
				contentInfo = sm_DecodeMessage (contentData, contentDataLen,
												&asnContext);
			}

			if (contentInfo)
			{
				messageType = sm_MessageType (contentInfo, &asnContext);
				if (messageType == PKCS7_CONTENT_SIGNED_DATA &&
					contentInfo->content)
				{
					PKIUnpackSignedData (&asnContext, &sigData,
										 contentInfo->content->val,
										 contentInfo->content->len,
										 &asnError);
					if (!asnError)
					{
						x509ExtractCertificatesFromSignedData (sigData, certSet);
						x509ExtractCRLsFromSignedData (sigData, crls);
						PKIFreeSignedData (&asnContext, sigData);
						sigData = NULL;
					}
				}
				PKIFreeContentInfo (&asnContext, contentInfo);
			}
		}
	}

	asnError = 0; /* reset error code */

	if (certSet->n)
	{
		*certOutSize = PKISizeofExtendedCertificatesAndCertificates (
			&asnContext,
			certSet,
			1);
		*certOut = PGPNewData (mem, *certOutSize, 0);
		asnError = 0;
		PKIPackExtendedCertificatesAndCertificates (&asnContext,
			*certOut,
			*certOutSize,
			certSet,
			&asnError);
		if (asnError)
		{
			PGPFreeData (*certOut);
			*certOut = NULL;
			*certOutSize = 0;
			err = kPGPError_ASNPackFailure;
			goto error_exit;
		}
	}

	if (crls->n)
	{
		/* if we found CRLs, pack them up now */
		*crlOutSize = PKISizeofCertificateRevocationLists (&asnContext, crls, 1);
		*crlOut = PGPNewData (mem, *crlOutSize, 0);
		PKIPackCertificateRevocationLists (&asnContext, *crlOut, *crlOutSize, crls, &asnError);
		if (asnError)
		{
			PGPFreeData (*crlOut);
			*crlOut = NULL;
			*crlOutSize = 0;

			PGPFreeData (*certOut);
			*certOut = NULL;
			*certOutSize = 0;

			err = kPGPError_ASNPackFailure;
			goto error_exit;
		}
	}

	err = kPGPError_NoErr;

error_exit:

	/* certSet and crls should always have been allocated */
	PKIFreeExtendedCertificatesAndCertificates (&asnContext, certSet);
	PKIFreeCertificateRevocationLists(&asnContext, crls);

	if (sigData)
		PKIFreeSignedData (&asnContext, sigData);
	if (contentData)
		PGPFreeData (contentData);
	if (passphrase)
		PGPFreeOptionList (passphrase);

	return err;
}

/* vim:ts=4:sw=4:
 */
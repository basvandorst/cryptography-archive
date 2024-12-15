/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael Elkins <michael_elkins@nai.com>
   Last Edit: October 20, 1998 */

#include "libpkcs7.h"

int
sm_CompareOID (
	PKIOBJECT_ID *a,
	PKIOBJECT_ID *b
)
{
    if (a->len == b->len && !memcmp (a->val, b->val, a->len))
	return PKCS7_OK;
    return PKCS7_ERROR_NOT_EQUAL;
}

int
sm_VerifyMessage (
	unsigned char	**signedData,		/* OUT */
	size_t		*signedDataLen,		/* OUT */
	PKISignedData	*msg,			/* IN */
	PKICertificate	*cert,			/* IN */
	HashCallback	*hashCallback,		/* IN */
	void		*hashCallbackData,	/* IN */
	VerifyCallback	*callback,		/* IN */
	void		*data,			/* IN */
	PKICONTEXT	*ctx			/* IN */
)
{
    int result = PKCS7_OK;
    unsigned char *tbs = NULL;
    size_t tbslen = 0;
    PKIIssuerAndSerialNumber *issuer;
    PKISignerInfo *signer = NULL;
    int i;
    int e = 0;
    char *digestAlg = NULL;
    char *digestEncAlg = NULL;
    char *contentOID;
    unsigned char *pcontent = NULL;
    size_t pcontentlen = 0;

    /* clear outputs */
    if( signedData )
    {
	*signedData = NULL;
	*signedDataLen = 0;
    }

    if (msg->contentInfo.content)
    {
	/* when signing, the type and length octets are not used */
	pcontent = msg->contentInfo.content->val + 1;
	pcontent += PKIGetLength (pcontent, &pcontentlen);
    }

    /* if we are given a certificate, attempt to verify the signature
       corresponding to it */
    if (cert)
    {
	/* find the RecipientInfo for the given certificate */
	issuer = PKINewIssuerAndSerialNumber (ctx);
	sm_CopyIssuerAndSerialNumber (issuer, cert, ctx);

	for (i = 0; i < msg->signerInfos.n; i++)
	    if (sm_CompareIssuerAndSerialNumber (issuer, &msg->signerInfos.elt[i]->issuerAndSerialNumber, ctx))
	    {
		signer = msg->signerInfos.elt[i];
		break;
	    }
	PKIFreeIssuerAndSerialNumber (ctx, issuer);
	if (!signer)
	    return (-1);

	digestAlg = sm_OIDToString (&signer->digestAlgorithm.algorithm, ctx);

	/* construct the message that is to be signed */
	if (signer->authenticatedAttributes)
	{
	    PKIOBJECT_ID *idMessageDigest = sm_StringToOID (SM_OID_MESSAGE_DIGEST, ctx);
	    PKIOCTET_STRING *digest = NULL;
	    PKIOCTET_STRING computedDigest;

	    /* must check the hash value in the auth attributes first */
	    for (i = 0; i < signer->authenticatedAttributes->n; i++)
	    {
		if (sm_CompareOID (idMessageDigest, &signer->authenticatedAttributes->elt[i]->type) == PKCS7_OK)
		{
		    PKIUnpackOCTET_STRING (ctx,
			    &digest,
			    signer->authenticatedAttributes->elt[i]->values.elt[0]->val,
			    signer->authenticatedAttributes->elt[i]->values.elt[0]->len,
			    &e);
		    if (e)
		    {
			PKIFreeOBJECT_ID (ctx, idMessageDigest);
			PKIFree (ctx->memMgr, digestAlg);
			return PKCS7_ERROR_HASH_ASN;
		    }
		    break;
		}
	    }
	    PKIFreeOBJECT_ID (ctx, idMessageDigest);
	    if (!digest)
	    {
		PKIFree (ctx->memMgr, digestAlg);
		return PKCS7_ERROR_MISSING_HASH;
	    }

	    /* compute the hash over the body of the message */
	    result = hashCallback (&computedDigest,
		    digestAlg,
		    pcontent,
		    pcontentlen,
		    hashCallbackData,
		    ctx);
	    if (result != PKCS7_OK)
	    {
		PKIFree (ctx->memMgr, digestAlg);
		PKIFreeOCTET_STRING (ctx, digest);
		return (result);
	    }

	    /* compare the computed and trasmitted digest values */
	    if (digest->len != computedDigest.len ||
		    memcmp (digest->val, computedDigest.val, digest->len))
	    {
		/* digest values are not equal */
		PKIFreeOCTET_STRING (ctx, digest);
		PKIFree (ctx->memMgr, digestAlg);
		PKIFree (ctx->memMgr, computedDigest.val);
		return PKCS7_ERROR_BAD_HASH_VAL;
	    }

	    PKIFree (ctx->memMgr, computedDigest.val);
	    PKIFreeOCTET_STRING (ctx, digest);

	    /* construct what is actually signed */
	    tbslen = PKISizeofAttributes (ctx, signer->authenticatedAttributes, 1);
	    tbs = PKIAlloc (ctx->memMgr, tbslen);
	    PKIPackAttributes (ctx,
		    tbs,
		    tbslen,
		    signer->authenticatedAttributes,
		    &e);
	    if (e)
	    {
		PKIFree (ctx->memMgr, digestAlg);
		return (-1);
	    }
	}
	else
	{
	    tbslen = pcontentlen;
	    tbs = PKIAlloc (ctx->memMgr, tbslen);
	    memcpy (tbs, pcontent, tbslen);
	}

	digestEncAlg = sm_OIDToString (&signer->digestEncryptionAlgorithm.algorithm, ctx);

	/* now to the verification */
	result = callback (tbs,
		tbslen,
		digestAlg,
		digestEncAlg,
		&signer->encryptedDigest,
		cert,
		data,
		ctx);

	/* clean up */
	PKIFree (ctx->memMgr, tbs);
	PKIFree (ctx->memMgr, digestAlg);
	PKIFree (ctx->memMgr, digestEncAlg);
    }

    /* return the inner content if requested */
    if (signedData && msg->contentInfo.content)
    {
	contentOID = sm_OIDToString (&msg->contentInfo.contentType, ctx);
	if (strcmp (contentOID, SM_OID_CONTENT_DATA) == 0)
	{
	    /* return the data without the type and length octets */
	    *signedDataLen = pcontentlen;
	    *signedData = PKIAlloc (ctx->memMgr, pcontentlen);
	    memcpy (*signedData, pcontent, pcontentlen);
	}
	else
	{
	    /* return the data as is */
	    *signedDataLen = msg->contentInfo.content->len;
	    *signedData = PKIAlloc (ctx->memMgr, *signedDataLen);
	    memcpy (*signedData, msg->contentInfo.content->val, *signedDataLen);
	}
	PKIFree (ctx->memMgr, contentOID);
	contentOID = NULL;
    }

    return (result);
}

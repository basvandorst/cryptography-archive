/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael Elkins <michael_elkins@nai.com>
   Last Edit: December 15, 1998 */

#include "libpkcs7.h"

const char *SM_OID_CONTENT_TYPE = "1.2.840.113549.1.9.3"; /* id-contentType */
const char *SM_OID_MESSAGE_DIGEST = "1.2.840.113549.1.9.4"; /* id-messageDigest */

const char *SM_OID_CONTENT_DATA = "1.2.840.113549.1.7.1"; /* id-data */
const char *SM_OID_CONTENT_SIGNED_DATA = "1.2.840.113549.1.7.2"; /* id-signedData */

/* const char *SM_OID_ALG_SHA = "2.16.840.1.101.2.1.1.21"; */
const char *SM_OID_ALG_SHA = "1.3.14.3.2.26";

/* const char *SM_OID_ALG_SHA_DSA = "2.16.840.1.101.2.1.1.19"; */
const char *SM_OID_ALG_DSA_SHA = "1.2.840.10040.4.3"; /* PKIX IPKI */

const char *SM_OID_ALG_DSA = "1.2.840.10040.4.1"; /* PKIX IPKI */

const char *SM_OID_ALG_MD2 = "1.2.840.113549.2.2";
const char *SM_OID_ALG_MD5 = "1.2.840.113549.2.5";
const char *SM_OID_ALG_RSA_MD2 = "1.2.840.113549.1.1.2";
const char *SM_OID_ALG_RSA_MD5 = "1.2.840.113549.1.1.4";

PKIOBJECT_ID *
sm_StringToOID (const char *str, PKICONTEXT *ctx)
{
    int num;
    int count = 0;
    int first = 0;
    int offset = 0;
    unsigned char buf[256]; /* should be large enough to avoid overflow */
    PKIOBJECT_ID *ret;

    while (*str)
    {
	for (num = 0; *str && *str != '.'; str++)
	{
	    num *= 10;
	    num += *str - '0';
	}
	if (*str)
	    str++;
	if (count == 0)
	    first = num;
	else
	{
	    if (count == 1)
		num += 40 * first;
	    if (num > 2097152) /* 128^3 */
	    {
		buf[offset++] = 0x80 | (num / 2097152);
		num -= 2097152 * (num / 2097152);
	    }
	    if (num > 16384) /* 128^2 */
	    {
		buf[offset++] = 0x80 | (num / 16384);
		num -= 16384 * (num / 16384);
	    }
	    if (num > 128)
	    {
		buf[offset++] = 0x80 | (num / 128);
		num -= 128 * (num / 128);
	    }
	    buf[offset++] = num;
	}
	count++;
    }

    ret = PKINewOBJECT_ID (ctx);
    ret->len = offset;
    ret->val = PKIAlloc (ctx->memMgr, offset);
    memcpy (ret->val, buf, offset);

    return (ret);
}

int
sm_CopyDistinguishedName (
	PKIName		*out,
	PKIName		*in,
	PKICONTEXT	*ctx
)
{
    unsigned char *buf;
    size_t buflen;
    int err = 0;

    buflen = PKISizeofName (ctx, in, 1);
    buf = PKIAlloc (ctx->memMgr, buflen);
    if (PKIPackName (ctx, buf, buflen, in, &err) != buflen || err)
    {
        PKIFree (ctx->memMgr, buf);
        return (-1);
    }
    memset (out, 0, sizeof (PKIName));
    if (PKIUnpkInPlaceName (ctx, out, buf, buflen, PKIID_Name, &err) != buflen || err)
    {
        PKIFree (ctx->memMgr, buf);
        return (-1);
    }
    PKIFree (ctx->memMgr, buf);
    return PKCS7_OK;
}

int
sm_CopyIssuerAndSerialNumber (
	PKIIssuerAndSerialNumber	*out,
	PKICertificate			*in,
	PKICONTEXT			*ctx
)
{
    int r;

    r = sm_CopyDistinguishedName (&out->issuer, &in->tbsCertificate.issuer, ctx);
    if (r == PKCS7_OK)
    {
        PKIPutOctVal (ctx, &out->serialNumber, in->tbsCertificate.serialNumber.val, in->tbsCertificate.serialNumber.len);
    }
    return (r);
}

static int sm_AddSimpleAttribute (
	PKIAttributes	*attr,	/* OUT */
	PKIOBJECT_ID	*type,	/* IN */
	PKIANY		*value,	/* IN */
	PKICONTEXT	*ctx	/* IN */
)
{
    attr->elt[attr->n] = PKINewAttribute (ctx);
    PKIPutOctVal (ctx, &attr->elt[attr->n]->type, type->val, type->len);
    attr->elt[attr->n]->values.n = 1;
    attr->elt[attr->n]->values.elt[0] = PKINewANY (ctx);
    PKIPutOctVal (ctx, attr->elt[attr->n]->values.elt[0], value->val, value->len);
    attr->n++;

    return PKCS7_OK;
}

static PKIAttribute *
sm_CopyAttribute (
	PKIAttribute	*attr,
	PKICONTEXT	*ctx
)
{
    unsigned char	*buf;
    size_t		buflen;
    int			err = 0;
    PKIAttribute	*ret;

    buflen = PKISizeofAttribute (ctx, attr, 1);
    buf = PKIAlloc (ctx->memMgr, buflen);
    if (PKIPackAttribute (ctx, buf, buflen, attr, &err) != buflen || err)
    {
        PKIFree (ctx->memMgr, buf);
        return (NULL);
    }
    if (PKIUnpackAttribute (ctx, &ret, buf, buflen, &err) != buflen || err)
    {
        PKIFree (ctx->memMgr, buf);
        return (NULL);
    }
    PKIFree (ctx->memMgr, buf);
    return (ret);
}

int
sm_SignMessage (
	unsigned char   **signedMessage,	/* OUT */
	size_t		*signedMessageLen,	/* OUT */
	unsigned char	*tbs,			/* IN data to sign */
	size_t		tbsLen,			/* IN length of `tbs' */
	int		nested,			/* IN */
	const char	*contentType,		/* IN type of `tbs' */
	const char	*digestAlgorithm,	/* IN */
	const unsigned char	*digestParam,	/* IN */
	size_t		digestParamSize,	/* IN */
	const char	*signatureAlgorithm,	/* IN */
	const unsigned char	*sigParam,	/* IN */
	size_t		sigParamSize,		/* IN */
	PKICertificate	*signerCertificate,	/* IN */
	PKIAttributes	*signedAttributes,	/* IN (optional) */
	List		*includeCerts,		/* IN (optional) */
	HashCallback	*hashCallback,		/* IN */
	void		*hashCallbackData,	/* IN (optional) */
	SignCallback	*signCallback,		/* IN */
	void		*signCallbackData,	/* IN (optional) */
	PKICONTEXT	*ctx)			/* IN */
{
    PKIContentInfo	contentInfo;
    PKISignedData	*signedData;
    PKISignerInfo	*signerInfo;
    PKIOBJECT_ID	*digestOID;
    PKIOBJECT_ID	*contentOID;
    PKIOBJECT_ID	*signatureOID;
    PKIOBJECT_ID	*contentTypeOID;
    PKIOBJECT_ID	*signedDataOID;
    PKIANY		asnBlock;
    PKIANY		digestValue;

    int			err = 0;
    int			i;
    size_t		msgLen;
    unsigned char	*msg;
    int			result;
    unsigned char	*pcontent;
    size_t		pcontentlen;

    signedData = PKINewSignedData (ctx);

    /* CMS states that the version is 1 if no attribute certificates are
       present and the underlying content type is id-data */
    PKIPutIntVal (ctx, &signedData->version, (!strcmp (contentType, SM_OID_CONTENT_DATA)) ? 1 : 3);

    signedData->digestAlgorithms.n = 1;
    signedData->digestAlgorithms.elt[0] = PKINewDigestAlgorithmIdentifier (ctx);

    digestOID = sm_StringToOID (digestAlgorithm, ctx);

    PKIPutOctVal (ctx, &signedData->digestAlgorithms.elt[0]->algorithm, digestOID->val, digestOID->len);
    if (digestParamSize)
    {
	signedData->digestAlgorithms.elt[0]->parameters = PKINewANY (ctx);
	PKIPutOctVal (ctx, signedData->digestAlgorithms.elt[0]->parameters, digestParam, digestParamSize);
    }

    contentOID = sm_StringToOID (contentType, ctx);
    PKIPutOctVal (ctx, &signedData->contentInfo.contentType, contentOID->val, contentOID->len);

    /* add the data that is being signed */
    signedData->contentInfo.content = PKINewANY (ctx);

    /* if we have type Data, it must be encoded as OCTET STRING */
    if (strcmp (contentType, SM_OID_CONTENT_DATA) == 0)
    {
        PKIOCTET_STRING    *content;

        content = PKINewOCTET_STRING (ctx);
        PKIPutOctVal (ctx, content, tbs, tbsLen);
        signedData->contentInfo.content->len = PKISizeofOCTET_STRING (ctx, content, 1);
        signedData->contentInfo.content->val = PKIAlloc (ctx->memMgr, signedData->contentInfo.content->len);
        PKIPackOCTET_STRING (ctx, signedData->contentInfo.content->val, signedData->contentInfo.content->len, content, &err);
        PKIFreeOCTET_STRING (ctx, content);
        if (err)
            return PKCS7_ERROR_UNKNOWN;
        pcontent = tbs;
        pcontentlen = tbsLen;
    }
    else
    {
        PKIPutOctVal (ctx, signedData->contentInfo.content, tbs, tbsLen);
        /* when signing, the type and length octets are not included in the hash */
        pcontent = tbs + 1;
        pcontent += PKIGetLength (pcontent, &pcontentlen);
    }

    signedData->signerInfos.n = 1;
    signerInfo = signedData->signerInfos.elt[0] = PKINewSignerInfo (ctx);

    PKIPutIntVal (ctx, &signerInfo->version, 1); /* version 1 */

    sm_CopyIssuerAndSerialNumber (&signerInfo->issuerAndSerialNumber, signerCertificate, ctx);

    PKIPutOctVal (ctx, &signerInfo->digestAlgorithm.algorithm, digestOID->val, digestOID->len);
    PKIFreeOBJECT_ID (ctx, digestOID);
    if (digestParamSize)
    {
	signerInfo->digestAlgorithm.parameters = PKINewANY (ctx);
	PKIPutOctVal (ctx, signerInfo->digestAlgorithm.parameters, digestParam, digestParamSize);
    }

    if (signedAttributes || (strcmp (contentType, SM_OID_CONTENT_DATA)))
    {
	contentTypeOID = sm_StringToOID (SM_OID_CONTENT_TYPE, ctx);

	signerInfo->authenticatedAttributes = PKINewAttributes (ctx);

	/* add the id-contentType attribute */
	memset (&asnBlock, 0, sizeof (asnBlock));
	asnBlock.len = PKISizeofOBJECT_ID (ctx, contentOID, 1);
	asnBlock.val = PKIAlloc (ctx->memMgr, asnBlock.len);
	PKIPackOBJECT_ID (ctx, asnBlock.val, asnBlock.len, contentOID, &err);
	sm_AddSimpleAttribute (signerInfo->authenticatedAttributes, contentTypeOID, &asnBlock, ctx);
	PKIFree (ctx->memMgr, asnBlock.val);
	PKIFreeOBJECT_ID (ctx, contentTypeOID);

	/* add the id-messageDigest attribute */
	result = hashCallback (&digestValue, digestAlgorithm, pcontent, pcontentlen, hashCallbackData, ctx);
	if (result != PKCS7_OK)
	    return (result);

	memset (&asnBlock, 0, sizeof (asnBlock));
	err = 0;
	asnBlock.len = PKISizeofOCTET_STRING (ctx, &digestValue, 1);
	asnBlock.val = PKIAlloc (ctx->memMgr, asnBlock.len);
	PKIPackOCTET_STRING (ctx, asnBlock.val, asnBlock.len, &digestValue, &err);
	contentTypeOID = sm_StringToOID (SM_OID_MESSAGE_DIGEST, ctx);
	sm_AddSimpleAttribute (signerInfo->authenticatedAttributes, contentTypeOID, &asnBlock, ctx);
	PKIFreeOBJECT_ID (ctx, contentTypeOID);
	PKIFree (ctx->memMgr, asnBlock.val);
	PKIFree (ctx->memMgr, digestValue.val);

	/* add the user specified attributes */
	if (signedAttributes)
	{
	    for (i = 0; i < signedAttributes->n; i++)
	    {
		signerInfo->authenticatedAttributes->elt[signerInfo->authenticatedAttributes->n++] = sm_CopyAttribute (signedAttributes->elt[i], ctx);
	    }
	}

	/* if auth attributes are present, we sign the DER of it.  the body
	   of the message is protected by the fact that the hash value is
	   included as an attribute */
	msgLen = PKISizeofAttributes (ctx, signerInfo->authenticatedAttributes, 1);
	msg = PKIAlloc (ctx->memMgr, msgLen);
	PKIPackAttributes (ctx, msg, msgLen, signerInfo->authenticatedAttributes, &err);
	if (err)
	    return PKCS7_ERROR_ATTRIBUTES;
    }
    else
    {
	/* otherwise sign the data itself */
	msgLen = pcontentlen;
	msg = PKIAlloc (ctx->memMgr, msgLen);
	memcpy (msg, pcontent, pcontentlen);
    }

    PKIFreeOBJECT_ID (ctx, contentOID);

    result = signCallback (
	    &signerInfo->encryptedDigest,
	    digestAlgorithm,
	    signatureAlgorithm,
	    signerCertificate,
	    msg,
	    msgLen,
	    signCallbackData,
	    ctx);
    PKIFree (ctx->memMgr, msg);
    if (result != PKCS7_OK)
        return (result);

    signatureOID = sm_StringToOID (signatureAlgorithm, ctx);
    PKIPutOctVal (ctx,
	    &signerInfo->digestEncryptionAlgorithm.algorithm,
	    signatureOID->val, signatureOID->len);
    PKIFreeOBJECT_ID (ctx, signatureOID);

    if (sigParamSize)
    {
	signerInfo->digestEncryptionAlgorithm.parameters = PKINewANY (ctx);
	PKIPutOctVal (ctx, signerInfo->digestEncryptionAlgorithm.parameters, sigParam, sigParamSize);
    }

    if (includeCerts)
    {
	List *pCert;
	int certCount = 0;
	
	signedData->certificates = PKINewExtendedCertificatesAndCertificates(ctx);
	for (pCert = includeCerts; pCert; pCert = pCert->next, certCount++)
	{
	    unsigned char *certData;
	    size_t certLen;
	    PKICertificate *tCert = (PKICertificate *) pCert->data;
	    int asnError = 0;
	    
	    certLen = PKISizeofCertificate (ctx, tCert, 1);
	    certData = PKIAlloc (ctx->memMgr, certLen);
	    PKIPackCertificate (ctx, certData, certLen, tCert, &asnError);
	    if (asnError)
	    {
		PKIFree (ctx->memMgr, certData);
		continue;
	    }
	    signedData->certificates->elt[certCount] = PKIAlloc (ctx->memMgr,
		    sizeof (PKIExtendedCertificateOrCertificate));
	    signedData->certificates->elt[certCount]->CHOICE_field_type = PKIID_Certificate;
	    PKIUnpackCertificate (ctx, (PKICertificate **) &signedData->certificates->elt[certCount]->data,
		    certData,
		    certLen,
		    &asnError);
	    PKIFree (ctx->memMgr, certData);
	}
	signedData->certificates->n = certCount;
    }

    if (nested)
    {
	/* when doing nested protection, the ContentInfo header will be
	   present in the upper level SignedData or EnvelopedData, so just
	   return the SignedData object directly */

	*signedMessageLen = PKISizeofSignedData (ctx, signedData, 1);
	*signedMessage = PKIAlloc (ctx->memMgr, *signedMessageLen);
	PKIPackSignedData (ctx, *signedMessage, *signedMessageLen, signedData, &err);
    }
    else
    {
	signedDataOID = sm_StringToOID (SM_OID_CONTENT_SIGNED_DATA, ctx);

	memset (&contentInfo, 0, sizeof (contentInfo));
	PKIPutOctVal (ctx, &contentInfo.contentType, signedDataOID->val, signedDataOID->len);
	PKIFreeOCTET_STRING (ctx, signedDataOID);

	contentInfo.content = PKINewANY (ctx);
	contentInfo.content->len = PKISizeofSignedData (ctx, signedData, 1);
	contentInfo.content->val = PKIAlloc (ctx->memMgr, contentInfo.content->len);
	PKIPackSignedData (ctx, contentInfo.content->val, contentInfo.content->len, signedData, &err);

	/* pack up the final version of the message */
	*signedMessageLen = PKISizeofContentInfo (ctx, &contentInfo, 1);
	*signedMessage = PKIAlloc (ctx->memMgr, *signedMessageLen);
	PKIPackContentInfo (ctx, *signedMessage, *signedMessageLen, &contentInfo, &err);

	PKIDropInPlaceContentInfo (ctx, &contentInfo);
    }

    PKIFreeSignedData (ctx, signedData);

    return 0;
}

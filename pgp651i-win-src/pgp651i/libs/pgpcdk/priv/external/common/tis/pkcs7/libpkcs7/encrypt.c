/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael Elkins <michael_elkins@nai.com>
   Last Edit: November 12, 1998 */

#include "libpkcs7.h"

const char *SM_OID_CONTENT_ENVELOPED_DATA = "1.2.840.113549.1.7.3";
const char *SM_OID_ALG_KEA = "2.16.840.1.101.2.1.1.10";
const char *SM_OID_ALG_SKIPJACK = "2.16.840.1.101.2.1.1.4"; /* skipjack-cbc */
const char *SM_OID_ALG_RSA = "1.2.840.113549.1.1.1";
const char *SM_OID_ALG_RC2 = "1.2.840.113549.3.2";
const char *SM_OID_ALG_DES = "1.3.14.3.2.7";
const char *SM_OID_ALG_3DES = "1.2.840.113549.3.7"; /* des ede3 cbc */

int
sm_EncryptMessage (
	unsigned char	**msg,                /* [OUT] output message */
	size_t		*msgLen,              /* [OUT] length of `msg' */
	const unsigned char	*tbe,         /* [IN] data to be encrypted */
	size_t	        tbeLen,               /* [IN] length of `tbe' */
	int		nested,
	const char      *contentType,         /* [IN] type of `tbe' */
	const char      *encAlg,              /* [IN] encryption alg to use */
	List	        *recipCerts,          /* [IN] recipient certificates */
	EncryptCallback	*encryptCallback,     /* [IN] encrypt function */
	void	        *encryptCallbackData, /* [IN] (optional) */
	PKICONTEXT	*ctx)
{
    PKIContentInfo	*contentInfo;
    PKIEnvelopedData	*envelopedData;
    List	   	*pList;
    PKIRecipientInfo   	*recipientInfo;
    EncryptRecipient	*encryptRecipient;
    PKIOBJECT_ID	*oid;
    int			err = 0;
    PKIOCTET_STRING	*encodedContent;
    const unsigned char *realtbe;
    size_t              realtbelen;
    unsigned char       *encodedTbe = NULL;
    int			result;

    envelopedData = PKINewEnvelopedData (ctx);
    PKIPutIntVal (ctx, &envelopedData->version, 1);

    if (strcmp (contentType, SM_OID_CONTENT_DATA) == 0)
    {
	/* if we have an internal type of Data, then it must be encoded as
	   OCTET STRING.  Do this here for the convenience of the
	   application */
	encodedContent = PKINewOCTET_STRING (ctx);
	PKIPutOctVal (ctx, encodedContent, tbe, tbeLen);
	realtbelen = PKISizeofOCTET_STRING (ctx, encodedContent, 1);
	encodedTbe = PKIAlloc (ctx->memMgr, realtbelen);
	PKIPackOCTET_STRING (ctx, encodedTbe, realtbelen, encodedContent, &err);
	PKIFreeOCTET_STRING (ctx, encodedContent);
	encodedContent = NULL;
	realtbe = encodedTbe;
    }
    else
    {
	/* don't alter the data */
	realtbelen = tbeLen;
	realtbe = tbe;
    }

    envelopedData->encryptedContentInfo.encryptedContent = PKINewEncryptedContent (ctx);
    envelopedData->encryptedContentInfo.contentEncryptionAlgorithm.parameters = PKINewANY (ctx);
    
    result = encryptCallback (
	    envelopedData->encryptedContentInfo.encryptedContent,
	    envelopedData->encryptedContentInfo.contentEncryptionAlgorithm.parameters,
	    encAlg,
	    realtbe,
	    realtbelen,
	    recipCerts,
	    encryptCallbackData,
	    ctx);
    if (result != PKCS7_OK)
    {
	PKIFreeEnvelopedData (ctx, envelopedData);
	return (result);
    }

    if (encodedTbe)
    {
        /* if we made a copy, free up the ctxory now */
        PKIFree (ctx->memMgr, encodedTbe);
        encodedTbe = NULL;
    }

    if (envelopedData->encryptedContentInfo.contentEncryptionAlgorithm.parameters->len == 0)
    {
        PKIFreeANY (ctx, envelopedData->encryptedContentInfo.contentEncryptionAlgorithm.parameters);
        envelopedData->encryptedContentInfo.contentEncryptionAlgorithm.parameters = NULL;
    }

    /*** RecipientInfos ***/

    for (pList = recipCerts; pList; pList = pList->next)
    {
	recipientInfo = PKINewRecipientInfo (ctx);
	PKIPutIntVal (ctx, &recipientInfo->version, 0);
	encryptRecipient = (EncryptRecipient *) pList->data;
	sm_CopyIssuerAndSerialNumber (&recipientInfo->issuerAndSerialNumber, encryptRecipient->certificate, ctx);

	oid = sm_StringToOID (encryptRecipient->algorithm, ctx);
	PKIPutOctVal (ctx, &recipientInfo->keyEncryptionAlgorithm.algorithm, oid->val, oid->len);
	PKIFreeOBJECT_ID (ctx, oid);

	PKIPutOctVal (ctx, &recipientInfo->encryptedKey, encryptRecipient->encryptedKey, encryptRecipient->encryptedKeyLen);
	envelopedData->recipientInfos.elt[envelopedData->recipientInfos.n++] = recipientInfo;
    }

    /*** EncryptedContentInfo ***/

    oid = sm_StringToOID (contentType, ctx);
    PKIPutOctVal (ctx, &envelopedData->encryptedContentInfo.contentType, oid->val, oid->len);
    PKIFreeOBJECT_ID (ctx, oid);
    
    oid = sm_StringToOID (encAlg, ctx);
    PKIPutOctVal (ctx, &envelopedData->encryptedContentInfo.contentEncryptionAlgorithm.algorithm, oid->val, oid->len);
    PKIFreeOBJECT_ID (ctx, oid);

    if (nested)
    {
	/* if nested, dont add the ContentInfo wrapper since it will be
	   present in the higher level SignedData or EnvelopedData structure */
	*msgLen = PKISizeofEnvelopedData (ctx, envelopedData, 1);
	*msg = PKIAlloc (ctx->memMgr, *msgLen);
	PKIPackEnvelopedData (ctx, *msg, *msgLen, envelopedData, &err);
    }
    else
    {
	/*** Build up the ContentInfo structure ***/
	contentInfo = PKINewContentInfo (ctx);

	oid = sm_StringToOID (SM_OID_CONTENT_ENVELOPED_DATA, ctx);
	PKIPutOctVal (ctx, &contentInfo->contentType, oid->val, oid->len);
	PKIFreeOBJECT_ID (ctx, oid);

	contentInfo->content = PKINewANY (ctx);
	contentInfo->content->len = PKISizeofEnvelopedData (ctx, envelopedData, 1);
	contentInfo->content->val = PKIAlloc (ctx->memMgr, contentInfo->content->len);
	PKIPackEnvelopedData (ctx, contentInfo->content->val, contentInfo->content->len, envelopedData, &err);

	*msgLen = PKISizeofContentInfo (ctx, contentInfo, 1);
	*msg = PKIAlloc (ctx->memMgr, *msgLen);
	PKIPackContentInfo (ctx, *msg, *msgLen, contentInfo, &err);
	PKIFreeContentInfo (ctx, contentInfo);
    }

    PKIFreeEnvelopedData (ctx, envelopedData);

    return 0;
}

/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael Elkins <michael_elkins@nai.com>
   Last Edit: November 18, 1998 */

#include "libpkcs7.h"
#include "pkcs7_oid.h"

PKIContentInfo *
sm_DecodeMessage (
	const unsigned char	*msg,
	size_t			msglen,
	PKICONTEXT		*ctx)
{
    int e = 0;
    PKIContentInfo *info;

    PKIUnpackContentInfo (ctx, &info, (unsigned char *) msg, msglen, &e);
    return (info);
}

int sm_ContentType (PKIOBJECT_ID *asn, PKICONTEXT *ctx)
{
    (void) ctx;
    if (asn->len == PKIdata_OID_LEN &&
	    !memcmp (asn->val, PKIdata_OID, PKIdata_OID_LEN))
	return PKCS7_CONTENT_DATA;
    if (asn->len == PKIsignedData_OID_LEN &&
	    !memcmp (asn->val, PKIsignedData_OID, PKIsignedData_OID_LEN))
	return PKCS7_CONTENT_SIGNED_DATA;
    if (asn->len == PKIenvelopedData_OID_LEN &&
	    !memcmp (asn->val, PKIenvelopedData_OID, PKIenvelopedData_OID_LEN))
	return PKCS7_CONTENT_ENVELOPED_DATA;
    if (asn->len == PKIencryptedData_OID_LEN &&
	    !memcmp (asn->val, PKIencryptedData_OID, PKIencryptedData_OID_LEN))
	return PKCS7_CONTENT_ENCRYPTED_DATA;
    return (-1);
}

int sm_MessageType (PKIContentInfo *info, PKICONTEXT *ctx)
{
    return (sm_ContentType (&info->contentType, ctx));
}

PKISignedData *
sm_DecodeSignedData (
	const unsigned char *signedData,
	size_t	signedDataLen,
	PKICONTEXT *ctx)
{
    int e = 0;
    PKISignedData *data;

    PKIUnpackSignedData (ctx, &data, (unsigned char *) signedData, signedDataLen, &e);
    return (data);
}

PKIEnvelopedData *
sm_DecodeEnvelopedData (
	const unsigned char	*envelopedData,
	size_t			envelopedDataLen,
	PKICONTEXT		*ctx)
{
    int e = 0;
    PKIEnvelopedData *data;

    PKIUnpackEnvelopedData (ctx, &data, (unsigned char *) envelopedData, envelopedDataLen, &e);
    return (data);
}

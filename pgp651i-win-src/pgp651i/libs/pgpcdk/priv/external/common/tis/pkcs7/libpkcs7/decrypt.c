/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael Elkins <michael_elkins@nai.com>
   Last Edit: October 20, 1998 */

#include <math.h>
#include <stdlib.h>
#include "libpkcs7.h"

/* converts a DER encoding ASN.1 object-identifier into dot-string format */
/* FOO - possible buffer overflow if the input string generates and encoded ASN.1 longer than 256 bytes */
char *
sm_OIDToString (PKIOBJECT_ID *id, PKICONTEXT *ctx)
{
    char buf[1024];
    unsigned int ioff;
    unsigned int ibeg;
    int a1;
    int t;
    char *result;
    
    if (id->val[0] < 40)
	    a1 = 0;
    else if (id->val[0] < 80)
	    a1 = 1;
    else
	    a1 = 2;

    sprintf (buf, "%d.%d", a1, id->val[0] - 40 * a1);
    
    for (ioff = 1; ioff < id->len; ioff++)
    {
	    ibeg = ioff;
	    while (id->val[ioff] & 0x80 && ioff < id->len)
	        ioff++;
	    for (t = 0; ibeg <= ioff; ibeg++)
	        t += (int)((id->val[ibeg] & 0x7f)) * (int) (pow (128.0, ioff - ibeg));
	    sprintf (buf + strlen (buf), ".%d", t);
    }
    result = PKIAlloc (ctx->memMgr, strlen (buf) + 1);
    strcpy (result, buf);
    return (result);
}

int
sm_CompareIssuerAndSerialNumber (
	PKIIssuerAndSerialNumber	*a,	/* [IN] */
	PKIIssuerAndSerialNumber	*b,	/* [IN] */
	PKICONTEXT			*ctx	/* [IN] */
)
{
    unsigned char	*da, *db;
    size_t		la, lb;
    int			e = 0;
    int			result = 0;

    la = PKISizeofIssuerAndSerialNumber (ctx, a, 1);
    lb = PKISizeofIssuerAndSerialNumber (ctx, b, 1);

    if (la == lb)
    {
	da = PKIAlloc (ctx->memMgr, la);
	PKIPackIssuerAndSerialNumber (ctx, da, la, a, &e);

	db = PKIAlloc (ctx->memMgr, lb);
	PKIPackIssuerAndSerialNumber (ctx, db, lb, b, &e);

	result = (memcmp (da, db, la) == 0);

	PKIFree (ctx->memMgr, da);
	PKIFree (ctx->memMgr, db);
    }

    return (result);
}

/* Returns the index if the specified certificate is listed as a recipent of the
   encrypted message, other -1 is returned */
static int sm_CertIsRecipient (
	PKICertificate		*cert,		/* [IN] cert to check for */
	PKIEnvelopedData	*envelope,	/* [IN] message to check */
	PKICONTEXT		*ctx		/* [IN] */
)
{
    PKIIssuerAndSerialNumber	*is;
    int				i;
    int				result = -1;

    is = PKINewIssuerAndSerialNumber (ctx);
    sm_CopyIssuerAndSerialNumber (is, cert, ctx);

    for (i = 0; i < envelope->recipientInfos.n; i++)
	if (sm_CompareIssuerAndSerialNumber (is, &envelope->recipientInfos.elt[i]->issuerAndSerialNumber, ctx))
	{
	    result = i;
	    break;
	}
    PKIFreeIssuerAndSerialNumber (ctx, is);
    return (result);
}

int
sm_DecryptMessage (
	unsigned char		**msg,	  /* [OUT] decrypted message */
	size_t			*msglen,  /* [OUT] decrypted message length */
	PKIEnvelopedData	*envelope,/* [IN] encrypted message */
	PKICertificate		*cert,	  /* [IN] cert to decrypt for */
	DecryptCallback		*callback,/* [IN] decrypt callback function */
	void	    		*data,	  /* [IN] callback data (optional) */
	PKICONTEXT		*ctx
)
{
    int     result;
    int     recip = 0;
    char    *contentEncryptionAlg = NULL;
    char    *keyEncryptionAlg = NULL;
    char    *contentOID;

    recip = sm_CertIsRecipient (cert, envelope, ctx);
    if (recip < 0)
	return (-1);
    contentEncryptionAlg = sm_OIDToString (&envelope->encryptedContentInfo.contentEncryptionAlgorithm.algorithm, ctx);
    keyEncryptionAlg = sm_OIDToString (&envelope->recipientInfos.elt[recip]->keyEncryptionAlgorithm.algorithm, ctx);

    result = callback (
	    msg,
	    msglen,
	    contentEncryptionAlg,
	    envelope->encryptedContentInfo.contentEncryptionAlgorithm.parameters,
	    envelope->encryptedContentInfo.encryptedContent,
	    keyEncryptionAlg,
	    &envelope->recipientInfos.elt[recip]->encryptedKey,
	    cert,
	    data,
	    ctx);

    if (contentEncryptionAlg)
    {
        PKIFree (ctx->memMgr, contentEncryptionAlg);
        contentEncryptionAlg = NULL;
    }
    if (keyEncryptionAlg)
    {
        PKIFree (ctx->memMgr, keyEncryptionAlg);
        keyEncryptionAlg = NULL;
    }

    if (result == PKCS7_OK)
    {
        /* if the internal type is Data, remove the ASN.1 OCTET STRING encoding as a favor
           to the application */
        contentOID = sm_OIDToString (&envelope->encryptedContentInfo.contentType, ctx);
        if (strcmp (contentOID, SM_OID_CONTENT_DATA) == 0)
        {
            PKIOCTET_STRING    *encodedContent;
            int                 e = 0;

            PKIUnpackOCTET_STRING (ctx, &encodedContent, *msg, *msglen, &e);
            PKIFree (ctx->memMgr, *msg);
            *msg = encodedContent->val;
            *msglen = encodedContent->len;
            encodedContent->val = NULL;
            encodedContent->len = 0;
            PKIFreeOCTET_STRING (ctx, encodedContent);
            encodedContent = NULL;
        }
        PKIFree (ctx->memMgr, contentOID);
        contentOID = NULL;
    }

    return (result);
}

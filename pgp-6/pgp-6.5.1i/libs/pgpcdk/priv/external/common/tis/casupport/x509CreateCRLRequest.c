/* Copyright (C) 1998-9 Network Associates, Inc.
   Author: michael_elkins@nai.com
   Last Edit: January 21, 1998 */

#include <stdio.h>
#include "x509CMS.h"
#include "pgpX509Priv.h"
#include "pkcsreq_asn.h"


/* TODO: this function is outdated by the latest CMS which is not part of the
   SDK, it should be replaced once the SDK is updated */
static int
tc_encode_generalizedtime (
    PKIGeneralizedTime  **gen,
    time_t              t,
    TC_CONTEXT          *ctx)
{
    struct tm   *tm;
    
    tm = gmtime(&t);
    *gen = TC_Alloc(ctx->memMgr, sizeof (PKIGeneralizedTime));
    if (!*gen)
        return TC_E_NOMEMORY;
    (*gen)->len = 15;
    /* allocate one extra byte here for the \0 that strftime() will write which
       is not part of the asn.1 encoding */
    (*gen)->val = TC_Alloc(ctx->memMgr, (*gen)->len + 1);
    if (!(*gen)->val)
        return TC_E_NOMEMORY;
    strftime((char *) (*gen)->val, (*gen)->len + 1, "%Y%m%d%H%M%SZ", tm);
    return 0;
}


PGPError
X509CreateCRLRequest (
    PGPContextRef	context,
    PGPKeyRef		key,		/* [IN] key for CA we request from */
    PGPByte		*crldistp,	/* [IN] asn.1 crl dist point ext (opt)*/
    PGPSize		crldistpsize,	/* [IN] size of crldistp */
    PGPExportFormat	format,		/* [IN] what type of CA */
    time_t		validity,	/* [IN] CRL for which date */
    PGPByte		**request,	/* [OUT] der encoded GetCRL msg */
    PGPSize		*requestsize)	/* [OUT] size of der encoding */
{
    TC_CONTEXT		tc;
    PKICONTEXT		asn;
    PKIGetCRL		*crl = NULL;
    PKICertificate	*cert = NULL;
    PGPError		err;
    int			asnerr = 0;
    PGPMemoryMgrRef	mem = PGPGetContextMemoryMgr (context);

    /* clear outputs */
    *request = NULL;
    *requestsize = 0;
    
    /* set up the asn.1 parser */
    memset(&asn, 0, sizeof(asn));
    asn.memMgr = &X509CMSMemoryMgr;
    asn.memMgr->customValue = mem;

    /* retrieve the CA's certificate */
    err = x509ExtractCertificate (context, key, &asn, &cert);
    if (IsPGPError (err))
	goto error;

    crl = PKINewGetCRL (&asn);
    
    /* copy the CA's distinguished name */
    if (sm_CopyDistinguishedName (&crl->issuerName, &cert->tbsCertificate.subject, &asn) != PKCS7_OK)
    {
	err = kPGPError_LazyProgrammer;
	goto error;
    }

    /* initialize cms structures */
    memset(&tc, 0, sizeof(tc));
    tc.memMgr = asn.memMgr;
    tc.certasnctx = &asn;

    if (tc_encode_generalizedtime (&crl->time, validity, &tc) != 0)
    {
	err = kPGPError_LazyProgrammer;
	goto error;
    }

    /* if communicating with an Entrust CA, we need to extract the
       crl distribution point info from the cert */
    if (format == kPGPExportFormat_EntrustV1_GetCRL)
    {
	PKIGeneralNames	*gn;

	PKIUnpackGeneralNames(&asn,&gn,crldistp,crldistpsize,&asnerr);
	if(asnerr)
	{
	    /* unable to parse the extension value into the GeneralNames */
	    err=kPGPError_LazyProgrammer;
	    goto error;
	}
	/* the crlDistributionPoint can have many names, but the GetCRL format
	   only allows one, so use the last one in the SET.  this is
	   completely arbitrary, but it makes free'ing the remaining names
	   easier since we don't have to shift down the rest of the array */
	if(gn->n)
	    crl->crlName=gn->elt[--gn->n];
	PKIFreeGeneralNames(&asn,gn);
    }

    *requestsize = PKISizeofGetCRL (&asn, crl, TRUE);
    *request = PGPNewData (mem, *requestsize, 0);
    if (!*request)
    {
	err = kPGPError_OutOfMemory;
	goto error;
    }
    
    PKIPackGetCRL (&asn, *request, *requestsize, crl, &asnerr);
    
    if (asnerr)
    {
	err = kPGPError_ASNPackFailure;
	goto error;
    }

    err = kPGPError_NoErr;

error:

    if (crl)
	PKIFreeGetCRL (&asn, crl);
    if (cert)
	PKIFreeCertificate (&asn, cert);
    if (IsPGPError (err))
    {
	if (*request)
	{
	    PGPFreeData (*request);
	    *request = NULL;
	}
	*requestsize = 0;
    }
    return err;
}

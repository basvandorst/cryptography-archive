/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "tc.h"
#include "cms.h"

#include "cms_proto.h"

int tc_set_alg (PKIAlgorithmIdentifier *p,
		const unsigned char *oid,
		size_t oidlen,
		const unsigned char *param,
		size_t paramlen,
		TC_CONTEXT *ctx)
{
    if (p == NULL)
	return TC_E_INVARGS;

    PKIPutOctVal(ctx->certasnctx, &p->algorithm, oid, oidlen);

    if (param != NULL && paramlen > 0) {
	p->parameters = PKINewANY(ctx->certasnctx);
	PKIPutOctVal(ctx->certasnctx, p->parameters, param, paramlen);
    }
    else 
	p->parameters = NULL;

    return 0;
}

static int tc_Certificate2CERT(TC_CERT **cert, PKICertificate *certificate,
			       TC_CONTEXT *ctx)
{  
  unsigned char *ptr;
  size_t ptrlen;
  int errorret = 0;

  ptrlen = PKISizeofCertificate(ctx->certasnctx, certificate, 1);
  ptr = TC_Alloc(ctx->memMgr, ptrlen);
  if (ptr == NULL)
     return TC_E_NOMEMORY;

  (void)PKIPackCertificate(ctx->certasnctx, ptr, ptrlen,
			   certificate, &errorret);
  if (errorret != 0) {
    TC_Free(ctx->memMgr, ptr);
    return compiler2tc_error(errorret);
  }

  errorret = tc_unpack_cert(cert, ptr, ptrlen, ctx);
  TC_Free(ctx->memMgr, ptr);

  return (errorret);

} /* tc_Certificate2CERT */

static int tc_pack_tbscert (unsigned char **ptr, size_t *ptrlen,
			    PKICertificate *cert, TC_CONTEXT *ctx)
{
  int status = 0;

  *ptrlen = PKISizeofTBSCertificate (ctx->certasnctx,
				     &cert->tbsCertificate, 1);
  *ptr = TC_Alloc(ctx->memMgr, *ptrlen);
  if (*ptr == NULL)
    return TC_E_NOMEMORY;

  (void)PKIPackTBSCertificate (ctx->certasnctx,
			       *ptr, *ptrlen, &cert->tbsCertificate, &status);
  if (status != 0)
  {
    TC_Free(ctx->memMgr, *ptr);
    *ptrlen = 0;
    return compiler2tc_error(status);
  }
  return 0;
}

int tc_create_cert (TC_CERT **cert,
		    int version,
		    unsigned char *serial,
		    size_t serialLen,
		    const unsigned char *sigoid,
		    size_t sigoidlen,
		    const unsigned char *sigparm,
		    size_t sigparmlen,
		    TC_Name *issuerName,
		    time_t notBefore,
		    time_t notAfter,
		    TC_Name *subjectName,
		    TC_ExtensionList *extensions,
		    const unsigned char *keyoid,
		    size_t keyoidlen,
		    const unsigned char *pubkey,
		    size_t pubkeylen,
		    const unsigned char *keyparm,
		    size_t keyparmlen,
		    TC_CONTEXT *ctx)
{
  PKICertificate *localCert;
  PKITBSCertificate *tbsCert;
  int status;
  unsigned char *sig = NULL;
  unsigned char *ber = NULL;
  size_t berlen;
  size_t siglen;
  TC_Name *localIssuer = NULL;
  TC_Name *localSubject = NULL;

  localCert = PKINewCertificate(ctx->certasnctx);
  if (localCert == NULL)
      return TC_E_NOMEMORY;
  tbsCert = &localCert->tbsCertificate;

  do {

    if (!ctx || !ctx->sign || cert == NULL || !issuerName || !subjectName) {
        status = TC_E_INVARGS;
        break;
    }

    if (version < TC_X509_VERSION_1 || version > TC_X509_VERSION_3) {
        status =  TC_E_INVARGS;
        break;
    }

    /* only create the version field if its not the DEFAULT of 1 */
    if (version != TC_X509_VERSION_1) {
	tbsCert->version = TC_Alloc(ctx->memMgr, sizeof (PKIINTEGER));
	if (tbsCert->version == NULL) {
	    status =  TC_E_NOMEMORY;
	    break;
	}
	memset(tbsCert->version, TC_X509_VERSION_1, sizeof (PKIINTEGER));
        PKIPutIntVal(ctx->certasnctx, tbsCert->version, version);
    }

    /* serial number */
    PKIPutOctVal(ctx->certasnctx, &tbsCert->serialNumber, serial, serialLen);

    /* Add the signature info. to the certificate, according to 
       draft-ietf-pkix-ipki-part1-11.txt section 4.1.2.3 the param's
       will usually be NULL, param's are retreived from issuer's
       subjectPublicKeyInfo block.
       Also according to the draft, this should be same alg. OID
       as what is used for the localCert.signatureAlgorithm below. */
    if ((status = tc_set_alg(&tbsCert->signature, sigoid, 
                     sigoidlen, sigparm, sigparmlen, ctx)) != 0)
        break;
 
    /* issuer, create a local copy first */
    if ((status = CopyName(&localIssuer, issuerName, ctx)) != 0)
	break;
    tbsCert->issuer.CHOICE_field_type = localIssuer->CHOICE_field_type;
    tbsCert->issuer.data = localIssuer->data;
    /* now free just the upper level structure of the copy, free'ing the
       Cert will free the rest */
    TC_Free(ctx->memMgr, localIssuer);

    /* add the validity period */
    tbsCert->validity.notBefore.CHOICE_field_type = PKIID_UTCTime;
    if ((status = tc_encode_utctime(
          (PKIUTCTime **)&tbsCert->validity.notBefore.data,
	  notBefore, ctx)) != 0)
        break;

    tbsCert->validity.notAfter.CHOICE_field_type = PKIID_UTCTime;
    if ((status = tc_encode_utctime(
           (PKIUTCTime **)&tbsCert->validity.notAfter.data,
	   notAfter, ctx)) != 0)
        break;

    /* subject, create a local copy first */
    if ((status = CopyName(&localSubject, subjectName, ctx)) != 0)
	break;
    tbsCert->subject.CHOICE_field_type = localSubject->CHOICE_field_type;
    tbsCert->subject.data = localSubject->data;
    /* now free just the upper level structure of the local copy, free'ing
       the Cert will free the rest */
    TC_Free(ctx->memMgr, localSubject);

    /* extensions */
    if (extensions && version < TC_X509_VERSION_3) {
        status = TC_E_EXTENSION;
        break;
    }
    if (extensions != NULL) {
	if ((status = CopyExtList(&tbsCert->extensions, extensions, ctx)) != 0)
	    break;
    }
    else
	tbsCert->extensions = NULL;

    /* --- subjectPublicKeyInfo --- */

    /* According to draft-ietf-pkix-ipki-part1-06.txt section 7.3, for
       DSA the param does not have to be NULL, the subject's params
       may be different from the issuer's. */
/* TODO: change call to have new seperate argument? */
    if ((status = tc_set_alg(&tbsCert->subjectPublicKeyInfo.algorithm,
	              keyoid, keyoidlen, keyparm, keyparmlen, ctx)) != 0)
        break;
    PKIPutBitString(ctx->certasnctx,
		    &tbsCert->subjectPublicKeyInfo.subjectPublicKey,
		    pubkey, pubkeylen, 0);

    /* --- pack up the TBS portion of the cert --- */
    if ((status = tc_pack_tbscert(&ber, &berlen, localCert, ctx)) != 0)
        break;

    /* use the callback to sign the certificate */
    if ((status = 
            ctx->sign(&sig, &siglen, ber, berlen,
		      sigoid, sigoidlen, ctx->sigfuncdata,
		      ctx)) != 0) {
        status = TC_E_SIGNFAIL;
        break;
    }

    /* Add the signature to the certificate, according to 
       draft-ietf-pkix-ipki-part1-06.txt section 4.1.1.2 the params
       will usually be NULL, param's are retreived from issuer's
       subjectPublicKeyInfo block.
       Also according to the draft, this should be same alg. OID
       as what was used for the tbsCert->signature above. */
    if ((status = tc_set_alg(&localCert->signatureAlgorithm, 
                          sigoid, sigoidlen, sigparm, sigparmlen, ctx)) != 0)
        break;

    PKIPutBitString(ctx->certasnctx, &localCert->signature, sig, siglen, 0);

    /* pack up the certificate into a CERT structure */
    if ((status = tc_Certificate2CERT(cert, localCert, ctx)) != 0)
        break;

  } while (/*CONSTCOND*/0);

  /* clean-up */
  TC_Free(ctx->memMgr, sig);
  TC_Free(ctx->memMgr, ber);
  PKIFreeCertificate(ctx->certasnctx, localCert);

  return(status);
} /* tc_create_cert */


/*****
* tc_pack_cert
*
* Pack up the DER value for the actual certificate given a CERT
* structure. 
*
* Parameters
*   input
*	cert - a completed CERT structure
*
*   output
*	ptr - a pointer that will be set to point to the created
*	      buffer space containing the DER encoded Certificate
*	ptrlen - length of the allocated buffer
*
* Return values
*   0 - okay
*   TC_E_MEMORY
*   TC_E_PARSE
*
*****/
int tc_pack_cert (unsigned char **ptr, size_t *ptrlen, TC_CERT *cert,
		  TC_CONTEXT *ctx)
{
  int status = 0;

  if (ptr == NULL || ptrlen == NULL || cert == NULL)
      return TC_E_INVARGS;

  *ptrlen = PKISizeofCertificate (ctx->certasnctx, cert->asnCertificate, 1);
  *ptr = TC_Alloc(ctx->memMgr, *ptrlen);
  if (*ptr == NULL)
    return TC_E_NOMEMORY;

  (void)PKIPackCertificate (ctx->certasnctx,
			    *ptr, *ptrlen,
			    cert->asnCertificate, &status);
  if (status != 0)
  {
    TC_Free(ctx->memMgr, *ptr);
    *ptrlen = 0;
    return compiler2tc_error(status);
  }

  return 0;
}


/*****
 *
 * tc_certExtensionList
 *
 * Return a pointer to the extension list in the provided cert.  It
 * could be NULL.  This does not provide a copy, the user SHOULD NOT
 * free or modify this list.
 *
 * return
 *    0 - okay
 *    TC_E_INVARGS
 *    
 *
 *****/
int tc_certExtensionList(
    TC_ExtensionList **extList,
    const TC_CERT *cert,
    TC_CONTEXT *ctx)
{
    (void)ctx; /* for future use */

    if (cert == NULL || extList == NULL)
	return TC_E_INVARGS; 

    *extList = cert->tbsCertificate->extensions;

    return 0;

} /* tc_certExtensionList */

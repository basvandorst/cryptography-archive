/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include <time.h>

#include "tc.h"
#include "cms.h"

#include "cms_proto.h"

/*****
*
* return
*   0 - okay
*   TC_E_INVARGS
*   TC_E_NOMEMORY
*   TC_E_PARSE

*****/
int tc_unpack_cert (
    TC_CERT **cert,
    unsigned char *ber, 
    const size_t berLen,
    TC_CONTEXT *ctx)
{
  int errorret = 0;
  TC_CERT *localcert;

  if (cert == NULL || berLen == 0)
    return TC_E_INVARGS;
  *cert = NULL;

  localcert = (TC_CERT *)TC_Alloc(ctx->memMgr, sizeof(TC_CERT) );
  if (localcert == (TC_CERT *)0)
    return TC_E_NOMEMORY;

  /* decode the certificate and check to make sure it is valid ASN,
     this provides a fully unpacked ASN.1 Certificate structure */
  (void)PKIUnpackCertificate(ctx->certasnctx, &(localcert->asnCertificate),
			     ber, berLen, &errorret);
  if (errorret != 0) {
    TC_Free(ctx->memMgr, localcert);
    return TC_E_PARSE;
  }

  /* unpack the outer certificate parts, this leaves the tbsCertficate
     DER available for quicker signature checks later */
  (void)PKIUnpackXCertificate(ctx->certasnctx,
			      &(localcert->cert), ber, berLen, &errorret);
  if (errorret != 0) {
    PKIFreeCertificate(ctx->certasnctx, localcert->asnCertificate);
    TC_Free(ctx->memMgr, localcert);
    return TC_E_PARSE;
  }

  /* assign the tbsCertificate pointer */
  localcert->tbsCertificate = &localcert->asnCertificate->tbsCertificate;

  *cert = localcert;
  return 0;
} /* tc_unpack_cert */

/*****
*
* return
*   0 - okay
*   TC_E_INVARGS
*   TC_E_NOMEMORY
*
* Note: if the cert is already on the list, then its okay for the
* caller to free it.  But, if the cert is not on the list it cannot
* be free'd since its pointer is only copied.  
* TODO: We need a copy TC_CERT call here.
*
*****/
int tc_add_cert(TC_CERT *cert, TC_CONTEXT *ctx)
{
  TC_CERT *pc;
  TC_LIST *tmplist;
  int status = 0;

  if (cert == NULL || ctx == NULL)
      return TC_E_INVARGS;

  /* make sure this cert isn't already on the list */
  for (tmplist = ctx->roots; tmplist; tmplist = tmplist->next)
  {
    pc = (TC_CERT *)tmplist->data;

    /* see if the serial numbers match */
    if (tc_compare_serial(
	    &pc->tbsCertificate->serialNumber,
	    &cert->tbsCertificate->serialNumber) == 1) {

      /* see if the issuers are the same */
      if (tc_compare_dname(&pc->tbsCertificate->issuer, 
                           &cert->tbsCertificate->issuer,
			   ctx)) {
	return 0;
      }
    }
  }

  /* add the decoded certificate to the list of root certificates */
  tmplist = TC_Alloc(ctx->memMgr, sizeof(TC_LIST));
  if (tmplist == NULL) 
      return status;

  tmplist->data = (void *)cert;
  tmplist->next = ctx->roots;
  ctx->roots = tmplist;
 
  return 0; /* signal success */
}

/*****
*
* return
*   TC_E_PARSE
*   TC_E_NOMEMORY
*   TC_E_INVARGS
*   0 - okay
*
*****/
int tc_import_cert (unsigned char *ber, size_t berlen, TC_CONTEXT *ctx)
{
  TC_CERT *cert;
  int status = 0;

  if (!ber || !ctx)
    return TC_E_INVARGS;

  if ( (status = tc_unpack_cert(&cert, ber, berlen, ctx)) != 0)
    return status; /* unable to parse certificate */

  return (tc_add_cert(cert, ctx));
}

/*****
*
* TODO- fix so that it also takes the serial number?
*
* return
*   TC_E_INVARGS
*   TC_E_NOTFOUND
*   0 - okay
*****/
int tc_find_cert(TC_CERT **cert, TC_Name *issuer, TC_CONTEXT *ctx)
{
  TC_LIST *certs;

  if (cert == NULL || ctx == NULL || issuer == NULL)
      return TC_E_INVARGS;

  certs = ctx->roots;
  *cert = NULL;

  for (; certs; certs = certs->next)
    if (tc_compare_dname(issuer, 
                         &((TC_CERT *)certs->data)->tbsCertificate->subject,
			 ctx)) {
	*cert = (TC_CERT *)certs->data;
	return 0;
    }

  return TC_E_NOTFOUND;
}

/*****
*
* return
*   0
*   TC_E_INVARGS
*   TC_E_NOISSUER
*
*****/
int tc_find_issuer(TC_CERT **issuer, TC_CERT *cert, TC_CONTEXT *ctx)
{
  int selfSigned;

  if (issuer == NULL || cert == NULL || ctx == NULL)
      return TC_E_INVARGS;

  /* check to see if the cert is self-signed */
  selfSigned = tc_is_self_signed(cert, ctx);

  if (selfSigned)
     *issuer = cert;
  else if (tc_find_cert(issuer, 
                         &cert->tbsCertificate->issuer, ctx) != 0) {
     return TC_E_NOISSUER; /* can't verify without the issuer's public key! */
  }

  return 0;
}


/*****
*
* tc_validate_cert
*
* Given an "end-entity" certificate (the cert. at the bottom of a chain)
* verify its signature up through to a self-signed certificate.  All
* certs in the chain except for the end-entity cert. must be have been
* imported into the context.
*
* If any extension handlers have been registered, these handlers are
* executed for every certificate.
*
* parameters
*   cert - certificate to verify, the "end-entity" cert.
*   ctx	- application context holding root certs, ext handlers, and crls
*   flags - TC_F_ERRNOCRL generate error if there is no CRL for each of 
*                         the CA certificates
*	    TC_F_NOERRDATE no error on invalid date in cert
*
* returns:
*	0	success
*	TC_E_INVARGS
*	TC_E_NOISSUER
*	TC_E_INVSIG
*	TC_E_INVDATE
*	TC_E_CERTREVOKED
*	TC_E_NOCRL
*	TC_E_CRITICAL
*	TC_E_EXTENSION
*****/
int tc_validate_cert (TC_CERT *cert, int flags, TC_CONTEXT *ctx)
{
  TC_CERT *issuer;
  int status = 0;

  if (!cert || !ctx || !ctx->verify)
    return TC_E_INVARGS; /* invalid arguments */

  /* get this cert's issuer */
  if ((status = tc_find_issuer(&issuer, cert, ctx)) != 0)
      return status;

  if ((status = tc_validate_one_cert(cert, flags, ctx)) != 0)
      return status;

  /* deal with the extensions */
  if ((status = tc_process_extensions(cert->tbsCertificate->extensions,
				      cert, ctx)) != 0)
    return status; /* Error handling extensions */

  if ( tc_is_self_signed(cert, ctx) ) {
    /* we are done */
    return 0;
  }

  /* validate next cert. up the chain */
  return (tc_validate_cert (issuer, flags, ctx));
}


/*****
*
* tc_validate_one_cert
*
* TODO: check that critical extensions have handlers, but don't execute
* them!  Currently it is the user's responsibility to process all extensions
* if they use this call.
*
* Given a certificate verify its signature. The cert's issuer certificate
* must have been imported into the context or the certificate must
* be self-signed.  Extensions are not processed.
*
* parameters
*   cert - certificate to verify
*   flags - TC_F_ERRNOCRL generate error if there is no CRL for 
*                         the CA's certificates
*	    TC_F_NOERRDATE no error on invalid date in cert
*           TC_F_ENFCRLDATE generate error if the nextUpdate date on the CRL
*                         is old
*   ctx	- application context holding root certs, ext handlers, and crls
*
* returns:
*	0	success
*	TC_E_INVSIG
*	TC_E_NOISSUER
*	TC_E_INVARGS
*	TC_E_INVDATE
*	TC_E_CERTREVOKED
*	TC_E_NOCRL
*       TC_E_INVDATECRL
*
*****/
int tc_validate_one_cert(TC_CERT *cert, int flags, TC_CONTEXT *ctx)
{
  TC_CERT *issuer;
  time_t now;
  time_t notBefore;
  time_t notAfter;
  unsigned char *params;
  size_t paramLen;
  int status;

  if (!cert || !ctx || !ctx->verify)
    return TC_E_INVARGS; /* invalid arguments */

  if ((status = tc_find_issuer(&issuer, cert, ctx)) != 0)
      return status;

  /* TODO: make sure the user's 2 signature alg OIDs agree and that the
     encryption part agree's with the issuer's key OID */

  /* TODO: deal with finding the parameters, for now assuming they are
     in the issuer's subjectPublicKeyInfo */
  if (issuer->tbsCertificate->subjectPublicKeyInfo.algorithm.parameters != NULL) {
      params = issuer->tbsCertificate->subjectPublicKeyInfo.algorithm.parameters->val;
      paramLen = issuer->tbsCertificate->subjectPublicKeyInfo.algorithm.parameters->len;
  }
  else {
      params = NULL;
      paramLen = 0;
  }

  /* use the provided call-back routine */
  status = ctx->verify(
      cert->cert->tbsCertificate.val, 
      cert->cert->tbsCertificate.len,
      cert->cert->signatureAlgorithm.algorithm.val,
      cert->cert->signatureAlgorithm.algorithm.len,
      params, paramLen,
      cert->cert->signature.val,
      cert->cert->signature.len,
      issuer->tbsCertificate->subjectPublicKeyInfo.subjectPublicKey.val,
      issuer->tbsCertificate->subjectPublicKeyInfo.subjectPublicKey.len,
      issuer,
      ctx->verfuncdata,
      ctx);

  if (status)
    return TC_E_INVSIG; /* invalid signature */

  /* check the validity period */
  notBefore = tc_decode_UTCTime((PKIUTCTime *) cert->tbsCertificate->validity.notBefore.data);
  notAfter = tc_decode_UTCTime ((PKIUTCTime *) cert->tbsCertificate->validity.notAfter.data);

  if ((flags & TC_F_NOERRDATE) == 0) {
    now = time (NULL);

    if (now < notBefore || now > notAfter) {
      /* Time is outside validity date */
      return TC_E_INVDATE;
    }
  }

  /* make sure this certificate is not on the issuer's CRL */
  if ((status = tc_check_crl (ctx, cert, flags)) != 0)
    return status;

  /* success */
  return 0;

} /* tc_validate_one_cert */

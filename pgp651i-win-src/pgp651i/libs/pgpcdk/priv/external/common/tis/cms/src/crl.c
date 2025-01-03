/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include <time.h>

#include "tc.h"
#include "cms.h"

void tc_free_crl(TC_CertificateList *crl, TC_CONTEXT *ctx)
{
    PKIFreeCertificateList(ctx->certasnctx, crl);
    return;
}

/****
 *
 * tc_create_crl
 *
 * return
 *    0 okay
 *    TC_E_INVARGS
 *    TC_E_NOMEMORY
 *    TC_E_INVDATE
 *
 *****/
int tc_create_crl (TC_CertificateList **crl,
		   const unsigned char *alg,	/* alg to sign crl */
		   size_t algLen,		/* len of `alg' */
		   const unsigned char *algParm,
		   size_t algParmLen,
		   TC_Name *issuer,	/* crl issuer */
		   time_t nextUpdate,	/* next crl update expected */
		   TC_ExtensionList *crlExts, /* CRL ext's */
                   TC_CONTEXT *ctx)
{
    int status = 0;
    PKICertificateList *localCRL = NULL;
    TC_Name *localIssuer = NULL;

    do {
 
        if (crl == NULL || issuer == NULL || nextUpdate < 0) {
            status = TC_E_INVARGS;
            break;
        }

        localCRL = PKINewCertificateList(ctx->certasnctx);
        if (localCRL == NULL) {
            status = TC_E_NOMEMORY;
            break;
        }

	/* set the version, if there are extensions then its v2, otherwise
	   leave NULL */
	if (crlExts != NULL) {
	    localCRL->tbsCertList.version = PKINewINTEGER(ctx->certasnctx);
	    if (localCRL->tbsCertList.version == NULL) {
		status = TC_E_NOMEMORY;
		break;
	    }
	    PKIPutIntVal(ctx->certasnctx, localCRL->tbsCertList.version,
			 TC_X509_VERSION_2);
	}

        if ((status = tc_set_alg(&localCRL->tbsCertList.signature,
		  alg, algLen, algParm, algParmLen, ctx)) != 0)
            break;
        if ((status = tc_set_alg(&localCRL->signatureAlgorithm,
		  alg, algLen, algParm, algParmLen, ctx)) != 0)
            break;

	/* issuer, create a local copy first */
	if ((status = CopyName(&localIssuer, issuer, ctx)) != 0)
	    break;
        localCRL->tbsCertList.issuer.CHOICE_field_type = 
                        localIssuer->CHOICE_field_type;
        localCRL->tbsCertList.issuer.data = localIssuer->data;
	/* now free just the upper level structure, free'ing the Cert will
           free the rest */
	TC_Free(ctx->memMgr, localIssuer);

	/* this update */
        localCRL->tbsCertList.thisUpdate.CHOICE_field_type =
                                        PKIID_UTCTime;
        if ((status = tc_encode_utctime (
               (PKIUTCTime **)&localCRL->tbsCertList.thisUpdate.data, 
               time(NULL), ctx) ) != 0)
           break;
 
	/* next update */
	if (nextUpdate != 0) { /* its optional */

	    /* the value for nextUpdate must be later than current time */
	    if (nextUpdate <= time(NULL)) {
		status = TC_E_INVDATE;
		break;
	    }

	    localCRL->tbsCertList.nextUpdate =
		PKINewTime(ctx->certasnctx);
	    localCRL->tbsCertList.nextUpdate->CHOICE_field_type =
                                        PKIID_UTCTime;
	    if ((status = tc_encode_utctime (
		     (PKIUTCTime **)&localCRL->tbsCertList.nextUpdate->data, 
                     nextUpdate, ctx)) != 0)
		break;
	}

	/* extensions */
	if (crlExts != NULL) {
	    if ((status = CopyExtList(&localCRL->tbsCertList.crlExtensions,
				      crlExts, ctx)) != 0)
		break;
	}
	else
	    localCRL->tbsCertList.crlExtensions = NULL;

    } while (0);
 
    if (status == 0)
        *crl = localCRL;
    else
        *crl = NULL;

    return status;
} /* tc_create_crl */

int tc_add_to_crl(
    TC_CertificateList *crl,          /* crl to update */
    unsigned char *serialNumber,  /* serial num of revoked cert */
    size_t serialLength,
    time_t revocationDate,
    TC_ExtensionList *entryExts,  /* CRL entry exts */
    TC_CONTEXT *ctx)
{
    PKIRevokedCertificate *revokedCert;
    int crlCount;
    int status;

    if (crl == NULL)
	return TC_E_INVARGS;

    if (!crl->tbsCertList.revokedCertificates) {
	crl->tbsCertList.revokedCertificates =
	              PKINewRevokedCertificates(ctx->certasnctx);
	if (crl->tbsCertList.revokedCertificates == NULL)
	    return TC_E_NOMEMORY;
    }

    revokedCert = PKINewRevokedCertificate(ctx->certasnctx);
    if (revokedCert == NULL) {
	/* don't really need to free crl->tbsCertList.revokedCertificates
	   here since the user could try again, or there may be other
	   entries already on the list and we don't want the user to
	   have to start from scratch */
	return TC_E_NOMEMORY;
    }

    crlCount = crl->tbsCertList.revokedCertificates->n;
    crl->tbsCertList.revokedCertificates->elt[crlCount] = revokedCert;
    crl->tbsCertList.revokedCertificates->n++;

    /* userCertificate field */
    PKIPutOctVal(ctx->certasnctx, &revokedCert->userCertificate,
		 serialNumber, serialLength);

    /* revocationDate field */
    revokedCert->revocationDate.CHOICE_field_type = PKIID_UTCTime;
    status = tc_encode_utctime(
	(PKIUTCTime **)&revokedCert->revocationDate.data,
	revocationDate, ctx);
    if (status != 0)
	return status;

    /* crlEntryExtensions field */
    if (entryExts != NULL) {

	/* first make sure the version is v2, following PKIX, if its
	   there assume its v2 */
	if (crl->tbsCertList.version == NULL) {
	    crl->tbsCertList.version = PKINewINTEGER(ctx->certasnctx);
	    if (crl->tbsCertList.version == NULL) {
		return TC_E_NOMEMORY;
	    }
	    PKIPutIntVal(ctx->certasnctx, crl->tbsCertList.version,
			 TC_X509_VERSION_2);
	}

	if ((status = CopyExtList(&revokedCert->crlEntryExtensions,
				      entryExts, ctx)) != 0)
		return status;
    }

    return status;
}

/*****
*
* return
*	TC_E_INVARGS
*	TC_E_NOMEMORY
*	TC_E_PARSE
*
*****/
static int tc_pack_tbscertlist(unsigned char **ptr,
			 size_t *ptrlen,
			 TC_CertificateList *crl,
			 TC_CONTEXT *ctx)
{
  int error = 0;

  if (!ptr || !ptrlen || !crl)
    return TC_E_INVARGS;

  *ptrlen = PKISizeofTBSCertList(ctx->certasnctx, &crl->tbsCertList, 1);
  *ptr = TC_Alloc(ctx->memMgr, *ptrlen);
  if (*ptr == NULL)
    return TC_E_NOMEMORY;

  (void)PKIPackTBSCertList(ctx->certasnctx, 
			   *ptr, *ptrlen, &crl->tbsCertList, &error);

  return compiler2tc_error(error);
}

int tc_pack_crl(
    unsigned char **ptr, 
    size_t *ptrlen, 
    TC_CertificateList *crl,
    TC_CONTEXT *ctx)
{
  int error = 0;

  if (!ptr || !ptrlen || !crl)
    return TC_E_INVARGS;

  *ptrlen = PKISizeofCertificateList (ctx->certasnctx, crl, 1);
  *ptr = TC_Alloc(ctx->memMgr, *ptrlen);
  if (*ptr == NULL)
    return TC_E_NOMEMORY;

  (void)PKIPackCertificateList (ctx->certasnctx, 
				*ptr, *ptrlen, crl, &error);

  return compiler2tc_error(error);
}

int tc_unpack_crl(TC_CertificateList **crl, unsigned char *data,
		  size_t dataLen,
		  TC_CONTEXT *ctx)
{
  int error = 0;

  (void)PKIUnpackCertificateList(ctx->certasnctx, 
				 crl, data, dataLen, &error);
  if (error != 0)
      return compiler2tc_error(error);
  return 0;
}

/*****
 *
 * verify the signature over a CRL
 *
 * return
 *   0 - okay, signature valid
 *   TC_E_INVARGS
 *   TC_E_NOTFOUND couldn't find the issuer's cert in the context
 *   TC_E_INVSIG
 *
 *****/
static int tc_verify_crl (
    TC_CertificateList *crl,
    TC_CONTEXT *ctx)
{
    unsigned char *asnptr= NULL;
    size_t asnlen;
    int error = 0;
    TC_CERT *issuer;
    unsigned char *params;
    size_t paramLen;
    PKIRevokedCertificate *revdCertEntry;
    PKIRevokedCertificates *revdCerts;
    int i;

    if (!crl || !ctx || !ctx->verify)
	return TC_E_INVARGS;

  do {

    /* need to find the crl issuer's certificate for verification */
    error = tc_find_cert (&issuer, &crl->tbsCertList.issuer, ctx);
    if (error != 0)
	break;

    error = tc_pack_tbscertlist(&asnptr, &asnlen, crl, ctx);
    if (error != 0)
	break;

    /* TODO: make sure the CRL's 2 sig alg OIDs agree and that the encryption
       algorithm agree's with the issuer's key alg. */

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

    error = ctx->verify(
        asnptr, asnlen,
	crl->signatureAlgorithm.algorithm.val,
	crl->signatureAlgorithm.algorithm.len,
        params, paramLen,
	crl->signature.val,
	crl->signature.len,
	issuer->tbsCertificate->subjectPublicKeyInfo.subjectPublicKey.val,
	issuer->tbsCertificate->subjectPublicKeyInfo.subjectPublicKey.len,
	issuer,
	ctx->verfuncdata,
	ctx);

    if (error != 0) {
	error = TC_E_INVSIG;
	break;
    }

    /* check for unhandled CRL extensions */
    if ((error = tc_process_extensions(crl->tbsCertList.crlExtensions,
				       crl, ctx)) != 0)
	break;

    revdCerts = crl->tbsCertList.revokedCertificates;
    if (revdCerts == NULL)
	break; /* no revd certs in this crl */

    /* check for unhandled crlEntry extensions */
    for (i = 0; i < revdCerts->n; i++) {
	revdCertEntry = revdCerts->elt[i];
	error = tc_process_extensions(revdCertEntry->crlEntryExtensions,
				      crl, ctx);
	if (error != 0)
	    break;
    }

    /* note, break in 'for' above will end up here, if new code is added
       here, need to add (error != 0) check... */

  } while (0);

    if (asnptr != NULL)
	TC_Free(ctx->memMgr, asnptr);

    return error;
}

/*****
*
* return
*   0 - okay
*   TC_E_INVARGS
*   TC_E_PARSE
*
*****/
int tc_import_crl (unsigned char *crlData, 
	size_t crlDataLen,
	TC_CONTEXT *ctx,
	int flags)
{
  int status = 0;
  TC_CertificateList *crl;

  if (ctx == NULL || crlData == NULL || crlDataLen == 0)
      return TC_E_INVARGS;

  if ((status = tc_unpack_crl(&crl, crlData, crlDataLen, ctx)) != 0) {
      return status;
  }

  /* note: do not free crl on success since it's pointer is copied into the
     context... */
  status = tc_add_crl(crl, ctx, flags);
  if (status != 0)
      tc_free_crl(crl, ctx);

  return (status);
}

/*****
*
* return
*	TC_E_INVARGS
*	TC_E_NOMEMORY
*	TC_E_PARSE
*	TC_E_SIGNFAIL
* 
*****/
int tc_sign_crl (TC_CertificateList *crl,
		 TC_CONTEXT *ctx)
{
  int r = 0;
  unsigned char *ptr;
  size_t ptrlen;
  size_t tbslen;

  if (!crl || !ctx || !ctx->sign)
    return TC_E_INVARGS;

/* TODO: possible memory leak, if you are provided with an existing CRL
   that is just being updated, then it will already have data in the signature.
   Do we need to check for and remove this memory? */
  r = tc_pack_tbscertlist(&ptr, &ptrlen, crl, ctx);
  if (!r)
  {
    r = ctx->sign(&crl->signature.val, &tbslen,
                  ptr, ptrlen, 
		  crl->signatureAlgorithm.algorithm.val,
		  crl->signatureAlgorithm.algorithm.len,
		  ctx->sigfuncdata, ctx);
    if (r == 0) {
      crl->signature.len = tbslen;
    }
    else {
	r = TC_E_SIGNFAIL;
    }
    TC_Free(ctx->memMgr, ptr);
  }
  return r;
}

/*****
* 
* Check to see if the specified certificate is in the CRL, assumes there
* is only one CRL for an issuer in context and only checks the first
* one in the list if there are more than 1.
*
* parameters
*   input
*       cert - certificate to find in CRL
*	flags - TC_F_ERRNOCRL
*		     generate an error if the issuer CRL is not present
*               TC_F_ENFCRLDATE
*                    generate an error if the 'nextUpdate' field is past
*
* returns:
*	0			no error
*	TC_E_CERTREVOKED	certificate was revoked by issuer
*	TC_E_NOCRL		issuer CRL is not present
*       TC_E_INVDATECRL         the nextUpdate field is past
*
*****/
int tc_check_crl(
    TC_CONTEXT *ctx,
    TC_CERT *cert,
    int flags)
{
    TC_LIST *crlentry;
    PKICertificateList *certList;
    int j;

    /* try to find the issuer's crl entry */
    for (crlentry = ctx->crl; crlentry; crlentry = crlentry->next) {
	certList = (PKICertificateList *)crlentry->data;

	if (tc_compare_dname (&certList->tbsCertList.issuer, 
			      &cert->tbsCertificate->issuer,
			      ctx)) {

	    /* see if this CRL's nextUpdate value is older than "now" */
	    if (flags & TC_F_ENFCRLDATE)
		if (tc_get_choiceoftime(certList->tbsCertList.nextUpdate,
					ctx) <=
		    time(NULL) )
		    return TC_E_INVDATECRL;

	    /* found the CRL, does it have a cert. list? */
	    if (certList->tbsCertList.revokedCertificates) {

		/* see if this certificate is specified in the list */
		for (j = 0; j < certList->tbsCertList.revokedCertificates->n; 
		     j++) {

		    /* compare serial numbers */
		    if (tc_compare_serial(
	  &certList->tbsCertList.revokedCertificates->elt[j]->userCertificate,
	                       &cert->tbsCertificate->serialNumber) == 1) {

			/* found entry, return error that cert is revoked */
			return TC_E_CERTREVOKED;
		    }
		} /* for each serial # in crl */

	    }
	    return 0; /* crl is present, but cert is not specified */

	} /* if issuer name matches */
    } /* for each crl */

    /* issuer crl not present */
    return ((flags & TC_F_ERRNOCRL) ? TC_E_NOCRL : 0);
}

/*****
*
* tc_delete_crl
*
* Remove an issuer's CRL from the context, if the CRL is not present
* return an error code.  Assumes only one CRL from issuer is present,
* which should be the case since tc_import_crl and tc_add_crl both
* ensure this.
*
* parameters
*   input
*	issuer - the issuer's name
*	ctx - the context to remove the CRL from
*
* return
*    0 - okay
*    TC_E_INVARGS - provided args were NULL
*    TC_E_NOTFOUND - a crl for the issuer was not found
*
*****/
int tc_delete_crl(TC_Name *issuer, TC_CONTEXT *ctx)
{
    TC_LIST *crlList, *last = NULL;
    PKICertificateList *crl;
    int status = TC_E_NOTFOUND;

    if (issuer == NULL || ctx == NULL)
	return TC_E_INVARGS;

    for (crlList = ctx->crl; crlList; 
         last = crlList, crlList = crlList->next) {
        
        crl = (PKICertificateList *)crlList->data;

        if (tc_compare_dname (&crl->tbsCertList.issuer, issuer, ctx) == 1) {
            if (last)
                last->next = crlList->next;
            else
                ctx->roots = crlList->next;
	    tc_free_crl(crl, ctx);
	    TC_Free(ctx->memMgr, crlList);
            status = 0;
            break;
        } 
    }

    return (status);

} /* tc_delete_crl */

/*****
*
* tc_add_crl
*
* Add a crl to a context.  If a crl from the same issuer is already
* present, the one with the newest date is used.
*
* parameters
*   input
*	crl - the crl to add
*	ctx - the context
*       flags - TC_F_ENFCRLDATE
*                    generate error if 'nextUpdate' field is later than "now"
*
* return
*   0 - okay
*   TC_E_INVARGS - the provided crl or ctx are NULL
*   TC_E_INVDATECRL - the crl's nextUpdate field is old
*   TC_E_NOTFOUND - the crl issuer's cert. is not in context
*   TC_E_INVSIG - the provided crl does not have a valid signature
*
*****/
int tc_add_crl(TC_CertificateList *newcrl, TC_CONTEXT *ctx, int flags)
{
  TC_LIST *list = NULL;
  int status = 0;
  TC_LIST *crlList;
  PKICertificateList *crl = NULL;

  do {

    if (ctx == NULL || newcrl == NULL) {
	status =  TC_E_INVARGS;
	break;
    }

    if (flags & TC_F_ENFCRLDATE)
	if (tc_get_choiceoftime(newcrl->tbsCertList.nextUpdate, ctx) <=
	    time(NULL)) {
	    status = TC_E_INVDATECRL;
	    break;
	}

    if((status = tc_verify_crl(newcrl, ctx)) != 0)
	break;
    
    /* look for a CRL from this issuer in current list */
    for (crlList = ctx->crl; crlList; crlList = crlList->next) {
        crl = (PKICertificateList *)crlList->data;
	if (tc_compare_dname (&crl->tbsCertList.issuer,
			      &newcrl->tbsCertList.issuer, ctx) == 1)
	    break;
    }

    /* if one is already there */
    if (crlList != NULL) {

        /* compare the dates to see which one is newer, replace the
           current one if necessary */
	if (tc_compare_time(&newcrl->tbsCertList.thisUpdate,
                            &crl->tbsCertList.thisUpdate, ctx) > 0) {
	    tc_free_crl((PKICertificateList *)crlList->data, ctx);
	    crlList->data = (void *)newcrl;
	}
    }

    /* if its a new one */
    else {
	list = TC_Alloc(ctx->memMgr, sizeof(TC_LIST) );
        if (list == NULL)
	    break;

        list->data = (void *)newcrl;
        list->next = ctx->crl;
        ctx->crl = list;
    }

  } while(/*CONSTCOND*/0);

    /* clean up */
    if (status != 0) {
	TC_Free(ctx->memMgr, list);
    }

    return status;

} /* tc_add_crl */


/****
 *
 * tc_reset_crl_times()
 *
 * Reset the thisUpdate time in the CRL to the current time, and
 * set the nextUpdate time to the current time plus the number of
 * days provided by the user.
 *
 * parameters
 *   input
 *       crl - a CRL
 *       nextUpdateDays - the number of days until next update, must be
 *                 positive number, 0 means no nextUpdate value
 *
 *   output
 *
 * return
 *
 *   TC_E_INVARGS
 *   TC_E_NOMEMORY
 *
 *****/
int tc_reset_crl_times(
    TC_CertificateList *crl,
    int nextUpdateDays,
    TC_CONTEXT *ctx)
{
    int status = 0;
    int now;

    if (ctx == NULL || crl == NULL ||
	nextUpdateDays < 0)
	return TC_E_INVARGS;

    now = (int)time(NULL);

    do {

        /* reset thisUpdate on the CRL */
        PKIFreeUTCTime(ctx->certasnctx,
		   (PKIUTCTime *)crl->tbsCertList.thisUpdate.data);

        status = tc_encode_utctime (
            (PKIUTCTime **)&crl->tbsCertList.thisUpdate.data,
	    now, ctx);
	if (status != 0)
	    break;

	/* deal with nextUpdate:
	       current value and user wants none - free current
	       no current and user wants new - allocate and fill in
	       current value and user wants new - clear and fill in
	       no current and user wants none - do nothing
	 */
	if (crl->tbsCertList.nextUpdate != NULL &&
	    nextUpdateDays == 0) {
	    PKIFreeTime(ctx->certasnctx, crl->tbsCertList.nextUpdate);
	    crl->tbsCertList.nextUpdate = NULL;
	}

	else if (crl->tbsCertList.nextUpdate == NULL &&
	         nextUpdateDays != 0) {
	    crl->tbsCertList.nextUpdate = 
		    PKINewTime(ctx->certasnctx);
	    crl->tbsCertList.nextUpdate->CHOICE_field_type =
                                        PKIID_UTCTime;
	    status = tc_encode_utctime (
			    (PKIUTCTime **)&crl->tbsCertList.nextUpdate->data,
			    now + nextUpdateDays * 86400,
			    ctx);
	    if (status != 0)
		break;
	}

	else if (crl->tbsCertList.nextUpdate != NULL &&
		 nextUpdateDays != 0) {
	    PKIFreeUTCTime(ctx->certasnctx,
		    (PKIUTCTime *)crl->tbsCertList.nextUpdate->data);
	    status = tc_encode_utctime (
			    (PKIUTCTime **)&crl->tbsCertList.nextUpdate->data,
			    now + nextUpdateDays * 86400,
			    ctx);
	    if (status != 0)
		break;
	}


    } while(0);

    return status;

} /* tc_reset_crl_times */

/*****
 *
 * tc_crlExtensionList
 *
 * Return a pointer to the CRL extension list in the provided CRL.  It
 * could be NULL.  This does not provide a copy, the user SHOULD NOT
 * free or modify this list, use tc_free_crl instead.
 *
 * return
 *    0 - okay
 *    TC_E_INVARGS
 *    
 *****/
int tc_crlExtensionList(
    TC_ExtensionList **extList,
    const TC_CertificateList *crl,
    TC_CONTEXT *ctx)
{
    (void)ctx; /* for future use */

    if (crl == NULL || extList == NULL)
	return TC_E_INVARGS; 

    *extList = crl->tbsCertList.crlExtensions;

    return 0;

} /* tc_crlExtensionList */

/*****
 *
 * tc_crlEntryExtList
 *
 * Return a pointer to the CRL entry extension list in the CRL entry
 * that matches the provided serial number.  It could be NULL or if there
 * isn't an entry in the CRL with the provided serial number.  This
 * does not provide a copy, the user SHOULD NOT free or modify this list,
 * use tc_free_crl instead.
 *
 * return
 *    0 - okay
 *    TC_E_INVARGS
 *    
 *    
 *****/
int tc_crlEntryExtList(
    TC_ExtensionList **extList,
    const TC_CertificateList *crl,
    TC_SerialNumber *serialNumber,
    TC_CONTEXT *ctx)
{
    int i;
    PKIRevokedCertificate *revdCertEntry;
    PKIRevokedCertificates *revdCerts;

    (void)ctx; /* for future use */

    if (crl == NULL || extList == NULL)
	return TC_E_INVARGS; 
    *extList = NULL;

    revdCerts = crl->tbsCertList.revokedCertificates;

    if (revdCerts == NULL)
	return 0;

    for (i = 0; i < revdCerts->n; i++) {
	revdCertEntry = revdCerts->elt[i];
	if (tc_compare_serial(serialNumber,
			      &revdCertEntry->userCertificate) == 1) {
	    *extList = revdCertEntry->crlEntryExtensions;
	    break;
	}
    }

    return 0;

} /* tc_crlEntryExtList */

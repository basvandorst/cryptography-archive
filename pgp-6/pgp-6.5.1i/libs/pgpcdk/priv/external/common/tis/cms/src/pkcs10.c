/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "tc.h"
#include "cms.h"

#include "cms_proto.h"
#include <time.h>

static const unsigned char TC_ALG_SIGNING_TIME[] = {
  0x2a , 0x86 , 0x48 , 0x86 , 0xf7 , 0x0d , 0x01 , 0x09 , 0x05
};

#define TC_ALG_SIGNING_TIME_LEN 9

/*****
*
* returns
*   0 - okay
*   TC_E_NOMEMORY - malloc failed
*   TC_E_PARSE - could not pack up UTC time
*****/
int tc_set_signing_time (TC_Attributes *p, TC_CONTEXT *ctx)
{
  PKIUTCTime *t;
  unsigned char *buf;
  size_t buflen;
  int status = 0;
  int elementNum = 0;

  if (p == NULL || ctx == NULL)
      return TC_E_INVARGS;

  if ((status = tc_encode_utctime(&t, time (NULL), ctx)) != 0)
      return status;

  elementNum = p->n;
  p->n = p->n + 1;
  p->elt[elementNum] = TC_Alloc(ctx->memMgr, sizeof (PKIAttribute));
  if (p->elt[elementNum] == NULL)
    return TC_E_NOMEMORY;
  memset(p->elt[elementNum], 0, sizeof (PKIAttribute));

  PKIPutOctVal(ctx->certasnctx, &p->elt[elementNum]->type,
               TC_ALG_SIGNING_TIME, TC_ALG_SIGNING_TIME_LEN);

  p->elt[elementNum]->values.n = 1;
  if ((p->elt[elementNum]->values.elt[0] = 
                    TC_Alloc(ctx->memMgr, sizeof (PKIAttributeValue))) == NULL)
    return TC_E_NOMEMORY;

  buflen = PKISizeofUTCTime (ctx->certasnctx, t, PKITRUE);
  p->elt[elementNum]->values.elt[0]->len = buflen;
  if ((buf = TC_Alloc(ctx->memMgr, buflen)) == NULL)
    return TC_E_NOMEMORY;
  p->elt[elementNum]->values.elt[0]->val = buf;

  (void)PKIPackUTCTime (ctx->certasnctx, buf, buflen, t, &status);

  PKIFreeUTCTime (ctx->certasnctx, t);

  if (status != 0)
      return compiler2tc_error(status);
  return (0);
}

static int
CopyAttributes(PKIAttributes *tolist, PKIAttributes *fromlist,
	       TC_CONTEXT *ctx)
{
    int status = 0;
    int i;

    if (tolist == NULL || fromlist == NULL)
	return TC_E_INVARGS;

    tolist->n = fromlist->n;

    for (i = 0; i < tolist->n; i++) {

	tolist->elt[i] = TC_Alloc(ctx->memMgr, sizeof (PKIAttribute));
	if (tolist->elt[i] == NULL) {
	    status = TC_E_NOMEMORY;
	    break;
	}
	memset(tolist->elt[i], 0, sizeof (PKIAttribute));

	PKIPutOctVal(ctx->certasnctx,
		     &tolist->elt[i]->type,
		    fromlist->elt[i]->type.val, fromlist->elt[i]->type.len);

	tolist->elt[i]->values.n = 1;
	tolist->elt[i]->values.elt[0] = TC_Alloc(ctx->memMgr, 
						 sizeof(PKIAttributeValue));
	if (tolist->elt[i]->values.elt[0] == NULL) {
	    status = TC_E_NOMEMORY;
	    break;
	}

	tolist->elt[i]->values.elt[0]->len = 
	                  fromlist->elt[i]->values.elt[0]->len;
	tolist->elt[i]->values.elt[0]->val = 
	                  TC_Alloc(ctx->memMgr,
				   fromlist->elt[i]->values.elt[0]->len);
	if (tolist->elt[i]->values.elt[0]->val == NULL) {
	    status = TC_E_NOMEMORY;
	    break;
	}
	memcpy(tolist->elt[i]->values.elt[0]->val,
	       fromlist->elt[i]->values.elt[0]->val,
	       fromlist->elt[i]->values.elt[0]->len);

    } /* loop */

    if (status != 0) {
	for (i = 0; i < fromlist->n; i++) {
	    if (tolist->elt[i] != NULL) {
		if (tolist->elt[i]->type.val != NULL)
		    TC_Free(ctx->memMgr, tolist->elt[i]->type.val);
		if (tolist->elt[i]->values.elt[0] != NULL) {
		    if (tolist->elt[i]->values.elt[0]->val != NULL) {
			TC_Free(ctx->memMgr,
				tolist->elt[i]->values.elt[0]->val);
		    }
		    TC_Free(ctx->memMgr, tolist->elt[i]->values.elt[0]);
		}
		TC_Free(ctx->memMgr, tolist->elt[i]);
		tolist->elt[i] = NULL;
	    }
	} /* loop */
	tolist->n = 0;
    }

    return status;

} /* CopyAttributes */

/*****
*
* Create a PKCS #10 certificate request
*
* return
*   0 - okau
*   TC_E_INVARGS - provided arguments are not okay
*   TC_E_NOMEMORY - malloc problems
*   TC_E_DNAMEPARSE - incorrect subjectName
*   TC_E_PARSE - could not pack portions of the cert
*   TC_E_SIGNFAIL - signature failed
*
*****/
int tc_create_request (TC_CertificationRequest **req,
		       int version,
		       const unsigned char *sigoid,
		       size_t sigoidlen,
		       const unsigned char *sigparm,
		       size_t sigparmlen,
		       TC_Name *subjectName,
		       const unsigned char *keyoid,
		       size_t keyoidlen,
		       const unsigned char *pubkey,
		       size_t pubkeylen,
		       const unsigned char *keyparm,
		       size_t keyparmlen,
		       TC_Attributes *attrlist,
		       TC_CONTEXT *ctx)
{
  int status = 0;
  int errRet = 0;
  unsigned char *sig = NULL;
  unsigned char *ber = NULL;
  size_t berlen;
  size_t siglen;
  PKICertificationRequest *localreq = NULL;
  TC_Name *localSubject = NULL;

  do {

    if (!ctx || !ctx->sign || req == NULL || !subjectName) {
        status = TC_E_INVARGS;
        break;
    }

    *req = NULL;

    localreq = PKINewCertificationRequest(ctx->certasnctx);
    if (localreq == NULL) {
      status = TC_E_NOMEMORY;
      break;
    }

    if (version < 0 || version > 2) {
        status = TC_E_INVARGS;
        break;
    }

    PKIPutIntVal(ctx->certasnctx,
		 &localreq->certificationRequestInfo.version, version);

    /* subject, create a local copy first */
    if ((status = CopyName(&localSubject, subjectName, ctx)) != 0)
	break;
    localreq->certificationRequestInfo.subject.CHOICE_field_type = 
                            localSubject->CHOICE_field_type;
    localreq->certificationRequestInfo.subject.data = localSubject->data;
    /* now free just the upper level structure, free'ing the Cert will
       free the rest */
    TC_Free(ctx->memMgr, localSubject);

    /* --- subjectPublicKeyInfo --- */
    if ((status = tc_set_alg(
          &localreq->certificationRequestInfo.subjectPublicKeyInfo.algorithm,
          keyoid, keyoidlen, keyparm, keyparmlen, ctx)) != 0)
        break;

    PKIPutBitString(ctx->certasnctx,
     &localreq->certificationRequestInfo.subjectPublicKeyInfo.subjectPublicKey,
      pubkey, pubkeylen, 0);

    /* --- attributes --- */
    if (attrlist != NULL)
	CopyAttributes(&localreq->certificationRequestInfo.attributes,
		       attrlist, ctx);
    else 
	localreq->certificationRequestInfo.attributes.n = 0;

    berlen = PKISizeofCertificationRequestInfo(ctx->certasnctx,
                  &localreq->certificationRequestInfo, PKITRUE);
    if ((ber = TC_Alloc(ctx->memMgr, berlen)) == NULL) {
        status = TC_E_NOMEMORY;
        break;
    }

    (void)PKIPackCertificationRequestInfo (ctx->certasnctx,
			       ber, berlen, 
                               &localreq->certificationRequestInfo, &errRet);
    if (errRet != 0) {
        status = compiler2tc_error(errRet);
        break;
    }

    /* use the callback to sign the certificate */
    if ((status = ctx->sign (&sig, &siglen, 
                             ber, berlen,
			     sigoid, sigoidlen,
			     ctx->sigfuncdata, ctx)) != 0) {
        status = TC_E_SIGNFAIL;
        break;
    }

    if ((status = tc_set_alg (&localreq->signatureAlgorithm, 
                      sigoid, sigoidlen, 
                      sigparm, sigparmlen,
		      ctx)) != 0)
        break;

    PKIPutBitString(ctx->certasnctx, &localreq->signature, sig, siglen, 0);
        
  } while (/*CONSTCOND*/0);

  /* clean-up */
  TC_Free(ctx->memMgr, sig);
  TC_Free(ctx->memMgr, ber);

  *req = localreq;
  return (status);
} /* tc_create_request */

int tc_pack_request (unsigned char **ptr, size_t *ptrlen,
		     TC_CertificationRequest *cert,
		     TC_CONTEXT *ctx)
{
  int status = 0;

  *ptrlen = PKISizeofCertificationRequest (ctx->certasnctx, cert, 1);
  if ((*ptr = TC_Alloc(ctx->memMgr, *ptrlen)) == NULL)
    return TC_E_NOMEMORY;

  (void)PKIPackCertificationRequest(ctx->certasnctx,
				    *ptr, *ptrlen, cert, &status);
  if (status) {
    TC_Free(ctx->memMgr, *ptr);
    *ptrlen = 0;
    return compiler2tc_error(status);
  }
  return 0;
} /* tc_pack_request */

/*****
* tc_unpack_request()
*
* return
*    0 - okay
*    TC_E_INVARGS
*    TC_E_PARSE
*   
*****/
int tc_unpack_request(
    TC_CertificationRequest **request,
    unsigned char *ber, 
    size_t berLen,
    TC_CONTEXT *ctx)
{
  int error=0;

  if (ber == NULL || request == NULL)
      return TC_E_INVARGS;

  /* decode the certification request and check to make sure it is valid */
  (void)PKIUnpackCertificationRequest(ctx->certasnctx,
				      request, ber, berLen, &error);
  if (error != 0)
    return compiler2tc_error(error);

  return 0;
}

/*****
*
* tc_validate_request()
*
*	Verify the certificate request.
*
*    returns
*	0 - success
*	TC_E_INVARGS
*	TC_E_NOMEMORY
*	TC_E_PARSE
*	TC_E_INVSIG
*	
*****/
int tc_validate_request (TC_CertificationRequest *certreq, TC_CONTEXT *ctx)
{
  unsigned char *reqinfo;
  size_t	reqinfoLen;
  unsigned char *params;
  size_t paramLen;
  int		error = 0;

  if (certreq == NULL || ctx == NULL || ctx->verify == NULL)
      return TC_E_INVARGS;

  /* repack the certification request info for signature verification */
  reqinfoLen = PKISizeofCertificationRequestInfo(ctx->certasnctx,
                                   &certreq->certificationRequestInfo, 1);
  if ((reqinfo = TC_Alloc(ctx->memMgr, reqinfoLen)) == NULL) {
	return TC_E_NOMEMORY; /* out of memory */
  }

  (void)PKIPackCertificationRequestInfo(ctx->certasnctx, 
		   reqinfo, reqinfoLen, 
                   &certreq->certificationRequestInfo, &error);
  if (error != 0) {
	/* error while packing (shouldn't happen?) */
	TC_Free(ctx->memMgr, reqinfo);
	return compiler2tc_error(error); 
  }

  /* TODO: ensure the 2 sig alg. values in the Request agree. */

    if (certreq->certificationRequestInfo.subjectPublicKeyInfo.algorithm.parameters != NULL) {
      params = certreq->certificationRequestInfo.subjectPublicKeyInfo.algorithm.parameters->val;
      paramLen = certreq->certificationRequestInfo.subjectPublicKeyInfo.algorithm.parameters->len;
  }
  else {
      params = NULL;
      paramLen = 0;
  }

  error = ctx->verify(
    reqinfo, reqinfoLen,
    certreq->signatureAlgorithm.algorithm.val,
    certreq->signatureAlgorithm.algorithm.len,
    params, paramLen,
    certreq->signature.val,
    certreq->signature.len,
    certreq->certificationRequestInfo.subjectPublicKeyInfo.subjectPublicKey.val,
    certreq->certificationRequestInfo.subjectPublicKeyInfo.subjectPublicKey.len,
    NULL,
    ctx->verfuncdata,
    ctx);

  TC_Free(ctx->memMgr, reqinfo);
  if (error)
    return TC_E_INVSIG; /* invalid signature */

  return(0);
}


/*****
 *
 * tc_create_attrlist
 *
 * Create an empty Attributes structure.
 *
 * paramaters
 *   input/output
 *       list - a pointer to hold the created list
 *
 * return
 *    TC_E_NOMEMORY
 *
 *****/
int tc_create_attrlist(TC_Attributes **list, TC_CONTEXT *ctx)
{
    PKIAttributes *locallist = NULL;

    if (list == NULL)
	return TC_E_INVARGS;

    locallist = PKINewAttributes(ctx->certasnctx);
    if (locallist == NULL)
	return TC_E_NOMEMORY;

    *list = locallist;
    return 0;

} /* tc_create_attrlist */

/*****
 *
 * tc_add_attribute
 *
 * Add an attribute type and value to an attribute list.  Currently we
 * only support only one value per attribute type.  It is up to the user
 * to create the appropriate DER/BER data for the attribute's value based
 * on the OID chosen.
 *
 * parameters
 *   input
 *        oid - the oid of the attribute
 *        oidLen  - length of above
 *        attrDER - The DER (or BER) binary data representing the attribute's
 *                  value
 *        attrLen - length of above
 *
 *   output
 *        list - the list is updated with an entry containing the values
 *               provided above
 *
 * return
 *       TC_E_INVARGS
 *       TC_E_NOMEMORY
 *
 *****/
int tc_add_attribute(TC_Attributes *list,
		     unsigned char *oid,
		     size_t oidLen,
		     unsigned char *attrDER,
		     size_t attrLen,
		     TC_CONTEXT *ctx)
{
    int status = 0;
    int elementNum = 0;

    if (list == NULL || oid == NULL || attrDER == NULL)
	return TC_E_INVARGS;

    do {
	elementNum = list->n;
	list->n = list->n + 1;
	list->elt[elementNum] = TC_Alloc(ctx->memMgr, sizeof(PKIAttribute));
	if (list->elt[elementNum] == NULL) {
	    status = TC_E_NOMEMORY;
	    break;
	}
	memset(list->elt[elementNum], 0, sizeof(PKIAttribute));

	PKIPutOctVal(ctx->certasnctx, &list->elt[elementNum]->type,
		    oid, oidLen);

	list->elt[elementNum]->values.n = 1;
	list->elt[elementNum]->values.elt[0] =
	    TC_Alloc(ctx->memMgr, sizeof(PKIAttributeValue));
	if (list->elt[elementNum]->values.elt[0] == NULL) {
	    status = TC_E_NOMEMORY;
	    break;
	}

	list->elt[elementNum]->values.elt[0]->len = attrLen;
	list->elt[elementNum]->values.elt[0]->val =
	    TC_Alloc(ctx->memMgr, attrLen);
	if (list->elt[elementNum]->values.elt[0]->val == NULL) {
	    status = TC_E_NOMEMORY;
	    break;
	}
	memcpy(list->elt[elementNum]->values.elt[0]->val, attrDER, attrLen);

    } while(0);

    /* clean up */
    if (status != 0) {
	if (list->elt[elementNum] != NULL) {
	   if (list->elt[elementNum]->type.val != NULL)
	       TC_Free(ctx->memMgr, list->elt[elementNum]->type.val);
	   if (list->elt[elementNum]->values.elt[0] != NULL) {
	       if (list->elt[elementNum]->values.elt[0]->val != NULL) {
		   TC_Free(ctx->memMgr, 
			   list->elt[elementNum]->values.elt[0]->val);
	       }
	       TC_Free(ctx->memMgr, list->elt[elementNum]->values.elt[0]);
	   }
	   TC_Free(ctx->memMgr, list->elt[elementNum]);
	   list->elt[elementNum] = NULL;
	   list->n = elementNum - 1;
	}
    }

    return status;
    
} /* tc_add_attribute */

/******
 *
 * tc_find_attribute
 *
 * Find an attribute's value given the OID and the certificate request
 * with an attribute list.  If the attribute has more than one value, only
 * the first value will be returned.
 *
 * TODO: support multiple attribute values
 *
 * parameters
 *   input
 *        cert - the certification request
 *        oid - the oid of the attribute to look for
 *        oidLen - length of above
 *
 *   output
 *        attrDER - a copy of the attributes DER value is returned
 *        attrLen - length of above
 *
 * return
 *
 *****/
int tc_find_attribute(unsigned char **attrDER,
		      size_t *attrLen,
		      TC_CertificationRequest *cert,
		      unsigned char *oid,
		      size_t oidLen,
		      TC_CONTEXT *ctx)
{
    int i;
    PKIAttributes *attrlist;

    if (attrDER == NULL || attrLen == NULL || oid == NULL)
	return TC_E_INVARGS;

    attrlist = &cert->certificationRequestInfo.attributes;

    for (i = 0; i < attrlist->n; i++) {

	if ( attrlist->elt[i]->type.len == oidLen &&
	     memcmp(attrlist->elt[i]->type.val, oid, oidLen) == 0 ) {

	    *attrLen = attrlist->elt[i]->values.elt[0]->len;
	    *attrDER = TC_Alloc(ctx->memMgr, *attrLen);
	    if (*attrDER == NULL) {
		*attrLen = 0;
		return TC_E_NOMEMORY;
	    }
	    memcpy(*attrDER, attrlist->elt[i]->values.elt[0]->val, *attrLen);
	    break;
	}
    } /* loop */

    if (*attrDER == NULL)
	return TC_E_NOTFOUND;

    return 0;

} /* tc_find_attribute */

/*****
 *
 * tc_free_request
 *
 *****/
void
tc_free_request(TC_CertificationRequest *cr, TC_CONTEXT *ctx)
{
    if (cr != NULL)
	PKIFreeCertificationRequest(ctx->certasnctx, cr);

    return;
} /* tc_free_request */

/*****
 *
 * tc_free_attrlist
 *
 * Free the memory from the creation of an attribute list.
 *
 *****/
void
tc_free_attrlist(TC_Attributes *list, TC_CONTEXT *ctx)
{
    if (list != NULL)
	PKIFreeAttributes(ctx->certasnctx, list);

    return;
} /* tc_free_attrlist */

/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include <string.h>
#include <ctype.h>

#include "tc.h"
#include "cms.h"
#include "cms_proto.h"

static void TrimMiddleWS(char *str)
{
    char *ptr1 = str;
    char *ptr2, *ptr3;

    /* while there are chars */
    while (*ptr1 != '\0') {

	/* if we have a space */
	if (isspace((int) (*ptr1))) {

	    ptr2 = ptr1;

	    /* skip any other spaces after that */
	    while( isspace((int) (*ptr2)) )
		ptr2++;

	    /* if there was more than one white space char */
	    if (ptr2 > ptr1 + 1) {

		/* move the remaining chars over */
		ptr3 = ptr1 + 1;
		while (*ptr2 != '\0') {
		    *ptr3 = *ptr2;
		    ptr2++;
		    ptr3++;
		}
		*ptr3 = '\0';
	    }
	} /* if space */

	ptr1++;
    }

    return;
}

/****
 *
 * ComparePrintableString
 *
 * Compare 2 printable string values in a Dname.  The leading and trailing
 * whitespace is removed, extra white space in the string is removed, and
 * case does not matter.
 *
 * return
 *   1 - match
 *   0 - no match
 *
 *****/
static int ComparePrintableString(
    PKIAttributeValue value1,
    PKIAttributeValue value2,
    TC_CONTEXT *ctx)
{
    PKIPrintableString *a = NULL, *b = NULL;
    int error;
    char *astring = NULL, *astr;
    char *bstring = NULL, *bstr;
    int status = 0;

    do {

	(void)PKIUnpackPrintableString(ctx->certasnctx, &a,
				value1.val, value1.len, &error);
	if (error != 0 || a == NULL) {
	    status = -1;
	    break;
	}

	(void)PKIUnpackPrintableString(ctx->certasnctx, &b,
				value2.val, value2.len, &error);
	if (error != 0 || b == NULL) {
	    status = -1;
	    break;
	}

	astring = (char *)TC_Alloc(ctx->memMgr, a->len + 1);
	if (astring == NULL) {
	    status = -1;
	    break;
	}
	astr = astring;
	strncpy(astr, (char *)a->val, a->len);
	astr[a->len] = '\0';
	SKIPWS(astr);
	TrimWS(astr);
	TrimMiddleWS(astr);

	bstring = (char *)TC_Alloc(ctx->memMgr, b->len + 1);
	if (bstring == NULL) {
	    status = -1;
	    break;
	}
	bstr = bstring;
	strncpy(bstr, (char *)b->val, b->len);
	bstr[b->len] = '\0';
	SKIPWS(bstr);
	TrimWS(bstr);
	TrimMiddleWS(bstr);

	/* returns 0 when strings are equal */
	status = strcasecmp(astr, bstr);

    } while(0);

    if (a != NULL) PKIFreePrintableString(ctx->certasnctx, a);
    if (b != NULL) PKIFreePrintableString(ctx->certasnctx, b);
    if (astring != NULL) TC_Free(ctx->memMgr, astring);
    if (bstring != NULL) TC_Free(ctx->memMgr, bstring);

    if (status != 0)
	return 0;
    else
	return 1;
    
} /* ComparePrintableString */

/* compares SET OF.  return 1 if equivalent, 0 otherwise */
static int compareDnameSet (PKIRelativeDistinguishedName *a,
		       PKIRelativeDistinguishedName *b,
		       TC_CONTEXT *ctx)
{
    int i, j;
    int found;

    if (a->n != b->n) {
	/* must have the same number of elements in the set */
	return 0;
    }

/* TODO: this does NOT work for all cases!  Eg.,
   set a = { A, A, A, A }
   set b = { A, B, C, D }

   The outer loop cycles through all the A's in set a and the inner loop will
   match on the first A in set b every time.

   Need to fix soon, in the meantime its okay because there are no
   applications actually using more than one entry in these sets for dnames.
*/
    for (i = 0; i < a->n; i++) {

	for (found = 0, j = 0; j < b->n; j++) {

	    /* see if they are the same OID type */
	    if (a->elt[i]->type.len == b->elt[j]->type.len &&
		memcmp (a->elt[i]->type.val,
			b->elt[j]->type.val,
			a->elt[i]->type.len) == 0) {

		/* see if both are PrintableStrings */
		if (a->elt[i]->value.val[0] == PKIID_PrintableString &&
		    b->elt[i]->value.val[0] == PKIID_PrintableString) {
		    if (ComparePrintableString(
				  a->elt[i]->value,
				  b->elt[i]->value, ctx) ) {
			found = 1;
			break;
		    }
		    else
			continue;
		}
		
		/* Otherwise do a binary compare on the value */
		if (a->elt[i]->value.len == b->elt[j]->value.len &&
		    memcmp (a->elt[i]->value.val,
			    b->elt[j]->value.val,
			    a->elt[i]->value.len) == 0) {
		    found = 1;
		    break;
		}
	    }
	}

	if (!found)
	    return 0;
    }

    return 1;
}

/* compares two distinguished names.  return 1 if equivalent, 0 otherwise */
int tc_compare_dname (TC_Name *pa, TC_Name *pb, TC_CONTEXT *ctx)
{
  int i;
  PKIRDNSequence *a;
  PKIRDNSequence *b;

  if (pa == NULL || pb == NULL)
      return TC_E_INVARGS;

  a = (PKIRDNSequence *) pa->data;
  b = (PKIRDNSequence *) pb->data;

  if (a->n != b->n)
  {
    /* the top level is a SEQUENCE, so we must have the same number of
     * elements for equality.
     */
    return 0;
  }

  for (i = 0; i < a->n; i++)
  {
    if (!compareDnameSet (a->elt[i], b->elt[i], ctx))
    {
      /* SET OF's are not equivalent */
      return 0;
    }
  }

  return 1;
}

int tc_compare_block (PKIVariableBlock *a, PKIVariableBlock *b)
{
    if ( a == (PKIVariableBlock *)0 || 
         b == (PKIVariableBlock *)0 )
	return TC_E_INVARGS;

    if (a->len == b->len)
	if (memcmp(a->val, b->val, a->len) == 0)
	    return 1;

    return 0;

} /* tc_compare_block */

/* if we had real in-lines they could be use here...*/
/* compares two serial numbers, returns 1 if equal, 0 otherwise, <0 if
   other error */
int tc_compare_serial(
    PKICertificateSerialNumber *a,
    PKICertificateSerialNumber *b)
{
    return (tc_compare_block( (PKIVariableBlock *)a,
			      (PKIVariableBlock *)b) );
} /* tc_compare_serial */

/*****
 *
 *****/
int tc_get_version(int *version,
		   TC_CERT *cert,
		   TC_CONTEXT *ctx)
{
    int error = 0;

    if (cert == NULL || version == NULL)
        return TC_E_INVARGS;

    *version = TC_X509_VERSION_1; /* the default is version 1 */

    if (cert->tbsCertificate->version) {

	switch( (int)PKIGetIntVal(ctx->certasnctx,
			     cert->tbsCertificate->version, &error) ) {
	    case 0:
		*version = TC_X509_VERSION_1;
	        break;
	    case 1:
	        *version = TC_X509_VERSION_2;
	        break;
	    case 2:
		*version = TC_X509_VERSION_3;
	        break;
	} /* switch */
    }

    return 0;

} /* tc_get_version */


/*****
*
* return
*   0 - okay
*   != 0 - not okay
*
*****/
int tc_get_serial(unsigned char **serial, 
		  size_t *serialLen, 
		  TC_CERT *cert,
		  TC_CONTEXT *ctx)
{
    if (cert == NULL || serial == NULL || serialLen == NULL)
        return TC_E_INVARGS;
    *serialLen = 0;
    *serial = NULL;

    *serial = TC_Alloc(ctx->memMgr, cert->tbsCertificate->serialNumber.len);
    if (*serial == NULL)
	return TC_E_NOMEMORY;

    *serialLen = cert->tbsCertificate->serialNumber.len;
    memcpy(*serial, cert->tbsCertificate->serialNumber.val, *serialLen); 
    
    return 0;
}

/*****
*
* return
*   0 - okay
*   != 0 - not okay
*
*****/
int tc_get_sigalgorithm(unsigned char **alg, 
		  size_t *len, 
		  TC_CERT *cert,
		  TC_CONTEXT *ctx)
{
    if (cert == NULL || alg == NULL || len == NULL || ctx == NULL)
        return TC_E_INVARGS;
    *len = 0;
    *alg = NULL;

    *alg = TC_Alloc(ctx->memMgr, cert->cert->signatureAlgorithm.algorithm.len);
    if (*alg == NULL)
	return TC_E_NOMEMORY;

    *len = cert->cert->signatureAlgorithm.algorithm.len;
    memcpy(*alg, cert->cert->signatureAlgorithm.algorithm.val, *len); 
    
    return 0;

} /* tc_get_sigalgorithm */


/*****
*
* return
*   0 - okay
*   != 0 - not okay
*
*****/
int tc_get_key(unsigned char **key, 
	       size_t *len, 
	       int *nuub,
	       TC_CERT *cert,
	       TC_CONTEXT *ctx)
{
    TC_SubjectPublicKeyInfo *info;

    if (cert == NULL || key == NULL || len == NULL ||
	ctx == NULL || nuub == NULL)
        return TC_E_INVARGS;
    *len = 0;
    *nuub = 0;
    *key = NULL;

    info = &cert->tbsCertificate->subjectPublicKeyInfo;

    *key = TC_Alloc(ctx->memMgr, info->subjectPublicKey.len);
    if (*key == NULL)
	return TC_E_NOMEMORY;

    *len = info->subjectPublicKey.len;
    *nuub = info->subjectPublicKey.nuub;
    memcpy(*key, info->subjectPublicKey.val, *len); 
    
    return 0;

} /* tc_get_key */

/*****
 *
 * tc_get_params
 *
 * Get the parameter block from the subjectPublicKeyInfo portion of the
 * provided certificate.
 *
 *****/
int tc_get_params(unsigned char **params, 
		  size_t *len, 
		  TC_CERT *cert,
		  TC_CONTEXT *ctx)
{
    TC_AlgorithmIdentifier *ident;

    if (cert == NULL || params == NULL || len == NULL || ctx == NULL)
        return TC_E_INVARGS;
    *len = 0;
    *params = NULL;

    ident = &cert->tbsCertificate->subjectPublicKeyInfo.algorithm;

    if (ident->parameters != NULL) {
	*params = TC_Alloc(ctx->memMgr, ident->parameters->len);
	if (*params == NULL)
	    return TC_E_NOMEMORY;

	*len = ident->parameters->len;
	memcpy(*params, ident->parameters->val, *len); 
    }
    
    return 0;

} /* tc_get_params */

/*****
*
* return
*   0 - okay
*   != 0 - not okay
*
*****/
int tc_get_signature(unsigned char **sig, 
	       size_t *len, 
	       int *nuub,
	       TC_CERT *cert,
	       TC_CONTEXT *ctx)
{
    if (cert == NULL || sig == NULL || len == NULL ||
	ctx == NULL || nuub == NULL)
        return TC_E_INVARGS;
    *len = 0;
    *nuub = 0;
    *sig = NULL;

    *sig = TC_Alloc(ctx->memMgr, cert->cert->signature.len);
    if (*sig == NULL)
	return TC_E_NOMEMORY;

    *len = cert->cert->signature.len;
    *nuub = cert->cert->signature.len;
    memcpy(*sig, cert->cert->signature.val, *len); 
    
    return 0;

} /* tc_get_signature */

int tc_compare_time(PKITime *a, PKITime *b, TC_CONTEXT *ctx)
{
    time_t atime, btime;

    atime = tc_get_choiceoftime(a, ctx);
    btime = tc_get_choiceoftime(b, ctx);

    if (atime > btime)
	return 1;
    if (atime == btime)
	return 0;
    return -1;
}


/*****
 *
 * tc_is_self_signed
 *
 * See if the certificate is self signed
 *
 *****/
int
tc_is_self_signed(TC_CERT *cert, TC_CONTEXT *ctx)
{
    (void)ctx; /* for future use */

    if (cert == NULL)
	return TC_E_INVARGS;

    return(tc_compare_dname(&cert->tbsCertificate->subject,
                            &cert->tbsCertificate->issuer,
			    ctx));
} /* tc_is_self_signed */

static int tc_compare_OtherName (PKIAnotherName *a, PKIAnotherName *b)
{
	int i;

	i = tc_compare_block (&a->type_id, &b->type_id);
	if (i != 1)
		return i;
	return (tc_compare_block (&a->value, &b->value));
}

int
tc_compare_GeneralName (PKIGeneralName *a, PKIGeneralName *b, TC_CONTEXT *ctx)
{
	if (a->CHOICE_field_type != b->CHOICE_field_type)
		return 0;

	switch ((a->CHOICE_field_type) & 0x1f)
	{
		case 0x00: /* otherName */
			return (tc_compare_OtherName (a->data, b->data));
		case 0x01: /* IA5String */
		case 0x02:
		case 0x06:
		case 0x07: /* OCTET STRING */
		case 0x08: /* OBJECT IDENTIFIER */
			return (tc_compare_block (a->data, b->data));
		case 0x04:
			return (tc_compare_dname (a->data, b->data, ctx));
		default:
			return TC_E_COMPARENOTSUPPORTED;
	}
	/* not reached */
}

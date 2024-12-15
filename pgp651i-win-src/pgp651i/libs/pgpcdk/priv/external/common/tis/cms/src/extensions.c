/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "cert.h"
#include "tc.h"
#include "cms_proto.h"
#include "cms.h"

#include "cert_asn.h"
#include "cert_oid.h"

#include "extensions.h"

typedef struct
{
  unsigned char *oid;
  size_t oidlen;
  tc_handler_t *handler;
} TC_HANDLER;

void tc_free_handlers (TC_LIST **db, TC_MemoryMgr *mgr)
{
	TC_LIST *t;

	while (*db)
	{
		TC_Free(mgr, ((TC_HANDLER *)(*db)->data)->oid );
		TC_Free(mgr, (*db)->data);
		t = *db;
		*db = (*db)->next;
		TC_Free(mgr, t);
	}
}

int tc_create_extlist(TC_ExtensionList **list, TC_CONTEXT *ctx)
{
	if (list == NULL)
		return TC_E_INVARGS;

	*list = PKINewExtensions(ctx->certasnctx);

	if (*list == NULL)
		return TC_E_NOMEMORY;
	return 0;
}

void tc_free_extlist(TC_ExtensionList *list, TC_CONTEXT *ctx)
{
	if (list != NULL)
		PKIFreeExtensions(ctx->certasnctx, list);
	return;
} /* tc_free_extlist */


static TC_HANDLER *find_handler (unsigned char *oid, size_t oidlen, TC_LIST *db)
{
	while (db)
	{
		if (oidlen == ((TC_HANDLER *) db->data)->oidlen &&
			memcmp (oid, ((TC_HANDLER *) db->data)->oid, oidlen) == 0)
			return ((TC_HANDLER *) db->data);
		db = db->next;
	}

	return NULL;
}

/*****
 *
 * Register a handler to use during extension processing.  The handler
 * can be NULL if you do not want to actually process the data in the
 * extension but want to indicate that a critical extension is "handled".
 * Non-critical extensions do not need to have a handler registered.
 *
 * If this is called and the extension already exists in the context,
 * the current handler will be replaced with the provided one.
 *
 * parameters
 *   input
 *	oid - the extension oid
 *	oidlen - length of oid
 *	func - a pointer to an extension handler, it's prototype
 *	       is ???
 *	ctx - a pointer to a CMS context to add the extension data to
 *
 * return
 *   TC_E_INVARGS
 *   TC_E_NOMEMORY
 *   0 - okay
 *
 *****/
int tc_register_handler (const unsigned char *oid,
						 size_t oidlen,
						 tc_handler_t *func,
						 TC_CONTEXT *ctx)
{
	TC_LIST *l;
	TC_HANDLER *h = NULL;

	if (ctx ==  NULL || oid == NULL || oidlen == 0)
		return TC_E_INVARGS;

	/* see if OID is already registered and update its handler if
	   so, otherwise create a new handler entry in list */
	if ((h = find_handler((unsigned char *)oid, oidlen, 
						  ctx->handlers)) != NULL) {
		h->handler = func;
	}
	else {
		if ( (h = TC_Alloc(ctx->memMgr, sizeof(TC_HANDLER)) ) == NULL)
			return TC_E_NOMEMORY;

		if ((h->oid = TC_Alloc(ctx->memMgr, oidlen)) == NULL) {
			TC_Free(ctx->memMgr, h);
			return TC_E_NOMEMORY;
		}

		(void)memcpy(h->oid, oid, oidlen);
		h->oidlen = oidlen;
		h->handler = func;

		if ((l = TC_Alloc(ctx->memMgr, sizeof (TC_LIST))) == NULL) {
			TC_Free(ctx->memMgr, h->oid);
			TC_Free(ctx->memMgr, h);
			return TC_E_NOMEMORY;
		}

		l->data = (void *) h;
		l->next = ctx->handlers;
		ctx->handlers = l;
	}

	return 0;
}

/*****
 * run through the extensions on a X.509 v3 certificate
 *
 * return
 *    0 - okay
 *    TC_E_CRITICAL
 *    TC_E_EXTENSION
 *****/
int tc_process_extensions (PKIExtensions *pExt, void *cert, TC_CONTEXT *ctx)
{
	TC_HANDLER *tmp;
	int i;
	TC_LIST *db = ctx->handlers;

	if (!pExt)
		return 0; /* nothing to do! */

	/* `->n' is the number of extensions present in the list */
	for (i = 0 ; i < pExt->n ; i++)
	{
		tmp = find_handler(pExt->elt[i]->extnID.val, pExt->elt[i]->extnID.len, db);
		if (tmp)
		{
			if (tmp->handler && (tmp->handler(pExt->elt[i], cert, ctx) != 0) )
				return TC_E_EXTENSION;
		}
		else if (pExt->elt[i]->critical && pExt->elt[i]->critical->val)
		{
			return TC_E_CRITICAL; /* unhandled critical flag */
		}
	}

	return 0; /* no errors occurred */
}

int tc_find_extension (
					   TC_Extension **ext,
					   const TC_ExtensionList *extList,
					   const unsigned char *oid, /* DER-encoded OID */
					   size_t oidLen,
					   TC_CONTEXT *ctx)
{
	int i;

	(void)ctx; /* for future use */

	if (ext == NULL || extList == NULL || oid == NULL)
		return TC_E_INVARGS;

	for (i = 0; i < extList->n; i++) {
		if ((size_t)extList->elt[i]->extnID.len == oidLen &&
			memcmp (extList->elt[i]->extnID.val, oid, oidLen) == 0) {
			*ext = extList->elt[i];
			return 0;
		}
	}

	return TC_E_NOTFOUND;
}

/*****
 *
 * tc_add_extension
 *
 * parameters
 *   input
 *	oid - the extention's oid (in DER format)
 *	oidlen - 
 *	critical - a flag indicating whether this extension is critical (1)
 *	           or not (0)
 *	der - the ASN.1 DER encoded data for the extension
 *	derlen - 
 *   output
 *	exts - an extension list, the new extension is added to the
 *	       end of the list
 *
 * return
 *	TC_E_INVARGS
 *	TC_E_EXTENSION
 *	TC_E_NOMEMORY
 *
 *****/
int tc_add_extension (
					  TC_ExtensionList *extList,
					  const unsigned char *oid,
					  size_t oidlen,
					  int critical,
					  unsigned char *der,
					  size_t derlen,
					  TC_CONTEXT *ctx)
{
	PKIExtension *localext;

	if (extList == NULL || oid == NULL || der == NULL || oidlen == 0)
		return TC_E_INVARGS;
	if (extList->n == PKIMAX_Extensions)
		return TC_E_EXTENSION;

	localext = PKINewExtension(ctx->certasnctx);
	if (localext == NULL)
		return TC_E_NOMEMORY;

	localext->extnID.len = oidlen;
	localext->extnID.val = TC_Alloc(ctx->memMgr, oidlen);
	if (localext->extnID.val == NULL) {
		PKIFreeExtension(ctx->certasnctx, localext);
		return TC_E_NOMEMORY;
	}
	(void)memcpy(localext->extnID.val, oid, oidlen);

	/* only fill in the critical field if its not the DEFAULT of FALSE */
	if (critical != 0) {
		localext->critical = PKINewBOOLEAN(ctx->certasnctx);
		if (localext->critical == NULL) {
			TC_Free(ctx->memMgr, localext->extnID.val);
			PKIFreeExtension(ctx->certasnctx, localext);
			return TC_E_NOMEMORY;
		}
		if (PKIPutBoolVal(ctx->certasnctx, localext->critical, 1) != 0) {
			PKIFreeBOOLEAN(ctx->certasnctx, localext->critical);
			TC_Free(ctx->memMgr, localext->extnID.val);
			PKIFreeExtension(ctx->certasnctx, localext);
			return TC_E_NOMEMORY;
		}
	}

	localext->extnValue.len = derlen;
	localext->extnValue.val = TC_Alloc(ctx->memMgr, derlen);
	if (localext->extnValue.val == NULL) {
		PKIFreeBOOLEAN(ctx->certasnctx, localext->critical);
		TC_Free(ctx->memMgr, localext->extnID.val);
		PKIFreeExtension(ctx->certasnctx, localext);
		return TC_E_NOMEMORY;
	}
	(void)memcpy(localext->extnValue.val, der, derlen);

	extList->elt[extList->n] = localext;
	(extList->n)++;

	return 0;
}

/******************************************************************/
/*                           Prototypes                           */
/******************************************************************/

static int AddKeyUsageExt(TC_ExtensionList *ext, 
						  const void *keyU,
						  int criticality,
						  TC_CONTEXT *ctx);

static int AddBasicConstraintsExt(TC_ExtensionList *ext, 
								  const void *simpleBCons,
								  int criticality,
								  TC_CONTEXT *ctx);

static int AddSubjectAltNameExt(TC_ExtensionList *ext,
								const void *genNames,
								int criticality,
								TC_CONTEXT *ctx);

static int AddIssuerAltNameExt(TC_ExtensionList *ext,
							   const void *genNames,
							   int criticality,
							   TC_CONTEXT *ctx);

static int AddPrivateKeyUsagePeriod (TC_ExtensionList *ext,
									 const void *privKeyUsagePeriod,
									 int critical,
									 TC_CONTEXT *ctx);

static int AddCertificatePolicies (TC_ExtensionList *ext,
								   const void *certPolicies,
								   int critical,
								   TC_CONTEXT *ctx);

static int AddPolicyMappings (TC_ExtensionList *ext,
							  const void *policyMap,
							  int critical,
							  TC_CONTEXT *ctx);

static int AddNameConstraints (TC_ExtensionList *ext,
							   const void *,
							   int critical,
							   TC_CONTEXT *ctx);

static int AddSubjectKeyIdentifier (TC_ExtensionList *ext,
							   const void *,
							   int critical,
							   TC_CONTEXT *ctx);

static int AddExtKeyUsageSyntax (TC_ExtensionList *ext,
								 const void *,
								 int critical,
								 TC_CONTEXT *ctx);

static int CreateKeyUsageDER(unsigned int bits, 
							 unsigned char **buf,
							 size_t *size,
							 TC_CONTEXT *ctx);

static int AddSubjectDirectoryAttributes (TC_ExtensionList *ext,
										  const void *,
										  int critical,
										  TC_CONTEXT *ctx);

static int AddPolicyConstraints (TC_ExtensionList *ext,
								 const void *,
								 int critical,
								 TC_CONTEXT *ctx);

static int AddAuthorityInfoAccess (TC_ExtensionList *ext,
								   const void *,
								   int critical,
								   TC_CONTEXT *ctx);

static int AddCRLDistributionPoints (TC_ExtensionList *ext,
									 const void *,
									 int critical,
									 TC_CONTEXT *ctx);

static int CreateBasicConstraintsDER(const TC_BASIC_CONSTRAINT *simpleBCons,
									 unsigned char **buf, 
									 size_t *size,
									 TC_CONTEXT *ctx);

static int GetBasicConstraints(void **returnBCons,
							   const PKIExtension *ext,
							   TC_CONTEXT *ctx);

static int GetKeyUsage (void **returnKeyU,
						const PKIExtension *ext,
						TC_CONTEXT *ctx);

static int GetAlternativeNameExt(void **genNames,
								 const PKIExtension *ext,
								 TC_CONTEXT *ctx);

int GetOctetStringExt(void **pos, /* OUT */
					  const PKIExtension *ext, /* IN */
					  TC_CONTEXT *ctx);/* IN */

static int GetPrivateKeyUsagePeriod (void **ret, /* OUT */
									 const PKIExtension *ext, /* IN */
									 TC_CONTEXT *ctx); /* IN */

static int GetCertificatePolicies (void **ret, /* OUT */
								   const PKIExtension *ext, /* IN */
								   TC_CONTEXT *ctx); /* IN */

static int GetPolicyMappings (void **ret, /* OUT */
							  const PKIExtension *ext, /* IN */
							  TC_CONTEXT *ctx); /* IN */

static int GetSubjectDirectoryAttributes (void **ret, /* OUT */
										  const PKIExtension *ext, /* IN */
										  TC_CONTEXT *ctx); /* IN */

static int GetNameConstraints (void **ret, /* OUT */
							   const PKIExtension *ext, /* IN */
							   TC_CONTEXT *ctx); /* IN */

static int GetPolicyConstraints (void **ret, /* OUT */
							   const PKIExtension *ext, /* IN */
							   TC_CONTEXT *ctx); /* IN */

static int GetExtKeyUsageSyntax (void **ret, /* OUT */
								 const PKIExtension *ext, /* IN */
								 TC_CONTEXT *ctx); /* IN */

static int GetCRLDistributionPoints (void **ret, /* OUT */
									 const PKIExtension *ext, /* IN */
									 TC_CONTEXT *ctx); /* IN */

static int GetAuthorityInfoAccess (void **ret, /* OUT */
								   const PKIExtension *ext, /* IN */
								   TC_CONTEXT *ctx); /* IN */

/******************************************************************/
/*                        Static structures                       */
/******************************************************************/

static  struct 
{
	int (* ExtensionSetter) ( TC_ExtensionList *ext,    /* set ext. fn ptr */
							  const void *simpleExtValue, 
							  int crit,
							  TC_CONTEXT *ctx);

	int (* ExtensionGetter) ( void **simpleExtValue, /* get ext. fn ptr */
							  const PKIExtension *ext,
							  TC_CONTEXT *ctx);
	unsigned char *oid;                              /* oid */
	size_t oidlen;                                   /* oid length */
} supportedExtensions[] = 
{
	{
		AddKeyUsageExt,         
		GetKeyUsage,
		PKIid_ce_keyUsage_OID,               /* key usage */
		PKIid_ce_keyUsage_OID_LEN
	},

	{
		AddBasicConstraintsExt, 
		GetBasicConstraints,
		PKIid_ce_basicConstraints_OID,       /* basic constraints */
		PKIid_ce_basicConstraints_OID_LEN
	},

	{
		AddSubjectAltNameExt,
		GetAlternativeNameExt,
		PKIid_ce_subjectAltName_OID,         /* subjectAltName */
		PKIid_ce_subjectAltName_OID_LEN
	},

	{
		AddIssuerAltNameExt,
		GetAlternativeNameExt,
		PKIid_ce_issuerAltName_OID,          /* issuerAltName */
		PKIid_ce_issuerAltName_OID_LEN
	},

	{
		AddCRLNumberExt,
		GetCRLNumberExt,
		PKIid_ce_cRLNumber_OID,          /* cRLNumber */
		PKIid_ce_cRLNumber_OID_LEN
	},

	{
		AddDeltaCRLIndicatorExt,
		GetDeltaCRLIndicatorExt,
		PKIid_ce_deltaCRLIndicator_OID,          /* deltaCRLIndicator */
		PKIid_ce_deltaCRLIndicator_OID_LEN
	},

	{
		AddAuthorityKeyIdentifier,
		GetAuthorityKeyIdentifier,
		PKIid_ce_authorityKeyIdentifier_OID,
		PKIid_ce_authorityKeyIdentifier_OID_LEN
	},

	{
		AddSubjectKeyIdentifier,
		GetOctetStringExt,
		PKIid_ce_subjectKeyIdentifier_OID,
		PKIid_ce_subjectKeyIdentifier_OID_LEN
	},

	{
		AddPrivateKeyUsagePeriod,
		GetPrivateKeyUsagePeriod,
		PKIid_ce_privateKeyUsagePeriod_OID,
		PKIid_ce_privateKeyUsagePeriod_OID_LEN
	},

	{
		AddCertificatePolicies,
		GetCertificatePolicies,
		PKIid_ce_certificatePolicies_OID,
		PKIid_ce_certificatePolicies_OID_LEN
	},

	{
		AddPolicyMappings,
		GetPolicyMappings,
		PKIid_ce_policyMappings_OID,
		PKIid_ce_policyMappings_OID_LEN
	},

	{
		AddSubjectDirectoryAttributes,
		GetSubjectDirectoryAttributes,
		PKIid_ce_subjectDirectoryAttributes_OID,
		PKIid_ce_subjectDirectoryAttributes_OID_LEN
	},

	{
		AddNameConstraints,
		GetNameConstraints,
		PKIid_ce_nameConstraints_OID,
		PKIid_ce_nameConstraints_OID_LEN
	},

	{
		AddPolicyConstraints,
		GetPolicyConstraints,
		PKIid_ce_policyConstraints_OID,
		PKIid_ce_policyConstraints_OID_LEN
	},

	{
		AddExtKeyUsageSyntax,
		GetExtKeyUsageSyntax,
		PKIid_ce_extKeyUsage_OID,
		PKIid_ce_extKeyUsage_OID_LEN
	},

	{
		AddCRLDistributionPoints,
		GetCRLDistributionPoints,
		PKIid_ce_cRLDistributionPoints_OID,
		PKIid_ce_cRLDistributionPoints_OID_LEN
	},

	{
		AddAuthorityInfoAccess,
		GetAuthorityInfoAccess,
		PKIid_pe_authorityInfoAccess_OID,
		PKIid_pe_authorityInfoAccess_OID_LEN
	},

	{
		AddIssuingDistributionPoint,
		GetIssuingDistributionPoint,
		PKIid_ce_issuingDistributionPoint_OID,
		PKIid_ce_issuingDistributionPoint_OID_LEN
	},

	{
		AddCRLReason,
		GetCRLReason,
		PKIid_ce_cRLReasons_OID,
		PKIid_ce_cRLReasons_OID_LEN
	},

	{
		AddHoldInstructionCode,
		GetHoldInstructionCode,
		PKIid_ce_holdInstructionCode_OID,
		PKIid_ce_holdInstructionCode_OID_LEN
	},

	{
		AddInvalidityDate,
		GetInvalidityDate,
		PKIid_ce_invalidityDate_OID,
		PKIid_ce_invalidityDate_OID_LEN
	},

	{
		NULL,               
		NULL,
		(unsigned char *) 0,                 /* null terminator */
		0
	} 
};  /* supportedExtensions */

/******************************************************************/
/*           Debugging functions for CreateKeyUsageDER            */
/******************************************************************/

#ifdef DEBUG
void print_bits (unsigned int bits)
{
	int i;

	for (i = sizeof (unsigned int) * 8 - 1; i >= 0; i--)
	{
		if (( bits >> i) & 0x01 )
			fprintf (stderr, "1");
		else
			fprintf (stderr, "0");
	} /* for */
	fprintf(stderr, "\n");
} /* print_bits */


void print_chars (unsigned char character)
{
	int i;

	for (i = sizeof (unsigned char) * 8 - 1; i >= 0; i--)
	{
		if (( character >> i) & 0x01 )
			fprintf (stderr, "1");
		else
			fprintf (stderr, "0");
	} /* for */
} /* print_chars */
#endif /* DEBUG */

/******************************************************************/
/*                       tc_setextval                             */
/******************************************************************/
/* Routine to add extension data to an existing  extension list.  */
/*                                                                */
/* Extensions are defined in ITU-T X.509 Recommendation.          */
/*                                                                */
/* Given a current extension list, OID, OID length, extension     */
/* data structure and criticality value for the extension, make   */
/* calls to create extension DER and add it to the extension list.*/
/*                                                                */
/* Caller is expected to free returned space when done.           */
/*                                                                */
/* Supported set extensions:                                      */
/*   key usage                                                    */
/*   basic constraints                                            */
/*                                                                */
/* Parameters                                                     */
/*   input                                                        */
/*	ext - pointer to current extension list                   */
/*      oid - OID of extension to be added (in DER)               */
/*	oidlen - length, in bytes, of OID                         */
/*      criticality - mark this extension as critical             */
/*                    1: critical                                 */
/*                    0: not critical                             */
/*                   -1: use default spec'd by ITU standard       */
/*      value - pointer to structure with values for extension    */
/*   output                                                       */
/*	ext - updated extension list                              */
/*                                                                */
/* Return                                                         */
/*   0  - okay                                                    */
/*   TC_E_INVARGS - invalid arguments                             */
/*   TC_E_NOMEMORY -  out of memory                               */ 
/*   TC_E_EXTENSION - error packing extension                     */
/*   TC_E_EXTNOTSUPPORTED - given oid is not supported            */ 
/******************************************************************/
int tc_setextval(TC_ExtensionList *ext, 
				 const unsigned char *oid, 
				 size_t oidlen,
				 int criticality,
				 const void *value,
				 TC_CONTEXT *ctx)
{


	int status = 0;
	int i = 0; /* loop index */
	boolean extensionOIDSupported = PKIFALSE;

	do
	{

		/* ----- Basic parameter checks ----- */

		if ((ext == (TC_ExtensionList *) 0) ||
			(oidlen <= 0) ||
			((criticality > 1)|| (criticality < -1)) ||
			(value == (void *) 0) ||
			(ctx == NULL))
		{
			status = TC_E_INVARGS;
			break;
		} /* if */

		/* ----- Check that provided OID has been implemented ----- */

		while (supportedExtensions[i].oid != (unsigned char *) 0)
		{
			if ( supportedExtensions[i].oidlen == oidlen )
				if (!memcmp((char *) supportedExtensions[i].oid, 
							(char *) oid, 
							oidlen))
					if (supportedExtensions[i].ExtensionSetter != NULL)
					{
						extensionOIDSupported = PKITRUE;
						break; /* from while loop */
					} /* if */
			i++;
		} /* while */

		if (! extensionOIDSupported)
		{
			status = TC_E_EXTNOTSUPPORTED;  
			break;
		} /* if */

		/* ----- Call ith extension adding function ----- */

		status = ((* supportedExtensions[i].ExtensionSetter) (ext, 
															  value, 
															  criticality,
															  ctx));
		if (status != 0)
			break;

	} while(0);

	/* --- No cleanup needed here: ext is either correct or unchanged  --- */

	return(status);

} /* tc_setextval */


/******************************************************************/
/*                       tc_getextval                             */
/******************************************************************/
/* Routine to extract extension data from an existing extension.  */
/*                                                                */
/* Extensions are defined in ITU-T X.509 Recommendation.          */
/*                                                                */
/* Given the extension, OID, and OID length, make calls to the    */
/* appropriate extension data "get" routine to fill up a simple   */
/* TC data structure.                                             */
/*                                                                */
/* Caller is expected to free returned space (value) when done.   */
/*                                                                */
/* Supported get extensions:                                      */
/*   basic constraints                                            */
/*                                                                */
/* Parameters                                                     */
/*   input                                                        */
/*	extlist - pointer to extension list                       */
/*      oid - OID of extension to be read (in DER)                */
/*	oidlen - length, in bytes, of OID                         */
/*   output                                                       */
/*      value - pointer to structure with extension values        */
/*      criticality - whether this extension was marked critical  */
/*                    1: critical                                 */
/*                    0: not critical                             */
/*                                                                */
/* Return                                                         */
/*   0  - okay                                                    */
/*   TC_E_INVARGS - invalid arguments                             */
/*   TC_E_NOMEMORY -  out of memory                               */ 
/*   TC_E_EXTENSION - error packing extension                     */ 
/*   TC_E_EXTNOTSUPPORTED - given oid is not supported            */ 
/******************************************************************/
int tc_getextval(void **value,
				 boolean *criticality,
				 const unsigned char *oid, 
				 size_t oidlen,
				 const TC_ExtensionList *extlist,
				 TC_CONTEXT *ctx)

{
	int status = 0;
	int i = 0; 
	boolean extensionOIDSupported = PKIFALSE;
	PKIExtension *myExt = NULL;

	do
	{
		/* ----- Basic parameter checks ----- */

		if ((extlist == (TC_ExtensionList *) 0) ||
			(oidlen <= 0) ||
			(value == NULL) ||
			(ctx == NULL))
		{
			status = TC_E_INVARGS;
			break;
		} /* if */

		/* ----- Check that provided OID has been implemented ----- */

		while (supportedExtensions[i].oid != (unsigned char *) 0)
		{
			if ( supportedExtensions[i].oidlen == oidlen )
				if (!memcmp((char *) supportedExtensions[i].oid, 
							(char *) oid, 
							oidlen))
					if (supportedExtensions[i].ExtensionGetter != NULL)
					{
						extensionOIDSupported = PKITRUE;
						break; /* from while loop */
					} /* if */
			i++;
		} /* while */

		if (! extensionOIDSupported)
		{
			status = TC_E_EXTNOTSUPPORTED;   
			break;
		} /* if */

		/* ----- Find the extension in the cert and get the value ----- */

		status = tc_find_extension (&myExt,
									extlist,
									oid,
									oidlen,
									ctx);
		if (status != 0)
			break;

		/* ----- check criticality ----- */
		/* TODO: the "critical" argument to the Get*Value() functions can be
		   removed since we do this here */

		*criticality = (myExt->critical && (myExt->critical->val != 0));

		/* ----- get the extension value ------ */
		status = ((* supportedExtensions[i].ExtensionGetter) 
				  (value,
				   myExt,
				   ctx));
		if (status != 0)
			break;

	} while(/* CONSTCOND */ 0);

	/* --- cleanup : value and criticality 
	   are either correct or unchanged  --- */

	return(status);

} /* tc_getextval */



/******************************************************************/
/*                       AddKeyUsageExt                           */
/******************************************************************/
/* Routine to set criticality and add key usage data              */
/* to an extension list.                                          */
/*                                                                */
/* Key usage extension is defined in ITU-T Recommendation X.509   */
/* Section 12.2.2.3                                               */
/*                                                                */
/* Given a current extension list and a string variable           */
/* representing key usage data, make calls to create DER and      */
/* append to the extension list.                                  */
/*                                                                */
/* Caller is expected to free returned space when done.           */
/*                                                                */
/* Parameters                                                     */
/*   input                                                        */
/*	ext - current extension list                              */
/*      keyU - pointer to bit string filled by caller             */
/*      criticality - mark this extension as critical             */
/*                    1: critical                                 */
/*                    0: not critical                             */
/*                   -1: use default (false)                      */
/*   output                                                       */
/*	ext - updated extension list                              */
/*                                                                */
/* Return                                                         */
/*   0  - okay                                                    */
/*   TC_E_INVARGS - invalid arguments                             */
/*   TC_E_NOMEMORY -  out of memory                               */ 
/*   TC_E_EXTENSION - error packing extension                     */ 
/******************************************************************/

static int AddKeyUsageExt(TC_ExtensionList *ext, 
						  const void *keyU,
						  int criticality,
						  TC_CONTEXT *ctx)
{
	int status = 0;
	unsigned char *derBuffer = NULL;
	size_t  derSize = 0;
	boolean mycriticality;
	unsigned int *keyUsage = NULL;
	boolean defaultCriticality = PKITRUE; /* per PKIX part 1 */

	keyUsage = (unsigned int *) keyU;

	do {

		/* ------ Basic parameter checks ------ */

		/* Parameters checked by tc_setextval */

		/* ---------- Set criticality ------------------ */

		if (criticality < 0)              /* use default criticality */
			mycriticality = defaultCriticality;
		else                              /* take override value */
			mycriticality = (boolean) criticality;


		/* ------- Create DER buffer and add to extensions list ------- */

		if ( (status = CreateKeyUsageDER(*keyUsage, 
										 &derBuffer, 
										 &derSize,
										 ctx)) < 0 ) 
		{
			break;
		} /* if */

		if ( (status = tc_add_extension(ext,
										PKIid_ce_keyUsage_OID, 
										PKIid_ce_keyUsage_OID_LEN,
										mycriticality,
										derBuffer, 
										derSize,
										ctx)) < 0 )
		{
			break;
		} /* if */

	} while (/* CONSTCOND */ 0);

	/* ------- Clean up: free unneeded space ------ */

	TC_Free(ctx->memMgr, derBuffer);

	return status;

} /* AddKeyUsageExt */


/******************************************************************/
/*                AddBasicConstraintsExt                          */
/******************************************************************/
/* Routine to set criticality and add basic constraints data      */
/* to an extension list.                                          */
/*                                                                */
/* Basic constraints extension is defined in ITU-T Recommendation */
/* X.509 Section 12.4.2.1                                         */
/*                                                                */
/* Given a current extension list and a simple structure          */
/* representing basic constraints, make calls to create DER and   */
/* append to the extension list.                                  */
/*                                                                */
/* Caller is expected to free returned space when done.           */
/*                                                                */
/* Parameters                                                     */
/*   input                                                        */
/*	ext - current extension list                              */
/*      simpleBCons - basic cons. structure filled by caller      */
/*      criticality - mark this extension as critical             */
/*                    1: critical                                 */
/*                    0: not critical                             */
/*                   -1: use default (true)                       */
/*   output                                                       */
/*	ext - updated extension list                              */
/*                                                                */
/* Return                                                         */
/*   0  - okay                                                    */
/*   TC_E_INVARGS - invalid arguments                             */
/*   TC_E_NOMEMORY -  out of memory                               */ 
/*   TC_E_EXTENSION - error packing extension                     */ 
/******************************************************************/


static int AddBasicConstraintsExt(TC_ExtensionList *ext, 
								  const void *simpleBC,
								  int criticality,
								  TC_CONTEXT *ctx)
{
	int status = 0;
	TC_BASIC_CONSTRAINT *simpleBCons = NULL;
	unsigned char *derBuffer = NULL;
	size_t  derSize = 0;
	boolean mycriticality;
	boolean defaultCriticality = PKITRUE;


	simpleBCons = (TC_BASIC_CONSTRAINT *) simpleBC;

	do 
	{

		/* ---------- Basic Parameter Checks ----------- */

		/* cA must be a boolean */
		/* ext and criticality checked by tc_setextval */

		if ((simpleBCons->cA != 0) && (simpleBCons->cA != 1))
		{
			status = TC_E_INVARGS;
			break;
		} /* if */

		/* ---------- Set criticality ------------------ */

		if (criticality < 0)              /* use default criticality */
			mycriticality = defaultCriticality;
		else                              /* take override value */
			mycriticality = (boolean) criticality;


		/* ------- Create DER buffer and add to extensions list ------- */

		if ( (status = CreateBasicConstraintsDER( simpleBCons, 
												  &derBuffer, 
												  &derSize,
												  ctx) ) < 0) 
		{
			break;
		} /* if */

		if ( (status = tc_add_extension( ext,
										 PKIid_ce_basicConstraints_OID, 
										 PKIid_ce_basicConstraints_OID_LEN,
										 mycriticality, 
										 derBuffer, 
										 derSize,
										 ctx)) < 0 )
		{
			break;
		} /* if */

	} while (/* CONSTCOND */ 0);

	/* ------- Clean up: free unneeded space ------ */

	TC_Free(ctx->memMgr, derBuffer);

	return status;

} /* AddBasicConstraintsExt */


/******************************************************************/
/*                CreateKeyUsageDER                               */
/******************************************************************/
/* Routine to create the key usage extension DER                  */
/*                                                                */
/* Given the bits to turn on, create the DER for the bit string.  */
/* Caller is expected to free returned space when done.           */
/*                                                                */
/* Parameters                                                     */
/*   input                                                        */
/*	bits - the OR of all the bits that should be set          */
/*   output                                                       */
/*	buf - pointer updated to location of DER block created    */
/*	size - size of DER block created                          */
/*                                                                */
/* Return                                                         */
/*   0  - okay                                                    */
/*   TC_E_INVARGS -   invalid arguments                           */
/*   TC_E_NOMEMORY -  out of memory                               */ 
/*   TC_E_EXTENSION - error packing extension                     */ 
/******************************************************************/
static int CreateKeyUsageDER(unsigned int bits, 
							 unsigned char **buf, 
							 size_t *size,
							 TC_CONTEXT *ctx)
{
	PKIKeyUsage         *keyUsage = NULL;
	unsigned char	*bufPtr = NULL; 
	int 		errorRet = 0;
	int			status = 0;
	unsigned char	*localBits = NULL;
	int                 lastUsedBit = sizeof (unsigned int) * 8 - 1;
	int                 numUnusedBits = 0;
	int                 bytesNeeded = 0;
	int                 localBytes = 0;
	int                 i;  
	int                 j;  

	do {

		/* ------ Basic parameter checks ------ */

		/* Note: input bits are interpreted from right to left */
		/* i.e., 001101 means dig signature, key enciph, data enciph */

		/* bits can have any value */
		if ( buf == NULL || size == NULL ) 
		{
			status = TC_E_INVARGS;
			break;
		} /* if */

		/* ----- Find end (high-bits) of one-bits in "bits" integer ----- */

		if ( bits == 0)  /* no key usage bits set */
			lastUsedBit = 0;
		else
			while ((( bits >> lastUsedBit ) & 0x1 ) == 0 )
				lastUsedBit--;

		bytesNeeded = lastUsedBit / 8 + 1;
		numUnusedBits = bytesNeeded * 8 - lastUsedBit - 1;

		/* ----- Create a localBits array to hold just enough bytes ----- */

		/* Note: localBits array is created for interpretation left */
		/* to right. I.e., 1101000 means dig sig, non repud, data enciph */

		localBits = (unsigned char *)TC_Alloc(ctx->memMgr,
											  sizeof(unsigned char) * bytesNeeded);
		if (localBits == NULL)
		{
			status = TC_E_NOMEMORY;
			break;
		} /* if */
		memset(localBits, 0, sizeof(unsigned char) * bytesNeeded);

		/* ----- copy key usage bits into localbits ----- */

		/* starting at low order byte, drop rightmost zeros, reverse
		   bit string and store in char array                        */

		j = 7; 

		for (i = 0; i <= lastUsedBit ; i++)
		{
			localBits[ localBytes ] = localBits[ localBytes ] |
				(unsigned char) ((( bits >> i ) & 0x1 ) << j );
			j--;

			/* reset for next byte */
			if (j < 0)
			{
				localBytes++;
				j = 7;
			} /* if j */
		} /* for */

		/* ----- DEBUG: test to print localBits array ----- */

#ifdef DEBUG
		fprintf(stderr, "CreateKeyUsageDER:input parameter bits: ");
		print_bits (bits);

		fprintf( stderr, "lastUsedBit: %i\n", lastUsedBit);
		fprintf( stderr, "bytesNeeded: %i\n", bytesNeeded);
		fprintf( stderr, "numUnusedBits: %i\n", numUnusedBits);

		fprintf(stderr, "CreateKeyUsageDER:localBits array: ");
		for (i = 0; i < bytesNeeded ; i++)
		{
			print_chars( localBits[i] );
		} /* for */
		fprintf(stderr, "\n");
#endif /* DEBUG */

		/* ------ create key usage structure ------ */

		keyUsage = PKINewKeyUsage(ctx->certasnctx);
		if (keyUsage == (PKIKeyUsage *)0) 
		{
			status = TC_E_NOMEMORY;
			break;
		} /* if */

		status = PKIPutBitString(ctx->certasnctx,
								 keyUsage, /* maps to a BIT_STRING type */
								 localBits,
								 bytesNeeded,
								 numUnusedBits);
		if (status != 0)
		{
			break;
		} /* if */

		/* ------ allocate memory for key usage DER buffer ------ */

		*size = PKISizeofKeyUsage(ctx->certasnctx, keyUsage, PKITRUE);
		bufPtr = (unsigned char *)TC_Alloc(ctx->memMgr, *size);
		if (bufPtr == (unsigned char *)0) 
		{
			status = TC_E_NOMEMORY;
			break;
		} /* if */

		/* ------ pack key usage data from C structure to DER buffer ------ */

		(void) PKIPackKeyUsage(ctx->certasnctx, bufPtr,
							   *size, keyUsage, &errorRet);
		if( errorRet != 0 )
		{
			status = TC_E_EXTENSION;
			break;
		}  /* if */

	} while (0);

	/* ------- Clean up: free unneeded space ------ */

	if (status < 0) 
	{
		TC_Free(ctx->memMgr, bufPtr);
		bufPtr = NULL;	
		*size = 0;
	} /* if */

	PKIFreeKeyUsage(ctx->certasnctx, keyUsage);
	TC_Free(ctx->memMgr, localBits);

	*buf = bufPtr;
	return status;

} /* CreateKeyUsageDER */


/******************************************************************/
/*              CreateBasicConstraintsDER                         */
/******************************************************************/
/* Routine to create the basic constraints extension DER          */
/*                                                                */
/* Given whether the cert will be for a CA and the CA path        */
/* length, create the DER.                                        */
/*                                                                */
/* Caller is expected to free returned space when done.           */
/*                                                                */
/* Parameters                                                     */
/*   input                                                        */
/*	CA - true for CA, false for non-CA cert                   */
/*      path length - max number of CA certs that may follow      */
/*                    this cert in a certification path           */
/*   output                                                       */
/*	buf - pointer updated to location of DER block created    */
/*	size - size of DER block created                          */
/*                                                                */
/* Return                                                         */
/*   0  - okay                                                    */
/*   TC_E_INVARGS -   invalid arguments                           */
/*   TC_E_NOMEMORY -  out of memory                               */ 
/*   TC_E_EXTENSION - error packing extension                     */ 
/******************************************************************/

static int CreateBasicConstraintsDER(const TC_BASIC_CONSTRAINT *simpleBCons,
									 unsigned char **buf, 
									 size_t *size,
									 TC_CONTEXT *ctx)

{
	PKIBasicConstraints    *basicConstraints = NULL;
	unsigned char	*bufPtr = NULL; 
	int  		errorRet = 0;
	int			status = 0;


	do {

		/* ------ check the provided parameters ------ */

		if ( buf == NULL || size == NULL || simpleBCons == NULL ) 
		{
			status = TC_E_INVARGS;
			break;
		} /* if */

		/* ------ create basicConstraints structure ------ */

		basicConstraints = PKINewBasicConstraints(ctx->certasnctx);
		if (basicConstraints == (PKIBasicConstraints *)0) 
		{
			status = TC_E_NOMEMORY;
			break;
		} /* if */


		/* only provide the cA value if its not the default of FALSE */
		if (simpleBCons->cA == PKITRUE) {
			basicConstraints->cA = PKINewBOOLEAN(ctx->certasnctx);
			if ((basicConstraints->cA) == (PKIBOOLEAN *) 0)
			{
				status = TC_E_NOMEMORY;
				break;
			} /* if basicConstraints */
			if ( PKIPutBoolVal(ctx->certasnctx, basicConstraints->cA,
							   (int)simpleBCons->cA) == -1 ) 
			{
				status = TC_E_INVARGS;
				break;
			} /* if put_bool_val */  
		}

		/* ------ if CA path length is needed, then allocate and add it ------ */

		if ((simpleBCons->cA == PKITRUE) && 
			(simpleBCons->pathLength >= 0 ))
		{
			basicConstraints->pathLenConstraint = PKINewINTEGER(ctx->certasnctx);
			if ((basicConstraints->pathLenConstraint) == (PKIINTEGER *) 0)
			{
				status = TC_E_NOMEMORY;
				break;
			} /* if basicConstraints */
			if ( PKIPutIntVal(ctx->certasnctx,
							  basicConstraints->pathLenConstraint, 
							  (long)(simpleBCons->pathLength)) == -1 ) 
			{
				status = -1;
				break;
			} /* if put_int_val */
		} /* if simpleBCons */

		/* ------ allocate memory for basicConstraints DER buffer ------ */

		*size = PKISizeofBasicConstraints(ctx->certasnctx,
										  basicConstraints, PKITRUE);
		bufPtr = (unsigned char *)TC_Alloc(ctx->memMgr, *size);
		if (bufPtr == (unsigned char *)0) 
		{
			status = TC_E_NOMEMORY;
			break;
		} /* if */

		/* ------ pack basicConstraints data from C structure to DER buffer --- */

		(void) PKIPackBasicConstraints(ctx->certasnctx,
									   bufPtr, *size,
									   basicConstraints, &errorRet);
		if( errorRet != 0 )
		{
			status = TC_E_EXTENSION;
			break;
		}

	} while (0);

	/* ------- Clean up: free unneeded space ------ */

	if (status < 0) 
	{
		TC_Free(ctx->memMgr, bufPtr);
		bufPtr = NULL;
		*size = 0;
	} /* if */

	PKIFreeBasicConstraints(ctx->certasnctx, basicConstraints);

	*buf = bufPtr;
	return status;

} /* CreateBasicConstraintsDER */


/******************************************************************/
/*                      GetBasicConstraints                       */
/******************************************************************/
/* Routine to fill a simple (TC) basic constraints structure from */
/* Extension provided.                                            */
/*                                                                */
/* Extensions are defined in ITU-T X.509 Recommendation.          */
/*                                                                */
/* Given an extension, unpack the basic constraints DER into a    */
/* simplified basic constraints "C" structure.  No checks on data */ 
/* semantics are performed.                                       */
/*                                                                */
/* The caller is responsible for freeing returnBCons.             */
/*                                                                */
/* Parameters                                                     */
/*   input                                                        */
/*	ext - pointer to the given extension                      */
/*   output                                                       */
/*      returnBCons - ptr to ptr for simple C structure for B.C.  */
/*                                                                */
/* Return                                                         */
/*   0  - okay                                                    */
/*   TC_E_NOMEMORY -  out of memory                               */ 
/*   TC_E_EXTENSION - error packing extension                     */ 
/*   TC_E_INVARGS - invalid arguments                             */
/******************************************************************/
static int GetBasicConstraints(void **returnBCons,
							   const PKIExtension *ext,
							   TC_CONTEXT *ctx)

{
	PKIBasicConstraints    *basicConstraints = NULL;
	TC_BASIC_CONSTRAINT *simpleBCons = NULL;
	unsigned char	*value = NULL;
	size_t		valueLen = 0;
	int			errorRet = 0;
	int                 status = 0;

	do
	{
		/* ----- Unpack extension value into basicConstraints ----- */

		value = ext->extnValue.val;
		valueLen = ext->extnValue.len;
		(void)PKIUnpackBasicConstraints(ctx->certasnctx,
										&basicConstraints, 
										value, 
										valueLen, 
										&errorRet); 
		if (errorRet != 0)
		{
			status = TC_E_EXTENSION; 
			break;
		} /* if */

		/* ----- convert ASN.1 structure to simple TC structure  ----- */

		/* create and init simple basic constraints structure */
		simpleBCons = (TC_BASIC_CONSTRAINT *) 
			TC_Alloc(ctx->memMgr, sizeof (TC_BASIC_CONSTRAINT));
		if (simpleBCons == (TC_BASIC_CONSTRAINT *) 0)
		{
			status = TC_E_NOMEMORY;
			break;
		} /*if */

		/* fill in values for simple structure using ASN.1 struct */
		if (basicConstraints->cA != NULL &&
			basicConstraints->cA->val == 1)
			simpleBCons->cA = (boolean) 1;
		else
			simpleBCons->cA = (boolean) 0;

		if (basicConstraints->pathLenConstraint != NULL)
		{	  
			simpleBCons->pathLength = 
				(int)PKIGetIntVal(ctx->certasnctx,
								  basicConstraints->pathLenConstraint, 
								  &errorRet);
			if (errorRet != 0)
			{
				status = TC_E_EXTENSION; 
				break;
			} 

		} /*if basicConstraints */
		else
			simpleBCons->pathLength = TC_PATH_LENGTH_UNDEFINED;

	} while (/* CONSTCOND */ 0);

	/* ------- Clean up: assign to return vbls and free space ------ */    

	PKIFreeBasicConstraints(ctx->certasnctx, basicConstraints );

	if ( status != 0 )
		TC_Free(ctx->memMgr, simpleBCons);
	else
	{
		*returnBCons = (void *) simpleBCons;
	} /* else */

	return status;

} /* GetBasicConstraints */


/******************************************************************/
/*                      GetKeyUsage                               */
/******************************************************************/
/* Routine to fill a key usage integer from extension provided.   */
/*                                                                */
/* Extensions are defined in ITU-T X.509 Recommendation.          */
/*                                                                */
/* Given an extension, unpack the key usage DER into an integer   */
/* No checks on data semantics are performed.                     */
/*                                                                */
/* The caller is responsible for freeing returnKeyU.              */
/*                                                                */
/* Parameters                                                     */
/*   input                                                        */
/*	ext - pointer to the given extension                      */
/*   output                                                       */
/*      returnKeyU - ptr to ptr for simple C structure for key u. */
/*                                                                */
/* Return                                                         */
/*   0  - okay                                                    */
/*   TC_E_EXTENSION - error packing extension                     */ 
/*   TC_E_INVARGS - invalid arguments                             */
/******************************************************************/

static int GetKeyUsage (void **returnKeyU,
						const PKIExtension *ext,
						TC_CONTEXT *ctx)

{
	PKIKeyUsage              *keyUsage = NULL;
	unsigned int          *intBits = NULL;
	unsigned char	        *value = NULL;
	size_t		valueLen = 0;
	int			errorRet = 0;
	unsigned char         *charBits = NULL;
	int                   bytesUsed = 0;
	int                   unusedBits = 0;
	int                   byteCount = 0;
	int                   status = 0;
	int                   i = 0; 
	int                   j = 0; 

	do
	{
		/* ----- Unpack extension value into keyUsage ----- */

		value = ext->extnValue.val;
		valueLen = ext->extnValue.len;
		(void) PKIUnpackKeyUsage(ctx->certasnctx,
								 &keyUsage,
								 value,
								 valueLen,
								 &errorRet);

		if ((errorRet != 0) ||
			(keyUsage == NULL))
		{
			status = TC_E_EXTENSION; 
			break;
		} /* if */

		/* ----- Set bit values for integer using ASN.1 struct ----- */

		bytesUsed = keyUsage->len;
		unusedBits = keyUsage->nuub;
		charBits = keyUsage->val;

		/* Note: charBits array is interpreted left to right.  I.e., 
		   1101000 means digital sig, non repud, data enciph         */

		/* starting at low-order byte, reverse charBits bit string 
		   and store in intBits integer                        */

		intBits = (unsigned int *)TC_Alloc(ctx->memMgr, sizeof (unsigned int));
		if (intBits == NULL)
		{
			status = TC_E_NOMEMORY;
			break;
		} /* if */
		memset(intBits, 0, sizeof (unsigned int));

		j = 7;
		for (i = 0; i < bytesUsed * 8 - unusedBits; i++)
		{
			*intBits = *intBits |
				(int) (((charBits[ byteCount ] >> j) & 0x1 ) << i );
			j--;

			/* reset for next byte */
			if (j < 0)
			{
				byteCount++;
				j = 7;
			} /* if */
		} /* for */

		/* Note: intBits is interpreted from right to left.  I.e., 
		   001101 means digital sig, key enciph, data enciph     */

#ifdef DEBUG

		fprintf(stderr, "bytesUsed: %i\n", bytesUsed);
		fprintf(stderr, "unusedBits: %i\n", unusedBits);

		if (keyUsage->val == NULL)
			fprintf(stderr, "keyUsage->val is NULL\n");

		fprintf(stderr, "keyUsage characters: \n");
		for (i = 0; i < bytesUsed; i++)
		{
			print_chars( charBits[i] );
		} /* for */
		fprintf(stderr, "\n");

		fprintf(stderr, "intBits: \n");
		print_bits (*intBits);

#endif

	} while (/* CONSTCOND */ 0);

	/* ----- Clean up:  assign to return vbls and free space ----- */

	if (status == 0)
	{
		*returnKeyU = (void *) intBits;
	} /* if */

	PKIFreeKeyUsage (ctx->certasnctx, keyUsage);

	return status;
} /* GetKeyUsage */


/*----- Subject and Issuer Alternative Names -----*/

/****
 *
 * CheckGeneratlNames()
 *
 * Check that the structure provided by the user meets some minimum
 * requirements.
 *
 *
 *****/
static int
CheckGeneralNames(TC_GEN_NAMES_LIST_T *names)
{
	TC_GEN_NAME_T *name;
	int i;

	/* for alternative names there must be at least one name and CMS
	   currently supports up to 32 names*/
	if ( (names->numberOfNames <= 0) ||
		 (names->numberOfNames > PKIMAX_GeneralNames) )
		return TC_E_INVARGS;

	/* we don't support all the types and the provided name must
	   have a value (eg., no blank strings allowed); all we really
	   check for here is that there is a length to the provided data */
	for(i = 0; i < names->numberOfNames; i++) {
		name = names->names[i];

		if (name == NULL || name->name == NULL)
			return TC_E_INVARGS;

		switch (name->nameType) {

			case TC_rfc822Name:
			case TC_dNSName:
			case TC_uniformResourceIdentifier:
			case TC_iPAddress:
			case TC_registeredID:
				if (name->nameLen <= 0)
					return TC_E_INVARGS;
				break;
			case TC_directoryName:
				break;
			default:
				return TC_E_INVARGS;
		}
	} /* for each name in list */

	return 0;

} /* CheckGeneralNames */

static int
tc_GEN_NAME_to_PKIGeneralName (PKIGeneralName *asnName,/*OUT*/
							   TC_GEN_NAME_T *name,/*IN*/
							   TC_CONTEXT *ctx)/*IN*/
{
	PKIIA5String *string;
	PKIOCTET_STRING *octblock;
	PKIOBJECT_ID *objidblock;
	int status=0;

	if(!asnName||!name||!ctx)
		return TC_E_INVARGS;

	/* set the CHOICE field type and the choice's data; these
	   are explicitly tagged values, so we need to use the
	   tag value for the CHOICE field */
	switch(name->nameType)
	{
		case TC_rfc822Name:
			asnName->CHOICE_field_type = 
                            PKIrfc822Name_GeneralNameFieldTag;
			    /*0x80|0x20|0x01;*/
			string = PKINewIA5String(ctx->certasnctx);
			if (string == NULL)
			{
				status = TC_E_NOMEMORY;
				break;
			}
			PKIPutOctVal(ctx->certasnctx,
						 string, (unsigned char *)name->name,
						 name->nameLen);
			asnName->data = (void *)string;
			break;

		case TC_dNSName:
			asnName->CHOICE_field_type = 
                            PKIdNSName_GeneralNameFieldTag;
			    /*0x80|0x20|0x02;*/
			string =  PKINewIA5String(ctx->certasnctx);
			if (string == NULL)
			{
				status = TC_E_NOMEMORY;
				break;
			}
			PKIPutOctVal(ctx->certasnctx,
						 string, (unsigned char *)name->name,
						 name->nameLen);
			asnName->data = (void *)string;
			break;

		case TC_directoryName:
			asnName->CHOICE_field_type = 
                            PKIdirectoryName_GeneralNameFieldTag;
			    /*0x80|0x20|0x04;*/
			CopyName((PKIName **)&asnName->data,
					 (PKIName *)name->name, ctx);
			break;

		case TC_uniformResourceIdentifier:
			asnName->CHOICE_field_type = 
                           PKIuniformResourceIdentifier_GeneralNameFieldTag;
			    /*0x80|0x20|0x06;*/
			string =  PKINewIA5String(ctx->certasnctx);
			if (string == NULL)
			{
				status = TC_E_NOMEMORY;
				break;
			}
			PKIPutOctVal(ctx->certasnctx,
						 string, (unsigned char *)name->name,
						 name->nameLen);
			asnName->data = (void *)string;
			break;

		case TC_iPAddress:
			asnName->CHOICE_field_type = 
                            PKIiPAddress_GeneralNameFieldTag;
			    /*0x80|0x20|0x07;*/
			octblock = PKINewOCTET_STRING(ctx->certasnctx);
			if (octblock == NULL)
			{
				status = TC_E_NOMEMORY;
				break;
			}
			PKIPutOctVal(ctx->certasnctx,
						 octblock, (unsigned char *)name->name,
						 name->nameLen);
			asnName->data = (void *)octblock;
			break;

		case TC_registeredID:
			asnName->CHOICE_field_type = 
                                   PKIregisteredID_GeneralNameFieldTag;
			    /*0x80|0x20|0x08;*/
			objidblock = PKINewOBJECT_ID(ctx->certasnctx);
			if (objidblock == NULL)
			{
				status = TC_E_NOMEMORY;
				break;
			}
			PKIPutOctVal(ctx->certasnctx,
						 objidblock, (unsigned char *)name->name,
						 name->nameLen);
			asnName->data = (void *)objidblock;
			break;

		default:
			status = TC_E_OTHER;
			break;
	}
	return status;
} /* tc_GEN_NAME_to_PKIGeneralName */

/* This function allocates the PKI list and fills it. */
int
tc_GEN_NAMES_to_PKIGeneralNames (PKIGeneralNames *localNameList,
								 TC_GEN_NAMES_LIST_T *nameList,
								 TC_CONTEXT *ctx)
{
	TC_GEN_NAME_T *name;
	PKIGeneralName *asnName = NULL;
	int i,status = 0;

	do
	{
		/* add each individual name to the ASN data structure list */
		for (i = 0; i < nameList->numberOfNames; i++)
		{
			name = nameList->names[i];

			asnName = PKINewGeneralName(ctx->certasnctx);
			if (asnName == NULL)
			{
				status = TC_E_NOMEMORY;
				break;
			}
			status=tc_GEN_NAME_to_PKIGeneralName(asnName,name,ctx);
			if (status)
				break;
			localNameList->elt[i] = asnName;
			localNameList->n = i+1;
		}
	}
	while(0);
	return status;
} /* tc_GEN_NAMES_to_PKIGeneralNames */

/*****
 *
 * Both SubjectAltName and IssuerAltName are just renames of GeneralNames.
 * So the same routine can be used to generate the DER for both.  This
 * routine will just deal with the GeneralNames DER.  We currently only
 * support a subset of the values allowed in the GeneralName CHOICE (see the
 * switch below).
 *
 * parameters
 *   input
 *       nameList - the list of general names provided by the user
 *
 *   output
 *       derBuffer - a pointer to a buffer that will be allocated to hold
 *                   the DER
 *       derBuffLen - length of above
 *       
 * returns
 *
 *****/
static int
CreateGeneralNamesDER (unsigned char **derBuffer,
					   size_t *derBuffLen,
					   TC_GEN_NAMES_LIST_T *nameList,
					   TC_CONTEXT *ctx)
{
	PKIGeneralNames *localNameList = NULL;
	int status = 0;
	int error = 0;

	do
	{
		localNameList=PKINewGeneralNames(ctx->certasnctx);
		status=tc_GEN_NAMES_to_PKIGeneralNames(localNameList,nameList,ctx);
		if(status)
			break;

		/* pack up to get DER */
		*derBuffLen = PKISizeofGeneralNames(ctx->certasnctx,
											localNameList, PKITRUE);
		if (*derBuffLen <= 0)
		{
			status = TC_E_OTHER;
			break;
		}
		*derBuffer = TC_Alloc(ctx->memMgr, *derBuffLen);
		if (*derBuffer == NULL)
		{
			status = TC_E_NOMEMORY;
			break;
		}

		(void)PKIPackGeneralNames(ctx->certasnctx,
								  *derBuffer, *derBuffLen,
								  localNameList, &error);
		if (error != 0)
		{
			status = TC_E_OTHER;
			break;
		}
	}
	while(0);

	if (status != 0) {
		if (*derBuffer != NULL)
			TC_Free(ctx->memMgr, derBuffer);
		*derBuffer = NULL;
		*derBuffLen = 0;
	}

	if (localNameList != NULL) {
		PKIFreeGeneralNames(ctx->certasnctx, localNameList);
	}

	return status;

} /* CreateGeneralNamesDER */

/*****
 *
 * Add an subject alternative name extension to the provided extension list.
 *
 * SubjectAltName extension is defined in
 * ITU-T Recommendation X.509, sections ???.
 *
 * parameters
 *   input
 *       genNames - a pointer to the TC_GEN_NAMES_LIST_T structure
 *       criticality - mark this extension as critical
 *                    1: critical
 *                    0: not critical
 *                   -1: use default (false)
 *
 *   output
 *       ext - the extension list will be updated with the new extension
 *
 * return
 *   0 - okay
 *   TC_E_INVARGS - invalid arguments
 *   TC_E_NOMEMORY -  out of memory
 *   TC_E_EXTENSION - error packing extension
 *
 *****/
static int 
AddSubjectAltNameExt(TC_ExtensionList *ext,
					 const void *genNames,
					 int criticality,
					 TC_CONTEXT *ctx)
{
	int status = 0;
	TC_GEN_NAMES_LIST_T *localNameList = (TC_GEN_NAMES_LIST_T *)genNames;
	boolean mycriticality;
	unsigned char *derBuffer = NULL;
	size_t  derSize = 0;

	do {
		/* check the provided names */
		if ((status = CheckGeneralNames(localNameList)) != 0)
			break;

		/* set criticality */
		if (criticality < 0)
			mycriticality = PKIFALSE; /* the default is false */
		else
			mycriticality = (boolean)criticality;

		/* create DER */
		if ((status = CreateGeneralNamesDER(&derBuffer, &derSize,
											localNameList,
											ctx)) != 0)
			break;

		/* add to extension list */
		if ((status = tc_add_extension(ext, 
									   PKIid_ce_subjectAltName_OID,
									   PKIid_ce_subjectAltName_OID_LEN,
									   mycriticality,
									   derBuffer,
									   derSize,
									   ctx)) != 0)
			break;

	} while(0);

	/* clean up */
	if (derBuffer != NULL)
		TC_Free(ctx->memMgr, derBuffer);

	return status;
} /* AddSubjectAltNameExt */

/*****
 *
 * Add an issuer alternative name extension to the provided extension list.
 *
 * IssuerAltName extension is defined in
 * ITU-T Recommendation X.509, section ???.
 *
 * parameters
 *   input
 *       genNames - a pointer to the TC_GEN_NAMES_LIST_T structure
 *       criticality - mark this extension as critical
 *                    1: critical
 *                    0: not critical
 *                   -1: use default (false)
 *
 *   output
 *       ext - the extension list will be updated with the new extension
 *
 * return
 *   0 - okay
 *   TC_E_INVARGS - invalid arguments
 *   TC_E_NOMEMORY -  out of memory
 *   TC_E_EXTENSION - error packing extension
 *
 *****/
static int 
AddIssuerAltNameExt(TC_ExtensionList *ext,
					const void *genNames,
					int criticality,
					TC_CONTEXT *ctx)
{
	int status = 0;
	TC_GEN_NAMES_LIST_T *localNameList = (TC_GEN_NAMES_LIST_T *)genNames;
	boolean mycriticality;
	unsigned char *derBuffer = NULL;
	size_t  derSize = 0;

	do {
		/* check the provided names */
		if ((status = CheckGeneralNames(localNameList)) != 0)
			break;

		/* set criticality */
		if (criticality < 0)
			mycriticality = PKIFALSE; /* the default is false */
		else
			mycriticality = (boolean)criticality;

		/* create DER */
		if ((status = CreateGeneralNamesDER(&derBuffer, &derSize,
											localNameList,
											ctx)) != 0)
			break;

		/* add to extension list */
		if ((status = tc_add_extension(ext, 
									   PKIid_ce_issuerAltName_OID,
									   PKIid_ce_issuerAltName_OID_LEN,
									   mycriticality,
									   derBuffer,
									   derSize,
									   ctx)) != 0)
			break;

	} while(0);

	/* clean up */
	if (derBuffer != NULL)
		TC_Free(ctx->memMgr, derBuffer);

	return status;
} /* AddIssuerAltNameExt */

static int tc_PKIGeneralNameToTC (TC_GEN_NAME_T *name, /* OUT */
								  PKIGeneralName *asnName, /* IN */
								  TC_CONTEXT *ctx) /* IN */
{
	PKIVariableBlock *asnblock;
	int status=0;

	switch(asnName->CHOICE_field_type)
	{
		case PKIrfc822Name_GeneralNameFieldTag:
			name->nameType = TC_rfc822Name;
			asnblock = (PKIVariableBlock *)asnName->data;
			name->nameLen = asnblock->len;
			name->name = (void *)TC_Alloc(ctx->memMgr,
										  asnblock->len);
			if (name->name == NULL) {
				status = TC_E_NOMEMORY;
				break;
			}
			memcpy(name->name, asnblock->val, asnblock->len);
			break;
		case PKIdNSName_GeneralNameFieldTag:
			name->nameType = TC_dNSName;
			asnblock = (PKIVariableBlock *)asnName->data;
			name->nameLen = asnblock->len;
			name->name = (void *)TC_Alloc(ctx->memMgr,
										  asnblock->len);
			if (name->name == NULL) {
				status = TC_E_NOMEMORY;
				break;
			}
			memcpy(name->name, asnblock->val, asnblock->len);
			break;

		case PKIdirectoryName_GeneralNameFieldTag:
			name->nameType = TC_directoryName;
			name->nameLen = 0; /* not used for Name type */
			status = CopyName((PKIName **)(&name->name),
							  (PKIName *)asnName->data,
							  ctx);
			break;

		case PKIuniformResourceIdentifier_GeneralNameFieldTag:
			name->nameType = TC_uniformResourceIdentifier;
			asnblock = (PKIVariableBlock *)asnName->data;
			name->nameLen = asnblock->len;
			name->name = (void *)TC_Alloc(ctx->memMgr,
										  asnblock->len);
			if (name->name == NULL) {
				status = TC_E_NOMEMORY;
				break;
			}
			memcpy(name->name, asnblock->val, asnblock->len);
			break;

		case PKIiPAddress_GeneralNameFieldTag:
			name->nameType = TC_iPAddress;
			asnblock = (PKIVariableBlock *)asnName->data;
			name->nameLen = asnblock->len;
			name->name = (void *)TC_Alloc(ctx->memMgr, asnblock->len);
			if (name->name == NULL) {
				status = TC_E_NOMEMORY;
				break;
			}
			memcpy(name->name, asnblock->val, asnblock->len);
			break;

		case PKIregisteredID_GeneralNameFieldTag:
			name->nameType = TC_registeredID;
			asnblock = (PKIVariableBlock *)asnName->data;
			name->nameLen = asnblock->len;
			name->name = (void *)TC_Alloc(ctx->memMgr, asnblock->len);
			if (name->name == NULL) {
				status = TC_E_NOMEMORY;
				break;
			}
			memcpy(name->name, asnblock->val, asnblock->len);
			break;

		default:
			status = TC_E_OTHER;
			break;
	} /* switch */

	return status;
} /* tc_PKIGeneralNameToTC */

/*****
 *
 * Converts GeneralName from ASN.1 compiler to TIS/CMS structures.
 * Note: this function makes a copy of the data, so the caller is responsible
 * for free'ing data returned by this function, most likely with
 * tc_free_gennamelist().
 *
 * inputs
 *	
 * returns
 *    0 - okay
 */

int tc_PKIGeneralNamesToTC (TC_GEN_NAMES_LIST_T **tc, /* OUT */
							PKIGeneralNames *asnNameList, /* IN */
							TC_CONTEXT *ctx) /* IN */

{
	int i, status=0;
	PKIGeneralName *asnName;
	TC_GEN_NAME_T *name;
	TC_GEN_NAMES_LIST_T *localNameList = 
		(TC_GEN_NAMES_LIST_T *)TC_Alloc(ctx->memMgr,
										sizeof(TC_GEN_NAMES_LIST_T));
	if (localNameList == NULL)
		return TC_E_NOMEMORY;
	*tc=localNameList;

	for (i = 0; i < PKIMAX_GeneralNames; i++)
		localNameList->names[i] = NULL;
	localNameList->numberOfNames = 0;

	for (i = 0; i < asnNameList->n; i++) {
		asnName = asnNameList->elt[i];

		name = (TC_GEN_NAME_T *)TC_Alloc(ctx->memMgr, sizeof(TC_GEN_NAME_T));
		if (name == NULL) {
			status = TC_E_NOMEMORY;
			break;
		}

		status = tc_PKIGeneralNameToTC(name,asnName,ctx);
		if (status != 0)
			break;

		localNameList->names[i] = name;
		localNameList->numberOfNames = i+1;

	} /* for each name */

	return status;
}

/*****
 *
 * Unpack and return the values in a subject or issuer alternative name
 * extension.  Since both subject and issuer alt. name are just renames
 * of GeneralNames, this routine just needs to deal with GeneralNames.
 * We currently only deal with a subset of the allowed values in GeneralName
 * (see the switch below).
 *
 * parameters
 *   intput
 *       ext - the extension to process
 *
 *   output
 *       genNames - a TC_GEN_NAMES_LIST_T structure will be returned containing
 *                  the values from the extension
 *
 * returns
 *   0 - okay
 *****/
static int
GetAlternativeNameExt(void **genNames,
					  const PKIExtension *ext,
					  TC_CONTEXT *ctx)
{
	int status = 0;
	int error = 0;
	PKIGeneralNames *asnNameList = NULL;
	TC_GEN_NAMES_LIST_T *localNameList = NULL;

	do {
		/* unpack extension data */
		(void)PKIUnpackGeneralNames(ctx->certasnctx,
									&asnNameList,
									ext->extnValue.val,
									ext->extnValue.len, &error);
		if (error != 0 || asnNameList == NULL) {
			status = TC_E_EXTENSION;
			break;
		}

		/* convert ASN compiler structure to TIS/CMS structure */
		status = tc_PKIGeneralNamesToTC(&localNameList,asnNameList,ctx);
		if (status != 0)
			break;

	} while(0);

	PKIFreeGeneralNames(ctx->certasnctx, asnNameList);

	if (status != 0) {
		tc_free_gennamelist(localNameList, ctx);
		localNameList = NULL;
	}

	*genNames = (void *)localNameList;
	return status;

} /* GetAlternativeNameExt */


/*****
 *
 *
 *****/
int tc_free_gennamelist(TC_GEN_NAMES_LIST_T *list, TC_CONTEXT *ctx)
{
	int i;

	if (list == NULL)
		return 0;

	for (i = 0; i < list->numberOfNames; i++)
		if (list->names[i] != NULL) {
			if (list->names[i]->name != NULL) {
				if (list->names[i]->nameType == TC_directoryName)
					PKIFreeName(ctx->certasnctx, 
								(TC_Name *)list->names[i]->name);
				else
					TC_Free(ctx->memMgr,
							(unsigned char *)list->names[i]->name);
			}
			TC_Free(ctx->memMgr, list->names[i]);
		}
	TC_Free(ctx->memMgr, list);

	return 0;
} /* tc_free_gennamelist */

/*****
 *
 * AddOctetStringExt
 *
 * Provides a generic method for adding OCTET STRING-valued extensions.
 */

static int AddOctetStringExt (TC_ExtensionList *ext,
							  PKIOCTET_STRING *os,
							  const unsigned char *oid,
							  size_t oidlen,
							  int critical,
							  TC_CONTEXT *ctx)
{
	int status=0;
	unsigned char *der=NULL;
	size_t dersize;

	do
	{
		/* ----- pack extension data ----- */

		dersize=PKISizeofOCTET_STRING(ctx->certasnctx,os,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackOCTET_STRING(ctx->certasnctx,der,dersize,os,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		/* ----- create extension ----- */

		status=tc_add_extension(ext, oid, oidlen, critical, der, dersize, ctx);
	}
	while (0);

	if(der)
		TC_Free(ctx->memMgr,der);
	return status;
}

/*****
 *
 * GetOctetStringExt
 *
 * Provides a generic interface for retrieving OCTET STRING-valued extensions.
 * The caller is responsible for deallocating the structure returned by this
 * function.
 *
 */

int GetOctetStringExt (void **pos, /* OUT */
					   const PKIExtension *ext, /* IN */
					   TC_CONTEXT *ctx) /* IN */
{
	PKIOCTET_STRING *os=NULL;
	int status=0;

	do
	{
		PKIUnpackOCTET_STRING(ctx->certasnctx,&os,ext->extnValue.val,ext->extnValue.len,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
	}
	while (0);
	*pos=(void*)os;

	return status;
}

static int
AddSubjectKeyIdentifier (TC_ExtensionList *ext,
						 const void *subjKeyId,
						 int critical,
						 TC_CONTEXT *ctx)
{
	(void) critical;
	return AddOctetStringExt (ext,
							  (PKIOCTET_STRING *) subjKeyId,
							  PKIid_ce_subjectKeyIdentifier_OID,
							  PKIid_ce_subjectKeyIdentifier_OID_LEN,
							  0/*always noncritical (RFC2459 4.2.1.2)*/,
							  ctx);
}

static int
AddPrivateKeyUsagePeriod (TC_ExtensionList *ext,
						  const void *privKeyUsagePeriod,
						  int critical,
						  TC_CONTEXT *ctx)
{
	int status=0;
	TC_PRIVATE_KEY_USAGE_PERIOD_T *p;
	PKIPrivateKeyUsagePeriod *asn=NULL;
	unsigned char *der=NULL;
	size_t dersize;

	(void)critical;

	do
	{
		p=(TC_PRIVATE_KEY_USAGE_PERIOD_T *)privKeyUsagePeriod;
		if(!p->notBefore && !p->notAfter)
		{
			/* RFC2459 4.2.1.4

			   "CAs conforming to this profile MUST NOT generate certificates
			   with private key usage period extensions unless at least one of
			   the two components is present." */
			status=TC_E_INVALIDPERIOD;
			break;
		}

		asn=PKINewPrivateKeyUsagePeriod(ctx->certasnctx);
		if(!asn)
		{
			status=TC_E_NOMEMORY;
			break;
		}

		if(p->notBefore)
		{
			status=tc_EncodeGeneralizedTime(&asn->notBefore,p->notBefore,ctx);
			if(status)
				break;
		}

		if(p->notAfter)
		{
			status=tc_EncodeGeneralizedTime(&asn->notAfter,p->notAfter,ctx);
			if(status)
				break;
		}

		dersize=PKISizeofPrivateKeyUsagePeriod(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackPrivateKeyUsagePeriod(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		status=tc_add_extension(ext,
								PKIid_ce_privateKeyUsagePeriod_OID,
								PKIid_ce_privateKeyUsagePeriod_OID_LEN,
								0,/*always noncritical (RFC2459 4.2.1.4) */
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreePrivateKeyUsagePeriod(ctx->certasnctx,asn);
	return status;
}

static int GetPrivateKeyUsagePeriod (void **ret, /* OUT */
									 const PKIExtension *ext, /* IN */
									 TC_CONTEXT *ctx) /* IN */
{
	PKIPrivateKeyUsagePeriod *asn=NULL;
	TC_PRIVATE_KEY_USAGE_PERIOD_T *u;
	int status=0;

	do
	{
		PKIUnpackPrivateKeyUsagePeriod(ctx->certasnctx,&asn,ext->extnValue.val,ext->extnValue.len,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		u=TC_Alloc(ctx->memMgr,sizeof(TC_PRIVATE_KEY_USAGE_PERIOD_T));
		if(!u)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		*ret=(void*)u;

		memset(u,0,sizeof(TC_PRIVATE_KEY_USAGE_PERIOD_T));

		if(asn->notBefore)
			u->notBefore=tc_decode_GeneralizedTime(asn->notBefore);
		if(asn->notAfter)
			u->notBefore=tc_decode_GeneralizedTime(asn->notAfter);
	}
	while (0);
	if(asn)
		PKIFreePrivateKeyUsagePeriod(ctx->certasnctx,asn);
	return status;
}

static int AddCertificatePolicies (TC_ExtensionList *ext,
								   const void *v_cp,
								   int critical,
								   TC_CONTEXT *ctx)
{
	TC_CERTIFICATE_POLICIES_T *cp;
	int i,j,k,status=0;
	unsigned char *der=NULL;
	size_t dersize;
	PKICertificatePolicies *asn=NULL;

	do
	{
		cp=(TC_CERTIFICATE_POLICIES_T *)v_cp;

		asn=PKINewCertificatePolicies(ctx->certasnctx);
		if(!asn)
		{
			status=TC_E_NOMEMORY;
			break;
		}

		/* ----- convert from CMS structure to ASN.1 structure ----- */

		asn->n=cp->numPolicies;
		for(i=0;status==0 && i<cp->numPolicies;i++)
		{
			asn->elt[i]=PKINewPolicyInformation(ctx->certasnctx);
			PKIPutOctVal(ctx->certasnctx,&asn->elt[i]->policyIdentifier,cp->policy[i]->policyOid,cp->policy[i]->policyOidLen);
			if(cp->policy[i]->numQualifiers)
			{
				asn->elt[i]->policyQualifiers=PKINewpolicyQualifiers_SEQ_OF(ctx->certasnctx);
				asn->elt[i]->policyQualifiers->n=cp->policy[i]->numQualifiers;
				for(j=0;status==0 && j<cp->policy[i]->numQualifiers;j++)
				{
					asn->elt[i]->policyQualifiers->elt[j]=PKINewPolicyQualifierInfo(ctx->certasnctx);
					if(cp->policy[i]->qualifier[j]->type == TC_POLICY_QUALIFIER_ID_CPS)
					{
						PKIIA5String *ia5;

						ia5=PKINewIA5String(ctx->certasnctx);
						if(!ia5)
						{
							status=TC_E_NOMEMORY;
							break;
						}
						PKIPutStrVal(ctx->certasnctx,ia5,(char*)cp->policy[i]->qualifier[j]->qualifier);
						dersize=PKISizeofIA5String(ctx->certasnctx,ia5,PKITRUE);
						der=TC_Alloc(ctx->memMgr,dersize);
						if(!der)
						{
							status=TC_E_NOMEMORY;
							break;
						}
						PKIPackIA5String(ctx->certasnctx,der,dersize,ia5,&status);
						if(status)
							break;
						PKIFreeIA5String(ctx->certasnctx,ia5);

						PKIPutOctVal(ctx->certasnctx,&asn->elt[i]->policyQualifiers->elt[j]->policyQualifierId,PKIid_qt_cps_OID,PKIid_qt_cps_OID_LEN);
					}
					else if(cp->policy[i]->qualifier[j]->type == TC_POLICY_QUALIFIER_ID_UNOTICE)
					{
						PKIVisibleString *vs;
						PKIUserNotice *pkiun=NULL;
						TC_USER_NOTICE_T *un=(TC_USER_NOTICE_T *)cp->policy[i]->qualifier[j]->qualifier;

						pkiun=PKINewUserNotice(ctx->certasnctx);
						if(!pkiun)
						{
							status=TC_E_NOMEMORY;
							break;
						}
						if(un->noticeRef)
						{
							/* TODO: this should be user controlled! */
							pkiun->noticeRef=PKINewNoticeReference(ctx->certasnctx);
							pkiun->noticeRef->organization.CHOICE_field_type=PKIID_VisibleString;
							vs=PKINewVisibleString(ctx->certasnctx);
							PKIPutStrVal(ctx->certasnctx,vs,un->noticeRef->organization);
							pkiun->noticeRef->organization.data=(void*)vs;

							pkiun->noticeRef->noticeNumbers.n=un->noticeRef->numNoticeNumbers;
							for(k=0;status==0&&k<un->noticeRef->numNoticeNumbers;k++)
							{
								pkiun->noticeRef->noticeNumbers.elt[k]=PKINewINTEGER(ctx->certasnctx);
								PKIPutIntVal(ctx->certasnctx,pkiun->noticeRef->noticeNumbers.elt[k],*(un->noticeRef->noticeNumber[k]));
							}
						}
						if(un->explicitText)
						{
							/* TODO: this should be user controlled! */
							pkiun->explicitText=PKINewDisplayText(ctx->certasnctx);
							pkiun->explicitText->CHOICE_field_type=PKIID_VisibleString;
							vs=PKINewVisibleString(ctx->certasnctx);
							PKIPutStrVal(ctx->certasnctx,vs,(char*)un->explicitText);
							pkiun->explicitText->data=(void*)vs;
						}
						dersize=PKISizeofUserNotice(ctx->certasnctx,pkiun,PKITRUE);
						der=TC_Alloc(ctx->memMgr,dersize);
						PKIPackUserNotice(ctx->certasnctx,der,dersize,pkiun,&status);
						if(status)
							break;
						PKIPutOctVal(ctx->certasnctx,&asn->elt[i]->policyQualifiers->elt[j]->policyQualifierId,PKIid_qt_unotice_OID,PKIid_qt_unotice_OID_LEN);
					}
					else
					{
						/* unknown type */
						status=TC_E_OTHER;
						break;
					}
					asn->elt[i]->policyQualifiers->elt[i]->qualifier.val=der;
					asn->elt[i]->policyQualifiers->elt[i]->qualifier.len=dersize;
				}
				if(status)
					break;
			}
		}
		der=NULL;/* so we don't try to free the same data twice */
		if(status)
			break;

		/* pack up extension data */
		dersize=PKISizeofCertificatePolicies(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		PKIPackCertificatePolicies(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		/* add extension to the list */
		status=tc_add_extension(ext,
								PKIid_ce_certificatePolicies_OID,
								PKIid_ce_certificatePolicies_OID_LEN,
								critical,
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreeCertificatePolicies(ctx->certasnctx,asn);
	return status;
}

static int GetCertificatePolicies (void **ret,
								   const PKIExtension *ext,
								   TC_CONTEXT *ctx)
{
	int i,j,k,status=0;
	TC_CERTIFICATE_POLICIES_T *cp=NULL;
	PKICertificatePolicies *pkicp=NULL;
	PKIUserNotice *pkiun=NULL;

	do
	{
		PKIUnpackCertificatePolicies(ctx->certasnctx,&pkicp,ext->extnValue.val,ext->extnValue.len,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		cp=TC_Alloc(ctx->memMgr,sizeof(TC_CERTIFICATE_POLICIES_T));
		if(!cp)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(cp,0,sizeof(TC_CERTIFICATE_POLICIES_T));

		cp->numPolicies=pkicp->n;
		for(i=0;status==0&&i<pkicp->n;i++)
		{
			cp->policy[i]=TC_Alloc(ctx->memMgr,sizeof(TC_POLICY_INFORMATION_T));
			memset(cp->policy[i],0,sizeof(TC_POLICY_INFORMATION_T));
			cp->policy[i]->policyOid=pkicp->elt[i]->policyIdentifier.val;
			cp->policy[i]->policyOidLen=pkicp->elt[i]->policyIdentifier.len;
			/*don't free this data, it is passed back to the caller*/
			memset(&pkicp->elt[i]->policyIdentifier,0,sizeof(PKICertPolicyId));

			if(pkicp->elt[i]->policyQualifiers)
			{
				cp->policy[i]->numQualifiers=pkicp->elt[i]->policyQualifiers->n;
				for(j=0;status==0&&j<cp->policy[i]->numQualifiers;j++)
				{
					cp->policy[i]->qualifier[j]=TC_Alloc(ctx->memMgr,sizeof(TC_POLICY_QUALIFIER_INFO_T));
					memset(cp->policy[i]->qualifier[j],0,sizeof(TC_POLICY_QUALIFIER_INFO_T));

					if(!memcmp(pkicp->elt[i]->policyQualifiers->elt[j]->policyQualifierId.val,
							   PKIid_qt_cps_OID,
							   PKIid_qt_cps_OID_LEN))
					{
						char *s;
						PKIIA5String *ia5;

						cp->policy[i]->qualifier[j]->type = TC_POLICY_QUALIFIER_ID_CPS;
						/*return as nul-terminated string*/
						PKIUnpackIA5String(ctx->certasnctx,
										   &ia5,
										   pkicp->elt[i]->policyQualifiers->elt[j]->qualifier.val,
										   pkicp->elt[i]->policyQualifiers->elt[j]->qualifier.len,
										   &status);
						if(status)
						{
							status=compiler2tc_error(status);
							break;
						}
						s=TC_Alloc(ctx->memMgr,ia5->len+1);
						if(!s)
						{
							status=TC_E_NOMEMORY;
							break;
						}
						strncpy(s,(char*)ia5->val,ia5->len);
						s[ia5->len]=0;
						cp->policy[i]->qualifier[j]->qualifier=(void*)s;
						PKIFreeIA5String(ctx->certasnctx,ia5);
					}
					else if(!memcmp(pkicp->elt[i]->policyQualifiers->elt[j]->policyQualifierId.val,
									PKIid_qt_unotice_OID,
									PKIid_qt_unotice_OID_LEN))
					{
						TC_USER_NOTICE_T *un;

						cp->policy[i]->qualifier[j]->type = TC_POLICY_QUALIFIER_ID_UNOTICE;

						PKIUnpackUserNotice(ctx->certasnctx,
											&pkiun,
											pkicp->elt[i]->policyQualifiers->elt[j]->qualifier.val,
											pkicp->elt[i]->policyQualifiers->elt[j]->qualifier.len,
											&status);
						if(status)
						{
							status=compiler2tc_error(status);
							break;
						}

						un=TC_Alloc(ctx->memMgr,sizeof(TC_USER_NOTICE_T));
						memset(un,0,sizeof(TC_USER_NOTICE_T));

						if(pkiun->noticeRef)
						{
							PKIVariableBlock *vb=(PKIVariableBlock*)pkiun->noticeRef->organization.data;

							un->noticeRef=TC_Alloc(ctx->memMgr,sizeof(TC_NOTICE_REFERENCE_T));
							memset(un->noticeRef,0,sizeof(TC_NOTICE_REFERENCE_T));
							/*copy organization name*/
							/*TODO: type of string is lost here*/
							un->noticeRef->organization=TC_Alloc(ctx->memMgr,vb->len+1);
							strncpy(un->noticeRef->organization,(char*)vb->val,vb->len);
							un->noticeRef->organization[vb->len]=0;
							/*copy noticeNumbers member*/
							un->noticeRef->numNoticeNumbers=pkiun->noticeRef->noticeNumbers.n;
							for(k=0;status==0&&k<un->noticeRef->numNoticeNumbers;k++)
							{
								un->noticeRef->noticeNumber[k]=TC_Alloc(ctx->memMgr,sizeof(int));
								*(un->noticeRef->noticeNumber[k])=PKIGetIntVal(ctx->certasnctx,pkiun->noticeRef->noticeNumbers.elt[k],&status);
							}
							if(status)
							{
								status=compiler2tc_error(status);
								break;
							}
						}

						if(pkiun->explicitText)
						{
							/*NOTE: this is probably a dangerous thing to do.
							  However, currently all the underlying types make
							  use of this structure so we can safely ignore
							  the CHOICE type*/
							PKIVariableBlock *vb=(PKIVariableBlock*)pkiun->explicitText;
							un->explicitText=TC_Alloc(ctx->memMgr,vb->len+1);
							memcpy(un->explicitText,vb->val,vb->len);
							un->explicitText[vb->len]=0;
						}

						cp->policy[i]->qualifier[j]->qualifier=(void*)un;
					}
					else
					{
						/*invalid policy id*/
						status=TC_E_OTHER;
						break;
					}
				}
				if(status)
					break;
			}
		}
		if(status)
			break;
	}
	while(0);
	*ret=(void*)cp;
	if(pkicp)
		PKIFreeCertificatePolicies(ctx->certasnctx,pkicp);
	if(pkiun)
		PKIFreeUserNotice(ctx->certasnctx,pkiun);
	return status;
}

static int AddPolicyMappings (TC_ExtensionList *ext,
							  const void *v_pm,
							  int critical,
							  TC_CONTEXT *ctx)
{
	PKIPolicyMappings *asn=NULL;
	TC_POLICY_MAPPINGS_T *pm;
	int i,status=0;
	unsigned char *der=NULL;
	size_t dersize;

	(void)critical;/*unused, always noncritical (RFC2459 4.2.1.6)*/
	do
	{
		pm=(TC_POLICY_MAPPINGS_T *)v_pm;
		asn=PKINewPolicyMappings(ctx->certasnctx);
		if(!asn)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		asn->n=pm->numPolicyMappings;
		for(i=0;i<pm->numPolicyMappings;i++)
		{
			asn->elt[i]=PKINewPolicyMappings_SEQUENCE(ctx->certasnctx);
			PKIPutOctVal(ctx->certasnctx,&asn->elt[i]->issuerDomainPolicy,pm->policyMapping[i]->issuerDomainPolicyOid,pm->policyMapping[i]->issuerDomainPolicyOidLen);
			PKIPutOctVal(ctx->certasnctx,&asn->elt[i]->subjectDomainPolicy,pm->policyMapping[i]->subjectDomainPolicyOid,pm->policyMapping[i]->subjectDomainPolicyOidLen);
		}

		dersize=PKISizeofPolicyMappings(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackPolicyMappings(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		status=tc_add_extension(ext,
								PKIid_ce_policyMappings_OID,
								PKIid_ce_policyMappings_OID_LEN,
								0, /* always noncritical */
								der,
								dersize,
								ctx);
	}
	while(0);
	if(asn)
		PKIFreePolicyMappings(ctx->certasnctx,asn);
	if(der)
		TC_Free(ctx->memMgr,der);
	return status;
}

static int GetPolicyMappings (void **ret,
							  const PKIExtension *ext,
							  TC_CONTEXT *ctx)
{
	int i,status=0;
	TC_POLICY_MAPPINGS_T *pm=NULL;
	PKIPolicyMappings *pkipm=NULL;

	do
	{
		PKIUnpackPolicyMappings(ctx->certasnctx,&pkipm,ext->extnValue.val,ext->extnValue.len,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		pm=TC_Alloc(ctx->memMgr,sizeof(TC_POLICY_MAPPINGS_T));
		if(!pm)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(pm,0,sizeof(TC_POLICY_MAPPINGS_T));
		pm->numPolicyMappings=pkipm->n;
		for(i=0;i<pkipm->n;i++)
		{
			pm->policyMapping[i]=TC_Alloc(ctx->memMgr,sizeof(TC_POLICY_MAPPING_T));
			memset(pm->policyMapping[i],0,sizeof(TC_POLICY_MAPPING_T));

			pm->policyMapping[i]->issuerDomainPolicyOid=pkipm->elt[i]->issuerDomainPolicy.val;
			pm->policyMapping[i]->issuerDomainPolicyOidLen=pkipm->elt[i]->issuerDomainPolicy.len;
			pm->policyMapping[i]->subjectDomainPolicyOid=pkipm->elt[i]->subjectDomainPolicy.val;
			pm->policyMapping[i]->subjectDomainPolicyOidLen=pkipm->elt[i]->subjectDomainPolicy.len;
			/* don't free the data we are returning */
			memset(pkipm->elt[i],0,sizeof(PKIPolicyMappings_SEQUENCE));
		}
	}
	while(0);
	*ret=(void*)pm;
	if(pkipm)
		PKIFreePolicyMappings(ctx->certasnctx,pkipm);
	return status;
}

static int AddSubjectDirectoryAttributes (TC_ExtensionList *ext,
										  const void *v_sda,
										  int critical,
										  TC_CONTEXT *ctx)
{
	int i,j,status=0;
	TC_SUBJECT_DIRECTORY_ATTRIBUTES_T *sda;
	PKISubjectDirectoryAttributes *asn=NULL;
	unsigned char *der=NULL;
	size_t dersize;

	(void)critical;/*unused, always noncritical (RFC2459 4.2.1.9)*/
	do
	{
		sda=(TC_SUBJECT_DIRECTORY_ATTRIBUTES_T *)v_sda;
		asn=PKINewSubjectDirectoryAttributes(ctx->certasnctx);
		asn->n=sda->numAttributes;
		for(i=0;i<asn->n;i++)
		{
			asn->elt[i]=PKINewAttribute(ctx->certasnctx);
			PKIPutOctVal(ctx->certasnctx,
						 &asn->elt[i]->type,
						 sda->attribute[i]->oid,
						 sda->attribute[i]->oidsize);

			asn->elt[i]->values.n=sda->attribute[i]->numValues;
			for(j=0;j<asn->elt[i]->values.n;j++)
			{
				asn->elt[i]->values.elt[j]=PKINewAttributeValue(ctx->certasnctx);
				PKIPutOctVal(ctx->certasnctx,
							 asn->elt[i]->values.elt[j],
							 sda->attribute[i]->val[j],
							 sda->attribute[i]->valsize[j]);
			}
		}

		dersize=PKISizeofSubjectDirectoryAttributes(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackSubjectDirectoryAttributes(ctx->certasnctx,
										  der,
										  dersize,
										  asn,
										  &status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		status=tc_add_extension(ext,
								PKIid_ce_subjectDirectoryAttributes_OID,
								PKIid_ce_subjectDirectoryAttributes_OID_LEN,
								0,/*always noncritical (rfc2459 4.2.1.9)*/
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreeSubjectDirectoryAttributes(ctx->certasnctx,asn);
	return status;
}

static int GetSubjectDirectoryAttributes(void **ret,
										 const PKIExtension *ext,
										 TC_CONTEXT *ctx)
{
	int i,j,status=0;
	PKISubjectDirectoryAttributes *asn=NULL;
	TC_SUBJECT_DIRECTORY_ATTRIBUTES_T *sda=NULL;

	do
	{
		PKIUnpackSubjectDirectoryAttributes(ctx->certasnctx,
											&asn,
											ext->extnValue.val,
											ext->extnValue.len,
											&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		sda=TC_Alloc(ctx->memMgr,sizeof(TC_SUBJECT_DIRECTORY_ATTRIBUTES_T));
		if(!sda)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(sda,0,sizeof(TC_SUBJECT_DIRECTORY_ATTRIBUTES_T));
		sda->numAttributes=asn->n;
		for(i=0;i<asn->n;i++)
		{
			sda->attribute[i]=TC_Alloc(ctx->memMgr,sizeof(TC_ATTRIBUTE_T));
			memset(sda->attribute[i],0,sizeof(TC_ATTRIBUTE_T));

			sda->attribute[i]->oidsize=asn->elt[i]->type.len;
			sda->attribute[i]->oid=TC_Alloc(ctx->memMgr,sda->attribute[i]->oidsize);
			memcpy(sda->attribute[i]->oid,asn->elt[i]->type.val,asn->elt[i]->type.len);

			sda->attribute[i]->numValues=asn->elt[i]->values.n;
			for(j=0;j<asn->elt[i]->values.n;j++)
			{
				sda->attribute[i]->valsize[j]=asn->elt[i]->values.elt[j]->len;
				sda->attribute[i]->val[j]=TC_Alloc(ctx->memMgr,sda->attribute[i]->valsize[j]);
				memcpy(sda->attribute[i]->val[j],
					   asn->elt[i]->values.elt[j]->val,
					   asn->elt[i]->values.elt[j]->len);
			}
		}
	}
	while(0);
	*ret=(void*)sda;
	if(asn)
		PKIFreeSubjectDirectoryAttributes(ctx->certasnctx,asn);
	return status;
}

static int tc_AddGeneralSubtrees(PKIGeneralSubtrees **g,
								 TC_GENERAL_SUBTREES_T *s,
								 TC_CONTEXT *ctx)
{
	PKIGeneralSubtrees *p;
	int i,status;

	*g=PKINewGeneralSubtrees(ctx->certasnctx);
	if(!*g)
		return TC_E_NOMEMORY;
	p=*g;
	for(i=0;i<s->numSubtrees;i++)
	{
		p->elt[i]=PKINewGeneralSubtree(ctx->certasnctx);
		status=tc_GEN_NAME_to_PKIGeneralName(&p->elt[i]->base,
											 s->base[i],
											 ctx);
		if(status)
		{
			PKIFreeGeneralSubtrees(ctx->certasnctx,*g);
			*g=NULL;
			return status;
		}
		p->n++;
	}
	return 0;
} /* tc_AddGeneralSubtrees */

static void tc_FreeGeneralSubtrees(TC_CONTEXT *ctx, TC_GENERAL_SUBTREES_T *g)
{
	int i;
	for(i=0;i<g->numSubtrees;i++)
	{
		if(g->base[i])
			TC_Free(ctx->memMgr,g->base[i]);
	}
	TC_Free(ctx->memMgr,g);
} /* tc_FreeGeneralSubtrees */

void tc_FreeNameConstraints(TC_CONTEXT *ctx, TC_NAME_CONSTRAINTS_T *nc)
{
	if(nc->permit)
		tc_FreeGeneralSubtrees(ctx,nc->permit);
	if(nc->deny)
		tc_FreeGeneralSubtrees(ctx,nc->deny);
	TC_Free(ctx->memMgr,nc);
} /* tc_FreeNameConstraints */

static int AddNameConstraints(TC_ExtensionList *ext,
							   const void *s_nc,
							   int critical,
							   TC_CONTEXT *ctx)
{
	int status=0;
	TC_NAME_CONSTRAINTS_T *nc;
	PKINameConstraints *asn=NULL;
	unsigned char *der=NULL;
	size_t dersize;

	(void)critical;/*unused,always critical(RFC2459 sect. 4.2.1.11) */
	do
	{
		nc=(TC_NAME_CONSTRAINTS_T *)s_nc;
		asn=PKINewNameConstraints(ctx->certasnctx);
		if(nc->permit)
		{
			status=tc_AddGeneralSubtrees(&asn->permittedSubtrees,nc->permit,ctx);
			if(status)
				break;
		}
		if(nc->deny)
		{
			status=tc_AddGeneralSubtrees(&asn->excludedSubtrees,nc->deny,ctx);
			if(status)
				break;
		}
		dersize=PKISizeofNameConstraints(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackNameConstraints(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		status=tc_add_extension(ext,
								PKIid_ce_nameConstraints_OID,
								PKIid_ce_nameConstraints_OID_LEN,
								1,/*always critical (RFC2459 sect. 4.2.1.11)*/
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreeNameConstraints(ctx->certasnctx,asn);
	return status;
} /* AddNameConstraints */

static int tc_GetGeneralSubtrees(TC_GENERAL_SUBTREES_T **g,
								 PKIGeneralSubtrees *asn,
								 TC_CONTEXT *ctx)
{
	int i,status=0;

	*g=TC_Alloc(ctx->memMgr,sizeof(TC_GENERAL_SUBTREES_T));
	if(*g)
		return TC_E_NOMEMORY;
	memset(*g,0,sizeof(TC_GENERAL_SUBTREES_T));
	for(i=0;i<asn->n;i++)
	{
		(*g)->base[i]=TC_Alloc(ctx->memMgr,sizeof(TC_GEN_NAME_T));
		tc_PKIGeneralNameToTC((*g)->base[i],&asn->elt[i]->base,ctx);
		if(asn->elt[i]->minimum)
			(*g)->minimum[i]=PKIGetIntVal(ctx->certasnctx,asn->elt[i]->minimum,&status);
		if(asn->elt[i]->maximum)
			(*g)->maximum[i]=PKIGetIntVal(ctx->certasnctx,asn->elt[i]->maximum,&status);
	}
	return status;
}

static int GetNameConstraints (void **ret,
							   const PKIExtension *ext,
							   TC_CONTEXT *ctx)
{
	int status=0;
	TC_NAME_CONSTRAINTS_T *nc=NULL;
	PKINameConstraints *asn=NULL;

	do
	{
		PKIUnpackNameConstraints(ctx->certasnctx,
								 &asn,
								 ext->extnValue.val,
								 ext->extnValue.len,
								 &status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		nc=TC_Alloc(ctx->memMgr,sizeof(TC_NAME_CONSTRAINTS_T));
		if(!nc)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(nc,0,sizeof(TC_NAME_CONSTRAINTS_T));

		if(asn->permittedSubtrees)
		{
			status=tc_GetGeneralSubtrees(&nc->permit,asn->permittedSubtrees,ctx);
			if(status)
				break;
		}
		if(asn->excludedSubtrees)
		{
			status=tc_GetGeneralSubtrees(&nc->deny,asn->excludedSubtrees,ctx);
			if(status)
				break;
		}
	}
	while(0);
	if(asn)
		PKIFreeNameConstraints(ctx->certasnctx,asn);
	if(status && nc)
	{
		tc_FreeNameConstraints(ctx,nc);
		nc=NULL;
	}
	*ret=(void*)nc;
	return status;
} /* GetNameConstraints */

static int AddPolicyConstraints (TC_ExtensionList *ext,
								 const void *v_pc,
								 int critical,
								 TC_CONTEXT *ctx)
{
	int status=0;
	PKIPolicyConstraints *asn=NULL;
	TC_POLICY_CONSTRAINTS_T *pc;
	unsigned char *der=NULL;
	size_t dersize;

	do
	{
		pc=(TC_POLICY_CONSTRAINTS_T *)v_pc;

		/* RFC 2459 sect. 4.2.1.12 requires that one of `requireExplicitPolicy'
		   or `inhibitPolicyMapping' be set */
		if(pc->inhibit==-1 && pc->require==-1)
		{
			status=TC_E_OTHER;
			break;
		}

		asn=PKINewPolicyConstraints(ctx->certasnctx);
		if(!asn)
		{
			status=TC_E_NOMEMORY;
			break;
		}

		if(pc->require>=0)
		{
			asn->requireExplicitPolicy=PKINewSkipCerts(ctx->certasnctx);
			PKIPutIntVal(ctx->certasnctx,asn->requireExplicitPolicy,pc->require);
		}

		if(pc->inhibit>=0)
		{
			asn->inhibitPolicyMapping=PKINewSkipCerts(ctx->certasnctx);
			PKIPutIntVal(ctx->certasnctx,asn->inhibitPolicyMapping,pc->inhibit);
		}

		dersize=PKISizeofPolicyConstraints(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackPolicyConstraints(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		status=tc_add_extension(ext,
								PKIid_ce_policyConstraints_OID,
								PKIid_ce_policyConstraints_OID_LEN,
								critical,
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreePolicyConstraints(ctx->certasnctx,asn);
	return status;
} /* AddPolicyConstraints */

static int GetPolicyConstraints (void **ret,
								 const PKIExtension *ext,
								 TC_CONTEXT *ctx)
{
	PKIPolicyConstraints *asn=NULL;
	TC_POLICY_CONSTRAINTS_T *pc=NULL;
	int status=0;

	do
	{
		PKIUnpackPolicyConstraints(ctx->certasnctx,&asn,ext->extnValue.val,ext->extnValue.len,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		pc=TC_Alloc(ctx->memMgr,sizeof(TC_POLICY_CONSTRAINTS_T));
		/* since 0 is a valid value, -1 is used to indicate absence as
		   noted in tc.h */
		pc->require=-1;
		pc->inhibit=-1;

		if(asn->requireExplicitPolicy)
			pc->require=PKIGetIntVal(ctx->certasnctx,asn->requireExplicitPolicy,&status);
		if(asn->inhibitPolicyMapping)
			pc->inhibit=PKIGetIntVal(ctx->certasnctx,asn->inhibitPolicyMapping,&status);
	}
	while(0);
	if(status && pc)
	{
		TC_Free(ctx->memMgr,pc);
		pc=NULL;
	}
	*ret=(void*)pc;
	if(asn)
		PKIFreePolicyConstraints(ctx->certasnctx,asn);
	return status;
} /* GetPolicyConstraints */

static struct {
	TC_KEY_PURPOSE_ID_T id;
	unsigned char *oid;
	size_t oidsize;
} TC_KEY_PURPOSE_MAP[] = {
	{ TC_KEY_PURPOSE_ID_SERVER_AUTH, PKIid_kp_serverAuth_OID,PKIid_kp_serverAuth_OID_LEN },
	{ TC_KEY_PURPOSE_ID_CLIENT_AUTH, PKIid_kp_clientAuth_OID,PKIid_kp_clientAuth_OID_LEN },
	{ TC_KEY_PURPOSE_ID_CODE_SIGNING, PKIid_kp_codeSigning_OID,PKIid_kp_codeSigning_OID_LEN },
	{ TC_KEY_PURPOSE_ID_EMAIL_PROTECTION, PKIid_kp_emailProtection_OID,PKIid_kp_emailProtection_OID_LEN },
	{ TC_KEY_PURPOSE_ID_TIME_STAMPING, PKIid_kp_timeStamping_OID,PKIid_kp_timeStamping_OID_LEN },
	{ TC_KEY_PURPOSE_ID_UNKNOWN, NULL, 0 }
};

static int AddExtKeyUsageSyntax (TC_ExtensionList *ext,
								 const void *v_eku,
								 int critical,
								 TC_CONTEXT *ctx)
{
	int i,j,status=0;
	TC_EXT_KEY_USAGE_SYNTAX_T *eku=(TC_EXT_KEY_USAGE_SYNTAX_T *)v_eku;
	PKIExtKeyUsageSyntax *asn=NULL;
	unsigned char *der=NULL;
	size_t dersize;
	unsigned char *oid;
	size_t oidsize=0;

	do
	{
		asn=PKINewExtKeyUsageSyntax(ctx->certasnctx);
		if(!asn)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		for(i=0;i<eku->numPurposes;i++)
		{
			oid=NULL;
			for(j=0;TC_KEY_PURPOSE_MAP[j].oid;i++)
			{
				if(TC_KEY_PURPOSE_MAP[j].id==eku->purpose[i])
				{
					oid=TC_KEY_PURPOSE_MAP[j].oid;
					oidsize=TC_KEY_PURPOSE_MAP[j].oidsize;
					break;
				}
			}
			if(!oid)
			{
				/*unsupported key purpose*/
				status=TC_E_OTHER;
				break;
			}
			asn->elt[i]=PKINewKeyPurposeId(ctx->certasnctx);
			PKIPutOctVal(ctx->certasnctx,asn->elt[i],oid,oidsize);
			asn->n++;
		} /*for each purpose*/
		if(status)
			break;

		dersize=PKISizeofExtKeyUsageSyntax(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackExtKeyUsageSyntax(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		status=tc_add_extension(ext,
								PKIid_ce_extKeyUsage_OID,
								PKIid_ce_extKeyUsage_OID_LEN,
								critical,
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreeExtKeyUsageSyntax(ctx->certasnctx,asn);
	return status;
} /* AddExtKeyUsageSyntax */

static int GetExtKeyUsageSyntax (void **ret,
								 const PKIExtension *ext,
								 TC_CONTEXT *ctx)
{
	int i,j,status=0;
	PKIExtKeyUsageSyntax *asn=NULL;
	TC_EXT_KEY_USAGE_SYNTAX_T *eku=NULL;

	do
	{
		PKIUnpackExtKeyUsageSyntax(ctx->certasnctx,&asn,ext->extnValue.val,ext->extnValue.len,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		eku=TC_Alloc(ctx->memMgr,sizeof(TC_EXT_KEY_USAGE_SYNTAX_T *));
		if(!eku)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(eku,0,sizeof(TC_EXT_KEY_USAGE_SYNTAX_T *));
		eku->numPurposes=asn->n;
		for(i=0;i<asn->n;i++)
		{
			/*find the enum for this purpose*/
			eku->purpose[i]=TC_KEY_PURPOSE_ID_UNKNOWN;
			for(j=0;TC_KEY_PURPOSE_MAP[j].oid;j++)
				if(!memcmp(asn->elt[i]->val,TC_KEY_PURPOSE_MAP[j].oid,asn->elt[i]->len))
				{
					eku->purpose[i]=TC_KEY_PURPOSE_MAP[j].id;
					break;
				}
		}/*for*/
	}
	while(0);
	if(asn)
		PKIFreeExtKeyUsageSyntax(ctx->certasnctx,asn);
	if(status && eku)
	{
		TC_Free(ctx->memMgr,eku);
		eku=NULL;
	}
	*ret=(void*)eku;
	return status;
} /* GetExtKeyUsageSyntax */

int tc_DistPointNameToPKI (PKIDistributionPointName **asn,
						   TC_DISTRIBUTION_POINT_NAME_T *dp,
						   TC_CONTEXT *ctx)
{
	int status=0;
	PKIGeneralNames *pkign=NULL;
	PKIRelativeDistinguishedName *pkirdn=NULL;

	do
	{
		*asn=PKINewDistributionPointName(ctx->certasnctx);
		if(!*asn)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		if(dp->type==TC_DISTRIBUTION_POINT_NAME_TYPE_GEN_NAMES)
		{
			TC_GEN_NAMES_LIST_T *gn=dp->name;

			pkign=PKINewGeneralNames(ctx->certasnctx);
			status=tc_GEN_NAMES_to_PKIGeneralNames(pkign,gn,ctx);
			if(status)
				break;
			(*asn)->CHOICE_field_type=0xA0|0x00;
			(*asn)->data=(void*)pkign;
			pkign=NULL;
		}
		else if(dp->type==TC_DISTRIBUTION_POINT_NAME_TYPE_RDN)
		{
			char *rdn=dp->name;

			pkirdn=PKINewRelativeDistinguishedName(ctx->certasnctx);
			status=ParseStringRDN(pkirdn,rdn,ctx);
			if(status)
				break;
			(*asn)->CHOICE_field_type=0xA0|0x01;
			(*asn)->data=(void*)pkirdn;
			pkirdn=NULL;
		}
		else
		{
			/*invalid type*/
			status = TC_E_OTHER;
		}
	}
	while(0);
	if(status)
	{
		if(*asn)
		{
			PKIFreeDistributionPointName(ctx->certasnctx,*asn);
			*asn=NULL;
		}
		if(pkign)
			PKIFreeGeneralNames(ctx->certasnctx,pkign);
		if(pkirdn)
			PKIFreeRelativeDistinguishedName(ctx->certasnctx,pkirdn);
	}
	return status;
}

static int AddCRLDistributionPoints (TC_ExtensionList *ext,
									 const void *v_cdp,
									 int critical,
									 TC_CONTEXT *ctx)
{
	int i,status=0;
	TC_CRL_DISTRIBUTION_POINTS_T *cdp=(TC_CRL_DISTRIBUTION_POINTS_T *)v_cdp;
	PKICRLDistPointsSyntax *asn=NULL;
	unsigned char *der=NULL;
	size_t dersize;

	do
	{
		asn=PKINewCRLDistPointsSyntax(ctx->certasnctx);
		for(i=0;i<cdp->numDistPoints;i++)
		{
			asn->n++;
			asn->elt[i]=PKINewDistributionPoint(ctx->certasnctx);

			if(cdp->distPoint[i]->name)
			{
				status=tc_DistPointNameToPKI(&asn->elt[i]->distributionPoint,
											 cdp->distPoint[i]->name,
											 ctx);
				if(status)
					break;
			}
			if(cdp->distPoint[i]->reasonFlags)
			{
				unsigned char b;
				asn->elt[i]->reasons=PKINewBIT_STRING(ctx->certasnctx);
				b=cdp->distPoint[i]->reasonFlags<<1;
				/*we use bits 0(lsb)-6(msb), which leaves 1 bit unused*/
				PKIPutBitString(ctx->certasnctx,asn->elt[i]->reasons,&b,1,1);
			}
			if(cdp->distPoint[i]->issuer)
			{
				asn->elt[i]->cRLIssuer=PKINewGeneralNames(ctx->certasnctx);
				status=tc_GEN_NAMES_to_PKIGeneralNames(asn->elt[i]->cRLIssuer,
													   cdp->distPoint[i]->issuer,
													   ctx);
				if(status)
					break;
			}
		}/*for*/
		if(status)
			break;

		dersize=PKISizeofCRLDistPointsSyntax(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackCRLDistPointsSyntax(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		status=tc_add_extension(ext,
								PKIid_ce_cRLDistributionPoints_OID,
								PKIid_ce_cRLDistributionPoints_OID_LEN,
								critical,
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	return status;
} /* AddCRLDistributionPoints */

void tc_FreeDistributionPointName(TC_DISTRIBUTION_POINT_NAME_T *name,
								  TC_CONTEXT *ctx)
{
	if(name->type==TC_DISTRIBUTION_POINT_NAME_TYPE_RDN)
		TC_Free(ctx->memMgr,name->name);
	else
		tc_free_gennamelist(name->name,ctx);
	TC_Free(ctx->memMgr,name);
}

void tc_FreeCRLDistributionPoints (TC_CRL_DISTRIBUTION_POINTS_T *cdp,
								   TC_CONTEXT *ctx)
{
	int i;

	for(i=0;i<cdp->numDistPoints;i++)
	{
		if(cdp->distPoint[i])
		{
			if(cdp->distPoint[i]->name)
				tc_FreeDistributionPointName(cdp->distPoint[i]->name,ctx);

			if(cdp->distPoint[i]->issuer)
				tc_free_gennamelist(cdp->distPoint[i]->issuer,ctx);

			TC_Free(ctx->memMgr,cdp->distPoint[i]);
		}
	}/*for*/
	TC_Free(ctx->memMgr,cdp);
}

int tc_DistPointNameToTC (TC_DISTRIBUTION_POINT_NAME_T **dp,
						  PKIDistributionPointName *asn,
						  TC_CONTEXT *ctx)
{
	int status=0;

	do
	{
		*dp=TC_Alloc(ctx->memMgr, sizeof(TC_DISTRIBUTION_POINT_NAME_T));
		if(!*dp)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(*dp,0,sizeof(TC_DISTRIBUTION_POINT_NAME_T));

		if(asn->CHOICE_field_type==(0xA0|0x00))
		{
			PKIGeneralNames *pkign=(PKIGeneralNames*)asn->data;
			TC_GEN_NAMES_LIST_T *gn;

			status=tc_PKIGeneralNamesToTC(&gn,pkign,ctx);
			if(status)
				break;
			(*dp)->name=(void*)gn;
			(*dp)->type=TC_DISTRIBUTION_POINT_NAME_TYPE_GEN_NAMES;
		}
		else if(asn->CHOICE_field_type==(0xA0|0x01))
		{
			PKIRelativeDistinguishedName *pkirdn=(PKIRelativeDistinguishedName *)asn->data;
			char *rdn;

			rdn=RDNToString(pkirdn,ctx);
			(*dp)->name=(void*)rdn;
			(*dp)->type=TC_DISTRIBUTION_POINT_NAME_TYPE_RDN;
		}
		else
		{
			/*this shouldn't happen so long as the ASN definition
			  doesnt change since the compiler will produce an
			  error.*/
			status=TC_E_OTHER;
		}
	}
	while(0);
	if(status&&*dp)
	{
		tc_FreeDistributionPointName(*dp,ctx);
		*dp=NULL;
	}
	return status;
} /* tc_DistPointNameToTC */

static int GetCRLDistributionPoints (void **ret,
									 const PKIExtension *ext,
									 TC_CONTEXT *ctx)
{
	int i,status=0;
	TC_CRL_DISTRIBUTION_POINTS_T *cdp=NULL;
	PKICRLDistPointsSyntax *asn=NULL;
	TC_GEN_NAMES_LIST_T *gn=NULL;

	do
	{
		PKIUnpackCRLDistPointsSyntax(ctx->certasnctx,&asn,ext->extnValue.val,ext->extnValue.len,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		cdp=TC_Alloc(ctx->memMgr,sizeof(TC_CRL_DISTRIBUTION_POINTS_T));
		if(!cdp)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(cdp,0,sizeof(TC_CRL_DISTRIBUTION_POINTS_T));

		for(i=0;i<asn->n;i++)
		{
			cdp->distPoint[i]=TC_Alloc(ctx->memMgr,sizeof(TC_DISTRIBUTION_POINT_T));
			if(!cdp->distPoint[i])
			{
				status=TC_E_NOMEMORY;
				break;
			}
			memset(cdp->distPoint[i],0,sizeof(TC_DISTRIBUTION_POINT_T));

			if(asn->elt[i]->distributionPoint)
			{
				status=tc_DistPointNameToTC(&cdp->distPoint[i]->name,
											asn->elt[i]->distributionPoint,
											ctx);
				if(status)
					break;
			}

			if(asn->elt[i]->reasons)
				cdp->distPoint[i]->reasonFlags=(*asn->elt[i]->reasons->val >> 1)&0x7f;

			if(asn->elt[i]->cRLIssuer)
			{
				status=tc_PKIGeneralNamesToTC(&cdp->distPoint[i]->issuer,asn->elt[i]->cRLIssuer,ctx);
				if(status)
					break;
			}
		}/*for*/
		if(status)
			break;
	}
	while(0);
	if(status&&cdp)
	{
		tc_FreeCRLDistributionPoints(cdp,ctx);
		cdp=NULL;
	}
	if(asn)
		PKIFreeCRLDistPointsSyntax(ctx->certasnctx,asn);
	if(gn)
		tc_free_gennamelist(gn,ctx);
	*ret=(void*)cdp;
	return status;
} /* GetCRLDistributionPoints */

static int AddAuthorityInfoAccess (TC_ExtensionList *ext,
								   const void *v_info,
								   int critical,
								   TC_CONTEXT *ctx)
{
	int i,status=0;
	TC_AUTHORITY_INFO_ACCESS_T *info;
	PKIAuthorityInfoAccessSyntax *asn=NULL;
	unsigned char *der=NULL;
	size_t dersize;

	(void)critical;/*unused, always noncritical*/
	do
	{
		info=(TC_AUTHORITY_INFO_ACCESS_T *)v_info;

		asn=PKINewAuthorityInfoAccessSyntax(ctx->certasnctx);
		for(i=0;i<info->numDescrips;i++)
		{
			asn->n++;
			asn->elt[i]=PKINewAccessDescription(ctx->certasnctx);

			PKIPutOctVal(ctx->certasnctx,&asn->elt[i]->accessMethod,info->descrip[i]->method,info->descrip[i]->methodsize);

			status=tc_GEN_NAME_to_PKIGeneralName(&asn->elt[i]->accessLocation,
												 info->descrip[i]->location,
												 ctx);
			if(status)
				break;
		}
		if(status)
			break;

		dersize=PKISizeofAuthorityInfoAccessSyntax(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackAuthorityInfoAccessSyntax(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		status=tc_add_extension(ext,
								PKIid_pe_authorityInfoAccess_OID,
								PKIid_pe_authorityInfoAccess_OID_LEN,
								0,/*always noncritical (RFC2459 4.2.2.1) */
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreeAuthorityInfoAccessSyntax(ctx->certasnctx,asn);
	return status;
} /* AddAuthorityInfoAccess */

void tc_free_genname (TC_GEN_NAME_T *g, TC_CONTEXT *c)
{
	if(g->name)
		TC_Free(c->memMgr,g->name);
	TC_Free(c->memMgr,g);
} /* tc_free_genname */

void tc_FreeAuthorityInfoAccess (TC_CONTEXT *ctx,
								 TC_AUTHORITY_INFO_ACCESS_T *info)
{
	int i;

	for(i=0;i<info->numDescrips;i++)
	{
		if(info->descrip[i])
		{
			if(info->descrip[i]->method)
				TC_Free(ctx->memMgr,info->descrip[i]->method);
			if(info->descrip[i]->location)
				tc_free_genname(info->descrip[i]->location,ctx);
			TC_Free(ctx->memMgr,info->descrip[i]);
		}
	}
	TC_Free(ctx->memMgr,info);
} /* tc_FreeAuthorityInfoAccess */

static int GetAuthorityInfoAccess (void **ret,
								   const PKIExtension *ext,
								   TC_CONTEXT *ctx)
{
	int i,status=0;
	TC_AUTHORITY_INFO_ACCESS_T *info=NULL;
	PKIAuthorityInfoAccessSyntax *asn=NULL;

	do
	{
		PKIUnpackAuthorityInfoAccessSyntax(ctx->certasnctx,
									 &asn,
									 ext->extnValue.val,
									 ext->extnValue.len,
									 &status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		info=TC_Alloc(ctx->memMgr,sizeof(TC_AUTHORITY_INFO_ACCESS_T));
		if(!info)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(info,0,sizeof(TC_AUTHORITY_INFO_ACCESS_T));

		for(i=0;i<asn->n;i++)
		{
			info->numDescrips++;
			info->descrip[i]=TC_Alloc(ctx->memMgr,sizeof(TC_ACCESS_DESCRIPTION_T));
			if(!info->descrip[i])
			{
				status=TC_E_NOMEMORY;
				break;
			}
			memset(info->descrip[i],0,sizeof(TC_ACCESS_DESCRIPTION_T));
			info->descrip[i]->methodsize=asn->elt[i]->accessMethod.len;
			info->descrip[i]->method=TC_Alloc(ctx->memMgr,
											  info->descrip[i]->methodsize);
			if(!info->descrip[i]->method)
			{
				status=TC_E_NOMEMORY;
				break;
			}
			memcpy(info->descrip[i]->method,
				   asn->elt[i]->accessMethod.val,
				   asn->elt[i]->accessMethod.len);

			info->descrip[i]->location=TC_Alloc(ctx->memMgr,sizeof(TC_GEN_NAME_T));
			if(!info->descrip[i]->location)
			{
				status=TC_E_NOMEMORY;
				break;
			}
			memset( info->descrip[i]->location,0,sizeof(TC_GEN_NAME_T));

			status=tc_PKIGeneralNameToTC(info->descrip[i]->location,
										 &asn->elt[i]->accessLocation,
										 ctx);
			if(status)
				break;
		}
	}
	while(0);
	if(status&&info)
	{
		tc_FreeAuthorityInfoAccess(ctx,info);
		info=NULL;
	}
	*ret=(void*)info;
	if(asn)
		PKIFreeAuthorityInfoAccessSyntax(ctx->certasnctx,asn);
	return status;
} /* GetAuthorityInfoAccess */

/****
 *
 * CopyExtList
 *
 * return 
 *   0 - okay
 *   TC_E_INVARGS
 *   TC_E_NOMEMORY
 *   
 *****/
int CopyExtList(TC_ExtensionList **newlist,
				TC_ExtensionList *oldlist,
				TC_CONTEXT *ctx)
{
	int status = 0;
	int i;
	TC_ExtensionList *locallist = NULL;
	PKIExtension *newExt, *oldExt;

	do {

		if (!newlist || !oldlist) {
			status = TC_E_INVARGS;
			break;
		}

		if ((status = tc_create_extlist(&locallist, ctx)) != 0)
			break;

		/* for each extension in the list */
		for (i = 0; i < oldlist->n; i++) {

			oldExt = oldlist->elt[i];
			if (oldExt == NULL) /* problem? */
				continue;

			newExt = PKINewExtension(ctx->certasnctx);
			if (newExt == NULL) {
				status = TC_E_NOMEMORY;
				break;
			}

			PKIPutOctVal(ctx->certasnctx, &newExt->extnID,
						 oldExt->extnID.val, oldExt->extnID.len);

			if (oldExt->critical != NULL) {
				newExt->critical = PKINewBOOLEAN(ctx->certasnctx);
				if (newExt->critical == NULL) {
					PKIFreeExtension(ctx->certasnctx, newExt);
					status = TC_E_NOMEMORY;
					break;
				}
				PKIPutBoolVal(ctx->certasnctx, newExt->critical,
							  oldExt->critical->val);
			}

			PKIPutOctVal(ctx->certasnctx, &newExt->extnValue,
						 oldExt->extnValue.val, oldExt->extnValue.len);

			locallist->elt[i] = newExt;
			locallist->n++;

		} /* for each extension */

		if (status != 0)
			break;

	} while(0);

	if (status != 0) {
		if (locallist != NULL) {
			tc_free_extlist(locallist, ctx);
			locallist = NULL;
		}
	}

	*newlist = locallist;
	return 0;

} /* CopyExtList */

/* vim:ts=4:sw=4:
 */

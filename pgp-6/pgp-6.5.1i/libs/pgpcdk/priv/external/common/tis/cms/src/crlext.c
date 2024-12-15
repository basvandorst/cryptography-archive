/****************************************************************************
 *
 * Copyright (c) 1999, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "cms.h"
#include "cert_oid.h"
#include "extensions.h"

static int AddINTEGERExt( TC_ExtensionList *ext,
						  const void *p_value,
						  int criticality,
						  TC_CONTEXT *ctx,
						  unsigned char *oid,
						  size_t oidlen,
						  boolean defaultCriticality )
{
	int status = 0;
	PKIINTEGER *IntBuf = NULL;
	unsigned char *derBuffer = NULL;
	size_t  derSize = 0;
	int value = *((unsigned int *)p_value);
	boolean mycriticality;

	do
	{
		/* ------ Basic parameter checks ------ */

		/* Parameters checked by tc_setextval */

		/* ---------- Set criticality ------------------ */

		if (criticality < 0)              /* use default criticality */
			mycriticality = defaultCriticality;
		else                              /* take override value */
			mycriticality = (boolean) criticality;

		IntBuf = PKINewINTEGER(ctx->certasnctx);
		if ( IntBuf == NULL )
		{
			status = TC_E_NOMEMORY;
			break;
		} /* if IntBuf */

		if ( PKIPutIntVal(ctx->certasnctx,
						  IntBuf,
						  (long)value) == -1 )
		{
			status = -1;
			break;
		} /* if put_int_val */

		derSize = PKISizeofINTEGER( ctx->certasnctx, IntBuf, PKITRUE );
		if ( derSize <= 0)
		{
			status = TC_E_OTHER;
			break;
		}

		derBuffer = TC_Alloc( ctx->memMgr, derSize );
		if (derBuffer == NULL)
		{
			status = TC_E_NOMEMORY;
			break;
		}

		PKIPackINTEGER( ctx->certasnctx, derBuffer, derSize,
						IntBuf, &status );
		if ( status != 0 )
			break;

		/* add to extension list */
		if ((status = tc_add_extension(ext,
									   oid,
									   oidlen,
									   mycriticality,
									   (unsigned char *) derBuffer,
									   derSize,
									   ctx)) != 0)
			break;

	} while (0);

	/* clean up */
	if (derBuffer != NULL)
		TC_Free(ctx->memMgr, derBuffer);

	return status;

}

int AddCRLNumberExt(TC_ExtensionList *ext,
					const void *p_CRLnumber,
					int criticality,
					TC_CONTEXT *ctx)
{
	boolean defaultCriticality = PKIFALSE;
	int status;


	status = AddINTEGERExt( ext, p_CRLnumber, criticality, ctx,
							PKIid_ce_cRLNumber_OID, PKIid_ce_cRLNumber_OID_LEN,
							defaultCriticality );

	return status;
}

int AddDeltaCRLIndicatorExt(TC_ExtensionList *ext,
							const void *p_deltaCRLIndicator,
							int criticality,
							TC_CONTEXT *ctx)
{
	boolean defaultCriticality = PKIFALSE;
	int status;


	status = AddINTEGERExt( ext, p_deltaCRLIndicator, criticality, ctx,
							PKIid_ce_deltaCRLIndicator_OID,
							PKIid_ce_deltaCRLIndicator_OID_LEN,
							defaultCriticality );

	return status;
}

int AddCRLReason (TC_ExtensionList *ext,
				  const void *p_CRLReason,
				  int criticality,
				  TC_CONTEXT *ctx)
{
	PKIINTEGER *i;
	int status=0;

	i=PKINewINTEGER(ctx->certasnctx);
	if(!i)
		return TC_E_NOMEMORY;
	PKIPutIntVal(ctx->certasnctx,i,(TC_CRL_REASON_T)p_CRLReason);
	if(status==0)
	{
		status=AddINTEGERExt(ext,
							 i,
							 criticality,
							 ctx,
							 PKIid_ce_cRLReasons_OID,
							 PKIid_ce_cRLReasons_OID_LEN,
							 0/*always noncritical*/);
	}

	PKIFreeINTEGER(ctx->certasnctx,i);
	return status;
}

static int GetINTEGERExt (void **p_value,
						  const PKIExtension *ext,
						  TC_CONTEXT *ctx)
{
	PKIINTEGER *IntBuf;
	int *p;
	int status;

	do
	{
		p = malloc( sizeof( int ) );
		*p_value = p;

		/* unpack extension data */
		(void)PKIUnpackINTEGER(ctx->certasnctx,
							   &IntBuf,
							   ext->extnValue.val,
							   ext->extnValue.len, &status);
		if (status != 0 || IntBuf == NULL)
		{
			status = TC_E_EXTENSION;
			break;
		}

		*p = (int)PKIGetIntVal(ctx->certasnctx,
							   IntBuf,
							   &status);
		if (status != 0)
			break;
	} while ( 0 );

	PKIFreeINTEGER( ctx->certasnctx, IntBuf );
	return status;
}

int GetCRLNumberExt(void **p_CRLNumber,
					const PKIExtension *ext,
					TC_CONTEXT *ctx)
{
	return GetINTEGERExt( p_CRLNumber, ext, ctx );
}

int GetDeltaCRLIndicatorExt(void **p_DeltaCRLIndicator,
							const PKIExtension *ext,
							TC_CONTEXT *ctx)
{
	return GetINTEGERExt( p_DeltaCRLIndicator, ext, ctx );
}

int GetCRLReason (void **ret,
				  const PKIExtension *ext,
				  TC_CONTEXT *ctx)
{
	return GetINTEGERExt(ret,ext,ctx);
}

/* TODO: this was moved to extensions.c */
#if 0
/*
 *  This function allocates the PKI list and fills it.
 */
static int
TC_GEN_NAMES_to_PKIGeneralNames(PKIGeneralNames **PKIlist,
								TC_GEN_NAMES_LIST_T *nameList,
								TC_CONTEXT *ctx)
{
	TC_GEN_NAME_T *name;
	PKIGeneralNames *localNameList;
	PKIGeneralName *asnName = NULL;
	PKIIA5String *string;
	PKIOCTET_STRING *octblock;
	PKIOBJECT_ID *objidblock;
	int status = 0;
	int i;

	do
	{
		localNameList = PKINewGeneralNames(ctx->certasnctx);
		if (localNameList == NULL)
		{
			status = TC_E_NOMEMORY;
			break;
		}

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

			/* set the CHOICE field type and the choice's data; these
			   are explicitly tagged values, so we need to use the
			   tag value for the CHOICE field */
			switch(name->nameType)
			{
				case TC_rfc822Name:
					asnName->CHOICE_field_type = 0x80|0x20|0x01;
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
					asnName->CHOICE_field_type = 0x80|0x20|0x02;
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
					asnName->CHOICE_field_type = 0x80|0x20|0x04;
					CopyName((PKIName **)&asnName->data,
							 (PKIName *)name->name, ctx);
					break;

				case TC_uniformResourceIdentifier:
					asnName->CHOICE_field_type = 0x80|0x20|0x06;
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
					asnName->CHOICE_field_type = 0x80|0x20|0x07;
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
					asnName->CHOICE_field_type = 0x80|0x20|0x08;
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
			} /* switch */

			if (status != 0)
				break;

			localNameList->elt[i] = asnName;
			localNameList->n = i+1;

		} /* for each name */

	} while(0);

			PKIlist = &localNameList;

			return status;

} /* TC_GEN_NAMES_to_PKIGeneralNames */
#endif /* 0 */

static int
CreateAuthorityKeyIdentifierDER(const TC_AUTHORITY_KEY_IDENTIFIER_T *simpleAKI,
								unsigned char **buf,
								size_t *size,
								TC_CONTEXT *ctx)
{
	PKIAuthorityKeyIdentifier *pki_aki = NULL;
	int status = 0;

	do
	{
		/* verify valid output pointers */
		if ( buf == NULL || size == NULL || simpleAKI == NULL )
		{
			status = TC_E_INVARGS;
			break;
		} /* if */

		pki_aki = PKINewAuthorityKeyIdentifier(ctx->certasnctx);
		if ( pki_aki == (PKIAuthorityKeyIdentifier *) 0 )
		{
			status = TC_E_NOMEMORY;
			break;
		}

		if ( simpleAKI->keyIdentifier_len > 0 )
		{
			pki_aki->keyIdentifier = PKINewKeyIdentifier( ctx->certasnctx );
			if(!pki_aki->keyIdentifier)
			{
				status=TC_E_NOMEMORY;
				break;
			}
			PKIPutOctVal(ctx->certasnctx,pki_aki->keyIdentifier,simpleAKI->keyIdentifier,simpleAKI->keyIdentifier_len);
		}

		if ( simpleAKI->authorityCertIssuer != NULL )
		{
			PKIGeneralNames *PGN;

			PGN=PKINewGeneralNames(ctx->certasnctx);
			if(!PGN)
			{
				status=TC_E_NOMEMORY;
				break;
			}
			status = tc_GEN_NAMES_to_PKIGeneralNames( PGN,
													  simpleAKI->authorityCertIssuer,
													  ctx);
			pki_aki->authorityCertIssuer = PGN;
		}

		if ( simpleAKI->authorityCertSerialNumber_len > 0 )
		{
			pki_aki->authorityCertSerialNumber=PKINewCertificateSerialNumber(ctx->certasnctx);
			if(!pki_aki->authorityCertSerialNumber)
			{
				status=TC_E_NOMEMORY;
				break;
			}
			PKIPutOctVal(ctx->certasnctx,pki_aki->authorityCertSerialNumber,simpleAKI->authorityCertSerialNumber,simpleAKI->authorityCertSerialNumber_len);
		}

		*size=PKISizeofAuthorityKeyIdentifier(ctx->certasnctx,pki_aki,1);
		*buf=TC_Alloc(ctx->memMgr,*size);
		if(!*buf)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackAuthorityKeyIdentifier(ctx->certasnctx,*buf,*size,pki_aki,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
	} while ( 0 );

	if(pki_aki)
		PKIFreeAuthorityKeyIdentifier(ctx->certasnctx,pki_aki);

	return status;
}

int AddAuthorityKeyIdentifier (TC_ExtensionList *ext,
							   const void *v_sAKI,
							   int critical,
							   TC_CONTEXT *ctx)
{
	int status;
	unsigned char *derBuffer = NULL;
	size_t  derSize;
	TC_AUTHORITY_KEY_IDENTIFIER_T *simpleAKI;

	(void)critical;

	do
	{
		/* ------ Basic parameter checks ------ */

		if(!ext || !v_sAKI|| !ctx)
			return TC_E_INVARGS;

		simpleAKI=(TC_AUTHORITY_KEY_IDENTIFIER_T*)v_sAKI;

		status=CreateAuthorityKeyIdentifierDER(simpleAKI,&derBuffer,&derSize,ctx);
		if(status)
			break;

		/* ----- Add extension ----- */
		status=tc_add_extension(ext,
								PKIid_ce_authorityKeyIdentifier_OID,
								PKIid_ce_authorityKeyIdentifier_OID_LEN,
								0, /* always non-critical (RFC2459 4.2.1.1) */
								derBuffer,
								derSize,
								ctx);
	}
	while ( 0 );

	/* ----- clean up ----- */

	if(derBuffer)
		TC_Free(ctx->memMgr,derBuffer);

	return status;
}

int GetAuthorityKeyIdentifier (void **authKeyId,
							   const PKIExtension *ext,
							   TC_CONTEXT *ctx)
{
	TC_AUTHORITY_KEY_IDENTIFIER_T *simpleAKI=NULL;
	PKIAuthorityKeyIdentifier *pki_aki = NULL;
	int status=0;

	do
	{
		/* ----- Check arguments ----- */
		if(!authKeyId || !ext || !ctx)
		{
			status = TC_E_INVARGS;
			break;
		}

		/* ----- Parse extension DER ----- */

		PKIUnpackAuthorityKeyIdentifier(ctx->certasnctx,&pki_aki,ext->extnValue.val,ext->extnValue.len,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		/* ----- allocate storage for simple data structure ----- */

		simpleAKI = TC_Alloc(ctx->memMgr, sizeof (TC_AUTHORITY_KEY_IDENTIFIER_T));
		memset(simpleAKI,0,sizeof(TC_AUTHORITY_KEY_IDENTIFIER_T));
		*authKeyId=(void*)simpleAKI;

		/* ----- Convert from compiler to simple data structure */

		if(pki_aki->keyIdentifier)
		{
			simpleAKI->keyIdentifier = pki_aki->keyIdentifier->val;
			simpleAKI->keyIdentifier_len = pki_aki->keyIdentifier->len;
			pki_aki->keyIdentifier->val=NULL;
			TC_Free(ctx->memMgr,pki_aki->keyIdentifier);
			pki_aki->keyIdentifier=NULL;
		}

		if(pki_aki->authorityCertIssuer)
		{
			status=tc_PKIGeneralNamesToTC(&simpleAKI->authorityCertIssuer,
										  pki_aki->authorityCertIssuer,
										  ctx);
			if(status)
				break;
		}

		if(pki_aki->authorityCertSerialNumber)
		{
			simpleAKI->authorityCertSerialNumber=pki_aki->authorityCertSerialNumber->val;
			simpleAKI->authorityCertSerialNumber_len=pki_aki->authorityCertSerialNumber->len;
			pki_aki->authorityCertSerialNumber->val=NULL;
			TC_Free(ctx->memMgr,pki_aki->authorityCertSerialNumber);
			pki_aki->authorityCertSerialNumber=NULL;
		}
	}
	while (0);

	/* ----- Clean up ----- */

	if(pki_aki)
		PKIFreeAuthorityKeyIdentifier(ctx->certasnctx,pki_aki);

	return status;
}

void tc_FreeIssuingDistributionPoint(TC_ISSUING_DISTRIBUTION_POINT_T *p,
									 TC_CONTEXT *c) {
	if(p->name)
		tc_FreeDistributionPointName(p->name,c);
}

int AddIssuingDistributionPoint (TC_ExtensionList *ext,
								 const void *v_idp,
								 int critical,
								 TC_CONTEXT *ctx)
{
	int status=0;
	unsigned char *der=NULL;
	size_t dersize;
	TC_ISSUING_DISTRIBUTION_POINT_T *idp;
	PKIIssuingDistributionPoint *asn=NULL;

	(void)critical;/*unused, always critical (RFC2459 5.2.5)*/
	do
	{
		idp=(TC_ISSUING_DISTRIBUTION_POINT_T *)v_idp;
		asn=PKINewIssuingDistributionPoint(ctx->certasnctx);
		if(idp->name)
		{
			status=tc_DistPointNameToPKI(&asn->distributionPoint,
										 idp->name,
										 ctx);
			if(status)
				break;
		}
		if(idp->onlyContainsUserCerts)
		{
			asn->onlyContainsUserCerts=PKINewBOOLEAN(ctx->certasnctx);
			PKIPutBoolVal(ctx->certasnctx,asn->onlyContainsUserCerts,PKITRUE);
		}
		if(idp->onlyContainsCACerts)
		{
			asn->onlyContainsCACerts=PKINewBOOLEAN(ctx->certasnctx);
			PKIPutBoolVal(ctx->certasnctx,asn->onlyContainsCACerts,PKITRUE);
		}
		if(idp->onlySomeReasons)
		{
			unsigned char b=idp->onlySomeReasons<<1;

			asn->onlySomeReasons=PKINewReasonFlags(ctx->certasnctx);
			PKIPutBitString(ctx->certasnctx,asn->onlySomeReasons,&b,1,1);
		}
		if(idp->indirectCRL)
		{
			asn->indirectCRL=PKINewBOOLEAN(ctx->certasnctx);
			PKIPutBoolVal(ctx->certasnctx,asn->indirectCRL,PKITRUE);
		}
		dersize=PKISizeofIssuingDistributionPoint(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackIssuingDistributionPoint(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		status=tc_add_extension(ext,
								PKIid_ce_issuingDistributionPoint_OID,
								PKIid_ce_issuingDistributionPoint_OID_LEN,
								1,/*always critical (RFC2459 5.2.5)*/
								der,
								dersize,
								ctx);
	}
	while(0);
	if(asn)
		PKIFreeIssuingDistributionPoint(ctx->certasnctx,asn);
	if(der)
		TC_Free(ctx->memMgr,der);
	return status;
} /* AddIssuingDistributionPoint */

int GetIssuingDistributionPoint (void **ret,
								 const PKIExtension *ext,
								 TC_CONTEXT *ctx)
{
	int status=0;
	TC_ISSUING_DISTRIBUTION_POINT_T *idp=NULL;
	PKIIssuingDistributionPoint *asn=NULL;

	do
	{
		PKIUnpackIssuingDistributionPoint(ctx->certasnctx,
										  &asn,
										  ext->extnValue.val,
										  ext->extnValue.len,
										  &status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		idp=TC_Alloc(ctx->memMgr,sizeof(TC_ISSUING_DISTRIBUTION_POINT_T));
		if(!idp)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		memset(idp,0,sizeof(TC_ISSUING_DISTRIBUTION_POINT_T));
		if(asn->distributionPoint)
		{
			status=tc_DistPointNameToTC(&idp->name,
										asn->distributionPoint,
										ctx);
			if(status)
				break;
		}

		if(asn->onlyContainsUserCerts)
			idp->onlyContainsUserCerts=PKIGetBoolVal(ctx->certasnctx,
													 asn->onlyContainsUserCerts);
		if(asn->onlyContainsCACerts)
			idp->onlyContainsCACerts=PKIGetBoolVal(ctx->certasnctx,
												   asn->onlyContainsCACerts);

		if(asn->onlySomeReasons)
			idp->onlySomeReasons=(*asn->onlySomeReasons->val>>1) & 0x7f;

		if(asn->indirectCRL)
			idp->indirectCRL=PKIGetBoolVal(ctx->certasnctx,
										   asn->indirectCRL);
	}
	while(0);
	if(status&&idp)
	{
		tc_FreeIssuingDistributionPoint(idp,ctx);
		idp=NULL;
	}
	*ret=(void*)idp;
	if(asn)
		PKIFreeIssuingDistributionPoint(ctx->certasnctx,asn);
	return status;
} /* GetIssuingDistributionPoint */

static struct
{
	TC_HOLD_INSTRUCTION_T code;
	unsigned char *oid;
	size_t oidsize;
} TC_HOLD_INSTRUCTION_MAP[] = {
	{
		TC_HOLD_INSTRUCTION_NONE,
		PKIid_holdinstruction_none_OID,
		PKIid_holdinstruction_none_OID_LEN
	},
	{
		TC_HOLD_INSTRUCTION_CALL_ISSUER,
		PKIid_holdinstruction_callissuer_OID,
		PKIid_holdinstruction_callissuer_OID_LEN
	},
	{
		TC_HOLD_INSTRUCTION_REJECT,
		PKIid_holdinstruction_reject_OID,
		PKIid_holdinstruction_reject_OID_LEN
	},
	{
		(TC_HOLD_INSTRUCTION_T) -1,
		NULL,
		0
	}
};

/*****
 *
 * AddHoldInstructionCode
 *
 * Inputs:
 *	p_hic
 *		integer (not pointer) value containing TC_HOLD_INSTRUCTION_T
 *	critical
 *		-1	use default criticality (FALSE)
 *		0	FALSE
 *		1	TRUE
 *	ctx
 *		CMS context structure
 *
 * Outputs:
 *	ext
 *		extension list to append
 *
 * Returns:
 *	0	success
 *	<0	failure
 */

int AddHoldInstructionCode (TC_ExtensionList *ext,
							const void *p_hic,
							int critical,
							TC_CONTEXT *ctx)
{
	TC_HOLD_INSTRUCTION_T code;
	PKIOBJECT_ID *asn=NULL;
	int j,status=0;
	unsigned char *der=NULL;
	size_t dersize;
	unsigned char *oid;
	size_t oidsize=0;

	do
	{
		code=(TC_HOLD_INSTRUCTION_T)p_hic;
		/*find the oid corresponding this this value*/
		oid=NULL;
		for(j=0;TC_HOLD_INSTRUCTION_MAP[j].oid;j++)
			if(TC_HOLD_INSTRUCTION_MAP[j].code==code)
			{
				oid=TC_HOLD_INSTRUCTION_MAP[j].oid;
				oidsize=TC_HOLD_INSTRUCTION_MAP[j].oidsize;
				break;
			}
		if(!oid)
		{
			status=TC_E_OTHER;
			break;
		}
		asn=PKINewOBJECT_ID(ctx->certasnctx);
		if(!asn)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPutOctVal(ctx->certasnctx,asn,oid,oidsize);

		dersize=PKISizeofOBJECT_ID(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackOBJECT_ID(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		status=tc_add_extension(ext,
								PKIid_ce_holdInstructionCode_OID,
								PKIid_ce_holdInstructionCode_OID_LEN,
								(critical < 0) ? 0 : critical,
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreeOBJECT_ID(ctx->certasnctx,asn);
	return status;
} /* AddHoldInstructionCode */

/*****
 *
 * GetHoldInstructionCode
 *
 * Inputs:
 *	ext
 *		pointer to extension to process
 *	ctx
 *		pointer to CMS context structure
 *
 * Outputs:
 *	ret
 *		TC_HOLD_INSTRUCTION_T value
 *
 * Returns:
 *	0	success
 *	<0	failure
 */
int GetHoldInstructionCode (void **ret,
							const PKIExtension *ext,
							TC_CONTEXT *ctx)
{
	TC_HOLD_INSTRUCTION_T hic=TC_HOLD_INSTRUCTION_NONE;
	PKIHoldInstructionCode *asn=NULL;
	int j,status=0;

	do
	{
		PKIUnpackHoldInstructionCode(ctx->certasnctx,
									 &asn,
									 ext->extnValue.val,
									 ext->extnValue.len,
									 &status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}

		/* find the enum for the given oid */
		for(j=0;TC_HOLD_INSTRUCTION_MAP[j].oid;j++)
		{
			if(!memcmp(asn->val,TC_HOLD_INSTRUCTION_MAP[j].oid,asn->len))
			{
				hic=TC_HOLD_INSTRUCTION_MAP[j].code;
				break;
			}
		}
	}
	while(0);
	if(asn)
		PKIFreeHoldInstructionCode(ctx->certasnctx,asn);
	*ret=(void*)hic;
	return status;
} /* GetHoldInstructionCode */

/*****
 *
 * AddInvalidityDate
 *
 * Inputs:
 *	p_id
 *		time_t value containing the invalidity date
 *	critical
 *		-1	default criticality (FALSE)
 *		0	FALSE
 *		1	TRUE
 *	ctx
 *		pointer to CMS context structure
 *
 * Outputs:
 *	ext
 *		extension list to append to
 *
 * Returns:
 *	0	success
 *	<0	failure
 */

int AddInvalidityDate (TC_ExtensionList *ext,
					   const void *p_id,
					   int critical,
					   TC_CONTEXT *ctx)
{
	time_t id;
	int status=0;
	unsigned char *der=NULL;
	size_t dersize;
	PKIGeneralizedTime *asn=NULL;

	do
	{
		id=(time_t)p_id;
		status=tc_EncodeGeneralizedTime(&asn,id,ctx);
		if(status)
			break;
		dersize=PKISizeofGeneralizedTime(ctx->certasnctx,asn,PKITRUE);
		der=TC_Alloc(ctx->memMgr,dersize);
		if(!der)
		{
			status=TC_E_NOMEMORY;
			break;
		}
		PKIPackGeneralizedTime(ctx->certasnctx,der,dersize,asn,&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		status=tc_add_extension(ext,
								PKIid_ce_invalidityDate_OID,
								PKIid_ce_invalidityDate_OID_LEN,
								(critical < 0) ? 0 : critical,
								der,
								dersize,
								ctx);
	}
	while(0);
	if(der)
		TC_Free(ctx->memMgr,der);
	if(asn)
		PKIFreeGeneralizedTime(ctx->certasnctx,asn);
	return status;
} /* AddInvalidityDate */

/*****
 *
 * GetInvalidityDate
 *
 * Inputs:
 *	ext
 *		pointer to extension to process
 *	ctx
 *		pointer to CMS context structure
 *
 * Outputs:
 *	ret
 *		time_t value containing invalidity date
 *
 * Returns:
 *	0	success
 *	<0	failure
 */

int GetInvalidityDate (void **ret,
					   const PKIExtension *ext,
					   TC_CONTEXT *ctx)
{
	int status=0;
	PKIInvalidityDate *asn=NULL;
	time_t id=0;

	do
	{
		PKIUnpackInvalidityDate(ctx->certasnctx,
								&asn,
								ext->extnValue.val,
								ext->extnValue.len,
								&status);
		if(status)
		{
			status=compiler2tc_error(status);
			break;
		}
		id=tc_decode_GeneralizedTime(asn);
	}
	while(0);
	if(asn)
		PKIFreeInvalidityDate(ctx->certasnctx,asn);
	*ret=(void*)id;
	return status;
} /* GetInvalidityDate */

/* vim:ts=4:sw=4:
 */

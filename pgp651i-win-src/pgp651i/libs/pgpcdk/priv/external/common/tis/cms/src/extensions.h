/****************************************************************************
 *
 * Copyright (c) 1999, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

int AddAuthorityKeyIdentifier(TC_ExtensionList *ext,
							  const void *authKeyId,
							  int critical,
							  TC_CONTEXT *ctx);

int AddCRLNumberExt(TC_ExtensionList *ext,
					const void *p_CRLnumber,
					int criticality,
					TC_CONTEXT *ctx);

int AddDeltaCRLIndicatorExt(TC_ExtensionList *ext,
							const void *p_value,
							int criticality,
							TC_CONTEXT *ctx);

int AddIssuingDistributionPoint (TC_ExtensionList *ext,
								 const void *p_distPoint,
								 int criticality,
								 TC_CONTEXT *);

int AddCRLReason (TC_ExtensionList *ext,
				  const void *p_CRLReason,
				  int criticality,
				  TC_CONTEXT *ctx);

int AddHoldInstructionCode (TC_ExtensionList *ext,
							const void *p_HoldCode,
							int criticality,
							TC_CONTEXT *ctx);

int AddInvalidityDate (TC_ExtensionList *ext,
					   const void *p_InvalidityDate,
					   int criticality,
					   TC_CONTEXT *ctx);

int GetAuthorityKeyIdentifier(void **returnAuthKeyId,
							  const PKIExtension *ext,
							  TC_CONTEXT *ctx);

int GetCRLNumberExt(void **p_CRLNumber,
					const PKIExtension *ext,
					TC_CONTEXT *ctx);

int GetDeltaCRLIndicatorExt(void **p_DeltaCRLIndicator,
							const PKIExtension *ext,
							TC_CONTEXT *ctx);

int GetIssuingDistributionPoint (void **p_DistPoint,
								 const PKIExtension *ext,
								 TC_CONTEXT *ctx);

int GetCRLReason (void **p_CRLReason,
				  const PKIExtension *ext,
				  TC_CONTEXT *ctx);

int GetHoldInstructionCode (void **p_HoldCode,
							const PKIExtension *ext,
							TC_CONTEXT *ctx);

int GetInvalidityDate (void **p_InvalidityDate,
					   const PKIExtension *ext,
					   TC_CONTEXT *ctx);

/* ----- utility functions ----- */

int tc_PKIGeneralNamesToTC (TC_GEN_NAMES_LIST_T **tc,/* OUT */
							PKIGeneralNames *asnNameList, /* IN */
							TC_CONTEXT *ctx);/* IN */

int
tc_GEN_NAMES_to_PKIGeneralNames (PKIGeneralNames *localNameList,
								 TC_GEN_NAMES_LIST_T *nameList,
								 TC_CONTEXT *ctx);

int tc_DistPointNameToTC (TC_DISTRIBUTION_POINT_NAME_T **dp,
						  PKIDistributionPointName *asn,
						  TC_CONTEXT *ctx);

int tc_DistPointNameToPKI (PKIDistributionPointName **asn,
						   TC_DISTRIBUTION_POINT_NAME_T *dp,
						   TC_CONTEXT *ctx);

/* vim:sw=4:ts=4:
 */

/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

#if defined(X500) && defined(STRONG)

#include <stdio.h>
#include "af.h"
#include "secude-stub.h"

static char	* IR            = "    IntendedRecipient:   ";
static char	* IAI           = "    IssuerAid:    ";
static char	* SIGN          = "    Signature of issuer: ";
static char	* SAI           = "    SignatureAid: ";
static char	* RANNUM        = "    Random number: ";
static char	* DERCODE       = "    DER code of ";
static char	* TIMEST        = "    Time-stamp:   ";

static char	* ETOKEN        = "Token is NULL pointer";
static char	* ETOKENTBS     = "TokenTBS is NULL pointer";
static char	* EADA          = "AddArgument is NULL pointer";
static char	* ECMA          = "CompareArgument is NULL pointer";
static char	* ECMR          = "CompareArgument is NULL pointer";
static char	* ELSA          = "ListArgument is NULL pointer";
static char	* ELSR          = "ListResult is NULL pointer";
static char	* ELSATBS       = "ListArgumentTBS is NULL pointer";
static char	* EMEA          = "ModifyEntryArgument is NULL pointer";
static char	* EMRA          = "ModifyRDNArgument is NULL pointer";
static char	* ERDA          = "ReadArgument is NULL pointer";
static char	* ERDR          = "ReadResult is NULL pointer";
static char	* ERDATBS       = "ReadArgumentTBS is NULL pointer";
static char	* ERMA          = "RemoveArgument is NULL pointer";
static char	* ESRA          = "SearchArgument is NULL pointer";
static char	* ESRR          = "SearchResult is NULL pointer";

static char	* ESECPARM      = "No Security Parameters provided";
static char	* SECPARMTIMEST = "    Signature is valid until:   ";
static char	* SECPARMFIR    = "    First Intended Recipient:   ";

static char	* ECA           = "No Common Arguments provided";
static char	* ECR           = "No Common Results provided";

static char	* CAREQ         = "    User who initiated the request:   ";
static char	* CRPER         = "    DSA which signed the result:   ";

static char	* ECERTPATH     = "No Certification Path provided";
static char	* ECPAIR        = "CertificatePair is NULL pointer";
static char	* ERANDOM       = "    Random number: No random number provided";



/***************************************************************
 *
 * Procedure strong_fprint_TokenTBS
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_TokenTBS(
	FILE		 *ff,
	TokenTBS	 *tok_tbs
)

#else

int strong_fprint_TokenTBS(
	ff,
	tok_tbs
)
FILE		 *ff;
TokenTBS	 *tok_tbs;

#endif

{
	Name      * printrepr;
	char	  * proc = "strong_fprint_TokenTBS";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);	
#endif

	if(! tok_tbs){
		fprintf(ff, "%s\n", ETOKENTBS);
		return(- 1);
	}

	fprintf(ff, "%s", SAI);
	aux_fprint_AlgId(ff, tok_tbs->signatureAI);

	printrepr = aux_DName2Name(tok_tbs->dname);
	fprintf(ff, "%s\n         %s\n", IR, printrepr);
	free(printrepr);

	fprintf(ff, "%s%s (%s)\n", TIMEST, aux_readable_UTCTime(tok_tbs->time), tok_tbs->time);

	aux_fprint_random (ff, tok_tbs->random);

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_Token
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_Token(
	FILE	 *ff,
	Token	 *tok
)

#else

int strong_fprint_Token(
	ff,
	tok
)
FILE	 *ff;
Token	 *tok;

#endif

{
	char	  * proc = "strong_fprint_Token";

	if(! tok){
		fprintf(ff, "%s\n", ETOKEN);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  ********************   T    O    K    E    N   ********************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of Token:\n");
		aux_fxdump(ff, tok->tbs_DERcode->octets, tok->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n");
	}

	strong_fprint_TokenTBS(ff, tok->tbs);

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, tok->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &tok->sig->signature);

	fprintf(ff, "\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_AddArgument
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_AddArgument(
	FILE		 *ff,
	AddArgument	 *addarg
)

#else

int strong_fprint_AddArgument(
	ff,
	addarg
)
FILE		 *ff;
AddArgument	 *addarg;

#endif

{
	char	  * proc = "strong_fprint_AddArgument";

	if(! addarg){
		fprintf(ff, "%s\n", EADA);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  ********************  A D D   A R G U M E N T  ********************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of AddArgument:\n\n");
		aux_fxdump(ff, addarg->tbs_DERcode->octets, addarg->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonArguments(ff, addarg->tbs->ada_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of AddArgument):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, addarg->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &addarg->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_CompareArgument
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_CompareArgument(
	FILE		 *ff,
	CompareArgument	 *comparearg
)

#else

int strong_fprint_CompareArgument(
	ff,
	comparearg
)
FILE		 *ff;
CompareArgument	 *comparearg;

#endif

{
	char	  * proc = "strong_fprint_CompareArgument";

	if(! comparearg){
		fprintf(ff, "%s\n", ECMA);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  ****************  C O M P A R E   A R G U M E N T  ****************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of CompareArgument:\n\n");
		aux_fxdump(ff, comparearg->tbs_DERcode->octets, comparearg->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonArguments(ff, comparearg->tbs->cma_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of CompareArgument):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, comparearg->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &comparearg->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_CompareResult
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_CompareResult(
	FILE		 *ff,
	CompareResult	 *compareres
)

#else

int strong_fprint_CompareResult(
	ff,
	compareres
)
FILE		 *ff;
CompareResult	 *compareres;

#endif

{
	char	  * proc = "strong_fprint_CompareResult";

	if(! compareres){
		fprintf(ff, "%s\n", ECMR);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  ******************  C O M P A R E   R E S U L T  ******************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of CompareResult:\n\n");
		aux_fxdump(ff, compareres->tbs_DERcode->octets, compareres->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonRes(ff, compareres->tbs->cmr_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of CompareResult):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, compareres->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &compareres->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_ListArgument
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_ListArgument(
	FILE		 *ff,
	ListArgument	 *listarg
)

#else

int strong_fprint_ListArgument(
	ff,
	listarg
)
FILE		 *ff;
ListArgument	 *listarg;

#endif

{
	char	  * proc = "strong_fprint_ListArgument";

	if(! listarg){
		fprintf(ff, "%s\n", ELSA);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  *******************  L I S T   A R G U M E N T  *******************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of ListArgument:\n\n");
		aux_fxdump(ff, listarg->tbs_DERcode->octets, listarg->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonArguments(ff, listarg->tbs->lsa_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of ListArgument):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, listarg->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &listarg->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_ListResult
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_ListResult(
	FILE		 *ff,
	ListResult	 *listres
)

#else

int strong_fprint_ListResult(
	ff,
	listres
)
FILE		 *ff;
ListResult	 *listres;

#endif

{
	char	  * proc = "strong_fprint_ListResult";

	if(! listres){
		fprintf(ff, "%s\n", ELSR);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  *********************  L I S T   R E S U L T  *********************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of ListResult:\n\n");
		aux_fxdump(ff, listres->tbs_DERcode->octets, listres->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	if(listres->tbs->lsr_type == 1){
		strong_fprint_CommonRes(ff, listres->tbs->lsrtbs_un.listinfo->lsr_common);
		fprintf(ff, "\n\n");
	}
	else if (listres->tbs->lsr_type == 2){
		strong_fprint_ListResult(ff, listres->tbs->lsrtbs_un.uncorrel_listinfo->element);
	}
	else return(- 1);

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of ListResult):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, listres->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &listres->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_ModifyEntryArgument
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_ModifyEntryArgument(
	FILE			 *ff,
	ModifyEntryArgument	 *modifyentryarg
)

#else

int strong_fprint_ModifyEntryArgument(
	ff,
	modifyentryarg
)
FILE			 *ff;
ModifyEntryArgument	 *modifyentryarg;

#endif

{
	char	  * proc = "strong_fprint_ModifyEntryArgument";

	if(! modifyentryarg){
		fprintf(ff, "%s\n", EMEA);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  ************  M O D I F Y E N T R Y   A R G U M E N T  ************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of ModifyEntryArgument:\n\n");
		aux_fxdump(ff, modifyentryarg->tbs_DERcode->octets, modifyentryarg->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonArguments(ff, modifyentryarg->tbs->mea_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of ModifyEntryArgument):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, modifyentryarg->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &modifyentryarg->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_ModifyRDNArgument
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_ModifyRDNArgument(
	FILE			 *ff,
	ModifyRDNArgument	 *modifyrdnarg
)

#else

int strong_fprint_ModifyRDNArgument(
	ff,
	modifyrdnarg
)
FILE			 *ff;
ModifyRDNArgument	 *modifyrdnarg;

#endif

{
	char	  * proc = "strong_fprint_ModifyRDNArgument";

	if(! modifyrdnarg){
		fprintf(ff, "%s\n", EMRA);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  **************  M O D I F Y R D N   A R G U M E N T  **************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of ModifyRDNArgument:\n\n");
		aux_fxdump(ff, modifyrdnarg->tbs_DERcode->octets, modifyrdnarg->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonArguments(ff, modifyrdnarg->tbs->mra_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of ModifyRDNArgument):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, modifyrdnarg->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &modifyrdnarg->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_ReadArgument
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_ReadArgument(
	FILE		 *ff,
	ReadArgument	 *readarg
)

#else

int strong_fprint_ReadArgument(
	ff,
	readarg
)
FILE		 *ff;
ReadArgument	 *readarg;

#endif

{
	char	  * proc = "strong_fprint_ReadArgument";

	if(! readarg){
		fprintf(ff, "%s\n", ERDA);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  *******************  R E A D   A R G U M E N T  *******************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of ReadArgument:\n\n");
		aux_fxdump(ff, readarg->tbs_DERcode->octets, readarg->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonArguments(ff, readarg->tbs->rda_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of ReadArgument):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, readarg->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &readarg->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_ReadResult
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_ReadResult(
	FILE		 *ff,
	ReadResult	 *readres
)

#else

int strong_fprint_ReadResult(
	ff,
	readres
)
FILE		 *ff;
ReadResult	 *readres;

#endif

{
	char	  * proc = "strong_fprint_ReadResult";

	if(! readres){
		fprintf(ff, "%s\n", ERDR);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  *********************  R E A D   R E S U L T  *********************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of ReadResult:\n\n");
		aux_fxdump(ff, readres->tbs_DERcode->octets, readres->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonRes(ff, readres->tbs->rdr_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of ReadResult):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, readres->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &readres->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_RemoveArgument
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_RemoveArgument(
	FILE		 *ff,
	RemoveArgument	 *removearg
)

#else

int strong_fprint_RemoveArgument(
	ff,
	removearg
)
FILE		 *ff;
RemoveArgument	 *removearg;

#endif

{
	char	  * proc = "strong_fprint_SearchArgument";

	if(! removearg){
		fprintf(ff, "%s\n", ERMA);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  *****************  R E M O V E   A R G U M E N T  *****************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of SearchArgument:\n\n");
		aux_fxdump(ff, removearg->tbs_DERcode->octets, removearg->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonArguments(ff, removearg->tbs->rma_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of RemoveArgument):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, removearg->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &removearg->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_SearchArgument
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_SearchArgument(
	FILE		 *ff,
	SearchArgument	 *searcharg
)

#else

int strong_fprint_SearchArgument(
	ff,
	searcharg
)
FILE		 *ff;
SearchArgument	 *searcharg;

#endif

{
	char	  * proc = "strong_fprint_SearchArgument";

	if(! searcharg){
		fprintf(ff, "%s\n", ESRA);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  *****************  S E A R C H   A R G U M E N T  *****************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of SearchArgument:\n\n");
		aux_fxdump(ff, searcharg->tbs_DERcode->octets, searcharg->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	strong_fprint_CommonArguments(ff, searcharg->tbs->sra_common);
	fprintf(ff, "\n\n");

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of SearchArgument):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, searcharg->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &searcharg->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_SearchResult
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_SearchResult(
	FILE		 *ff,
	SearchResult	 *searchres
)

#else

int strong_fprint_SearchResult(
	ff,
	searchres
)
FILE		 *ff;
SearchResult	 *searchres;

#endif

{
	char	  * proc = "strong_fprint_SearchResult";

	if(! searchres){
		fprintf(ff, "%s\n", ESRR);
		return(- 1);
	}

	fprintf(ff, "\n  *******************************************************************");
	fprintf(ff, "\n  ******************   S E A R C H   R E S U L T   ******************");
	fprintf(ff, "\n  *******************************************************************\n\n\n");

	if(sec_verbose == TRUE){
		fprintf(ff, "%s", DERCODE);
		fprintf(ff, "ToBeSigned of SearchResult:\n\n");
		aux_fxdump(ff, searchres->tbs_DERcode->octets, searchres->tbs_DERcode->noctets, 0);
		fprintf(ff, "\n\n");
	}

	if(searchres->tbs->srr_correlated == FALSE) {
		/* uncorrelated stuff needs to be added here */
	}
	else {
		strong_fprint_CommonRes(ff, searchres->tbs->srrtbs_un.searchinfo->srr_common);
		fprintf(ff, "\n\n");
	}

	fprintf(ff, "\n\n  **************************************************************\n\n");
	fprintf(ff, "\n    S I G N A T U R E  (applied to ToBeSigned of SearchResult):\n\n");

	fprintf(ff, "%s", IAI);
	aux_fprint_AlgId(ff, searchres->sig->signAI);

	fprintf(ff, "%s\n", SIGN);
	aux_fprint_BitString(ff, &searchres->sig->signature);

	fprintf(ff, "\n\n  **************************************************************\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_CommonArguments
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_CommonArguments(
	FILE		 *ff,
	CommonArguments	 *ca
)

#else

int strong_fprint_CommonArguments(
	ff,
	ca
)
FILE		 *ff;
CommonArguments	 *ca;

#endif

{
	Name      * printrepr;
	char	  * proc = "strong_fprint_CommonArguments";


	if(! ca ){
		fprintf(ff, "%s\n", ECA);
		return(- 1);
	}

	if(ca->requestor){  /* requestor is OPTIONAL (X.511) */
		printrepr = aux_DName2Name(ca->requestor);
		fprintf(ff, "\n%s\n         %s\n", CAREQ, printrepr);
		free(printrepr);
	}
	else
		fprintf(ff, "\n%sNo distinguished name provided\n", CAREQ);

	strong_fprint_SecurityParameters(ff, ca->sec_parm);
	
	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_CommonRes
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_CommonRes(
	FILE		 *ff,
	CommonRes	 *cr
)

#else

int strong_fprint_CommonRes(
	ff,
	cr
)
FILE		 *ff;
CommonRes	 *cr;

#endif

{
	Name      * printrepr;
	char	  * proc = "strong_fprint_CommonRes";


	if(! cr){
		fprintf(ff, "%s\n", ECR);
		return(- 1);
	}

	if(cr->performer){  /* performer is OPTIONAL (X.511) */
		printrepr = aux_DName2Name(cr->performer);
		fprintf(ff, "\n%s\n         %s\n", CRPER, printrepr);
		free(printrepr);
	}
	else
		fprintf(ff, "\n%sNo distinguished name provided\n", CRPER);

	strong_fprint_SecurityParameters(ff, cr->sec_parm);
	
	return(0);
}


/***************************************************************
 *
 * Procedure strong_fprint_SecurityParameters
 *
 ***************************************************************/
#ifdef __STDC__

int strong_fprint_SecurityParameters(
	FILE			 *ff,
	SecurityParameters	 *sec_parm
)

#else

int strong_fprint_SecurityParameters(
	ff,
	sec_parm
)
FILE			 *ff;
SecurityParameters	 *sec_parm;

#endif

{
	Name      * printrepr;
	char	  * proc = "strong_fprint_SecurityParameters";


	if(! sec_parm){
		fprintf(ff, "%s\n", ESECPARM);
		return(- 1);
	}

	if(sec_parm->name){		/* name is OPTIONAL (X.511) */
		printrepr = aux_DName2Name(sec_parm->name);
		fprintf(ff, "%s\n         %s\n", SECPARMFIR, printrepr);
		free(printrepr);
	}
	else
		fprintf(ff, "%sNo distinguished name provided\n", SECPARMFIR);

	if(sec_parm->time)   /* time is OPTIONAL (X.511) */
		fprintf(ff, "%s%s (%s)\n", SECPARMTIMEST, aux_readable_UTCTime(sec_parm->time), sec_parm->time);
	else
		fprintf(ff, "%sno date of expiry provided\n", SECPARMTIMEST);

	aux_fprint_random (ff, sec_parm->random);   /* random is OPTIONAL (X.511) */

	return(0);
}


/***************************************************************
 *
 * Procedure aux_fprint_CertificationPath
 *
 ***************************************************************/
#ifdef __STDC__

int aux_fprint_CertificationPath(
	FILE			 *ff,
	CertificationPath	 *certpath
)

#else

int aux_fprint_CertificationPath(
	ff,
	certpath
)
FILE			 *ff;
CertificationPath	 *certpath;

#endif

{
	char	  * proc = "aux_fprint_CertificationPath";


	if(! certpath){
		fprintf(ff, "%s\n", ECERTPATH);
		return(- 1);
	}

	fprintf (ff, "\n Certification Path\n");

	fprintf (ff, "\n - UserCertificate:\n\n");
	aux_fprint_Certificate (ff, certpath->userCertificate);
	fprintf (ff, "\n - theCACertificates:\n\n");
	aux_fprint_CertificatePairs (ff, certpath->theCACertificates);

	return(0);
}


/***************************************************************
 *
 * Procedure aux_fprint_CertificatePairs
 *
 ***************************************************************/
#ifdef __STDC__

int aux_fprint_CertificatePairs(
	FILE			 *ff,
	CertificatePairs	 *certpairs
)

#else

int aux_fprint_CertificatePairs(
	ff,
	certpairs
)
FILE			 *ff;
CertificatePairs	 *certpairs;

#endif

{
	SEQUENCE_OF_CertificatePair * seq;
	CertificatePair             * cpair;
	char	  	            * proc = "aux_fprint_CertificatePairs";


	while (certpairs) {
		fprintf(ff, "\n --- next level in path: ---\n");
		seq = certpairs->liste;
		while (seq) {
			cpair = seq->element;
			aux_fprint_CertificatePair(ff, cpair); 
			if(seq = seq->next){
				fprintf(ff, "\n --- next CertificatePair on same level: ---\n");
			}
		}
		certpairs = certpairs->superior;
	}

	return(0);
}


/***************************************************************
 *
 * Procedure aux_fprint_CertificatePair
 *
 ***************************************************************/
#ifdef __STDC__

int aux_fprint_CertificatePair(
	FILE		 *ff,
	CertificatePair	 *cpair
)

#else

int aux_fprint_CertificatePair(
	ff,
	cpair
)
FILE		 *ff;
CertificatePair	 *cpair;

#endif

{
	char	  * proc = "aux_fprint_CertificatePair";


	if(! cpair){
		fprintf(ff, "%s\n", ECPAIR);
		return(- 1);
	}

	fprintf(ff, "\n --------------- CertificatePair ---------------\n\n\n");
	fprintf(ff, "  --- ForwardCertificate ---\n\n");
	aux_fprint_Certificate(ff, cpair->forward);
	fprintf(ff, "\n\n  --- ReverseCertificate ---\n\n");
	aux_fprint_Certificate(ff, cpair->reverse);
	fprintf(ff, "\n -----------------------------------------------\n\n\n");

	return(0);
}


/***************************************************************
 *
 * Procedure aux_fprint_random
 *
 ***************************************************************/
#ifdef __STDC__

int aux_fprint_random(
	FILE		 *ff,
	BitString	 *random
)

#else

int aux_fprint_random(
	ff,
	random
)
FILE		 *ff;
BitString	 *random;

#endif

{
	char	  * proc = "aux_fprint_random";


	if(! random){
		fprintf(ff, "%s\n", ERANDOM);
		return(- 1);
	}

	fprintf(ff, "%s\n", RANNUM);
	aux_fprint_BitString(ff, random);

	return(0);
}




#else

/* dummy */
/***************************************************************
 *
 * Procedure strong_print_dummy
 *
 ***************************************************************/

int strong_print_dummy()

{
	return(0);
}

#endif

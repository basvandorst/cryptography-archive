STRONG    

DEFINITIONS ::=

%{	/* surrounding global definitions */
#include        "secude-stub.h"

	static char 	    * aux_int2strb_alloc();
%}


PREFIXES build parse none

BEGIN


-- EXPORTS
--              TokenTBS, Token
--
-- IMPORTS	
--		Name
--			FROM InformationFramework(IF)
--
--              UTCTime, BITSTRING
--			FROM UNIV
--
--              AlgorithmIdentifier
--                      FROM SecurityFramework(SEC)
--


ENCODER build


-- Temp labels to aid Pepsy parameter passing

TSETOFSECExtension [[P CommonArguments *]]
	::=
	SETOFSECExtension [[p parm->ext]]


TInteger [[P CommonArguments *]]
	::=
        INTEGER [[i parm->aliasedRDNs]]


TOperationProgress [[P CommonArguments *]]
	::=
	OperationProgress [[p parm->progress]]


TRequestor [[P CommonArguments *]]
	::=
        SECIF.Name [[p parm->requestor]]


TSecParmArgument [[P CommonArguments *]]
	::=
	SecurityParameters [[p parm->sec_parm]]


TServiceControls [[P CommonArguments *]]
	::=
	ServiceControls [[p parm->svc]]


TBoolean [[P CommonRes *]]
	::=
	BOOLEAN [[b parm->aliasDereferenced]]


TPerformer [[P CommonRes *]]
	::=
	SECIF.Name [[p parm->performer]]


TSecParmResult [[P CommonRes *]]
	::=
	SecurityParameters [[p parm->sec_parm]]


TAttribute [[P SET_OF_Attr *]]
	::= 
	SECIF.Attribute [[p parm->element]]


-- *************************************************************


TokenTBS [[P TokenTBS *]] 
	::=
        SEQUENCE 
	{
	signature
                [0] SEC.AlgorithmIdentifier [[p signatureAI]],

        name
                [1] SECIF.Name [[p dname]],

	time
                [2] UTCTime [[s time]],

	random
                [3] SEC.BitString [[p random]]
        }


Token [[P Token *]]
    	::=
        %D{
        if (((*parm) = (Token *) calloc (1, sizeof (Token))) == ((Token *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
		advise (NULLCP, "out of memory");
		return NOTOK;
	}
        %} 
	SEQUENCE
	-- SIGNED 
	{
		TokenTBS [[p tbs]]
		%D{
		(*parm)->tbs_DERcode = e_TokenTBS((*parm)->tbs);
		%},

                SEC.TAlgorithmIdentifier [[p sig]],

		-- ENCRYPTED OCTET STRING
		SEC.TBitString [[p sig]]
        }


AddArgumentTBS [[P AddArgumentTBS *]] 
	::=
        SET
	{
        object
                [0] SECIF.Name [[p ada_object]],

	entry
		[1] SECIF.SETOFAttribute [[p ada_entry]],

	extensions
		[25] TSETOFSECExtension [[p ada_common]]
		OPTIONAL <E<parm->ada_common->ext != (SET_OF_SECExtension *) 0>><D<0>>,

	aliasedRDNs
                [26] TInteger [[p ada_common]]
		OPTIONAL <E<parm->ada_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED>><D<0>>,

	progress
	        [27] TOperationProgress [[p ada_common]]
		OPTIONAL <E<parm->ada_common->progress != (OperationProgress *) 0>><D<0>>,

	requestor
                [28] TRequestor [[p ada_common]]
		OPTIONAL <E<parm->ada_common->requestor != NULLDNAME>><D<0>>,

	secparm
		[29] TSecParmArgument [[p ada_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->ada_common->sec_parm != (SecurityParameters *) 0>><D<0>>,

	servcontr
		[30] TServiceControls [[p ada_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->ada_common->svc != (ServiceControls *) 0>><D<0>>
	}


AddArgument [[P AddArgument *]]
    	::=
        %D{
        if (((*parm) = (AddArgument *) calloc (1, sizeof (AddArgument))) == ((AddArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %} 
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>> 
	{
		AddArgumentTBS [[p parm->tbs]],

		SEQUENCE [[ T AddArgument * $ * ]]
		-- SIGNED
		{ 
			AddArgumentTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_AddArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


CompareArgumentTBS [[P CompareArgumentTBS *]] 
	::=
        SET 
	{
	object
                [0] SECIF.Name [[p cma_object]],

	purported
		[1] SECIF.AttributeValueAssertion [[p cma_purported]],

	extensions
		[25] TSETOFSECExtension [[p cma_common]]
		OPTIONAL <E<parm->cma_common->ext != (SET_OF_SECExtension *) 0>><D<0>>,

	aliasedRDNs
                [26] TInteger [[p cma_common]]
		OPTIONAL <E<parm->cma_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED>><D<0>>,

	progress
	        [27] TOperationProgress [[p cma_common]]
		OPTIONAL <E<parm->cma_common->progress != (OperationProgress *) 0>><D<0>>,

	requestor
                [28] TRequestor [[p cma_common]]
		OPTIONAL <E<parm->cma_common->requestor != NULLDNAME>><D<0>>,

	secparm
		[29] TSecParmArgument [[p cma_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->cma_common->sec_parm != (SecurityParameters *) 0>><D<0>>,

	servcontr
		[30] TServiceControls [[p cma_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->cma_common->svc != (ServiceControls *) 0>><D<0>>
	}


CompareArgument [[P CompareArgument *]]
    	::=
        %D{
        if (((*parm) = (CompareArgument *) calloc (1, sizeof (CompareArgument))) == ((CompareArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		CompareArgumentTBS [[p tbs]],

		SEQUENCE [[ T CompareArgument * $ * ]]
		-- SIGNED
		{
			CompareArgumentTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_CompareArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


CompareResultTBS [[P CompareResultTBS *]] 
	::=
        SET 
	{
	        SECIF.Name [[p cmr_object]]
		OPTIONAL,

	matched
		[0] BOOLEAN [[b cmr_matched]],

	fromEntry
		[1] BOOLEAN [[b cmr_fromEntry]]
		DEFAULT TRUE,

	aliasDereferenced
		[28] TBoolean [[b cmr_common]]
		-- DEFAULT FALSE
		OPTIONAL <E<parm->cmr_common->aliasDereferenced != FALSE>><D<0>>,

	performer
                [29] TPerformer [[p cmr_common]]
		OPTIONAL <E<parm->cmr_common->performer != NULLDNAME>><D<0>>,

	secparm
		[30] TSecParmResult [[p cmr_common]]
		OPTIONAL <E<parm->cmr_common->sec_parm != (SecurityParameters *) 0>><D<0>>
	}


CompareResult [[P CompareResult *]]
    	::=
        %D{
        if (((*parm) = (CompareResult *) calloc (1, sizeof (CompareResult))) == ((CompareResult *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %} 
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		CompareResultTBS [[p tbs]],

		SEQUENCE [[ T CompareResult * $ * ]]
		-- SIGNED
		{
			CompareResultTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_CompareResultTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


ListArgumentTBS [[P ListArgumentTBS *]] 
	::=
        SET 
	{
	object
                [0] SECIF.Name [[p object]],

	extensions
		[25] TSETOFSECExtension [[p lsa_common]]
		OPTIONAL <E<parm->lsa_common->ext != (SET_OF_SECExtension *) 0>><D<0>>,

	aliasedRDNs
                [26] TInteger [[p lsa_common]]
		OPTIONAL <E<parm->lsa_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED>><D<0>>,

	progress
	        [27] TOperationProgress [[p lsa_common]]
		OPTIONAL <E<parm->lsa_common->progress != (OperationProgress *) 0>><D<0>>,

	requestor
                [28] TRequestor [[p lsa_common]]
		OPTIONAL <E<parm->lsa_common->requestor != NULLDNAME>><D<0>>,

	secparm
		[29] TSecParmArgument [[p lsa_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->lsa_common->sec_parm != (SecurityParameters *) 0>><D<0>>,

	servcontr
		[30] TServiceControls [[p lsa_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->lsa_common->svc != (ServiceControls *) 0>><D<0>>
	}


ListArgument [[P ListArgument *]]
    	::=
        %D{
        if (((*parm) = (ListArgument *) calloc (1, sizeof (ListArgument))) == ((ListArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		ListArgumentTBS [[p tbs]],

		SEQUENCE [[ T ListArgument * $ * ]]
		-- SIGNED
		{ 
			ListArgumentTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_ListArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


ListResultTBS [[P ListResultTBS *]] 
	::=
	CHOICE <E<parm->lsr_type>><D<0>>
	{
	listInfo
		ListInfo
		[[p parm->lsrtbs_un.listinfo]],

	uncorrelatedListInfo
		[0] SETOFListResult
		[[p parm->lsrtbs_un.uncorrel_listinfo]]
	}


ListResult [[P ListResult *]]
    	::=
        %D{
        if (((*parm) = (ListResult *) calloc (1, sizeof (ListResult))) == ((ListResult *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %} 
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		ListResultTBS [[p tbs]],

		SEQUENCE [[ T ListResult * $ * ]]
		-- SIGNED
		{
			ListResultTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_ListResultTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


SETOFListResult [[P SET_OF_ListResult *]]
	::=
        SET OF [[T SET_OF_ListResult * $ *]] <<next>>
        	ListResult [[p element]]


ModifyEntryArgumentTBS [[P ModifyEntryArgumentTBS *]] 
	::=
        SET 
	{
	object
                [0] SECIF.Name [[p mea_object]],

	changes
		[1] EntryModificationSequence [[p mea_changes]],

	extensions
		[25] TSETOFSECExtension [[p mea_common]]
		OPTIONAL <E<parm->mea_common->ext != (SET_OF_SECExtension *) 0>><D<0>>,

	aliasedRDNs
                [26] TInteger [[p mea_common]]
		OPTIONAL <E<parm->mea_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED>><D<0>>,

	progress
	        [27] TOperationProgress [[p mea_common]]
		OPTIONAL <E<parm->mea_common->progress != (OperationProgress *) 0>><D<0>>,

	requestor
                [28] TRequestor [[p mea_common]]
		OPTIONAL <E<parm->mea_common->requestor != NULLDNAME>><D<0>>,

	secparm
		[29] TSecParmArgument [[p mea_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->mea_common->sec_parm != (SecurityParameters *) 0>><D<0>>,

	servcontr
		[30] TServiceControls [[p mea_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->mea_common->svc != (ServiceControls *) 0>><D<0>>
	}


ModifyEntryArgument [[P ModifyEntryArgument *]]
    	::=
        %D{
        if (((*parm) = (ModifyEntryArgument *) calloc (1, sizeof (ModifyEntryArgument))) == ((ModifyEntryArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %} 
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		ModifyEntryArgumentTBS [[p tbs]],

		SEQUENCE [[ T ModifyEntryArgument * $ * ]]
		-- SIGNED 
		{
			ModifyEntryArgumentTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_ModifyEntryArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


ModifyRDNArgumentTBS [[P ModifyRDNArgumentTBS *]] 
	::=
        SET 
	{
	object
                [0] SECIF.Name [[p mra_object]],

	newRDN
		[1] SECIF.RelativeDistinguishedName [[p mra_newrdn]],

	deleteOldRDN
		[2] BOOLEAN [[b deleterdn]]
		DEFAULT FALSE,

	extensions
		[25] TSETOFSECExtension [[p mra_common]]
		OPTIONAL <E<parm->mra_common->ext != (SET_OF_SECExtension *) 0>><D<0>>,

	aliasedRDNs
                [26] TInteger [[p mra_common]]
		OPTIONAL <E<parm->mra_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED>><D<0>>,

	progress
	        [27] TOperationProgress [[p mra_common]]
		OPTIONAL <E<parm->mra_common->progress != (OperationProgress *) 0>><D<0>>,

	requestor
                [28] TRequestor [[p mra_common]]
		OPTIONAL <E<parm->mra_common->requestor != NULLDNAME>><D<0>>,

	secparm
		[29] TSecParmArgument [[p mra_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->mra_common->sec_parm != (SecurityParameters *) 0>><D<0>>,

	servcontr
		[30] TServiceControls [[p mra_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->mra_common->svc != (ServiceControls *) 0>><D<0>>
	}


ModifyRDNArgument [[P ModifyRDNArgument *]]
    	::=
        %D{
        if (((*parm) = (ModifyRDNArgument *) calloc (1, sizeof (ModifyRDNArgument))) == ((ModifyRDNArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %} 
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		ModifyRDNArgumentTBS [[p tbs]],

		SEQUENCE [[ T ModifyRDNArgument * $ * ]]
		-- SIGNED 
		{ 
			ModifyRDNArgumentTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_ModifyRDNArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


ReadArgumentTBS [[P ReadArgumentTBS *]] 
	::=
        SET 
	{
	object
                [0] SECIF.Name [[p object]],

	selection
		[1] EntryInfoSEL [[p rda_eis]]
		-- DEFAULT {},
		OPTIONAL,

	extensions
		[25] TSETOFSECExtension [[p rda_common]]
		OPTIONAL <E<parm->rda_common->ext != (SET_OF_SECExtension *) 0>><D<0>>,

	aliasedRDNs
                [26] TInteger [[p rda_common]]
		OPTIONAL <E<parm->rda_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED>><D<0>>,

	progress
	        [27] TOperationProgress [[p rda_common]]
		OPTIONAL <E<parm->rda_common->progress != (OperationProgress *) 0>><D<0>>,

	requestor
                [28] TRequestor [[p rda_common]]
		OPTIONAL <E<parm->rda_common->requestor != NULLDNAME>><D<0>>,

	secparm
		[29] TSecParmArgument [[p rda_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->rda_common->sec_parm != (SecurityParameters *) 0>><D<0>>,

	servcontr
		[30] TServiceControls [[p rda_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->rda_common->svc != (ServiceControls *) 0>><D<0>>
	}


ReadArgument [[P ReadArgument *]]
    	::= 
        %D{
        if (((*parm) = (ReadArgument *) calloc (1, sizeof (ReadArgument))) == ((ReadArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		ReadArgumentTBS [[p tbs]],

		SEQUENCE [[ T ReadArgument * $ * ]]
		-- SIGNED
		{ 
			ReadArgumentTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_ReadArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


ReadResultTBS [[P ReadResultTBS *]] 
	::=
        SET 
	{
	entry
		[0] EntryINFO [[p rdr_entry]],

	aliasDereferenced
		[28] TBoolean [[b rdr_common]]
		-- DEFAULT FALSE
		OPTIONAL <E<parm->rdr_common->aliasDereferenced != FALSE>><D<0>>,

	performer
                [29] TPerformer [[p rdr_common]]
		OPTIONAL <E<parm->rdr_common->performer != NULLDNAME>><D<0>>,

	secparm
		[30] TSecParmResult [[p rdr_common]]
		OPTIONAL <E<parm->rdr_common->sec_parm != (SecurityParameters *) 0>><D<0>>
	}


ReadResult [[P ReadResult *]]
    	::=
        %D{
        if (((*parm) = (ReadResult *) calloc (1, sizeof (ReadResult))) == ((ReadResult *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		ReadResultTBS [[p tbs]],

		SEQUENCE [[ T ReadResult * $ * ]]
		-- SIGNED
		{
			ReadResultTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_ReadResultTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


RemoveArgumentTBS [[P RemoveArgumentTBS *]] 
	::=
        SET 
	{
	object
                [0] SECIF.Name [[p rma_object]],

	extensions
		[25] TSETOFSECExtension [[p rma_common]]
		OPTIONAL <E<parm->rma_common->ext != (SET_OF_SECExtension *) 0>><D<0>>,

	aliasedRDNs
                [26] TInteger [[p rma_common]]
		OPTIONAL <E<parm->rma_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED>><D<0>>,

	progress
	        [27] TOperationProgress [[p rma_common]]
		OPTIONAL <E<parm->rma_common->progress != (OperationProgress *) 0>><D<0>>,

	requestor
                [28] TRequestor [[p rma_common]]
		OPTIONAL <E<parm->rma_common->requestor != NULLDNAME>><D<0>>,

	secparm
		[29] TSecParmArgument [[p rma_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->rma_common->sec_parm != (SecurityParameters *) 0>><D<0>>,

	servcontr
		[30] TServiceControls [[p rma_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->rma_common->svc != (ServiceControls *) 0>><D<0>>
	}


RemoveArgument [[P RemoveArgument *]]
    	::= 
        %D{
        if (((*parm) = (RemoveArgument *) calloc (1, sizeof (RemoveArgument))) == ((RemoveArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		RemoveArgumentTBS [[p tbs]],

		SEQUENCE [[ T RemoveArgument * $ * ]]
		-- SIGNED 
		{
			RemoveArgumentTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_RemoveArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


SearchArgumentTBS [[P SearchArgumentTBS *]] 
	::=
        SET 
	{
	baseObject
		[0] SECIF.Name [[p baseobject]],

	subset
		[1] INTEGER [[i subset]]
		{
		baseObject(0) ,
		oneLevel(1) ,
		wholeSubtree(2)
		}
		DEFAULT baseObject,

	filter
		[2] Filter [[p filter]]
		OPTIONAL,

	searchAliases
		[3] BOOLEAN [[b searchaliases]]
		DEFAULT TRUE,

	selection
		[4] EntryInfoSEL [[p sra_eis]]
		-- DEFAULT {},
		OPTIONAL,

	extensions
		[25] TSETOFSECExtension [[p sra_common]]
		OPTIONAL <E<parm->sra_common->ext != (SET_OF_SECExtension *) 0>><D<0>>,

	aliasedRDNs
                [26] TInteger [[p sra_common]]
		OPTIONAL <E<parm->sra_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED>><D<0>>,

	progress
	        [27] TOperationProgress [[p sra_common]]
		OPTIONAL <E<parm->sra_common->progress != (OperationProgress *) 0>><D<0>>,

	requestor
                [28] TRequestor [[p sra_common]]
		OPTIONAL <E<parm->sra_common->requestor != NULLDNAME>><D<0>>,

	secparm
		[29] TSecParmArgument [[p sra_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->sra_common->sec_parm != (SecurityParameters *) 0>><D<0>>,

	servcontr
		[30] TServiceControls [[p sra_common]]
		-- DEFAULT {}
		OPTIONAL <E<parm->sra_common->svc != (ServiceControls *) 0>><D<0>>
	}


SearchArgument [[P SearchArgument *]]
    	::= 
        %D{
        if (((*parm) = (SearchArgument *) calloc (1, sizeof (SearchArgument))) == ((SearchArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		SearchArgumentTBS [[p tbs]],

		SEQUENCE [[ T SearchArgument * $ * ]]
		-- SIGNED 
		{
			SearchArgumentTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_SearchArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


SearchResultTBS [[P SearchResultTBS *]] 
	::=
	CHOICE <E<((parm->srr_correlated == FALSE) ? 2 : 1)>><D<0>>
	{
	searchInfo
		SearchInfo [[p parm->srrtbs_un.searchinfo]],

	uncorrelatedSearchInfo
		[0] SETOFSearchResult [[p parm->srrtbs_un.uncorrel_searchinfo]]
	}


SearchResult [[P SearchResult *]]
    	::= 
        %D{
        if (((*parm) = (SearchResult *) calloc (1, sizeof (SearchResult))) == ((SearchResult *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	CHOICE <D<0>>
	<E<parm->sig == (Signature *) 0 ? 1:2>>
	{
		SearchResultTBS [[p tbs]],

		SEQUENCE [[ T SearchResult * $ * ]]
		-- SIGNED
		{ 
			SearchResultTBS [[p tbs]]
			%D{
			(*parm)->tbs_DERcode = e_SearchResultTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			%},

			SEC.TAlgorithmIdentifier [[p sig]],

			-- ENCRYPTED OCTET STRING
			SEC.TBitString [[p sig]]
        	}
	}


SETOFSearchResult [[P SET_OF_SearchResult *]]
	::=
        SET OF [[T SET_OF_SearchResult * $ *]] <<next>>
        	SearchResult [[p element]]


EntryModification [[P EntryModification *]] 
	::=
	CHOICE <E<parm->em_type>><D<0>>
	{
	addAttribute
		[0] SECIF.Attribute 
		[[p parm->em_un.em_un_attr]],

	removeAttribute
		[1] OBJECT IDENTIFIER
		[[O parm->em_un.em_un_attrtype]],

	addValues
		[2] SECIF.Attribute 
		[[p parm->em_un.em_un_attr]],

	removeValues
		[3] SECIF.Attribute 
		[[p parm->em_un.em_un_attr]]
	}


EntryModificationSequence [[P SEQUENCE_OF_EntryModification *]] 
	::=
	SEQUENCE OF [[T SEQUENCE_OF_EntryModification * $ *]] <<next>>
		EntryModification [[p element]]


ServiceControls [[P ServiceControls *]] 
	::=
	%E{
	if (parm->svc_options != 0) {
		parm->svc_len = 5;
		parm->svc_tmp = aux_int2strb_alloc (parm->svc_options,parm->svc_len);
	}
	%}
	SET 
	{
	options
		[0] BITSTRING
		[[x parm->svc_tmp $ parm->svc_len]]
		{
		preferChaining(0) ,
		chainingProhibited(1) ,
		localScope(2) ,
		dontUseCopy(3) ,
		dontDereferenceAliases(4)
		}
		%E{
		if (parm->svc_tmp){
			free (parm->svc_tmp);
			parm->svc_tmp = CNULL;
		}
		%}
		OPTIONAL <E<parm->svc_options != 0>><D<0>>,

	priority
                [1] INTEGER
                [[i parm->svc_prio]]
		{
		low(0) ,
		medium(1) ,
		high(2)
		}
		DEFAULT medium,

	timeLimit
                [2] INTEGER
                [[i parm->svc_timelimit]]
		OPTIONAL <E<parm->svc_timelimit != SVC_NOTIMELIMIT>>
			 <D<0>>,

	sizeLimit
                [3] INTEGER
                [[i parm->svc_sizelimit]]
		OPTIONAL <E<parm->svc_sizelimit != SVC_NOSIZELIMIT>>
			 <D<0>>,

	scopeOfReferral
                [4] INTEGER
                [[i parm->svc_scopeofreferral]]
		{
		dmd(0) ,
		country(1)
		}
		OPTIONAL <E<parm->svc_scopeofreferral != SVC_REFSCOPE_NONE>>
			 <D<0>>
        }


SecurityParameters [[P SecurityParameters *]] 
	::=
	SET 
	{
	certificationPath
		[0] SECAF.CertificationPath [[p certPath]]
		OPTIONAL,

	name
		[1] SECIF.Name [[p name]]
		OPTIONAL,

	time
                [2] UTCTime [[s time]]
		OPTIONAL,

	random
	        [3] SEC.BitString [[p random]]
		OPTIONAL,

	target
                [4] INTEGER [[i target]]
		{
		none(0) ,
		signed(1)
		}
		DEFAULT 0
        }


OperationProgress [[P OperationProgress *]] 
	::=
	%E{
	if (parm->opResolutionPhase < 0)
		parm->opResolutionPhase = 1;
	else if (parm->opResolutionPhase > 3)
		parm->opResolutionPhase = 3;
	%}
        SET 
	{
	opResolutionPhase
                [0] ENUMERATED
                [[i parm->opResolutionPhase]]
		{
		notStarted(1),
		proceeding(2),
		completed(3)
		},

	opNextRDNToBeResolved
                [1] INTEGER
                [[i parm->opNextRDNToBeResolved]]
	        OPTIONAL <E<parm->opNextRDNToBeResolved != -1>><D<0>>
        }


SETOFSECExtension [[P SET_OF_SECExtension *]]
	::=
        SET OF [[T SET_OF_SECExtension * $ *]] <<next>>
        	SECExtension [[p element]]


SECExtension [[P SECExtension *]] 
	::= 
	SET 
	{
	identifier
                [0] INTEGER [[i ext_id]],

	critical
                [1] BOOLEAN [[b ext_critical]]
		DEFAULT FALSE,

	item
		[2] ANY [[a parm->ext_item]]
        }


SETOFAttrAttrTypeCHOICE [[P SET_OF_AttrAttrTypeCHOICE *]]
	::=
        SET OF [[T SET_OF_AttrAttrTypeCHOICE * $ *]] <<next>>
        	AttrAttrTypeCHOICE [[p element]]


AttrAttrTypeCHOICE [[P AttrAttrTypeCHOICE *]] 
	::=
	CHOICE <E<parm->offset>><D<0>>
	{
		OBJECT IDENTIFIER
		[[O choice_un.choice_un_attrtype]],

		TAttribute
		-- only first element within SET is needed, see secude-stub.h
		[[p choice_un.choice_un_attr]]
	}


EntryInfoSEL [[P EntryInfoSEL *]]
	::=
	SET 
	{
	attributeTypes
		CHOICE [[ T EntryInfoSEL * $ * ]] <E<parm->eis_allattributes ? 1 : 2>><D<0>>
		{
		allAttributes
			[0] NULL,

		select
			[1] SECIF.SETOFAttributeType 
			[[p parm->eis_select]]
		}
		-- DEFAULT allAttributes NULL,
		OPTIONAL <E<parm->eis_allattributes != TRUE>><D<0>>,

	infoTypes
		[2] INTEGER 
		[[i parm->eis_infotypes]]
		{
		attributeTypesOnly(0) ,
		attributeTypesAndValues(1)
		}
		DEFAULT attributeTypesAndValues
	}


SETOFEntryINFO [[P SET_OF_EntryINFO *]]
	::=
        SET OF [[T SET_OF_EntryINFO * $ *]] <<next>>
        	EntryINFO [[p element]]


EntryINFO [[P EntryINFO *]]
	::=
	SEQUENCE 
	{
	name
		SECIF.Name
		[[p ent_dn]],

	fromEntry
		BOOLEAN 
		[[b ent_fromentry]]
		DEFAULT TRUE,

		SETOFAttrAttrTypeCHOICE
		[[p ent_attr]]
		OPTIONAL
	}


StringsCHOICE [[P StringsCHOICE *]]
	::=
	CHOICE <E<parm->strings_type>><D<0>>
	{
	initial
		[0] ANY
		[[a strings_un.initial]],
	any
		[1] ANY
		[[a strings_un.any]],
	final
		[2] ANY
		[[a strings_un.final]]
	}


SEQUENCEOFStringsCHOICE [[P SEQUENCE_OF_StringsCHOICE *]] 
	::=
	SEQUENCE OF [[T SEQUENCE_OF_StringsCHOICE * $ *]] <<next>>
		StringsCHOICE [[p element]]


FilterSubstrings [[P SFilterSubstrings *]] 
	::=
	SEQUENCE 
	{
	type
		  OBJECT IDENTIFIER
		  [[O type]],

	strings
		  SEQUENCEOFStringsCHOICE 
		  [[p seq]]
	}


FilterItem [[P SFilterItem *]]
	::=
	CHOICE <E<parm->fi_type>><D<0>>
	{
	equality
		[0] SECIF.AttributeValueAssertion 
		[[p parm->fi_un.fi_un_ava]],
	substrings 
		[1] FilterSubstrings
		[[p parm->fi_un.fi_un_substrings]], 
	greaterOrEqual
		[2] SECIF.AttributeValueAssertion 
		[[p parm->fi_un.fi_un_ava]],
	lessOrEqual
		[3] SECIF.AttributeValueAssertion 
		[[p parm->fi_un.fi_un_ava]],
	present
		[4] OBJECT IDENTIFIER 
		[[O parm->fi_un.fi_un_type]],
	approximateMatch
		[5] SECIF.AttributeValueAssertion 
		[[p parm->fi_un.fi_un_ava]]
	}


SETOFFilter [[P SET_OF_SFilter *]]
	::=
        SET OF [[T SET_OF_SFilter * $ *]] <<next>>
        	Filter [[p element]]


Filter [[P SFilter *]]
	::=
	CHOICE <E<parm->flt_type>><D<0>>
	{
	item
		[0] FilterItem 
		[[p parm->flt_un.flt_un_item]],
	and
		[1] SETOFFilter
		[[p parm->flt_un.flt_un_filterset]],
	or
		[2] SETOFFilter
	        [[p parm->flt_un.flt_un_filterset]],
	not
		[3] Filter 
		[[p parm->flt_un.flt_un_filter]]
	}


SETOFSubordEntry [[P SET_OF_SubordEntry *]]
	::=
        SET OF [[T SET_OF_SubordEntry * $ *]] <<next>>
        	SubordEntry [[p element]]


SubordEntry [[P SubordEntry *]] 
	::=
	SEQUENCE 
	{
		SECIF.RelativeDistinguishedName
		[[p sub_rdn]],

	aliasEntry
		[0] BOOLEAN 
		[[b sub_aliasentry]] 
		DEFAULT FALSE,

	fromEntry
		[1] BOOLEAN 
		[[b sub_copy]]
		DEFAULT TRUE
	}


ListInfo [[P ListInfo *]] 
	::=
	SET 
	{
		SECIF.Name
		[[p lsr_object]]
		OPTIONAL,

	subordinates
		[1] SETOFSubordEntry
		[[p lsr_subordinates]],

	partialOutcomeQualifier
		[2] PartialOutcomeQualifier
		[[p lsr_poq]] 
		OPTIONAL <E<
			  (parm->lsr_poq &&
			   ( ((parm->lsr_poq->poq_limitproblem != LSR_NOLIMITPROBLEM) || 
			      (parm->lsr_poq->poq_cref != (SET_OF_ContReference * )0)) ) 
			  )>><D<0>>,

	aliasDereferenced
		[28] TBoolean [[b lsr_common]]
		-- DEFAULT FALSE
		OPTIONAL <E<parm->lsr_common->aliasDereferenced != FALSE>><D<0>>,

	performer
                [29] TPerformer [[p lsr_common]]
		OPTIONAL <E<parm->lsr_common->performer != NULLDNAME>><D<0>>,

	secparm
		[30] TSecParmResult [[p lsr_common]]
		OPTIONAL <E<parm->lsr_common->sec_parm != (SecurityParameters *) 0>><D<0>>
	}


SearchInfo [[P SearchInfo *]] 
	::=
	SET 
	{
		SECIF.Name
		[[p srr_object]]
		OPTIONAL,

	entries
		[0] SETOFEntryINFO
		[[p srr_entries]],

	partialOutcomeQualifier
		[2] PartialOutcomeQualifier
		[[p srr_poq]] 
		OPTIONAL <E<
			  (parm->srr_poq &&
			   ( ((parm->srr_poq->poq_limitproblem != LSR_NOLIMITPROBLEM) || 
			   (parm->srr_poq->poq_cref != (SET_OF_ContReference * )0)) ) )
			 >><D<0>>,

	aliasDereferenced
		[28] TBoolean [[b srr_common]]
		-- DEFAULT FALSE
		OPTIONAL <E<parm->srr_common->aliasDereferenced != FALSE>><D<0>>,

	performer
                [29] TPerformer [[p srr_common]]
		OPTIONAL <E<parm->srr_common->performer != NULLDNAME>><D<0>>,

	secparm
		[30] TSecParmResult [[p srr_common]]
		OPTIONAL <E<parm->srr_common->sec_parm != (SecurityParameters *) 0>><D<0>>
	}


SETOFDName [[P SET_OF_DName *]]
	::=
        SET OF [[T SET_OF_DName * $ *]] <<next>>
        	SECIF.Name [[p element]]


PartialOutcomeQualifier [[P PartialOutQual *]]
	::=
	SET
	{
	limitProblem
		[0] LimitProblem [[i poq_limitproblem]]
		OPTIONAL <E<parm->poq_limitproblem != LSR_NOLIMITPROBLEM>><D<0>>,					 
	unexplored
		[1] SETOFContinuationReference [[p poq_cref]]
		OPTIONAL,

	unavailableCriticalExtensions
		[2] BOOLEAN [[b poq_no_ext]]
		DEFAULT FALSE
	}


-- This is pulled up as an 'i' type, so no parameters are needed.
ReferenceType
        ::=
        ENUMERATED
        {
        superior(1) ,
        subordinate(2) ,
        cross(3) ,
        nonSpecificSubordinate(4)
        }


-- Pulled up
LimitProblem
	::=
	INTEGER 
	{
	timeLimitExceeded(0) ,
	sizeLimitExceeded(1) ,
	administrativeLimitExceeded(2)
	}


SETOFContinuationReference [[P SET_OF_ContReference *]]
	::=
        SET OF [[T SET_OF_ContReference * $ *]] <<next>>
        	ContinuationReference [[p element]]


ContinuationReference [[P ContReference *]]
        ::=
        SET
        {
        targetObject
                [0] SECIF.Name [[p cr_name]],

        aliasedRDNs
                [1] INTEGER [[i cr_aliasedRDNs]]
                OPTIONAL <E< parm->cr_aliasedRDNs != CR_NOALIASEDRDNS>><D<0>>,

        operationProgress
                [2] OperationProgress [[p cr_progress]],

        rdnsResolved
                [3] INTEGER [[i cr_rdn_resolved]]
                OPTIONAL <E< parm->cr_rdn_resolved != CR_RDNRESOLVED_NOTDEFINED>><D<0>>,

        referenceType
                [4] ReferenceType [[i cr_reftype]]
                OPTIONAL <E< parm->cr_reftype != RT_UNDEFINED>><D<0>>,

        accessPoints
		[5] SETOFAccessPoint [[p cr_accesspoints]]
	}


SETOFAccessPoint [[P SET_OF_AccessPoint *]]
	::=
        SET OF [[T SET_OF_AccessPoint * $ *]] <<next>>
        	AccessPoint [[p element]]


AccessPoint [[P AccessPoint *]]
        ::=
        SET
        {
                [0] SECIF.Name [[p parm->ap_name]],

                [1] PSAPaddr [[p parm->ap_address]]				
        }


PSAPaddr [[P typeDSE_PSAPaddr *]]
	::=
        %D{
        if ((*(parm) = (typeDSE_PSAPaddr *) calloc (1, sizeof **(parm))) == ((typeDSE_PSAPaddr *) 0)) {
        	advise (NULLCP, "out of memory");
                return NOTOK;
        }
	(*parm)->nAddress = (SET_OF_OctetString *)0;
	(*parm)->pSelector.octets = CNULL;
	(*parm)->pSelector.noctets = 0;
	(*parm)->sSelector.octets = CNULL;
	(*parm)->sSelector.noctets = 0;
	(*parm)->tSelector.octets = CNULL;
	(*parm)->tSelector.noctets = 0;
	%}
	SEQUENCE 
	{
	pSelector
		[0] OCTET STRING
		[[o pSelector.octets $ pSelector.noctets]]
		OPTIONAL <E<parm->pSelector.noctets > 0>><D<(*parm)->pSelector.noctets > 0>>
		,

	sSelector
		[1] OCTET STRING
		[[o sSelector.octets $ sSelector.noctets]]
		OPTIONAL <E<parm->sSelector.noctets > 0>><D<(*parm)->sSelector.noctets > 0>>
		,

	tSelector
		[2] OCTET STRING
		[[o tSelector.octets $ tSelector.noctets]]
		OPTIONAL <E<parm->tSelector.noctets > 0>><D<(*parm)->tSelector.noctets > 0>>
		,

	nAddress
		[3] SEC.SETOFOctetString [[p nAddress]]
	}


DECODER parse


PSAPaddr [[P typeDSE_PSAPaddr *]]
	::=
        %D{
        if ((*(parm) = (typeDSE_PSAPaddr *) calloc (1, sizeof **(parm))) == ((typeDSE_PSAPaddr *) 0)) {
        	advise (NULLCP, "out of memory");
                return NOTOK;
        }
	(*parm)->nAddress = (SET_OF_OctetString *)0;
	(*parm)->pSelector.octets = CNULL;
	(*parm)->pSelector.noctets = 0;
	(*parm)->sSelector.octets = CNULL;
	(*parm)->sSelector.noctets = 0;
	(*parm)->tSelector.octets = CNULL;
	(*parm)->tSelector.noctets = 0;
	%}
	SEQUENCE 
	{
	pSelector
		[0] OCTET STRING
		[[o pSelector.octets $ pSelector.noctets]]
		OPTIONAL <E<parm->pSelector.noctets > 0>><D<(*parm)->pSelector.noctets > 0>>
		,

	sSelector
		[1] OCTET STRING
		[[o sSelector.octets $ sSelector.noctets]]
		OPTIONAL <E<parm->sSelector.noctets > 0>><D<(*parm)->sSelector.noctets > 0>>
		,

	tSelector
		[2] OCTET STRING
		[[o tSelector.octets $ tSelector.noctets]]
		OPTIONAL <E<parm->tSelector.noctets > 0>><D<(*parm)->tSelector.noctets > 0>>
		,

	nAddress
		[3] SEC.SETOFOctetString [[p nAddress]]
	}

END



%{
/************************ local functions: ************************/




/* from ISODEDIR/dsap/x500as/DAS_tables.c */
static
char   * aux_int2strb_alloc (n, len)
register int    n;
int     len;
{
	register int    i;
	static char *buffer;

	buffer = calloc (1,sizeof (int) + 1);

	for (i = 0; i < len; i++)
		if (n & (1 << i))
			buffer[i / 8] |= (1 << (7 - (i % 8)));

	return buffer;
}


%}

#if defined(X500) && defined(STRONG)

#include <stdio.h>
#include "STRONG-types.h"


# line 5 "strong.py"
	/* surrounding global definitions */
#include        "secude-stub.h"

	static char 	    * aux_int2strb_alloc();

extern caddr_t _ZptrtabSTRONG[];	/* forward decl */

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static ptpe et_TSETOFSECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[0] },
	{ OBJECT, OFFSET(CommonArguments , ext), _ZSETOFSECExtensionSTRONG, 0, (char **)&_ZptrtabSTRONG[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[0] }
	};

static ptpe et_TIntegerSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[1] },
	{ INTEGER, OFFSET(CommonArguments , aliasedRDNs), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[1] }
	};

static ptpe et_TOperationProgressSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[2] },
	{ OBJECT, OFFSET(CommonArguments , progress), _ZOperationProgressSTRONG, 0, (char **)&_ZptrtabSTRONG[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[2] }
	};

static ptpe et_TRequestorSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[3] },
	{ EXTOBJ, OFFSET(CommonArguments , requestor), _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[3] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[3] }
	};

static ptpe et_TSecParmArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[5] },
	{ OBJECT, OFFSET(CommonArguments , sec_parm), _ZSecurityParametersSTRONG, 0, (char **)&_ZptrtabSTRONG[5] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[5] }
	};

static ptpe et_TServiceControlsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[6] },
	{ OBJECT, OFFSET(CommonArguments , svc), _ZServiceControlsSTRONG, 0, (char **)&_ZptrtabSTRONG[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[6] }
	};

static ptpe et_TBooleanSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[7] },
	{ BOOLEAN, OFFSET(CommonRes , aliasDereferenced), 1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[7] }
	};

static ptpe et_TPerformerSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[8] },
	{ EXTOBJ, OFFSET(CommonRes , performer), _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[8] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[8] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[8] }
	};

static ptpe et_TSecParmResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[9] },
	{ OBJECT, OFFSET(CommonRes , sec_parm), _ZSecurityParametersSTRONG, 0, (char **)&_ZptrtabSTRONG[9] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[9] }
	};

static ptpe et_TAttributeSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[10] },
	{ EXTOBJ, OFFSET(SET_OF_Attr , element), _ZAttributeSECIF, 0, (char **)&_ZptrtabSTRONG[10] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[10] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[10] }
	};

static ptpe et_TokenTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[11] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[11] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[12] },
	{ EXTOBJ, OFFSET(TokenTBS , signatureAI), _ZAlgorithmIdentifierSEC, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[12] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[12] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[14] },
	{ EXTOBJ, OFFSET(TokenTBS , dname), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[14] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[14] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[15] },
	{ T_STRING, OFFSET(TokenTBS , time), 23, FL_UNIVERSAL, NULLVP },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[16] },
	{ EXTOBJ, OFFSET(TokenTBS , random), _ZBitStringSEC, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[16] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[16] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[11] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[11] }
	};

static ptpe et_TokenSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[17] },
	{ OBJECT, OFFSET(Token , tbs), _ZTokenTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[17] },
	{ EXTOBJ, OFFSET(Token , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[17] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ EXTOBJ, OFFSET(Token , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[17] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] }
	};

static ptpe et_AddArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[18] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[18] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(AddArgumentTBS , ada_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[20] },
	{ EXTOBJ, OFFSET(AddArgumentTBS , ada_entry), _ZSETOFAttributeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[20] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[20] },
	{ BOPTIONAL, 0, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, 1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, 2, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, 3, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, 4, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, 5, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[18] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[18] }
	};

static ptpe et_AddArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ SCTRL, 6, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[27] },
	{ OBJECT, OFFSET(AddArgument , tbs), _ZAddArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[27] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[27] },
	{ OBJECT, OFFSET(AddArgument , tbs), _ZAddArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[27] },
	{ EXTOBJ, OFFSET(AddArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[27] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ EXTOBJ, OFFSET(AddArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[27] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] }
	};

static ptpe et_CompareArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[28] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[28] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(CompareArgumentTBS , cma_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[29] },
	{ EXTOBJ, OFFSET(CompareArgumentTBS , cma_purported), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[29] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[29] },
	{ BOPTIONAL, 7, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, 8, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, 9, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, 10, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, 11, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, 12, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[28] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[28] }
	};

static ptpe et_CompareArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ SCTRL, 13, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[30] },
	{ OBJECT, OFFSET(CompareArgument , tbs), _ZCompareArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[30] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[30] },
	{ OBJECT, OFFSET(CompareArgument , tbs), _ZCompareArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[30] },
	{ EXTOBJ, OFFSET(CompareArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[30] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ EXTOBJ, OFFSET(CompareArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[30] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] }
	};

static ptpe et_CompareResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[31] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[31] },
	{ EXTOBJ, OFFSET(CompareResultTBS , cmr_object), _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[31] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[31] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[32] },
	{ BOOLEAN, OFFSET(CompareResultTBS , cmr_matched), 1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[32] },
	{ DFLT_F,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ BOOLEAN, OFFSET(CompareResultTBS , cmr_fromEntry), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ BOPTIONAL, 14, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[34]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, OFFSET(CompareResultTBS , cmr_common), 1, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[7] },
	{ BOPTIONAL, 15, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[35]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, OFFSET(CompareResultTBS , cmr_common), _ZTPerformerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[35] },
	{ BOPTIONAL, 16, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(CompareResultTBS , cmr_common), _ZTSecParmResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[31] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[31] }
	};

static ptpe et_CompareResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ SCTRL, 17, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[36] },
	{ OBJECT, OFFSET(CompareResult , tbs), _ZCompareResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[36] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[36] },
	{ OBJECT, OFFSET(CompareResult , tbs), _ZCompareResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[36] },
	{ EXTOBJ, OFFSET(CompareResult , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[36] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ EXTOBJ, OFFSET(CompareResult , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[36] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] }
	};

static ptpe et_ListArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[37] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[37] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(ListArgumentTBS , object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ BOPTIONAL, 18, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, 19, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, 20, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, 21, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, 22, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, 23, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[37] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[37] }
	};

static ptpe et_ListArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ SCTRL, 24, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[38] },
	{ OBJECT, OFFSET(ListArgument , tbs), _ZListArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[38] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[38] },
	{ OBJECT, OFFSET(ListArgument , tbs), _ZListArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[38] },
	{ EXTOBJ, OFFSET(ListArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[38] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ EXTOBJ, OFFSET(ListArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[38] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] }
	};

static ptpe et_ListResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] },
	{ SCTRL, 25, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[39] },
	{ OBJECT, OFFSET(ListResultTBS , lsrtbs_un.listinfo), _ZListInfoSTRONG, 0, (char **)&_ZptrtabSTRONG[40] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[41] },
	{ OBJECT, OFFSET(ListResultTBS , lsrtbs_un.uncorrel_listinfo), _ZSETOFListResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[41] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] }
	};

static ptpe et_ListResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ SCTRL, 26, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[42] },
	{ OBJECT, OFFSET(ListResult , tbs), _ZListResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[42] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[42] },
	{ OBJECT, OFFSET(ListResult , tbs), _ZListResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[42] },
	{ EXTOBJ, OFFSET(ListResult , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[42] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ EXTOBJ, OFFSET(ListResult , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[42] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] }
	};

static ptpe et_SETOFListResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[43] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[43] },
	{ OBJECT, OFFSET(SET_OF_ListResult , element), _ZListResultSTRONG, 0, (char **)&_ZptrtabSTRONG[43] },
	{ PE_END, OFFSET(SET_OF_ListResult , next), 0, 0, (char **)&_ZptrtabSTRONG[43] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[43] }
	};

static ptpe et_ModifyEntryArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[44] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[44] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(ModifyEntryArgumentTBS , mea_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[45] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_changes), _ZEntryModificationSequenceSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[45] },
	{ BOPTIONAL, 27, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, 28, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, 29, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, 30, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, 31, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, 32, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[44] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[44] }
	};

static ptpe et_ModifyEntryArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ SCTRL, 33, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[46] },
	{ OBJECT, OFFSET(ModifyEntryArgument , tbs), _ZModifyEntryArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[46] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[46] },
	{ OBJECT, OFFSET(ModifyEntryArgument , tbs), _ZModifyEntryArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[46] },
	{ EXTOBJ, OFFSET(ModifyEntryArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[46] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ EXTOBJ, OFFSET(ModifyEntryArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[46] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] }
	};

static ptpe et_ModifyRDNArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[47] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[47] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(ModifyRDNArgumentTBS , mra_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[48] },
	{ EXTOBJ, OFFSET(ModifyRDNArgumentTBS , mra_newrdn), _ZRelativeDistinguishedNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[48] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[48] },
	{ DFLT_F,	0,	0,	0, (char **)&_ZptrtabSTRONG[49] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[49] },
	{ BOOLEAN, OFFSET(ModifyRDNArgumentTBS , deleterdn), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[49] },
	{ BOPTIONAL, 34, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, 35, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, 36, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, 37, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, 38, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, 39, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[47] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[47] }
	};

static ptpe et_ModifyRDNArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ SCTRL, 40, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[50] },
	{ OBJECT, OFFSET(ModifyRDNArgument , tbs), _ZModifyRDNArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[50] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[50] },
	{ OBJECT, OFFSET(ModifyRDNArgument , tbs), _ZModifyRDNArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[50] },
	{ EXTOBJ, OFFSET(ModifyRDNArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[50] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ EXTOBJ, OFFSET(ModifyRDNArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[50] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] }
	};

static ptpe et_ReadArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[51] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[51] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(ReadArgumentTBS , object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[52] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_eis), _ZEntryInfoSELSTRONG, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[52] },
	{ BOPTIONAL, 41, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, 42, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, 43, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, 44, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, 45, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, 46, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[51] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[51] }
	};

static ptpe et_ReadArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ SCTRL, 47, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[53] },
	{ OBJECT, OFFSET(ReadArgument , tbs), _ZReadArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[53] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[53] },
	{ OBJECT, OFFSET(ReadArgument , tbs), _ZReadArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[53] },
	{ EXTOBJ, OFFSET(ReadArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[53] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ EXTOBJ, OFFSET(ReadArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[53] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] }
	};

static ptpe et_ReadResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[54] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[54] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[20] },
	{ OBJECT, OFFSET(ReadResultTBS , rdr_entry), _ZEntryINFOSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[20] },
	{ BOPTIONAL, 48, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[34]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, OFFSET(ReadResultTBS , rdr_common), 1, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[7] },
	{ BOPTIONAL, 49, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[35]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, OFFSET(ReadResultTBS , rdr_common), _ZTPerformerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[35] },
	{ BOPTIONAL, 50, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ReadResultTBS , rdr_common), _ZTSecParmResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[54] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[54] }
	};

static ptpe et_ReadResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ SCTRL, 51, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[55] },
	{ OBJECT, OFFSET(ReadResult , tbs), _ZReadResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[55] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[55] },
	{ OBJECT, OFFSET(ReadResult , tbs), _ZReadResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[55] },
	{ EXTOBJ, OFFSET(ReadResult , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[55] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ EXTOBJ, OFFSET(ReadResult , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[55] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] }
	};

static ptpe et_RemoveArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[56] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[56] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(RemoveArgumentTBS , rma_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ BOPTIONAL, 52, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, 53, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, 54, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, 55, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, 56, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, 57, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[56] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[56] }
	};

static ptpe et_RemoveArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ SCTRL, 58, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[57] },
	{ OBJECT, OFFSET(RemoveArgument , tbs), _ZRemoveArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[57] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[57] },
	{ OBJECT, OFFSET(RemoveArgument , tbs), _ZRemoveArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[57] },
	{ EXTOBJ, OFFSET(RemoveArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[57] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ EXTOBJ, OFFSET(RemoveArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[57] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] }
	};

static ptpe et_SearchArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[58] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[58] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[59] },
	{ EXTOBJ, OFFSET(SearchArgumentTBS , baseobject), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[59] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[59] },
	{ DFLT_F,	0,	0,	0, (char **)&_ZptrtabSTRONG[60] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[60] },
	{ INTEGER, OFFSET(SearchArgumentTBS , subset), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[60] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[61] },
	{ OBJECT, OFFSET(SearchArgumentTBS , filter), _ZFilterSTRONG, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[61] },
	{ DFLT_F,	1,	0,	0, (char **)&_ZptrtabSTRONG[62] },
	{ ETAG, 0, 3, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[62] },
	{ BOOLEAN, OFFSET(SearchArgumentTBS , searchaliases), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[62] },
	{ ETAG, 0, 4, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[52] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_eis), _ZEntryInfoSELSTRONG, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[52] },
	{ BOPTIONAL, 59, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, 60, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, 61, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, 62, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, 63, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, 64, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[58] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[58] }
	};

static ptpe et_SearchArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ SCTRL, 65, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[63] },
	{ OBJECT, OFFSET(SearchArgument , tbs), _ZSearchArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[63] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[63] },
	{ OBJECT, OFFSET(SearchArgument , tbs), _ZSearchArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[63] },
	{ EXTOBJ, OFFSET(SearchArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[63] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ EXTOBJ, OFFSET(SearchArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[63] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] }
	};

static ptpe et_SearchResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] },
	{ SCTRL, 66, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[64] },
	{ OBJECT, OFFSET(SearchResultTBS , srrtbs_un.searchinfo), _ZSearchInfoSTRONG, 0, (char **)&_ZptrtabSTRONG[65] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[66] },
	{ OBJECT, OFFSET(SearchResultTBS , srrtbs_un.uncorrel_searchinfo), _ZSETOFSearchResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[66] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] }
	};

static ptpe et_SearchResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ SCTRL, 67, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[67] },
	{ OBJECT, OFFSET(SearchResult , tbs), _ZSearchResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[67] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[67] },
	{ OBJECT, OFFSET(SearchResult , tbs), _ZSearchResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[67] },
	{ EXTOBJ, OFFSET(SearchResult , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[67] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ EXTOBJ, OFFSET(SearchResult , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[67] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] }
	};

static ptpe et_SETOFSearchResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[68] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[68] },
	{ OBJECT, OFFSET(SET_OF_SearchResult , element), _ZSearchResultSTRONG, 0, (char **)&_ZptrtabSTRONG[68] },
	{ PE_END, OFFSET(SET_OF_SearchResult , next), 0, 0, (char **)&_ZptrtabSTRONG[68] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[68] }
	};

static ptpe et_EntryModificationSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] },
	{ SCTRL, 68, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[69] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[70] },
	{ EXTOBJ, OFFSET(EntryModification , em_un.em_un_attr), _ZAttributeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[70] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[70] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[71] },
	{ OBJID, OFFSET(EntryModification , em_un.em_un_attrtype), 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[71] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[72] },
	{ EXTOBJ, OFFSET(EntryModification , em_un.em_un_attr), _ZAttributeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[72] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[72] },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[73] },
	{ EXTOBJ, OFFSET(EntryModification , em_un.em_un_attr), _ZAttributeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[73] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[73] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] }
	};

static ptpe et_EntryModificationSequenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[74] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[74] },
	{ OBJECT, OFFSET(SEQUENCE_OF_EntryModification , element), _ZEntryModificationSTRONG, 0, (char **)&_ZptrtabSTRONG[74] },
	{ PE_END, OFFSET(SEQUENCE_OF_EntryModification , next), 0, 0, (char **)&_ZptrtabSTRONG[74] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[74] }
	};

static ptpe et_ServiceControlsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[75] },
	{ UCODE, 69, 0, 0, (char **)&_ZptrtabSTRONG[75] }, /* line 973 */
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[75] },
	{ BOPTIONAL, 71, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[76]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[76] },
	{ BITSTR_PTR, OFFSET(ServiceControls , svc_tmp), 3, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[76] },
	{ BITSTR_LEN, OFFSET(ServiceControls , svc_len), 3, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[76] },
	{ UCODE, 70, 0, 0, (char **)&_ZptrtabSTRONG[76] }, /* line 991 */
	{ DFLT_F,	1,	0,	0, (char **)&_ZptrtabSTRONG[77] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[77] },
	{ INTEGER, OFFSET(ServiceControls , svc_prio), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[77] },
	{ BOPTIONAL, 72, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[78]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[78] },
	{ INTEGER, OFFSET(ServiceControls , svc_timelimit), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[78] },
	{ BOPTIONAL, 73, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[79]},
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[79] },
	{ INTEGER, OFFSET(ServiceControls , svc_sizelimit), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[79] },
	{ BOPTIONAL, 74, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[80]},
	{ ETAG, 0, 4, FL_CONTEXT, (char **)&_ZptrtabSTRONG[80] },
	{ INTEGER, OFFSET(ServiceControls , svc_scopeofreferral), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[80] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[75] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[75] }
	};

static ptpe et_SecurityParametersSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[81] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[81] },
	{ ETAG, 0, 0, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[82] },
	{ EXTOBJ, OFFSET(SecurityParameters , certPath), _ZCertificationPathSECAF, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[82] },
	{ EXTMOD, 83, 0, 0, (char **)&_ZptrtabSTRONG[82] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[14] },
	{ EXTOBJ, OFFSET(SecurityParameters , name), _ZNameSECIF, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[14] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[14] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[15] },
	{ T_STRING, OFFSET(SecurityParameters , time), 23, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, 0, 3, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[16] },
	{ EXTOBJ, OFFSET(SecurityParameters , random), _ZBitStringSEC, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[16] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[16] },
	{ DFLT_F,	0,	0,	0, (char **)&_ZptrtabSTRONG[84] },
	{ ETAG, 0, 4, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[84] },
	{ INTEGER, OFFSET(SecurityParameters , target), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[84] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[81] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[81] }
	};

static ptpe et_OperationProgressSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[85] },
	{ UCODE, 75, 0, 0, (char **)&_ZptrtabSTRONG[85] }, /* line 1065 */
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[85] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[86] },
	{ INTEGER, OFFSET(OperationProgress , opResolutionPhase), 10, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[86] },
	{ BOPTIONAL, 76, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[87]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[87] },
	{ INTEGER, OFFSET(OperationProgress , opNextRDNToBeResolved), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[87] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[85] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[85] }
	};

static ptpe et_SETOFSECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[88] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[88] },
	{ OBJECT, OFFSET(SET_OF_SECExtension , element), _ZSECExtensionSTRONG, 0, (char **)&_ZptrtabSTRONG[88] },
	{ PE_END, OFFSET(SET_OF_SECExtension , next), 0, 0, (char **)&_ZptrtabSTRONG[88] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[88] }
	};

static ptpe et_SECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[89] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[89] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[90] },
	{ INTEGER, OFFSET(SECExtension , ext_id), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[90] },
	{ DFLT_F,	0,	0,	0, (char **)&_ZptrtabSTRONG[91] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[91] },
	{ BOOLEAN, OFFSET(SECExtension , ext_critical), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[91] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[92] },
	{ ANY, OFFSET(SECExtension , ext_item), -1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[92] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[89] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[89] }
	};

static ptpe et_SETOFAttrAttrTypeCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[93] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[93] },
	{ OBJECT, OFFSET(SET_OF_AttrAttrTypeCHOICE , element), _ZAttrAttrTypeCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[93] },
	{ PE_END, OFFSET(SET_OF_AttrAttrTypeCHOICE , next), 0, 0, (char **)&_ZptrtabSTRONG[93] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[93] }
	};

static ptpe et_AttrAttrTypeCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] },
	{ SCTRL, 77, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[94] },
	{ OBJID, OFFSET(AttrAttrTypeCHOICE , choice_un.choice_un_attrtype), 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[94] },
	{ OBJECT, OFFSET(AttrAttrTypeCHOICE , choice_un.choice_un_attr), _ZTAttributeSTRONG, 0, (char **)&_ZptrtabSTRONG[94] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] }
	};

static ptpe et_EntryInfoSELSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[95] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[95] },
	{ BOPTIONAL, 79, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[96]},
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[96] },
	{ SCTRL, 78, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[96] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[97] },
	{ T_NULL, 0, 5, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[97] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[98] },
	{ EXTOBJ, OFFSET(EntryInfoSEL , eis_select), _ZSETOFAttributeTypeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[98] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[98] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[96] },
	{ DFLT_F,	1,	0,	0, (char **)&_ZptrtabSTRONG[99] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[99] },
	{ INTEGER, OFFSET(EntryInfoSEL , eis_infotypes), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[99] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[95] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[95] }
	};

static ptpe et_SETOFEntryINFOSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[100] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[100] },
	{ OBJECT, OFFSET(SET_OF_EntryINFO , element), _ZEntryINFOSTRONG, 0, (char **)&_ZptrtabSTRONG[100] },
	{ PE_END, OFFSET(SET_OF_EntryINFO , next), 0, 0, (char **)&_ZptrtabSTRONG[100] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[100] }
	};

static ptpe et_EntryINFOSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[101] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[101] },
	{ EXTOBJ, OFFSET(EntryINFO , ent_dn), _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[14] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[14] },
	{ DFLT_F,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ BOOLEAN, OFFSET(EntryINFO , ent_fromentry), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ OBJECT, OFFSET(EntryINFO , ent_attr), _ZSETOFAttrAttrTypeCHOICESTRONG, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[101] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[101] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[101] }
	};

static ptpe et_StringsCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] },
	{ SCTRL, 80, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[102] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[103] },
	{ ANY, OFFSET(StringsCHOICE , strings_un.initial), -1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[103] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[104] },
	{ ANY, OFFSET(StringsCHOICE , strings_un.any), -1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[104] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[105] },
	{ ANY, OFFSET(StringsCHOICE , strings_un.final), -1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[105] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] }
	};

static ptpe et_SEQUENCEOFStringsCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[106] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[106] },
	{ OBJECT, OFFSET(SEQUENCE_OF_StringsCHOICE , element), _ZStringsCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[106] },
	{ PE_END, OFFSET(SEQUENCE_OF_StringsCHOICE , next), 0, 0, (char **)&_ZptrtabSTRONG[106] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[106] }
	};

static ptpe et_FilterSubstringsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[107] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[107] },
	{ OBJID, OFFSET(SFilterSubstrings , type), 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[108] },
	{ OBJECT, OFFSET(SFilterSubstrings , seq), _ZSEQUENCEOFStringsCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[109] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[107] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[107] }
	};

static ptpe et_FilterItemSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] },
	{ SCTRL, 81, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[110] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[111] },
	{ EXTOBJ, OFFSET(SFilterItem , fi_un.fi_un_ava), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[111] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[111] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[112] },
	{ OBJECT, OFFSET(SFilterItem , fi_un.fi_un_substrings), _ZFilterSubstringsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[112] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[113] },
	{ EXTOBJ, OFFSET(SFilterItem , fi_un.fi_un_ava), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[113] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[113] },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[114] },
	{ EXTOBJ, OFFSET(SFilterItem , fi_un.fi_un_ava), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[114] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[114] },
	{ ETAG, 0, 4, FL_CONTEXT, (char **)&_ZptrtabSTRONG[115] },
	{ OBJID, OFFSET(SFilterItem , fi_un.fi_un_type), 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[115] },
	{ ETAG, 0, 5, FL_CONTEXT, (char **)&_ZptrtabSTRONG[116] },
	{ EXTOBJ, OFFSET(SFilterItem , fi_un.fi_un_ava), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[116] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[116] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] }
	};

static ptpe et_SETOFFilterSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[117] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[117] },
	{ OBJECT, OFFSET(SET_OF_SFilter , element), _ZFilterSTRONG, 0, (char **)&_ZptrtabSTRONG[117] },
	{ PE_END, OFFSET(SET_OF_SFilter , next), 0, 0, (char **)&_ZptrtabSTRONG[117] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[117] }
	};

static ptpe et_FilterSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] },
	{ SCTRL, 82, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[118] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[92] },
	{ OBJECT, OFFSET(SFilter , flt_un.flt_un_item), _ZFilterItemSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[92] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[119] },
	{ OBJECT, OFFSET(SFilter , flt_un.flt_un_filterset), _ZSETOFFilterSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[119] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[120] },
	{ OBJECT, OFFSET(SFilter , flt_un.flt_un_filterset), _ZSETOFFilterSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[120] },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[121] },
	{ OBJECT, OFFSET(SFilter , flt_un.flt_un_filter), _ZFilterSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[121] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] }
	};

static ptpe et_SETOFSubordEntrySTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[122] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[122] },
	{ OBJECT, OFFSET(SET_OF_SubordEntry , element), _ZSubordEntrySTRONG, 0, (char **)&_ZptrtabSTRONG[122] },
	{ PE_END, OFFSET(SET_OF_SubordEntry , next), 0, 0, (char **)&_ZptrtabSTRONG[122] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[122] }
	};

static ptpe et_SubordEntrySTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[123] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[123] },
	{ EXTOBJ, OFFSET(SubordEntry , sub_rdn), _ZRelativeDistinguishedNameSECIF, 0, (char **)&_ZptrtabSTRONG[123] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[123] },
	{ DFLT_F,	0,	0,	0, (char **)&_ZptrtabSTRONG[124] },
	{ ETAG, 0, 0, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[124] },
	{ BOOLEAN, OFFSET(SubordEntry , sub_aliasentry), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[124] },
	{ DFLT_F,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ BOOLEAN, OFFSET(SubordEntry , sub_copy), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[123] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[123] }
	};

static ptpe et_ListInfoSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[125] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[125] },
	{ EXTOBJ, OFFSET(ListInfo , lsr_object), _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[125] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[125] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[126] },
	{ OBJECT, OFFSET(ListInfo , lsr_subordinates), _ZSETOFSubordEntrySTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[126] },
	{ BOPTIONAL, 83, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[127]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[127] },
	{ OBJECT, OFFSET(ListInfo , lsr_poq), _ZPartialOutcomeQualifierSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[127] },
	{ BOPTIONAL, 84, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[34]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, OFFSET(ListInfo , lsr_common), 1, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[7] },
	{ BOPTIONAL, 85, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[35]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, OFFSET(ListInfo , lsr_common), _ZTPerformerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[35] },
	{ BOPTIONAL, 86, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ListInfo , lsr_common), _ZTSecParmResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[125] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[125] }
	};

static ptpe et_SearchInfoSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[128] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[128] },
	{ EXTOBJ, OFFSET(SearchInfo , srr_object), _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[128] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[128] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[129] },
	{ OBJECT, OFFSET(SearchInfo , srr_entries), _ZSETOFEntryINFOSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[129] },
	{ BOPTIONAL, 87, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[127]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[127] },
	{ OBJECT, OFFSET(SearchInfo , srr_poq), _ZPartialOutcomeQualifierSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[127] },
	{ BOPTIONAL, 88, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[34]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, OFFSET(SearchInfo , srr_common), 1, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[7] },
	{ BOPTIONAL, 89, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[35]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, OFFSET(SearchInfo , srr_common), _ZTPerformerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[35] },
	{ BOPTIONAL, 90, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(SearchInfo , srr_common), _ZTSecParmResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[128] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[128] }
	};

static ptpe et_SETOFDNameSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[130] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[130] },
	{ EXTOBJ, OFFSET(SET_OF_DName , element), _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[130] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[130] },
	{ PE_END, OFFSET(SET_OF_DName , next), 0, 0, (char **)&_ZptrtabSTRONG[130] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[130] }
	};

static ptpe et_PartialOutcomeQualifierSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[131] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[131] },
	{ BOPTIONAL, 91, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[132]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[132] },
	{ INTEGER, OFFSET(PartialOutQual , poq_limitproblem), 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[133] },
	{ OBJECT, OFFSET(PartialOutQual , poq_cref), _ZSETOFContinuationReferenceSTRONG, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[133] },
	{ DFLT_F,	0,	0,	0, (char **)&_ZptrtabSTRONG[134] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[134] },
	{ BOOLEAN, OFFSET(PartialOutQual , poq_no_ext), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[134] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[131] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[131] }
	};

static ptpe et_ReferenceTypeSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[135] },
	{ INTEGER, OFFSET(struct type_STRONG_ReferenceType, parm), 10, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[135] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[135] }
	};

static ptpe et_LimitProblemSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[136] },
	{ INTEGER, OFFSET(struct type_STRONG_LimitProblem, parm), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[136] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[136] }
	};

static ptpe et_SETOFContinuationReferenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[137] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[137] },
	{ OBJECT, OFFSET(SET_OF_ContReference , element), _ZContinuationReferenceSTRONG, 0, (char **)&_ZptrtabSTRONG[137] },
	{ PE_END, OFFSET(SET_OF_ContReference , next), 0, 0, (char **)&_ZptrtabSTRONG[137] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[137] }
	};

static ptpe et_ContinuationReferenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[138] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[138] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[139] },
	{ EXTOBJ, OFFSET(ContReference , cr_name), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[139] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[139] },
	{ BOPTIONAL, 92, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ INTEGER, OFFSET(ContReference , cr_aliasedRDNs), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[140] },
	{ OBJECT, OFFSET(ContReference , cr_progress), _ZOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[140] },
	{ BOPTIONAL, 93, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[141]},
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[141] },
	{ INTEGER, OFFSET(ContReference , cr_rdn_resolved), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[141] },
	{ BOPTIONAL, 94, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[142]},
	{ ETAG, 0, 4, FL_CONTEXT, (char **)&_ZptrtabSTRONG[142] },
	{ INTEGER, OFFSET(ContReference , cr_reftype), 10, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[135] },
	{ ETAG, 0, 5, FL_CONTEXT, (char **)&_ZptrtabSTRONG[143] },
	{ OBJECT, OFFSET(ContReference , cr_accesspoints), _ZSETOFAccessPointSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[143] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[138] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[138] }
	};

static ptpe et_SETOFAccessPointSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[144] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[144] },
	{ OBJECT, OFFSET(SET_OF_AccessPoint , element), _ZAccessPointSTRONG, 0, (char **)&_ZptrtabSTRONG[144] },
	{ PE_END, OFFSET(SET_OF_AccessPoint , next), 0, 0, (char **)&_ZptrtabSTRONG[144] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[144] }
	};

static ptpe et_AccessPointSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[145] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[145] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[145] },
	{ EXTOBJ, OFFSET(AccessPoint , ap_name), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[145] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[145] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[145] },
	{ OBJECT, OFFSET(AccessPoint , ap_address), _ZPSAPaddrSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[145] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[145] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[145] }
	};

static ptpe et_PSAPaddrSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[146] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[146] },
	{ BOPTIONAL, 95, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[147]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[147] },
	{ OCTET_PTR, OFFSET(typeDSE_PSAPaddr , pSelector.octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[147] },
	{ OCTET_LEN, OFFSET(typeDSE_PSAPaddr , pSelector.noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[147] },
	{ BOPTIONAL, 96, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[148]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[148] },
	{ OCTET_PTR, OFFSET(typeDSE_PSAPaddr , sSelector.octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[148] },
	{ OCTET_LEN, OFFSET(typeDSE_PSAPaddr , sSelector.noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[148] },
	{ BOPTIONAL, 97, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[149]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[149] },
	{ OCTET_PTR, OFFSET(typeDSE_PSAPaddr , tSelector.octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[149] },
	{ OCTET_LEN, OFFSET(typeDSE_PSAPaddr , tSelector.noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[149] },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[150] },
	{ EXTOBJ, OFFSET(typeDSE_PSAPaddr , nAddress), _ZSETOFOctetStringSEC, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[150] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[150] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[146] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[146] }
	};

static ptpe dt_TSETOFSECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[0] },
	{ OBJECT, OFFSET(CommonArguments , ext), _ZSETOFSECExtensionSTRONG, 0, (char **)&_ZptrtabSTRONG[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[0] }
	};

static ptpe dt_TIntegerSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[1] },
	{ MEMALLOC, 0, sizeof (CommonArguments ), 0, (char **)&_ZptrtabSTRONG[1] },
	{ INTEGER, OFFSET(CommonArguments , aliasedRDNs), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[1] }
	};

static ptpe dt_TOperationProgressSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[2] },
	{ OBJECT, OFFSET(CommonArguments , progress), _ZOperationProgressSTRONG, 0, (char **)&_ZptrtabSTRONG[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[2] }
	};

static ptpe dt_TRequestorSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[3] },
	{ EXTOBJ, OFFSET(CommonArguments , requestor), _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[3] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[3] }
	};

static ptpe dt_TSecParmArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[5] },
	{ OBJECT, OFFSET(CommonArguments , sec_parm), _ZSecurityParametersSTRONG, 0, (char **)&_ZptrtabSTRONG[5] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[5] }
	};

static ptpe dt_TServiceControlsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[6] },
	{ OBJECT, OFFSET(CommonArguments , svc), _ZServiceControlsSTRONG, 0, (char **)&_ZptrtabSTRONG[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[6] }
	};

static ptpe dt_TBooleanSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[7] },
	{ MEMALLOC, 0, sizeof (CommonRes ), 0, (char **)&_ZptrtabSTRONG[7] },
	{ BOOLEAN, OFFSET(CommonRes , aliasDereferenced), 1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[7] }
	};

static ptpe dt_TPerformerSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[8] },
	{ EXTOBJ, OFFSET(CommonRes , performer), _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[8] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[8] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[8] }
	};

static ptpe dt_TSecParmResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[9] },
	{ OBJECT, OFFSET(CommonRes , sec_parm), _ZSecurityParametersSTRONG, 0, (char **)&_ZptrtabSTRONG[9] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[9] }
	};

static ptpe dt_TAttributeSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[10] },
	{ EXTOBJ, OFFSET(SET_OF_Attr , element), _ZAttributeSECIF, 0, (char **)&_ZptrtabSTRONG[10] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[10] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[10] }
	};

static ptpe dt_TokenTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[11] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[11] },
	{ MEMALLOC, 0, sizeof (TokenTBS ), 0, (char **)&_ZptrtabSTRONG[11] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[12] },
	{ EXTOBJ, OFFSET(TokenTBS , signatureAI), _ZAlgorithmIdentifierSEC, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[12] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[12] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[14] },
	{ EXTOBJ, OFFSET(TokenTBS , dname), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[14] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[14] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[15] },
	{ T_STRING, OFFSET(TokenTBS , time), 23, FL_UNIVERSAL, NULLVP },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[16] },
	{ EXTOBJ, OFFSET(TokenTBS , random), _ZBitStringSEC, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[16] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[16] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[11] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[11] }
	};

static ptpe dt_TokenSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ UCODE, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] }, /* line 110 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[17] },
	{ MEMALLOC, 0, sizeof (Token ), 0, (char **)&_ZptrtabSTRONG[17] },
	{ OBJECT, OFFSET(Token , tbs), _ZTokenTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[17] },
	{ UCODE, 1, 0, 0, (char **)&_ZptrtabSTRONG[17] }, /* line 124 */
	{ EXTOBJ, OFFSET(Token , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[17] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ EXTOBJ, OFFSET(Token , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[17] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] }
	};

static ptpe dt_AddArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[18] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[18] },
	{ MEMALLOC, 0, sizeof (AddArgumentTBS ), 0, (char **)&_ZptrtabSTRONG[18] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(AddArgumentTBS , ada_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[20] },
	{ EXTOBJ, OFFSET(AddArgumentTBS , ada_entry), _ZSETOFAttributeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[20] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[20] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(AddArgumentTBS , ada_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[18] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[18] }
	};

static ptpe dt_AddArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ UCODE, 8, 0, 0, (char **)&_ZptrtabSTRONG[27] }, /* line 175 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ MEMALLOC, 0, sizeof (AddArgument ), 0, (char **)&_ZptrtabSTRONG[27] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[27] },
	{ OBJECT, OFFSET(AddArgument , tbs), _ZAddArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[27] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[27] },
	{ OBJECT, OFFSET(AddArgument , tbs), _ZAddArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[27] },
	{ UCODE, 10, 0, 0, (char **)&_ZptrtabSTRONG[27] }, /* line 190 */
	{ EXTOBJ, OFFSET(AddArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[27] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ EXTOBJ, OFFSET(AddArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[27] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] }
	};

static ptpe dt_CompareArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[28] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[28] },
	{ MEMALLOC, 0, sizeof (CompareArgumentTBS ), 0, (char **)&_ZptrtabSTRONG[28] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(CompareArgumentTBS , cma_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[29] },
	{ EXTOBJ, OFFSET(CompareArgumentTBS , cma_purported), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[29] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[29] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(CompareArgumentTBS , cma_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[28] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[28] }
	};

static ptpe dt_CompareArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ UCODE, 17, 0, 0, (char **)&_ZptrtabSTRONG[30] }, /* line 246 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ MEMALLOC, 0, sizeof (CompareArgument ), 0, (char **)&_ZptrtabSTRONG[30] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[30] },
	{ OBJECT, OFFSET(CompareArgument , tbs), _ZCompareArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[30] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[30] },
	{ OBJECT, OFFSET(CompareArgument , tbs), _ZCompareArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[30] },
	{ UCODE, 19, 0, 0, (char **)&_ZptrtabSTRONG[30] }, /* line 261 */
	{ EXTOBJ, OFFSET(CompareArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[30] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ EXTOBJ, OFFSET(CompareArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[30] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] }
	};

static ptpe dt_CompareResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[31] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[31] },
	{ MEMALLOC, 0, sizeof (CompareResultTBS ), 0, (char **)&_ZptrtabSTRONG[31] },
	{ EXTOBJ, OFFSET(CompareResultTBS , cmr_object), _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[31] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[31] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[32] },
	{ BOOLEAN, OFFSET(CompareResultTBS , cmr_matched), 1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[32] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ BOOLEAN, OFFSET(CompareResultTBS , cmr_fromEntry), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[34]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, OFFSET(CompareResultTBS , cmr_common), 1, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[7] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[35]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, OFFSET(CompareResultTBS , cmr_common), _ZTPerformerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[35] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(CompareResultTBS , cmr_common), _ZTSecParmResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[31] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[31] }
	};

static ptpe dt_CompareResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ UCODE, 23, 0, 0, (char **)&_ZptrtabSTRONG[36] }, /* line 308 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ MEMALLOC, 0, sizeof (CompareResult ), 0, (char **)&_ZptrtabSTRONG[36] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[36] },
	{ OBJECT, OFFSET(CompareResult , tbs), _ZCompareResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[36] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[36] },
	{ OBJECT, OFFSET(CompareResult , tbs), _ZCompareResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[36] },
	{ UCODE, 25, 0, 0, (char **)&_ZptrtabSTRONG[36] }, /* line 323 */
	{ EXTOBJ, OFFSET(CompareResult , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[36] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ EXTOBJ, OFFSET(CompareResult , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[36] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] }
	};

static ptpe dt_ListArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[37] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[37] },
	{ MEMALLOC, 0, sizeof (ListArgumentTBS ), 0, (char **)&_ZptrtabSTRONG[37] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(ListArgumentTBS , object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(ListArgumentTBS , lsa_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[37] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[37] }
	};

static ptpe dt_ListArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ UCODE, 32, 0, 0, (char **)&_ZptrtabSTRONG[38] }, /* line 376 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ MEMALLOC, 0, sizeof (ListArgument ), 0, (char **)&_ZptrtabSTRONG[38] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[38] },
	{ OBJECT, OFFSET(ListArgument , tbs), _ZListArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[38] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[38] },
	{ OBJECT, OFFSET(ListArgument , tbs), _ZListArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[38] },
	{ UCODE, 34, 0, 0, (char **)&_ZptrtabSTRONG[38] }, /* line 391 */
	{ EXTOBJ, OFFSET(ListArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[38] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ EXTOBJ, OFFSET(ListArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[38] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] }
	};

static ptpe dt_ListResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] },
	{ MEMALLOC, 0, sizeof (ListResultTBS ), 0, (char **)&_ZptrtabSTRONG[39] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[39] },
	{ OBJECT, OFFSET(ListResultTBS , lsrtbs_un.listinfo), _ZListInfoSTRONG, 0, (char **)&_ZptrtabSTRONG[40] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[41] },
	{ OBJECT, OFFSET(ListResultTBS , lsrtbs_un.uncorrel_listinfo), _ZSETOFListResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[41] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] }
	};

static ptpe dt_ListResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ UCODE, 36, 0, 0, (char **)&_ZptrtabSTRONG[42] }, /* line 423 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ MEMALLOC, 0, sizeof (ListResult ), 0, (char **)&_ZptrtabSTRONG[42] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[42] },
	{ OBJECT, OFFSET(ListResult , tbs), _ZListResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[42] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[42] },
	{ OBJECT, OFFSET(ListResult , tbs), _ZListResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[42] },
	{ UCODE, 38, 0, 0, (char **)&_ZptrtabSTRONG[42] }, /* line 438 */
	{ EXTOBJ, OFFSET(ListResult , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[42] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ EXTOBJ, OFFSET(ListResult , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[42] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] }
	};

static ptpe dt_SETOFListResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[43] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[43] },
	{ MEMALLOC, 0, sizeof (SET_OF_ListResult ), 0, (char **)&_ZptrtabSTRONG[43] },
	{ OBJECT, OFFSET(SET_OF_ListResult , element), _ZListResultSTRONG, 0, (char **)&_ZptrtabSTRONG[43] },
	{ PE_END, OFFSET(SET_OF_ListResult , next), 0, 0, (char **)&_ZptrtabSTRONG[43] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[43] }
	};

static ptpe dt_ModifyEntryArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[44] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[44] },
	{ MEMALLOC, 0, sizeof (ModifyEntryArgumentTBS ), 0, (char **)&_ZptrtabSTRONG[44] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(ModifyEntryArgumentTBS , mea_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[45] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_changes), _ZEntryModificationSequenceSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[45] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(ModifyEntryArgumentTBS , mea_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[44] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[44] }
	};

static ptpe dt_ModifyEntryArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ UCODE, 45, 0, 0, (char **)&_ZptrtabSTRONG[46] }, /* line 500 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ MEMALLOC, 0, sizeof (ModifyEntryArgument ), 0, (char **)&_ZptrtabSTRONG[46] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[46] },
	{ OBJECT, OFFSET(ModifyEntryArgument , tbs), _ZModifyEntryArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[46] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[46] },
	{ OBJECT, OFFSET(ModifyEntryArgument , tbs), _ZModifyEntryArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[46] },
	{ UCODE, 47, 0, 0, (char **)&_ZptrtabSTRONG[46] }, /* line 515 */
	{ EXTOBJ, OFFSET(ModifyEntryArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[46] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ EXTOBJ, OFFSET(ModifyEntryArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[46] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] }
	};

static ptpe dt_ModifyRDNArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[47] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[47] },
	{ MEMALLOC, 0, sizeof (ModifyRDNArgumentTBS ), 0, (char **)&_ZptrtabSTRONG[47] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(ModifyRDNArgumentTBS , mra_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[48] },
	{ EXTOBJ, OFFSET(ModifyRDNArgumentTBS , mra_newrdn), _ZRelativeDistinguishedNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[48] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[48] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[49] },
	{ BOOLEAN, OFFSET(ModifyRDNArgumentTBS , deleterdn), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[49] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[49] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(ModifyRDNArgumentTBS , mra_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[47] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[47] }
	};

static ptpe dt_ModifyRDNArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ UCODE, 54, 0, 0, (char **)&_ZptrtabSTRONG[50] }, /* line 575 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ MEMALLOC, 0, sizeof (ModifyRDNArgument ), 0, (char **)&_ZptrtabSTRONG[50] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[50] },
	{ OBJECT, OFFSET(ModifyRDNArgument , tbs), _ZModifyRDNArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[50] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[50] },
	{ OBJECT, OFFSET(ModifyRDNArgument , tbs), _ZModifyRDNArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[50] },
	{ UCODE, 56, 0, 0, (char **)&_ZptrtabSTRONG[50] }, /* line 590 */
	{ EXTOBJ, OFFSET(ModifyRDNArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[50] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ EXTOBJ, OFFSET(ModifyRDNArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[50] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] }
	};

static ptpe dt_ReadArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[51] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[51] },
	{ MEMALLOC, 0, sizeof (ReadArgumentTBS ), 0, (char **)&_ZptrtabSTRONG[51] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(ReadArgumentTBS , object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[52] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_eis), _ZEntryInfoSELSTRONG, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[52] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(ReadArgumentTBS , rda_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[51] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[51] }
	};

static ptpe dt_ReadArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ UCODE, 63, 0, 0, (char **)&_ZptrtabSTRONG[53] }, /* line 648 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ MEMALLOC, 0, sizeof (ReadArgument ), 0, (char **)&_ZptrtabSTRONG[53] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[53] },
	{ OBJECT, OFFSET(ReadArgument , tbs), _ZReadArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[53] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[53] },
	{ OBJECT, OFFSET(ReadArgument , tbs), _ZReadArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[53] },
	{ UCODE, 65, 0, 0, (char **)&_ZptrtabSTRONG[53] }, /* line 663 */
	{ EXTOBJ, OFFSET(ReadArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[53] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ EXTOBJ, OFFSET(ReadArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[53] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] }
	};

static ptpe dt_ReadResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[54] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[54] },
	{ MEMALLOC, 0, sizeof (ReadResultTBS ), 0, (char **)&_ZptrtabSTRONG[54] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[20] },
	{ OBJECT, OFFSET(ReadResultTBS , rdr_entry), _ZEntryINFOSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[20] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[34]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, OFFSET(ReadResultTBS , rdr_common), 1, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[7] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[35]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, OFFSET(ReadResultTBS , rdr_common), _ZTPerformerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[35] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ReadResultTBS , rdr_common), _ZTSecParmResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[54] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[54] }
	};

static ptpe dt_ReadResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ UCODE, 69, 0, 0, (char **)&_ZptrtabSTRONG[55] }, /* line 703 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ MEMALLOC, 0, sizeof (ReadResult ), 0, (char **)&_ZptrtabSTRONG[55] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[55] },
	{ OBJECT, OFFSET(ReadResult , tbs), _ZReadResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[55] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[55] },
	{ OBJECT, OFFSET(ReadResult , tbs), _ZReadResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[55] },
	{ UCODE, 71, 0, 0, (char **)&_ZptrtabSTRONG[55] }, /* line 718 */
	{ EXTOBJ, OFFSET(ReadResult , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[55] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ EXTOBJ, OFFSET(ReadResult , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[55] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] }
	};

static ptpe dt_RemoveArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[56] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[56] },
	{ MEMALLOC, 0, sizeof (RemoveArgumentTBS ), 0, (char **)&_ZptrtabSTRONG[56] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, OFFSET(RemoveArgumentTBS , rma_object), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[19] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(RemoveArgumentTBS , rma_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[56] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[56] }
	};

static ptpe dt_RemoveArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ UCODE, 78, 0, 0, (char **)&_ZptrtabSTRONG[57] }, /* line 771 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ MEMALLOC, 0, sizeof (RemoveArgument ), 0, (char **)&_ZptrtabSTRONG[57] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[57] },
	{ OBJECT, OFFSET(RemoveArgument , tbs), _ZRemoveArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[57] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[57] },
	{ OBJECT, OFFSET(RemoveArgument , tbs), _ZRemoveArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[57] },
	{ UCODE, 80, 0, 0, (char **)&_ZptrtabSTRONG[57] }, /* line 786 */
	{ EXTOBJ, OFFSET(RemoveArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[57] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ EXTOBJ, OFFSET(RemoveArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[57] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] }
	};

static ptpe dt_SearchArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[58] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[58] },
	{ MEMALLOC, 0, sizeof (SearchArgumentTBS ), 0, (char **)&_ZptrtabSTRONG[58] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[59] },
	{ EXTOBJ, OFFSET(SearchArgumentTBS , baseobject), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[59] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[59] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[60] },
	{ INTEGER, OFFSET(SearchArgumentTBS , subset), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[60] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[60] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[61] },
	{ OBJECT, OFFSET(SearchArgumentTBS , filter), _ZFilterSTRONG, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[61] },
	{ ETAG, 0, 3, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[62] },
	{ BOOLEAN, OFFSET(SearchArgumentTBS , searchaliases), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[62] },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[62] },
	{ ETAG, 0, 4, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[52] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_eis), _ZEntryInfoSELSTRONG, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[52] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[21]},
	{ ETAG, 0, 25, FL_CONTEXT, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[21] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 26, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTIntegerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[23]},
	{ ETAG, 0, 27, FL_CONTEXT, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[24]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTRequestorSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[24] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTSecParmArgumentSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[26]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, OFFSET(SearchArgumentTBS , sra_common), _ZTServiceControlsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[58] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[58] }
	};

static ptpe dt_SearchArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ UCODE, 87, 0, 0, (char **)&_ZptrtabSTRONG[63] }, /* line 861 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ MEMALLOC, 0, sizeof (SearchArgument ), 0, (char **)&_ZptrtabSTRONG[63] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[63] },
	{ OBJECT, OFFSET(SearchArgument , tbs), _ZSearchArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[63] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[63] },
	{ OBJECT, OFFSET(SearchArgument , tbs), _ZSearchArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[63] },
	{ UCODE, 89, 0, 0, (char **)&_ZptrtabSTRONG[63] }, /* line 876 */
	{ EXTOBJ, OFFSET(SearchArgument , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[63] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ EXTOBJ, OFFSET(SearchArgument , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[63] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] }
	};

static ptpe dt_SearchResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] },
	{ MEMALLOC, 0, sizeof (SearchResultTBS ), 0, (char **)&_ZptrtabSTRONG[64] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[64] },
	{ OBJECT, OFFSET(SearchResultTBS , srrtbs_un.searchinfo), _ZSearchInfoSTRONG, 0, (char **)&_ZptrtabSTRONG[65] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[66] },
	{ OBJECT, OFFSET(SearchResultTBS , srrtbs_un.uncorrel_searchinfo), _ZSETOFSearchResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[66] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] }
	};

static ptpe dt_SearchResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ UCODE, 91, 0, 0, (char **)&_ZptrtabSTRONG[67] }, /* line 906 */
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ MEMALLOC, 0, sizeof (SearchResult ), 0, (char **)&_ZptrtabSTRONG[67] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[67] },
	{ OBJECT, OFFSET(SearchResult , tbs), _ZSearchResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[67] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[67] },
	{ OBJECT, OFFSET(SearchResult , tbs), _ZSearchResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[67] },
	{ UCODE, 93, 0, 0, (char **)&_ZptrtabSTRONG[67] }, /* line 921 */
	{ EXTOBJ, OFFSET(SearchResult , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[67] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ EXTOBJ, OFFSET(SearchResult , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[67] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] }
	};

static ptpe dt_SETOFSearchResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[68] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[68] },
	{ MEMALLOC, 0, sizeof (SET_OF_SearchResult ), 0, (char **)&_ZptrtabSTRONG[68] },
	{ OBJECT, OFFSET(SET_OF_SearchResult , element), _ZSearchResultSTRONG, 0, (char **)&_ZptrtabSTRONG[68] },
	{ PE_END, OFFSET(SET_OF_SearchResult , next), 0, 0, (char **)&_ZptrtabSTRONG[68] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[68] }
	};

static ptpe dt_EntryModificationSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] },
	{ MEMALLOC, 0, sizeof (EntryModification ), 0, (char **)&_ZptrtabSTRONG[69] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[69] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[70] },
	{ EXTOBJ, OFFSET(EntryModification , em_un.em_un_attr), _ZAttributeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[70] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[70] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[71] },
	{ OBJID, OFFSET(EntryModification , em_un.em_un_attrtype), 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[71] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[72] },
	{ EXTOBJ, OFFSET(EntryModification , em_un.em_un_attr), _ZAttributeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[72] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[72] },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[73] },
	{ EXTOBJ, OFFSET(EntryModification , em_un.em_un_attr), _ZAttributeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[73] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[73] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] }
	};

static ptpe dt_EntryModificationSequenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[74] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[74] },
	{ MEMALLOC, 0, sizeof (SEQUENCE_OF_EntryModification ), 0, (char **)&_ZptrtabSTRONG[74] },
	{ OBJECT, OFFSET(SEQUENCE_OF_EntryModification , element), _ZEntryModificationSTRONG, 0, (char **)&_ZptrtabSTRONG[74] },
	{ PE_END, OFFSET(SEQUENCE_OF_EntryModification , next), 0, 0, (char **)&_ZptrtabSTRONG[74] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[74] }
	};

static ptpe dt_ServiceControlsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[75] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[75] },
	{ MEMALLOC, 0, sizeof (ServiceControls ), 0, (char **)&_ZptrtabSTRONG[75] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[76]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[76] },
	{ BITSTR_PTR, OFFSET(ServiceControls , svc_tmp), 3, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[76] },
	{ BITSTR_LEN, OFFSET(ServiceControls , svc_len), 3, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[76] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[77] },
	{ INTEGER, OFFSET(ServiceControls , svc_prio), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[77] },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[77] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[78]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[78] },
	{ INTEGER, OFFSET(ServiceControls , svc_timelimit), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[78] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[79]},
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[79] },
	{ INTEGER, OFFSET(ServiceControls , svc_sizelimit), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[79] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[80]},
	{ ETAG, 0, 4, FL_CONTEXT, (char **)&_ZptrtabSTRONG[80] },
	{ INTEGER, OFFSET(ServiceControls , svc_scopeofreferral), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[80] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[75] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[75] }
	};

static ptpe dt_SecurityParametersSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[81] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[81] },
	{ MEMALLOC, 0, sizeof (SecurityParameters ), 0, (char **)&_ZptrtabSTRONG[81] },
	{ ETAG, 0, 0, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[82] },
	{ EXTOBJ, OFFSET(SecurityParameters , certPath), _ZCertificationPathSECAF, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[82] },
	{ EXTMOD, 83, 0, 0, (char **)&_ZptrtabSTRONG[82] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[14] },
	{ EXTOBJ, OFFSET(SecurityParameters , name), _ZNameSECIF, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[14] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[14] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[15] },
	{ T_STRING, OFFSET(SecurityParameters , time), 23, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, 0, 3, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[16] },
	{ EXTOBJ, OFFSET(SecurityParameters , random), _ZBitStringSEC, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[16] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[16] },
	{ ETAG, 0, 4, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[84] },
	{ INTEGER, OFFSET(SecurityParameters , target), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[84] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[84] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[81] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[81] }
	};

static ptpe dt_OperationProgressSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[85] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[85] },
	{ MEMALLOC, 0, sizeof (OperationProgress ), 0, (char **)&_ZptrtabSTRONG[85] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[86] },
	{ INTEGER, OFFSET(OperationProgress , opResolutionPhase), 10, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[86] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[87]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[87] },
	{ INTEGER, OFFSET(OperationProgress , opNextRDNToBeResolved), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[87] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[85] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[85] }
	};

static ptpe dt_SETOFSECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[88] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[88] },
	{ MEMALLOC, 0, sizeof (SET_OF_SECExtension ), 0, (char **)&_ZptrtabSTRONG[88] },
	{ OBJECT, OFFSET(SET_OF_SECExtension , element), _ZSECExtensionSTRONG, 0, (char **)&_ZptrtabSTRONG[88] },
	{ PE_END, OFFSET(SET_OF_SECExtension , next), 0, 0, (char **)&_ZptrtabSTRONG[88] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[88] }
	};

static ptpe dt_SECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[89] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[89] },
	{ MEMALLOC, 0, sizeof (SECExtension ), 0, (char **)&_ZptrtabSTRONG[89] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[90] },
	{ INTEGER, OFFSET(SECExtension , ext_id), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[90] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[91] },
	{ BOOLEAN, OFFSET(SECExtension , ext_critical), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[91] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[91] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[92] },
	{ ANY, OFFSET(SECExtension , ext_item), -1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[92] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[89] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[89] }
	};

static ptpe dt_SETOFAttrAttrTypeCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[93] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[93] },
	{ MEMALLOC, 0, sizeof (SET_OF_AttrAttrTypeCHOICE ), 0, (char **)&_ZptrtabSTRONG[93] },
	{ OBJECT, OFFSET(SET_OF_AttrAttrTypeCHOICE , element), _ZAttrAttrTypeCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[93] },
	{ PE_END, OFFSET(SET_OF_AttrAttrTypeCHOICE , next), 0, 0, (char **)&_ZptrtabSTRONG[93] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[93] }
	};

static ptpe dt_AttrAttrTypeCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] },
	{ MEMALLOC, 0, sizeof (AttrAttrTypeCHOICE ), 0, (char **)&_ZptrtabSTRONG[94] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[94] },
	{ OBJID, OFFSET(AttrAttrTypeCHOICE , choice_un.choice_un_attrtype), 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[94] },
	{ OBJECT, OFFSET(AttrAttrTypeCHOICE , choice_un.choice_un_attr), _ZTAttributeSTRONG, 0, (char **)&_ZptrtabSTRONG[94] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] }
	};

static ptpe dt_EntryInfoSELSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[95] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[95] },
	{ MEMALLOC, 0, sizeof (EntryInfoSEL ), 0, (char **)&_ZptrtabSTRONG[95] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[96]},
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[96] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[96] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[97] },
	{ T_NULL, 0, 5, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[97] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[98] },
	{ EXTOBJ, OFFSET(EntryInfoSEL , eis_select), _ZSETOFAttributeTypeSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[98] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[98] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[96] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[99] },
	{ INTEGER, OFFSET(EntryInfoSEL , eis_infotypes), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[99] },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[99] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[95] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[95] }
	};

static ptpe dt_SETOFEntryINFOSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[100] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[100] },
	{ MEMALLOC, 0, sizeof (SET_OF_EntryINFO ), 0, (char **)&_ZptrtabSTRONG[100] },
	{ OBJECT, OFFSET(SET_OF_EntryINFO , element), _ZEntryINFOSTRONG, 0, (char **)&_ZptrtabSTRONG[100] },
	{ PE_END, OFFSET(SET_OF_EntryINFO , next), 0, 0, (char **)&_ZptrtabSTRONG[100] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[100] }
	};

static ptpe dt_EntryINFOSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[101] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[101] },
	{ MEMALLOC, 0, sizeof (EntryINFO ), 0, (char **)&_ZptrtabSTRONG[101] },
	{ EXTOBJ, OFFSET(EntryINFO , ent_dn), _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[14] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[14] },
	{ BOOLEAN, OFFSET(EntryINFO , ent_fromentry), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ OBJECT, OFFSET(EntryINFO , ent_attr), _ZSETOFAttrAttrTypeCHOICESTRONG, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[101] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[101] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[101] }
	};

static ptpe dt_StringsCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] },
	{ MEMALLOC, 0, sizeof (StringsCHOICE ), 0, (char **)&_ZptrtabSTRONG[102] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[102] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[103] },
	{ ANY, OFFSET(StringsCHOICE , strings_un.initial), -1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[103] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[104] },
	{ ANY, OFFSET(StringsCHOICE , strings_un.any), -1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[104] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[105] },
	{ ANY, OFFSET(StringsCHOICE , strings_un.final), -1, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[105] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] }
	};

static ptpe dt_SEQUENCEOFStringsCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[106] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[106] },
	{ MEMALLOC, 0, sizeof (SEQUENCE_OF_StringsCHOICE ), 0, (char **)&_ZptrtabSTRONG[106] },
	{ OBJECT, OFFSET(SEQUENCE_OF_StringsCHOICE , element), _ZStringsCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[106] },
	{ PE_END, OFFSET(SEQUENCE_OF_StringsCHOICE , next), 0, 0, (char **)&_ZptrtabSTRONG[106] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[106] }
	};

static ptpe dt_FilterSubstringsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[107] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[107] },
	{ MEMALLOC, 0, sizeof (SFilterSubstrings ), 0, (char **)&_ZptrtabSTRONG[107] },
	{ OBJID, OFFSET(SFilterSubstrings , type), 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[108] },
	{ OBJECT, OFFSET(SFilterSubstrings , seq), _ZSEQUENCEOFStringsCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[109] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[107] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[107] }
	};

static ptpe dt_FilterItemSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] },
	{ MEMALLOC, 0, sizeof (SFilterItem ), 0, (char **)&_ZptrtabSTRONG[110] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[110] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[111] },
	{ EXTOBJ, OFFSET(SFilterItem , fi_un.fi_un_ava), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[111] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[111] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[112] },
	{ OBJECT, OFFSET(SFilterItem , fi_un.fi_un_substrings), _ZFilterSubstringsSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[112] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[113] },
	{ EXTOBJ, OFFSET(SFilterItem , fi_un.fi_un_ava), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[113] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[113] },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[114] },
	{ EXTOBJ, OFFSET(SFilterItem , fi_un.fi_un_ava), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[114] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[114] },
	{ ETAG, 0, 4, FL_CONTEXT, (char **)&_ZptrtabSTRONG[115] },
	{ OBJID, OFFSET(SFilterItem , fi_un.fi_un_type), 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[115] },
	{ ETAG, 0, 5, FL_CONTEXT, (char **)&_ZptrtabSTRONG[116] },
	{ EXTOBJ, OFFSET(SFilterItem , fi_un.fi_un_ava), _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[116] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[116] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] }
	};

static ptpe dt_SETOFFilterSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[117] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[117] },
	{ MEMALLOC, 0, sizeof (SET_OF_SFilter ), 0, (char **)&_ZptrtabSTRONG[117] },
	{ OBJECT, OFFSET(SET_OF_SFilter , element), _ZFilterSTRONG, 0, (char **)&_ZptrtabSTRONG[117] },
	{ PE_END, OFFSET(SET_OF_SFilter , next), 0, 0, (char **)&_ZptrtabSTRONG[117] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[117] }
	};

static ptpe dt_FilterSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] },
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] },
	{ MEMALLOC, 0, sizeof (SFilter ), 0, (char **)&_ZptrtabSTRONG[118] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[118] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[92] },
	{ OBJECT, OFFSET(SFilter , flt_un.flt_un_item), _ZFilterItemSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[92] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[119] },
	{ OBJECT, OFFSET(SFilter , flt_un.flt_un_filterset), _ZSETOFFilterSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[119] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[120] },
	{ OBJECT, OFFSET(SFilter , flt_un.flt_un_filterset), _ZSETOFFilterSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[120] },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[121] },
	{ OBJECT, OFFSET(SFilter , flt_un.flt_un_filter), _ZFilterSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[121] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] }
	};

static ptpe dt_SETOFSubordEntrySTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[122] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[122] },
	{ MEMALLOC, 0, sizeof (SET_OF_SubordEntry ), 0, (char **)&_ZptrtabSTRONG[122] },
	{ OBJECT, OFFSET(SET_OF_SubordEntry , element), _ZSubordEntrySTRONG, 0, (char **)&_ZptrtabSTRONG[122] },
	{ PE_END, OFFSET(SET_OF_SubordEntry , next), 0, 0, (char **)&_ZptrtabSTRONG[122] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[122] }
	};

static ptpe dt_SubordEntrySTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[123] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[123] },
	{ MEMALLOC, 0, sizeof (SubordEntry ), 0, (char **)&_ZptrtabSTRONG[123] },
	{ EXTOBJ, OFFSET(SubordEntry , sub_rdn), _ZRelativeDistinguishedNameSECIF, 0, (char **)&_ZptrtabSTRONG[123] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[123] },
	{ ETAG, 0, 0, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[124] },
	{ BOOLEAN, OFFSET(SubordEntry , sub_aliasentry), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[124] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[124] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ BOOLEAN, OFFSET(SubordEntry , sub_copy), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[123] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[123] }
	};

static ptpe dt_ListInfoSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[125] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[125] },
	{ MEMALLOC, 0, sizeof (ListInfo ), 0, (char **)&_ZptrtabSTRONG[125] },
	{ EXTOBJ, OFFSET(ListInfo , lsr_object), _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[125] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[125] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[126] },
	{ OBJECT, OFFSET(ListInfo , lsr_subordinates), _ZSETOFSubordEntrySTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[126] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[127]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[127] },
	{ OBJECT, OFFSET(ListInfo , lsr_poq), _ZPartialOutcomeQualifierSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[127] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[34]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, OFFSET(ListInfo , lsr_common), 1, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[7] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[35]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, OFFSET(ListInfo , lsr_common), _ZTPerformerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[35] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(ListInfo , lsr_common), _ZTSecParmResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[125] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[125] }
	};

static ptpe dt_SearchInfoSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[128] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[128] },
	{ MEMALLOC, 0, sizeof (SearchInfo ), 0, (char **)&_ZptrtabSTRONG[128] },
	{ EXTOBJ, OFFSET(SearchInfo , srr_object), _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[128] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[128] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[129] },
	{ OBJECT, OFFSET(SearchInfo , srr_entries), _ZSETOFEntryINFOSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[129] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[127]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[127] },
	{ OBJECT, OFFSET(SearchInfo , srr_poq), _ZPartialOutcomeQualifierSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[127] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[34]},
	{ ETAG, 0, 28, FL_CONTEXT, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, OFFSET(SearchInfo , srr_common), 1, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[7] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[35]},
	{ ETAG, 0, 29, FL_CONTEXT, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, OFFSET(SearchInfo , srr_common), _ZTPerformerSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[35] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[25]},
	{ ETAG, 0, 30, FL_CONTEXT, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, OFFSET(SearchInfo , srr_common), _ZTSecParmResultSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[128] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[128] }
	};

static ptpe dt_SETOFDNameSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[130] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[130] },
	{ MEMALLOC, 0, sizeof (SET_OF_DName ), 0, (char **)&_ZptrtabSTRONG[130] },
	{ EXTOBJ, OFFSET(SET_OF_DName , element), _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[130] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[130] },
	{ PE_END, OFFSET(SET_OF_DName , next), 0, 0, (char **)&_ZptrtabSTRONG[130] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[130] }
	};

static ptpe dt_PartialOutcomeQualifierSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[131] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[131] },
	{ MEMALLOC, 0, sizeof (PartialOutQual ), 0, (char **)&_ZptrtabSTRONG[131] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[132]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[132] },
	{ INTEGER, OFFSET(PartialOutQual , poq_limitproblem), 2, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[136] },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[133] },
	{ OBJECT, OFFSET(PartialOutQual , poq_cref), _ZSETOFContinuationReferenceSTRONG, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[133] },
	{ ETAG, 0, 2, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSTRONG[134] },
	{ BOOLEAN, OFFSET(PartialOutQual , poq_no_ext), 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[134] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[134] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[131] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[131] }
	};

static ptpe dt_ReferenceTypeSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[135] },
	{ MEMALLOC, 0, sizeof (struct type_STRONG_ReferenceType), 0, (char **)&_ZptrtabSTRONG[135] },
	{ INTEGER, OFFSET(struct type_STRONG_ReferenceType, parm), 10, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[135] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[135] }
	};

static ptpe dt_LimitProblemSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[136] },
	{ MEMALLOC, 0, sizeof (struct type_STRONG_LimitProblem), 0, (char **)&_ZptrtabSTRONG[136] },
	{ INTEGER, OFFSET(struct type_STRONG_LimitProblem, parm), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[136] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[136] }
	};

static ptpe dt_SETOFContinuationReferenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[137] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[137] },
	{ MEMALLOC, 0, sizeof (SET_OF_ContReference ), 0, (char **)&_ZptrtabSTRONG[137] },
	{ OBJECT, OFFSET(SET_OF_ContReference , element), _ZContinuationReferenceSTRONG, 0, (char **)&_ZptrtabSTRONG[137] },
	{ PE_END, OFFSET(SET_OF_ContReference , next), 0, 0, (char **)&_ZptrtabSTRONG[137] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[137] }
	};

static ptpe dt_ContinuationReferenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[138] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[138] },
	{ MEMALLOC, 0, sizeof (ContReference ), 0, (char **)&_ZptrtabSTRONG[138] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[139] },
	{ EXTOBJ, OFFSET(ContReference , cr_name), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[139] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[139] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[22]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[22] },
	{ INTEGER, OFFSET(ContReference , cr_aliasedRDNs), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[22] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[140] },
	{ OBJECT, OFFSET(ContReference , cr_progress), _ZOperationProgressSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[140] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[141]},
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[141] },
	{ INTEGER, OFFSET(ContReference , cr_rdn_resolved), 2, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[141] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[142]},
	{ ETAG, 0, 4, FL_CONTEXT, (char **)&_ZptrtabSTRONG[142] },
	{ INTEGER, OFFSET(ContReference , cr_reftype), 10, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSTRONG[135] },
	{ ETAG, 0, 5, FL_CONTEXT, (char **)&_ZptrtabSTRONG[143] },
	{ OBJECT, OFFSET(ContReference , cr_accesspoints), _ZSETOFAccessPointSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[143] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[138] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[138] }
	};

static ptpe dt_SETOFAccessPointSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[144] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[144] },
	{ MEMALLOC, 0, sizeof (SET_OF_AccessPoint ), 0, (char **)&_ZptrtabSTRONG[144] },
	{ OBJECT, OFFSET(SET_OF_AccessPoint , element), _ZAccessPointSTRONG, 0, (char **)&_ZptrtabSTRONG[144] },
	{ PE_END, OFFSET(SET_OF_AccessPoint , next), 0, 0, (char **)&_ZptrtabSTRONG[144] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[144] }
	};

static ptpe dt_AccessPointSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[145] },
	{ SSET_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[145] },
	{ MEMALLOC, 0, sizeof (AccessPoint ), 0, (char **)&_ZptrtabSTRONG[145] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[145] },
	{ EXTOBJ, OFFSET(AccessPoint , ap_name), _ZNameSECIF, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[145] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSTRONG[145] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[145] },
	{ OBJECT, OFFSET(AccessPoint , ap_address), _ZPSAPaddrSTRONG, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[145] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[145] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[145] }
	};

static ptpe dt_PSAPaddrSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[146] },
	{ UCODE, 118, 0, 0, (char **)&_ZptrtabSTRONG[146] }, /* line 1463 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[146] },
	{ MEMALLOC, 0, sizeof (typeDSE_PSAPaddr ), 0, (char **)&_ZptrtabSTRONG[146] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[147]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[147] },
	{ OCTET_PTR, OFFSET(typeDSE_PSAPaddr , pSelector.octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[147] },
	{ OCTET_LEN, OFFSET(typeDSE_PSAPaddr , pSelector.noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[147] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[148]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[148] },
	{ OCTET_PTR, OFFSET(typeDSE_PSAPaddr , sSelector.octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[148] },
	{ OCTET_LEN, OFFSET(typeDSE_PSAPaddr , sSelector.noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[148] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSTRONG[149]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[149] },
	{ OCTET_PTR, OFFSET(typeDSE_PSAPaddr , tSelector.octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[149] },
	{ OCTET_LEN, OFFSET(typeDSE_PSAPaddr , tSelector.noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[149] },
	{ ETAG, 0, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[150] },
	{ EXTOBJ, OFFSET(typeDSE_PSAPaddr , nAddress), _ZSETOFOctetStringSEC, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[150] },
	{ EXTMOD, 13, 0, 0, (char **)&_ZptrtabSTRONG[150] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[146] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[146] }
	};

static ptpe pt_TSETOFSECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[0] },
	{ OBJECT, 0, _ZSETOFSECExtensionSTRONG, 0, (char **)&_ZptrtabSTRONG[88] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[0] }
	};

static ptpe pt_TIntegerSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[1] },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[1] }
	};

static ptpe pt_TOperationProgressSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[2] },
	{ OBJECT, 0, _ZOperationProgressSTRONG, 0, (char **)&_ZptrtabSTRONG[85] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[2] }
	};

static ptpe pt_TRequestorSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[3] },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[151] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[3] }
	};

static ptpe pt_TSecParmArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[5] },
	{ OBJECT, 0, _ZSecurityParametersSTRONG, 0, (char **)&_ZptrtabSTRONG[81] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[5] }
	};

static ptpe pt_TServiceControlsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[6] },
	{ OBJECT, 0, _ZServiceControlsSTRONG, 0, (char **)&_ZptrtabSTRONG[75] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[6] }
	};

static ptpe pt_TBooleanSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[7] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[7] }
	};

static ptpe pt_TPerformerSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[8] },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[151] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[8] }
	};

static ptpe pt_TSecParmResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[9] },
	{ OBJECT, 0, _ZSecurityParametersSTRONG, 0, (char **)&_ZptrtabSTRONG[81] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[9] }
	};

static ptpe pt_TAttributeSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[10] },
	{ EXTOBJ, 0, _ZAttributeSECIF, 0, (char **)&_ZptrtabSTRONG[152] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[10] }
	};

static ptpe pt_TokenTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[11] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[12] },
	{ EXTOBJ, 0, _ZAlgorithmIdentifierSEC, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[14] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[15] },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[16] },
	{ EXTOBJ, 0, _ZBitStringSEC, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[11] }
	};

static ptpe pt_TokenSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZTokenTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[11] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[17] }
	};

static ptpe pt_AddArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[18] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[20] },
	{ EXTOBJ, 0, _ZSETOFAttributeSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 25, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, 0, _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 26, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, 0, _ZTIntegerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 27, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, 0, _ZTOperationProgressSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, 0, _ZTRequestorSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmArgumentSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, 0, _ZTServiceControlsSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[18] }
	};

static ptpe pt_AddArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZAddArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[18] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAddArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[18] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[27] }
	};

static ptpe pt_CompareArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[28] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[29] },
	{ EXTOBJ, 0, _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 25, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, 0, _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 26, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, 0, _ZTIntegerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 27, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, 0, _ZTOperationProgressSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, 0, _ZTRequestorSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmArgumentSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, 0, _ZTServiceControlsSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[28] }
	};

static ptpe pt_CompareArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZCompareArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[28] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCompareArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[28] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[30] }
	};

static ptpe pt_CompareResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[31] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[151] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[32] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[33] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, 0, _ZTPerformerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmResultSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[31] }
	};

static ptpe pt_CompareResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZCompareResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[31] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCompareResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[31] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[36] }
	};

static ptpe pt_ListArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[37] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 25, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, 0, _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 26, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, 0, _ZTIntegerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 27, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, 0, _ZTOperationProgressSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, 0, _ZTRequestorSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmArgumentSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, 0, _ZTServiceControlsSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[37] }
	};

static ptpe pt_ListArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZListArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[37] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZListArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[37] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[38] }
	};

static ptpe pt_ListResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZListInfoSTRONG, 0, (char **)&_ZptrtabSTRONG[40] },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[41] },
	{ OBJECT, 0, _ZSETOFListResultSTRONG, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[39] }
	};

static ptpe pt_ListResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZListResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[39] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZListResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[39] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[42] }
	};

static ptpe pt_SETOFListResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[43] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZListResultSTRONG, 0, (char **)&_ZptrtabSTRONG[42] },
	{ PE_END, OFFSET(SET_OF_ListResult , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[43] }
	};

static ptpe pt_ModifyEntryArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[44] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[45] },
	{ OBJECT, 0, _ZEntryModificationSequenceSTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 25, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, 0, _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 26, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, 0, _ZTIntegerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 27, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, 0, _ZTOperationProgressSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, 0, _ZTRequestorSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmArgumentSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, 0, _ZTServiceControlsSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[44] }
	};

static ptpe pt_ModifyEntryArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZModifyEntryArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[44] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZModifyEntryArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[44] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[46] }
	};

static ptpe pt_ModifyRDNArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[47] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[48] },
	{ EXTOBJ, 0, _ZRelativeDistinguishedNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[49] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[49] },
	{ ETAG, -1, 25, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, 0, _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 26, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, 0, _ZTIntegerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 27, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, 0, _ZTOperationProgressSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, 0, _ZTRequestorSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmArgumentSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, 0, _ZTServiceControlsSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[47] }
	};

static ptpe pt_ModifyRDNArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZModifyRDNArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[47] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZModifyRDNArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[47] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[50] }
	};

static ptpe pt_ReadArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[51] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[52] },
	{ OBJECT, 0, _ZEntryInfoSELSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 25, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, 0, _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 26, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, 0, _ZTIntegerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 27, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, 0, _ZTOperationProgressSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, 0, _ZTRequestorSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmArgumentSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, 0, _ZTServiceControlsSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[51] }
	};

static ptpe pt_ReadArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZReadArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[51] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZReadArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[51] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[53] }
	};

static ptpe pt_ReadResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[54] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[20] },
	{ OBJECT, 0, _ZEntryINFOSTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, 0, _ZTPerformerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmResultSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[54] }
	};

static ptpe pt_ReadResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZReadResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[54] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZReadResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[54] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[55] }
	};

static ptpe pt_RemoveArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[56] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[19] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 25, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, 0, _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 26, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, 0, _ZTIntegerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 27, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, 0, _ZTOperationProgressSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, 0, _ZTRequestorSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmArgumentSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, 0, _ZTServiceControlsSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[56] }
	};

static ptpe pt_RemoveArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZRemoveArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[56] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZRemoveArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[56] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[57] }
	};

static ptpe pt_SearchArgumentTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[58] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[59] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[60] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[60] },
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[61] },
	{ OBJECT, 0, _ZFilterSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[62] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[62] },
	{ ETAG, -1, 4, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[52] },
	{ OBJECT, 0, _ZEntryInfoSELSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 25, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[21] },
	{ OBJECT, 0, _ZTSETOFSECExtensionSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 26, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ OBJECT, 0, _ZTIntegerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 27, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[23] },
	{ OBJECT, 0, _ZTOperationProgressSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[24] },
	{ OBJECT, 0, _ZTRequestorSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmArgumentSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[26] },
	{ OBJECT, 0, _ZTServiceControlsSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[58] }
	};

static ptpe pt_SearchArgumentSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZSearchArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[58] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZSearchArgumentTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[58] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[63] }
	};

static ptpe pt_SearchResultTBSSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZSearchInfoSTRONG, 0, (char **)&_ZptrtabSTRONG[65] },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[66] },
	{ OBJECT, 0, _ZSETOFSearchResultSTRONG, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[64] }
	};

static ptpe pt_SearchResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJECT, 0, _ZSearchResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[64] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZSearchResultTBSSTRONG, 0, (char **)&_ZptrtabSTRONG[64] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSTRONG[153] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSTRONG[154] },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[67] }
	};

static ptpe pt_SETOFSearchResultSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[68] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZSearchResultSTRONG, 0, (char **)&_ZptrtabSTRONG[67] },
	{ PE_END, OFFSET(SET_OF_SearchResult , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[68] }
	};

static ptpe pt_EntryModificationSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[70] },
	{ EXTOBJ, 0, _ZAttributeSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[71] },
	{ OBJID, -1, 6, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[72] },
	{ EXTOBJ, 0, _ZAttributeSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[73] },
	{ EXTOBJ, 0, _ZAttributeSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[69] }
	};

static ptpe pt_EntryModificationSequenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[74] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZEntryModificationSTRONG, 0, (char **)&_ZptrtabSTRONG[69] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[74] }
	};

static ptpe pt_ServiceControlsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[75] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[76] },
	{ BITSTR_PTR, 155, 3, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ BITSTR_LEN, 155, 3, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[77] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[77] },
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[78] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[79] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 4, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[80] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[75] }
	};

static ptpe pt_SecurityParametersSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[81] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[82] },
	{ EXTOBJ, 0, _ZCertificationPathSECAF, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ EXTMOD, 83, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[14] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[15] },
	{ T_STRING, -1, 23, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[16] },
	{ EXTOBJ, 0, _ZBitStringSEC, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ ETAG, -1, 4, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[84] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[84] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[81] }
	};

static ptpe pt_OperationProgressSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[85] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[86] },
	{ INTEGER, -1, 10, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[87] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[85] }
	};

static ptpe pt_SETOFSECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[88] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZSECExtensionSTRONG, 0, (char **)&_ZptrtabSTRONG[89] },
	{ PE_END, OFFSET(SET_OF_SECExtension , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[88] }
	};

static ptpe pt_SECExtensionSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[89] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[90] },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[91] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[91] },
	{ ETAG, -1, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[92] },
	{ ANY, -1, -1, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[89] }
	};

static ptpe pt_SETOFAttrAttrTypeCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[93] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAttrAttrTypeCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[94] },
	{ PE_END, OFFSET(SET_OF_AttrAttrTypeCHOICE , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[93] }
	};

static ptpe pt_AttrAttrTypeCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ OBJID, -1, 6, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZTAttributeSTRONG, 0, (char **)&_ZptrtabSTRONG[10] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[94] }
	};

static ptpe pt_EntryInfoSELSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[95] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ SCHOICE_START, -1, 0, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[96] },
	{ SCTRL, 0, 0, 0, (char **)&_ZptrtabSTRONG[96] },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[97] },
	{ T_NULL, -1, 5, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[98] },
	{ EXTOBJ, 0, _ZSETOFAttributeTypeSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[99] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[99] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[95] }
	};

static ptpe pt_SETOFEntryINFOSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[100] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZEntryINFOSTRONG, 0, (char **)&_ZptrtabSTRONG[101] },
	{ PE_END, OFFSET(SET_OF_EntryINFO , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[100] }
	};

static ptpe pt_EntryINFOSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[101] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[14] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSTRONG[33] },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ OBJECT, 0, _ZSETOFAttrAttrTypeCHOICESTRONG, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[93] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[101] }
	};

static ptpe pt_StringsCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[103] },
	{ ANY, -1, -1, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[104] },
	{ ANY, -1, -1, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[105] },
	{ ANY, -1, -1, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[102] }
	};

static ptpe pt_SEQUENCEOFStringsCHOICESTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[106] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZStringsCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[102] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[106] }
	};

static ptpe pt_FilterSubstringsSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[107] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJID, -1, 6, FL_UNIVERSAL, (char **)&_ZptrtabSTRONG[108] },
	{ OBJECT, 0, _ZSEQUENCEOFStringsCHOICESTRONG, 0, (char **)&_ZptrtabSTRONG[109] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[107] }
	};

static ptpe pt_FilterItemSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[111] },
	{ EXTOBJ, 0, _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[112] },
	{ OBJECT, 0, _ZFilterSubstringsSTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[113] },
	{ EXTOBJ, 0, _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[114] },
	{ EXTOBJ, 0, _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 4, FL_CONTEXT, (char **)&_ZptrtabSTRONG[115] },
	{ OBJID, -1, 6, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 5, FL_CONTEXT, (char **)&_ZptrtabSTRONG[116] },
	{ EXTOBJ, 0, _ZAttributeValueAssertionSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[110] }
	};

static ptpe pt_SETOFFilterSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[117] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZFilterSTRONG, 0, (char **)&_ZptrtabSTRONG[118] },
	{ PE_END, OFFSET(SET_OF_SFilter , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[117] }
	};

static ptpe pt_FilterSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] },
	{ SCHOICE_START, -1, 0, 0, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[92] },
	{ OBJECT, 0, _ZFilterItemSTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[119] },
	{ OBJECT, 0, _ZSETOFFilterSTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[120] },
	{ OBJECT, 0, _ZSETOFFilterSTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[121] },
	{ OBJECT, 0, _ZFilterSTRONG, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[118] }
	};

static ptpe pt_SETOFSubordEntrySTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[122] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZSubordEntrySTRONG, 0, (char **)&_ZptrtabSTRONG[123] },
	{ PE_END, OFFSET(SET_OF_SubordEntry , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[122] }
	};

static ptpe pt_SubordEntrySTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[123] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZRelativeDistinguishedNameSECIF, 0, (char **)&_ZptrtabSTRONG[156] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[124] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[124] },
	{ ETAG, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[33] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	1,	0,	0, (char **)&_ZptrtabSTRONG[33] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[123] }
	};

static ptpe pt_ListInfoSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[125] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[151] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[126] },
	{ OBJECT, 0, _ZSETOFSubordEntrySTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[127] },
	{ OBJECT, 0, _ZPartialOutcomeQualifierSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, 0, _ZTPerformerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmResultSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[125] }
	};

static ptpe pt_SearchInfoSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[128] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, FL_OPTIONAL, (char **)&_ZptrtabSTRONG[151] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[129] },
	{ OBJECT, 0, _ZSETOFEntryINFOSTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[127] },
	{ OBJECT, 0, _ZPartialOutcomeQualifierSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 28, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[34] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[35] },
	{ OBJECT, 0, _ZTPerformerSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[25] },
	{ OBJECT, 0, _ZTSecParmResultSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[128] }
	};

static ptpe pt_SETOFDNameSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[130] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSTRONG[151] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ PE_END, OFFSET(SET_OF_DName , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[130] }
	};

static ptpe pt_PartialOutcomeQualifierSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[131] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[132] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[133] },
	{ OBJECT, 0, _ZSETOFContinuationReferenceSTRONG, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSTRONG[134] },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSTRONG[134] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[131] }
	};

static ptpe pt_ReferenceTypeSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[135] },
	{ INTEGER, -1, 10, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[135] }
	};

static ptpe pt_LimitProblemSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[136] },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[136] }
	};

static ptpe pt_SETOFContinuationReferenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[137] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZContinuationReferenceSTRONG, 0, (char **)&_ZptrtabSTRONG[138] },
	{ PE_END, OFFSET(SET_OF_ContReference , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[137] }
	};

static ptpe pt_ContinuationReferenceSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[138] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[139] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[22] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT, (char **)&_ZptrtabSTRONG[140] },
	{ OBJECT, 0, _ZOperationProgressSTRONG, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[141] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 4, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[142] },
	{ INTEGER, -1, 10, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 5, FL_CONTEXT, (char **)&_ZptrtabSTRONG[143] },
	{ OBJECT, 0, _ZSETOFAccessPointSTRONG, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[138] }
	};

static ptpe pt_SETOFAccessPointSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[144] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAccessPointSTRONG, 0, (char **)&_ZptrtabSTRONG[145] },
	{ PE_END, OFFSET(SET_OF_AccessPoint , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[144] }
	};

static ptpe pt_AccessPointSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[145] },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSTRONG[151] },
	{ EXTOBJ, 0, _ZNameSECIF, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSTRONG[146] },
	{ OBJECT, 0, _ZPSAPaddrSTRONG, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[145] }
	};

static ptpe pt_PSAPaddrSTRONG[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSTRONG[146] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[147] },
	{ OCTET_PTR, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ OCTET_LEN, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[148] },
	{ OCTET_PTR, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ OCTET_LEN, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSTRONG[149] },
	{ OCTET_PTR, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ OCTET_LEN, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 3, FL_CONTEXT, (char **)&_ZptrtabSTRONG[150] },
	{ EXTOBJ, 0, _ZSETOFOctetStringSEC, FL_UNIVERSAL, NULLVP },
	{ EXTMOD, 13, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSTRONG[146] }
	};


/*VARARGS*/
static	int
efn_STRONG(__p, ppe, _Zp)
caddr_t	__p;
PE	*ppe;
ptpe	*_Zp;
{
		
	/* 101 cases */
    switch(_Zp->pe_ucode) {

#define parm	((AddArgumentTBS * )__p)
	case 0: /*  */
		{
# line 147 "strong.py"
		return (parm->ada_common->ext != (SET_OF_SECExtension *) 0);

		}
	    break;

#undef parm

#define parm	((AddArgumentTBS * )__p)
	case 1: /*  */
		{
# line 151 "strong.py"
		return (parm->ada_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED);

		}
	    break;

#undef parm

#define parm	((AddArgumentTBS * )__p)
	case 2: /*  */
		{
# line 155 "strong.py"
		return (parm->ada_common->progress != (OperationProgress *) 0);

		}
	    break;

#undef parm

#define parm	((AddArgumentTBS * )__p)
	case 3: /*  */
		{
# line 159 "strong.py"
		return (parm->ada_common->requestor != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((AddArgumentTBS * )__p)
	case 4: /*  */
		{
# line 164 "strong.py"
		return (parm->ada_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((AddArgumentTBS * )__p)
	case 5: /*  */
		{
# line 169 "strong.py"
		return (parm->ada_common->svc != (ServiceControls *) 0);

		}
	    break;

#undef parm

#define parm	((AddArgument * )__p)
	case 6: /*  */
		{
# line 182 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS * )__p)
	case 7: /*  */
		{
# line 218 "strong.py"
		return (parm->cma_common->ext != (SET_OF_SECExtension *) 0);

		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS * )__p)
	case 8: /*  */
		{
# line 222 "strong.py"
		return (parm->cma_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED);

		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS * )__p)
	case 9: /*  */
		{
# line 226 "strong.py"
		return (parm->cma_common->progress != (OperationProgress *) 0);

		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS * )__p)
	case 10: /*  */
		{
# line 230 "strong.py"
		return (parm->cma_common->requestor != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS * )__p)
	case 11: /*  */
		{
# line 235 "strong.py"
		return (parm->cma_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS * )__p)
	case 12: /*  */
		{
# line 240 "strong.py"
		return (parm->cma_common->svc != (ServiceControls *) 0);

		}
	    break;

#undef parm

#define parm	((CompareArgument * )__p)
	case 13: /*  */
		{
# line 253 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((CompareResultTBS * )__p)
	case 14: /*  */
		{
# line 294 "strong.py"
		return (parm->cmr_common->aliasDereferenced != FALSE);

		}
	    break;

#undef parm

#define parm	((CompareResultTBS * )__p)
	case 15: /*  */
		{
# line 298 "strong.py"
		return (parm->cmr_common->performer != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((CompareResultTBS * )__p)
	case 16: /*  */
		{
# line 302 "strong.py"
		return (parm->cmr_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((CompareResult * )__p)
	case 17: /*  */
		{
# line 315 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((ListArgumentTBS * )__p)
	case 18: /*  */
		{
# line 348 "strong.py"
		return (parm->lsa_common->ext != (SET_OF_SECExtension *) 0);

		}
	    break;

#undef parm

#define parm	((ListArgumentTBS * )__p)
	case 19: /*  */
		{
# line 352 "strong.py"
		return (parm->lsa_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED);

		}
	    break;

#undef parm

#define parm	((ListArgumentTBS * )__p)
	case 20: /*  */
		{
# line 356 "strong.py"
		return (parm->lsa_common->progress != (OperationProgress *) 0);

		}
	    break;

#undef parm

#define parm	((ListArgumentTBS * )__p)
	case 21: /*  */
		{
# line 360 "strong.py"
		return (parm->lsa_common->requestor != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((ListArgumentTBS * )__p)
	case 22: /*  */
		{
# line 365 "strong.py"
		return (parm->lsa_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((ListArgumentTBS * )__p)
	case 23: /*  */
		{
# line 370 "strong.py"
		return (parm->lsa_common->svc != (ServiceControls *) 0);

		}
	    break;

#undef parm

#define parm	((ListArgument * )__p)
	case 24: /*  */
		{
# line 383 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((ListResultTBS * )__p)
	case 25: /*  */
		{
# line 409 "strong.py"
		return (parm->lsr_type);

		}
	    break;

#undef parm

#define parm	((ListResult * )__p)
	case 26: /*  */
		{
# line 430 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS * )__p)
	case 27: /*  */
		{
# line 472 "strong.py"
		return (parm->mea_common->ext != (SET_OF_SECExtension *) 0);

		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS * )__p)
	case 28: /*  */
		{
# line 476 "strong.py"
		return (parm->mea_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED);

		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS * )__p)
	case 29: /*  */
		{
# line 480 "strong.py"
		return (parm->mea_common->progress != (OperationProgress *) 0);

		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS * )__p)
	case 30: /*  */
		{
# line 484 "strong.py"
		return (parm->mea_common->requestor != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS * )__p)
	case 31: /*  */
		{
# line 489 "strong.py"
		return (parm->mea_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS * )__p)
	case 32: /*  */
		{
# line 494 "strong.py"
		return (parm->mea_common->svc != (ServiceControls *) 0);

		}
	    break;

#undef parm

#define parm	((ModifyEntryArgument * )__p)
	case 33: /*  */
		{
# line 507 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS * )__p)
	case 34: /*  */
		{
# line 547 "strong.py"
		return (parm->mra_common->ext != (SET_OF_SECExtension *) 0);

		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS * )__p)
	case 35: /*  */
		{
# line 551 "strong.py"
		return (parm->mra_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED);

		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS * )__p)
	case 36: /*  */
		{
# line 555 "strong.py"
		return (parm->mra_common->progress != (OperationProgress *) 0);

		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS * )__p)
	case 37: /*  */
		{
# line 559 "strong.py"
		return (parm->mra_common->requestor != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS * )__p)
	case 38: /*  */
		{
# line 564 "strong.py"
		return (parm->mra_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS * )__p)
	case 39: /*  */
		{
# line 569 "strong.py"
		return (parm->mra_common->svc != (ServiceControls *) 0);

		}
	    break;

#undef parm

#define parm	((ModifyRDNArgument * )__p)
	case 40: /*  */
		{
# line 582 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS * )__p)
	case 41: /*  */
		{
# line 620 "strong.py"
		return (parm->rda_common->ext != (SET_OF_SECExtension *) 0);

		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS * )__p)
	case 42: /*  */
		{
# line 624 "strong.py"
		return (parm->rda_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED);

		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS * )__p)
	case 43: /*  */
		{
# line 628 "strong.py"
		return (parm->rda_common->progress != (OperationProgress *) 0);

		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS * )__p)
	case 44: /*  */
		{
# line 632 "strong.py"
		return (parm->rda_common->requestor != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS * )__p)
	case 45: /*  */
		{
# line 637 "strong.py"
		return (parm->rda_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS * )__p)
	case 46: /*  */
		{
# line 642 "strong.py"
		return (parm->rda_common->svc != (ServiceControls *) 0);

		}
	    break;

#undef parm

#define parm	((ReadArgument * )__p)
	case 47: /*  */
		{
# line 655 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((ReadResultTBS * )__p)
	case 48: /*  */
		{
# line 689 "strong.py"
		return (parm->rdr_common->aliasDereferenced != FALSE);

		}
	    break;

#undef parm

#define parm	((ReadResultTBS * )__p)
	case 49: /*  */
		{
# line 693 "strong.py"
		return (parm->rdr_common->performer != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((ReadResultTBS * )__p)
	case 50: /*  */
		{
# line 697 "strong.py"
		return (parm->rdr_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((ReadResult * )__p)
	case 51: /*  */
		{
# line 710 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS * )__p)
	case 52: /*  */
		{
# line 743 "strong.py"
		return (parm->rma_common->ext != (SET_OF_SECExtension *) 0);

		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS * )__p)
	case 53: /*  */
		{
# line 747 "strong.py"
		return (parm->rma_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED);

		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS * )__p)
	case 54: /*  */
		{
# line 751 "strong.py"
		return (parm->rma_common->progress != (OperationProgress *) 0);

		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS * )__p)
	case 55: /*  */
		{
# line 755 "strong.py"
		return (parm->rma_common->requestor != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS * )__p)
	case 56: /*  */
		{
# line 760 "strong.py"
		return (parm->rma_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS * )__p)
	case 57: /*  */
		{
# line 765 "strong.py"
		return (parm->rma_common->svc != (ServiceControls *) 0);

		}
	    break;

#undef parm

#define parm	((RemoveArgument * )__p)
	case 58: /*  */
		{
# line 778 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS * )__p)
	case 59: /*  */
		{
# line 833 "strong.py"
		return (parm->sra_common->ext != (SET_OF_SECExtension *) 0);

		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS * )__p)
	case 60: /*  */
		{
# line 837 "strong.py"
		return (parm->sra_common->aliasedRDNs != CA_NO_ALIASDEREFERENCED);

		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS * )__p)
	case 61: /*  */
		{
# line 841 "strong.py"
		return (parm->sra_common->progress != (OperationProgress *) 0);

		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS * )__p)
	case 62: /*  */
		{
# line 845 "strong.py"
		return (parm->sra_common->requestor != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS * )__p)
	case 63: /*  */
		{
# line 850 "strong.py"
		return (parm->sra_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS * )__p)
	case 64: /*  */
		{
# line 855 "strong.py"
		return (parm->sra_common->svc != (ServiceControls *) 0);

		}
	    break;

#undef parm

#define parm	((SearchArgument * )__p)
	case 65: /*  */
		{
# line 868 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((SearchResultTBS * )__p)
	case 66: /*  */
		{
# line 894 "strong.py"
		return (((parm->srr_correlated == FALSE) ? 2 : 1));

		}
	    break;

#undef parm

#define parm	((SearchResult * )__p)
	case 67: /*  */
		{
# line 913 "strong.py"
		return (parm->sig == (Signature *) 0 ? 1:2);

		}
	    break;

#undef parm

#define parm	((EntryModification * )__p)
	case 68: /*  */
		{
# line 945 "strong.py"
		return (parm->em_type);

		}
	    break;

#undef parm

#define parm	((ServiceControls * )__p)
	case 69: /*  */
		{
# line 973 "strong.py"

	if (parm->svc_options != 0) {
		parm->svc_len = 5;
		parm->svc_tmp = aux_int2strb_alloc (parm->svc_options,parm->svc_len);
	}
	;
		}
	    break;

#undef parm

#define parm	((ServiceControls * )__p)
	case 70: /*  */
		{
# line 991 "strong.py"

		if (parm->svc_tmp){
			free (parm->svc_tmp);
			parm->svc_tmp = CNULL;
		}
		;
		}
	    break;

#undef parm

#define parm	((ServiceControls * )__p)
	case 71: /*  */
		{
# line 997 "strong.py"
		return (parm->svc_options != 0);

		}
	    break;

#undef parm

#define parm	((ServiceControls * )__p)
	case 72: /*  */
		{
# line 1012 "strong.py"
		return (parm->svc_timelimit != SVC_NOTIMELIMIT);

		}
	    break;

#undef parm

#define parm	((ServiceControls * )__p)
	case 73: /*  */
		{
# line 1018 "strong.py"
		return (parm->svc_sizelimit != SVC_NOSIZELIMIT);

		}
	    break;

#undef parm

#define parm	((ServiceControls * )__p)
	case 74: /*  */
		{
# line 1028 "strong.py"
		return (parm->svc_scopeofreferral != SVC_REFSCOPE_NONE);

		}
	    break;

#undef parm

#define parm	((OperationProgress * )__p)
	case 75: /*  */
		{
# line 1065 "strong.py"

	if (parm->opResolutionPhase < 0)
		parm->opResolutionPhase = 1;
	else if (parm->opResolutionPhase > 3)
		parm->opResolutionPhase = 3;
	;
		}
	    break;

#undef parm

#define parm	((OperationProgress * )__p)
	case 76: /*  */
		{
# line 1085 "strong.py"
		return (parm->opNextRDNToBeResolved != -1);

		}
	    break;

#undef parm

#define parm	((AttrAttrTypeCHOICE * )__p)
	case 77: /*  */
		{
# line 1119 "strong.py"
		return (parm->offset);

		}
	    break;

#undef parm

#define parm	((EntryInfoSEL * )__p)
	case 78: /*  */
		{
# line 1135 "strong.py"
		return (parm->eis_allattributes ? 1 : 2);

		}
	    break;

#undef parm

#define parm	((EntryInfoSEL * )__p)
	case 79: /*  */
		{
# line 1145 "strong.py"
		return (parm->eis_allattributes != TRUE);

		}
	    break;

#undef parm

#define parm	((StringsCHOICE * )__p)
	case 80: /*  */
		{
# line 1185 "strong.py"
		return (parm->strings_type);

		}
	    break;

#undef parm

#define parm	((SFilterItem * )__p)
	case 81: /*  */
		{
# line 1221 "strong.py"
		return (parm->fi_type);

		}
	    break;

#undef parm

#define parm	((SFilter * )__p)
	case 82: /*  */
		{
# line 1252 "strong.py"
		return (parm->flt_type);

		}
	    break;

#undef parm

#define parm	((ListInfo * )__p)
	case 83: /*  */
		{
# line 1309 "strong.py"
		return (
			  (parm->lsr_poq &&
			   ( ((parm->lsr_poq->poq_limitproblem != LSR_NOLIMITPROBLEM) || 
			      (parm->lsr_poq->poq_cref != (SET_OF_ContReference * )0)) ) 
			  ));

		}
	    break;

#undef parm

#define parm	((ListInfo * )__p)
	case 84: /*  */
		{
# line 1318 "strong.py"
		return (parm->lsr_common->aliasDereferenced != FALSE);

		}
	    break;

#undef parm

#define parm	((ListInfo * )__p)
	case 85: /*  */
		{
# line 1322 "strong.py"
		return (parm->lsr_common->performer != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((ListInfo * )__p)
	case 86: /*  */
		{
# line 1326 "strong.py"
		return (parm->lsr_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((SearchInfo * )__p)
	case 87: /*  */
		{
# line 1345 "strong.py"
		return (
			  (parm->srr_poq &&
			   ( ((parm->srr_poq->poq_limitproblem != LSR_NOLIMITPROBLEM) || 
			   (parm->srr_poq->poq_cref != (SET_OF_ContReference * )0)) ) )
			 );

		}
	    break;

#undef parm

#define parm	((SearchInfo * )__p)
	case 88: /*  */
		{
# line 1354 "strong.py"
		return (parm->srr_common->aliasDereferenced != FALSE);

		}
	    break;

#undef parm

#define parm	((SearchInfo * )__p)
	case 89: /*  */
		{
# line 1358 "strong.py"
		return (parm->srr_common->performer != NULLDNAME);

		}
	    break;

#undef parm

#define parm	((SearchInfo * )__p)
	case 90: /*  */
		{
# line 1362 "strong.py"
		return (parm->srr_common->sec_parm != (SecurityParameters *) 0);

		}
	    break;

#undef parm

#define parm	((PartialOutQual * )__p)
	case 91: /*  */
		{
# line 1378 "strong.py"
		return (parm->poq_limitproblem != LSR_NOLIMITPROBLEM);

		}
	    break;

#undef parm

#define parm	((ContReference * )__p)
	case 92: /*  */
		{
# line 1427 "strong.py"
		return ( parm->cr_aliasedRDNs != CR_NOALIASEDRDNS);

		}
	    break;

#undef parm

#define parm	((ContReference * )__p)
	case 93: /*  */
		{
# line 1434 "strong.py"
		return ( parm->cr_rdn_resolved != CR_RDNRESOLVED_NOTDEFINED);

		}
	    break;

#undef parm

#define parm	((ContReference * )__p)
	case 94: /*  */
		{
# line 1438 "strong.py"
		return ( parm->cr_reftype != RT_UNDEFINED);

		}
	    break;

#undef parm

#define parm	((typeDSE_PSAPaddr * )__p)
	case 95: /*  */
		{
# line 1481 "strong.py"
		return (parm->pSelector.noctets > 0);

		}
	    break;

#undef parm

#define parm	((typeDSE_PSAPaddr * )__p)
	case 96: /*  */
		{
# line 1487 "strong.py"
		return (parm->sSelector.noctets > 0);

		}
	    break;

#undef parm

#define parm	((typeDSE_PSAPaddr * )__p)
	case 97: /*  */
		{
# line 1493 "strong.py"
		return (parm->tSelector.noctets > 0);

		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, *ppe, "enf_STRONG:Bad table entry: %d",
             _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}

/*VARARGS*/
static	int
dfn_STRONG(__p, pe, _Zp, _val)
caddr_t	__p;
PE	pe;
ptpe	*_Zp;
int _val;
{
		
	/* 126 cases */
    switch(_Zp->pe_ucode) {

#define parm	((Token ** )__p)
	case 0: /*  */
		{
# line 110 "strong.py"

        if (((*parm) = (Token *) calloc (1, sizeof (Token))) == ((Token *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
		advise (NULLCP, "out of memory");
		return NOTOK;
	}
        ;
		}
	    break;

#undef parm

#define parm	((Token ** )__p)
	case 1: /*  */
		{
# line 124 "strong.py"

		(*parm)->tbs_DERcode = e_TokenTBS((*parm)->tbs);
		;
		}
	    break;

#undef parm

#define parm	((AddArgumentTBS ** )__p)
	case 2: /*  */
		{
# line 147 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((AddArgumentTBS ** )__p)
	case 3: /*  */
		{
# line 151 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((AddArgumentTBS ** )__p)
	case 4: /*  */
		{
# line 155 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((AddArgumentTBS ** )__p)
	case 5: /*  */
		{
# line 159 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((AddArgumentTBS ** )__p)
	case 6: /*  */
		{
# line 164 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((AddArgumentTBS ** )__p)
	case 7: /*  */
		{
# line 169 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((AddArgument ** )__p)
	case 8: /*  */
		{
# line 175 "strong.py"

        if (((*parm) = (AddArgument *) calloc (1, sizeof (AddArgument))) == ((AddArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((AddArgument ** )__p)
	case 9: /*  */
		{
# line 181 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((AddArgument ** )__p)
	case 10: /*  */
		{
# line 190 "strong.py"

			(*parm)->tbs_DERcode = e_AddArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS ** )__p)
	case 11: /*  */
		{
# line 218 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS ** )__p)
	case 12: /*  */
		{
# line 222 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS ** )__p)
	case 13: /*  */
		{
# line 226 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS ** )__p)
	case 14: /*  */
		{
# line 230 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS ** )__p)
	case 15: /*  */
		{
# line 235 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareArgumentTBS ** )__p)
	case 16: /*  */
		{
# line 240 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareArgument ** )__p)
	case 17: /*  */
		{
# line 246 "strong.py"

        if (((*parm) = (CompareArgument *) calloc (1, sizeof (CompareArgument))) == ((CompareArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((CompareArgument ** )__p)
	case 18: /*  */
		{
# line 252 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((CompareArgument ** )__p)
	case 19: /*  */
		{
# line 261 "strong.py"

			(*parm)->tbs_DERcode = e_CompareArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((CompareResultTBS ** )__p)
	case 20: /*  */
		{
# line 294 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareResultTBS ** )__p)
	case 21: /*  */
		{
# line 298 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareResultTBS ** )__p)
	case 22: /*  */
		{
# line 302 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((CompareResult ** )__p)
	case 23: /*  */
		{
# line 308 "strong.py"

        if (((*parm) = (CompareResult *) calloc (1, sizeof (CompareResult))) == ((CompareResult *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((CompareResult ** )__p)
	case 24: /*  */
		{
# line 314 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((CompareResult ** )__p)
	case 25: /*  */
		{
# line 323 "strong.py"

			(*parm)->tbs_DERcode = e_CompareResultTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((ListArgumentTBS ** )__p)
	case 26: /*  */
		{
# line 348 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListArgumentTBS ** )__p)
	case 27: /*  */
		{
# line 352 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListArgumentTBS ** )__p)
	case 28: /*  */
		{
# line 356 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListArgumentTBS ** )__p)
	case 29: /*  */
		{
# line 360 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListArgumentTBS ** )__p)
	case 30: /*  */
		{
# line 365 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListArgumentTBS ** )__p)
	case 31: /*  */
		{
# line 370 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListArgument ** )__p)
	case 32: /*  */
		{
# line 376 "strong.py"

        if (((*parm) = (ListArgument *) calloc (1, sizeof (ListArgument))) == ((ListArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((ListArgument ** )__p)
	case 33: /*  */
		{
# line 382 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ListArgument ** )__p)
	case 34: /*  */
		{
# line 391 "strong.py"

			(*parm)->tbs_DERcode = e_ListArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((ListResultTBS ** )__p)
	case 35: /*  */
		{
# line 409 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ListResult ** )__p)
	case 36: /*  */
		{
# line 423 "strong.py"

        if (((*parm) = (ListResult *) calloc (1, sizeof (ListResult))) == ((ListResult *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((ListResult ** )__p)
	case 37: /*  */
		{
# line 429 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ListResult ** )__p)
	case 38: /*  */
		{
# line 438 "strong.py"

			(*parm)->tbs_DERcode = e_ListResultTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS ** )__p)
	case 39: /*  */
		{
# line 472 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS ** )__p)
	case 40: /*  */
		{
# line 476 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS ** )__p)
	case 41: /*  */
		{
# line 480 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS ** )__p)
	case 42: /*  */
		{
# line 484 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS ** )__p)
	case 43: /*  */
		{
# line 489 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyEntryArgumentTBS ** )__p)
	case 44: /*  */
		{
# line 494 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyEntryArgument ** )__p)
	case 45: /*  */
		{
# line 500 "strong.py"

        if (((*parm) = (ModifyEntryArgument *) calloc (1, sizeof (ModifyEntryArgument))) == ((ModifyEntryArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((ModifyEntryArgument ** )__p)
	case 46: /*  */
		{
# line 506 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ModifyEntryArgument ** )__p)
	case 47: /*  */
		{
# line 515 "strong.py"

			(*parm)->tbs_DERcode = e_ModifyEntryArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS ** )__p)
	case 48: /*  */
		{
# line 547 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS ** )__p)
	case 49: /*  */
		{
# line 551 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS ** )__p)
	case 50: /*  */
		{
# line 555 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS ** )__p)
	case 51: /*  */
		{
# line 559 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS ** )__p)
	case 52: /*  */
		{
# line 564 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyRDNArgumentTBS ** )__p)
	case 53: /*  */
		{
# line 569 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ModifyRDNArgument ** )__p)
	case 54: /*  */
		{
# line 575 "strong.py"

        if (((*parm) = (ModifyRDNArgument *) calloc (1, sizeof (ModifyRDNArgument))) == ((ModifyRDNArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((ModifyRDNArgument ** )__p)
	case 55: /*  */
		{
# line 581 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ModifyRDNArgument ** )__p)
	case 56: /*  */
		{
# line 590 "strong.py"

			(*parm)->tbs_DERcode = e_ModifyRDNArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS ** )__p)
	case 57: /*  */
		{
# line 620 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS ** )__p)
	case 58: /*  */
		{
# line 624 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS ** )__p)
	case 59: /*  */
		{
# line 628 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS ** )__p)
	case 60: /*  */
		{
# line 632 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS ** )__p)
	case 61: /*  */
		{
# line 637 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadArgumentTBS ** )__p)
	case 62: /*  */
		{
# line 642 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadArgument ** )__p)
	case 63: /*  */
		{
# line 648 "strong.py"

        if (((*parm) = (ReadArgument *) calloc (1, sizeof (ReadArgument))) == ((ReadArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((ReadArgument ** )__p)
	case 64: /*  */
		{
# line 654 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ReadArgument ** )__p)
	case 65: /*  */
		{
# line 663 "strong.py"

			(*parm)->tbs_DERcode = e_ReadArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((ReadResultTBS ** )__p)
	case 66: /*  */
		{
# line 689 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadResultTBS ** )__p)
	case 67: /*  */
		{
# line 693 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadResultTBS ** )__p)
	case 68: /*  */
		{
# line 697 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ReadResult ** )__p)
	case 69: /*  */
		{
# line 703 "strong.py"

        if (((*parm) = (ReadResult *) calloc (1, sizeof (ReadResult))) == ((ReadResult *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((ReadResult ** )__p)
	case 70: /*  */
		{
# line 709 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ReadResult ** )__p)
	case 71: /*  */
		{
# line 718 "strong.py"

			(*parm)->tbs_DERcode = e_ReadResultTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS ** )__p)
	case 72: /*  */
		{
# line 743 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS ** )__p)
	case 73: /*  */
		{
# line 747 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS ** )__p)
	case 74: /*  */
		{
# line 751 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS ** )__p)
	case 75: /*  */
		{
# line 755 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS ** )__p)
	case 76: /*  */
		{
# line 760 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((RemoveArgumentTBS ** )__p)
	case 77: /*  */
		{
# line 765 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((RemoveArgument ** )__p)
	case 78: /*  */
		{
# line 771 "strong.py"

        if (((*parm) = (RemoveArgument *) calloc (1, sizeof (RemoveArgument))) == ((RemoveArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((RemoveArgument ** )__p)
	case 79: /*  */
		{
# line 777 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((RemoveArgument ** )__p)
	case 80: /*  */
		{
# line 786 "strong.py"

			(*parm)->tbs_DERcode = e_RemoveArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS ** )__p)
	case 81: /*  */
		{
# line 833 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS ** )__p)
	case 82: /*  */
		{
# line 837 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS ** )__p)
	case 83: /*  */
		{
# line 841 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS ** )__p)
	case 84: /*  */
		{
# line 845 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS ** )__p)
	case 85: /*  */
		{
# line 850 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchArgumentTBS ** )__p)
	case 86: /*  */
		{
# line 855 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchArgument ** )__p)
	case 87: /*  */
		{
# line 861 "strong.py"

        if (((*parm) = (SearchArgument *) calloc (1, sizeof (SearchArgument))) == ((SearchArgument *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((SearchArgument ** )__p)
	case 88: /*  */
		{
# line 867 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((SearchArgument ** )__p)
	case 89: /*  */
		{
# line 876 "strong.py"

			(*parm)->tbs_DERcode = e_SearchArgumentTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((SearchResultTBS ** )__p)
	case 90: /*  */
		{
# line 894 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((SearchResult ** )__p)
	case 91: /*  */
		{
# line 906 "strong.py"

        if (((*parm) = (SearchResult *) calloc (1, sizeof (SearchResult))) == ((SearchResult *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((SearchResult ** )__p)
	case 92: /*  */
		{
# line 912 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((SearchResult ** )__p)
	case 93: /*  */
		{
# line 921 "strong.py"

			(*parm)->tbs_DERcode = e_SearchResultTBS((*parm)->tbs);
			if (((*parm)->sig = (Signature *) calloc (1, sizeof (Signature))) == ((Signature *) 0)) {
				advise (NULLCP, "out of memory");
				return NOTOK;
			}
			;
		}
	    break;

#undef parm

#define parm	((EntryModification ** )__p)
	case 94: /*  */
		{
# line 945 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ServiceControls ** )__p)
	case 95: /*  */
		{
# line 997 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ServiceControls ** )__p)
	case 96: /*  */
		{
# line 1013 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ServiceControls ** )__p)
	case 97: /*  */
		{
# line 1019 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ServiceControls ** )__p)
	case 98: /*  */
		{
# line 1029 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((OperationProgress ** )__p)
	case 99: /*  */
		{
# line 1085 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((AttrAttrTypeCHOICE ** )__p)
	case 100: /*  */
		{
# line 1119 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((EntryInfoSEL ** )__p)
	case 101: /*  */
		{
# line 1135 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((EntryInfoSEL ** )__p)
	case 102: /*  */
		{
# line 1145 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((StringsCHOICE ** )__p)
	case 103: /*  */
		{
# line 1185 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((SFilterItem ** )__p)
	case 104: /*  */
		{
# line 1221 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((SFilter ** )__p)
	case 105: /*  */
		{
# line 1252 "strong.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((ListInfo ** )__p)
	case 106: /*  */
		{
# line 1313 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListInfo ** )__p)
	case 107: /*  */
		{
# line 1318 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListInfo ** )__p)
	case 108: /*  */
		{
# line 1322 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ListInfo ** )__p)
	case 109: /*  */
		{
# line 1326 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchInfo ** )__p)
	case 110: /*  */
		{
# line 1349 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchInfo ** )__p)
	case 111: /*  */
		{
# line 1354 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchInfo ** )__p)
	case 112: /*  */
		{
# line 1358 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((SearchInfo ** )__p)
	case 113: /*  */
		{
# line 1362 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((PartialOutQual ** )__p)
	case 114: /*  */
		{
# line 1378 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ContReference ** )__p)
	case 115: /*  */
		{
# line 1427 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ContReference ** )__p)
	case 116: /*  */
		{
# line 1434 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((ContReference ** )__p)
	case 117: /*  */
		{
# line 1438 "strong.py"
0;
		}
	    break;

#undef parm

#define parm	((typeDSE_PSAPaddr ** )__p)
	case 118: /*  */
		{
# line 1463 "strong.py"

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
	;
		}
	    break;

#undef parm

#define parm	((typeDSE_PSAPaddr ** )__p)
	case 119: /*  */
		{
# line 1481 "strong.py"
(*parm)->pSelector.noctets > 0;
		}
	    break;

#undef parm

#define parm	((typeDSE_PSAPaddr ** )__p)
	case 120: /*  */
		{
# line 1487 "strong.py"
(*parm)->sSelector.noctets > 0;
		}
	    break;

#undef parm

#define parm	((typeDSE_PSAPaddr ** )__p)
	case 121: /*  */
		{
# line 1493 "strong.py"
(*parm)->tSelector.noctets > 0;
		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, pe, "dnf_STRONG:Bad table entry: %d",
            _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}
static ptpe *etabl[] = {
	et_TSETOFSECExtensionSTRONG,
	et_TIntegerSTRONG,
	et_TOperationProgressSTRONG,
	et_TRequestorSTRONG,
	et_TSecParmArgumentSTRONG,
	et_TServiceControlsSTRONG,
	et_TBooleanSTRONG,
	et_TPerformerSTRONG,
	et_TSecParmResultSTRONG,
	et_TAttributeSTRONG,
	et_TokenTBSSTRONG,
	et_TokenSTRONG,
	et_AddArgumentTBSSTRONG,
	et_AddArgumentSTRONG,
	et_CompareArgumentTBSSTRONG,
	et_CompareArgumentSTRONG,
	et_CompareResultTBSSTRONG,
	et_CompareResultSTRONG,
	et_ListArgumentTBSSTRONG,
	et_ListArgumentSTRONG,
	et_ListResultTBSSTRONG,
	et_ListResultSTRONG,
	et_SETOFListResultSTRONG,
	et_ModifyEntryArgumentTBSSTRONG,
	et_ModifyEntryArgumentSTRONG,
	et_ModifyRDNArgumentTBSSTRONG,
	et_ModifyRDNArgumentSTRONG,
	et_ReadArgumentTBSSTRONG,
	et_ReadArgumentSTRONG,
	et_ReadResultTBSSTRONG,
	et_ReadResultSTRONG,
	et_RemoveArgumentTBSSTRONG,
	et_RemoveArgumentSTRONG,
	et_SearchArgumentTBSSTRONG,
	et_SearchArgumentSTRONG,
	et_SearchResultTBSSTRONG,
	et_SearchResultSTRONG,
	et_SETOFSearchResultSTRONG,
	et_EntryModificationSTRONG,
	et_EntryModificationSequenceSTRONG,
	et_ServiceControlsSTRONG,
	et_SecurityParametersSTRONG,
	et_OperationProgressSTRONG,
	et_SETOFSECExtensionSTRONG,
	et_SECExtensionSTRONG,
	et_SETOFAttrAttrTypeCHOICESTRONG,
	et_AttrAttrTypeCHOICESTRONG,
	et_EntryInfoSELSTRONG,
	et_SETOFEntryINFOSTRONG,
	et_EntryINFOSTRONG,
	et_StringsCHOICESTRONG,
	et_SEQUENCEOFStringsCHOICESTRONG,
	et_FilterSubstringsSTRONG,
	et_FilterItemSTRONG,
	et_SETOFFilterSTRONG,
	et_FilterSTRONG,
	et_SETOFSubordEntrySTRONG,
	et_SubordEntrySTRONG,
	et_ListInfoSTRONG,
	et_SearchInfoSTRONG,
	et_SETOFDNameSTRONG,
	et_PartialOutcomeQualifierSTRONG,
	et_ReferenceTypeSTRONG,
	et_LimitProblemSTRONG,
	et_SETOFContinuationReferenceSTRONG,
	et_ContinuationReferenceSTRONG,
	et_SETOFAccessPointSTRONG,
	et_AccessPointSTRONG,
	et_PSAPaddrSTRONG,
	};

static ptpe *dtabl[] = {
	dt_TSETOFSECExtensionSTRONG,
	dt_TIntegerSTRONG,
	dt_TOperationProgressSTRONG,
	dt_TRequestorSTRONG,
	dt_TSecParmArgumentSTRONG,
	dt_TServiceControlsSTRONG,
	dt_TBooleanSTRONG,
	dt_TPerformerSTRONG,
	dt_TSecParmResultSTRONG,
	dt_TAttributeSTRONG,
	dt_TokenTBSSTRONG,
	dt_TokenSTRONG,
	dt_AddArgumentTBSSTRONG,
	dt_AddArgumentSTRONG,
	dt_CompareArgumentTBSSTRONG,
	dt_CompareArgumentSTRONG,
	dt_CompareResultTBSSTRONG,
	dt_CompareResultSTRONG,
	dt_ListArgumentTBSSTRONG,
	dt_ListArgumentSTRONG,
	dt_ListResultTBSSTRONG,
	dt_ListResultSTRONG,
	dt_SETOFListResultSTRONG,
	dt_ModifyEntryArgumentTBSSTRONG,
	dt_ModifyEntryArgumentSTRONG,
	dt_ModifyRDNArgumentTBSSTRONG,
	dt_ModifyRDNArgumentSTRONG,
	dt_ReadArgumentTBSSTRONG,
	dt_ReadArgumentSTRONG,
	dt_ReadResultTBSSTRONG,
	dt_ReadResultSTRONG,
	dt_RemoveArgumentTBSSTRONG,
	dt_RemoveArgumentSTRONG,
	dt_SearchArgumentTBSSTRONG,
	dt_SearchArgumentSTRONG,
	dt_SearchResultTBSSTRONG,
	dt_SearchResultSTRONG,
	dt_SETOFSearchResultSTRONG,
	dt_EntryModificationSTRONG,
	dt_EntryModificationSequenceSTRONG,
	dt_ServiceControlsSTRONG,
	dt_SecurityParametersSTRONG,
	dt_OperationProgressSTRONG,
	dt_SETOFSECExtensionSTRONG,
	dt_SECExtensionSTRONG,
	dt_SETOFAttrAttrTypeCHOICESTRONG,
	dt_AttrAttrTypeCHOICESTRONG,
	dt_EntryInfoSELSTRONG,
	dt_SETOFEntryINFOSTRONG,
	dt_EntryINFOSTRONG,
	dt_StringsCHOICESTRONG,
	dt_SEQUENCEOFStringsCHOICESTRONG,
	dt_FilterSubstringsSTRONG,
	dt_FilterItemSTRONG,
	dt_SETOFFilterSTRONG,
	dt_FilterSTRONG,
	dt_SETOFSubordEntrySTRONG,
	dt_SubordEntrySTRONG,
	dt_ListInfoSTRONG,
	dt_SearchInfoSTRONG,
	dt_SETOFDNameSTRONG,
	dt_PartialOutcomeQualifierSTRONG,
	dt_ReferenceTypeSTRONG,
	dt_LimitProblemSTRONG,
	dt_SETOFContinuationReferenceSTRONG,
	dt_ContinuationReferenceSTRONG,
	dt_SETOFAccessPointSTRONG,
	dt_AccessPointSTRONG,
	dt_PSAPaddrSTRONG,
	};

static ptpe *ptabl[] = {
	pt_TSETOFSECExtensionSTRONG,
	pt_TIntegerSTRONG,
	pt_TOperationProgressSTRONG,
	pt_TRequestorSTRONG,
	pt_TSecParmArgumentSTRONG,
	pt_TServiceControlsSTRONG,
	pt_TBooleanSTRONG,
	pt_TPerformerSTRONG,
	pt_TSecParmResultSTRONG,
	pt_TAttributeSTRONG,
	pt_TokenTBSSTRONG,
	pt_TokenSTRONG,
	pt_AddArgumentTBSSTRONG,
	pt_AddArgumentSTRONG,
	pt_CompareArgumentTBSSTRONG,
	pt_CompareArgumentSTRONG,
	pt_CompareResultTBSSTRONG,
	pt_CompareResultSTRONG,
	pt_ListArgumentTBSSTRONG,
	pt_ListArgumentSTRONG,
	pt_ListResultTBSSTRONG,
	pt_ListResultSTRONG,
	pt_SETOFListResultSTRONG,
	pt_ModifyEntryArgumentTBSSTRONG,
	pt_ModifyEntryArgumentSTRONG,
	pt_ModifyRDNArgumentTBSSTRONG,
	pt_ModifyRDNArgumentSTRONG,
	pt_ReadArgumentTBSSTRONG,
	pt_ReadArgumentSTRONG,
	pt_ReadResultTBSSTRONG,
	pt_ReadResultSTRONG,
	pt_RemoveArgumentTBSSTRONG,
	pt_RemoveArgumentSTRONG,
	pt_SearchArgumentTBSSTRONG,
	pt_SearchArgumentSTRONG,
	pt_SearchResultTBSSTRONG,
	pt_SearchResultSTRONG,
	pt_SETOFSearchResultSTRONG,
	pt_EntryModificationSTRONG,
	pt_EntryModificationSequenceSTRONG,
	pt_ServiceControlsSTRONG,
	pt_SecurityParametersSTRONG,
	pt_OperationProgressSTRONG,
	pt_SETOFSECExtensionSTRONG,
	pt_SECExtensionSTRONG,
	pt_SETOFAttrAttrTypeCHOICESTRONG,
	pt_AttrAttrTypeCHOICESTRONG,
	pt_EntryInfoSELSTRONG,
	pt_SETOFEntryINFOSTRONG,
	pt_EntryINFOSTRONG,
	pt_StringsCHOICESTRONG,
	pt_SEQUENCEOFStringsCHOICESTRONG,
	pt_FilterSubstringsSTRONG,
	pt_FilterItemSTRONG,
	pt_SETOFFilterSTRONG,
	pt_FilterSTRONG,
	pt_SETOFSubordEntrySTRONG,
	pt_SubordEntrySTRONG,
	pt_ListInfoSTRONG,
	pt_SearchInfoSTRONG,
	pt_SETOFDNameSTRONG,
	pt_PartialOutcomeQualifierSTRONG,
	pt_ReferenceTypeSTRONG,
	pt_LimitProblemSTRONG,
	pt_SETOFContinuationReferenceSTRONG,
	pt_ContinuationReferenceSTRONG,
	pt_SETOFAccessPointSTRONG,
	pt_AccessPointSTRONG,
	pt_PSAPaddrSTRONG,
	};


/* Pointer table 157 entries */
caddr_t _ZptrtabSTRONG[] = {
    (caddr_t ) "TSETOFSECExtension",
    (caddr_t ) "TInteger",
    (caddr_t ) "TOperationProgress",
    (caddr_t ) "TRequestor",
    (caddr_t ) &_ZSECIF_mod,
    (caddr_t ) "TSecParmArgument",
    (caddr_t ) "TServiceControls",
    (caddr_t ) "TBoolean",
    (caddr_t ) "TPerformer",
    (caddr_t ) "TSecParmResult",
    (caddr_t ) "TAttribute",
    (caddr_t ) "TokenTBS",
    (caddr_t ) "signature",
    (caddr_t ) &_ZSEC_mod,
    (caddr_t ) "name",
    (caddr_t ) "time",
    (caddr_t ) "random",
    (caddr_t ) "Token",
    (caddr_t ) "AddArgumentTBS",
    (caddr_t ) "object",
    (caddr_t ) "entry",
    (caddr_t ) "extensions",
    (caddr_t ) "aliasedRDNs",
    (caddr_t ) "progress",
    (caddr_t ) "requestor",
    (caddr_t ) "secparm",
    (caddr_t ) "servcontr",
    (caddr_t ) "AddArgument",
    (caddr_t ) "CompareArgumentTBS",
    (caddr_t ) "purported",
    (caddr_t ) "CompareArgument",
    (caddr_t ) "CompareResultTBS",
    (caddr_t ) "matched",
    (caddr_t ) "fromEntry",
    (caddr_t ) "aliasDereferenced",
    (caddr_t ) "performer",
    (caddr_t ) "CompareResult",
    (caddr_t ) "ListArgumentTBS",
    (caddr_t ) "ListArgument",
    (caddr_t ) "ListResultTBS",
    (caddr_t ) "listInfo",
    (caddr_t ) "uncorrelatedListInfo",
    (caddr_t ) "ListResult",
    (caddr_t ) "SETOFListResult",
    (caddr_t ) "ModifyEntryArgumentTBS",
    (caddr_t ) "changes",
    (caddr_t ) "ModifyEntryArgument",
    (caddr_t ) "ModifyRDNArgumentTBS",
    (caddr_t ) "newRDN",
    (caddr_t ) "deleteOldRDN",
    (caddr_t ) "ModifyRDNArgument",
    (caddr_t ) "ReadArgumentTBS",
    (caddr_t ) "selection",
    (caddr_t ) "ReadArgument",
    (caddr_t ) "ReadResultTBS",
    (caddr_t ) "ReadResult",
    (caddr_t ) "RemoveArgumentTBS",
    (caddr_t ) "RemoveArgument",
    (caddr_t ) "SearchArgumentTBS",
    (caddr_t ) "baseObject",
    (caddr_t ) "subset",
    (caddr_t ) "filter",
    (caddr_t ) "searchAliases",
    (caddr_t ) "SearchArgument",
    (caddr_t ) "SearchResultTBS",
    (caddr_t ) "searchInfo",
    (caddr_t ) "uncorrelatedSearchInfo",
    (caddr_t ) "SearchResult",
    (caddr_t ) "SETOFSearchResult",
    (caddr_t ) "EntryModification",
    (caddr_t ) "addAttribute",
    (caddr_t ) "removeAttribute",
    (caddr_t ) "addValues",
    (caddr_t ) "removeValues",
    (caddr_t ) "EntryModificationSequence",
    (caddr_t ) "ServiceControls",
    (caddr_t ) "options",
    (caddr_t ) "priority",
    (caddr_t ) "timeLimit",
    (caddr_t ) "sizeLimit",
    (caddr_t ) "scopeOfReferral",
    (caddr_t ) "SecurityParameters",
    (caddr_t ) "certificationPath",
    (caddr_t ) &_ZSECAF_mod,
    (caddr_t ) "target",
    (caddr_t ) "OperationProgress",
    (caddr_t ) "opResolutionPhase",
    (caddr_t ) "opNextRDNToBeResolved",
    (caddr_t ) "SETOFSECExtension",
    (caddr_t ) "SECExtension",
    (caddr_t ) "identifier",
    (caddr_t ) "critical",
    (caddr_t ) "item",
    (caddr_t ) "SETOFAttrAttrTypeCHOICE",
    (caddr_t ) "AttrAttrTypeCHOICE",
    (caddr_t ) "EntryInfoSEL",
    (caddr_t ) "attributeTypes",
    (caddr_t ) "allAttributes",
    (caddr_t ) "select",
    (caddr_t ) "infoTypes",
    (caddr_t ) "SETOFEntryINFO",
    (caddr_t ) "EntryINFO",
    (caddr_t ) "StringsCHOICE",
    (caddr_t ) "initial",
    (caddr_t ) "any",
    (caddr_t ) "final",
    (caddr_t ) "SEQUENCEOFStringsCHOICE",
    (caddr_t ) "FilterSubstrings",
    (caddr_t ) "type",
    (caddr_t ) "strings",
    (caddr_t ) "FilterItem",
    (caddr_t ) "equality",
    (caddr_t ) "substrings",
    (caddr_t ) "greaterOrEqual",
    (caddr_t ) "lessOrEqual",
    (caddr_t ) "present",
    (caddr_t ) "approximateMatch",
    (caddr_t ) "SETOFFilter",
    (caddr_t ) "Filter",
    (caddr_t ) "and",
    (caddr_t ) "or",
    (caddr_t ) "not",
    (caddr_t ) "SETOFSubordEntry",
    (caddr_t ) "SubordEntry",
    (caddr_t ) "aliasEntry",
    (caddr_t ) "ListInfo",
    (caddr_t ) "subordinates",
    (caddr_t ) "partialOutcomeQualifier",
    (caddr_t ) "SearchInfo",
    (caddr_t ) "entries",
    (caddr_t ) "SETOFDName",
    (caddr_t ) "PartialOutcomeQualifier",
    (caddr_t ) "limitProblem",
    (caddr_t ) "unexplored",
    (caddr_t ) "unavailableCriticalExtensions",
    (caddr_t ) "ReferenceType",
    (caddr_t ) "LimitProblem",
    (caddr_t ) "SETOFContinuationReference",
    (caddr_t ) "ContinuationReference",
    (caddr_t ) "targetObject",
    (caddr_t ) "operationProgress",
    (caddr_t ) "rdnsResolved",
    (caddr_t ) "referenceType",
    (caddr_t ) "accessPoints",
    (caddr_t ) "SETOFAccessPoint",
    (caddr_t ) "AccessPoint",
    (caddr_t ) "PSAPaddr",
    (caddr_t ) "pSelector",
    (caddr_t ) "sSelector",
    (caddr_t ) "tSelector",
    (caddr_t ) "nAddress",
    (caddr_t ) "Name",
    (caddr_t ) "Attribute",
    (caddr_t ) "TAlgorithmIdentifier",
    (caddr_t ) "TBitString",
    (caddr_t ) bits_STRONG_options,
    (caddr_t ) "RelativeDistinguishedName",
};

modtyp _ZSTRONG_mod = {
	"STRONG",
	69,
	etabl,
	dtabl,
	ptabl,
	efn_STRONG,
	dfn_STRONG,
	0,
	_ZptrtabSTRONG,
	};


# line 1547 "strong.py"

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



#if	defined(lint) || defined(PEPSY_LINKABLE_FUNCS)

#undef parse_STRONG_TSETOFSECExtension
int	parse_STRONG_TSETOFSECExtension(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonArguments  **parm;
{
  return (dec_f(_ZTSETOFSECExtensionSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TSETOFSECExtension
void free_STRONG_TSETOFSECExtension(val)
CommonArguments  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTSETOFSECExtensionSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TInteger
int	parse_STRONG_TInteger(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonArguments  **parm;
{
  return (dec_f(_ZTIntegerSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TInteger
void free_STRONG_TInteger(val)
CommonArguments  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTIntegerSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TOperationProgress
int	parse_STRONG_TOperationProgress(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonArguments  **parm;
{
  return (dec_f(_ZTOperationProgressSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TOperationProgress
void free_STRONG_TOperationProgress(val)
CommonArguments  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTOperationProgressSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TRequestor
int	parse_STRONG_TRequestor(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonArguments  **parm;
{
  return (dec_f(_ZTRequestorSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TRequestor
void free_STRONG_TRequestor(val)
CommonArguments  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTRequestorSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TSecParmArgument
int	parse_STRONG_TSecParmArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonArguments  **parm;
{
  return (dec_f(_ZTSecParmArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TSecParmArgument
void free_STRONG_TSecParmArgument(val)
CommonArguments  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTSecParmArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TServiceControls
int	parse_STRONG_TServiceControls(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonArguments  **parm;
{
  return (dec_f(_ZTServiceControlsSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TServiceControls
void free_STRONG_TServiceControls(val)
CommonArguments  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTServiceControlsSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TBoolean
int	parse_STRONG_TBoolean(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonRes  **parm;
{
  return (dec_f(_ZTBooleanSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TBoolean
void free_STRONG_TBoolean(val)
CommonRes  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTBooleanSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TPerformer
int	parse_STRONG_TPerformer(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonRes  **parm;
{
  return (dec_f(_ZTPerformerSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TPerformer
void free_STRONG_TPerformer(val)
CommonRes  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTPerformerSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TSecParmResult
int	parse_STRONG_TSecParmResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CommonRes  **parm;
{
  return (dec_f(_ZTSecParmResultSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TSecParmResult
void free_STRONG_TSecParmResult(val)
CommonRes  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTSecParmResultSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TAttribute
int	parse_STRONG_TAttribute(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_Attr  **parm;
{
  return (dec_f(_ZTAttributeSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TAttribute
void free_STRONG_TAttribute(val)
SET_OF_Attr  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTAttributeSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_TokenTBS
int	parse_STRONG_TokenTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
TokenTBS  **parm;
{
  return (dec_f(_ZTokenTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_TokenTBS
void free_STRONG_TokenTBS(val)
TokenTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTokenTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_Token
int	parse_STRONG_Token(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Token  **parm;
{
  return (dec_f(_ZTokenSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_Token
void free_STRONG_Token(val)
Token  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZTokenSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_AddArgumentTBS
int	parse_STRONG_AddArgumentTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AddArgumentTBS  **parm;
{
  return (dec_f(_ZAddArgumentTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_AddArgumentTBS
void free_STRONG_AddArgumentTBS(val)
AddArgumentTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZAddArgumentTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_AddArgument
int	parse_STRONG_AddArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AddArgument  **parm;
{
  return (dec_f(_ZAddArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_AddArgument
void free_STRONG_AddArgument(val)
AddArgument  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZAddArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_CompareArgumentTBS
int	parse_STRONG_CompareArgumentTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CompareArgumentTBS  **parm;
{
  return (dec_f(_ZCompareArgumentTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_CompareArgumentTBS
void free_STRONG_CompareArgumentTBS(val)
CompareArgumentTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZCompareArgumentTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_CompareArgument
int	parse_STRONG_CompareArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CompareArgument  **parm;
{
  return (dec_f(_ZCompareArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_CompareArgument
void free_STRONG_CompareArgument(val)
CompareArgument  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZCompareArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_CompareResultTBS
int	parse_STRONG_CompareResultTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CompareResultTBS  **parm;
{
  return (dec_f(_ZCompareResultTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_CompareResultTBS
void free_STRONG_CompareResultTBS(val)
CompareResultTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZCompareResultTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_CompareResult
int	parse_STRONG_CompareResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CompareResult  **parm;
{
  return (dec_f(_ZCompareResultSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_CompareResult
void free_STRONG_CompareResult(val)
CompareResult  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZCompareResultSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ListArgumentTBS
int	parse_STRONG_ListArgumentTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ListArgumentTBS  **parm;
{
  return (dec_f(_ZListArgumentTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ListArgumentTBS
void free_STRONG_ListArgumentTBS(val)
ListArgumentTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZListArgumentTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ListArgument
int	parse_STRONG_ListArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ListArgument  **parm;
{
  return (dec_f(_ZListArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ListArgument
void free_STRONG_ListArgument(val)
ListArgument  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZListArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ListResultTBS
int	parse_STRONG_ListResultTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ListResultTBS  **parm;
{
  return (dec_f(_ZListResultTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ListResultTBS
void free_STRONG_ListResultTBS(val)
ListResultTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZListResultTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ListResult
int	parse_STRONG_ListResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ListResult  **parm;
{
  return (dec_f(_ZListResultSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ListResult
void free_STRONG_ListResult(val)
ListResult  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZListResultSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFListResult
int	parse_STRONG_SETOFListResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_ListResult  **parm;
{
  return (dec_f(_ZSETOFListResultSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFListResult
void free_STRONG_SETOFListResult(val)
SET_OF_ListResult  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFListResultSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ModifyEntryArgumentTBS
int	parse_STRONG_ModifyEntryArgumentTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ModifyEntryArgumentTBS  **parm;
{
  return (dec_f(_ZModifyEntryArgumentTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ModifyEntryArgumentTBS
void free_STRONG_ModifyEntryArgumentTBS(val)
ModifyEntryArgumentTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZModifyEntryArgumentTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ModifyEntryArgument
int	parse_STRONG_ModifyEntryArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ModifyEntryArgument  **parm;
{
  return (dec_f(_ZModifyEntryArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ModifyEntryArgument
void free_STRONG_ModifyEntryArgument(val)
ModifyEntryArgument  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZModifyEntryArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ModifyRDNArgumentTBS
int	parse_STRONG_ModifyRDNArgumentTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ModifyRDNArgumentTBS  **parm;
{
  return (dec_f(_ZModifyRDNArgumentTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ModifyRDNArgumentTBS
void free_STRONG_ModifyRDNArgumentTBS(val)
ModifyRDNArgumentTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZModifyRDNArgumentTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ModifyRDNArgument
int	parse_STRONG_ModifyRDNArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ModifyRDNArgument  **parm;
{
  return (dec_f(_ZModifyRDNArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ModifyRDNArgument
void free_STRONG_ModifyRDNArgument(val)
ModifyRDNArgument  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZModifyRDNArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ReadArgumentTBS
int	parse_STRONG_ReadArgumentTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ReadArgumentTBS  **parm;
{
  return (dec_f(_ZReadArgumentTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ReadArgumentTBS
void free_STRONG_ReadArgumentTBS(val)
ReadArgumentTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZReadArgumentTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ReadArgument
int	parse_STRONG_ReadArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ReadArgument  **parm;
{
  return (dec_f(_ZReadArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ReadArgument
void free_STRONG_ReadArgument(val)
ReadArgument  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZReadArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ReadResultTBS
int	parse_STRONG_ReadResultTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ReadResultTBS  **parm;
{
  return (dec_f(_ZReadResultTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ReadResultTBS
void free_STRONG_ReadResultTBS(val)
ReadResultTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZReadResultTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ReadResult
int	parse_STRONG_ReadResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ReadResult  **parm;
{
  return (dec_f(_ZReadResultSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ReadResult
void free_STRONG_ReadResult(val)
ReadResult  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZReadResultSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_RemoveArgumentTBS
int	parse_STRONG_RemoveArgumentTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
RemoveArgumentTBS  **parm;
{
  return (dec_f(_ZRemoveArgumentTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_RemoveArgumentTBS
void free_STRONG_RemoveArgumentTBS(val)
RemoveArgumentTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZRemoveArgumentTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_RemoveArgument
int	parse_STRONG_RemoveArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
RemoveArgument  **parm;
{
  return (dec_f(_ZRemoveArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_RemoveArgument
void free_STRONG_RemoveArgument(val)
RemoveArgument  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZRemoveArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SearchArgumentTBS
int	parse_STRONG_SearchArgumentTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SearchArgumentTBS  **parm;
{
  return (dec_f(_ZSearchArgumentTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SearchArgumentTBS
void free_STRONG_SearchArgumentTBS(val)
SearchArgumentTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSearchArgumentTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SearchArgument
int	parse_STRONG_SearchArgument(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SearchArgument  **parm;
{
  return (dec_f(_ZSearchArgumentSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SearchArgument
void free_STRONG_SearchArgument(val)
SearchArgument  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSearchArgumentSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SearchResultTBS
int	parse_STRONG_SearchResultTBS(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SearchResultTBS  **parm;
{
  return (dec_f(_ZSearchResultTBSSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SearchResultTBS
void free_STRONG_SearchResultTBS(val)
SearchResultTBS  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSearchResultTBSSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SearchResult
int	parse_STRONG_SearchResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SearchResult  **parm;
{
  return (dec_f(_ZSearchResultSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SearchResult
void free_STRONG_SearchResult(val)
SearchResult  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSearchResultSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFSearchResult
int	parse_STRONG_SETOFSearchResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_SearchResult  **parm;
{
  return (dec_f(_ZSETOFSearchResultSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFSearchResult
void free_STRONG_SETOFSearchResult(val)
SET_OF_SearchResult  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFSearchResultSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_EntryModification
int	parse_STRONG_EntryModification(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
EntryModification  **parm;
{
  return (dec_f(_ZEntryModificationSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_EntryModification
void free_STRONG_EntryModification(val)
EntryModification  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZEntryModificationSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_EntryModificationSequence
int	parse_STRONG_EntryModificationSequence(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SEQUENCE_OF_EntryModification  **parm;
{
  return (dec_f(_ZEntryModificationSequenceSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_EntryModificationSequence
void free_STRONG_EntryModificationSequence(val)
SEQUENCE_OF_EntryModification  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZEntryModificationSequenceSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ServiceControls
int	parse_STRONG_ServiceControls(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ServiceControls  **parm;
{
  return (dec_f(_ZServiceControlsSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ServiceControls
void free_STRONG_ServiceControls(val)
ServiceControls  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZServiceControlsSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SecurityParameters
int	parse_STRONG_SecurityParameters(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SecurityParameters  **parm;
{
  return (dec_f(_ZSecurityParametersSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SecurityParameters
void free_STRONG_SecurityParameters(val)
SecurityParameters  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSecurityParametersSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_OperationProgress
int	parse_STRONG_OperationProgress(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
OperationProgress  **parm;
{
  return (dec_f(_ZOperationProgressSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_OperationProgress
void free_STRONG_OperationProgress(val)
OperationProgress  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZOperationProgressSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFSECExtension
int	parse_STRONG_SETOFSECExtension(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_SECExtension  **parm;
{
  return (dec_f(_ZSETOFSECExtensionSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFSECExtension
void free_STRONG_SETOFSECExtension(val)
SET_OF_SECExtension  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFSECExtensionSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SECExtension
int	parse_STRONG_SECExtension(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SECExtension  **parm;
{
  return (dec_f(_ZSECExtensionSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SECExtension
void free_STRONG_SECExtension(val)
SECExtension  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSECExtensionSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFAttrAttrTypeCHOICE
int	parse_STRONG_SETOFAttrAttrTypeCHOICE(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_AttrAttrTypeCHOICE  **parm;
{
  return (dec_f(_ZSETOFAttrAttrTypeCHOICESTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFAttrAttrTypeCHOICE
void free_STRONG_SETOFAttrAttrTypeCHOICE(val)
SET_OF_AttrAttrTypeCHOICE  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFAttrAttrTypeCHOICESTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_AttrAttrTypeCHOICE
int	parse_STRONG_AttrAttrTypeCHOICE(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AttrAttrTypeCHOICE  **parm;
{
  return (dec_f(_ZAttrAttrTypeCHOICESTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_AttrAttrTypeCHOICE
void free_STRONG_AttrAttrTypeCHOICE(val)
AttrAttrTypeCHOICE  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZAttrAttrTypeCHOICESTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_EntryInfoSEL
int	parse_STRONG_EntryInfoSEL(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
EntryInfoSEL  **parm;
{
  return (dec_f(_ZEntryInfoSELSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_EntryInfoSEL
void free_STRONG_EntryInfoSEL(val)
EntryInfoSEL  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZEntryInfoSELSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFEntryINFO
int	parse_STRONG_SETOFEntryINFO(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_EntryINFO  **parm;
{
  return (dec_f(_ZSETOFEntryINFOSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFEntryINFO
void free_STRONG_SETOFEntryINFO(val)
SET_OF_EntryINFO  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFEntryINFOSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_EntryINFO
int	parse_STRONG_EntryINFO(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
EntryINFO  **parm;
{
  return (dec_f(_ZEntryINFOSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_EntryINFO
void free_STRONG_EntryINFO(val)
EntryINFO  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZEntryINFOSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_StringsCHOICE
int	parse_STRONG_StringsCHOICE(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
StringsCHOICE  **parm;
{
  return (dec_f(_ZStringsCHOICESTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_StringsCHOICE
void free_STRONG_StringsCHOICE(val)
StringsCHOICE  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZStringsCHOICESTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SEQUENCEOFStringsCHOICE
int	parse_STRONG_SEQUENCEOFStringsCHOICE(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SEQUENCE_OF_StringsCHOICE  **parm;
{
  return (dec_f(_ZSEQUENCEOFStringsCHOICESTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SEQUENCEOFStringsCHOICE
void free_STRONG_SEQUENCEOFStringsCHOICE(val)
SEQUENCE_OF_StringsCHOICE  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSEQUENCEOFStringsCHOICESTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_FilterSubstrings
int	parse_STRONG_FilterSubstrings(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SFilterSubstrings  **parm;
{
  return (dec_f(_ZFilterSubstringsSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_FilterSubstrings
void free_STRONG_FilterSubstrings(val)
SFilterSubstrings  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZFilterSubstringsSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_FilterItem
int	parse_STRONG_FilterItem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SFilterItem  **parm;
{
  return (dec_f(_ZFilterItemSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_FilterItem
void free_STRONG_FilterItem(val)
SFilterItem  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZFilterItemSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFFilter
int	parse_STRONG_SETOFFilter(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_SFilter  **parm;
{
  return (dec_f(_ZSETOFFilterSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFFilter
void free_STRONG_SETOFFilter(val)
SET_OF_SFilter  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFFilterSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_Filter
int	parse_STRONG_Filter(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SFilter  **parm;
{
  return (dec_f(_ZFilterSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_Filter
void free_STRONG_Filter(val)
SFilter  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZFilterSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFSubordEntry
int	parse_STRONG_SETOFSubordEntry(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_SubordEntry  **parm;
{
  return (dec_f(_ZSETOFSubordEntrySTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFSubordEntry
void free_STRONG_SETOFSubordEntry(val)
SET_OF_SubordEntry  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFSubordEntrySTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SubordEntry
int	parse_STRONG_SubordEntry(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SubordEntry  **parm;
{
  return (dec_f(_ZSubordEntrySTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SubordEntry
void free_STRONG_SubordEntry(val)
SubordEntry  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSubordEntrySTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ListInfo
int	parse_STRONG_ListInfo(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ListInfo  **parm;
{
  return (dec_f(_ZListInfoSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ListInfo
void free_STRONG_ListInfo(val)
ListInfo  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZListInfoSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SearchInfo
int	parse_STRONG_SearchInfo(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SearchInfo  **parm;
{
  return (dec_f(_ZSearchInfoSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SearchInfo
void free_STRONG_SearchInfo(val)
SearchInfo  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSearchInfoSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFDName
int	parse_STRONG_SETOFDName(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_DName  **parm;
{
  return (dec_f(_ZSETOFDNameSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFDName
void free_STRONG_SETOFDName(val)
SET_OF_DName  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFDNameSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_PartialOutcomeQualifier
int	parse_STRONG_PartialOutcomeQualifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
PartialOutQual  **parm;
{
  return (dec_f(_ZPartialOutcomeQualifierSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_PartialOutcomeQualifier
void free_STRONG_PartialOutcomeQualifier(val)
PartialOutQual  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZPartialOutcomeQualifierSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ReferenceType
int	parse_STRONG_ReferenceType(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_STRONG_ReferenceType **parm;
{
  return (dec_f(_ZReferenceTypeSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ReferenceType
void free_STRONG_ReferenceType(val)
struct type_STRONG_ReferenceType *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZReferenceTypeSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_LimitProblem
int	parse_STRONG_LimitProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_STRONG_LimitProblem **parm;
{
  return (dec_f(_ZLimitProblemSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_LimitProblem
void free_STRONG_LimitProblem(val)
struct type_STRONG_LimitProblem *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZLimitProblemSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFContinuationReference
int	parse_STRONG_SETOFContinuationReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_ContReference  **parm;
{
  return (dec_f(_ZSETOFContinuationReferenceSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFContinuationReference
void free_STRONG_SETOFContinuationReference(val)
SET_OF_ContReference  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFContinuationReferenceSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_ContinuationReference
int	parse_STRONG_ContinuationReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ContReference  **parm;
{
  return (dec_f(_ZContinuationReferenceSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_ContinuationReference
void free_STRONG_ContinuationReference(val)
ContReference  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZContinuationReferenceSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_SETOFAccessPoint
int	parse_STRONG_SETOFAccessPoint(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_AccessPoint  **parm;
{
  return (dec_f(_ZSETOFAccessPointSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_SETOFAccessPoint
void free_STRONG_SETOFAccessPoint(val)
SET_OF_AccessPoint  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZSETOFAccessPointSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_AccessPoint
int	parse_STRONG_AccessPoint(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AccessPoint  **parm;
{
  return (dec_f(_ZAccessPointSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_AccessPoint
void free_STRONG_AccessPoint(val)
AccessPoint  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZAccessPointSTRONG], &_ZSTRONG_mod, 1);
}

#undef parse_STRONG_PSAPaddr
int	parse_STRONG_PSAPaddr(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
typeDSE_PSAPaddr  **parm;
{
  return (dec_f(_ZPSAPaddrSTRONG, &_ZSTRONG_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_STRONG_PSAPaddr
void free_STRONG_PSAPaddr(val)
typeDSE_PSAPaddr  *val;
{
(void) fre_obj((char *) val, _ZSTRONG_mod.md_dtab[_ZPSAPaddrSTRONG], &_ZSTRONG_mod, 1);
}

#endif	/* lint||PEPSY_LINKABLE_FUNCS */

#else

/* dummy */
/***************************************************************
*
* Procedure strong_tables_dummy
*
***************************************************************/

int strong_tables_dummy()

{
return(0);
}

#endif

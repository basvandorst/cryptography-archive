#include <stdio.h>
#include "SECAF-types.h"


# line 22 "af.py"
	/* surrounding global definitions */
#include        "af.h"
	
	static OctetString * tmp_ostr, * save_serialnumber;
	static OctetString * nullostr = NULLOCTETSTRING;
	static int i,j;

extern caddr_t _ZptrtabSECAF[];	/* forward decl */

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static ptpe et_TCertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[0] },
	{ OBJECT, OFFSET(SEQUENCE_OF_CertificatePair , element), _ZCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[0] }
	};

static ptpe et_ValiditySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[1] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[1] },
	{ T_STRING, OFFSET(Validity , notbefore), 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(Validity , notafter), 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[1] }
	};

static ptpe et_PKRootSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[2] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[2] },
	{ EXTOBJ, OFFSET(PKRoot , ca), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[3] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[3] },
	{ OBJECT, OFFSET(PKRoot , newkey), _ZRootKeySECAF, 0, (char **)&_ZptrtabSECAF[5] },
	{ BOPTIONAL, 0, 0, FL_USELECT, (char **)&_ZptrtabSECAF[6]},
	{ OBJECT, OFFSET(PKRoot , oldkey), _ZRootKeySECAF, 0, (char **)&_ZptrtabSECAF[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[2] }
	};

static ptpe et_RootKeySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[7] },
	{ UCODE, 1, 0, 0, (char **)&_ZptrtabSECAF[7] }, /* line 75 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[7] },
	{ DFLT_F,	0,	0,	0, (char **)&_ZptrtabSECAF[8] },
	{ ETAG, 0, 0, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSECAF[8] },
	{ INTEGER, OFFSET(struct Serial , version), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSECAF[8] },
	{ EXTOBJ, OFFSET(struct Serial , serial), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[9] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[9] },
	{ EXTOBJ, OFFSET(struct Serial , key), _ZKeyInfoSEC, 0, (char **)&_ZptrtabSECAF[11] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[11] },
	{ BOPTIONAL, 2, 0, FL_USELECT, (char **)&_ZptrtabSECAF[12]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSECAF[12] },
	{ OBJECT, OFFSET(struct Serial , valid), _ZValiditySECAF, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[12] },
	{ BOPTIONAL, 3, 0, FL_USELECT, (char **)&_ZptrtabSECAF[13]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSECAF[13] },
	{ EXTOBJ, OFFSET(struct Serial , sig), _ZTAlgorithmIdentifierSEC, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[13] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[13] },
	{ BOPTIONAL, 4, 0, FL_USELECT, (char **)&_ZptrtabSECAF[14]},
	{ EXTOBJ, OFFSET(struct Serial , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSECAF[14] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[14] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[7] },
	{ UCODE, 5, 0, 0, (char **)&_ZptrtabSECAF[7] }, /* line 130 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[7] }
	};

static ptpe et_TBSCertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[15] },
	{ UCODE, 6, 0, 0, (char **)&_ZptrtabSECAF[15] }, /* line 155 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[15] },
	{ BOPTIONAL, 7, 0, FL_USELECT, (char **)&_ZptrtabSECAF[8]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSECAF[8] },
	{ INTEGER, OFFSET(ToBeSigned , version), 2, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[8] },
	{ EXTOBJ, OFFSET(ToBeSigned , serialnumber), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTOBJ, OFFSET(ToBeSigned , signatureAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTOBJ, OFFSET(ToBeSigned , issuer), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[18] },
	{ OBJECT, OFFSET(ToBeSigned , valid), _ZValiditySECAF, 0, (char **)&_ZptrtabSECAF[12] },
	{ EXTOBJ, OFFSET(ToBeSigned , subject), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[19] },
	{ EXTOBJ, OFFSET(ToBeSigned , subjectPK), _ZKeyInfoSEC, 0, (char **)&_ZptrtabSECAF[20] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[20] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[15] },
	{ UCODE, 8, 0, 0, (char **)&_ZptrtabSECAF[15] }, /* line 200 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[15] }
	};

static ptpe et_CertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[21] },
	{ OBJECT, OFFSET(Certificate , tbs), _ZTBSCertificateSECAF, 0, (char **)&_ZptrtabSECAF[21] },
	{ EXTOBJ, OFFSET(Certificate , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[21] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ EXTOBJ, OFFSET(Certificate , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSECAF[21] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[21] }
	};

static ptpe et_CertificatesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[22] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[22] },
	{ OBJECT, OFFSET(Certificates , usercertificate), _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[23] },
	{ BOPTIONAL, 9, 0, FL_USELECT, (char **)&_ZptrtabSECAF[24]},
	{ OBJECT, OFFSET(Certificates , forwardpath), _ZForwardCertificationPathSECAF, 0, (char **)&_ZptrtabSECAF[24] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[22] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[22] }
	};

static ptpe et_ForwardCertificationPathSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[25] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[25] },
	{ OBJECT, OFFSET(FCPath , liste), _ZCertificateSetSECAF, 0, (char **)&_ZptrtabSECAF[25] },
	{ PE_END, OFFSET(FCPath , next_forwardpath), 0, 0, (char **)&_ZptrtabSECAF[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[25] }
	};

static ptpe et_CertificateSetSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[26] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[26] },
	{ OBJECT, OFFSET(SET_OF_Certificate , element), _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[26] },
	{ PE_END, OFFSET(SET_OF_Certificate , next), 0, 0, (char **)&_ZptrtabSECAF[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[26] }
	};

static ptpe et_CertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[27] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[27] },
	{ BOPTIONAL, 10, 0, FL_USELECT, (char **)&_ZptrtabSECAF[28]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSECAF[28] },
	{ OBJECT, OFFSET(CertificatePair , forward), _ZCertificateSECAF, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[28] },
	{ BOPTIONAL, 11, 0, FL_USELECT, (char **)&_ZptrtabSECAF[29]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSECAF[29] },
	{ OBJECT, OFFSET(CertificatePair , reverse), _ZCertificateSECAF, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[29] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[27] }
	};

static ptpe et_CrossCertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[30] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[30] },
	{ OBJECT, OFFSET(SET_OF_CertificatePair , element), _ZCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[30] },
	{ PE_END, OFFSET(SET_OF_CertificatePair , next), 0, 0, (char **)&_ZptrtabSECAF[30] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[30] }
	};

static ptpe et_PKListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[31] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[31] },
	{ OBJECT, OFFSET(PKList , element), _ZTBSCertificateSECAF, 0, (char **)&_ZptrtabSECAF[31] },
	{ PE_END, OFFSET(PKList , next), 0, 0, (char **)&_ZptrtabSECAF[31] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[31] }
	};

static ptpe et_TBSCRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[32] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[32] },
	{ EXTOBJ, OFFSET(CRLTBS , signatureAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTOBJ, OFFSET(CRLTBS , issuer), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[18] },
	{ T_STRING, OFFSET(CRLTBS , lastUpdate), 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(CRLTBS , nextUpdate), 23, FL_UNIVERSAL, NULLVP },
	{ BOPTIONAL, 12, 0, FL_USELECT, (char **)&_ZptrtabSECAF[33]},
	{ OBJECT, OFFSET(CRLTBS , revokedCertificates), _ZCRLEntrySequenceSECAF, 0, (char **)&_ZptrtabSECAF[33] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[32] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[32] }
	};

static ptpe et_CRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[34] },
	{ OBJECT, OFFSET(CRL , tbs), _ZTBSCRLSECAF, 0, (char **)&_ZptrtabSECAF[34] },
	{ EXTOBJ, OFFSET(CRL , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[34] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ EXTOBJ, OFFSET(CRL , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSECAF[34] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[34] }
	};

static ptpe et_SETOFCRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[35] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[35] },
	{ OBJECT, OFFSET(SET_OF_CRL , element), _ZCRLSECAF, 0, (char **)&_ZptrtabSECAF[35] },
	{ PE_END, OFFSET(SET_OF_CRL , next), 0, 0, (char **)&_ZptrtabSECAF[35] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[35] }
	};

static ptpe et_CRLEntrySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[36] },
	{ UCODE, 13, 0, 0, (char **)&_ZptrtabSECAF[36] }, /* line 368 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[36] },
	{ EXTOBJ, OFFSET(CRLEntry , serialnumber), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[16] },
	{ T_STRING, OFFSET(CRLEntry , revocationDate), 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[36] },
	{ UCODE, 14, 0, 0, (char **)&_ZptrtabSECAF[36] }, /* line 392 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[36] }
	};

static ptpe et_CRLEntrySequenceSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[37] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[37] },
	{ OBJECT, OFFSET(SEQUENCE_OF_CRLEntry , element), _ZCRLEntrySECAF, 0, (char **)&_ZptrtabSECAF[37] },
	{ PE_END, OFFSET(SEQUENCE_OF_CRLEntry , next), 0, 0, (char **)&_ZptrtabSECAF[37] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[37] }
	};

static ptpe et_CrlSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[38] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[38] },
	{ EXTOBJ, OFFSET(Crl , issuer), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[18] },
	{ T_STRING, OFFSET(Crl , nextUpdate), 23, FL_UNIVERSAL, NULLVP },
	{ BOPTIONAL, 15, 0, FL_USELECT, (char **)&_ZptrtabSECAF[39]},
	{ OBJECT, OFFSET(Crl , revcerts), _ZCRLEntrySequenceSECAF, 0, (char **)&_ZptrtabSECAF[39] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[38] }
	};

static ptpe et_CrlSetSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[40] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[40] },
	{ OBJECT, OFFSET(CrlSet , element), _ZCrlSECAF, 0, (char **)&_ZptrtabSECAF[40] },
	{ PE_END, OFFSET(CrlSet , next), 0, 0, (char **)&_ZptrtabSECAF[40] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[40] }
	};

static ptpe et_CertificationPathSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[41] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[41] },
	{ OBJECT, OFFSET(CertificationPath , userCertificate), _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[42] },
	{ BOPTIONAL, 16, 0, FL_USELECT, (char **)&_ZptrtabSECAF[43]},
	{ OBJECT, OFFSET(CertificationPath , theCACertificates), _ZCertificatePairsSECAF, 0, (char **)&_ZptrtabSECAF[43] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[41] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[41] }
	};

static ptpe et_CRLWithCertificatesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[44] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[44] },
	{ OBJECT, OFFSET(CRLWithCertificates , crl), _ZCRLSECAF, 0, (char **)&_ZptrtabSECAF[45] },
	{ BOPTIONAL, 17, 0, FL_USELECT, (char **)&_ZptrtabSECAF[46]},
	{ OBJECT, OFFSET(CRLWithCertificates , certificates), _ZCertificatesSECAF, 0, (char **)&_ZptrtabSECAF[46] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[44] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[44] }
	};

static ptpe et_AliasesMemberSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[47] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[47] },
	{ T_STRING, OFFSET(Aliases , aname), 22, FL_UNIVERSAL, NULLVP },
	{ INTEGER, OFFSET(Aliases , aliasfile), 10, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[48] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[47] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[47] }
	};

static ptpe et_AliasesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[49] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[49] },
	{ SOBJECT, 0, _ZAliasesMemberSECAF, 0, (char **)&_ZptrtabSECAF[49] },
	{ PE_END, OFFSET(Aliases , next), 0, 0, (char **)&_ZptrtabSECAF[49] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[49] }
	};

static ptpe et_AliasListMemberSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[50] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[50] },
	{ OBJECT, OFFSET(AliasList , a), _ZAliasesSECAF, 0, (char **)&_ZptrtabSECAF[51] },
	{ T_STRING, OFFSET(AliasList , dname), 22, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[50] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[50] }
	};

static ptpe et_AliasListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[52] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[52] },
	{ SOBJECT, 0, _ZAliasListMemberSECAF, 0, (char **)&_ZptrtabSECAF[52] },
	{ PE_END, OFFSET(AliasList , next), 0, 0, (char **)&_ZptrtabSECAF[52] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[52] }
	};

static ptpe et_OldCertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[53] },
	{ UCODE, 18, 0, 0, (char **)&_ZptrtabSECAF[53] }, /* line 507 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[53] },
	{ EXTOBJ, OFFSET(OCList , serialnumber), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[16] },
	{ OBJECT, OFFSET(OCList , ccert), _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[54] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[53] },
	{ UCODE, 19, 0, 0, (char **)&_ZptrtabSECAF[53] }, /* line 531 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[53] }
	};

static ptpe et_OldCertificateListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[55] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[55] },
	{ SOBJECT, 0, _ZOldCertificateSECAF, 0, (char **)&_ZptrtabSECAF[55] },
	{ PE_END, OFFSET(OCList , next), 0, 0, (char **)&_ZptrtabSECAF[55] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[55] }
	};

static ptpe et_CertificatePairsSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[56] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[56] },
	{ OBJECT, OFFSET(CertificatePairs , liste), _ZTCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[56] },
	{ PE_END, OFFSET(CertificatePairs , superior), 0, 0, (char **)&_ZptrtabSECAF[56] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[56] }
	};

static ptpe dt_TCertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[0] },
	{ OBJECT, OFFSET(SEQUENCE_OF_CertificatePair , element), _ZCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[0] }
	};

static ptpe dt_ValiditySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[1] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[1] },
	{ MEMALLOC, 0, sizeof (Validity ), 0, (char **)&_ZptrtabSECAF[1] },
	{ T_STRING, OFFSET(Validity , notbefore), 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(Validity , notafter), 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[1] }
	};

static ptpe dt_PKRootSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[2] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[2] },
	{ MEMALLOC, 0, sizeof (PKRoot ), 0, (char **)&_ZptrtabSECAF[2] },
	{ EXTOBJ, OFFSET(PKRoot , ca), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[3] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[3] },
	{ OBJECT, OFFSET(PKRoot , newkey), _ZRootKeySECAF, 0, (char **)&_ZptrtabSECAF[5] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[6]},
	{ OBJECT, OFFSET(PKRoot , oldkey), _ZRootKeySECAF, 0, (char **)&_ZptrtabSECAF[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[2] }
	};

static ptpe dt_RootKeySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[7] },
	{ UCODE, 1, 0, 0, (char **)&_ZptrtabSECAF[7] }, /* line 91 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[7] },
	{ MEMALLOC, 0, sizeof (struct Serial ), 0, (char **)&_ZptrtabSECAF[7] },
	{ ETAG, 0, 0, FL_CONTEXT|FL_DEFAULT, (char **)&_ZptrtabSECAF[8] },
	{ INTEGER, OFFSET(struct Serial , version), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabSECAF[8] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSECAF[8] },
	{ EXTOBJ, OFFSET(struct Serial , serial), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[9] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[9] },
	{ EXTOBJ, OFFSET(struct Serial , key), _ZKeyInfoSEC, 0, (char **)&_ZptrtabSECAF[11] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[11] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[12]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSECAF[12] },
	{ OBJECT, OFFSET(struct Serial , valid), _ZValiditySECAF, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[12] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[13]},
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSECAF[13] },
	{ EXTOBJ, OFFSET(struct Serial , sig), _ZTAlgorithmIdentifierSEC, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[13] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[13] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[14]},
	{ EXTOBJ, OFFSET(struct Serial , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSECAF[14] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[14] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[7] },
	{ UCODE, 5, 0, 0, (char **)&_ZptrtabSECAF[7] }, /* line 136 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[7] }
	};

static ptpe dt_TBSCertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[15] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[15] },
	{ MEMALLOC, 0, sizeof (ToBeSigned ), 0, (char **)&_ZptrtabSECAF[15] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[8]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSECAF[8] },
	{ INTEGER, OFFSET(ToBeSigned , version), 2, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[8] },
	{ EXTOBJ, OFFSET(ToBeSigned , serialnumber), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTOBJ, OFFSET(ToBeSigned , signatureAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTOBJ, OFFSET(ToBeSigned , issuer), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[18] },
	{ OBJECT, OFFSET(ToBeSigned , valid), _ZValiditySECAF, 0, (char **)&_ZptrtabSECAF[12] },
	{ EXTOBJ, OFFSET(ToBeSigned , subject), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[19] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[19] },
	{ EXTOBJ, OFFSET(ToBeSigned , subjectPK), _ZKeyInfoSEC, 0, (char **)&_ZptrtabSECAF[20] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[20] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[15] },
	{ UCODE, 7, 0, 0, (char **)&_ZptrtabSECAF[15] }, /* line 206 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[15] }
	};

static ptpe dt_CertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ UCODE, 8, 0, 0, (char **)&_ZptrtabSECAF[21] }, /* line 221 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[21] },
	{ MEMALLOC, 0, sizeof (Certificate ), 0, (char **)&_ZptrtabSECAF[21] },
	{ OBJECT, OFFSET(Certificate , tbs), _ZTBSCertificateSECAF, 0, (char **)&_ZptrtabSECAF[21] },
	{ UCODE, 9, 0, 0, (char **)&_ZptrtabSECAF[21] }, /* line 235 */
	{ EXTOBJ, OFFSET(Certificate , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[21] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ EXTOBJ, OFFSET(Certificate , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSECAF[21] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[21] }
	};

static ptpe dt_CertificatesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[22] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[22] },
	{ MEMALLOC, 0, sizeof (Certificates ), 0, (char **)&_ZptrtabSECAF[22] },
	{ OBJECT, OFFSET(Certificates , usercertificate), _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[23] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[24]},
	{ OBJECT, OFFSET(Certificates , forwardpath), _ZForwardCertificationPathSECAF, 0, (char **)&_ZptrtabSECAF[24] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[22] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[22] }
	};

static ptpe dt_ForwardCertificationPathSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[25] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[25] },
	{ MEMALLOC, 0, sizeof (FCPath ), 0, (char **)&_ZptrtabSECAF[25] },
	{ OBJECT, OFFSET(FCPath , liste), _ZCertificateSetSECAF, 0, (char **)&_ZptrtabSECAF[25] },
	{ PE_END, OFFSET(FCPath , next_forwardpath), 0, 0, (char **)&_ZptrtabSECAF[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[25] }
	};

static ptpe dt_CertificateSetSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[26] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[26] },
	{ MEMALLOC, 0, sizeof (SET_OF_Certificate ), 0, (char **)&_ZptrtabSECAF[26] },
	{ OBJECT, OFFSET(SET_OF_Certificate , element), _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[26] },
	{ PE_END, OFFSET(SET_OF_Certificate , next), 0, 0, (char **)&_ZptrtabSECAF[26] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[26] }
	};

static ptpe dt_CertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[27] },
	{ UCODE, 11, 0, 0, (char **)&_ZptrtabSECAF[27] }, /* line 273 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[27] },
	{ MEMALLOC, 0, sizeof (CertificatePair ), 0, (char **)&_ZptrtabSECAF[27] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[28]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSECAF[28] },
	{ OBJECT, OFFSET(CertificatePair , forward), _ZCertificateSECAF, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[28] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[29]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSECAF[29] },
	{ OBJECT, OFFSET(CertificatePair , reverse), _ZCertificateSECAF, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[29] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[27] },
	{ UCODE, 14, 0, 0, (char **)&_ZptrtabSECAF[27] }, /* line 289 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[27] }
	};

static ptpe dt_CrossCertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[30] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[30] },
	{ MEMALLOC, 0, sizeof (SET_OF_CertificatePair ), 0, (char **)&_ZptrtabSECAF[30] },
	{ OBJECT, OFFSET(SET_OF_CertificatePair , element), _ZCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[30] },
	{ PE_END, OFFSET(SET_OF_CertificatePair , next), 0, 0, (char **)&_ZptrtabSECAF[30] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[30] }
	};

static ptpe dt_PKListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[31] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[31] },
	{ MEMALLOC, 0, sizeof (PKList ), 0, (char **)&_ZptrtabSECAF[31] },
	{ OBJECT, OFFSET(PKList , element), _ZTBSCertificateSECAF, 0, (char **)&_ZptrtabSECAF[31] },
	{ PE_END, OFFSET(PKList , next), 0, 0, (char **)&_ZptrtabSECAF[31] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[31] }
	};

static ptpe dt_TBSCRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[32] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[32] },
	{ MEMALLOC, 0, sizeof (CRLTBS ), 0, (char **)&_ZptrtabSECAF[32] },
	{ EXTOBJ, OFFSET(CRLTBS , signatureAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTOBJ, OFFSET(CRLTBS , issuer), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[18] },
	{ T_STRING, OFFSET(CRLTBS , lastUpdate), 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(CRLTBS , nextUpdate), 23, FL_UNIVERSAL, NULLVP },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[33]},
	{ OBJECT, OFFSET(CRLTBS , revokedCertificates), _ZCRLEntrySequenceSECAF, 0, (char **)&_ZptrtabSECAF[33] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[32] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[32] }
	};

static ptpe dt_CRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ UCODE, 16, 0, 0, (char **)&_ZptrtabSECAF[34] }, /* line 333 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[34] },
	{ MEMALLOC, 0, sizeof (CRL ), 0, (char **)&_ZptrtabSECAF[34] },
	{ OBJECT, OFFSET(CRL , tbs), _ZTBSCRLSECAF, 0, (char **)&_ZptrtabSECAF[34] },
	{ UCODE, 17, 0, 0, (char **)&_ZptrtabSECAF[34] }, /* line 347 */
	{ EXTOBJ, OFFSET(CRL , sig), _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[34] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ EXTOBJ, OFFSET(CRL , sig), _ZTBitStringSEC, 0, (char **)&_ZptrtabSECAF[34] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[34] }
	};

static ptpe dt_SETOFCRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[35] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[35] },
	{ MEMALLOC, 0, sizeof (SET_OF_CRL ), 0, (char **)&_ZptrtabSECAF[35] },
	{ OBJECT, OFFSET(SET_OF_CRL , element), _ZCRLSECAF, 0, (char **)&_ZptrtabSECAF[35] },
	{ PE_END, OFFSET(SET_OF_CRL , next), 0, 0, (char **)&_ZptrtabSECAF[35] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[35] }
	};

static ptpe dt_CRLEntrySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[36] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[36] },
	{ MEMALLOC, 0, sizeof (CRLEntry ), 0, (char **)&_ZptrtabSECAF[36] },
	{ EXTOBJ, OFFSET(CRLEntry , serialnumber), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[16] },
	{ T_STRING, OFFSET(CRLEntry , revocationDate), 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[36] },
	{ UCODE, 18, 0, 0, (char **)&_ZptrtabSECAF[36] }, /* line 398 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[36] }
	};

static ptpe dt_CRLEntrySequenceSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[37] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[37] },
	{ MEMALLOC, 0, sizeof (SEQUENCE_OF_CRLEntry ), 0, (char **)&_ZptrtabSECAF[37] },
	{ OBJECT, OFFSET(SEQUENCE_OF_CRLEntry , element), _ZCRLEntrySECAF, 0, (char **)&_ZptrtabSECAF[37] },
	{ PE_END, OFFSET(SEQUENCE_OF_CRLEntry , next), 0, 0, (char **)&_ZptrtabSECAF[37] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[37] }
	};

static ptpe dt_CrlSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[38] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[38] },
	{ MEMALLOC, 0, sizeof (Crl ), 0, (char **)&_ZptrtabSECAF[38] },
	{ EXTOBJ, OFFSET(Crl , issuer), _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, (char **)&_ZptrtabSECAF[18] },
	{ T_STRING, OFFSET(Crl , nextUpdate), 23, FL_UNIVERSAL, NULLVP },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[39]},
	{ OBJECT, OFFSET(Crl , revcerts), _ZCRLEntrySequenceSECAF, 0, (char **)&_ZptrtabSECAF[39] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[38] }
	};

static ptpe dt_CrlSetSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[40] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[40] },
	{ MEMALLOC, 0, sizeof (CrlSet ), 0, (char **)&_ZptrtabSECAF[40] },
	{ OBJECT, OFFSET(CrlSet , element), _ZCrlSECAF, 0, (char **)&_ZptrtabSECAF[40] },
	{ PE_END, OFFSET(CrlSet , next), 0, 0, (char **)&_ZptrtabSECAF[40] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[40] }
	};

static ptpe dt_CertificationPathSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[41] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[41] },
	{ MEMALLOC, 0, sizeof (CertificationPath ), 0, (char **)&_ZptrtabSECAF[41] },
	{ OBJECT, OFFSET(CertificationPath , userCertificate), _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[42] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[43]},
	{ OBJECT, OFFSET(CertificationPath , theCACertificates), _ZCertificatePairsSECAF, 0, (char **)&_ZptrtabSECAF[43] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[41] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[41] }
	};

static ptpe dt_CRLWithCertificatesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[44] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[44] },
	{ MEMALLOC, 0, sizeof (CRLWithCertificates ), 0, (char **)&_ZptrtabSECAF[44] },
	{ OBJECT, OFFSET(CRLWithCertificates , crl), _ZCRLSECAF, 0, (char **)&_ZptrtabSECAF[45] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSECAF[46]},
	{ OBJECT, OFFSET(CRLWithCertificates , certificates), _ZCertificatesSECAF, 0, (char **)&_ZptrtabSECAF[46] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[44] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[44] }
	};

static ptpe dt_AliasesMemberSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[47] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[47] },
	{ MEMALLOC, 0, sizeof (Aliases ), 0, (char **)&_ZptrtabSECAF[47] },
	{ T_STRING, OFFSET(Aliases , aname), 22, FL_UNIVERSAL, NULLVP },
	{ INTEGER, OFFSET(Aliases , aliasfile), 10, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[48] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[47] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[47] }
	};

static ptpe dt_AliasesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[49] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[49] },
	{ MEMALLOC, 0, sizeof (Aliases ), 0, (char **)&_ZptrtabSECAF[49] },
	{ SOBJECT, 0, _ZAliasesMemberSECAF, 0, (char **)&_ZptrtabSECAF[49] },
	{ PE_END, OFFSET(Aliases , next), 0, 0, (char **)&_ZptrtabSECAF[49] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[49] }
	};

static ptpe dt_AliasListMemberSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[50] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[50] },
	{ MEMALLOC, 0, sizeof (AliasList ), 0, (char **)&_ZptrtabSECAF[50] },
	{ OBJECT, OFFSET(AliasList , a), _ZAliasesSECAF, 0, (char **)&_ZptrtabSECAF[51] },
	{ T_STRING, OFFSET(AliasList , dname), 22, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[50] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[50] }
	};

static ptpe dt_AliasListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[52] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[52] },
	{ MEMALLOC, 0, sizeof (AliasList ), 0, (char **)&_ZptrtabSECAF[52] },
	{ SOBJECT, 0, _ZAliasListMemberSECAF, 0, (char **)&_ZptrtabSECAF[52] },
	{ PE_END, OFFSET(AliasList , next), 0, 0, (char **)&_ZptrtabSECAF[52] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[52] }
	};

static ptpe dt_OldCertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[53] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[53] },
	{ MEMALLOC, 0, sizeof (OCList ), 0, (char **)&_ZptrtabSECAF[53] },
	{ EXTOBJ, OFFSET(OCList , serialnumber), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, (char **)&_ZptrtabSECAF[16] },
	{ OBJECT, OFFSET(OCList , ccert), _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[54] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[53] },
	{ UCODE, 22, 0, 0, (char **)&_ZptrtabSECAF[53] }, /* line 537 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[53] }
	};

static ptpe dt_OldCertificateListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[55] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[55] },
	{ MEMALLOC, 0, sizeof (OCList ), 0, (char **)&_ZptrtabSECAF[55] },
	{ SOBJECT, 0, _ZOldCertificateSECAF, 0, (char **)&_ZptrtabSECAF[55] },
	{ PE_END, OFFSET(OCList , next), 0, 0, (char **)&_ZptrtabSECAF[55] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[55] }
	};

static ptpe dt_CertificatePairsSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[56] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[56] },
	{ MEMALLOC, 0, sizeof (CertificatePairs ), 0, (char **)&_ZptrtabSECAF[56] },
	{ OBJECT, OFFSET(CertificatePairs , liste), _ZTCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[56] },
	{ PE_END, OFFSET(CertificatePairs , superior), 0, 0, (char **)&_ZptrtabSECAF[56] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[56] }
	};

static ptpe pt_TCertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[0] },
	{ OBJECT, 0, _ZCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[0] }
	};

static ptpe pt_ValiditySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[1] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[1] }
	};

static ptpe pt_PKRootSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[2] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[3] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ OBJECT, 0, _ZRootKeySECAF, 0, (char **)&_ZptrtabSECAF[5] },
	{ OBJECT, 0, _ZRootKeySECAF, FL_OPTIONAL, (char **)&_ZptrtabSECAF[6] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[2] }
	};

static ptpe pt_RootKeySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[7] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, (char **)&_ZptrtabSECAF[8] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_DEFAULT, NULLVP },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabSECAF[8] },
	{ EXTOBJ, 0, _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[9] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZKeyInfoSEC, 0, (char **)&_ZptrtabSECAF[11] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSECAF[12] },
	{ OBJECT, 0, _ZValiditySECAF, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSECAF[13] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, FL_OPTIONAL, (char **)&_ZptrtabSECAF[14] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[7] }
	};

static ptpe pt_TBSCertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[15] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSECAF[8] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ EXTOBJ, 0, _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ OBJECT, 0, _ZValiditySECAF, 0, (char **)&_ZptrtabSECAF[12] },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[19] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZKeyInfoSEC, 0, (char **)&_ZptrtabSECAF[20] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[15] }
	};

static ptpe pt_CertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[21] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZTBSCertificateSECAF, 0, (char **)&_ZptrtabSECAF[15] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[57] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSECAF[58] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[21] }
	};

static ptpe pt_CertificatesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[22] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[23] },
	{ OBJECT, 0, _ZForwardCertificationPathSECAF, FL_OPTIONAL, (char **)&_ZptrtabSECAF[24] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[22] }
	};

static ptpe pt_ForwardCertificationPathSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[25] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCertificateSetSECAF, 0, (char **)&_ZptrtabSECAF[26] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[25] }
	};

static ptpe pt_CertificateSetSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[26] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[21] },
	{ PE_END, OFFSET(SET_OF_Certificate , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[26] }
	};

static ptpe pt_CertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[27] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSECAF[28] },
	{ OBJECT, 0, _ZCertificateSECAF, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSECAF[29] },
	{ OBJECT, 0, _ZCertificateSECAF, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[27] }
	};

static ptpe pt_CrossCertificatePairSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[30] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[27] },
	{ PE_END, OFFSET(SET_OF_CertificatePair , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[30] }
	};

static ptpe pt_PKListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[31] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZTBSCertificateSECAF, 0, (char **)&_ZptrtabSECAF[15] },
	{ PE_END, OFFSET(PKList , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[31] }
	};

static ptpe pt_TBSCRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[32] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[17] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCRLEntrySequenceSECAF, FL_OPTIONAL, (char **)&_ZptrtabSECAF[33] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[32] }
	};

static ptpe pt_CRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[34] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZTBSCRLSECAF, 0, (char **)&_ZptrtabSECAF[32] },
	{ EXTOBJ, 0, _ZTAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSECAF[57] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ EXTOBJ, 0, _ZTBitStringSEC, 0, (char **)&_ZptrtabSECAF[58] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[34] }
	};

static ptpe pt_SETOFCRLSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[35] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCRLSECAF, 0, (char **)&_ZptrtabSECAF[34] },
	{ PE_END, OFFSET(SET_OF_CRL , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[35] }
	};

static ptpe pt_CRLEntrySECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[36] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[36] }
	};

static ptpe pt_CRLEntrySequenceSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[37] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCRLEntrySECAF, 0, (char **)&_ZptrtabSECAF[36] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[37] }
	};

static ptpe pt_CrlSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[38] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZNameSECIF, 0, (char **)&_ZptrtabSECAF[18] },
	{ EXTMOD, 4, 0, 0, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCRLEntrySequenceSECAF, FL_OPTIONAL, (char **)&_ZptrtabSECAF[39] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[38] }
	};

static ptpe pt_CrlSetSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[40] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCrlSECAF, 0, (char **)&_ZptrtabSECAF[38] },
	{ PE_END, OFFSET(CrlSet , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[40] }
	};

static ptpe pt_CertificationPathSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[41] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[42] },
	{ OBJECT, 0, _ZCertificatePairsSECAF, FL_OPTIONAL, (char **)&_ZptrtabSECAF[43] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[41] }
	};

static ptpe pt_CRLWithCertificatesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[44] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZCRLSECAF, 0, (char **)&_ZptrtabSECAF[45] },
	{ OBJECT, 0, _ZCertificatesSECAF, FL_OPTIONAL, (char **)&_ZptrtabSECAF[46] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[44] }
	};

static ptpe pt_AliasesMemberSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[47] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 22, FL_UNIVERSAL, NULLVP },
	{ INTEGER, -1, 10, FL_UNIVERSAL, (char **)&_ZptrtabSECAF[48] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[47] }
	};

static ptpe pt_AliasesSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[49] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ SOBJECT, 0, _ZAliasesMemberSECAF, 0, (char **)&_ZptrtabSECAF[47] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[49] }
	};

static ptpe pt_AliasListMemberSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[50] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAliasesSECAF, 0, (char **)&_ZptrtabSECAF[51] },
	{ T_STRING, -1, 22, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[50] }
	};

static ptpe pt_AliasListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[52] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ SOBJECT, 0, _ZAliasListMemberSECAF, 0, (char **)&_ZptrtabSECAF[50] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[52] }
	};

static ptpe pt_OldCertificateSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[53] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZOURINTEGERSEC, 0, (char **)&_ZptrtabSECAF[16] },
	{ EXTMOD, 10, 0, 0, NULLVP },
	{ OBJECT, 0, _ZCertificateSECAF, 0, (char **)&_ZptrtabSECAF[54] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[53] }
	};

static ptpe pt_OldCertificateListSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[55] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ SOBJECT, 0, _ZOldCertificateSECAF, 0, (char **)&_ZptrtabSECAF[53] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[55] }
	};

static ptpe pt_CertificatePairsSECAF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECAF[56] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZTCertificatePairSECAF, 0, (char **)&_ZptrtabSECAF[0] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECAF[56] }
	};


/*VARARGS*/
static	int
efn_SECAF(__p, ppe, _Zp)
caddr_t	__p;
PE	*ppe;
ptpe	*_Zp;
{
		
	/* 20 cases */
    switch(_Zp->pe_ucode) {

#define parm	((PKRoot * )__p)
	case 0: /*  */
		{
# line 69 "af.py"
		return (parm->oldkey != (struct Serial *)0);

		}
	    break;

#undef parm

#define parm	((struct Serial * )__p)
	case 1: /*  */
		{
# line 75 "af.py"

	tmp_ostr = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if(parm->serial && (parm->serial->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = aux_concatenate_OctetString(nullostr, parm->serial);
		save_serialnumber = parm->serial;
		parm->serial = tmp_ostr;
	}
	;
		}
	    break;

#undef parm

#define parm	((struct Serial * )__p)
	case 5: /*  */
		{
# line 130 "af.py"

	if(tmp_ostr) {
		aux_free_OctetString(&tmp_ostr);
		parm->serial = save_serialnumber;
	}
	;
		}
	    break;

#undef parm

#define parm	((struct Serial * )__p)
	case 2: /*  */
		{
# line 119 "af.py"
		return (parm->valid && parm->valid->notbefore != (UTCTime *)0);

		}
	    break;

#undef parm

#define parm	((struct Serial * )__p)
	case 3: /*  */
		{
# line 123 "af.py"
		return (parm->sig != (Signature *)0);

		}
	    break;

#undef parm

#define parm	((struct Serial * )__p)
	case 4: /*  */
		{
# line 128 "af.py"
		return (parm->sig != (Signature *)0);

		}
	    break;

#undef parm

#define parm	((ToBeSigned * )__p)
	case 6: /*  */
		{
# line 155 "af.py"

	tmp_ostr = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if(parm->serialnumber && (parm->serialnumber->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = aux_concatenate_OctetString(nullostr, parm->serialnumber);
		save_serialnumber = parm->serialnumber;
		parm->serialnumber = tmp_ostr;
	}
	;
		}
	    break;

#undef parm

#define parm	((ToBeSigned * )__p)
	case 8: /*  */
		{
# line 200 "af.py"

	if(tmp_ostr){
		aux_free_OctetString(&tmp_ostr);
		parm->serialnumber = save_serialnumber;
	}
	;
		}
	    break;

#undef parm

#define parm	((ToBeSigned * )__p)
	case 7: /*  */
		{
# line 180 "af.py"
		return (parm->version != 0);

		}
	    break;

#undef parm

#define parm	((Certificates * )__p)
	case 9: /*  */
		{
# line 255 "af.py"
		return (parm->forwardpath != (FCPath *)0);

		}
	    break;

#undef parm

#define parm	((CertificatePair * )__p)
	case 10: /*  */
		{
# line 283 "af.py"
		return (parm->forward != (Certificate *)0);

		}
	    break;

#undef parm

#define parm	((CertificatePair * )__p)
	case 11: /*  */
		{
# line 287 "af.py"
		return (parm->reverse != (Certificate *)0);

		}
	    break;

#undef parm

#define parm	((CRLTBS * )__p)
	case 12: /*  */
		{
# line 327 "af.py"
		return (parm->revokedCertificates != (SEQUENCE_OF_CRLEntry *)0);

		}
	    break;

#undef parm

#define parm	((CRLEntry * )__p)
	case 13: /*  */
		{
# line 368 "af.py"

	tmp_ostr = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if(parm->serialnumber && (parm->serialnumber->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = aux_concatenate_OctetString(nullostr, parm->serialnumber);
		save_serialnumber = parm->serialnumber;
		parm->serialnumber = tmp_ostr;
	}
	;
		}
	    break;

#undef parm

#define parm	((CRLEntry * )__p)
	case 14: /*  */
		{
# line 392 "af.py"

	if(tmp_ostr) {
		aux_free_OctetString(&tmp_ostr);
		parm->serialnumber = save_serialnumber;
	}
	;
		}
	    break;

#undef parm

#define parm	((Crl * )__p)
	case 15: /*  */
		{
# line 429 "af.py"
		return (parm->revcerts != (SEQUENCE_OF_CRLEntry *)0);

		}
	    break;

#undef parm

#define parm	((CertificationPath * )__p)
	case 16: /*  */
		{
# line 448 "af.py"
		return (parm->theCACertificates != (CertificatePairs *)0);

		}
	    break;

#undef parm

#define parm	((CRLWithCertificates * )__p)
	case 17: /*  */
		{
# line 461 "af.py"
		return (parm->certificates != (Certificates *)0);

		}
	    break;

#undef parm

#define parm	((OCList * )__p)
	case 18: /*  */
		{
# line 507 "af.py"

	tmp_ostr = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if(parm->serialnumber && (parm->serialnumber->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = aux_concatenate_OctetString(nullostr, parm->serialnumber);
		save_serialnumber = parm->serialnumber;
		parm->serialnumber = tmp_ostr;
	}
	;
		}
	    break;

#undef parm

#define parm	((OCList * )__p)
	case 19: /*  */
		{
# line 531 "af.py"

	if(tmp_ostr) {
		aux_free_OctetString(&tmp_ostr);
		parm->serialnumber = save_serialnumber;
	}
	;
		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, *ppe, "enf_SECAF:Bad table entry: %d",
             _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}

/*VARARGS*/
static	int
dfn_SECAF(__p, pe, _Zp, _val)
caddr_t	__p;
PE	pe;
ptpe	*_Zp;
int _val;
{
		
	/* 23 cases */
    switch(_Zp->pe_ucode) {

#define parm	((PKRoot ** )__p)
	case 0: /*  */
		{
# line 69 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((struct Serial ** )__p)
	case 1: /*  */
		{
# line 91 "af.py"

        if (((*parm) = (struct Serial *) malloc (sizeof (struct Serial))) == ((struct Serial *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	if (((*parm)->sig = (Signature *) malloc (sizeof (Signature))) == ((Signature *) 0)) {
		    advise (NULLCP, "out of memory");
		    return NOTOK;
	}
        ;
		}
	    break;

#undef parm

#define parm	((struct Serial ** )__p)
	case 5: /*  */
		{
# line 136 "af.py"

	if(! (*parm)->sig->signAI){
		aux_free_KeyInfo( (KeyInfo **) & ((*parm)->sig) );
		(*parm)->sig = (Signature * ) 0;
	}
	if((*parm)->serial->noctets > 1 && !((*parm)->serial->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serial->noctets - 1; i++)
			(*parm)->serial->octets[i] = (*parm)->serial->octets[i + 1];
		(*parm)->serial->noctets -= 1;
		(*parm)->serial->octets = (char *)realloc((*parm)->serial->octets, (*parm)->serial->noctets);
	}
	;
		}
	    break;

#undef parm

#define parm	((struct Serial ** )__p)
	case 2: /*  */
		{
# line 119 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((struct Serial ** )__p)
	case 3: /*  */
		{
# line 123 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((struct Serial ** )__p)
	case 4: /*  */
		{
# line 128 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((ToBeSigned ** )__p)
	case 7: /*  */
		{
# line 206 "af.py"

	if((*parm)->serialnumber->noctets > 1 && !((*parm)->serialnumber->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serialnumber->noctets - 1; i++)
			(*parm)->serialnumber->octets[i] = (*parm)->serialnumber->octets[i + 1];
		(*parm)->serialnumber->noctets -= 1;
		(*parm)->serialnumber->octets = (char *)realloc((*parm)->serialnumber->octets, (*parm)->serialnumber->noctets);
	}
	;
		}
	    break;

#undef parm

#define parm	((ToBeSigned ** )__p)
	case 6: /*  */
		{
# line 180 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((Certificate ** )__p)
	case 8: /*  */
		{
# line 221 "af.py"

        if (((*parm) = (Certificate *) malloc (sizeof (Certificate))) == ((Certificate *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	if (((*parm)->sig = (Signature *) malloc (sizeof (Signature))) == ((Signature *) 0)) {
		advise (NULLCP, "out of memory");
		return NOTOK;
	}
        ;
		}
	    break;

#undef parm

#define parm	((Certificate ** )__p)
	case 9: /*  */
		{
# line 235 "af.py"

		(*parm)->tbs_DERcode = e_ToBeSigned((*parm)->tbs);
		;
		}
	    break;

#undef parm

#define parm	((Certificates ** )__p)
	case 10: /*  */
		{
# line 255 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((CertificatePair ** )__p)
	case 11: /*  */
		{
# line 273 "af.py"

        if (((*parm) = (CertificatePair *) malloc (sizeof (CertificatePair))) == ((CertificatePair *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((CertificatePair ** )__p)
	case 14: /*  */
		{
# line 289 "af.py"

        if (!(*parm)->forward && !(*parm)->reverse) {
            	advise (NULLCP, "at least one certificate must be present");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((CertificatePair ** )__p)
	case 12: /*  */
		{
# line 283 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((CertificatePair ** )__p)
	case 13: /*  */
		{
# line 287 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((CRLTBS ** )__p)
	case 15: /*  */
		{
# line 327 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((CRL ** )__p)
	case 16: /*  */
		{
# line 333 "af.py"

        if (((*parm) = (CRL *) malloc (sizeof (CRL))) == ((CRL *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	if (((*parm)->sig = (Signature *) malloc (sizeof (Signature))) == ((Signature *) 0)) {
		advise (NULLCP, "out of memory");
		return NOTOK;
	}
        ;
		}
	    break;

#undef parm

#define parm	((CRL ** )__p)
	case 17: /*  */
		{
# line 347 "af.py"

		(*parm)->tbs_DERcode = e_CRLTBS((*parm)->tbs);
		;
		}
	    break;

#undef parm

#define parm	((CRLEntry ** )__p)
	case 18: /*  */
		{
# line 398 "af.py"

	if((*parm)->serialnumber->noctets > 1 && !((*parm)->serialnumber->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serialnumber->noctets - 1; i++)
			(*parm)->serialnumber->octets[i] = (*parm)->serialnumber->octets[i + 1];
		(*parm)->serialnumber->noctets -= 1;
		(*parm)->serialnumber->octets = (char *)realloc((*parm)->serialnumber->octets, (*parm)->serialnumber->noctets);
	}
	;
		}
	    break;

#undef parm

#define parm	((Crl ** )__p)
	case 19: /*  */
		{
# line 429 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((CertificationPath ** )__p)
	case 20: /*  */
		{
# line 448 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((CRLWithCertificates ** )__p)
	case 21: /*  */
		{
# line 461 "af.py"
0;
		}
	    break;

#undef parm

#define parm	((OCList ** )__p)
	case 22: /*  */
		{
# line 537 "af.py"

	if((*parm)->serialnumber->noctets > 1 && !((*parm)->serialnumber->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serialnumber->noctets - 1; i++)
			(*parm)->serialnumber->octets[i] = (*parm)->serialnumber->octets[i + 1];
		(*parm)->serialnumber->noctets -= 1;
		(*parm)->serialnumber->octets = (char *)realloc((*parm)->serialnumber->octets, (*parm)->serialnumber->noctets);
	}
	;
		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, pe, "dnf_SECAF:Bad table entry: %d",
            _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}
static ptpe *etabl[] = {
	et_TCertificatePairSECAF,
	et_ValiditySECAF,
	et_PKRootSECAF,
	et_RootKeySECAF,
	et_TBSCertificateSECAF,
	et_CertificateSECAF,
	et_CertificatesSECAF,
	et_ForwardCertificationPathSECAF,
	et_CertificateSetSECAF,
	et_CertificatePairSECAF,
	et_CrossCertificatePairSECAF,
	et_PKListSECAF,
	et_TBSCRLSECAF,
	et_CRLSECAF,
	et_SETOFCRLSECAF,
	et_CRLEntrySECAF,
	et_CRLEntrySequenceSECAF,
	et_CrlSECAF,
	et_CrlSetSECAF,
	et_CertificationPathSECAF,
	et_CRLWithCertificatesSECAF,
	et_AliasesMemberSECAF,
	et_AliasesSECAF,
	et_AliasListMemberSECAF,
	et_AliasListSECAF,
	et_OldCertificateSECAF,
	et_OldCertificateListSECAF,
	et_CertificatePairsSECAF,
	};

static ptpe *dtabl[] = {
	dt_TCertificatePairSECAF,
	dt_ValiditySECAF,
	dt_PKRootSECAF,
	dt_RootKeySECAF,
	dt_TBSCertificateSECAF,
	dt_CertificateSECAF,
	dt_CertificatesSECAF,
	dt_ForwardCertificationPathSECAF,
	dt_CertificateSetSECAF,
	dt_CertificatePairSECAF,
	dt_CrossCertificatePairSECAF,
	dt_PKListSECAF,
	dt_TBSCRLSECAF,
	dt_CRLSECAF,
	dt_SETOFCRLSECAF,
	dt_CRLEntrySECAF,
	dt_CRLEntrySequenceSECAF,
	dt_CrlSECAF,
	dt_CrlSetSECAF,
	dt_CertificationPathSECAF,
	dt_CRLWithCertificatesSECAF,
	dt_AliasesMemberSECAF,
	dt_AliasesSECAF,
	dt_AliasListMemberSECAF,
	dt_AliasListSECAF,
	dt_OldCertificateSECAF,
	dt_OldCertificateListSECAF,
	dt_CertificatePairsSECAF,
	};

static ptpe *ptabl[] = {
	pt_TCertificatePairSECAF,
	pt_ValiditySECAF,
	pt_PKRootSECAF,
	pt_RootKeySECAF,
	pt_TBSCertificateSECAF,
	pt_CertificateSECAF,
	pt_CertificatesSECAF,
	pt_ForwardCertificationPathSECAF,
	pt_CertificateSetSECAF,
	pt_CertificatePairSECAF,
	pt_CrossCertificatePairSECAF,
	pt_PKListSECAF,
	pt_TBSCRLSECAF,
	pt_CRLSECAF,
	pt_SETOFCRLSECAF,
	pt_CRLEntrySECAF,
	pt_CRLEntrySequenceSECAF,
	pt_CrlSECAF,
	pt_CrlSetSECAF,
	pt_CertificationPathSECAF,
	pt_CRLWithCertificatesSECAF,
	pt_AliasesMemberSECAF,
	pt_AliasesSECAF,
	pt_AliasListMemberSECAF,
	pt_AliasListSECAF,
	pt_OldCertificateSECAF,
	pt_OldCertificateListSECAF,
	pt_CertificatePairsSECAF,
	};


/* Pointer table 59 entries */
caddr_t _ZptrtabSECAF[] = {
    (caddr_t ) "TCertificatePair",
    (caddr_t ) "Validity",
    (caddr_t ) "PKRoot",
    (caddr_t ) "cA",
    (caddr_t ) &_ZSECIF_mod,
    (caddr_t ) "newkey",
    (caddr_t ) "oldkey",
    (caddr_t ) "RootKey",
    (caddr_t ) "version",
    (caddr_t ) "serial",
    (caddr_t ) &_ZSEC_mod,
    (caddr_t ) "key",
    (caddr_t ) "validity",
    (caddr_t ) "signAlgID",
    (caddr_t ) "signBitString",
    (caddr_t ) "TBSCertificate",
    (caddr_t ) "serialNumber",
    (caddr_t ) "signature",
    (caddr_t ) "issuer",
    (caddr_t ) "subject",
    (caddr_t ) "subjectPublickeyInfo",
    (caddr_t ) "Certificate",
    (caddr_t ) "Certificates",
    (caddr_t ) "certificate",
    (caddr_t ) "certificationPath",
    (caddr_t ) "ForwardCertificationPath",
    (caddr_t ) "CertificateSet",
    (caddr_t ) "CertificatePair",
    (caddr_t ) "forward",
    (caddr_t ) "reverse",
    (caddr_t ) "CrossCertificatePair",
    (caddr_t ) "PKList",
    (caddr_t ) "TBSCRL",
    (caddr_t ) "revokedCertificates",
    (caddr_t ) "CRL",
    (caddr_t ) "SETOFCRL",
    (caddr_t ) "CRLEntry",
    (caddr_t ) "CRLEntrySequence",
    (caddr_t ) "Crl",
    (caddr_t ) "revcerts",
    (caddr_t ) "CrlSet",
    (caddr_t ) "CertificationPath",
    (caddr_t ) "userCertificate",
    (caddr_t ) "theCACertificates",
    (caddr_t ) "CRLWithCertificates",
    (caddr_t ) "pemCRL",
    (caddr_t ) "certificates",
    (caddr_t ) "AliasesMember",
    (caddr_t ) "aliasfile",
    (caddr_t ) "Aliases",
    (caddr_t ) "AliasListMember",
    (caddr_t ) "aliases",
    (caddr_t ) "AliasList",
    (caddr_t ) "OldCertificate",
    (caddr_t ) "crossCertificate",
    (caddr_t ) "OldCertificateList",
    (caddr_t ) "CertificatePairs",
    (caddr_t ) "TAlgorithmIdentifier",
    (caddr_t ) "TBitString",
};

modtyp _ZSECAF_mod = {
	"SECAF",
	28,
	etabl,
	dtabl,
	ptabl,
	efn_SECAF,
	dfn_SECAF,
	0,
	_ZptrtabSECAF,
	};


#if	defined(lint) || defined(PEPSY_LINKABLE_FUNCS)

#undef parse_SECAF_TCertificatePair
int	parse_SECAF_TCertificatePair(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SEQUENCE_OF_CertificatePair  **parm;
{
  return (dec_f(_ZTCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_TCertificatePair
void free_SECAF_TCertificatePair(val)
SEQUENCE_OF_CertificatePair  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZTCertificatePairSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_Validity
int	parse_SECAF_Validity(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Validity  **parm;
{
  return (dec_f(_ZValiditySECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_Validity
void free_SECAF_Validity(val)
Validity  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZValiditySECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_PKRoot
int	parse_SECAF_PKRoot(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
PKRoot  **parm;
{
  return (dec_f(_ZPKRootSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_PKRoot
void free_SECAF_PKRoot(val)
PKRoot  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZPKRootSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_RootKey
int	parse_SECAF_RootKey(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct Serial  **parm;
{
  return (dec_f(_ZRootKeySECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_RootKey
void free_SECAF_RootKey(val)
struct Serial  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZRootKeySECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_TBSCertificate
int	parse_SECAF_TBSCertificate(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
ToBeSigned  **parm;
{
  return (dec_f(_ZTBSCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_TBSCertificate
void free_SECAF_TBSCertificate(val)
ToBeSigned  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZTBSCertificateSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_Certificate
int	parse_SECAF_Certificate(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Certificate  **parm;
{
  return (dec_f(_ZCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_Certificate
void free_SECAF_Certificate(val)
Certificate  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCertificateSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_Certificates
int	parse_SECAF_Certificates(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Certificates  **parm;
{
  return (dec_f(_ZCertificatesSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_Certificates
void free_SECAF_Certificates(val)
Certificates  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCertificatesSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_ForwardCertificationPath
int	parse_SECAF_ForwardCertificationPath(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
FCPath  **parm;
{
  return (dec_f(_ZForwardCertificationPathSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_ForwardCertificationPath
void free_SECAF_ForwardCertificationPath(val)
FCPath  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZForwardCertificationPathSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CertificateSet
int	parse_SECAF_CertificateSet(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_Certificate  **parm;
{
  return (dec_f(_ZCertificateSetSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CertificateSet
void free_SECAF_CertificateSet(val)
SET_OF_Certificate  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCertificateSetSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CertificatePair
int	parse_SECAF_CertificatePair(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CertificatePair  **parm;
{
  return (dec_f(_ZCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CertificatePair
void free_SECAF_CertificatePair(val)
CertificatePair  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCertificatePairSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CrossCertificatePair
int	parse_SECAF_CrossCertificatePair(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_CertificatePair  **parm;
{
  return (dec_f(_ZCrossCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CrossCertificatePair
void free_SECAF_CrossCertificatePair(val)
SET_OF_CertificatePair  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCrossCertificatePairSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_PKList
int	parse_SECAF_PKList(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
PKList  **parm;
{
  return (dec_f(_ZPKListSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_PKList
void free_SECAF_PKList(val)
PKList  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZPKListSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_TBSCRL
int	parse_SECAF_TBSCRL(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CRLTBS  **parm;
{
  return (dec_f(_ZTBSCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_TBSCRL
void free_SECAF_TBSCRL(val)
CRLTBS  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZTBSCRLSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CRL
int	parse_SECAF_CRL(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CRL  **parm;
{
  return (dec_f(_ZCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CRL
void free_SECAF_CRL(val)
CRL  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCRLSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_SETOFCRL
int	parse_SECAF_SETOFCRL(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_CRL  **parm;
{
  return (dec_f(_ZSETOFCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_SETOFCRL
void free_SECAF_SETOFCRL(val)
SET_OF_CRL  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZSETOFCRLSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CRLEntry
int	parse_SECAF_CRLEntry(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CRLEntry  **parm;
{
  return (dec_f(_ZCRLEntrySECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CRLEntry
void free_SECAF_CRLEntry(val)
CRLEntry  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCRLEntrySECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CRLEntrySequence
int	parse_SECAF_CRLEntrySequence(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SEQUENCE_OF_CRLEntry  **parm;
{
  return (dec_f(_ZCRLEntrySequenceSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CRLEntrySequence
void free_SECAF_CRLEntrySequence(val)
SEQUENCE_OF_CRLEntry  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCRLEntrySequenceSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_Crl
int	parse_SECAF_Crl(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Crl  **parm;
{
  return (dec_f(_ZCrlSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_Crl
void free_SECAF_Crl(val)
Crl  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCrlSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CrlSet
int	parse_SECAF_CrlSet(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CrlSet  **parm;
{
  return (dec_f(_ZCrlSetSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CrlSet
void free_SECAF_CrlSet(val)
CrlSet  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCrlSetSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CertificationPath
int	parse_SECAF_CertificationPath(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CertificationPath  **parm;
{
  return (dec_f(_ZCertificationPathSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CertificationPath
void free_SECAF_CertificationPath(val)
CertificationPath  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCertificationPathSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CRLWithCertificates
int	parse_SECAF_CRLWithCertificates(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CRLWithCertificates  **parm;
{
  return (dec_f(_ZCRLWithCertificatesSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CRLWithCertificates
void free_SECAF_CRLWithCertificates(val)
CRLWithCertificates  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCRLWithCertificatesSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_AliasesMember
int	parse_SECAF_AliasesMember(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Aliases  **parm;
{
  return (dec_f(_ZAliasesMemberSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_AliasesMember
void free_SECAF_AliasesMember(val)
Aliases  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZAliasesMemberSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_Aliases
int	parse_SECAF_Aliases(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Aliases  **parm;
{
  return (dec_f(_ZAliasesSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_Aliases
void free_SECAF_Aliases(val)
Aliases  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZAliasesSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_AliasListMember
int	parse_SECAF_AliasListMember(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AliasList  **parm;
{
  return (dec_f(_ZAliasListMemberSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_AliasListMember
void free_SECAF_AliasListMember(val)
AliasList  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZAliasListMemberSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_AliasList
int	parse_SECAF_AliasList(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AliasList  **parm;
{
  return (dec_f(_ZAliasListSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_AliasList
void free_SECAF_AliasList(val)
AliasList  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZAliasListSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_OldCertificate
int	parse_SECAF_OldCertificate(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
OCList  **parm;
{
  return (dec_f(_ZOldCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_OldCertificate
void free_SECAF_OldCertificate(val)
OCList  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZOldCertificateSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_OldCertificateList
int	parse_SECAF_OldCertificateList(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
OCList  **parm;
{
  return (dec_f(_ZOldCertificateListSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_OldCertificateList
void free_SECAF_OldCertificateList(val)
OCList  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZOldCertificateListSECAF], &_ZSECAF_mod, 1);
}

#undef parse_SECAF_CertificatePairs
int	parse_SECAF_CertificatePairs(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
CertificatePairs  **parm;
{
  return (dec_f(_ZCertificatePairsSECAF, &_ZSECAF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECAF_CertificatePairs
void free_SECAF_CertificatePairs(val)
CertificatePairs  *val;
{
(void) fre_obj((char *) val, _ZSECAF_mod.md_dtab[_ZCertificatePairsSECAF], &_ZSECAF_mod, 1);
}

#endif	/* lint||PEPSY_LINKABLE_FUNCS */

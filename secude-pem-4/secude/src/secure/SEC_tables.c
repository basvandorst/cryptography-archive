#include <stdio.h>
#include "SEC-types.h"


# line 6 "sec.py"
      /* surrounding global definitions */
#include "secure.h"

	static OctetString   * tmp_ostr1, * tmp_ostr2, * tmp_ostr3, * tmp_ostr4, * ostr;
	static OctetString     save_part1, save_part2, save_part3, save_part4;
	static rsa_parm_type * rsa_parm;
	static int             i, j, rc;
	static KeyBits       * tmp_keybits;
	static OctetString   * nullostr = NULLOCTETSTRING;

extern caddr_t _ZptrtabSEC[];	/* forward decl */

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static ptpe et_TOURINTEGERSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[0] },
	{ OCTET_PTR, OFFSET(OctetString , octets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[0] },
	{ OCTET_LEN, OFFSET(OctetString , noctets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[0] }
	};

static ptpe et_TBitStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[1] },
	{ BITSTR_PTR, OFFSET(Signature , signature.bits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[1] },
	{ BITSTR_LEN, OFFSET(Signature , signature.nbits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[1] }
	};

static ptpe et_TAlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[2] },
	{ OBJECT, OFFSET(Signature , signAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[2] }
	};

static ptpe et_OctetStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[3] },
	{ OCTET_PTR, OFFSET(OctetString , octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[3] },
	{ OCTET_LEN, OFFSET(OctetString , noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[3] }
	};

static ptpe et_SETOFOctetStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[4] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSEC[4] },
	{ OBJECT, OFFSET(SET_OF_OctetString , element), _ZOctetStringSEC, 0, (char **)&_ZptrtabSEC[4] },
	{ PE_END, OFFSET(SET_OF_OctetString , next), 0, 0, (char **)&_ZptrtabSEC[4] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[4] }
	};

static ptpe et_SETOFObjIdSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[5] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSEC[5] },
	{ OBJID, OFFSET(SET_OF_ObjId , element), 6, FL_UNIVERSAL, (char **)&_ZptrtabSEC[5] },
	{ PE_END, OFFSET(SET_OF_ObjId , next), 0, 0, (char **)&_ZptrtabSEC[5] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[5] }
	};

static ptpe et_BitStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[6] },
	{ BITSTR_PTR, OFFSET(BitString , bits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[6] },
	{ BITSTR_LEN, OFFSET(BitString , nbits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[6] }
	};

static ptpe et_OURINTEGERSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[7] },
	{ OCTET_PTR, OFFSET(OctetString , octets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[7] },
	{ OCTET_LEN, OFFSET(OctetString , noctets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[7] }
	};

static ptpe et_KeyBitsSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ UCODE, 0, 0, 0, (char **)&_ZptrtabSEC[8] }, /* line 89 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[8] },
	{ SOBJECT, OFFSET(KeyBits , part1), _ZTOURINTEGERSEC, 0, (char **)&_ZptrtabSEC[9] },
	{ BOPTIONAL, 2, 0, FL_USELECT, (char **)&_ZptrtabSEC[8]},
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ SCTRL, 1, 0, FL_USELECT, (char **)&_ZptrtabSEC[8] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[10] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSEC[11] },
	{ SOBJECT, OFFSET(KeyBits , part2), _ZTOURINTEGERSEC, FL_UNIVERSAL, (char **)&_ZptrtabSEC[11] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSEC[12] },
	{ SOBJECT, OFFSET(KeyBits , part3), _ZTOURINTEGERSEC, FL_UNIVERSAL, (char **)&_ZptrtabSEC[12] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSEC[13] },
	{ SOBJECT, OFFSET(KeyBits , part4), _ZTOURINTEGERSEC, FL_UNIVERSAL, (char **)&_ZptrtabSEC[13] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[10] },
	{ SOBJECT, OFFSET(KeyBits , part2), _ZTOURINTEGERSEC, 0, (char **)&_ZptrtabSEC[14] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ BOPTIONAL, 3, 0, FL_USELECT, (char **)&_ZptrtabSEC[15]},
	{ SOBJECT, OFFSET(KeyBits , part3), _ZTOURINTEGERSEC, 0, (char **)&_ZptrtabSEC[15] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ UCODE, 4, 0, 0, (char **)&_ZptrtabSEC[8] }, /* line 208 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[8] }
	};

static ptpe et_KeyInfoSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[16] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[16] },
	{ OBJECT, OFFSET(KeyInfo , subjectAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[17] },
	{ BITSTR_PTR, OFFSET(KeyInfo , subjectkey.bits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ BITSTR_LEN, OFFSET(KeyInfo , subjectkey.nbits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[16] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[16] }
	};

static ptpe et_DigestInfoSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[19] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[19] },
	{ OBJECT, OFFSET(DigestInfo , digestAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[20] },
	{ OCTET_PTR, OFFSET(DigestInfo , digest.octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[21] },
	{ OCTET_LEN, OFFSET(DigestInfo , digest.noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[21] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[19] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[19] }
	};

static ptpe et_EncryptedKeySEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[22] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[22] },
	{ OBJECT, OFFSET(EncryptedKey , encryptionAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[23] },
	{ OBJECT, OFFSET(EncryptedKey , subjectAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[24] },
	{ BITSTR_PTR, OFFSET(EncryptedKey , subjectkey.bits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ BITSTR_LEN, OFFSET(EncryptedKey , subjectkey.nbits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[22] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[22] }
	};

static ptpe et_AlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[25] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[25] },
	{ OBJID, OFFSET(AlgId , objid), 6, FL_UNIVERSAL, (char **)&_ZptrtabSEC[26] },
	{ UCODE, 5, 0, 0, (char **)&_ZptrtabSEC[26] }, /* line 339 */
	{ BOPTIONAL, 7, 0, FL_USELECT, (char **)&_ZptrtabSEC[27]},
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSEC[27] },
	{ SCTRL, 6, 0, FL_USELECT, (char **)&_ZptrtabSEC[27] },
	{ INTEGER, OFFSET(AlgId , un.keyorblocksize), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[28] },
	{ OBJECT, OFFSET(AlgId , un.des_parm), _ZOctetStringSEC, 0, (char **)&_ZptrtabSEC[29] },
	{ OBJECT, OFFSET(AlgId , un.diffie_hellmann_parm), _ZKeyBitsSEC, 0, (char **)&_ZptrtabSEC[30] },
	{ T_NULL, 0, 5, FL_UNIVERSAL, (char **)&_ZptrtabSEC[31] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[25] }
	};

static ptpe et_SETOFAlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[32] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSEC[32] },
	{ OBJECT, OFFSET(SET_OF_AlgId , element), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[32] },
	{ PE_END, OFFSET(SET_OF_AlgId , next), 0, 0, (char **)&_ZptrtabSEC[32] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[32] }
	};

static ptpe et_PSEObjectsSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[33] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSEC[33] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[33] },
	{ T_STRING, OFFSET(struct PSE_Objects , name), 19, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(struct PSE_Objects , create), 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(struct PSE_Objects , update), 23, FL_UNIVERSAL, NULLVP },
	{ BOPTIONAL, 8, 0, FL_USELECT, (char **)&_ZptrtabSEC[34]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSEC[34] },
	{ INTEGER, OFFSET(struct PSE_Objects , noOctets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[34] },
	{ BOPTIONAL, 9, 0, FL_USELECT, (char **)&_ZptrtabSEC[35]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSEC[35] },
	{ INTEGER, OFFSET(struct PSE_Objects , status), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[35] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[33] },
	{ PE_END, OFFSET(struct PSE_Objects , next), 0, 0, (char **)&_ZptrtabSEC[33] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[33] }
	};

static ptpe et_PSETocSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[36] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[36] },
	{ T_STRING, OFFSET(PSEToc , owner), 19, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(PSEToc , create), 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(PSEToc , update), 23, FL_UNIVERSAL, NULLVP },
	{ BOPTIONAL, 10, 0, FL_USELECT, (char **)&_ZptrtabSEC[35]},
	{ INTEGER, OFFSET(PSEToc , status), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[35] },
	{ BOPTIONAL, 11, 0, FL_USELECT, (char **)&_ZptrtabSEC[37]},
	{ OBJECT, OFFSET(PSEToc , obj), _ZPSEObjectsSEC, 0, (char **)&_ZptrtabSEC[37] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[36] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[36] }
	};

static ptpe et_PSEObjectSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[38] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[38] },
	{ OBJID, OFFSET(PSEObject , objectType), 6, FL_UNIVERSAL, (char **)&_ZptrtabSEC[39] },
	{ UCODE, 12, 0, 0, (char **)&_ZptrtabSEC[39] }, /* line 458 */
	{ ANY, OFFSET(PSEObject , asn), -1, FL_UNIVERSAL, (char **)&_ZptrtabSEC[40] },
	{ UCODE, 13, 0, 0, (char **)&_ZptrtabSEC[40] }, /* line 464 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[38] }
	};

static ptpe dt_TOURINTEGERSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[0] },
	{ OCTET_PTR, OFFSET(OctetString , octets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[0] },
	{ OCTET_LEN, OFFSET(OctetString , noctets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[0] }
	};

static ptpe dt_TBitStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[1] },
	{ BITSTR_PTR, OFFSET(Signature , signature.bits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[1] },
	{ BITSTR_LEN, OFFSET(Signature , signature.nbits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[1] }
	};

static ptpe dt_TAlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[2] },
	{ OBJECT, OFFSET(Signature , signAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[2] }
	};

static ptpe dt_OctetStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[3] },
	{ UCODE, 0, 0, 0, (char **)&_ZptrtabSEC[3] }, /* line 44 */
	{ OCTET_PTR, OFFSET(OctetString , octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[3] },
	{ OCTET_LEN, OFFSET(OctetString , noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[3] }
	};

static ptpe dt_SETOFOctetStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[4] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSEC[4] },
	{ MEMALLOC, 0, sizeof (SET_OF_OctetString ), 0, (char **)&_ZptrtabSEC[4] },
	{ OBJECT, OFFSET(SET_OF_OctetString , element), _ZOctetStringSEC, 0, (char **)&_ZptrtabSEC[4] },
	{ PE_END, OFFSET(SET_OF_OctetString , next), 0, 0, (char **)&_ZptrtabSEC[4] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[4] }
	};

static ptpe dt_SETOFObjIdSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[5] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSEC[5] },
	{ MEMALLOC, 0, sizeof (SET_OF_ObjId ), 0, (char **)&_ZptrtabSEC[5] },
	{ OBJID, OFFSET(SET_OF_ObjId , element), 6, FL_UNIVERSAL, (char **)&_ZptrtabSEC[5] },
	{ PE_END, OFFSET(SET_OF_ObjId , next), 0, 0, (char **)&_ZptrtabSEC[5] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[5] }
	};

static ptpe dt_BitStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[6] },
	{ UCODE, 1, 0, 0, (char **)&_ZptrtabSEC[6] }, /* line 67 */
	{ BITSTR_PTR, OFFSET(BitString , bits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[6] },
	{ BITSTR_LEN, OFFSET(BitString , nbits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[6] }
	};

static ptpe dt_OURINTEGERSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[7] },
	{ UCODE, 2, 0, 0, (char **)&_ZptrtabSEC[7] }, /* line 78 */
	{ OCTET_PTR, OFFSET(OctetString , octets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[7] },
	{ OCTET_LEN, OFFSET(OctetString , noctets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[7] }
	};

static ptpe dt_KeyBitsSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ UCODE, 3, 0, 0, (char **)&_ZptrtabSEC[8] }, /* line 169 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[8] },
	{ MEMALLOC, 0, sizeof (KeyBits ), 0, (char **)&_ZptrtabSEC[8] },
	{ SOBJECT, OFFSET(KeyBits , part1), _ZTOURINTEGERSEC, 0, (char **)&_ZptrtabSEC[9] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSEC[8]},
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ SCTRL, -1, 0, FL_USELECT, (char **)&_ZptrtabSEC[8] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[10] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSEC[11] },
	{ SOBJECT, OFFSET(KeyBits , part2), _ZTOURINTEGERSEC, FL_UNIVERSAL, (char **)&_ZptrtabSEC[11] },
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSEC[12] },
	{ SOBJECT, OFFSET(KeyBits , part3), _ZTOURINTEGERSEC, FL_UNIVERSAL, (char **)&_ZptrtabSEC[12] },
	{ ETAG, 0, 2, FL_CONTEXT, (char **)&_ZptrtabSEC[13] },
	{ SOBJECT, OFFSET(KeyBits , part4), _ZTOURINTEGERSEC, FL_UNIVERSAL, (char **)&_ZptrtabSEC[13] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[10] },
	{ UCODE, 5, 0, 0, (char **)&_ZptrtabSEC[10] }, /* line 194 */
	{ SOBJECT, OFFSET(KeyBits , part2), _ZTOURINTEGERSEC, 0, (char **)&_ZptrtabSEC[14] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ BOPTIONAL, 7, 0, FL_USELECT, (char **)&_ZptrtabSEC[15]},
	{ SOBJECT, OFFSET(KeyBits , part3), _ZTOURINTEGERSEC, 0, (char **)&_ZptrtabSEC[15] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ UCODE, 8, 0, 0, (char **)&_ZptrtabSEC[8] }, /* line 230 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[8] }
	};

static ptpe dt_KeyInfoSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[16] },
	{ UCODE, 9, 0, 0, (char **)&_ZptrtabSEC[16] }, /* line 271 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[16] },
	{ MEMALLOC, 0, sizeof (KeyInfo ), 0, (char **)&_ZptrtabSEC[16] },
	{ OBJECT, OFFSET(KeyInfo , subjectAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[17] },
	{ BITSTR_PTR, OFFSET(KeyInfo , subjectkey.bits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ BITSTR_LEN, OFFSET(KeyInfo , subjectkey.nbits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[16] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[16] }
	};

static ptpe dt_DigestInfoSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[19] },
	{ UCODE, 10, 0, 0, (char **)&_ZptrtabSEC[19] }, /* line 289 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[19] },
	{ MEMALLOC, 0, sizeof (DigestInfo ), 0, (char **)&_ZptrtabSEC[19] },
	{ OBJECT, OFFSET(DigestInfo , digestAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[20] },
	{ OCTET_PTR, OFFSET(DigestInfo , digest.octets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[21] },
	{ OCTET_LEN, OFFSET(DigestInfo , digest.noctets), 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[21] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[19] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[19] }
	};

static ptpe dt_EncryptedKeySEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[22] },
	{ UCODE, 11, 0, 0, (char **)&_ZptrtabSEC[22] }, /* line 307 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[22] },
	{ MEMALLOC, 0, sizeof (EncryptedKey ), 0, (char **)&_ZptrtabSEC[22] },
	{ OBJECT, OFFSET(EncryptedKey , encryptionAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[23] },
	{ OBJECT, OFFSET(EncryptedKey , subjectAI), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[24] },
	{ BITSTR_PTR, OFFSET(EncryptedKey , subjectkey.bits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ BITSTR_LEN, OFFSET(EncryptedKey , subjectkey.nbits), 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[22] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[22] }
	};

static ptpe dt_AlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[25] },
	{ UCODE, 12, 0, 0, (char **)&_ZptrtabSEC[25] }, /* line 328 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[25] },
	{ MEMALLOC, 0, sizeof (AlgId ), 0, (char **)&_ZptrtabSEC[25] },
	{ OBJID, OFFSET(AlgId , objid), 6, FL_UNIVERSAL, (char **)&_ZptrtabSEC[26] },
	{ UCODE, 13, 0, 0, (char **)&_ZptrtabSEC[26] }, /* line 352 */
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSEC[27]},
	{ SCHOICE_START, 0, 0, 0, (char **)&_ZptrtabSEC[27] },
	{ SCTRL, 14, 0, FL_USELECT, (char **)&_ZptrtabSEC[27] },
	{ INTEGER, OFFSET(AlgId , un.keyorblocksize), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[28] },
	{ UCODE, 15, 0, 0, (char **)&_ZptrtabSEC[28] }, /* line 369 */
	{ OBJECT, OFFSET(AlgId , un.des_parm), _ZOctetStringSEC, 0, (char **)&_ZptrtabSEC[29] },
	{ UCODE, 16, 0, 0, (char **)&_ZptrtabSEC[29] }, /* line 377 */
	{ OBJECT, OFFSET(AlgId , un.diffie_hellmann_parm), _ZKeyBitsSEC, 0, (char **)&_ZptrtabSEC[30] },
	{ UCODE, 17, 0, 0, (char **)&_ZptrtabSEC[30] }, /* line 386 */
	{ T_NULL, 0, 5, FL_UNIVERSAL, (char **)&_ZptrtabSEC[31] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[27] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[25] }
	};

static ptpe dt_SETOFAlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[32] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSEC[32] },
	{ MEMALLOC, 0, sizeof (SET_OF_AlgId ), 0, (char **)&_ZptrtabSEC[32] },
	{ OBJECT, OFFSET(SET_OF_AlgId , element), _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[32] },
	{ PE_END, OFFSET(SET_OF_AlgId , next), 0, 0, (char **)&_ZptrtabSEC[32] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[32] }
	};

static ptpe dt_PSEObjectsSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[33] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSEC[33] },
	{ MEMALLOC, 0, sizeof (struct PSE_Objects ), 0, (char **)&_ZptrtabSEC[33] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[33] },
	{ T_STRING, OFFSET(struct PSE_Objects , name), 19, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(struct PSE_Objects , create), 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(struct PSE_Objects , update), 23, FL_UNIVERSAL, NULLVP },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSEC[34]},
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabSEC[34] },
	{ INTEGER, OFFSET(struct PSE_Objects , noOctets), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[34] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSEC[35]},
	{ ETAG, 0, 1, FL_CONTEXT, (char **)&_ZptrtabSEC[35] },
	{ INTEGER, OFFSET(struct PSE_Objects , status), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[35] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[33] },
	{ PE_END, OFFSET(struct PSE_Objects , next), 0, 0, (char **)&_ZptrtabSEC[33] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[33] }
	};

static ptpe dt_PSETocSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[36] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[36] },
	{ MEMALLOC, 0, sizeof (PSEToc ), 0, (char **)&_ZptrtabSEC[36] },
	{ T_STRING, OFFSET(PSEToc , owner), 19, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(PSEToc , create), 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, OFFSET(PSEToc , update), 23, FL_UNIVERSAL, NULLVP },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSEC[35]},
	{ INTEGER, OFFSET(PSEToc , status), 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[35] },
	{ BOPTIONAL, -1, 0, FL_USELECT, (char **)&_ZptrtabSEC[37]},
	{ OBJECT, OFFSET(PSEToc , obj), _ZPSEObjectsSEC, 0, (char **)&_ZptrtabSEC[37] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[36] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[36] }
	};

static ptpe dt_PSEObjectSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[38] },
	{ UCODE, 23, 0, 0, (char **)&_ZptrtabSEC[38] }, /* line 447 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[38] },
	{ MEMALLOC, 0, sizeof (PSEObject ), 0, (char **)&_ZptrtabSEC[38] },
	{ OBJID, OFFSET(PSEObject , objectType), 6, FL_UNIVERSAL, (char **)&_ZptrtabSEC[39] },
	{ ANY, OFFSET(PSEObject , asn), -1, FL_UNIVERSAL, (char **)&_ZptrtabSEC[40] },
	{ UCODE, 24, 0, 0, (char **)&_ZptrtabSEC[40] }, /* line 467 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[38] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[38] }
	};

static ptpe pt_TOURINTEGERSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[0] },
	{ OCTET_PTR, -1, 2, FL_UNIVERSAL, NULLVP },
	{ OCTET_LEN, -1, 2, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[0] }
	};

static ptpe pt_TBitStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[1] },
	{ BITSTR_PTR, -1, 3, FL_UNIVERSAL, NULLVP },
	{ BITSTR_LEN, -1, 3, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[1] }
	};

static ptpe pt_TAlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[2] },
	{ OBJECT, 0, _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[25] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[2] }
	};

static ptpe pt_OctetStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[3] },
	{ OCTET_PTR, -1, 4, FL_UNIVERSAL, NULLVP },
	{ OCTET_LEN, -1, 4, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[3] }
	};

static ptpe pt_SETOFOctetStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[4] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZOctetStringSEC, 0, (char **)&_ZptrtabSEC[3] },
	{ PE_END, OFFSET(SET_OF_OctetString , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[4] }
	};

static ptpe pt_SETOFObjIdSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[5] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJID, -1, 6, FL_UNIVERSAL, NULLVP },
	{ PE_END, OFFSET(SET_OF_ObjId , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[5] }
	};

static ptpe pt_BitStringSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[6] },
	{ BITSTR_PTR, -1, 3, FL_UNIVERSAL, NULLVP },
	{ BITSTR_LEN, -1, 3, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[6] }
	};

static ptpe pt_OURINTEGERSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[7] },
	{ OCTET_PTR, -1, 2, FL_UNIVERSAL, NULLVP },
	{ OCTET_LEN, -1, 2, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[7] }
	};

static ptpe pt_KeyBitsSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[8] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ SOBJECT, 0, _ZTOURINTEGERSEC, 0, (char **)&_ZptrtabSEC[9] },
	{ SCHOICE_START, -1, 0, FL_OPTIONAL, NULLVP },
	{ SCTRL, 0, 0, 0, NULLVP },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, (char **)&_ZptrtabSEC[10] },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabSEC[11] },
	{ SOBJECT, 0, _ZTOURINTEGERSEC, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT, (char **)&_ZptrtabSEC[12] },
	{ SOBJECT, 0, _ZTOURINTEGERSEC, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 2, FL_CONTEXT, (char **)&_ZptrtabSEC[13] },
	{ SOBJECT, 0, _ZTOURINTEGERSEC, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ SOBJECT, 0, _ZTOURINTEGERSEC, 0, (char **)&_ZptrtabSEC[14] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ SOBJECT, 0, _ZTOURINTEGERSEC, FL_OPTIONAL, (char **)&_ZptrtabSEC[15] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[8] }
	};

static ptpe pt_KeyInfoSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[16] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[17] },
	{ BITSTR_PTR, -1, 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ BITSTR_LEN, -1, 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[16] }
	};

static ptpe pt_DigestInfoSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[19] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[20] },
	{ OCTET_PTR, -1, 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[21] },
	{ OCTET_LEN, -1, 4, FL_UNIVERSAL, (char **)&_ZptrtabSEC[21] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[19] }
	};

static ptpe pt_EncryptedKeySEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[22] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[23] },
	{ OBJECT, 0, _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[24] },
	{ BITSTR_PTR, -1, 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ BITSTR_LEN, -1, 3, FL_UNIVERSAL, (char **)&_ZptrtabSEC[18] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[22] }
	};

static ptpe pt_AlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[25] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJID, -1, 6, FL_UNIVERSAL, (char **)&_ZptrtabSEC[26] },
	{ SCHOICE_START, -1, 0, FL_OPTIONAL, (char **)&_ZptrtabSEC[27] },
	{ SCTRL, 0, 0, 0, (char **)&_ZptrtabSEC[27] },
	{ INTEGER, -1, 2, FL_UNIVERSAL, (char **)&_ZptrtabSEC[28] },
	{ OBJECT, 0, _ZOctetStringSEC, 0, (char **)&_ZptrtabSEC[29] },
	{ OBJECT, 0, _ZKeyBitsSEC, 0, (char **)&_ZptrtabSEC[30] },
	{ T_NULL, -1, 5, FL_UNIVERSAL, (char **)&_ZptrtabSEC[31] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[25] }
	};

static ptpe pt_SETOFAlgorithmIdentifierSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[32] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAlgorithmIdentifierSEC, 0, (char **)&_ZptrtabSEC[25] },
	{ PE_END, OFFSET(SET_OF_AlgId , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[32] }
	};

static ptpe pt_PSEObjectsSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[33] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 19, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSEC[34] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, (char **)&_ZptrtabSEC[35] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, OFFSET(struct PSE_Objects , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[33] }
	};

static ptpe pt_PSETocSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[36] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 19, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabSEC[35] },
	{ OBJECT, 0, _ZPSEObjectsSEC, FL_OPTIONAL, (char **)&_ZptrtabSEC[37] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[36] }
	};

static ptpe pt_PSEObjectSEC[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSEC[38] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJID, -1, 6, FL_UNIVERSAL, (char **)&_ZptrtabSEC[39] },
	{ ANY, -1, -1, FL_UNIVERSAL, (char **)&_ZptrtabSEC[40] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSEC[38] }
	};


/*VARARGS*/
static	int
efn_SEC(__p, ppe, _Zp)
caddr_t	__p;
PE	*ppe;
ptpe	*_Zp;
{
		
	/* 14 cases */
    switch(_Zp->pe_ucode) {

#define parm	((KeyBits * )__p)
	case 0: /*  */
		{
# line 89 "sec.py"


	/* valid constellations:

	   (parm->part2.noctets = 0 && parm->part3.noctets = 0 && parm->part4.noctets = 0) ||
	   (parm->part2.noctets > 0 && parm->part3.noctets = 0 && parm->part4.noctets = 0) ||
	   (parm->part2.noctets > 0 && parm->part3.noctets > 0 && parm->part4.noctets = 0) ||
	   (parm->part2.noctets > 0 && parm->part3.noctets > 0 && parm->part4.noctets > 0)

	*/
	
	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	tmp_ostr1 = tmp_ostr2 = tmp_ostr3 = tmp_ostr4 = NULLOCTETSTRING;
	parm->choice = 1;

	if ((parm->part1.noctets == 0) ||
	    ((parm->part2.noctets > 0 || parm->part3.noctets > 0 || parm->part4.noctets > 0) &&
	     (parm->part2.noctets == 0 || parm->part3.noctets > 0 || parm->part4.noctets > 0) &&
	     (parm->part2.noctets == 0 || parm->part3.noctets == 0 || parm->part4.noctets > 0) &&
	     (parm->part2.noctets == 0 || parm->part3.noctets == 0 || parm->part4.noctets == 0)))
		return(NOTOK);

	if(parm->part1.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr1 = aux_concatenate_OctetString(nullostr, &parm->part1);

		/* Save values of parm->part1 */
		save_part1.noctets = parm->part1.noctets;
		save_part1.octets = parm->part1.octets;
		parm->part1.noctets = tmp_ostr1->noctets;
		parm->part1.octets = tmp_ostr1->octets;
	}

	if (parm->part2.noctets > 0 && (parm->part2.octets[0] & MSBITINBYTE)){
		/* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr2 = aux_concatenate_OctetString(nullostr, &parm->part2);

		/* Save values of parm->part2 */
		save_part2.noctets = parm->part2.noctets;
		save_part2.octets = parm->part2.octets;
		parm->part2.noctets = tmp_ostr2->noctets;
		parm->part2.octets = tmp_ostr2->octets;
	}

	if (parm->part3.noctets > 0 && (parm->part3.octets[0] & MSBITINBYTE)){
		/* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr3 = aux_concatenate_OctetString(nullostr, &parm->part3);

		/* Save values of parm->part3 */
		save_part3.noctets = parm->part3.noctets;
		save_part3.octets = parm->part3.octets;
		parm->part3.noctets = tmp_ostr3->noctets;
		parm->part3.octets = tmp_ostr3->octets;
	}

	if (parm->part4.noctets > 0 && (parm->part4.octets[0] & MSBITINBYTE)){
		/* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr4 = aux_concatenate_OctetString(nullostr, &parm->part4);

		/* Save values of parm->part4 */
		save_part4.noctets = parm->part4.noctets;
		save_part4.octets = parm->part4.octets;
		parm->part4.noctets = tmp_ostr4->noctets;
		parm->part4.octets = tmp_ostr4->octets;
	}

	if (parm->part4.noctets > 0) 
		parm->choice = 1; /* DSA */
	else {
		parm->choice = 2; /* RSA or DH or DSA (without SEQUENCE) */
	}
	;
		}
	    break;

#undef parm

#define parm	((KeyBits * )__p)
	case 4: /*  */
		{
# line 208 "sec.py"

	if(tmp_ostr1) {
		aux_free_OctetString(&tmp_ostr1);
		parm->part1.noctets = save_part1.noctets;
		parm->part1.octets = save_part1.octets;
	}
	if(tmp_ostr2) {
		aux_free_OctetString(&tmp_ostr2);
		parm->part2.noctets = save_part2.noctets;
		parm->part2.octets = save_part2.octets;
	}
	if(tmp_ostr3) {
		aux_free_OctetString(&tmp_ostr3);
		parm->part3.noctets = save_part3.noctets;
		parm->part3.octets = save_part3.octets;
	}
	if(tmp_ostr4) {
		aux_free_OctetString(&tmp_ostr4);
		parm->part4.noctets = save_part4.noctets;
		parm->part4.octets = save_part4.octets;
	}
	;
		}
	    break;

#undef parm

#define parm	((KeyBits * )__p)
	case 1: /*  */
		{
# line 182 "sec.py"
		return (parm->choice);

		}
	    break;

#undef parm

#define parm	((KeyBits * )__p)
	case 2: /*  */
		{
# line 201 "sec.py"
		return (parm->part2.noctets > 0);

		}
	    break;

#undef parm

#define parm	((KeyBits * )__p)
	case 3: /*  */
		{
# line 205 "sec.py"
		return (parm->choice == 2 && parm->part3.noctets > 0);

		}
	    break;

#undef parm

#define parm	((AlgId * )__p)
	case 5: /*  */
		{
# line 339 "sec.py"

	    	parm->paramchoice = aux_ObjId2ParmType(parm->objid);
	    	if (parm->paramchoice < 0){
            		advise (NULLCP, "parm->paramchoice has undefined value");
            		return NOTOK;
       		} 
		if (parm->paramchoice == PARM_INTEGER)
			parm->un.keyorblocksize = *((rsa_parm_type *)(parm->param));
		else if (parm->paramchoice == PARM_OctetString)
			parm->un.des_parm = (desCBC_parm_type *)(parm->param);
		else if (parm->paramchoice == PARM_KeyBits)
			parm->un.diffie_hellmann_parm = (KeyBits *)(parm->param);
		;
		}
	    break;

#undef parm

#define parm	((AlgId * )__p)
	case 6: /*  */
		{
# line 364 "sec.py"
		return (parm->paramchoice);

		}
	    break;

#undef parm

#define parm	((AlgId * )__p)
	case 7: /*  */
		{
# line 395 "sec.py"
		return (parm->paramchoice > 0);

		}
	    break;

#undef parm

#define parm	((struct PSE_Objects * )__p)
	case 8: /*  */
		{
# line 419 "sec.py"
		return (parm->noOctets != 0);

		}
	    break;

#undef parm

#define parm	((struct PSE_Objects * )__p)
	case 9: /*  */
		{
# line 422 "sec.py"
		return (parm->status != 0);

		}
	    break;

#undef parm

#define parm	((PSEToc * )__p)
	case 10: /*  */
		{
# line 438 "sec.py"
		return (parm->status != 0);

		}
	    break;

#undef parm

#define parm	((PSEToc * )__p)
	case 11: /*  */
		{
# line 441 "sec.py"
		return (parm->obj != (struct PSE_Objects *)0);

		}
	    break;

#undef parm

#define parm	((PSEObject * )__p)
	case 12: /*  */
		{
# line 458 "sec.py"

		parm->asn = aux_OctetString2PE(parm->objectValue);
     		;
		}
	    break;

#undef parm

#define parm	((PSEObject * )__p)
	case 13: /*  */
		{
# line 464 "sec.py"

		pe_free(parm->asn);
		;
		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, *ppe, "enf_SEC:Bad table entry: %d",
             _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}

/*VARARGS*/
static	int
dfn_SEC(__p, pe, _Zp, _val)
caddr_t	__p;
PE	pe;
ptpe	*_Zp;
int _val;
{
		
	/* 25 cases */
    switch(_Zp->pe_ucode) {

#define parm	((OctetString ** )__p)
	case 0: /*  */
		{
# line 44 "sec.py"

        if (((*parm) = (OctetString *) calloc (1, sizeof (OctetString))) == ((OctetString *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((BitString ** )__p)
	case 1: /*  */
		{
# line 67 "sec.py"

        if (((*parm) = (BitString *) calloc (1, sizeof (BitString))) == ((BitString *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((OctetString ** )__p)
	case 2: /*  */
		{
# line 78 "sec.py"

        if (((*parm) = (OctetString *) calloc (1, sizeof (OctetString))) == ((OctetString *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((KeyBits ** )__p)
	case 3: /*  */
		{
# line 169 "sec.py"

        if (((*parm) = (KeyBits *) calloc (1, sizeof (KeyBits))) == ((KeyBits *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	(*parm)->choice = 0;
        ;
		}
	    break;

#undef parm

#define parm	((KeyBits ** )__p)
	case 8: /*  */
		{
# line 230 "sec.py"

	if(! ((*parm)->part1.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->part1.noctets - 1; i++)
			(*parm)->part1.octets[i] = (*parm)->part1.octets[i + 1];
		(*parm)->part1.noctets -= 1;
		(*parm)->part1.octets = (char *)realloc((*parm)->part1.octets, (*parm)->part1.noctets);
	}

	if ((*parm)->part2.noctets > 0 && !((*parm)->part2.octets[0] | 0x00)){

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->part2.noctets - 1; i++)
			(*parm)->part2.octets[i] = (*parm)->part2.octets[i + 1];
		(*parm)->part2.noctets -= 1;
		(*parm)->part2.octets = (char *)realloc((*parm)->part2.octets, (*parm)->part2.noctets);
	}

	if ((*parm)->part3.noctets > 0 && !((*parm)->part3.octets[0] | 0x00)){

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->part3.noctets - 1; i++)
			(*parm)->part3.octets[i] = (*parm)->part3.octets[i + 1];
		(*parm)->part3.noctets -= 1;
		(*parm)->part3.octets = (char *)realloc((*parm)->part3.octets, (*parm)->part3.noctets);
	}

	if ((*parm)->part4.noctets > 0 && !((*parm)->part4.octets[0] | 0x00)){
	
		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->part4.noctets - 1; i++)
			(*parm)->part4.octets[i] = (*parm)->part4.octets[i + 1];
		(*parm)->part4.noctets -= 1;
		(*parm)->part4.octets = (char *)realloc((*parm)->part4.octets, (*parm)->part4.noctets);
	}
	;
		}
	    break;

#undef parm

#define parm	((KeyBits ** )__p)
	case 4: /*  */
		{
# line 182 "sec.py"
		/* ignored - empty expression */

		}
	    break;

#undef parm

#define parm	((KeyBits ** )__p)
	case 6: /*  */
		{
# line 201 "sec.py"
0;
		}
	    break;

#undef parm

#define parm	((KeyBits ** )__p)
	case 5: /*  */
		{
# line 194 "sec.py"

				(*parm)->choice = 1;
				;
		}
	    break;

#undef parm

#define parm	((KeyBits ** )__p)
	case 7: /*  */
		{
# line 205 "sec.py"
(*parm)->choice != 1;
		}
	    break;

#undef parm

#define parm	((KeyInfo ** )__p)
	case 9: /*  */
		{
# line 271 "sec.py"

        if (((*parm) = (KeyInfo *) calloc (1, sizeof (KeyInfo))) == ((KeyInfo *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((DigestInfo ** )__p)
	case 10: /*  */
		{
# line 289 "sec.py"

        if (((*parm) = (DigestInfo *) calloc (1, sizeof (DigestInfo))) == ((DigestInfo *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((EncryptedKey ** )__p)
	case 11: /*  */
		{
# line 307 "sec.py"

        if (((*parm) = (EncryptedKey *) calloc (1, sizeof (EncryptedKey))) == ((EncryptedKey *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((AlgId ** )__p)
	case 12: /*  */
		{
# line 328 "sec.py"

        if (((*parm) = (AlgId *) calloc (1, sizeof (AlgId))) == ((AlgId *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((AlgId ** )__p)
	case 13: /*  */
		{
# line 352 "sec.py"

		(*parm)->param = CNULL;
		(*parm)->un.des_parm = (desCBC_parm_type *)0;
		(*parm)->un.diffie_hellmann_parm = (KeyBits *)0;
	    	(*parm)->paramchoice = aux_ObjId2ParmType((*parm)->objid);
	    	if ((*parm)->paramchoice == NoParmType){
            		advise (NULLCP, "parm->paramchoice has undefined value");
            		return NOTOK;
       		}
		;
		}
	    break;

#undef parm

#define parm	((AlgId ** )__p)
	case 14: /*  */
		{
# line 364 "sec.py"
		((*parm)->paramchoice) = _val;

		}
	    break;

#undef parm

#define parm	((AlgId ** )__p)
	case 18: /*  */
		{
# line 395 "sec.py"
(*parm)->paramchoice > 0;
		}
	    break;

#undef parm

#define parm	((AlgId ** )__p)
	case 15: /*  */
		{
# line 369 "sec.py"

		  	rsa_parm = (rsa_parm_type *)malloc(sizeof(rsa_parm_type));
		  	*rsa_parm = (*parm)->un.keyorblocksize;
		  	(*parm)->param = (char *)rsa_parm;
     			;
		}
	    break;

#undef parm

#define parm	((AlgId ** )__p)
	case 16: /*  */
		{
# line 377 "sec.py"

		  	(*parm)->param = (char *)aux_cpy_OctetString((*parm)->un.des_parm);
		  	if((*parm)->un.des_parm)
				aux_free_OctetString(&((*parm)->un.des_parm));
    			;
		}
	    break;

#undef parm

#define parm	((AlgId ** )__p)
	case 17: /*  */
		{
# line 386 "sec.py"

		  	(*parm)->param = (char *)aux_cpy_KeyBits((*parm)->un.diffie_hellmann_parm);
		  	if((*parm)->un.diffie_hellmann_parm)
				aux_free_KeyBits(&((*parm)->un.diffie_hellmann_parm));
     			;
		}
	    break;

#undef parm

#define parm	((struct PSE_Objects ** )__p)
	case 19: /*  */
		{
# line 419 "sec.py"
0;
		}
	    break;

#undef parm

#define parm	((struct PSE_Objects ** )__p)
	case 20: /*  */
		{
# line 422 "sec.py"
0;
		}
	    break;

#undef parm

#define parm	((PSEToc ** )__p)
	case 21: /*  */
		{
# line 438 "sec.py"
0;
		}
	    break;

#undef parm

#define parm	((PSEToc ** )__p)
	case 22: /*  */
		{
# line 441 "sec.py"
0;
		}
	    break;

#undef parm

#define parm	((PSEObject ** )__p)
	case 23: /*  */
		{
# line 447 "sec.py"

        if (((*parm) = (PSEObject *) calloc (1, sizeof (PSEObject))) == ((PSEObject *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
	}
        ;
		}
	    break;

#undef parm

#define parm	((PSEObject ** )__p)
	case 24: /*  */
		{
# line 467 "sec.py"

		(*parm)->objectValue = aux_PE2OctetString((*parm)->asn);
		pe_free((*parm)->asn);
		;
		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, pe, "dnf_SEC:Bad table entry: %d",
            _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}
static ptpe *etabl[] = {
	et_TOURINTEGERSEC,
	et_TBitStringSEC,
	et_TAlgorithmIdentifierSEC,
	et_OctetStringSEC,
	et_SETOFOctetStringSEC,
	et_SETOFObjIdSEC,
	et_BitStringSEC,
	et_OURINTEGERSEC,
	et_KeyBitsSEC,
	et_KeyInfoSEC,
	et_DigestInfoSEC,
	et_EncryptedKeySEC,
	et_AlgorithmIdentifierSEC,
	et_SETOFAlgorithmIdentifierSEC,
	et_PSEObjectsSEC,
	et_PSETocSEC,
	et_PSEObjectSEC,
	};

static ptpe *dtabl[] = {
	dt_TOURINTEGERSEC,
	dt_TBitStringSEC,
	dt_TAlgorithmIdentifierSEC,
	dt_OctetStringSEC,
	dt_SETOFOctetStringSEC,
	dt_SETOFObjIdSEC,
	dt_BitStringSEC,
	dt_OURINTEGERSEC,
	dt_KeyBitsSEC,
	dt_KeyInfoSEC,
	dt_DigestInfoSEC,
	dt_EncryptedKeySEC,
	dt_AlgorithmIdentifierSEC,
	dt_SETOFAlgorithmIdentifierSEC,
	dt_PSEObjectsSEC,
	dt_PSETocSEC,
	dt_PSEObjectSEC,
	};

static ptpe *ptabl[] = {
	pt_TOURINTEGERSEC,
	pt_TBitStringSEC,
	pt_TAlgorithmIdentifierSEC,
	pt_OctetStringSEC,
	pt_SETOFOctetStringSEC,
	pt_SETOFObjIdSEC,
	pt_BitStringSEC,
	pt_OURINTEGERSEC,
	pt_KeyBitsSEC,
	pt_KeyInfoSEC,
	pt_DigestInfoSEC,
	pt_EncryptedKeySEC,
	pt_AlgorithmIdentifierSEC,
	pt_SETOFAlgorithmIdentifierSEC,
	pt_PSEObjectsSEC,
	pt_PSETocSEC,
	pt_PSEObjectSEC,
	};


/* Pointer table 41 entries */
caddr_t _ZptrtabSEC[] = {
    (caddr_t ) "TOURINTEGER",
    (caddr_t ) "TBitString",
    (caddr_t ) "TAlgorithmIdentifier",
    (caddr_t ) "OctetString",
    (caddr_t ) "SETOFOctetString",
    (caddr_t ) "SETOFObjId",
    (caddr_t ) "BitString",
    (caddr_t ) "OURINTEGER",
    (caddr_t ) "KeyBits",
    (caddr_t ) "part1",
    (caddr_t ) "part234",
    (caddr_t ) "prime1",
    (caddr_t ) "prime2",
    (caddr_t ) "base",
    (caddr_t ) "part2",
    (caddr_t ) "part3",
    (caddr_t ) "KeyInfo",
    (caddr_t ) "algorithm",
    (caddr_t ) "key",
    (caddr_t ) "DigestInfo",
    (caddr_t ) "digestai",
    (caddr_t ) "digest",
    (caddr_t ) "EncryptedKey",
    (caddr_t ) "encalg",
    (caddr_t ) "subalg",
    (caddr_t ) "AlgorithmIdentifier",
    (caddr_t ) "objectid",
    (caddr_t ) "parameters",
    (caddr_t ) "keyorblocksize",
    (caddr_t ) "desIv",
    (caddr_t ) "diffhell",
    (caddr_t ) "nullparm",
    (caddr_t ) "SETOFAlgorithmIdentifier",
    (caddr_t ) "PSEObjects",
    (caddr_t ) "noOctets",
    (caddr_t ) "status",
    (caddr_t ) "PSEToc",
    (caddr_t ) "sCObjects",
    (caddr_t ) "PSEObject",
    (caddr_t ) "type",
    (caddr_t ) "value",
};

modtyp _ZSEC_mod = {
	"SEC",
	17,
	etabl,
	dtabl,
	ptabl,
	efn_SEC,
	dfn_SEC,
	0,
	_ZptrtabSEC,
	};


#if	defined(lint) || defined(PEPSY_LINKABLE_FUNCS)

#undef parse_SEC_TOURINTEGER
int	parse_SEC_TOURINTEGER(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
OctetString  **parm;
{
  return (dec_f(_ZTOURINTEGERSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef parse_SEC_TBitString
int	parse_SEC_TBitString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Signature  **parm;
{
  return (dec_f(_ZTBitStringSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef parse_SEC_TAlgorithmIdentifier
int	parse_SEC_TAlgorithmIdentifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Signature  **parm;
{
  return (dec_f(_ZTAlgorithmIdentifierSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_TAlgorithmIdentifier
void free_SEC_TAlgorithmIdentifier(val)
Signature  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZTAlgorithmIdentifierSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_OctetString
int	parse_SEC_OctetString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
OctetString  **parm;
{
  return (dec_f(_ZOctetStringSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef parse_SEC_SETOFOctetString
int	parse_SEC_SETOFOctetString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_OctetString  **parm;
{
  return (dec_f(_ZSETOFOctetStringSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_SETOFOctetString
void free_SEC_SETOFOctetString(val)
SET_OF_OctetString  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZSETOFOctetStringSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_SETOFObjId
int	parse_SEC_SETOFObjId(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_ObjId  **parm;
{
  return (dec_f(_ZSETOFObjIdSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_SETOFObjId
void free_SEC_SETOFObjId(val)
SET_OF_ObjId  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZSETOFObjIdSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_BitString
int	parse_SEC_BitString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
BitString  **parm;
{
  return (dec_f(_ZBitStringSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef parse_SEC_OURINTEGER
int	parse_SEC_OURINTEGER(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
OctetString  **parm;
{
  return (dec_f(_ZOURINTEGERSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef parse_SEC_KeyBits
int	parse_SEC_KeyBits(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
KeyBits  **parm;
{
  return (dec_f(_ZKeyBitsSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_KeyBits
void free_SEC_KeyBits(val)
KeyBits  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZKeyBitsSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_KeyInfo
int	parse_SEC_KeyInfo(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
KeyInfo  **parm;
{
  return (dec_f(_ZKeyInfoSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_KeyInfo
void free_SEC_KeyInfo(val)
KeyInfo  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZKeyInfoSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_DigestInfo
int	parse_SEC_DigestInfo(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
DigestInfo  **parm;
{
  return (dec_f(_ZDigestInfoSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_DigestInfo
void free_SEC_DigestInfo(val)
DigestInfo  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZDigestInfoSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_EncryptedKey
int	parse_SEC_EncryptedKey(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
EncryptedKey  **parm;
{
  return (dec_f(_ZEncryptedKeySEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_EncryptedKey
void free_SEC_EncryptedKey(val)
EncryptedKey  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZEncryptedKeySEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_AlgorithmIdentifier
int	parse_SEC_AlgorithmIdentifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AlgId  **parm;
{
  return (dec_f(_ZAlgorithmIdentifierSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_AlgorithmIdentifier
void free_SEC_AlgorithmIdentifier(val)
AlgId  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZAlgorithmIdentifierSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_SETOFAlgorithmIdentifier
int	parse_SEC_SETOFAlgorithmIdentifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_AlgId  **parm;
{
  return (dec_f(_ZSETOFAlgorithmIdentifierSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_SETOFAlgorithmIdentifier
void free_SEC_SETOFAlgorithmIdentifier(val)
SET_OF_AlgId  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZSETOFAlgorithmIdentifierSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_PSEObjects
int	parse_SEC_PSEObjects(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct PSE_Objects  **parm;
{
  return (dec_f(_ZPSEObjectsSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_PSEObjects
void free_SEC_PSEObjects(val)
struct PSE_Objects  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZPSEObjectsSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_PSEToc
int	parse_SEC_PSEToc(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
PSEToc  **parm;
{
  return (dec_f(_ZPSETocSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_PSEToc
void free_SEC_PSEToc(val)
PSEToc  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZPSETocSEC], &_ZSEC_mod, 1);
}

#undef parse_SEC_PSEObject
int	parse_SEC_PSEObject(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
PSEObject  **parm;
{
  return (dec_f(_ZPSEObjectSEC, &_ZSEC_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SEC_PSEObject
void free_SEC_PSEObject(val)
PSEObject  *val;
{
(void) fre_obj((char *) val, _ZSEC_mod.md_dtab[_ZPSEObjectSEC], &_ZSEC_mod, 1);
}

#endif	/* lint||PEPSY_LINKABLE_FUNCS */

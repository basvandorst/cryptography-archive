/* automatically generated by pepsy 10.0 #12 (tiber), do not edit! */

#ifndef	_module_SECAF_defined_
#define	_module_SECAF_defined_

#ifndef	PEPSY_VERSION
#define	PEPSY_VERSION		2
#endif

#include <isode/asn1/asn1.h>
#include <isode/asn1/pepsy.h>
#include <isode/pepsy/UNIV-types.h>


extern modtyp	_ZSECAF_mod;
#define _ZRootKeySECAF	3
#define _ZCertificatePairsSECAF	27
#define _ZOldCertificateSECAF	25
#define _ZPKListSECAF	11
#define _ZTBSCRLSECAF	12
#define _ZAliasListMemberSECAF	23
#define _ZCrlSetSECAF	18
#define _ZCRLEntrySECAF	15
#define _ZCrlSECAF	17
#define _ZCertificateSECAF	5
#define _ZCRLEntrySequenceSECAF	16
#define _ZAliasesSECAF	22
#define _ZAliasListSECAF	24
#define _ZTBSCertificateSECAF	4
#define _ZCertificatePairSECAF	9
#define _ZAliasesMemberSECAF	21
#define _ZValiditySECAF	1
#define _ZOldCertificateListSECAF	26
#define _ZCRLWithCertificatesSECAF	20
#define _ZForwardCertificationPathSECAF	7
#define _ZCRLSECAF	13
#define _ZCrossCertificatePairSECAF	10
#define _ZCertificationPathSECAF	19
#define _ZPKRootSECAF	2
#define _ZTCertificatePairSECAF	0
#define _ZSETOFCRLSECAF	14
#define _ZCertificatesSECAF	6
#define _ZCertificateSetSECAF	8

#ifndef	lint
#define build_SECAF_TCertificatePair(pe, top, len, buffer, parm) \
    enc_f(_ZTCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_TCertificatePair(pe, top, len, buffer, parm) \
    dec_f(_ZTCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_TCertificatePair(pe, top, len, buffer, parm) \
    prnt_f(_ZTCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_TCertificatePair_P    _ZTCertificatePairSECAF, &_ZSECAF_mod

#define build_SECAF_Validity(pe, top, len, buffer, parm) \
    enc_f(_ZValiditySECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_Validity(pe, top, len, buffer, parm) \
    dec_f(_ZValiditySECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_Validity(pe, top, len, buffer, parm) \
    prnt_f(_ZValiditySECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_Validity_P    _ZValiditySECAF, &_ZSECAF_mod

#define build_SECAF_PKRoot(pe, top, len, buffer, parm) \
    enc_f(_ZPKRootSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_PKRoot(pe, top, len, buffer, parm) \
    dec_f(_ZPKRootSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_PKRoot(pe, top, len, buffer, parm) \
    prnt_f(_ZPKRootSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_PKRoot_P    _ZPKRootSECAF, &_ZSECAF_mod

#define build_SECAF_RootKey(pe, top, len, buffer, parm) \
    enc_f(_ZRootKeySECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_RootKey(pe, top, len, buffer, parm) \
    dec_f(_ZRootKeySECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_RootKey(pe, top, len, buffer, parm) \
    prnt_f(_ZRootKeySECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_RootKey_P    _ZRootKeySECAF, &_ZSECAF_mod

#define build_SECAF_TBSCertificate(pe, top, len, buffer, parm) \
    enc_f(_ZTBSCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_TBSCertificate(pe, top, len, buffer, parm) \
    dec_f(_ZTBSCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_TBSCertificate(pe, top, len, buffer, parm) \
    prnt_f(_ZTBSCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_TBSCertificate_P    _ZTBSCertificateSECAF, &_ZSECAF_mod

#define build_SECAF_Certificate(pe, top, len, buffer, parm) \
    enc_f(_ZCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_Certificate(pe, top, len, buffer, parm) \
    dec_f(_ZCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_Certificate(pe, top, len, buffer, parm) \
    prnt_f(_ZCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_Certificate_P    _ZCertificateSECAF, &_ZSECAF_mod

#define build_SECAF_Certificates(pe, top, len, buffer, parm) \
    enc_f(_ZCertificatesSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_Certificates(pe, top, len, buffer, parm) \
    dec_f(_ZCertificatesSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_Certificates(pe, top, len, buffer, parm) \
    prnt_f(_ZCertificatesSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_Certificates_P    _ZCertificatesSECAF, &_ZSECAF_mod

#define build_SECAF_ForwardCertificationPath(pe, top, len, buffer, parm) \
    enc_f(_ZForwardCertificationPathSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_ForwardCertificationPath(pe, top, len, buffer, parm) \
    dec_f(_ZForwardCertificationPathSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_ForwardCertificationPath(pe, top, len, buffer, parm) \
    prnt_f(_ZForwardCertificationPathSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_ForwardCertificationPath_P    _ZForwardCertificationPathSECAF, &_ZSECAF_mod

#define build_SECAF_CertificateSet(pe, top, len, buffer, parm) \
    enc_f(_ZCertificateSetSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CertificateSet(pe, top, len, buffer, parm) \
    dec_f(_ZCertificateSetSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CertificateSet(pe, top, len, buffer, parm) \
    prnt_f(_ZCertificateSetSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CertificateSet_P    _ZCertificateSetSECAF, &_ZSECAF_mod

#define build_SECAF_CertificatePair(pe, top, len, buffer, parm) \
    enc_f(_ZCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CertificatePair(pe, top, len, buffer, parm) \
    dec_f(_ZCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CertificatePair(pe, top, len, buffer, parm) \
    prnt_f(_ZCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CertificatePair_P    _ZCertificatePairSECAF, &_ZSECAF_mod

#define build_SECAF_CrossCertificatePair(pe, top, len, buffer, parm) \
    enc_f(_ZCrossCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CrossCertificatePair(pe, top, len, buffer, parm) \
    dec_f(_ZCrossCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CrossCertificatePair(pe, top, len, buffer, parm) \
    prnt_f(_ZCrossCertificatePairSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CrossCertificatePair_P    _ZCrossCertificatePairSECAF, &_ZSECAF_mod

#define build_SECAF_PKList(pe, top, len, buffer, parm) \
    enc_f(_ZPKListSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_PKList(pe, top, len, buffer, parm) \
    dec_f(_ZPKListSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_PKList(pe, top, len, buffer, parm) \
    prnt_f(_ZPKListSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_PKList_P    _ZPKListSECAF, &_ZSECAF_mod

#define build_SECAF_TBSCRL(pe, top, len, buffer, parm) \
    enc_f(_ZTBSCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_TBSCRL(pe, top, len, buffer, parm) \
    dec_f(_ZTBSCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_TBSCRL(pe, top, len, buffer, parm) \
    prnt_f(_ZTBSCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_TBSCRL_P    _ZTBSCRLSECAF, &_ZSECAF_mod

#define build_SECAF_CRL(pe, top, len, buffer, parm) \
    enc_f(_ZCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CRL(pe, top, len, buffer, parm) \
    dec_f(_ZCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CRL(pe, top, len, buffer, parm) \
    prnt_f(_ZCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CRL_P    _ZCRLSECAF, &_ZSECAF_mod

#define build_SECAF_SETOFCRL(pe, top, len, buffer, parm) \
    enc_f(_ZSETOFCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_SETOFCRL(pe, top, len, buffer, parm) \
    dec_f(_ZSETOFCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_SETOFCRL(pe, top, len, buffer, parm) \
    prnt_f(_ZSETOFCRLSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_SETOFCRL_P    _ZSETOFCRLSECAF, &_ZSECAF_mod

#define build_SECAF_CRLEntry(pe, top, len, buffer, parm) \
    enc_f(_ZCRLEntrySECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CRLEntry(pe, top, len, buffer, parm) \
    dec_f(_ZCRLEntrySECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CRLEntry(pe, top, len, buffer, parm) \
    prnt_f(_ZCRLEntrySECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CRLEntry_P    _ZCRLEntrySECAF, &_ZSECAF_mod

#define build_SECAF_CRLEntrySequence(pe, top, len, buffer, parm) \
    enc_f(_ZCRLEntrySequenceSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CRLEntrySequence(pe, top, len, buffer, parm) \
    dec_f(_ZCRLEntrySequenceSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CRLEntrySequence(pe, top, len, buffer, parm) \
    prnt_f(_ZCRLEntrySequenceSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CRLEntrySequence_P    _ZCRLEntrySequenceSECAF, &_ZSECAF_mod

#define build_SECAF_Crl(pe, top, len, buffer, parm) \
    enc_f(_ZCrlSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_Crl(pe, top, len, buffer, parm) \
    dec_f(_ZCrlSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_Crl(pe, top, len, buffer, parm) \
    prnt_f(_ZCrlSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_Crl_P    _ZCrlSECAF, &_ZSECAF_mod

#define build_SECAF_CrlSet(pe, top, len, buffer, parm) \
    enc_f(_ZCrlSetSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CrlSet(pe, top, len, buffer, parm) \
    dec_f(_ZCrlSetSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CrlSet(pe, top, len, buffer, parm) \
    prnt_f(_ZCrlSetSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CrlSet_P    _ZCrlSetSECAF, &_ZSECAF_mod

#define build_SECAF_CertificationPath(pe, top, len, buffer, parm) \
    enc_f(_ZCertificationPathSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CertificationPath(pe, top, len, buffer, parm) \
    dec_f(_ZCertificationPathSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CertificationPath(pe, top, len, buffer, parm) \
    prnt_f(_ZCertificationPathSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CertificationPath_P    _ZCertificationPathSECAF, &_ZSECAF_mod

#define build_SECAF_CRLWithCertificates(pe, top, len, buffer, parm) \
    enc_f(_ZCRLWithCertificatesSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CRLWithCertificates(pe, top, len, buffer, parm) \
    dec_f(_ZCRLWithCertificatesSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CRLWithCertificates(pe, top, len, buffer, parm) \
    prnt_f(_ZCRLWithCertificatesSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CRLWithCertificates_P    _ZCRLWithCertificatesSECAF, &_ZSECAF_mod

#define build_SECAF_AliasesMember(pe, top, len, buffer, parm) \
    enc_f(_ZAliasesMemberSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_AliasesMember(pe, top, len, buffer, parm) \
    dec_f(_ZAliasesMemberSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_AliasesMember(pe, top, len, buffer, parm) \
    prnt_f(_ZAliasesMemberSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_AliasesMember_P    _ZAliasesMemberSECAF, &_ZSECAF_mod

#define build_SECAF_Aliases(pe, top, len, buffer, parm) \
    enc_f(_ZAliasesSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_Aliases(pe, top, len, buffer, parm) \
    dec_f(_ZAliasesSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_Aliases(pe, top, len, buffer, parm) \
    prnt_f(_ZAliasesSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_Aliases_P    _ZAliasesSECAF, &_ZSECAF_mod

#define build_SECAF_AliasListMember(pe, top, len, buffer, parm) \
    enc_f(_ZAliasListMemberSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_AliasListMember(pe, top, len, buffer, parm) \
    dec_f(_ZAliasListMemberSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_AliasListMember(pe, top, len, buffer, parm) \
    prnt_f(_ZAliasListMemberSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_AliasListMember_P    _ZAliasListMemberSECAF, &_ZSECAF_mod

#define build_SECAF_AliasList(pe, top, len, buffer, parm) \
    enc_f(_ZAliasListSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_AliasList(pe, top, len, buffer, parm) \
    dec_f(_ZAliasListSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_AliasList(pe, top, len, buffer, parm) \
    prnt_f(_ZAliasListSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_AliasList_P    _ZAliasListSECAF, &_ZSECAF_mod

#define build_SECAF_OldCertificate(pe, top, len, buffer, parm) \
    enc_f(_ZOldCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_OldCertificate(pe, top, len, buffer, parm) \
    dec_f(_ZOldCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_OldCertificate(pe, top, len, buffer, parm) \
    prnt_f(_ZOldCertificateSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_OldCertificate_P    _ZOldCertificateSECAF, &_ZSECAF_mod

#define build_SECAF_OldCertificateList(pe, top, len, buffer, parm) \
    enc_f(_ZOldCertificateListSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_OldCertificateList(pe, top, len, buffer, parm) \
    dec_f(_ZOldCertificateListSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_OldCertificateList(pe, top, len, buffer, parm) \
    prnt_f(_ZOldCertificateListSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_OldCertificateList_P    _ZOldCertificateListSECAF, &_ZSECAF_mod

#define build_SECAF_CertificatePairs(pe, top, len, buffer, parm) \
    enc_f(_ZCertificatePairsSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char *) parm)

#define parse_SECAF_CertificatePairs(pe, top, len, buffer, parm) \
    dec_f(_ZCertificatePairsSECAF, &_ZSECAF_mod, pe, top, len, buffer, (char **) parm)

#define print_SECAF_CertificatePairs(pe, top, len, buffer, parm) \
    prnt_f(_ZCertificatePairsSECAF, &_ZSECAF_mod, pe, top, len, buffer)
#define print_SECAF_CertificatePairs_P    _ZCertificatePairsSECAF, &_ZSECAF_mod


#endif   /* lint */
#include "SECIF-types.h"
#include "SEC-types.h"


#define	type_SECAF_TCertificatePair	type_SECAF_CertificatePair
#define	free_SECAF_TCertificatePair	free_SECAF_CertificatePair

struct type_SECAF_Validity {
    struct	qbuf	*notBefore;

    struct	qbuf	*notAfter;
};
#define	free_SECAF_Validity(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZValiditySECAF], &_ZSECAF_mod, 1)

struct type_SECAF_PKRoot {
    struct type_SECIF_Name *cA;

    struct type_SECAF_RootKey *newkey;

    struct type_SECAF_RootKey *oldkey;
};
#define	free_SECAF_PKRoot(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZPKRootSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_RootKey {
    integer     version;
#define	int_SECAF_version_v1988	0
#define	int_SECAF_version_v1992	1

    struct type_SEC_OURINTEGER *serial;

    struct type_SEC_KeyInfo *key;

    struct type_SECAF_Validity *validity;

    struct type_SEC_TAlgorithmIdentifier *signAlgID;

    struct type_SEC_TBitString *signBitString;
};
#define	free_SECAF_RootKey(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZRootKeySECAF], &_ZSECAF_mod, 1)

struct type_SECAF_TBSCertificate {
    integer     optionals;
#define	opt_SECAF_TBSCertificate_version (000000001)

    integer     version;
#define	int_SECAF_version_v1988	0
#define	int_SECAF_version_v1992	1

    struct type_SEC_OURINTEGER *serialNumber;

    struct type_SEC_AlgorithmIdentifier *signature;

    struct type_SECIF_Name *issuer;

    struct type_SECAF_Validity *validity;

    struct type_SECIF_Name *subject;

    struct type_SEC_KeyInfo *subjectPublickeyInfo;
};
#define	free_SECAF_TBSCertificate(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZTBSCertificateSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_Certificate {
    struct type_SECAF_TBSCertificate *element_SECAF_0;

    struct type_SEC_TAlgorithmIdentifier *element_SECAF_1;

    struct type_SEC_TBitString *element_SECAF_2;
};
#define	free_SECAF_Certificate(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCertificateSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_Certificates {
    struct type_SECAF_Certificate *certificate;

    struct type_SECAF_ForwardCertificationPath *certificationPath;
};
#define	free_SECAF_Certificates(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCertificatesSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_ForwardCertificationPath {
        struct type_SECAF_CertificateSet *CertificateSet;

        struct type_SECAF_ForwardCertificationPath *next;
};
#define	free_SECAF_ForwardCertificationPath(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZForwardCertificationPathSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CertificateSet {
        struct type_SECAF_Certificate *Certificate;

        struct type_SECAF_CertificateSet *next;
};
#define	free_SECAF_CertificateSet(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCertificateSetSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CertificatePair {
    struct type_SECAF_Certificate *forward;

    struct type_SECAF_Certificate *reverse;
};
#define	free_SECAF_CertificatePair(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCertificatePairSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CrossCertificatePair {
        struct type_SECAF_CertificatePair *CertificatePair;

        struct type_SECAF_CrossCertificatePair *next;
};
#define	free_SECAF_CrossCertificatePair(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCrossCertificatePairSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_PKList {
        struct type_SECAF_TBSCertificate *TBSCertificate;

        struct type_SECAF_PKList *next;
};
#define	free_SECAF_PKList(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZPKListSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_TBSCRL {
    struct type_SEC_AlgorithmIdentifier *signature;

    struct type_SECIF_Name *issuer;

    struct	qbuf	*lastUpdate;

    struct	qbuf	*nextUpdate;

    struct type_SECAF_CRLEntrySequence *revokedCertificates;
};
#define	free_SECAF_TBSCRL(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZTBSCRLSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CRL {
    struct type_SECAF_TBSCRL *element_SECAF_3;

    struct type_SEC_TAlgorithmIdentifier *element_SECAF_4;

    struct type_SEC_TBitString *element_SECAF_5;
};
#define	free_SECAF_CRL(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCRLSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_SETOFCRL {
        struct type_SECAF_CRL *CRL;

        struct type_SECAF_SETOFCRL *next;
};
#define	free_SECAF_SETOFCRL(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZSETOFCRLSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CRLEntry {
    struct type_SEC_OURINTEGER *serialNumber;

    struct	qbuf	*revocationDate;
};
#define	free_SECAF_CRLEntry(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCRLEntrySECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CRLEntrySequence {
        struct type_SECAF_CRLEntry *CRLEntry;

        struct type_SECAF_CRLEntrySequence *next;
};
#define	free_SECAF_CRLEntrySequence(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCRLEntrySequenceSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_Crl {
    struct type_SECIF_Name *issuer;

    struct	qbuf	*nextUpdate;

    struct type_SECAF_CRLEntrySequence *revcerts;
};
#define	free_SECAF_Crl(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCrlSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CrlSet {
        struct type_SECAF_Crl *Crl;

        struct type_SECAF_CrlSet *next;
};
#define	free_SECAF_CrlSet(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCrlSetSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CertificationPath {
    struct type_SECAF_Certificate *userCertificate;

    struct type_SECAF_CertificatePairs *theCACertificates;
};
#define	free_SECAF_CertificationPath(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCertificationPathSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CRLWithCertificates {
    struct type_SECAF_CRL *pemCRL;

    struct type_SECAF_Certificates *certificates;
};
#define	free_SECAF_CRLWithCertificates(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCRLWithCertificatesSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_AliasesMember {
    struct	qbuf	*aname;

    integer     aliasfile;
#define	int_SECAF_aliasfile_useralias	0
#define	int_SECAF_aliasfile_sytemalias	1
};
#define	free_SECAF_AliasesMember(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZAliasesMemberSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_Aliases {
        struct type_SECAF_AliasesMember *AliasesMember;

        struct type_SECAF_Aliases *next;
};
#define	free_SECAF_Aliases(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZAliasesSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_AliasListMember {
    struct type_SECAF_Aliases *aliases;

    struct	qbuf	*dname;
};
#define	free_SECAF_AliasListMember(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZAliasListMemberSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_AliasList {
        struct type_SECAF_AliasListMember *AliasListMember;

        struct type_SECAF_AliasList *next;
};
#define	free_SECAF_AliasList(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZAliasListSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_OldCertificate {
    struct type_SEC_OURINTEGER *serialNumber;

    struct type_SECAF_Certificate *crossCertificate;
};
#define	free_SECAF_OldCertificate(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZOldCertificateSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_OldCertificateList {
        struct type_SECAF_OldCertificate *OldCertificate;

        struct type_SECAF_OldCertificateList *next;
};
#define	free_SECAF_OldCertificateList(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZOldCertificateListSECAF], &_ZSECAF_mod, 1)

struct type_SECAF_CertificatePairs {
        struct type_SECAF_TCertificatePair *TCertificatePair;

        struct type_SECAF_CertificatePairs *next;
};
#define	free_SECAF_CertificatePairs(parm)\
	(void) fre_obj((char *) parm, _ZSECAF_mod.md_dtab[_ZCertificatePairsSECAF], &_ZSECAF_mod, 1)
#endif

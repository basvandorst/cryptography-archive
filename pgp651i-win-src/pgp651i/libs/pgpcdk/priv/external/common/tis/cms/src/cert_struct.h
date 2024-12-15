/*******************************************************************
 *
 * This file was generated by TIS/ASN1COMP Ver. 4.2, an ASN.1 compiler.
 * TIS/ASN1COMP is Copyright (c) 1998, TIS Labs at Network Associates, Inc.
 *
 * This file was AUTOMATICALLY GENERATED on Mon May 17 09:25:53 1999
 *
 ******************************************************************/

#ifndef cert_STRUCT_H_
#define cert_STRUCT_H_

#include "cert_util.h"
#include "cert_sizes.h"


/*******************************************************************
 *
 * Structure and type definitions
 *
 ******************************************************************/


typedef PKIOBJECT_ID   PKIAttributeType;
#define PKIID_AttributeType PKIID_OBJECT_ID

typedef PKIANY   PKIAttributeValue;
#define PKIID_AttributeValue PKIID_ANY

typedef PKIOCTET_STRING PKIBMPString;
#define PKIID_BMPString 0x1e

typedef PKIINTEGER   PKIBaseDistance;
#define PKIID_BaseDistance PKIID_INTEGER

typedef PKIIA5String   PKICPSuri;
#define PKIID_CPSuri PKIID_IA5String

typedef PKIINTEGER   PKICRLNumber;
#define PKIID_CRLNumber PKIID_INTEGER

typedef PKIINTEGER  PKICRLReason;
#define PKIID_CRLReason PKIID_INTEGER

/* define values for PKICRLReason */
#define PKICRLReason_affiliationChanged 3	
#define PKICRLReason_cACompromise 2	
#define PKICRLReason_certificateHold 6	
#define PKICRLReason_cessationOfOperation 5	
#define PKICRLReason_keyCompromise 1	
#define PKICRLReason_removeFromCRL 8	
#define PKICRLReason_superseded 4	
#define PKICRLReason_unspecified 0	
/* end of values for PKICRLReason */

typedef PKIOBJECT_ID   PKICertPolicyId;
#define PKIID_CertPolicyId PKIID_OBJECT_ID

typedef PKIINTEGER   PKICertificateSerialNumber;
#define PKIID_CertificateSerialNumber PKIID_INTEGER

typedef PKIINTEGER   PKIDSAPublicKey;
#define PKIID_DSAPublicKey PKIID_INTEGER

typedef PKIOBJECT_ID   PKIHoldInstructionCode;
#define PKIID_HoldInstructionCode PKIID_OBJECT_ID

typedef PKIGeneralizedTime   PKIInvalidityDate;
#define PKIID_InvalidityDate PKIID_GeneralizedTime

typedef PKIOCTET_STRING   PKIKeyIdentifier;
#define PKIID_KeyIdentifier PKIID_OCTET_STRING

typedef PKIOBJECT_ID   PKIKeyPurposeId;
#define PKIID_KeyPurposeId PKIID_OBJECT_ID

typedef PKIBIT_STRING PKIKeyUsage;
#define PKIID_KeyUsage PKIID_BIT_STRING

/* define bits for PKIKeyUsage */
#define PKIKeyUsage_cRLSign 0x40	/* bit 6 */
#define PKIKeyUsage_dataEncipherment 0x8	/* bit 3 */
#define PKIKeyUsage_decipherOnly 0x100	/* bit 8 */
#define PKIKeyUsage_digitalSignature 0x1	/* bit 0 */
#define PKIKeyUsage_encipherOnly 0x80	/* bit 7 */
#define PKIKeyUsage_keyAgreement 0x10	/* bit 4 */
#define PKIKeyUsage_keyCertSign 0x20	/* bit 5 */
#define PKIKeyUsage_keyEncipherment 0x4	/* bit 2 */
#define PKIKeyUsage_nonRepudiation 0x2	/* bit 1 */
/* end of bits for PKIKeyUsage */

typedef PKIIA5String   PKIPkcs9email;
#define PKIID_Pkcs9email PKIID_IA5String

typedef PKIOBJECT_ID   PKIPolicyQualifierId;
#define PKIID_PolicyQualifierId PKIID_OBJECT_ID

typedef PKIBIT_STRING PKIReasonFlags;
#define PKIID_ReasonFlags PKIID_BIT_STRING

/* define bits for PKIReasonFlags */
#define PKIReasonFlags_affiliationChanged 0x8	/* bit 3 */
#define PKIReasonFlags_cACompromise 0x4	/* bit 2 */
#define PKIReasonFlags_certificateHold 0x40	/* bit 6 */
#define PKIReasonFlags_cessationOfOperation 0x20	/* bit 5 */
#define PKIReasonFlags_keyCompromise 0x2	/* bit 1 */
#define PKIReasonFlags_superseded 0x10	/* bit 4 */
#define PKIReasonFlags_unused 0x1	/* bit 0 */
/* end of bits for PKIReasonFlags */

typedef PKIINTEGER   PKISkipCerts;
#define PKIID_SkipCerts PKIID_INTEGER

typedef PKIOCTET_STRING PKIUTF8String;
#define PKIID_UTF8String 0x0c

typedef PKIBIT_STRING   PKIUniqueIdentifier;
#define PKIID_UniqueIdentifier PKIID_BIT_STRING

typedef PKIOCTET_STRING PKIUniversalString;
#define PKIID_UniversalString 0x1c

typedef PKIINTEGER  PKIVersion;
#define PKIID_Version PKIID_INTEGER

/* define values for PKIVersion */
#define PKIVersion_v1 0	
#define PKIVersion_v2 1	
#define PKIVersion_v3 2	
/* end of values for PKIVersion */

typedef struct {
    long  n;          /* number of elements */
    PKIINTEGER *elt[ PKIMAX_noticeNumbers_SEQ_OF ];
} PKInoticeNumbers_SEQ_OF;
#define PKIID_noticeNumbers_SEQ_OF PKIID_SEQUENCE_OF

typedef struct {
    PKIOBJECT_ID  algorithm;
    PKIANY *parameters;
} PKIAlgorithmIdentifier;
#define PKIID_AlgorithmIdentifier PKIID_SEQUENCE

typedef struct {
    PKIOBJECT_ID  type_id;
    PKIANY  value;
} PKIAnotherName;
#define PKIID_AnotherName PKIID_SEQUENCE

typedef struct {
    PKIAttributeType  type;
    PKIAttributeValue  value;
} PKIAttributeTypeAndValue;
#define PKIID_AttributeTypeAndValue PKIID_SEQUENCE

typedef PKICRLNumber PKIBaseCRLNumber;
#define PKIID_BaseCRLNumber PKIID_CRLNumber

typedef struct {
    PKIBOOLEAN *cA;
    PKIINTEGER *pathLenConstraint;
} PKIBasicConstraints;
#define PKIID_BasicConstraints PKIID_SEQUENCE

typedef struct {
    unsigned char CHOICE_field_type;
    void *data;
} PKIDirectoryString;
#define PKIID_DirectoryString PKIID_CHOICE
/* tags for fields defined for DirectoryString */
#define PKIteletexString_DirectoryStringFieldTag PKIID_T61String
#define PKIprintableString_DirectoryStringFieldTag PKIID_PrintableString
#define PKIuniversalString_DirectoryStringFieldTag PKIID_UniversalString
#define PKIutf8String_DirectoryStringFieldTag PKIID_UTF8String
#define PKIbmpString_DirectoryStringFieldTag PKIID_BMPString

typedef struct {
    unsigned char CHOICE_field_type;
    void *data;
} PKIDisplayText;
#define PKIID_DisplayText PKIID_CHOICE
/* tags for fields defined for DisplayText */
#define PKIvisibleString_DisplayTextFieldTag PKIID_VisibleString
#define PKIbmpString_DisplayTextFieldTag PKIID_BMPString
#define PKIutf8String_DisplayTextFieldTag PKIID_UTF8String

typedef struct {
    PKIINTEGER  p;
    PKIINTEGER  q;
    PKIINTEGER  g;
} PKIDss_Parms;
#define PKIID_Dss_Parms PKIID_SEQUENCE

typedef struct {
    PKIINTEGER  r;
    PKIINTEGER  s;
} PKIDss_Sig_Value;
#define PKIID_Dss_Sig_Value PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIKeyPurposeId *elt[ PKIMAX_ExtKeyUsageSyntax ];
} PKIExtKeyUsageSyntax;
#define PKIID_ExtKeyUsageSyntax PKIID_SEQUENCE_OF

typedef struct {
    PKIOBJECT_ID  extnID;
    PKIBOOLEAN *critical;
    PKIOCTET_STRING  extnValue;
} PKIExtension;
#define PKIID_Extension PKIID_SEQUENCE

typedef struct {
    PKISkipCerts *requireExplicitPolicy;
    PKISkipCerts *inhibitPolicyMapping;
} PKIPolicyConstraints;
#define PKIID_PolicyConstraints PKIID_SEQUENCE

typedef struct {
    PKICertPolicyId  issuerDomainPolicy;
    PKICertPolicyId  subjectDomainPolicy;
} PKIPolicyMappings_SEQUENCE;
#define PKIID_PolicyMappings_SEQUENCE PKIID_SEQUENCE

typedef struct {
    PKIPolicyQualifierId  policyQualifierId;
    PKIANY  qualifier;
} PKIPolicyQualifierInfo;
#define PKIID_PolicyQualifierInfo PKIID_SEQUENCE

typedef struct {
    PKIGeneralizedTime *notBefore;
    PKIGeneralizedTime *notAfter;
} PKIPrivateKeyUsagePeriod;
#define PKIID_PrivateKeyUsagePeriod PKIID_SEQUENCE

typedef struct {
    PKIINTEGER  modulus;
    PKIINTEGER  exponent;
} PKIRSAKey;
#define PKIID_RSAKey PKIID_SEQUENCE

typedef PKIKeyIdentifier PKISubjectKeyIdentifier;
#define PKIID_SubjectKeyIdentifier PKIID_KeyIdentifier

typedef struct {
    unsigned char CHOICE_field_type;
    void *data;
} PKITime;
#define PKIID_Time PKIID_CHOICE
/* tags for fields defined for Time */
#define PKIutcTime_TimeFieldTag PKIID_UTCTime
#define PKIgeneralTime_TimeFieldTag PKIID_GeneralizedTime

typedef struct {
    PKIBIT_STRING  seed;
    PKIINTEGER  pgenCounter;
} PKIValidationParms;
#define PKIID_ValidationParms PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIAttributeValue *elt[ PKIMAX_values_SET_OF ];
} PKIvalues_SET_OF;
#define PKIID_values_SET_OF PKIID_SET_OF

typedef struct {
    PKIAttributeType  type;
    PKIvalues_SET_OF  values;
} PKIAttribute;
#define PKIID_Attribute PKIID_SEQUENCE

typedef struct {
    PKIINTEGER  p;
    PKIINTEGER  g;
    PKIINTEGER  q;
    PKIINTEGER *j;
    PKIValidationParms *validationParms;
} PKIDomainParameters;
#define PKIID_DomainParameters PKIID_SEQUENCE

typedef struct {
    PKIDirectoryString *nameAssigner;
    PKIDirectoryString  partyName;
} PKIEDIPartyName;
#define PKIID_EDIPartyName PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIExtension *elt[ PKIMAX_Extensions ];
} PKIExtensions;
#define PKIID_Extensions PKIID_SEQUENCE_OF

typedef struct {
    PKIDisplayText  organization;
    PKInoticeNumbers_SEQ_OF  noticeNumbers;
} PKINoticeReference;
#define PKIID_NoticeReference PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIPolicyMappings_SEQUENCE *elt[ PKIMAX_PolicyMappings ];
} PKIPolicyMappings;
#define PKIID_PolicyMappings PKIID_SEQUENCE_OF

typedef struct {
    long  n;          /* number of elements */
    PKIAttributeTypeAndValue *elt[ PKIMAX_RelativeDistinguishedName ];
} PKIRelativeDistinguishedName;
#define PKIID_RelativeDistinguishedName PKIID_SET_OF

typedef struct {
    PKIAlgorithmIdentifier  algorithm;
    PKIBIT_STRING  subjectPublicKey;
} PKISubjectPublicKeyInfo;
#define PKIID_SubjectPublicKeyInfo PKIID_SEQUENCE

typedef struct {
    PKITime  notBefore;
    PKITime  notAfter;
} PKIValidity;
#define PKIID_Validity PKIID_SEQUENCE

typedef struct {
    PKIANY  tbsCertificate;
    PKIAlgorithmIdentifier  signatureAlgorithm;
    PKIBIT_STRING  signature;
} PKIXCertificate;
#define PKIID_XCertificate PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIPolicyQualifierInfo *elt[ PKIMAX_policyQualifiers_SEQ_OF ];
} PKIpolicyQualifiers_SEQ_OF;
#define PKIID_policyQualifiers_SEQ_OF PKIID_SEQUENCE_OF

typedef struct {
    long  n;          /* number of elements */
    PKIAttribute *elt[ PKIMAX_Attributes ];
} PKIAttributes;
#define PKIID_Attributes PKIID_SET_OF

typedef struct {
    PKICertPolicyId  policyIdentifier;
    PKIpolicyQualifiers_SEQ_OF *policyQualifiers;
} PKIPolicyInformation;
#define PKIID_PolicyInformation PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIRelativeDistinguishedName *elt[ PKIMAX_RDNSequence ];
} PKIRDNSequence;
#define PKIID_RDNSequence PKIID_SEQUENCE_OF

typedef struct {
    PKICertificateSerialNumber  userCertificate;
    PKITime  revocationDate;
    PKIExtensions *crlEntryExtensions;
} PKIRevokedCertificate;
#define PKIID_RevokedCertificate PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIAttribute *elt[ PKIMAX_SubjectDirectoryAttributes ];
} PKISubjectDirectoryAttributes;
#define PKIID_SubjectDirectoryAttributes PKIID_SEQUENCE_OF

typedef struct {
    PKINoticeReference *noticeRef;
    PKIDisplayText *explicitText;
} PKIUserNotice;
#define PKIID_UserNotice PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIPolicyInformation *elt[ PKIMAX_CertificatePolicies ];
} PKICertificatePolicies;
#define PKIID_CertificatePolicies PKIID_SEQUENCE_OF

typedef PKIRDNSequence PKIDistinguishedName;
#define PKIID_DistinguishedName PKIID_RDNSequence

typedef struct {
    unsigned char CHOICE_field_type;
    void *data;
} PKIName;
#define PKIID_Name PKIID_CHOICE
/* tags for fields defined for Name */
#define PKIrdnSequence_NameFieldTag PKIID_RDNSequence

typedef struct {
    long  n;          /* number of elements */
    PKIRevokedCertificate *elt[ PKIMAX_RevokedCertificates ];
} PKIRevokedCertificates;
#define PKIID_RevokedCertificates PKIID_SEQUENCE_OF

typedef struct {
    PKIVersion  version;
    PKIName  subject;
    PKISubjectPublicKeyInfo  subjectPublicKeyInfo;
    PKIAttributes  attributes;
} PKICertificationRequestInfo;
#define PKIID_CertificationRequestInfo PKIID_SEQUENCE

typedef struct {
    unsigned char CHOICE_field_type;
    void *data;
} PKIGeneralName;
#define PKIID_GeneralName PKIID_CHOICE
/* tags for fields defined for GeneralName */
#define PKIotherName_GeneralNameFieldTag 0x80|0x00
#define PKIrfc822Name_GeneralNameFieldTag 0x80|0x01
#define PKIdNSName_GeneralNameFieldTag 0x80|0x02
#define PKIdirectoryName_GeneralNameFieldTag 0x80|0x20|0x04
#define PKIediPartyName_GeneralNameFieldTag 0x80|0x05
#define PKIuniformResourceIdentifier_GeneralNameFieldTag 0x80|0x06
#define PKIiPAddress_GeneralNameFieldTag 0x80|0x07
#define PKIregisteredID_GeneralNameFieldTag 0x80|0x08

typedef struct {
    PKIVersion *version;
    PKIAlgorithmIdentifier  signature;
    PKIName  issuer;
    PKITime  thisUpdate;
    PKITime *nextUpdate;
    PKIRevokedCertificates *revokedCertificates;
    PKIExtensions *crlExtensions;
} PKITBSCertList;
#define PKIID_TBSCertList PKIID_SEQUENCE

typedef struct {
    PKIVersion *version;
    PKICertificateSerialNumber  serialNumber;
    PKIAlgorithmIdentifier  signature;
    PKIName  issuer;
    PKIValidity  validity;
    PKIName  subject;
    PKISubjectPublicKeyInfo  subjectPublicKeyInfo;
    PKIUniqueIdentifier *issuerUniqueID;
    PKIUniqueIdentifier *subjectUniqueID;
    PKIExtensions *extensions;
} PKITBSCertificate;
#define PKIID_TBSCertificate PKIID_SEQUENCE

typedef struct {
    PKIOBJECT_ID  accessMethod;
    PKIGeneralName  accessLocation;
} PKIAccessDescription;
#define PKIID_AccessDescription PKIID_SEQUENCE

typedef struct {
    PKITBSCertificate  tbsCertificate;
    PKIAlgorithmIdentifier  signatureAlgorithm;
    PKIBIT_STRING  signature;
} PKICertificate;
#define PKIID_Certificate PKIID_SEQUENCE

typedef struct {
    PKITBSCertList  tbsCertList;
    PKIAlgorithmIdentifier  signatureAlgorithm;
    PKIBIT_STRING  signature;
} PKICertificateList;
#define PKIID_CertificateList PKIID_SEQUENCE

typedef struct {
    PKICertificationRequestInfo  certificationRequestInfo;
    PKIAlgorithmIdentifier  signatureAlgorithm;
    PKIBIT_STRING  signature;
} PKICertificationRequest;
#define PKIID_CertificationRequest PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIGeneralName *elt[ PKIMAX_GeneralNames ];
} PKIGeneralNames;
#define PKIID_GeneralNames PKIID_SEQUENCE_OF

typedef struct {
    PKIGeneralName  base;
    PKIBaseDistance *minimum;
    PKIBaseDistance *maximum;
} PKIGeneralSubtree;
#define PKIID_GeneralSubtree PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIAccessDescription *elt[ PKIMAX_AuthorityInfoAccessSyntax ];
} PKIAuthorityInfoAccessSyntax;
#define PKIID_AuthorityInfoAccessSyntax PKIID_SEQUENCE_OF

typedef struct {
    PKIKeyIdentifier *keyIdentifier;
    PKIGeneralNames *authorityCertIssuer;
    PKICertificateSerialNumber *authorityCertSerialNumber;
} PKIAuthorityKeyIdentifier;
#define PKIID_AuthorityKeyIdentifier PKIID_SEQUENCE

typedef PKIGeneralNames PKICertificateIssuer;
#define PKIID_CertificateIssuer PKIID_GeneralNames

typedef struct {
    unsigned char CHOICE_field_type;
    void *data;
} PKIDistributionPointName;
#define PKIID_DistributionPointName PKIID_CHOICE
/* tags for fields defined for DistributionPointName */
#define PKIfullName_DistributionPointNameFieldTag 0x80|0x00
#define PKInameRelativeToCRLIssuer_DistributionPointNameFieldTag 0x80|0x01

typedef struct {
    long  n;          /* number of elements */
    PKIGeneralSubtree *elt[ PKIMAX_GeneralSubtrees ];
} PKIGeneralSubtrees;
#define PKIID_GeneralSubtrees PKIID_SEQUENCE_OF

typedef PKIGeneralNames PKIIssuerAltName;
#define PKIID_IssuerAltName PKIID_GeneralNames

typedef PKIGeneralNames PKISubjectAltName;
#define PKIID_SubjectAltName PKIID_GeneralNames

typedef struct {
    PKIDistributionPointName *distributionPoint;
    PKIReasonFlags *reasons;
    PKIGeneralNames *cRLIssuer;
} PKIDistributionPoint;
#define PKIID_DistributionPoint PKIID_SEQUENCE

typedef struct {
    PKIDistributionPointName *distributionPoint;
    PKIBOOLEAN *onlyContainsUserCerts;
    PKIBOOLEAN *onlyContainsCACerts;
    PKIReasonFlags *onlySomeReasons;
    PKIBOOLEAN *indirectCRL;
} PKIIssuingDistributionPoint;
#define PKIID_IssuingDistributionPoint PKIID_SEQUENCE

typedef struct {
    PKIGeneralSubtrees *permittedSubtrees;
    PKIGeneralSubtrees *excludedSubtrees;
} PKINameConstraints;
#define PKIID_NameConstraints PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIDistributionPoint *elt[ PKIMAX_CRLDistPointsSyntax ];
} PKICRLDistPointsSyntax;
#define PKIID_CRLDistPointsSyntax PKIID_SEQUENCE_OF


#endif /* cert_STRUCT_H_ */

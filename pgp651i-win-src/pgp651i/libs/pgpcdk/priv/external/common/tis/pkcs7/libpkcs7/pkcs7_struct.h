/*******************************************************************
 *
 * This file was generated by TIS/ASN1COMP Ver. 4.0, an ASN.1 compiler.
 * TIS/ASN1COMP is Copyright (c) 1998, TIS Labs at Network Associates, Inc.
 *
 * This file was AUTOMATICALLY GENERATED on Wed Nov 18 16:32:30 1998
 *
 ******************************************************************/

#ifndef pkcs7_STRUCT_H_
#define pkcs7_STRUCT_H_

#include "pkcs7_sizes.h"

#include "cert_asn.h"

/*******************************************************************
 *
 * Structure and type definitions
 *
 ******************************************************************/


typedef PKIOBJECT_ID   PKIContentType;
#define PKIID_ContentType PKIID_OBJECT_ID

typedef PKIOCTET_STRING   PKIData;
#define PKIID_Data PKIID_OCTET_STRING

typedef PKIOCTET_STRING   PKIDigest;
#define PKIID_Digest PKIID_OCTET_STRING

typedef PKIOCTET_STRING   PKIEncryptedContent;
#define PKIID_EncryptedContent PKIID_OCTET_STRING

typedef PKIOCTET_STRING   PKIEncryptedDigest;
#define PKIID_EncryptedDigest PKIID_OCTET_STRING

typedef PKIOCTET_STRING   PKIEncryptedKey;
#define PKIID_EncryptedKey PKIID_OCTET_STRING

typedef PKIBIT_STRING   PKISignature;
#define PKIID_Signature PKIID_BIT_STRING

typedef struct {
    long  n;          /* number of elements */
    PKICertificateList *elt[ PKIMAX_CertificateRevocationLists ];
} PKICertificateRevocationLists;
#define PKIID_CertificateRevocationLists PKIID_SET_OF

typedef PKIAlgorithmIdentifier PKIContentEncryptionAlgorithmIdentifier;
#define PKIID_ContentEncryptionAlgorithmIdentifier PKIID_AlgorithmIdentifier

typedef struct {
    PKIContentType  contentType;
    PKIANY *content;
} PKIContentInfo;
#define PKIID_ContentInfo PKIID_SEQUENCE

typedef PKIAlgorithmIdentifier PKIDigestAlgorithmIdentifier;
#define PKIID_DigestAlgorithmIdentifier PKIID_AlgorithmIdentifier

typedef PKIAlgorithmIdentifier PKIDigestEncryptionAlgorithmIdentifier;
#define PKIID_DigestEncryptionAlgorithmIdentifier PKIID_AlgorithmIdentifier

typedef struct {
    PKIVersion  version;
    PKICertificate  certificate;
    PKIAttributes  attributes;
} PKIExtendedCertificateInfo;
#define PKIID_ExtendedCertificateInfo PKIID_SEQUENCE

typedef struct {
    PKIName  issuer;
    PKICertificateSerialNumber  serialNumber;
} PKIIssuerAndSerialNumber;
#define PKIID_IssuerAndSerialNumber PKIID_SEQUENCE

typedef PKIAlgorithmIdentifier PKIKeyEncryptionAlgorithmIdentifier;
#define PKIID_KeyEncryptionAlgorithmIdentifier PKIID_AlgorithmIdentifier

typedef PKIAlgorithmIdentifier PKISignatureAlgorithmIdentifier;
#define PKIID_SignatureAlgorithmIdentifier PKIID_AlgorithmIdentifier

typedef struct {
    long  n;          /* number of elements */
    PKIDigestAlgorithmIdentifier *elt[ PKIMAX_DigestAlgorithmIdentifiers ];
} PKIDigestAlgorithmIdentifiers;
#define PKIID_DigestAlgorithmIdentifiers PKIID_SET_OF

typedef struct {
    PKIDigestAlgorithmIdentifier  digestAlgorithm;
    PKIDigest  digest;
} PKIDigestInfo;
#define PKIID_DigestInfo PKIID_SEQUENCE

typedef struct {
    PKIContentType  contentType;
    PKIContentEncryptionAlgorithmIdentifier  contentEncryptionAlgorithm;
    PKIEncryptedContent *encryptedContent;
} PKIEncryptedContentInfo;
#define PKIID_EncryptedContentInfo PKIID_SEQUENCE

typedef struct {
    PKIExtendedCertificateInfo  extendedCertificateInfo;
    PKISignatureAlgorithmIdentifier  signatureAlgorithm;
    PKISignature  signature;
} PKIExtendedCertificate;
#define PKIID_ExtendedCertificate PKIID_SEQUENCE

typedef struct {
    PKIVersion  version;
    PKIIssuerAndSerialNumber  issuerAndSerialNumber;
    PKIKeyEncryptionAlgorithmIdentifier  keyEncryptionAlgorithm;
    PKIEncryptedKey  encryptedKey;
} PKIRecipientInfo;
#define PKIID_RecipientInfo PKIID_SEQUENCE

typedef struct {
    PKIVersion  version;
    PKIIssuerAndSerialNumber  issuerAndSerialNumber;
    PKIDigestAlgorithmIdentifier  digestAlgorithm;
    PKIAttributes *authenticatedAttributes;
    PKIDigestEncryptionAlgorithmIdentifier  digestEncryptionAlgorithm;
    PKIEncryptedDigest  encryptedDigest;
    PKIAttributes *unauthenticatedAttributes;
} PKISignerInfo;
#define PKIID_SignerInfo PKIID_SEQUENCE

typedef struct {
    PKIVersion  version;
    PKIEncryptedContentInfo  encryptedContentInfo;
} PKIEncryptedData;
#define PKIID_EncryptedData PKIID_SEQUENCE

typedef struct {
    unsigned char CHOICE_field_type;
    void *data;
} PKIExtendedCertificateOrCertificate;
#define PKIID_ExtendedCertificateOrCertificate PKIID_CHOICE

typedef struct {
    long  n;          /* number of elements */
    PKIRecipientInfo *elt[ PKIMAX_RecipientInfos ];
} PKIRecipientInfos;
#define PKIID_RecipientInfos PKIID_SET_OF

typedef struct {
    long  n;          /* number of elements */
    PKISignerInfo *elt[ PKIMAX_SignerInfos ];
} PKISignerInfos;
#define PKIID_SignerInfos PKIID_SET_OF

typedef struct {
    PKIVersion  version;
    PKIRecipientInfos  recipientInfos;
    PKIEncryptedContentInfo  encryptedContentInfo;
} PKIEnvelopedData;
#define PKIID_EnvelopedData PKIID_SEQUENCE

typedef struct {
    long  n;          /* number of elements */
    PKIExtendedCertificateOrCertificate *elt[ PKIMAX_ExtendedCertificatesAndCertificates ];
} PKIExtendedCertificatesAndCertificates;
#define PKIID_ExtendedCertificatesAndCertificates PKIID_SET_OF

typedef struct {
    PKIVersion  version;
    PKIDigestAlgorithmIdentifiers  digestAlgorithms;
    PKIContentInfo  contentInfo;
    PKIExtendedCertificatesAndCertificates *certificates;
    PKICertificateRevocationLists *crls;
    PKISignerInfos  signerInfos;
} PKISignedData;
#define PKIID_SignedData PKIID_SEQUENCE

#endif /* pkcs7_STRUCT_H_ */
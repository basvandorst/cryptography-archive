/*******************************************************************
 *
 * This file was generated by TIS/ASN1COMP Ver. 4.0, an ASN.1 compiler.
 * TIS/ASN1COMP is Copyright (c) 1998, TIS Labs at Network Associates, Inc.
 *
 * This file was AUTOMATICALLY GENERATED on Fri Jan 15 15:22:21 1999
 *
 ******************************************************************/

#ifndef pkcsreq_STRUCT_H_
#define pkcsreq_STRUCT_H_

#include "pkcsreq_sizes.h"

#include "pkcs7_asn.h"

/*******************************************************************
 *
 * Structure and type definitions
 *
 ******************************************************************/


typedef PKIANY   PKIEndEntityInfo;
#define PKIID_EndEntityInfo PKIID_ANY

typedef struct {
    PKIContentInfo *certs;
    PKIOCTET_STRING  rspInfo;
} PKICertRep;
#define PKIID_CertRep PKIID_SEQUENCE

typedef struct {
    PKIName  issuerName;
    PKIGeneralName *crlName;
    PKIGeneralizedTime *time;
    PKIReasonFlags *reason;
} PKIGetCRL;
#define PKIID_GetCRL PKIID_SEQUENCE

typedef struct {
    PKIName  Issuername;
    PKIINTEGER  serialNumber;
} PKIGetCert;
#define PKIID_GetCert PKIID_SEQUENCE

typedef struct {
    PKIName  issuer;
    PKIName  subject;
} PKIIssuerAndSubject;
#define PKIID_IssuerAndSubject PKIID_SEQUENCE

typedef struct {
    PKIEndEntityInfo  endEntityInfo;
    PKIOCTET_STRING *regInfo;
} PKIPKCSReq;
#define PKIID_PKCSReq PKIID_SEQUENCE

typedef struct {
    PKIIssuerAndSubject  issuerAndSubject;
    PKIOCTET_STRING *reginfo;
} PKIGetCertInitial;
#define PKIID_GetCertInitial PKIID_SEQUENCE

#endif /* pkcsreq_STRUCT_H_ */

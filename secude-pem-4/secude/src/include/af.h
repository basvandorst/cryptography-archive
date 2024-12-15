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

/*-----------------------------------------------------------------------*/
/*  INCLUDE FILE  af.h  (Authentication Framework Interface)             */
/*  Definition of structures and types for Authentication Framework      */
/*-----------------------------------------------------------------------*/

#ifndef _AF_
#define _AF_

/*
 *   secure.h defines:
 *
 *          AlgId               (typedef struct AlgId)
 *          ObjId               (typedef struct ObjId)
 *          OctetString         (typedef struct OctetString)
 *          BitString           (typedef struct BitString)
 *          ENCRYPTED           (typedef struct BitString)
 *          UTCTime             (typedef char)
 *          KeyInfo             (typedef struct KeyInfo) 
 *          Signature           (typedef struct Signature)
 */

#include "If-types.h"           /* from ISODE */

/* Declaration of build- and parse-routines */
#include "SECAF-types.h"
#include "CADB-types.h"

#ifndef _SECURE_
#include "secure.h"
#endif

/*-----------------------------------------------------------------------*/
/*    T y p e d e f ' s   f o r   A F                                    */
/*-----------------------------------------------------------------------*/

typedef struct ToBeSigned                       ToBeSigned;
typedef struct ToBeSigned                       CertificateTBS;
typedef struct Certificate                      Certificate;
typedef struct Certificates                     Certificates;
typedef SET_OF(Certificate)                     SET_OF_Certificate;
typedef SET_OF(ToBeSigned)                      SET_OF_ToBeSigned;
typedef struct AF_PSE_Sel                       AFPSESel;
typedef enum { SIGNATURE, ENCRYPTION }          KeyType;
typedef SET_OF_Certificate                      CrossCertificates;
typedef struct Attribute                        Attribute;
typedef struct OCList                           OCList;
typedef struct Serial                           Serial;
typedef struct CRLEntry                         CRLEntry;
typedef SEQUENCE_OF(CRLEntry)                   SEQUENCE_OF_CRLEntry;
typedef struct CertificatePair                  CertificatePair;
typedef SEQUENCE_OF(CertificatePair)            SEQUENCE_OF_CertificatePair;
typedef SET_OF(CertificatePair)                 SET_OF_CertificatePair;
typedef struct Crl                              Crl;
typedef SET_OF(Crl)                             SET_OF_Crl;
typedef enum { userCertificate, cACertificate } CertificateType;
typedef struct CertificationPath                CertificationPath;
typedef struct CertificatePairs                 CertificatePairs;
typedef struct VerificationResult               VerificationResult;
typedef struct VerificationStep                 VerificationStep;
typedef struct Validity                         Validity;

#define NULLCERTIFICATE                         ((Certificate *) 0)


typedef struct CRLTBS                           CRLTBS;
typedef struct CRL                              CRL;
typedef SET_OF(CRL)                             SET_OF_CRL;
typedef struct CRLWithCertificates              CRLWithCertificates;    
typedef SET_OF(CRLWithCertificates)             SET_OF_CRLWithCertificates;


/*    P S E - O b j e c t s      */

typedef Certificate              SignCert;
typedef Certificate              EncCert;
typedef SET_OF_Certificate       SignCSet;
typedef SET_OF_Certificate       EncCSet;
typedef KeyInfo                  SignSK;
typedef KeyInfo                  DecSKnew;
typedef KeyInfo                  DecSKold;
typedef struct FCPath            FCPath;
typedef struct PKRoot            PKRoot; 
typedef SET_OF_ToBeSigned        PKList;
typedef SET_OF_ToBeSigned        EKList;
typedef SET_OF_ToBeSigned        PCAList;
typedef SET_OF_CertificatePair   CrossCSet;
typedef SET_OF_Crl               CrlSet;
typedef OctetString              SerialNumber;
typedef char 			*QuipuPWD;

#define NULLSERIALNUMBER                         ((SerialNumber *) 0)

/*    DistinguishedName - H a n d l i n g   */

typedef char                                       Name;
typedef struct Attrlist                            AttrList;
typedef SET_OF(DName)                              SET_OF_DName;

typedef struct NAME				   NAME;
typedef SET_OF(NAME)				   SET_OF_NAME;
struct NAME {
	Name  *alias;
        Name  *name;
        DName *dname;
};

#define NULLDNAME ((DName *) 0)
#define NULLRDNAME ((RDName *) 0)


#ifdef X500
#define DBA_AUTH_NONE 0
#define DBA_AUTH_SIMPLE 1
#define DBA_AUTH_STRONG 2
#endif

#ifdef COSINE
typedef struct AuthorisationAttributes AuthorisationAttributes;

typedef char CountryId;
typedef char GroupId;
typedef enum { Normal, Privileged } ClassId;

struct AuthorisationAttributes {
        CountryId  *country;
        GroupId    *group;
        ClassId    class;
};
#endif


/*-----------------------------------------------------------------------*/
/*     SET_OF, Attribute                                                 */
/*-----------------------------------------------------------------------*/

struct Attribute {
        ObjId           type;
        SET_OF(char)   *value;
};


/*-----------------------------------------------------------------------*/
/*     Certificate and Certificates                                      */
/*-----------------------------------------------------------------------*/


struct Validity {
        char *notbefore;
        char *notafter;
};

struct ToBeSigned {
        int             version;
        OctetString    *serialnumber;
        AlgId          *signatureAI;  /* algorithm of issuer's signature */
        DName          *issuer;
        Validity       *valid;
        DName          *subject;
        KeyInfo        *subjectPK;
#ifdef COSINE
        AuthorisationAttributes *authatts;
#endif
};

struct Certificate {
        OctetString    *tbs_DERcode; /* Return-Parameter of e_ToBeSigned */
        ToBeSigned     *tbs;
        Signature      *sig;         /* issuer's signature               */
};

struct FCPath {
        SET_OF_Certificate *liste;
        FCPath         *next_forwardpath;
};

struct Certificates {
        Certificate    *usercertificate;
        FCPath         *forwardpath;
};

struct CertificationPath {
        Certificate        * userCertificate;
        CertificatePairs   * theCACertificates;
};

struct CertificatePairs {
        SEQUENCE_OF_CertificatePair * liste;
        CertificatePairs   * superior;
};

/*-----------------------------------------------------------------------*/
/*     Revoked Certificates, according to PEM                            */
/*-----------------------------------------------------------------------*/

struct CRLEntry {
        OctetString     *serialnumber;
        UTCTime         *revocationDate;
};

struct  CRLTBS {
        AlgId                    *signatureAI;
        DName                    *issuer;
        SEQUENCE_OF_CRLEntry     *revokedCertificates;
        UTCTime                  *lastUpdate;
        UTCTime                  *nextUpdate;
};

struct  CRL {
        OctetString     *tbs_DERcode;  /* Return-Parameter of e_CRLTBS */
        CRLTBS          *tbs;
        Signature       *sig;          /* issuing CA's signature          */
};

struct CRLWithCertificates {
        CRL             * crl;
        Certificates    * certificates;
};


/*-----------------------------------------------------------------------*/
/*     Old Certificates                                                  */
/*-----------------------------------------------------------------------*/

struct OCList {
        OctetString    *serialnumber;
        Certificate    *ccert;
        OCList         *next;
};


/*-----------------------------------------------------------------------*/
/*     Structures used for Verification of Certification Paths           */
/*-----------------------------------------------------------------------*/

struct VerificationResult {
        VerificationStep     ** verifstep;
        int                     trustedKey;
        Boolean                 success;
        Boolean                 textverified;
        Name                  * top_name;
        Name                  * originator;
        Validity              * valid;
        OctetString           * top_serial;
};


struct VerificationStep {
        Certificate           * cert;
        int                     crlcheck;
        UTCTime               * date;
        Validity              * valid;
        int                     supplied;
        Boolean                 policy_CA;
        Boolean                 dn_subordination_violation;
};

extern VerificationResult * verifresult;

#define REVOKED 1
#define NOT_REVOKED 2
#define CRL_NOT_AVAILABLE 3
#define CRL_OUT_OF_DATE 4
#define NOT_REQUESTED 5


/*--------------------------------------------------------------------------*/
/*       CertificatePair, according to X.509                                */
/*--------------------------------------------------------------------------*/

struct CertificatePair {
        Certificate     *forward;
        Certificate     *reverse;
};

/*-----------------------------------------------------------------------*/
/*     P S E  -  O b j e c t s                                           */
/*-----------------------------------------------------------------------*/


/*
 *      Names of PSE Objects (File-Names in the PSE)
 */


#define PSE_MAXOBJ      24           /* # of objects defined within af */

/*
 *      Object Identifiers of PSE Objects (extern: af-init.c)
 */

extern ObjId *SignCert_OID;
extern ObjId *EncCert_OID;
extern ObjId *Cert_OID;
extern ObjId *SignCSet_OID;
extern ObjId *EncCSet_OID;
extern ObjId *CSet_OID;
extern ObjId *SignSK_OID;
extern ObjId *DecSKnew_OID;
extern ObjId *DecSKold_OID;
extern ObjId *SKnew_OID;
extern ObjId *SKold_OID;
extern ObjId *FCPath_OID;
extern ObjId *PKRoot_OID;
extern ObjId *PKList_OID;
extern ObjId *EKList_OID;
extern ObjId *PCAList_OID;
extern ObjId *CrossCSet_OID;
extern ObjId *CrlSet_OID;
extern ObjId *Name_OID;
extern ObjId *SerialNumber_OID;
extern ObjId *EDBKey_OID;
extern ObjId *Random_OID;
extern ObjId *AliasList_OID;
extern ObjId *QuipuPWD_OID;
extern ObjId *DHparam_OID;
extern ObjId *DHprivate_OID;
extern ObjId *DHkey_OID;
extern ObjId *KeyInfo_OID;
extern ObjId *RSA_SK_OID;
extern ObjId *RSA_PK_OID;
extern ObjId *DSA_SK_OID;
extern ObjId *DSA_PK_OID;
extern ObjId *DES_OID;
extern ObjId *DES3_OID;
extern ObjId *Uid_OID;

/*
 *     List of all PSE Objects
 */

struct AF_PSE_Sel {
        char    *app_name;
        char    *pin;
        Boolean  open;
        struct {
            char *name;
            char *pin;
            ObjId *oid;
            Boolean  open;
        }        object[PSE_MAXOBJ];
        int      app_id;
};

extern AFPSESel AF_pse;

/*
 *     External initialization of variable AF_pse:
 *
 *     AFPSESel AF_pse = { 
 *              DEF_PSE, 0, FALSE,
        { SignSK_name, CNULL, &SignSK_oid, FALSE } ,
        { DecSKnew_name, CNULL, &DecSKnew_oid, FALSE } ,
        { DecSKold_name, CNULL, &DecSKold_oid, FALSE } ,
        { SKnew_name, CNULL, &SKnew_oid, FALSE } ,
        { SKold_name, CNULL, &SKold_oid, FALSE } ,
        { SignCert_name, CNULL, &SignCert_oid, FALSE } ,
        { EncCert_name, CNULL, &EncCert_oid, FALSE } ,
        { Cert_name, CNULL, &Cert_oid, FALSE } ,
        { FCPath_name, CNULL, &FCPath_oid, FALSE } ,
        { PKRoot_name, CNULL, &PKRoot_oid, FALSE } ,
        { PKList_name, CNULL, &PKList_oid, FALSE } ,
        { EKList_name, CNULL, &EKList_oid, FALSE } ,
        { PCAList_name, CNULL, &PCAList_oid, FALSE } ,
        { CrossCSet_name, CNULL, &CrossCSet_oid, FALSE } ,
        { CrlSet_name, CNULL, &CrlSet_oid, FALSE } ,
        { SignCSet_name, CNULL, &SignCSet_oid, FALSE } ,
        { EncCSet_name, CNULL, &EncCSet_oid, FALSE } ,
        { CSet_name, CNULL, &CSet_oid, FALSE } ,
        { SerialNumber_name, CNULL, &SerialNumber_oid, FALSE } ,
        { EDBKey_name, CNULL, &EDBKey_oid, FALSE } ,
        { AliasList_name, CNULL, &AliasList_oid, FALSE } ,
        { Random_name, CNULL, &Random_oid, FALSE } ,
        { QuipuPWD_name, CNULL, &QuipuPWD_oid, FALSE }
        { DHparam_name, CNULL, &DHparam_oid, FALSE }
 *              0, 0 
 *     };
 */

/*
 *      Formats of other PSE Objects
 */

/*      PKRoot          */
/*      ======          */

struct PKRoot {
        DName              *ca;
        struct Serial      *newkey;
        struct Serial      *oldkey;
};

struct Serial {           /* Public Key and serial number */
        OctetString   *serial;
        int            version;
        KeyInfo       *key;
        Validity      *valid;
        Signature     *sig;
};



/*      CrlSet          */
/*      ==========      */


struct Crl {
        DName                   *issuer;
        UTCTime                 *nextUpdate;
        SEQUENCE_OF_CRLEntry  *revcerts;
};


/*-----------------------------------------------------------------------*/
/*       DistinguishedName - Handling                                    */
/*-----------------------------------------------------------------------*/

struct Attrlist {
        char            *abbrev;
        char            *keyword;
        ObjId           *objid;
        ObjId           *syntax_oid;
};

/* ObjId addresses */

extern ObjId *countryName;
extern ObjId *orgName;
extern ObjId *orgUnitName;
extern ObjId *commonName;
extern ObjId *surName;
extern ObjId *localityName;
extern ObjId *streetAddress;
extern ObjId *title;
extern ObjId *serialNumber;
extern ObjId *businessCategory;
extern ObjId *description;
extern ObjId *stateOrProvinceName;

extern ObjId *CountryString;
extern ObjId *CaseIgnoreString;
extern ObjId *PrintableString;

extern AttrList attrlist[];

/*
 * External initialization of attrlist[]:
 *

AttrList attrlist[] = {

"C",     "COUNTRY",             &countryName_oid,         &CountryString_oid,
"O",     "ORGANIZATION",        &orgName_oid,             &CaseIgnoreString_oid,
"OU",    "ORGANIZATIONAL UNIT", &orgUnitName_oid,         &CaseIgnoreString_oid,
"CN",    "COMMON NAME",         &commonName_oid,          &CaseIgnoreString_oid,
"S",     "SURNAME",             &surName_oid,             &CaseIgnoreString_oid,
"L",     "LOCALITY",            &localityName_oid,        &CaseIgnoreString_oid,
"ST",    "STREET ADDRESS",      &streetAddress_oid,       &CaseIgnoreString_oid,
"T",     "TITLE",               &title_oid,               &CaseIgnoreString_oid,
"SN",    "SERIAL NUMBER",       &serialNumber_oid,        &PrintableString_oid,
"BC,     "BUSINESS CATEGORY",   &businessCategory_oid,    &CaseIgnoreString_oid,
"D",     "DESCRIPTION",         &description_oid,         &CaseIgnoreString_oid,
"SP",    "STATE OR PROVINCE",   &stateOrProvinceName_oid, &CaseIgnoreString_oid,
0 };

*/


/*-----------------------------------------------------------------------*/
/*       CA Database - Handling  (cadb)                                  */
/*-----------------------------------------------------------------------*/

typedef struct IssuedCertificate        IssuedCertificate;
typedef SET_OF(IssuedCertificate)       SET_OF_IssuedCertificate;
typedef SET_OF(Name)                    SET_OF_Name;


struct IssuedCertificate {
        OctetString *serial;
        UTCTime     *date_of_issue;
};


/*-----------------------------------------------------------------------*/
/*    Aliases                                                            */
/*-----------------------------------------------------------------------*/

typedef enum { useralias, systemalias, notstored } AliasFile;

struct aliaslist {
        struct aliases {
                char          *aname; /* alias names */
                AliasFile  aliasfile; /* SYSTEM or USER Alias File */
                struct aliases *next;
        } *a;
        Name    *dname;               /* distinguished name */
        struct aliaslist *next;
};
typedef struct aliaslist AliasList;
typedef struct aliases Aliases;

/* Aliastypes */

typedef enum { 
        ANYALIAS,
        RFCMAIL, 
        X400MAIL, 
        LOCALNAME
} AliasType;


#ifdef X500
typedef enum { DAP_BIND, DAP_UNBIND, DAP_ENTER, DAP_DELETE, DAP_RETRIEVE} DapOperation;
typedef enum { NOTYPE, USERCERTIFICATE, CACERTIFICATE, CERTIFICATEPAIR, REVLIST, OCLIST} AttributeTYPE;
#endif


#include "af_prototypes.h"

/* Global variables defined in af_init.c  */

extern Boolean  af_verbose;
extern Boolean  af_chk_crl, af_access_directory;
extern Boolean  af_FCPath_is_trusted;
extern Boolean  af_check_Validity;
extern Boolean  af_sign_check_Validity;
extern Boolean  chk_PEM_subordination;
extern Boolean  accept_alias_without_verification;
extern Boolean  af_x500;
extern Boolean  af_strong;
extern Boolean  af_afdb;
extern Boolean  af_COSINE;
extern Boolean  af_use_alias;
extern DName  * directory_user_dname;

#ifdef X500
extern char   * af_dir_dsaname;
extern char   * af_dir_tailor;
extern int      af_dir_authlevel;
extern Boolean  af_dir_dsabound;
#endif


#endif
 

/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#ifndef _tc_h_
#define _tc_h_

#if PGP_MACINTOSH
#include <unix.h>
#else
#include <sys/types.h>
#endif
#include <stdlib.h>

#include "cert_asn.h"
#include "cert_oid.h"

/* ----- X.509 Certificate Version Numbers ----- */

#define TC_X509_VERSION_1    0
#define TC_X509_VERSION_2    1
#define TC_X509_VERSION_3    2

/* true and false */
#define TC_TRUE PKITRUE
#define TC_FALSE PKIFALSE

/* ----- CMS Error Messages ----- */

#define TC_E_INVARGS	-1	/* invalid arguments to function */
#define TC_E_PARSE	-2	/* error parsing block of data */
#define TC_E_NOMEMORY	-3      /* ran out of memory */
#define TC_E_NOISSUER	-4	/* could not find issuer cert */
#define TC_E_INVSIG	-5	/* signature is invalid */
#define TC_E_INVDATE	-6      /* current time is not withing validity date */
#define TC_E_EXTENSION	-7	/* error while handling extension */
#define TC_E_FILE	-8	/* error reading/writing to a file */
#define TC_E_CRITICAL	-9	/* unhandled critical extension */
#define TC_E_DNAMEPARSE -10	/* error parsing distinguished name string */
#define TC_E_NOTFOUND	-11	/* certificate not found */
#define TC_E_INVSUBJ	-12	/* alt-subject not critical, and no subject */
#define TC_E_INVISSU	-13	/* alt-issuer not critical, and no issuer */
#define TC_E_CERTREVOKED -14	/* certificate is revoked by issuer */
#define TC_E_NOCRL	-15	/* issuer CRL is not present */
#define TC_E_PQG	-16	/* unable to find PQG values for key */
#define TC_E_NOSERIAL   -17	/* could not find serial number in cert */
#define TC_E_SIGNFAIL	-18	/* could not sign the provided data */
#define TC_E_NOTASCII  -19	/* processed Name data is not ascii */
#define TC_E_INVDATECRL -20     /* CRL's nextDate past "now" */
#define TC_E_OTHER	-21     /* other error we can't classify */
#define TC_E_EXTNOTSUPPORTED -22 /* extension not currently supported */
#define TC_E_NAMETOOLONG -23    /* too many components in string dname */
#define TC_E_BADNUMERICSTRING -24 /* disallowed chars in string */
#define TC_E_BADPRINTABLESTRING -25 /* disallowed chars in string */
#define TC_E_BADDNAMESTRINGTYPE -26 /* string type not allowed in AVA */
#define TC_E_INVALIDPERIOD -27 /* notBefore and notAfter must not both be
								  missing in privateKeyUsagePeriod */
#define TC_E_COPYNOTSUPPORTED -28 /* GeneralName contains subtype not
									 supported by CMS */
#define TC_E_COMPARENOTSUPPORTED -29 /* GeneralName conaints subtype not
										supported by CMS */
#define TC_E_INVALIDCRL -30 /* bad crl */
#define TC_E_NOTV2CRL -31 /* CRL is not V2 */
#define TC_E_MISSINGNEXTUPDATE -32 /* CRL is missing nextUpdate field */
#define TC_E_MISSINGAUTHKEYIDEXT -33 /* crl is missing AuthorityKeyIdentifier
										extension */
#define TC_E_MISSINGCRLNUMBEREXT -34 /* crl is missing cRLNumber extension */
#define TC_E_WRONGCRL -35 /* crl issuer and cert issuer don't match */
#define TC_E_INVALIDURI -36 /* GeneralName containts invalue URI value */
#define TC_E_INVALIDIPCONSTRAINT -36 /* malformed iPAddress constraint */
#define TC_E_WRONGNAMETYPE -37 /* GeneralName type does not match
								  GeneralSubtree base type */
#define TC_E_NAMETYPEUNSUPPORTED -38 /* not supported */
#define TC_E_CONSTRAINTFAIL -39 /* name constraints were not satisfied */
#define TC_E_MISSINGPOLICY -40 /* certificate is missing required policy
								  extension */
#define TC_E_INVALIDPOLICY -41 /* policy doesn't match acceptable use */
#define TC_E_NOTCA -42 /* certificate is not a CA */
#define TC_E_NOTSIGNINGKEY -43 /* CA key does not have keyCertSign bit set
								  in its keyUsage extension */
#define TC_E_PATHCONSTRAINT -44 /* certificate path length is longer than
								   constraint set by CAs */
#define TC_E_POLICYMAPPED -45 /* policy mapping illegal due to CA constraint */
#define TC_E_POLICYMISMATCH -46 /* initial policies and acceptable policies
								   do not overlap */

#define tc_strerror(n) TC_errlist[-(n+1)]

/* error definitions for the PKIX compliance checking routines */
#define TC_E_SigAlgSignatureMismatch (-5000)
#define TC_E_VersionShouldBe3        (-5001)
#define TC_E_VersionShouldBe2        (-5002)
#define TC_E_VersionShouldBeNULL     (-5003)
#define TC_E_EmptyIssuerName         (-5004)
#define TC_E_ValidityNotUTCTime      (-5005)
#define TC_E_CantHandleCurrentTimeValue (-5006)
#define TC_E_IssuerSubjectNotSameInRootCert (-5007)
#define TC_E_RSAParametersMissing    (-5008)
#define TC_E_RSAParametersNotASN1NULL (-5009)
#define TC_E_DSAParametersArePresent (-5010)
#define TC_E_IssuerUniqueIDPresent   (-5011)
#define TC_E_SubjectUniqueIDPresent  (-5012)
#define TC_E_DuplicateExtensionEntry (-5013)
#define TC_E_KeyUsageNotMarkedCritical (-5014)
#define TC_E_DontUsePrivateKeyUsage  (-5015)
#define TC_E_PolicyMappingMarkedCritical (-5016)
#define TC_E_IssuerAltNameMarkedCritical (-5017)
#define TC_E_SubjectDirAttrMarkedCritical (-5018)
#define TC_E_BasicConstraintsMarkedCritical (-5019)
#define TC_E_NonCACertWithNameConstraints (-5020)
#define TC_E_NameConstraintsNotMarkedCritical (-5021)
#define TC_E_PolicyConstraintsMarkedCritical (-5022)
#define TC_E_UnknownExtensionMarkedCritical (-5023)
#define TC_E_EECertWithBasicConstraints (-5024)
#define TC_E_SubjectKeyIDMarkedCritical (-5025)
#define TC_E_AuthKeyIDMarkedCritical (-5026)


/***** Memory Mgmt callbacks *****/

/* we are going to use the same memory mgmt. prototypes and structure
   as used for the compiler.  See cert_util.h for the details. */

typedef PKIMemoryMgr TC_MemoryMgr;  

typedef PKIMemoryMgrAllocationProc TC_MemoryMgrAllocationProc;

typedef PKIMemoryMgrReallocationProc TC_MemoryMgrReallocationProc;

typedef PKIMemoryMgrDeallocationProc TC_MemoryMgrDeallocationProc;

#define TC_Alloc(mgr, size) (*(mgr)->allocProc)(mgr, size)
#define TC_Realloc(mgr, ptr, newsize) \
              (*(mgr)->reallocProc)(mgr, ptr, newsize)
#define TC_Free(mgr, ptr) (*(mgr)->deallocProc)(mgr, ptr)


/* rename the compiler generated stuff to isolate its name
   from user */
typedef PKIXCertificate TC_XCertificate;
typedef PKITBSCertificate TC_TBSCertificate;
typedef PKIExtension TC_Extension;
typedef PKIExtensions TC_ExtensionList;
typedef PKIAlgorithmIdentifier TC_AlgorithmIdentifier;
typedef PKIBIT_STRING TC_BIT_STRING;
typedef PKISubjectPublicKeyInfo TC_SubjectPublicKeyInfo;
typedef PKIName TC_Name;
typedef PKIINTEGER TC_INTEGER;
typedef PKIAttributes TC_Attributes;
typedef PKICertificate TC_Certificate;
typedef PKICertificationRequest TC_CertificationRequest;
typedef PKICertificateList TC_CertificateList;
typedef PKIUTCTime TC_UTCTime;
typedef PKIGeneralizedTime TC_GeneralizedTime;
typedef PKITime TC_Time;
typedef PKIValidity TC_Validity;
typedef PKICertificateSerialNumber TC_SerialNumber;
typedef PKIRevokedCertificate TC_RevokedCertificate;
typedef PKICertificatePolicies TC_CertificatePolicies;
typedef PKIOCTET_STRING TC_OCTET_STRING;

enum
{
  TC_EXTRACT_SUBJECT,
  TC_EXTRACT_ISSUER,
  TC_EXTRACT_CRLISSUER,
  TC_EXTRACT_REQ_NAME
};

#define TC_NAME_LEN 3

#define TC_F_ERRNOCRL	0x01	/* generate error if issuer CRL is missing */
#define TC_F_NOERRDATE	0x02	/* don't generate error on invalid date */
#define TC_F_ENFCRLDATE 0x04    /* generate error if CRL nextUpdate past */
#define TC_F_PKIX		0x08	/* enable strict PKIX compliance checks */

typedef struct list_t
{
  void *data;
  struct list_t *next;
} TC_LIST;

typedef struct cert_t
{
  TC_XCertificate	*cert;
  TC_TBSCertificate	*tbsCertificate;
  TC_Certificate        *asnCertificate;
} TC_CERT;


typedef struct tc_ava_entry_t TC_AVA_ENTRY;
typedef struct tc_context_t TC_CONTEXT;

/*****
 *
 * prototype for user supplied handler functions
 *
 * ext - the extension to check
 * cert - void pointer to the cert or CRL containing the extension
 * ctx - a CMS context
 *
 *****/
typedef int tc_handler_t (TC_Extension *ext,
			  void *cert,
			  TC_CONTEXT *ctx);

/*****
*
* prototype for user supplied callback function to verify a signature
*
* block		- data to verify
* blockLen	- length of data to verify
* sigalg	- the signature algorithm (hash + enc), from the signed
*                 entity
* algLen        - length of alg
* param         - DER block of parameters from issuer cert, if any
* paramLen      - length of param
* signature	- the signature from certificate/request
* sigLen        - length of signature
* pubkey        - the public key of the signer
* pubkeyLen     - length of pubkey
* signingCert   - the signer's certificate, if available
* verData       - verification data registered into context
* ctx           - the CMS context value provided to the calling CMS routine
*
*****/
typedef int tc_verify_t (unsigned char *block,
			 size_t blockLen,
			 unsigned char *sigalg,
			 size_t algLen,
			 unsigned char *params,
			 size_t paramLen,
			 unsigned char *signature,
			 size_t sigLen,
			 unsigned char *pubkey,
			 size_t pubkeyLen,
			 TC_CERT *issuer,
			 void *verData,
			 TC_CONTEXT *ctx);

/*****
*
* prototype for signing call-back function
*
* sig		- the returned signature
* sigLen	- length of sig
* block		- the block of data (eg., cert) to sign
* blockLen	- length of block
* alg           - the signature algorithm (hash + enc), supplied by the user
* algLen        - length of alg
* sigData	- signing data registered into context
* ctx           - the CMS context value provided to the calling CMS routine
*
*****/
typedef int tc_sign_t	(unsigned char **sig,
			 size_t *sigLen,
			 unsigned char *block,
			 size_t blockLen,
			 const unsigned char *alg,
			 size_t algLen,
			 void *sigData,
			 TC_CONTEXT *ctx);

/* TODO: hide this structure from the user...*/
/*****
*
* context structure
*
* memMgr        - the memory mgmt callbacks provided by the user
* certasnctx    - the context for the ASN.1 generated code
* roots		- list of parent certificates 
* handlers	- list of handlers for X.509v3 extensions
* crl		- list of certificate revocations lists
* avaList       - a list of ava OID and default ASN type definitions
* verify	- call-back function to perform a sig. check on cert.
* sign		- call-back function to sign a cert., request, or crl
* sigfuncdata	- a pointer to data used by sign function
* verfuncdata	- a pointer to data used by verify function
*
*****/
struct tc_context_t
{
  TC_MemoryMgr *memMgr;
  PKICONTEXT *certasnctx;
  TC_LIST *roots;
  TC_LIST *handlers;
  TC_LIST *crl;
  TC_AVA_ENTRY *avaList;
  tc_verify_t *verify;
  tc_sign_t *sign;
  void *sigfuncdata;
  void *verfuncdata;
};


/************************************
 * Library Functions
 ************************************/

/* 
-----
context
----- 
*/
int tc_init_context (TC_CONTEXT **context,
		     TC_MemoryMgr *memMgr,
		     tc_sign_t *signFunc, 
		     void *signFuncData, 
		     tc_verify_t *verifyFunc,
		     void *verifyFuncData);

int tc_free_context (TC_CONTEXT *);

int tc_change_ava_entry(
        unsigned char *oid,
        size_t oidlen,
        int newASNType,
        TC_CONTEXT *context);

int tc_add_avatype(
        unsigned char *oid,
        size_t oidlen,
        const char *printableAttrName,
        int asnType,
        TC_CONTEXT *context);

/* 
-----
extensions
----- 
*/

#ifndef boolean
#define boolean unsigned int
#endif

int tc_setextval(TC_ExtensionList *ext, 
	     const unsigned char *oid, 
	     size_t oidlen,
	     int criticality,
	     const void *value,
	     TC_CONTEXT *);

int tc_getextval(void **value,
		 boolean *criticality,
		 const unsigned char *oid, 
		 size_t oidlen,
		 const TC_ExtensionList *extlist,
		 TC_CONTEXT *);

int tc_create_extlist(TC_ExtensionList **, TC_CONTEXT *);

void tc_free_extlist(TC_ExtensionList *, TC_CONTEXT *);

/* allow the user to specify callback functions for specific extensions */
int tc_register_handler (const unsigned char *,	/* OID of extension */
			 size_t,		/* len of OID */
			 tc_handler_t *,	/* function to handle data */
			 TC_CONTEXT *);

int tc_add_extension (TC_ExtensionList *exts, 
		const unsigned char *oid, 
		size_t oidlen,
		int critical, 
		unsigned char *der, 
		size_t derlen,
		TC_CONTEXT *);

int tc_find_extension (TC_Extension **ext,
		const TC_ExtensionList *extList, /* array of exts from cert/crl */
		const unsigned char *,	/* type to look for */
		size_t,
		TC_CONTEXT *);		/* len of type */

int tc_certExtensionList(
    TC_ExtensionList **extList,
    const TC_CERT *cert,
    TC_CONTEXT *);

int tc_crlExtensionList(
    TC_ExtensionList **extList,
    const TC_CertificateList *crl,
    TC_CONTEXT *);

int tc_crlEntryExtList(
    TC_ExtensionList **extList,
    const TC_CertificateList *crl,
    TC_SerialNumber *serialNumber,
    TC_CONTEXT *);

/* ----- Basic Constraints defns for application program usage ----- */

typedef struct tc_basic_constraint_
{
  boolean cA;
  int pathLength;
} TC_BASIC_CONSTRAINT;

#define TC_PATH_LENGTH_UNDEFINED -1

/* ----- Key Usage definitions for application program usage ----- */

#define TC_DIGITAL_SIGNATURE   PKIKeyUsage_digitalSignature
#define TC_NON_REPUDIATION     PKIKeyUsage_nonRepudiation
#define TC_KEY_ENCIPHERMENT    PKIKeyUsage_keyEncipherment
#define TC_DATA_ENCIPHERMENT   PKIKeyUsage_dataEncipherment
#define TC_KEY_AGREEMENT       PKIKeyUsage_keyAgreement
#define TC_KEY_CERT_SIGN       PKIKeyUsage_keyCertSign
#define TC_CRL_SIGN            PKIKeyUsage_cRLSign
#define TC_ENCIPHER_ONLY       PKIKeyUsage_encipherOnly
#define TC_DECIPHER_ONLY       PKIKeyUsage_decipherOnly

/* ----- Subject & Issuer Alternative Name  ----- */

typedef struct _gen_name_ {
    void   *name;    /* actual data depends on values used for nameType */
    size_t nameLen;
    int    nameType; /* one of below */
} TC_GEN_NAME_T;

#define TC_rfc822Name    1
#define TC_dNSName       2
#define TC_directoryName 3
#define TC_uniformResourceIdentifier 4
#define TC_iPAddress     5
#define TC_registeredID  6

typedef struct _gen_name_list_ {
    TC_GEN_NAME_T *names[PKIMAX_GeneralNames];   /* list of names */
    int           numberOfNames;
} TC_GEN_NAMES_LIST_T;

int tc_free_gennamelist(TC_GEN_NAMES_LIST_T *, TC_CONTEXT *);
void tc_free_genname(TC_GEN_NAME_T *, TC_CONTEXT *);

/* ----- Authority Key Identifier definitions for application
         program usage ----- */
typedef struct tc_authority_key_identifier_t
{
    unsigned char	    *keyIdentifier;
    size_t		     keyIdentifier_len;
    TC_GEN_NAMES_LIST_T	    *authorityCertIssuer;
    unsigned char	    *authorityCertSerialNumber;
    size_t		     authorityCertSerialNumber_len;
} TC_AUTHORITY_KEY_IDENTIFIER_T;

typedef TC_OCTET_STRING TC_SUBJECT_KEY_IDENTIFIER_T;

/* ----- PrivateKeyUsagePeriod ----- */

typedef struct tc_private_key_usage_period_t
{
	time_t notBefore;
	time_t notAfter;
} TC_PRIVATE_KEY_USAGE_PERIOD_T;

/* ----- CertificatePolicies ----- */

typedef enum
{
	TC_POLICY_QUALIFIER_ID_CPS,
	TC_POLICY_QUALIFIER_ID_UNOTICE
} TC_POLICY_QUALIFIER_ID_T;

typedef struct tc_notice_reference_t
{
	char *organization;
	int numNoticeNumbers;
	int *noticeNumber[PKIMAX_noticeNumbers_SEQ_OF];
} TC_NOTICE_REFERENCE_T;

typedef struct tc_user_notice_t
{
	TC_NOTICE_REFERENCE_T *noticeRef; /* optional */
	unsigned char *explicitText; /* optional */
} TC_USER_NOTICE_T;

typedef struct tc_policy_qualifier_info_t
{
	TC_POLICY_QUALIFIER_ID_T type;
	/* if type==TC_POLICY_QUALIFIER_ID_CPS, then qualifier points to a "char *"
	   string containing the URI of the policy.
	   if type==TC_POLICY_QUALIFIER_ID_UNOTICE, then qualifier points to a
	   "TC_USER_NOTICE_T *" as described above. */
	void *qualifier;
} TC_POLICY_QUALIFIER_INFO_T;

typedef struct tc_policy_information_t
{
	unsigned char *policyOid;
	size_t policyOidLen;
	int numQualifiers;
	TC_POLICY_QUALIFIER_INFO_T *qualifier[PKIMAX_policyQualifiers_SEQ_OF];
} TC_POLICY_INFORMATION_T;

typedef struct tc_certificate_policies_t
{
	int numPolicies;
	TC_POLICY_INFORMATION_T *policy[PKIMAX_CertificatePolicies];
} TC_CERTIFICATE_POLICIES_T;

/* ----- PolicyMappings ----- */

typedef struct tc_policy_mapping_t
{
	unsigned char *issuerDomainPolicyOid;
	size_t issuerDomainPolicyOidLen;
	unsigned char *subjectDomainPolicyOid;
	size_t subjectDomainPolicyOidLen;
} TC_POLICY_MAPPING_T;

typedef struct tc_policy_mappings_t
{
	int numPolicyMappings;
	TC_POLICY_MAPPING_T *policyMapping[PKIMAX_PolicyMappings];
} TC_POLICY_MAPPINGS_T;

/* ----- SubjectDirectoryAttributes ----- */

typedef struct tc_attribute_t
{
	unsigned char *oid;
	size_t oidsize;

	int numValues;
	unsigned char *val[PKIMAX_values_SET_OF];
	size_t valsize[PKIMAX_values_SET_OF];
} TC_ATTRIBUTE_T;

typedef struct tc_subject_directory_attributes_t
{
	int numAttributes;
	TC_ATTRIBUTE_T *attribute[PKIMAX_SubjectDirectoryAttributes];
} TC_SUBJECT_DIRECTORY_ATTRIBUTES_T;

/* ----- NameConstraints ----- */

typedef struct tc_general_subtress_t
{
	int numSubtrees;
	TC_GEN_NAME_T *base[PKIMAX_GeneralSubtrees];

	/* RFC2459 sec 4.2.1.11 specifies that the following fields should not
	   be used for certificates in the PKIX profile.  As such, information in
	   them is ignored when creating certificate extensions. */
	int minimum[PKIMAX_GeneralSubtrees];
	int maximum[PKIMAX_GeneralSubtrees];
} TC_GENERAL_SUBTREES_T;

typedef struct tc_name_constraints_t
{
	TC_GENERAL_SUBTREES_T *permit;
	TC_GENERAL_SUBTREES_T *deny;
} TC_NAME_CONSTRAINTS_T;
 
void tc_FreeNameConstraints (TC_CONTEXT *, TC_NAME_CONSTRAINTS_T *);

/* ----- PolicyConstraints ----- */

typedef struct tc_policy_constraints_t
{
	/* One of `require' and `inhibit' is required to be set (RFC2459
	   sect. 4.2.1.12).

	   Since 0 is a valid value, a value of -1 is used to indicate the field
	   is not set. */

	int require; /* optional */
	int inhibit; /* optional */
} TC_POLICY_CONSTRAINTS_T;

/* ----- ExtKeyUsageSyntax ----- */

typedef enum
{
	TC_KEY_PURPOSE_ID_UNKNOWN,

	/*these are the currently defined key purpose OIDs defined in RFC2459*/
	TC_KEY_PURPOSE_ID_SERVER_AUTH,
	TC_KEY_PURPOSE_ID_CLIENT_AUTH,
	TC_KEY_PURPOSE_ID_CODE_SIGNING,
	TC_KEY_PURPOSE_ID_EMAIL_PROTECTION,
	TC_KEY_PURPOSE_ID_TIME_STAMPING
} TC_KEY_PURPOSE_ID_T;

typedef struct tc_ext_key_usage_syntax_t
{
	int numPurposes;
	TC_KEY_PURPOSE_ID_T purpose[PKIMAX_ExtKeyUsageSyntax];
} TC_EXT_KEY_USAGE_SYNTAX_T;

/* ----- cRLDistributionPoints ----- */

#define TC_REASON_UNUSED PKIReasonFlags_unused
#define TC_REASON_KEY_COMPROMISE PKIReasonFlags_keyCompromise
#define TC_REASON_CA_COMPROMISE PKIReasonFlags_cACompromise
#define TC_REASON_AFFILIATION_CHANGED PKIReasonFlags_affiliationChanged
#define TC_REASON_SUPERSEDED PKIReasonFlags_superseded
#define TC_REASON_CESSATION_OF_OPERATION PKIReasonFlags_cessationOfOperation
#define TC_REASON_CERTIFICATE_HOLD PKIReasonFlags_certificateHold

typedef enum
{
	TC_DISTRIBUTION_POINT_NAME_TYPE_GEN_NAMES,
	TC_DISTRIBUTION_POINT_NAME_TYPE_RDN
} TC_DISTRIBUTION_POINT_NAME_TYPE_T;

typedef struct
{
	TC_DISTRIBUTION_POINT_NAME_TYPE_T type;
	void *name;/* either TC_GEN_NAMES_LIST_T or char*,depending
				 on `type'*/
} TC_DISTRIBUTION_POINT_NAME_T;

typedef struct tc_distribution_point_t
{
	TC_DISTRIBUTION_POINT_NAME_T *name;/*optional*/
	int reasonFlags; /*optional*/
	TC_GEN_NAMES_LIST_T *issuer;/*optional*/
} TC_DISTRIBUTION_POINT_T;

typedef struct tc_crl_dist_points_t
{
	int numDistPoints;
	TC_DISTRIBUTION_POINT_T *distPoint[PKIMAX_CRLDistPointsSyntax];
} TC_CRL_DISTRIBUTION_POINTS_T;

void tc_FreeDistributionPointName(TC_DISTRIBUTION_POINT_NAME_T *,
								  TC_CONTEXT *);

void tc_FreeCRLDistributionPoints (TC_CRL_DISTRIBUTION_POINTS_T *,
								   TC_CONTEXT *);

/* ----- AuthorityInfoAccess ----- */

typedef struct tc_access_description_t
{
	/* OID describing the access type */
	unsigned char *method;
	size_t methodsize;

	/* URI specifying the description of the access method */
	TC_GEN_NAME_T *location;
} TC_ACCESS_DESCRIPTION_T;

typedef struct tc_authority_info_access_t
{
	int numDescrips;
	TC_ACCESS_DESCRIPTION_T *descrip[PKIMAX_AuthorityInfoAccessSyntax];
} TC_AUTHORITY_INFO_ACCESS_T;

void tc_FreeAuthorityInfoAccess (TC_CONTEXT *ctx,
								 TC_AUTHORITY_INFO_ACCESS_T *info);

/* -----
   CRL Extensions
   ----- */

/* ----- IssuingDistributionPoint ----- */

typedef struct
{
	TC_DISTRIBUTION_POINT_NAME_T *name;/*optional*/
	boolean onlyContainsUserCerts;/*default false*/
	boolean onlyContainsCACerts;/*default false*/
	int onlySomeReasons; /*optional,bitmask of ReasonFlags as described above*/
	boolean indirectCRL;/*default false*/
} TC_ISSUING_DISTRIBUTION_POINT_T;

void tc_FreeIssuingDistributionPoint (TC_ISSUING_DISTRIBUTION_POINT_T *,
									  TC_CONTEXT *);

/* -----
   CRL Entry Extensions
   ----- */

/* ----- cRLReason ----- */

typedef enum
{
	TC_CRL_REASON_UNSPECIFIED,
	TC_CRL_REASON_KEY_COMPROMISE,
	TC_CRL_REASON_CA_COMPROMISE,
	TC_CRL_REASON_AFFLICATION_CHANGED,
	TC_CRL_REASON_SUPERSEDED,
	TC_CRL_REASON_CESSATION_OF_OPERATION,
	TC_CRL_REASON_CERTIFICATE_HOLD,
	TC_CRL_REASON_REMOVE_FROM_CRL
} TC_CRL_REASON_T;

/* ----- holdInstructionCode ----- */

typedef enum
{
	TC_HOLD_INSTRUCTION_NONE,
	TC_HOLD_INSTRUCTION_CALL_ISSUER,
	TC_HOLD_INSTRUCTION_REJECT
} TC_HOLD_INSTRUCTION_T;

/* 
-----
certificates
----- 
*/
int tc_unpack_cert (TC_CERT **cert,
		    unsigned char *,	/* DER encoded cert */
		    const size_t,
		    TC_CONTEXT *ctx);	

int tc_pack_cert (unsigned char **, 
		size_t *, 
		TC_CERT *,
		  TC_CONTEXT *ctx);

int tc_read_cert(TC_CERT **cert, 
		const char *fname,
		 TC_CONTEXT *ctx);

int tc_write_cert(TC_CERT *cert, 
		  const char *fname,
		  TC_CONTEXT *ctx);

int tc_create_cert (TC_CERT **cert,
		    int version,
		    unsigned char *serial,
		    size_t serialLen,
		    const unsigned char *sigoid,
		    size_t sigoidlen,
		    const unsigned char *sigparm,
		    size_t sigparmlen,
		    TC_Name *issuerName,
		    time_t notBefore,
		    time_t notAfter,
		    TC_Name *subjectName,
		    TC_ExtensionList *extensions,
		    const unsigned char *keyoid,
		    size_t keyoidlen,
		    const unsigned char *pubkey,
		    size_t pubkeylen,
		    const unsigned char *keyparm,
		    size_t keyparmlen,
		    TC_CONTEXT *ctx);

void tc_free_cert (TC_CERT *, TC_CONTEXT *);

int tc_validate_cert (TC_CERT *,		/* certificate to verify */
		      int flags,
		      TC_CONTEXT *ctx);

int tc_validate_path (TC_CERT *eecert,
					  TC_CertificatePolicies *initialPolicies,
					  time_t when,
					  int flags,
					  TC_CONTEXT *ctx);

int tc_validate_one_cert(TC_CERT *cert,
			int flags, 
			TC_CONTEXT *ctx);

/* import DER certificate into context */
int tc_import_cert (unsigned char *,	/* DER encoded cert */
		    size_t,		/* length of cert */
		    TC_CONTEXT *);

/* import Certificate struct into context */
int tc_add_cert (TC_CERT *cert,
		TC_CONTEXT *ctx);

int tc_delete_cert (TC_Name *issuer, 
		TC_SerialNumber *serial, 
		TC_CONTEXT *ctx);

int tc_find_cert (TC_CERT **cert, 
		TC_Name *issuer, 
		TC_CONTEXT *ctx);

int tc_find_issuer(TC_CERT **issuer, 
		TC_CERT *cert, 
		TC_CONTEXT *ctx);


/* 
-----
certificate requests
----- 
*/

int tc_create_attrlist(TC_Attributes **list,
		       TC_CONTEXT *ctx);

int tc_add_attribute(TC_Attributes *list,
		     unsigned char *oid,
		     size_t oidLen,
		     unsigned char *attrDER,
		     size_t attrLen,
		     TC_CONTEXT *ctx);

int tc_set_signing_time(TC_Attributes *list,
			TC_CONTEXT *ctx);

int tc_find_attribute(unsigned char **attrDER,
		      size_t *attrLen,
		      TC_CertificationRequest *cert,
		      unsigned char *oid,
		      size_t oidLen,
		      TC_CONTEXT *ctx);

void tc_free_attrlist(TC_Attributes *list,
		     TC_CONTEXT *ctx);


int tc_read_request(TC_CertificationRequest **request, 
		const char *fname,
		    TC_CONTEXT *ctx);

int tc_write_request(TC_CertificationRequest *request, 
		const char *fname,
		     TC_CONTEXT *ctx);

int tc_unpack_request (TC_CertificationRequest **request,
                       unsigned char *, /* DER encoded certrequest */
		       size_t,
		       TC_CONTEXT *ctx);	

int tc_pack_request (unsigned char **ptr, 
		     size_t *ptrlen,
                     TC_CertificationRequest *cert,
		     TC_CONTEXT *ctx);

int tc_create_request (TC_CertificationRequest **cert,
                       int version,
                       const unsigned char *sigoid,
                       size_t sigoidlen,
                       const unsigned char *sigparm,
                       size_t sigparmlen,
                       TC_Name *subject,
                       const unsigned char *keyoid,
                       size_t keyoidlen,
                       const unsigned char *pubkey,
                       size_t pubkeylen,
		       const unsigned char *keyparm,
                       size_t keyparmlen,
		       TC_Attributes *list,
                       TC_CONTEXT *ctx) ;

void tc_free_request(TC_CertificationRequest *,
		     TC_CONTEXT *ctx);

int tc_validate_request (TC_CertificationRequest *, /* certification request */
			 TC_CONTEXT *ctx);


/* 
-----
certificate revocation lists
----- 
*/
int tc_unpack_crl (TC_CertificateList **crl, 
		unsigned char *data, 
		size_t dataLen,
		   TC_CONTEXT *ctx);

int tc_pack_crl(unsigned char **ptr, 
		size_t *ptrlen, 
                TC_CertificateList *crl,
		TC_CONTEXT *ctx);

int tc_read_crl(TC_CertificateList **crl, 
		const char *fname,
		TC_CONTEXT *ctx);

int tc_write_crl(TC_CertificateList *crl, 
		const char *fname,
		 TC_CONTEXT *ctx);

int tc_create_crl (TC_CertificateList **crl,
                   const unsigned char *alg,    /* alg to sign crl */
                   size_t algLen,       /* len of `alg' */
                   const unsigned char *algParm,
                   size_t algParmLen,
                   TC_Name *issuer,  /* crl issuer */
                   time_t nextUpdate,   /* next crl update expected */
		   TC_ExtensionList *extensions, /* CRL ext's */
                   TC_CONTEXT *ctx);

void tc_free_crl(TC_CertificateList *crl, TC_CONTEXT *);

int tc_sign_crl (TC_CertificateList *crl,
		 TC_CONTEXT *ctx);

int tc_add_to_crl (TC_CertificateList *crl,
		   unsigned char *serial,
		   size_t serialLen,
		   time_t revocationDate,
		   TC_ExtensionList *entryExts,  /* CRL entry exts */
		   TC_CONTEXT *ctx);

int tc_import_crl (unsigned char *crlData, 
                   size_t crlDataLen,
		   TC_CONTEXT *ctx,
		   int flags);

int tc_add_crl (TC_CertificateList *crl, 
		TC_CONTEXT *ctx,
		int flags);

int tc_delete_crl(TC_Name *issuer, 
		TC_CONTEXT *ctx);

int tc_reset_crl_times(
	       TC_CertificateList *crl, 
	       int nextUpdateDays,
	       TC_CONTEXT *ctx);

/* 
-----
Dnames
----- 
*/

int tc_create_dname(TC_Name **dname, TC_CONTEXT *ctx);

int tc_free_dname(TC_Name *dname, TC_CONTEXT *ctx);

int tc_addAVAto_dname(
        TC_Name *dname,
        unsigned char *oid,
        size_t oidlen,
        unsigned char *der,
        size_t derlen,
	TC_CONTEXT *ctx);

int tc_make_dname_fromstring (
        TC_Name *dname, 
        const char *name,
        TC_CONTEXT *context);

int tc_get_dname(
    TC_Name **dname,
    void *cert,
    int type,
    TC_CONTEXT *context);

int tc_extractdname_fromcert (
	char **name, 
	void *cert, 
        int whichName,
        TC_CONTEXT *context);

int tc_extract_dname_string (
	char **stringname, 
	TC_Name *dname, 
        TC_CONTEXT *context);

int tc_compare_dname (TC_Name *, TC_Name *, TC_CONTEXT *);

int tc_is_self_signed(TC_CERT *cert, TC_CONTEXT *);

int tc_get_attributeValue(
    char **value,
    TC_Name *dname,
    char *attributeType,
    TC_CONTEXT *context);


/* 
-----
PKIX compliance checks
----- 
*/

enum TC_CertType_ {
    TC_RootCertAuthority,
    TC_CertificateAuthority,
    TC_EndEntity
};
typedef enum TC_CertType_ TC_CertType;

int tc_CertCheckPKIXCompliance(
    const TC_CERT     *cert,
    TC_CertType certType,
    int              **errorList,
    int              *numErrors,
    TC_CONTEXT  *ctx);

enum TC_CRLType_ {
    TC_FullCRL,
    TC_DeltaCRL
};
typedef enum TC_CRLType_ TC_CRLType;

int tc_CRLCheckPKIXCompliance(
    const TC_CertificateList *crl,
    TC_CRLType       crlType,
    TC_CONTEXT *ctx);



/* 
-----
utilities
----- 
*/
int tc_read_block(unsigned char **buf, 
		size_t *buflen, 
		const char *fname,
		  TC_CONTEXT *context);

int tc_write_block(const unsigned char *p, 
		   const size_t plen,
	           const char *fname,
		   TC_CONTEXT *);

int tc_encode_base64 (unsigned char **out, 
		      size_t *outlen,
		      const unsigned char *in, 
		      size_t inlen,
		      TC_CONTEXT *ctx);

int tc_decode_base64 (unsigned char **out, 
		      size_t *outlen,
		      const unsigned char *in,
		      TC_CONTEXT *ctx);

int tc_create_oid (unsigned char **p, 
		size_t *plen, 
		const char *oid,
		   TC_CONTEXT *ctx);

int tc_get_sigalgorithm(unsigned char **alg, 
		  size_t *len, 
		  TC_CERT *cert,
		  TC_CONTEXT *ctx);

int tc_get_serial(unsigned char **serial, 
		  size_t *len, 
		  TC_CERT *cert,
		  TC_CONTEXT *ctx);

int tc_get_key(unsigned char **key, 
	       size_t *len, 
	       int *nuub,
	       TC_CERT *cert,
	       TC_CONTEXT *ctx);

int tc_get_signature(unsigned char **sig, 
	       size_t *len, 
	       int *nuub,
	       TC_CERT *cert,
	       TC_CONTEXT *ctx);

int tc_get_version(int *version, 
		  TC_CERT *cert,
		  TC_CONTEXT *ctx);

int tc_get_params(unsigned char **params, 
		  size_t *len, 
		  TC_CERT *cert,
		  TC_CONTEXT *ctx);

time_t tc_get_choiceoftime (TC_Time *,
			    TC_CONTEXT *);

int tc_encode_utctime (TC_UTCTime **, time_t, TC_CONTEXT *ctx);

int tc_EncodeGeneralizedTime(
    TC_GeneralizedTime **gtTime,
    time_t t,
    TC_CONTEXT *ctx);


/************************************
 * Globals Constants
 ************************************/

#define TC_ASN_BOOLEAN PKIID_BOOLEAN
#define TC_ASN_INTEGER PKIID_INTEGER
#define TC_ASN_BIT_STRING PKIID_BIT_STRING
#define TC_ASN_OCTET_STRING PKIID_OCTET_STRING
#define TC_ASN_NULL PKIID_NULL
#define TC_ASN_OBJECT_ID PKIID_OBJECT_ID
#define TC_ASN_SEQUENCE PKIID_SEQUENCE
#define TC_ASN_SEQUENCE_OF PKIID_SEQUENCE_OF
#define TC_ASN_SET PKIID_SET
#define TC_ASN_SET_OF PKIID_SET_OF
#define TC_ASN_NumericString PKIID_NumericString
#define TC_ASN_PrintableString PKIID_PrintableString
#define TC_ASN_T61String PKIID_T61String
#define TC_ASN_TeletexString PKIID_TeletexString
#define TC_ASN_VideotexString PKIID_VideotexString
#define TC_ASN_IA5String PKIID_IA5String
#define TC_ASN_UTCTime PKIID_UTCTime
#define TC_ASN_GeneralizedTime PKIID_GeneralizedTime
#define TC_ASN_GraphicString PKIID_GraphicString
#define TC_ASN_VisibleString PKIID_VisibleString
#define TC_ASN_ISO646String PKIID_ISO646String
#define TC_ASN_GeneralString PKIID_GeneralString
#define TC_ASN_CHOICE PKIID_CHOICE

extern const char *TC_errlist[];	/* library error messages */


/** renaming of compiler gen'd OID values */

/* RSA */
#define TC_ALG_RSA      PKIrsaEncryption_OID
#define TC_ALG_RSA_MD2  PKImd2WithRSAEncryption_OID
#define TC_ALG_RSA_MD5  PKImd5WithRSAEncryption_OID
#define TC_ALG_RSA_SHA1 PKIsha1WithRSAEncryption_OID

#define TC_ALG_RSA_LEN      PKIrsaEncryption_OID_LEN
#define TC_ALG_RSA_MD2_LEN  PKImd2WithRSAEncryption_OID_LEN
#define TC_ALG_RSA_MD5_LEN  PKImd5WithRSAEncryption_OID_LEN
#define TC_ALG_RSA_SHA1_LEN PKIsha1WithRSAEncryption_OID_LEN

/* DSA */
#define TC_ALG_DSA      PKIid_dsa_OID
#define TC_ALG_DSA_SHA1 PKIid_dsa_with_sha1_OID

#define TC_ALG_DSA_LEN      PKIid_dsa_OID_LEN
#define TC_ALG_DSA_SHA1_LEN PKIid_dsa_with_sha1_OID_LEN

/* should only used for testing with BSAFE 3.0 */
extern const unsigned char TC_ALG_RSA_DSA[];
extern const unsigned char TC_ALG_RSA_DSA_SHA1[];
#define TC_ALG_RSA_DSA_LEN 5
#define TC_ALG_RSA_DSA_SHA1_LEN 5

#endif /* _tc_h_ */

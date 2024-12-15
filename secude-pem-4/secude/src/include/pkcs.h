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
/*  INCLUDE FILE  pkcs.h  (PKCS Standard)                                */
/*  Definition of structures and types for PKCS#1 - PKCS#10              */
/*-----------------------------------------------------------------------*/

#ifndef _PKCS_
#define _PKCS_

/*
 *   pkcs.h defines:
 *
 *      RSAPrivateKey 			    (typedef struct RSAPrivateKey)
 *      ExtendedCertificateOrCertificate    (typedef struct ExtendedCertificateOrCertificate)
 *      ExtendedCertificatesAndCertificates (typedef struct ExtendedCertificatesAndCertificates)
 *      ExtendedCertificateInfo 	    (typedef struct ExtendedCertificateInfo)
 *      ExtendedCertificate 		    (typedef struct ExtendedCertificate)
 *      IssuerAndSerialNumber 		    (typedef struct IssuerAndSerialNumber)
 *      ContentInfo 			    (typedef struct ContentInfo)
 *      SignerInfo 			    (typedef struct SignerInfo)
 *      SignerInfos 			    (typedef SET_OF(SignerInfo))
 *      RecipientInfo 			    (typedef struct RecipientInfo)
 *      RecipientInfos 			    (typedef SET_OF(RecipientInfo))
 *      EncryptedContentInfo 		    (typedef struct EncryptedContentInfo)
 *      SignedData 			    (typedef struct SignedData)
 *      EnvelopedData 			    (typedef struct EnvelopedData)
 *      SignedAndEnvelopedData 		    (typedef struct SignedAndEnvelopedData)
 *      DigestedData 			    (typedef struct DigestedData)
 *      EncryptedData 			    (typedef struct EncryptedData)
 *      CertificationRequestInfo 	    (typedef struct CertificationRequestInfo)
 *      CertificationRequest 		    (typedef struct CertificationRequest)
 *
 *
 */

#ifndef	_module_If_defined_
#include "If-types.h"           /* from ISODE */
#endif

#ifndef _AF_
#include "af.h" 
#endif

typedef struct RSAPrivateKey 				RSAPrivateKey;
typedef struct ExtendedCertificateOrCertificate 	ExtendedCertificateOrCertificate;
typedef SET_OF(ExtendedCertificateOrCertificate)        ExtendedCertificatesAndCertificates;
typedef struct ExtendedCertificateInfo 			ExtendedCertificateInfo;
typedef struct ExtendedCertificate 			ExtendedCertificate;
typedef struct IssuerAndSerialNumber 			IssuerAndSerialNumber;
typedef struct ContentInfo 				ContentInfo;
typedef struct SignerInfo 				SignerInfo;
typedef SET_OF(SignerInfo)                          	SignerInfos;
typedef struct RecipientInfo 				RecipientInfo;
typedef SET_OF(RecipientInfo) 				RecipientInfos;
typedef struct EncryptedContentInfo 			EncryptedContentInfo;
typedef struct SignedData				SignedData;
typedef struct EnvelopedData				EnvelopedData;
typedef struct SignedAndEnvelopedData			SignedAndEnvelopedData;
typedef struct DigestedData				DigestedData;
typedef struct EncryptedData				EncryptedData;
typedef struct CertificationRequestInfo			CertificationRequestInfo;
typedef struct CertificationRequest			CertificationRequest;

typedef OctetString 		data_type;
typedef SignedData 		signedData_type;
typedef EnvelopedData 		envelopedData_type;
typedef SignedAndEnvelopedData  signedAndEnvelopedData_type;
typedef DigestedData 		digestedData_type;
typedef EncryptedData 		encryptedData_type;







struct RSAPrivateKey {
	int 	    version;
	OctetString modulus;
	OctetString pubex;
	OctetString privex;
	OctetString prime1;
	OctetString prime2;
	OctetString exp1;
	OctetString exp2;
	OctetString coeff;
};



struct ExtendedCertificateOrCertificate {
	int     choice;
        union {
                Certificate         * cert;
                ExtendedCertificate * extcert;
        } un;
};


struct ExtendedCertificateInfo {
	int 		version;
	Certificate   * cert;
	SET_OF_Attr   * attributes;
};

struct ExtendedCertificate {
        OctetString    		* tbs_DERcode; /* Return-Parameter of e_ExtendedCertificateInfo */
        ExtendedCertificateInfo * extcertinfo;
        Signature      		* sig;
};


struct IssuerAndSerialNumber {
  	DName       * issuer; 
	OctetString * serialnumber;
};


struct ContentInfo {
	ObjId * contentType;
	char  * content;
        union {
		OctetString            * data;
		SignedData             * signedData;
		EnvelopedData          * envelopedData;
		SignedAndEnvelopedData * signedAndEnvelopedData;
		DigestedData 	       * digestedData;
		EncryptedData 	       * encryptedData;
        } un;
        int     contentchoice;
};


struct SignerInfo {
  	int 			  version;  
	IssuerAndSerialNumber 	* issuerandserial;
	AlgId		      	* digestAI;
  	SET_OF_Attr	        * authAttributes;
	AlgId 			* digencAI;
	OctetString 		* encryptedDigest;
	SET_OF_Attr	        * unauthAttributes;
};


struct RecipientInfo {
  	int 			version;
	IssuerAndSerialNumber * issuerandserial;
	AlgId		      * keyencAI;
	OctetString 	      * encryptedKey;
};


struct EncryptedContentInfo {
	ObjId      * contentType;
	AlgId      * contentencAI;
	OctetString  encryptedContent;
};


struct SignedData {
  	int 	       			      version;
	SET_OF_AlgId 			    * digestAlgorithms;
	ContentInfo  			    * contentInfo;
	ExtendedCertificatesAndCertificates * certificates;
	SET_OF_CRL 			    * crls;
	SignerInfos 			    * signerInfos;
};


struct EnvelopedData {
  	int 		       version;
	RecipientInfos       * recipientInfos;
	EncryptedContentInfo * encryptedContInfo;
};


struct SignedAndEnvelopedData {
  	int				      version;
	RecipientInfos 			    * recipientInfos;
	SET_OF_AlgId 			    * digestAIs;
	EncryptedContentInfo 		    * encryptedContInfo;
	ExtendedCertificatesAndCertificates * certificates;
	SET_OF_CRL 			    * crls;
	SignerInfos 			    * signerInfos;
};


struct DigestedData {
  	int 		  version;
	AlgId 		* digestAI;
	ContentInfo 	* contentInfo;
	OctetString 	* digest;
};


struct EncryptedData {
  	int		       version; 
	EncryptedContentInfo * encryptedContInfo;
};


struct CertificationRequestInfo {
  	int	  	   version;
	DName   	 * subject;
	KeyInfo 	 * subjectPK;
	SET_OF_Attr      * attributes;
};


struct CertificationRequest {
        OctetString    		 * tbs_DERcode; /* Return-Parameter of e_CertificationRequestInfo */
	CertificationRequestInfo * certreqinfo;
        Signature      		 * sig;         /* issuer's signature */
};

#endif

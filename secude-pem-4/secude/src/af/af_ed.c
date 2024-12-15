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
 ************************************************************************/

/*-----------------------af-encdec.c------------------------------------*/
/*----------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (F2.G3)               	*/
/* Rheinstr. 75 / Dolivostr. 15                                     	*/
/* 6100 Darmstadt                                                  	*/
/* Project ``Secure DFN'' 1990                                      	*/
/* Grimm/Nausester/Schneider/Viebeg/Vollmer et alii                 	*/
/*----------------------------------------------------------------------*/
/*                                                                  	*/
/* PACKAGE   encode-decode   VERSION   1.3                          	*/
/*                              DATE   27.09.1990                   	*/
/*                                BY   Nausester/Grimm              	*/
/*                                                                  	*/
/*                            REVIEW                                	*/
/*                              DATE                                	*/
/*                                BY                                	*/
/* DESCRIPTION                                                      	*/
/*   This modul presents     functions to encode and                    */
/*   decode X509-Octetstrings into/from C-structures               	*/
/*   ``Encode'': C-structure ---> ASN.1-Octetstring                 	*/
/*   ``Decode'': ASN.1-Octetstring ---> C-structure                 	*/
/*                                                                  	*/
/*                                                                  	*/
/* EXPORT                    DESCRIPTION                            	*/
/*                                                                  	*/
/*  e_DName()                  Encodes DName structure           	*/
/*  d_DName()                  Decodes DName structure         	        */
/*                                                                  	*/
/*  e_Certificates()           Encodes certificates                 	*/
/*  d_Certificates()           Decodes certificates                 	*/
/*                                                                  	*/
/*  e_Certificate ()           Encodes one certiciate               	*/
/*  d_Certificate ()           Decodes one certificate              	*/
/*                                                                  	*/
/*  e_CertificateSet ()        Encodes one certificateSet           	*/
/*  d_CertificateSet ()        Decodes one certificateSet           	*/
/*                                                                  	*/
/*  e_CertificatePairSet ()    Encodes one CrossCertificatePairSet  	*/
/*  d_CertificatePairSet ()    Decodes one CrossCertificatePairSet  	*/
/*                                                                 	*/
/*  e_Crl ()    	       Encodes one Revocation List (X.500) 	*/
/*  d_Crl ()                   Decodes one Revocation List (X.500)      */
/*                                                                      */
/*  e_RevCert ()    	       Encodes one Revoked Certificate (X.500)  */
/*  d_RevCert ()               Decodes one Revoked Certificate (X.500)  */
/*                                                                      */
/*  e_CRL ()                Encodes one Revocation List (PEM)        */
/*  d_CRL ()                Decodes one Revocation List (PEM)        */
/*                                                                      */
/*  e_CRLEntry ()    	       Encodes one Revoked Certificate (PEM)    */
/*  d_CRLEntry ()            Decodes one Revoked Certificate (PEM)    */
/*                                                                      */
/*  e_CrlSet ()    	       Encodes a CrlSet                         */
/*  d_CrlSet ()                Decodes a CrlSet                         */
/*                                                                      */
/*  e_ToBeSigned()             Encodes the ToBeSigned                   */
/*                             subfield of a Certificate          	*/
/*                                                                	*/
/*  e_FCPath()                 Encodes an FCPath                   	*/
/*  d_FCPath()                 Decodes an FCPath                   	*/
/*                                                                 	*/
/*  e_PKRoot()                 Encodes a PKRoot table              	*/
/*  d_PKRoot()                 Decodes a PKRoot table              	*/
/*                                                                 	*/
/*  e_PKList()                 Encodes a PKList table              	*/
/*  d_PKList()                 Decodes a PKList table              	*/
/*                                                                      */
/*  more encode-decode functions are defined                       	*/
/*  in sec-encdec.c:                                                	*/
/*                                                                  	*/
/*  e_AlgId()                  Encodes an algorithm id             	*/
/*  d_AlgId()                  Decodes an algorithm id             	*/
/*  d2_AlgId()                 Decodes an algorithm id                  */
/*                             into a given struct AlgId            	*/
/*                                                                      */
/*  e_KeyInfo()                Encodes a KeyInfo                   	*/
/*  d_KeyInfo()                Decodes a KeyInfo                   	*/
/*  d2_KeyInfo()               Decodes a KeyInfo into a            	*/
/*                             given struct KeyInfo                	*/
/*                                                                 	*/
/*  e_KeyBits()                Encodes KeyBits                      	*/
/*  d_KeyBits()                Decodes KeyBits                      	*/
/*                                                                  	*/
/*  e_PSEToc()                 Encodes PSEToc table                 	*/
/*  d_PSEToc()                 Decodes PSEToc table                 	*/
/*                                                                  	*/
/*                                                                      */
/* IMPORT                    DESCRIPTION                            	*/
/*                                                                  	*/
/*  build_..., parse_...       ISODE-PEPY resulted                  	*/
/*                             encode-decode functions             	*/
/*                             from: AF.py, IF.py, SEC.py          	*/
/*                                                                 	*/
/*  free_Octetstring()         Releases Octetstring memory          	*/
/*  free_Certificates()        Releases Certificates mem.          	*/
/*  err_Certficates()          error diagnostic                     	*/
/*                             from: sec-encdec.c                  	*/
/*                                                                 	*/
/*  aux_DName2Name()          Map between ISODE-PEPY               	*/
/*  aux_Name2DName()          defined C-structures and             	*/
/*                             "C=de;..." printable                 	*/
/*                             representations of Names             	*/
/*                             from: aux-encdec.c                   	*/
/*                                                                  	*/
/*  aux_PE2OctetString(),             Map between ISODE-PE              */
/*  aux_OctetString2PE()              (presentation elements)           */
/*                             and ASN.1 Octetstrings               	*/
/*                             from: aux-encdec.c                   	*/
/*                                                                  	*/
/*----------------------------------------------------------------------*/

#include "psap.h"
#include "af.h"
#include "SECIF-types.h"    /*   Definition of parse_SECIF_NonDerAttributeValues()
			         and parse_SECIF_NonDerRDN()  */
#ifdef TEST
#include <stdio.h>
#endif




/***************************************************************
 *
 * Procedure e_DName
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_DName(
	DName	 *namestruct
)

#else

OctetString *e_DName(
	namestruct
)
DName	 *namestruct;

#endif

{
	PE                 P_Name;
	OctetString      * ret;
	char	   	 * proc = "e_DName";

	if (namestruct == NULLDNAME)
		return (NULLOCTETSTRING);

	if (build_SECIF_Name(&P_Name, 1, 0, CNULL, namestruct) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_Name);
	pe_free(P_Name);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_DName
 *
 ***************************************************************/
#ifdef __STDC__

DName *d_DName(
	OctetString	 *asn1_string
)

#else

DName *d_DName(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE		P_Name;
	int		result;
	DName         * namestruct;
	char	      * proc = "d_DName";

	if (asn1_string == (OctetString * )0)
		return (NULLDNAME);

	P_Name = aux_OctetString2PE(asn1_string);

	if (P_Name == NULLPE)
		return (NULLDNAME);

	result = parse_SECIF_Name(P_Name, 1, NULLIP, NULLVP, &namestruct);
	pe_free(P_Name);

	return (result ? NULLDNAME : namestruct);
}


/***************************************************************
 *
 * Procedure e_Attribute
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_Attribute(
	Attr	 *attr
)

#else

OctetString *e_Attribute(
	attr
)
Attr	 *attr;

#endif

{
	PE                 P_Attribute;
	OctetString      * ret;
	char	   	 * proc = "e_Attribute";

	if (! attr)
		return (NULLOCTETSTRING);

	if (build_SECIF_Attribute(&P_Attribute, 1, 0, CNULL, attr) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_Attribute);
	pe_free(P_Attribute);

	return (ret);
}


/***************************************************************
 *
 * Procedure e_AttributeType
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_AttributeType(
	AttrType	 *attrtype
)

#else

OctetString *e_AttributeType(
	attrtype
)
AttrType	 *attrtype;

#endif

{
	PE                 P_AttributeType;
	OctetString      * ret;
	char	   	 * proc = "e_AttributeType";

	if (! attrtype)
		return (NULLOCTETSTRING);

	if (build_SECIF_AttributeType(&P_AttributeType, 1, 0, CNULL, attrtype) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_AttributeType);
	pe_free(P_AttributeType);

	return (ret);
}


/***************************************************************
 *
 * Procedure e_AttributeValueAssertion
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_AttributeValueAssertion(
	AttrValueAssertion	 *ava
)

#else

OctetString *e_AttributeValueAssertion(
	ava
)
AttrValueAssertion	 *ava;

#endif

{
	PE                 P_AVA;
	OctetString      * ret;
	char	   	 * proc = "e_AttributeValueAssertion";

	if (! ava)
		return (NULLOCTETSTRING);

	if (build_SECIF_AttributeValueAssertion(&P_AVA, 1, 0, CNULL, ava) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_AVA);
	pe_free(P_AVA);

	return (ret);
}


/***************************************************************
 *
 * Procedure e_Certificates
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_Certificates(
	Certificates	 *certificates
)

#else

OctetString *e_Certificates(
	certificates
)
Certificates	 *certificates;

#endif

{
	PE             P_Certificates;
	OctetString  * ret;
	char	     * proc = "e_Certificates";

	if (certificates == (Certificates * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_Certificates(&P_Certificates, 1, 0, CNULL, certificates) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_Certificates);
	pe_free(P_Certificates);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_Certificates
 *
 ***************************************************************/
#ifdef __STDC__

Certificates *d_Certificates(
	OctetString	 *asn1_string
)

#else

Certificates *d_Certificates(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE              P_Certificates;
	Certificates  * ret;
	int		result;
	char	      * proc = "d_Certificates";

	if (asn1_string == (OctetString * )0)
		return (Certificates * ) 0;

	P_Certificates = aux_OctetString2PE(asn1_string);

	if (P_Certificates == NULLPE)
		return (Certificates * ) 0;

	result = parse_SECAF_Certificates(P_Certificates, 1, NULLIP, NULLVP, &ret);
	pe_free(P_Certificates);

	return (result ? (Certificates * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_Certificate(
	Certificate	 *certificate
)

#else

OctetString *e_Certificate(
	certificate
)
Certificate	 *certificate;

#endif

{
	PE             P_Certificate ;
	OctetString  * ret;
	char	     * proc = "e_Certificate";

	if (certificate  == (Certificate  * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_Certificate (&P_Certificate , 1, 0, CNULL, certificate ) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_Certificate );
	pe_free(P_Certificate );

	return (ret);
}


/***************************************************************
 *
 * Procedure d_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

Certificate *d_Certificate(
	OctetString	 *asn1_string
)

#else

Certificate *d_Certificate(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE             P_Certificate ;
	Certificate  * ret;
	int	       result;
	char	     * proc = "d_Certificate";

	if (asn1_string == (OctetString * )0)
		return (Certificate  * ) 0;

	P_Certificate  = aux_OctetString2PE(asn1_string);

	if (P_Certificate  == NULLPE)
		return (Certificate  * ) 0;

	result = parse_SECAF_Certificate (P_Certificate , 1, NULLIP, NULLVP, &ret);
	pe_free(P_Certificate);

	return (result ? (Certificate  * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_CertificateSet
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CertificateSet(
	SET_OF_Certificate	 *certset
)

#else

OctetString *e_CertificateSet(
	certset
)
SET_OF_Certificate	 *certset;

#endif

{
	PE             P_CertSet ;
	OctetString  * ret;
	char	     * proc = "e_CertificateSet";

	if (certset  == (SET_OF_Certificate  * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_CertificateSet (&P_CertSet , 1, 0, CNULL, certset ) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_CertSet );
	pe_free(P_CertSet );

	return (ret);
}


/***************************************************************
 *
 * Procedure d_CertificateSet
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_Certificate *d_CertificateSet(
	OctetString	 *asn1_string
)

#else

SET_OF_Certificate *d_CertificateSet(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE                    P_CertSet ;
	SET_OF_Certificate  * ret;
	int		      result;
	char		    * proc = "d_CertificateSet";

	if (asn1_string == (OctetString * )0)
		return (SET_OF_Certificate  * ) 0;

	P_CertSet  = aux_OctetString2PE(asn1_string);

	if (P_CertSet  == NULLPE)
		return (SET_OF_Certificate  * ) 0;

	result = parse_SECAF_CertificateSet (P_CertSet , 1, NULLIP, NULLVP, &ret);
	pe_free(P_CertSet);

	return (result ? (SET_OF_Certificate  * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_CertificatePairSet
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CertificatePairSet(
	SET_OF_CertificatePair	 *cpairset
)

#else

OctetString *e_CertificatePairSet(
	cpairset
)
SET_OF_CertificatePair	 *cpairset;

#endif

{
	PE             P_CPairSet ;
	OctetString  * ret;
	char	     * proc = "e_CertificatePairSet";

	if (cpairset  == (SET_OF_CertificatePair  * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_CrossCertificatePair (&P_CPairSet , 1, 0, CNULL, cpairset ) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_CPairSet );
	pe_free(P_CPairSet );

	return (ret);
}


/***************************************************************
 *
 * Procedure d_CertificatePairSet
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_CertificatePair *d_CertificatePairSet(
	OctetString	 *asn1_string
)

#else

SET_OF_CertificatePair *d_CertificatePairSet(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE           		  P_Cpairset ;
	SET_OF_CertificatePair  * ret;
	int			  result;
	char			* proc = "d_CertificatePairSet";

	if (asn1_string == (OctetString * )0)
		return (SET_OF_CertificatePair  * ) 0;

	P_Cpairset  = aux_OctetString2PE(asn1_string);

	if (P_Cpairset  == NULLPE)
		return (SET_OF_CertificatePair  * ) 0;

	result = parse_SECAF_CrossCertificatePair (P_Cpairset , 1, NULLIP, NULLVP, &ret);
	pe_free(P_Cpairset);

	return (result ? (SET_OF_CertificatePair  * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_ToBeSigned
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_ToBeSigned(
	ToBeSigned	 *tobesigned
)

#else

OctetString *e_ToBeSigned(
	tobesigned
)
ToBeSigned	 *tobesigned;

#endif

{
	PE             P_TBSCertificate;
	OctetString  * ret;
	char	     * proc = "e_ToBeSigned";

	if (tobesigned == (ToBeSigned * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_TBSCertificate(&P_TBSCertificate, 1, 0, CNULL, tobesigned) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_TBSCertificate);
	pe_free(P_TBSCertificate);

	return (ret);
}


#ifdef COSINE
/***************************************************************
 *
 * Procedure e_AuthorisationAttributes
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_AuthorisationAttributes(
	AuthorisationAttributes	 *authattrbts
)

#else

OctetString *e_AuthorisationAttributes(
	authattrbts
)
AuthorisationAttributes	 *authattrbts;

#endif

{
	PE             P_AuthorisationAttributes;
	OctetString  * ret;
	char	     * proc = "e_AuthorisationAttributes";

	if (authattrbts == (AuthorisationAttributes * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_AuthorisationAttributes(&P_AuthorisationAttributes, 1, 0, CNULL, authattrbts)
	     == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_AuthorisationAttributes);
	pe_free(P_AuthorisationAttributes);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_AuthorisationAttributes
 *
 ***************************************************************/
#ifdef __STDC__

AuthorisationAttributes *d_AuthorisationAttributes(
	OctetString	 *asn1_string
)

#else

AuthorisationAttributes *d_AuthorisationAttributes(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE                         P_AuthorisationAttributes;
	AuthorisationAttributes  * ret;
	int			   result;
	char			 * proc = "d_AuthorisationAttributes";

	if (asn1_string == (OctetString * )0)
		return (AuthorisationAttributes * ) 0;

	P_AuthorisationAttributes = aux_OctetString2PE(asn1_string);

	if (P_AuthorisationAttributes == NULLPE)
		return (AuthorisationAttributes * ) 0;

	result = parse_SECAF_AuthorisationAttributes(P_AuthorisationAttributes, 1, NULLIP, NULLVP, &ret);
	pe_free(P_AuthorisationAttributes);

	return (result ? (AuthorisationAttributes * ) 0 : ret);
}
#endif


/***************************************************************
 *
 * Procedure e_FCPath
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_FCPath(
	FCPath	 *fcpath
)

#else

OctetString *e_FCPath(
	fcpath
)
FCPath	 *fcpath;

#endif

{
	PE             P_FCPath;
	OctetString  * ret;
	char	     * proc = "e_FCPath";

	if (fcpath == (FCPath * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_ForwardCertificationPath(&P_FCPath, 1, 0, CNULL, fcpath)
	     == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_FCPath);
	pe_free(P_FCPath);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_FCPath
 *
 ***************************************************************/
#ifdef __STDC__

FCPath *d_FCPath(
	OctetString	 *asn1_string
)

#else

FCPath *d_FCPath(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE            P_FCPath;
	FCPath      * ret;
	int	      result;
	char	    * proc = "d_FCPath";

	if (asn1_string == (OctetString * )0)
		return (FCPath * ) 0;

	P_FCPath = aux_OctetString2PE(asn1_string);

	if (P_FCPath == NULLPE)
		return (FCPath * ) 0;

	result = parse_SECAF_ForwardCertificationPath(P_FCPath, 1, NULLIP, NULLVP, &ret);
	pe_free(P_FCPath);

	return (result ? (FCPath * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_PKRoot
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_PKRoot(
	PKRoot	 *pkroot
)

#else

OctetString *e_PKRoot(
	pkroot
)
PKRoot	 *pkroot;

#endif

{
	PE             P_PKRoot ;
	OctetString  * ret;
	char	     * proc = "e_PKRoot";

	if (pkroot  == (PKRoot  * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_PKRoot (&P_PKRoot , 1, 0, CNULL, pkroot ) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_PKRoot );
	pe_free(P_PKRoot );

	return (ret);
}


/***************************************************************
 *
 * Procedure d_PKRoot
 *
 ***************************************************************/
#ifdef __STDC__

PKRoot *d_PKRoot(
	OctetString	 *asn1_string
)

#else

PKRoot *d_PKRoot(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE            P_PKRoot ;
	PKRoot      * ret;
	int	      result;
	char	    * proc = "d_PKRoot";

	if (asn1_string == (OctetString * )0)
		return (PKRoot  * ) 0;

	P_PKRoot  = aux_OctetString2PE(asn1_string);

	if (P_PKRoot  == NULLPE)
		return (PKRoot  * ) 0;

	result = parse_SECAF_PKRoot (P_PKRoot , 1, NULLIP, NULLVP, &ret);
	pe_free(P_PKRoot);

	return (result ? (PKRoot  * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_PKList
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_PKList(
	PKList	 *pklist
)

#else

OctetString *e_PKList(
	pklist
)
PKList	 *pklist;

#endif

{
	PE             P_PKList;
	OctetString  * ret;
	char	     * proc = "e_PKList";

	if (pklist == (PKList * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_PKList(&P_PKList, 1, 0, CNULL, pklist) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_PKList);
	pe_free(P_PKList);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_PKList
 *
 ***************************************************************/
#ifdef __STDC__

PKList *d_PKList(
	OctetString	 *asn1_string
)

#else

PKList *d_PKList(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE            P_PKList;
	PKList      * ret;
	int	      result;
	char	    * proc = "d_PKList";

	if (asn1_string == (OctetString * )0)
		return (PKList * ) 0;

	P_PKList = aux_OctetString2PE(asn1_string);

	if (P_PKList == NULLPE)
		return (PKList * ) 0;

	result = parse_SECAF_PKList(P_PKList, 1, NULLIP, NULLVP, &ret);
	pe_free(P_PKList);

	return (result ? (PKList * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_CRL
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CRL(
	CRL	 *crl
)

#else

OctetString *e_CRL(
	crl
)
CRL	 *crl;

#endif

{
	PE                 P_CRL;
	OctetString 	 * ret;
	char		 * proc = "e_CRL";

	if ( crl == (CRL * )0 )
		return( (OctetString * )0 );

	if ( build_SECAF_CRL (&P_CRL, 1, 0, CNULL, crl) == NOTOK )
		return( (OctetString * )0 );

	ret = aux_PE2OctetString(P_CRL);
	pe_free(P_CRL);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_CRL
 *
 ***************************************************************/
#ifdef __STDC__

CRL *d_CRL(
	OctetString	 *asn1_string
)

#else

CRL *d_CRL(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE         	   P_CRL;
	CRL 	 * ret;
	int		   result;
	char		 * proc = "d_CRL";

	if ( asn1_string == (OctetString * )0 )
		return( (CRL * )0 );

	P_CRL = aux_OctetString2PE(asn1_string);

	if ( P_CRL == NULLPE )
		return( (CRL * )0 );

	result = parse_SECAF_CRL (P_CRL, 1, NULLIP, NULLVP, &ret);
	pe_free(P_CRL);

	return ( result ? (CRL * )0 : ret );
}


/***************************************************************
 *
 * Procedure e_CRLEntry
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CRLEntry(
	CRLEntry	 *crlentry
)

#else

OctetString *e_CRLEntry(
	crlentry
)
CRLEntry	 *crlentry;

#endif

{
	PE 		     P_CRLEntry;
	OctetString	   * ret;
	char		   * proc = "e_CRLEntry";

	if ( crlentry == (CRLEntry * )0 )
		return( (OctetString * )0 );

	if ( build_SECAF_CRLEntry (&P_CRLEntry, 1, 0, CNULL, crlentry) == NOTOK )
		return( (OctetString * )0 );

	ret = aux_PE2OctetString(P_CRLEntry);
	pe_free(P_CRLEntry);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_CRLEntry
 *
 ***************************************************************/
#ifdef __STDC__

CRLEntry *d_CRLEntry(
	OctetString	 *asn1_string
)

#else

CRLEntry *d_CRLEntry(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE                   P_CRLEntry;
	CRLEntry	   * ret;
	int		     result;
	char	 	   * proc = "d_CRLEntry";

	if (asn1_string == (OctetString * )0 )
		return( (CRLEntry * )0 );

	P_CRLEntry = aux_OctetString2PE(asn1_string);

	if ( P_CRLEntry == NULLPE )
		return( (CRLEntry * )0 );

	result = parse_SECAF_CRLEntry (P_CRLEntry, 1, NULLIP, NULLVP, &ret);
	pe_free(P_CRLEntry);

	return ( result ? (CRLEntry * )0 : ret );
}


/***************************************************************
 *
 * Procedure e_CRLEntrySequence
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CRLEntrySequence(
	SEQUENCE_OF_CRLEntry	 *seq
)

#else

OctetString *e_CRLEntrySequence(
	seq
)
SEQUENCE_OF_CRLEntry	 *seq;

#endif

{
	PE                          P_CRLEntrySeq;
	OctetString               * ret;
	char			  * proc = "e_CRLEntrySequence";

	if ( seq == (SEQUENCE_OF_CRLEntry * )0 )
		return( (OctetString * )0 );

	if ( build_SECAF_CRLEntrySequence (&P_CRLEntrySeq, 1, 0, CNULL, seq) == NOTOK )
		return( (OctetString * )0 );

	ret = aux_PE2OctetString(P_CRLEntrySeq);
	pe_free(P_CRLEntrySeq);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_CRLEntrySequence
 *
 ***************************************************************/
#ifdef __STDC__

SEQUENCE_OF_CRLEntry *d_CRLEntrySequence(
	OctetString	 *asn1_string
)

#else

SEQUENCE_OF_CRLEntry *d_CRLEntrySequence(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE                          P_CRLEntrySeq;
	SEQUENCE_OF_CRLEntry    * ret;
	int			    result;
	char			  * proc = "d_CRLEntrySequence";

	if ( asn1_string == (OctetString * )0 )
		return( (SEQUENCE_OF_CRLEntry * )0 );

	P_CRLEntrySeq = aux_OctetString2PE(asn1_string);

	if ( P_CRLEntrySeq == NULLPE )
		return( (SEQUENCE_OF_CRLEntry * )0 );

	result = parse_SECAF_CRLEntrySequence (P_CRLEntrySeq, 1, NULLIP, NULLVP, &ret);
	pe_free(P_CRLEntrySeq);

	return ( result ? (SEQUENCE_OF_CRLEntry * )0 : ret );
}


/***************************************************************
 *
 * Procedure e_CrlSet
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CrlSet(
	CrlSet	 *crlset
)

#else

OctetString *e_CrlSet(
	crlset
)
CrlSet	 *crlset;

#endif

{
	PE              P_CrlSet ;
	OctetString   * ret;
	char	      * proc = "e_CrlSet";



	if (crlset  == (CrlSet  * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_CrlSet (&P_CrlSet , 1, 0, CNULL, crlset ) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_CrlSet );
	pe_free(P_CrlSet );

	return (ret);
}


/***************************************************************
 *
 * Procedure d_CrlSet
 *
 ***************************************************************/
#ifdef __STDC__

CrlSet *d_CrlSet(
	OctetString	 *asn1_string
)

#else

CrlSet *d_CrlSet(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE             P_CrlSet ;
	CrlSet   * ret;
	int            result;
	char	     * proc = "d_CrlSet";

	if (asn1_string == (OctetString * )0)
		return (CrlSet  * ) 0;

	P_CrlSet  = aux_OctetString2PE(asn1_string);

	if (P_CrlSet  == NULLPE)
		return (CrlSet  * ) 0;

	result = parse_SECAF_CrlSet (P_CrlSet , 1, NULLIP, NULLVP, &ret);
	pe_free(P_CrlSet);

	return (result ? (CrlSet  * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_CRLTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CRLTBS(
	CRLTBS	 *tbs
)

#else

OctetString *e_CRLTBS(
	tbs
)
CRLTBS	 *tbs;

#endif

{
	PE            P_CRLTBS;
	OctetString * ret;
	char	    * proc = "e_CRLTBS";

	if ( tbs == (CRLTBS * )0 )
		return( (OctetString * )0 );

	if ( build_SECAF_TBSCRL (&P_CRLTBS, 1, 0, CNULL, tbs) == NOTOK )
		return( (OctetString * )0 );

	ret = aux_PE2OctetString(P_CRLTBS);
	pe_free(P_CRLTBS);

	return (ret);
}


/***************************************************************
 *
 * Procedure certificate_enc
 *
 ***************************************************************/
#ifdef __STDC__

PE certificate_enc(
	Certificate	 *parm
)

#else

PE certificate_enc(
	parm
)
Certificate	 *parm;

#endif

{
	PE	   pe;
	char	 * proc = "certificate_enc";

	(void) build_SECAF_Certificate (&pe, 0, 0, CNULL, parm);
	return (pe);
}


/***************************************************************
 *
 * Procedure certificate_dec
 *
 ***************************************************************/
#ifdef __STDC__

Certificate *certificate_dec(
	PE	  pe
)

#else

Certificate *certificate_dec(
	pe
)
PE	  pe;

#endif

{
	Certificate * ret;
	int	      result;
	char	    * proc = "certificate_dec";

	if ( pe == NULLPE )
		return( (Certificate * )0 );

	result = parse_SECAF_Certificate (pe, 1, NULLIP, NULLVP, &ret);

	return (result ? (Certificate * )0 : ret);
}


/***************************************************************
 *
 * Procedure e_CRLWithCertificates
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CRLWithCertificates(
	CRLWithCertificates	 *arg
)

#else

OctetString *e_CRLWithCertificates(
	arg
)
CRLWithCertificates	 *arg;

#endif

{
	PE             P_CRLWithCertificates;
	OctetString  * ret;
	char	     * proc = "e_CRLWithCertificates";

	if (arg == (CRLWithCertificates * )0)
		return (NULLOCTETSTRING);

	if (build_SECAF_CRLWithCertificates(&P_CRLWithCertificates, 1, 0, CNULL, arg) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_CRLWithCertificates);
	pe_free(P_CRLWithCertificates);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_CRLWithCertificates
 *
 ***************************************************************/
#ifdef __STDC__

CRLWithCertificates *d_CRLWithCertificates(
	OctetString	 *asn1_string
)

#else

CRLWithCertificates *d_CRLWithCertificates(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE                        P_CRLWithCertificates;
	CRLWithCertificates         * ret;
	int		          result;
	char	      		* proc = "d_CRLWithCertificates";

	if (asn1_string == (OctetString * )0)
		return (CRLWithCertificates * ) 0;

	P_CRLWithCertificates = aux_OctetString2PE(asn1_string);

	if (P_CRLWithCertificates == NULLPE)
		return (CRLWithCertificates * ) 0;

	result = parse_SECAF_CRLWithCertificates(P_CRLWithCertificates, 1, NULLIP, NULLVP, &ret);
	pe_free(P_CRLWithCertificates);

	return (result ? (CRLWithCertificates * ) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_OCList
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_OCList(
	OCList	 *ocl
)

#else

OctetString *e_OCList(
	ocl
)
OCList	 *ocl;

#endif

{
	PE            P_Ocl;
	OctetString * ret;
	char	    * proc = "e_OCList";

	if ( ocl == (OCList * )0 )
		return( (OctetString * )0 );

	if ( build_SECAF_OldCertificateList (&P_Ocl, 1, 0, CNULL, ocl) == NOTOK )
		return( (OctetString * )0 );

	ret = aux_PE2OctetString(P_Ocl);
	pe_free(P_Ocl);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_OCList
 *
 ***************************************************************/
#ifdef __STDC__

OCList *d_OCList(
	OctetString	 *asn1_string
)

#else

OCList *d_OCList(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE             P_Ocl;
	OCList       * ret;
	int	       result;
	char	     * proc = "d_OCList";

	if ( asn1_string == (OctetString * )0 )
		return( (OCList * )0 );

	P_Ocl = aux_OctetString2PE(asn1_string);

	if ( P_Ocl == NULLPE )
		return( (OCList * )0 );

	result = parse_SECAF_OldCertificateList (P_Ocl, 1, NULLIP, NULLVP, &ret);
	pe_free(P_Ocl);

	return ( result ? (OCList * )0 : ret );
}


/***************************************************************
 *
 * Procedure e_AliasList
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_AliasList(
	AliasList	 *alist
)

#else

OctetString *e_AliasList(
	alist
)
AliasList	 *alist;

#endif

{
	PE                 P_AliasList;
	OctetString      * ret;
	char	   	 * proc = "e_AliasList";

	if (!alist) return (NULLOCTETSTRING);

	if (build_SECAF_AliasList(&P_AliasList, 1, 0, CNULL, alist) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_AliasList);
	pe_free(P_AliasList);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_AliasList
 *
 ***************************************************************/
#ifdef __STDC__

AliasList *d_AliasList(
	OctetString	 *asn1_string
)

#else

AliasList *d_AliasList(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE		P_AliasList;
	int		result;
	AliasList     * alist;
	char	      * proc = "d_AliasList";

	if (asn1_string == NULLOCTETSTRING)
		return ((AliasList * )0);

	P_AliasList = aux_OctetString2PE(asn1_string);

	if (P_AliasList == NULLPE)
		return ((AliasList * )0);

	result = parse_SECAF_AliasList(P_AliasList, 1, NULLIP, NULLVP, &alist);
	pe_free(P_AliasList);

	return (result ? (AliasList * )0 : alist);
}


/***************************************************************
 *
 * Procedure e_SerialNumber
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_SerialNumber(
	SerialNumber	 *serial
)

#else

OctetString *e_SerialNumber(
	serial
)
SerialNumber	 *serial;

#endif

{
	char           *proc = "e_SerialNumber";

	return (e_OctetString((OctetString *)serial));
}


/***************************************************************
 *
 * Procedure d_SerialNumber
 *
 ***************************************************************/
#ifdef __STDC__

SerialNumber *d_SerialNumber(
	OctetString	 *asn1_string
)

#else

SerialNumber *d_SerialNumber(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	char           *proc = "d_SerialNumber";

	return ((SerialNumber *)e_OctetString(asn1_string));
}








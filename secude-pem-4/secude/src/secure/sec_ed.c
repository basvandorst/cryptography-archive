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

/*-----------------------sec-encdec.c-------------------------------*/
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (F2.G3)               */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990                                      */
/* Grimm/Nausester/Schneider/Viebeg/Vollmer et alii                 */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PACKAGE   encode-decode   VERSION   1.3                          */
/*                              DATE   27.09.1990                   */
/*                                BY   Nausester/Grimm              */
/*                                                                  */
/*                            REVIEW                                */
/*                              DATE                                */
/*                                BY                                */
/* DESCRIPTION                                                      */
/*   This modul presents     functions to encode and                */
/*   decode X509-Octetstrings into/from C-structures                */
/*   ``Encode'': C-structure ---> ASN.1-Octetstring                 */
/*   ``Decode'': ASN.1-Octetstring ---> C-structure                 */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* EXPORT                    DESCRIPTION                            */
/*                                                                  */
/*  e_AlgId()                  Encodes an algorithm id              */
/*  d_AlgId()                  Decodes an algorithm id              */
/*  d2_AlgId()                 Decodes an algorithm id              */
/*                             into a given struct AlgId            */
/*                                                                  */
/*  e_KeyInfo()                Encodes a KeyInfo                    */
/*  d_KeyInfo()                Decodes a KeyInfo                    */
/*  d2_KeyInfo()               Decodes a KeyInfo into a             */
/*                             given struct KeyInfo                 */
/*                                                                  */
/*  e_DigestInfo()             Encodes a DigestInfo                 */
/*  d_DigestInfo()             Decodes a DigestInfo                 */
/*  d2_DigestInfo()            Decodes a DigestInfo into a          */
/*                             given struct DigestInfo              */
/*                                                                  */
/*  e_Signature()              Encodes a Signature                  */
/*  d_Signature()              Decodes a Signature                  */
/*  d2_Signature()             Decodes a Signature into a           */
/*                             given struct Signature               */
/*                                                                  */
/*  e_KeyBits()                Encodes KeyBits                      */
/*  e2_KeyBits()               Encodes KeyBits into a               */
/*                             given BitString                      */
/*  d_KeyBits()                Decodes KeyBits                      */
/*                                                                  */
/*  e_PSEToc()                  Encodes PSEToc table                */
/*  d_PSEToc()                  Decodes PSEToc table                */
/*                                                                  */
/*  free_Octetstring()         Releases Octetstring memory          */
/*  free_Certificates()        Releases Certificates mem.           */
/*  err_Certficates()          error diagnostic                     */
/*                                                                  */
/*                                                                  */
/*  more encode-decode functions are defined                        */
/*  in af-encdec.c:                                                 */
/*                                                                  */
/*  e_Certificates()           Encodes certificates                 */
/*  d_Certificates()           Decodes certificates                 */
/*                                                                  */
/*  e_Certificate ()           Encodes one certiciate               */
/*  d_Certificate ()           Decodes one certificate              */
/*                                                                  */
/*  e_FCPath()                 Encodes an FCPath                    */
/*  d_FCPath()                 Decodes an FCPath                    */
/*                                                                  */
/*  e_PKRoot()                 Encodes a PKRoot table               */
/*  d_PKRoot()                 Decodes a PKRoot table               */
/*                                                                  */
/*  e_PKList()                 Encodes a PKList table               */
/*  d_PKList()                 Decodes a PKList table               */
/*                                                                  */
/*  e_ToBeSigned()             Encodes the ToBeSigned               */
/*                             subfield of a Certificate            */
/*                                                                  */
/*                                                                  */
/* IMPORT                    DESCRIPTION                            */
/*                                                                  */
/*  build_..., parse_...       ISODE-PEPY resulted                  */
/*                             encode-decode functions              */
/*                             from: SEC.py                         */
/*                                                                  */
/*  aux_DName2Name()          Map between ISODE-PEPY                */
/*  aux_Name2DName()          defined C-structures and              */
/*                             "C=de;..." printable                 */
/*                             representations of Names             */
/*                             from: aux-encdec.c                   */
/*                                                                  */
/*  aux_PE2OctetString(),             Map between ISODE-PE          */
/*  aux_OctetString2PE()              (presentation elements)       */
/*                             and ASN.1 Octetstrings               */
/*                             from: aux-encdec.c                   */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/


#include <stdio.h>
#include "psap.h"
#include "secure.h"
#include "UNIV-types.h"

OctetString    *aux_PE2OctetString( /* PE */ );
PE              aux_OctetString2PE( /* (OctetString *) */ );


/***************************************************************
 *
 * Procedure e_AlgId
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_AlgId(
	AlgId	 *algid
)

#else

OctetString *e_AlgId(
	algid
)
AlgId	 *algid;

#endif

{
	PE              P_AlgId;
	OctetString    *ret;
	char           *proc = "e_AlgId";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (algid == NULLALGID)
		return (NULLOCTETSTRING);

	if (build_SEC_AlgorithmIdentifier(&P_AlgId, 1, 0, CNULL, algid) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_AlgId);
	pe_free(P_AlgId);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_AlgId
 *
 ***************************************************************/
#ifdef __STDC__

AlgId *d_AlgId(
	OctetString	 *asn1_string
)

#else

AlgId *d_AlgId(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE              P_AlgId;
	AlgId          *ret;	/* return value */
	int             result;
	char           *proc = "d_AlgId";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING)
		return (NULLALGID);

	P_AlgId = aux_OctetString2PE(asn1_string);
	if (P_AlgId == NULLPE) 
		return (NULLALGID);

	result = parse_SEC_AlgorithmIdentifier(P_AlgId, 1, NULLIP, NULLVP, &ret);
	pe_free(P_AlgId);

	return (result ? NULLALGID : ret);
}

/***************************************************************
 *
 * Procedure d2_AlgId
 *
 ***************************************************************/
#ifdef __STDC__

int d2_AlgId(
	OctetString	 *asn1_string,
	AlgId		 *algid
)

#else

int d2_AlgId(
	asn1_string,
	algid
)
OctetString	 *asn1_string;
AlgId		 *algid;

#endif

{
	PE              P_AlgId;
	AlgId 	       *tmp_algid;
	int             result;
	char           *proc = "d2_AlgId";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING || !algid){
		aux_add_error(EINVALID, "Invalid Parameter", CNULL, 0, proc);
		return (-1);
	}

	P_AlgId = aux_OctetString2PE(asn1_string);
	if (P_AlgId == NULLPE) 
		return (-1);

      	/* Space for AlgId structure is given by calling routine. */

	result = parse_SEC_AlgorithmIdentifier(P_AlgId, 1, NULLIP, NULLVP, &tmp_algid);
	pe_free(P_AlgId);

	if(result < 0) return(result);

	result = aux_cpy2_AlgId(algid, tmp_algid);
	if(tmp_algid) aux_free_AlgId(&tmp_algid);

	return (0);
}

/***************************************************************
 *
 * Procedure e_KeyInfo
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_KeyInfo(
	KeyInfo	 *ki
)

#else

OctetString *e_KeyInfo(
	ki
)
KeyInfo	 *ki;

#endif

{
	PE              P_KeyInfo;
	OctetString    *ret;
	char           *proc = "e_KeyInfo";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (ki == (KeyInfo *) 0)
		return (NULLOCTETSTRING);

	if (build_SEC_KeyInfo(&P_KeyInfo, 1, 0, CNULL, ki) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_KeyInfo);
	pe_free(P_KeyInfo);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_KeyInfo
 *
 ***************************************************************/
#ifdef __STDC__

KeyInfo *d_KeyInfo(
	OctetString	 *asn1_string
)

#else

KeyInfo *d_KeyInfo(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE              P_KeyInfo;
	KeyInfo        *ret;	/* return value */
	int             result;
	char           *proc = "d_KeyInfo";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING)
		return (KeyInfo *) 0;

	P_KeyInfo = aux_OctetString2PE(asn1_string);
	if (P_KeyInfo == NULLPE) 
		return (KeyInfo *) 0;

	result = parse_SEC_KeyInfo(P_KeyInfo, 1, NULLIP, NULLVP, &ret);
	pe_free(P_KeyInfo);

	return (result ? (KeyInfo *) 0 : ret);
}


/***************************************************************
 *
 * Procedure d2_KeyInfo
 *
 ***************************************************************/
#ifdef __STDC__

int d2_KeyInfo(
	OctetString	 *asn1_string,
	KeyInfo		 *ki
)

#else

int d2_KeyInfo(
	asn1_string,
	ki
)
OctetString	 *asn1_string;
KeyInfo		 *ki;

#endif

{
	PE              P_KeyInfo;
	KeyInfo	       *tmp_ki;
	int             result;
	char           *proc = "d2_KeyInfo";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING || !ki){
		aux_add_error(EINVALID, "Invalid Parameter", CNULL, 0, proc);
		return (-1);
	}

	P_KeyInfo = aux_OctetString2PE(asn1_string);

	if (P_KeyInfo == NULLPE)
		return (-1);

      	/* Space for KeyInfo structure is given by calling routine. */

	result = parse_SEC_KeyInfo(P_KeyInfo, 1, NULLIP, NULLVP, &tmp_ki);
	pe_free(P_KeyInfo);

	if(result < 0) return(result);

	result = aux_cpy2_KeyInfo(ki, tmp_ki);
	aux_free_KeyInfo(&tmp_ki);

	return (0);
}


/***************************************************************
 *
 * Procedure e_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_EncryptedKey(
	EncryptedKey	 *enki
)

#else

OctetString *e_EncryptedKey(
	enki
)
EncryptedKey	 *enki;

#endif

{
	PE              P_EncryptedKey;
	OctetString    *ret;
	char           *proc = "e_EncryptedKey";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (enki == (EncryptedKey *) 0)
		return (NULLOCTETSTRING);

	if (build_SEC_EncryptedKey(&P_EncryptedKey, 1, 0, CNULL, enki) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_EncryptedKey);
	pe_free(P_EncryptedKey);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

EncryptedKey *d_EncryptedKey(
	OctetString	 *asn1_string
)

#else

EncryptedKey *d_EncryptedKey(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE              P_EncryptedKey;
	EncryptedKey   *ret;	/* return value */
	int             result;
	char           *proc = "d_EncryptedKey";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING)
		return (EncryptedKey *) 0;

	P_EncryptedKey = aux_OctetString2PE(asn1_string);
	if (P_EncryptedKey == NULLPE) 
		return (EncryptedKey *) 0;

	result = parse_SEC_EncryptedKey(P_EncryptedKey, 1, NULLIP, NULLVP, &ret);
	pe_free(P_EncryptedKey);

	return (result ? (EncryptedKey *) 0 : ret);
}


/***************************************************************
 *
 * Procedure d2_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

int d2_EncryptedKey(
	OctetString	 *asn1_string,
	EncryptedKey	 *enki
)

#else

int d2_EncryptedKey(
	asn1_string,
	enki
)
OctetString	 *asn1_string;
EncryptedKey	 *enki;

#endif

{
	PE              P_EncryptedKey;
	EncryptedKey   *tmp_enki;
	int             result;
	char           *proc = "d2_EncryptedKey";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING || !enki){
		aux_add_error(EINVALID, "Invalid Parameter", CNULL, 0, proc);
		return (-1);
	}

	P_EncryptedKey = aux_OctetString2PE(asn1_string);
	if (P_EncryptedKey == NULLPE)
		return (-1);

      	/* Space for EncryptedKey structure is given by calling routine. */

	result = parse_SEC_EncryptedKey(P_EncryptedKey, 1, NULLIP, NULLVP, &tmp_enki);
	pe_free(P_EncryptedKey);

	if(result < 0) return(result);

	result = aux_cpy2_EncryptedKey(enki, tmp_enki);
	aux_free_EncryptedKey(&tmp_enki);

	return (0);
}


/***************************************************************
 *
 * Procedure e_Signature
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_Signature(
	Signature	 *sig
)

#else

OctetString *e_Signature(
	sig
)
Signature	 *sig;

#endif

{
	PE              P_KeyInfo;
	OctetString    *ret;
	char           *proc = "e_Signature";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (sig == (Signature *) 0)
		return (NULLOCTETSTRING);

	if (build_SEC_KeyInfo(&P_KeyInfo, 1, 0, CNULL, (KeyInfo *) sig) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_KeyInfo);
	pe_free(P_KeyInfo);

	return (ret);
}

/***************************************************************
 *
 * Procedure d_Signature
 *
 ***************************************************************/
#ifdef __STDC__

Signature *d_Signature(
	OctetString	 *asn1_string
)

#else

Signature *d_Signature(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE              P_KeyInfo;
	Signature      *ret;	/* return value */
	int             result;
	char           *proc = "d_Signature";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING)
		return (Signature *) 0;

	P_KeyInfo = aux_OctetString2PE(asn1_string);
	if (P_KeyInfo == NULLPE) 
		return (Signature *) 0;

	result = parse_SEC_KeyInfo(P_KeyInfo, 1, NULLIP, NULLVP, (KeyInfo **)&ret);
	pe_free(P_KeyInfo);

	return (result ? (Signature *) 0 : ret);
}

/***************************************************************
 *
 * Procedure d2_Signature
 *
 ***************************************************************/
#ifdef __STDC__

int d2_Signature(
	OctetString	 *asn1_string,
	Signature	 *sig
)

#else

int d2_Signature(
	asn1_string,
	sig
)
OctetString	 *asn1_string;
Signature	 *sig;

#endif

{
	PE              P_KeyInfo;
	KeyInfo        *tmp_sig;
	int             result;
	char           *proc = "d2_Signature";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING || !sig){
		aux_add_error(EINVALID, "Invalid Parameter", CNULL, 0, proc);
		return (-1);
	}

	P_KeyInfo = aux_OctetString2PE(asn1_string);
	if (P_KeyInfo == NULLPE)
		return (-1);

      	/* Space for KeyInfo structure is given by calling routine. */

	result = parse_SEC_KeyInfo(P_KeyInfo, 1, NULLIP, NULLVP, &tmp_sig);
	pe_free(P_KeyInfo);

	if(result < 0) return(result);

	result = aux_cpy2_Signature(sig, tmp_sig);
	aux_free_KeyInfo(&tmp_sig);

	return (0);
}

/***************************************************************
 *
 * Procedure e_KeyBits
 *
 ***************************************************************/
#ifdef __STDC__

BitString *e_KeyBits(
	KeyBits	 *kb
)

#else

BitString *e_KeyBits(
	kb
)
KeyBits	 *kb;

#endif

{
	PE              P_KeyBits;

	/*
	 * OctetString *tempret;
	 */
	BitString      *ret;
	char           *proc = "e_KeyBits";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (kb == (KeyBits *) 0)
		return (BitString *) 0;

	if (build_SEC_KeyBits(&P_KeyBits, 1, 0, CNULL, kb) == NOTOK)
		return (BitString *) 0;

	ret = (BitString *) aux_PE2OctetString(P_KeyBits);
	ret->nbits = ret->nbits * 8;
	pe_free(P_KeyBits);

	return (ret);
}


/***************************************************************
 *
 * Procedure e2_KeyBits
 *
 ***************************************************************/
#ifdef __STDC__

int e2_KeyBits(
	KeyBits		 *kb,
	BitString	 *bstr
)

#else

int e2_KeyBits(
	kb,
	bstr
)
KeyBits		 *kb;
BitString	 *bstr;

#endif

{
	PE              P_KeyBits;
	OctetString    *ostr;
	int             i;
	char           *proc = "e2_KeyBits";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (kb == (KeyBits *) 0)
		return (-1);

	if (build_SEC_KeyBits(&P_KeyBits, 1, 0, CNULL, kb) == NOTOK)
		return (-1);

	ostr = aux_PE2OctetString(P_KeyBits);
	bstr->bits = ostr->octets;
	bstr->nbits = ostr->noctets * 8;
	free(ostr);

	pe_free(P_KeyBits);

	return (0);
}


/***************************************************************
 *
 * Procedure d_KeyBits
 *
 ***************************************************************/
#ifdef __STDC__

KeyBits *d_KeyBits(
	BitString	 *asn1_bstr
)

#else

KeyBits *d_KeyBits(
	asn1_bstr
)
BitString	 *asn1_bstr;

#endif

{
	PE              P_KeyBits;
	KeyBits        *ret;	/* return value */
	int             result;
	OctetString    *asn1_ostr;	/* !!! input is a BitString !!! */
	char           *proc = "d_KeyBits";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_bstr == (BitString *) 0)
		return (KeyBits *) 0;

	if ((asn1_ostr = (OctetString *) malloc(sizeof(OctetString)))
	    == NULLOCTETSTRING) {
		aux_add_error(EMALLOC, "asn1_ostr", CNULL, 0, proc);
		return (KeyBits *) 0;
	}
	asn1_ostr->octets = asn1_bstr->bits;
	asn1_ostr->noctets = asn1_bstr->nbits / 8;
	P_KeyBits = aux_OctetString2PE(asn1_ostr);

	if (P_KeyBits == NULLPE)
		return (KeyBits *) 0;

	result = parse_SEC_KeyBits(P_KeyBits, 1, NULLIP, NULLVP, &ret);
	pe_free(P_KeyBits);

	return (result ? (KeyBits *) 0 : ret);
}


/***************************************************************
 *
 * Procedure e_PSEToc
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_PSEToc(
	PSEToc	 *toc
)

#else

OctetString *e_PSEToc(
	toc
)
PSEToc	 *toc;

#endif

{
	PE              P_PSEToc;
	OctetString    *ret;
	char           *proc = "e_PSEToc";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (toc == (PSEToc *) 0)
		return (NULLOCTETSTRING);

	if (build_SEC_PSEToc(&P_PSEToc, 1, 0, CNULL, toc) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_PSEToc);
	pe_free(P_PSEToc);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_PSEToc
 *
 ***************************************************************/
#ifdef __STDC__

PSEToc *d_PSEToc(
	OctetString	 *asn1_string
)

#else

PSEToc *d_PSEToc(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE              P_PSEToc;
	PSEToc         *ret;	/* return value */
	int             result;
	char           *proc = "d_PSEToc";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING)
		return (PSEToc *) 0;

	P_PSEToc = aux_OctetString2PE(asn1_string);
	if (P_PSEToc == NULLPE)
		return (PSEToc *) 0;

	result = parse_SEC_PSEToc(P_PSEToc, 1, NULLIP, NULLVP, &ret);
	pe_free(P_PSEToc);

	return (result ? (PSEToc *) 0 : ret);
}

/***************************************************************
 *
 * Procedure e_PSEObject
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_PSEObject(
	ObjId		 *objectType,
	OctetString	 *objectValue
)

#else

OctetString *e_PSEObject(
	objectType,
	objectValue
)
ObjId		 *objectType;
OctetString	 *objectValue;

#endif

{
	PE              P_PSEObject;
	OctetString    *ret;
	PSEObject      *pse_obj;
	int             i;
	char           *proc = "e_PSEObject";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (objectType == (ObjId *) 0)
		return (NULLOCTETSTRING);

	if (!objectValue || !objectValue->octets)
		return (NULLOCTETSTRING);

	/* allocate space for PSEObject structure: */
	if ((pse_obj = (PSEObject *) malloc(sizeof(PSEObject)))
	    == (PSEObject *) 0) {
		aux_add_error(EMALLOC, "pse_obj", CNULL, 0, proc);
		return (NULLOCTETSTRING);
	}
	pse_obj->objectType = aux_cpy_ObjId(objectType);
	if (!(pse_obj->objectValue = (OctetString *) malloc(sizeof(OctetString)))) {
		aux_add_error(EMALLOC, "pse_obj->objectValue", CNULL, 0, proc);
		return (NULLOCTETSTRING);
	}
	pse_obj->objectValue->noctets = objectValue->noctets;
	if ((pse_obj->objectValue->octets = (char *) malloc(pse_obj->objectValue->noctets)) == (char *) 0) {
		aux_add_error(EMALLOC, "pse_obj->objectValue->octets", CNULL, 0, proc);
		return (NULLOCTETSTRING);
	}
	for (i = 0; i < pse_obj->objectValue->noctets; i++) {
		pse_obj->objectValue->octets[i] = objectValue->octets[i];
	}

	if (build_SEC_PSEObject(&P_PSEObject, 1, 0, CNULL, pse_obj) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_PSEObject);
	pe_free(P_PSEObject);

	return (ret);
}

/***************************************************************
 *
 * Procedure d_PSEObject
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *d_PSEObject(
	ObjId		 *objectType,
	OctetString	 *asn1_string
)

#else

OctetString *d_PSEObject(
	objectType,
	asn1_string
)
ObjId		 *objectType;
OctetString	 *asn1_string;

#endif

{
	PE              P_PSEObject;
	PSEObject      *pse_obj;
	int             result;
	int             i;
	char           *proc = "d_PSEObject";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (objectType == (ObjId *) 0)
		return (NULLOCTETSTRING);

	if (asn1_string == NULLOCTETSTRING)
		return (NULLOCTETSTRING);

	P_PSEObject = aux_OctetString2PE(asn1_string);
	if (P_PSEObject == NULLPE)
		return (NULLOCTETSTRING);

	result = parse_SEC_PSEObject(P_PSEObject, 1, NULLIP, NULLVP, &pse_obj);
	pe_free(P_PSEObject);

	if (result) return (NULLOCTETSTRING);

	aux_cpy2_ObjId(objectType, pse_obj->objectType);

	return (pse_obj->objectValue);
}


/***************************************************************
 *
 * Procedure e_OctetString
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_OctetString(
	OctetString	 *ostr
)

#else

OctetString *e_OctetString(
	ostr
)
OctetString	 *ostr;

#endif

{
	PE              P_OctetString;
	OctetString    *ret;
	char           *proc = "e_OctetString";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (ostr == NULLOCTETSTRING)
		return (NULLOCTETSTRING);

	if (build_SEC_OctetString(&P_OctetString, 1, 0, CNULL, ostr) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_OctetString);
	pe_free(P_OctetString);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_OctetString
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *d_OctetString(
	OctetString	 *asn1_string
)

#else

OctetString *d_OctetString(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE              P_OctetString;
	OctetString    *ret;	/* return value */
	int             result;
	char           *proc = "d_OctetString";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING)
		return (NULLOCTETSTRING);

	P_OctetString = aux_OctetString2PE(asn1_string);
	if (P_OctetString == NULLPE)
		return (NULLOCTETSTRING);

	result = parse_SEC_OctetString(P_OctetString, 1, NULLIP, NULLVP, &ret);
	pe_free(P_OctetString);

	return (result ? NULLOCTETSTRING : ret);
}

/***************************************************************
 *
 * Procedure e_BitString
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_BitString(
	BitString	 *bstr
)

#else

OctetString *e_BitString(
	bstr
)
BitString	 *bstr;

#endif

{
	PE              P_BitString;
	OctetString    *ret;
	char           *proc = "e_BitString";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (bstr == NULLBITSTRING)
		return (NULLOCTETSTRING);

	if (build_SEC_BitString(&P_BitString, 1, 0, CNULL, bstr) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_BitString);
	pe_free(P_BitString);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_BitString
 *
 ***************************************************************/
#ifdef __STDC__

BitString *d_BitString(
	OctetString	 *asn1_string
)

#else

BitString *d_BitString(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE              P_BitString;
	BitString    	*ret;	/* return value */
	int             result;
	char           *proc = "d_BitString";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING)
		return (NULLBITSTRING);

	P_BitString = aux_OctetString2PE(asn1_string);
	if (P_BitString == NULLPE)
		return (NULLBITSTRING);

	result = parse_SEC_BitString(P_BitString, 1, NULLIP, NULLVP, &ret);
	pe_free(P_BitString);

	return (result ? NULLBITSTRING : ret);
}


/***************************************************************
 *
 * Procedure e_DigestInfo
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_DigestInfo(
	DigestInfo	 *di
)

#else

OctetString *e_DigestInfo(
	di
)
DigestInfo	 *di;

#endif

{
	PE               P_DigestInfo;
	OctetString    * ret;
	char           * proc = "e_DigestInfo";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (di == (DigestInfo *) 0)
		return (NULLOCTETSTRING);

	if (build_SEC_DigestInfo(&P_DigestInfo, 1, 0, CNULL, di) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_DigestInfo);
	pe_free(P_DigestInfo);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_DigestInfo
 *
 ***************************************************************/
#ifdef __STDC__

DigestInfo *d_DigestInfo(
	OctetString	 *asn1_string
)

#else

DigestInfo *d_DigestInfo(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE               P_DigestInfo;
	DigestInfo     * ret;	/* return value */
	int              result;
	char           * proc = "d_DigestInfo";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING)
		return (DigestInfo *) 0;

	P_DigestInfo = aux_OctetString2PE(asn1_string);
	if (P_DigestInfo == NULLPE)
		return (DigestInfo *) 0;

	result = parse_SEC_DigestInfo(P_DigestInfo, 1, NULLIP, NULLVP, &ret);
	pe_free(P_DigestInfo);

	return (result ? (DigestInfo *) 0 : ret);
}


/***************************************************************
 *
 * Procedure d2_DigestInfo
 *
 ***************************************************************/
#ifdef __STDC__

int d2_DigestInfo(
	OctetString	 *asn1_string,
	DigestInfo	 *di
)

#else

int d2_DigestInfo(
	asn1_string,
	di
)
OctetString	 *asn1_string;
DigestInfo	 *di;

#endif

{
	PE               P_DigestInfo;
	DigestInfo     * tmp_di;
	int              result;
	char           * proc = "d2_DigestInfo";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (asn1_string == NULLOCTETSTRING || !di){
		aux_add_error(EINVALID, "Invalid Parameter", CNULL, 0, proc);
		return (-1);
	}

	P_DigestInfo = aux_OctetString2PE(asn1_string);
	if (P_DigestInfo == NULLPE)
		return (-1);

      	/* Space for DigestInfo structure is given by calling routine. */

	result = parse_SEC_DigestInfo(P_DigestInfo, 1, NULLIP, NULLVP, &tmp_di);
	pe_free(P_DigestInfo);

	if(result < 0) return(result);

	result = aux_cpy2_DigestInfo(di, tmp_di);
	aux_free_DigestInfo(&tmp_di);

	return (0);
}


/***************************************************************
 *
 * Procedure e_GRAPHICString
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_GRAPHICString(
	char	 *string
)

#else

OctetString *e_GRAPHICString(
	string
)
char	 *string;

#endif

{
	PE               P_GRAPHICString;
	OctetString    * ret;
	struct qbuf    * qb;
	char           * proc = "e_GRAPHICString";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (! string)
		return (NULLOCTETSTRING);

	qb = str2qb (string, strlen(string), 1);
	if(! qb){
		aux_add_error(EINVALID, "str2qb failed", CNULL, 0, proc);
		return (NULLOCTETSTRING);
	}

	if (encode_UNIV_GraphicString(&P_GRAPHICString, 1, 0, CNULL, qb) == NOTOK)
		return (NULLOCTETSTRING);

	ret = aux_PE2OctetString(P_GRAPHICString);
	pe_free(P_GRAPHICString);
	qb_free(qb);

	return (ret);
}


/***************************************************************
 *
 * Procedure d_GRAPHICString
 *
 ***************************************************************/
#ifdef __STDC__

char *d_GRAPHICString(
	OctetString	 *asn1_string
)

#else

char *d_GRAPHICString(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE               P_GRAPHICString;
	struct qbuf    * qb;
	char           * ret;	/* return value */
	int              result;
	char           * proc = "d_GRAPHICString";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (! asn1_string)
		return (CNULL);

	P_GRAPHICString = aux_OctetString2PE(asn1_string);

	if (P_GRAPHICString == NULLPE)
		return (CNULL);

	result = decode_UNIV_GraphicString(P_GRAPHICString, 1, NULLIP, NULLVP, &qb);
	pe_free(P_GRAPHICString);

	if(result == NOTOK)
		return (CNULL);

	ret = qb2str(qb);
	if(qb) qb_free(qb);

	return (ret);
}

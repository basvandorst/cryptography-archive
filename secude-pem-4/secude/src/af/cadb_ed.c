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

/*---------------------------km-ed.c--------------------------------*/
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (I2)                  */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990                                      */
/*------------------------------------------------------------------*/
/*                                                                  */
/* DESCRIPTION                                                      */
/*   This modul presents     functions to encode and                */
/*   decode X509-Octetstrings into/from C-structures                */
/*   ``Encode'': C-structure ---> ASN.1-Octetstring                 */
/*   ``Decode'': ASN.1-Octetstring ---> C-structure                 */
/*                                                                  */
/*                                                                  */
/* EXPORT                    DESCRIPTION                            */
/*                                                                  */
/*  e_SET_OF_IssuedCertificate()  				    */
/*  d_SET_OF_IssuedCertificate()             		            */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "psap.h"
#include "cadb.h"




/***************************************************************
 *
 * Procedure e_SET_OF_IssuedCertificate
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_SET_OF_IssuedCertificate(
	SET_OF_IssuedCertificate	 *isscertset
)

#else

OctetString *e_SET_OF_IssuedCertificate(
	isscertset
)
SET_OF_IssuedCertificate	 *isscertset;

#endif

{
	PE            P_IssCertSet ;
	OctetString  *ret;

	if (isscertset  == (SET_OF_IssuedCertificate  * )0)
		return (OctetString * ) 0;

	if (build_CADB_IssuedCertificateSet (&P_IssCertSet , 1, 0, CNULL, isscertset ) == NOTOK)
		return (OctetString * ) 0;

	ret = aux_PE2OctetString(P_IssCertSet );
	pe_free(P_IssCertSet );

	return (ret);
}


/***************************************************************
 *
 * Procedure d_SET_OF_IssuedCertificate
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_IssuedCertificate *d_SET_OF_IssuedCertificate(
	OctetString	 *asn1_string
)

#else

SET_OF_IssuedCertificate *d_SET_OF_IssuedCertificate(
	asn1_string
)
OctetString	 *asn1_string;

#endif

{
	PE                         P_IssCertSet ;
	SET_OF_IssuedCertificate  *ret;
	int			   result;

	if (asn1_string == (OctetString * )0)
		return (SET_OF_IssuedCertificate  * ) 0;

	P_IssCertSet  = aux_OctetString2PE(asn1_string);

	if (P_IssCertSet  == NULLPE)
		return (SET_OF_IssuedCertificate  * ) 0;

	result = parse_CADB_IssuedCertificateSet (P_IssCertSet , 1, NULLIP, NULLVP, &ret);
	pe_free(P_IssCertSet);

	return (result ? (SET_OF_IssuedCertificate  * ) 0 : ret);
}

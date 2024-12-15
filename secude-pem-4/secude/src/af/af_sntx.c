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

#ifdef X500
#include <stdio.h>

#include "psap.h"
#include "af.h"

#include "UNIV-types.h"
#include "config.h"
#include "isoaddrs.h"
#include "attr.h"
#include "config.h"
#include "dsp.h"
#include "oid.h"
#include "AF-types.h"
#include "af-cdefs.h"
#include "if-cdefs.h"
#include "nrs-cdefs.h"
#include "qu-cdefs.h"
#include "syntaxes.h"
#include "authen.h"

/* from IC/isode/src/lib/syntax/x500/certificate.c */
extern void print_algid();
extern void print_encrypted();
extern void str2alg();
extern void str2encrypted();
extern int cert_cmp();
extern struct certificate *cert_dec();
extern struct certificate *str2cert();
extern PE cert_enc();

/* from IC/isode/src/lib/dsap/common/dn_str.c */
extern DN dn_dec();
extern DN str2dn();
extern PE dn_enc();

/* from IC/isode/src/lib/dsap/common/dn_print.c */
extern void	dn_print();

extern int	dn_free();
extern int	dn_cmp();

extern OIDentifier *name2oid();


/************* local functions: ******************************/

/***************************************************************
 *
 * Procedure aux_cert_cmp
 *
 ***************************************************************/
#ifdef __STDC__

int aux_cert_cmp(
	Certificate	 *a,
	Certificate	 *b
)

#else

int aux_cert_cmp(
	a,
	b
)
Certificate	 *a;
Certificate	 *b;

#endif

{
	struct certificate * quipu_a, * quipu_b;
	PE		     pe;
	int		     ret;
	char		   * proc = "aux_cert_cmp";

	build_SECAF_Certificate (&pe, 0, 0, NULLCP, a);
	quipu_a = cert_dec(pe);
	pe_free(pe);

	build_SECAF_Certificate (&pe, 0, 0, NULLCP, b);
	quipu_b = cert_dec(pe);
	pe_free(pe);

	ret = cert_cmp(quipu_a, quipu_b);

	cert_free(quipu_a);
	cert_free(quipu_b);

	return (ret);
}


/***************************************************************
 *
 * Procedure AlgId_enc
 *
 ***************************************************************/
#ifdef __STDC__

PE AlgId_enc(
	AlgId	 *parm
)

#else

PE AlgId_enc(
	parm
)
AlgId	 *parm;

#endif

{
	PE pe;
	(void) build_SEC_AlgorithmIdentifier(&pe, 1, 0, NULLCP, parm);
	return (pe);
}


/***************************************************************
 *
 * Procedure AlgId_dec
 *
 ***************************************************************/
#ifdef __STDC__

AlgId *AlgId_dec(
	PE	  pe
)

#else

AlgId *AlgId_dec(
	pe
)
PE	  pe;

#endif

{
	AlgId * ret;  /*return value*/
	int	result;

	if ( pe == NULLPE )
		return ( (AlgId * )0 );

	result = parse_SEC_AlgorithmIdentifier (pe, 1, NULLIP, NULLVP, &ret);

	return (result ? NULLALGID : ret);
}


/*******************************************************************************
 *
 *      Attribute Syntax for OldCertificates
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure oclist_enc
 *
 ***************************************************************/
#ifdef __STDC__

PE oclist_enc(
	OCList	 *parm
)

#else

PE oclist_enc(
	parm
)
OCList	 *parm;

#endif

{
	PE pe;

	(void) build_SECAF_OldCertificateList (&pe, 0, 0, NULLCP, parm);
	return (pe);
}


/***************************************************************
 *
 * Procedure oclist_dec
 *
 ***************************************************************/
#ifdef __STDC__

OCList *oclist_dec(
	PE	  pe
)

#else

OCList *oclist_dec(
	pe
)
PE	  pe;

#endif

{
	OCList * ret;
	int	result;

	if ( pe == NULLPE )
		return ( (OCList * )0 );

	result = parse_SECAF_OldCertificateList (pe, 1, NULLIP, NULLVP, &ret);

	return (result ? (OCList * )0 : ret);
}


/***************************************************************
 *
 * Procedure str2ocl
 *
 ***************************************************************/
#ifdef __STDC__

OCList *str2ocl(
	char	 *str
)

#else

OCList *str2ocl(
	str
)
char	 *str;

#endif

{
	OCList			 * ret;
	OCList			 * ocl;
	OCList			 * save_ocl;
	struct certificate 	 * quipu_cert;
	PE 			   pe;
	char			 * ptr;
	char			 * tmp_ptr;
	int			   i;
	int			   int_serial;
	char			 * proc = "str2ocl";

	/*  If the "Old Certificates"-list is not empty, it contains at least
	 *  two cross-certificates
	 */

	if ( !(ret = (OCList * )malloc(sizeof(OCList))) ) {
		aux_add_error(EMALLOC, "ret", CNULL, 0, proc);
		return( (OCList * )0 );
	}
	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("serialNumber not present", NULLCP);
		aux_add_error(EINVALID, "serialNumber not present", CNULL, 0, proc);
		free( (OCList * )ret );
		return( (OCList * )0 );
	}
	*ptr = '\0';
	ptr++;

	int_serial = atoi(str);
	ret->serialnumber = aux_int2OctetString(int_serial);
	if(!ret->serialnumber){
		aux_add_error(EINVALID, "aux_int2OctetString failed", CNULL, 0, proc);
		free( (OCList * )ret );
		return( (OCList * )0 );
	}

	str = ptr;

	/* The printable representation of a certificate is subdivided by 14 '#'-symbols */
	for (tmp_ptr = str, i = 0; i < 14; i++) {
		ptr = strchr(tmp_ptr, '#');
		if ( ptr == NULLCP ) {
			parse_error("error in printable representation of cross-certificate", NULLCP);
			aux_add_error(EINVALID, "error in printable representation of cross-certificate", CNULL, 0, proc);
			free( (OCList * )ret );
			return( (OCList * )0 );
		}
		tmp_ptr = ptr + 1;
	}

	*(tmp_ptr - 1) = '\0'; /* replacing the last '#' of the first cross-certificate
					by '\0' */

	quipu_cert = str2cert(str);
	if ( quipu_cert == (struct certificate *)0 ) {
		free( (OCList * )ret );
		aux_add_error(EINVALID, "str2cert(str) empty", CNULL, 0, proc);
		return( (OCList * )0 );
	}

	pe = cert_enc(quipu_cert);
	if ( pe == NULLPE ) {
		free( (OCList * )ret );
		aux_add_error(EINVALID, "cert_enc failed", CNULL, 0, proc);
		cert_free(quipu_cert);
		return( (OCList * )0 );
	}
	ret->ccert = certificate_dec(pe);
	pe_free(pe);
	if ( ret->ccert == (Certificate * )0 ) {
		free( (OCList * )ret );
		aux_add_error(EINVALID, "certificate_dec failed", CNULL, 0, proc);
		cert_free(quipu_cert);
		return( (OCList * )0 );
	}
	cert_free(quipu_cert);

	if ( !(ocl = (OCList * )malloc(sizeof(OCList))) ) {
		aux_add_error(EMALLOC, "ocl", CNULL, 0, proc);
		free( (Certificate * )ret->ccert );
		free( (OCList * )ret );
		return( (OCList * )0 );
	}


	ret->next = ocl;
	ocl->next = (OCList * )0;
	ocl->ccert = (Certificate * )0;


	str = tmp_ptr;

	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("serialNumber of second list element not present", NULLCP);
		aux_add_error(EINVALID, "serialNumber of second list element not present", CNULL, 0, proc);
		aux_free_OCList(&ret);
		return( (OCList * )0 );
	}
	*ptr = '\0';
	ptr++;

	int_serial = atoi(str);
	ocl->serialnumber = aux_int2OctetString(int_serial);
	if(!ocl->serialnumber){
		aux_add_error(EINVALID, "aux_int2OctetString failed", CNULL, 0, proc);
		aux_free_OCList(&ret);
		return( (OCList * )0 );
	}

	str = ptr;

	for (tmp_ptr = str, i = 0; i < 14; i++) {
		ptr = strchr(tmp_ptr, '#');
		if ( ptr == NULLCP ) {
			parse_error("error in printable representation of cross-certificate", NULLCP);
			aux_add_error(EINVALID, "error in printable representation of cross-certificate", CNULL, 0, proc);
			free( (OCList * )ret );
			return( (OCList * )0 );
		}
		tmp_ptr = ptr + 1;
	}

	*(tmp_ptr - 1) = '\0'; /* replacing the last '#' of the second cross-certificate
					by '\0' */

	quipu_cert = str2cert(str);
	if ( quipu_cert == (struct certificate *)0 ) {
		aux_free_OCList(&ret);
		aux_add_error(EINVALID, "str2cert(str) empty", CNULL, 0, proc);
		return( (OCList * )0 );
	}

	pe = cert_enc(quipu_cert);
	if ( pe == NULLPE ) {
		aux_free_OCList(&ret);
		cert_free(quipu_cert);
		aux_add_error(EINVALID, "cert_enc failed", CNULL, 0, proc);
		return( (OCList * )0 );
	}
	ocl->ccert = certificate_dec(pe);
	pe_free(pe);
	if ( ocl->ccert == (Certificate * )0 ) {
		aux_free_OCList(&ret);
		aux_add_error(EINVALID, "certificate_dec failed", CNULL, 0, proc);
		cert_free(quipu_cert);
		return( (OCList * )0 );
	}
	cert_free(quipu_cert);


	save_ocl = ocl;


	str = tmp_ptr;


	while ( (ptr = strchr(str, '#')) != NULLCP ) {

		if ( !(ocl = (OCList * )malloc(sizeof(OCList))) ) {
			aux_add_error(EMALLOC, "ocl", CNULL, 0, proc);
			aux_free_OCList(&ret);
			return( (OCList * )0 );
		}

		save_ocl->next = ocl;
		save_ocl = ocl;
		ocl->next = (OCList * )0;
		ocl->ccert = (Certificate * )0;

		*ptr = '\0';
		ptr++;

		int_serial = atoi(str);
		ocl->serialnumber = aux_int2OctetString(int_serial);
		if(!ocl->serialnumber){
			aux_add_error(EINVALID, "aux_int2OctetString failed", CNULL, 0, proc);
			aux_free_OCList(&ret);
			return( (OCList * )0 );
		}


		str = ptr;

		for (tmp_ptr = str, i = 0; i < 14; i++) {
			ptr = strchr(tmp_ptr, '#');
			if ( ptr == NULLCP ) {
				parse_error("error in printable representation of cross-certificate", NULLCP);
				aux_add_error(EINVALID, "error in printable representation of cross-certificate", CNULL, 0, proc);
				free( (OCList * )ret );
				return( (OCList * )0 );
			}
			tmp_ptr = ptr + 1;
		}

		*(tmp_ptr - 1) = '\0'; /* replacing the last '#' of the second cross-certificate
							by '\0' */

		quipu_cert = str2cert(str);
		if ( quipu_cert == (struct certificate *)0 ) {
			aux_add_error(EINVALID, "str2cert(str) empty", CNULL, 0, proc);
			aux_free_OCList(&ret);
			return( (OCList * )0 );
		}

		pe = cert_enc(quipu_cert);
		if ( pe == NULLPE ) {
			aux_free_OCList(&ret);
			aux_add_error(EINVALID, "cert_enc failed", CNULL, 0, proc);
			cert_free(quipu_cert);
			return( (OCList * )0 );
		}
		ocl->ccert = certificate_dec(pe);
		pe_free(pe);
		if ( ocl->ccert == (Certificate * )0 ) {
			aux_free_OCList(&ret);
			aux_add_error(EINVALID, "certificate_dec failed", CNULL, 0, proc);
			cert_free(quipu_cert);
			return( (OCList * )0 );
		}
		cert_free(quipu_cert);

		str = tmp_ptr;

	}    /* while */

	return (ret);
}


/***************************************************************
 *
 * Procedure printocl
 *
 ***************************************************************/
#ifdef __STDC__

int printocl(
	PS	  ps,
	OCList	 *parm,
	int	  format
)

#else

int printocl(
	ps,
	parm,
	format
)
PS	  ps;
OCList	 *parm;
int	  format;

#endif

{
	PE 		     pe;
	struct certificate * quipu_cert;
	OCList 		   * ocl;
	char	  	   * proc = "printocl";

	if ( parm ) {

		ps_printf(ps, "%d#", parm->serialnumber);

		if ( parm->ccert ) {
			build_SECAF_Certificate (&pe, 0, 0, NULLCP, parm->ccert);
			quipu_cert = cert_dec(pe);
			pe_free(pe);
			if ( quipu_cert ) {
				printcert(ps, quipu_cert, format);
				cert_free(quipu_cert);
			}
		}

		/*  Notice: printcert() appends a "#" to the printed certificate.
	 */

		ocl = parm->next;

		while ( ocl ) {
			ps_printf(ps, "%d#", ocl->serialnumber);

			if ( ocl->ccert ) {
				build_SECAF_Certificate (&pe, 0, 0, NULLCP, ocl->ccert);
				quipu_cert = cert_dec(pe);
				pe_free(pe);
				if ( quipu_cert ) {
					printcert(ps, quipu_cert, format);
					cert_free(quipu_cert);
				}
			}

			ocl = ocl->next;
		}  /*while*/

	}  /*if*/
}


/***************************************************************
 *
 * Procedure aux_oclist_cmp
 *
 ***************************************************************/
#ifdef __STDC__

int aux_oclist_cmp(
	OCList	 *a,
	OCList	 *b
)

#else

int aux_oclist_cmp(
	a,
	b
)
OCList	 *a;
OCList	 *b;

#endif

{
	int	ret;

	if ( a->serialnumber > b->serialnumber )
		return (1);
	if ( a->serialnumber < b->serialnumber )
		return (-1);

	if ( a->ccert == (Certificate * )0 ) {
		if ( b->ccert == (Certificate * )0 )
			ret = 0;
		else
			ret = 1;
	} else {
		if ( b->ccert == (Certificate * )0 )
			ret = -1;
		else
			ret = aux_cert_cmp(a->ccert, b->ccert);
	}

	if ( ret != 0 )
		return (ret);

	a = a->next;
	b = b->next;

	while ( a && b ) {
		if ( a->serialnumber > b->serialnumber )
			return (1);
		if ( a->serialnumber < b->serialnumber )
			return (-1);

		if ( a->ccert == (Certificate * )0 ) {
			if ( b->ccert == (Certificate * )0 )
				ret = 0;
			else
				ret = 1;
		} else {
			if ( b->ccert == (Certificate * )0 )
				ret = -1;
			else
				ret = aux_cert_cmp(a->ccert, b->ccert);
		}

		if ( ret != 0 )
			return (ret);

		a = a->next;
		b = b->next;
	}    /*while*/

	if ( a == (OCList * )0 ) {
		if ( b == (OCList * )0 )
			ret = 0;
		else
			ret = 1;
	} else /*  b = (OCList *)0  */
		ret = -1;

	return (ret);
}


/***************************************************************
 *
 * Procedure oclist_syntax
 *
 ***************************************************************/
#ifdef __STDC__

int oclist_syntax(
)

#else

int oclist_syntax(
)

#endif

{
	(void) add_attribute_syntax(
	    "OldCertificateList",
	    (IFP) oclist_enc,       (IFP) oclist_dec,
	    (IFP) str2ocl,          (IFP) printocl,
	    (IFP) aux_cpy_OCList,   (IFP) aux_oclist_cmp,
	    aux_free_OCList,        NULLCP,
	    NULLIFP, 	        TRUE);
}


/*******************************************************************************
 *
 *      Attribute Syntax for CRLEntry (revoked certificate according to PEM)
 *
 *******************************************************************************/



/***************************************************************
 *
 * Procedure crlentry_enc
 *
 ***************************************************************/
#ifdef __STDC__

PE crlentry_enc(
	CRLEntry	 *parm
)

#else

PE crlentry_enc(
	parm
)
CRLEntry	 *parm;

#endif

{
	PE pe;

	if ( parm == (CRLEntry * )0 )
		return (NULLPE);

	(void) build_SECAF_CRLEntry (&pe, 0, 0, NULLCP, parm);
	return (pe);
}


/***************************************************************
 *
 * Procedure crlentry_dec
 *
 ***************************************************************/
#ifdef __STDC__

CRLEntry *crlentry_dec(
	PE	  pe
)

#else

CRLEntry *crlentry_dec(
	pe
)
PE	  pe;

#endif

{
	CRLEntry * ret;
	int	     result;

	if ( pe == NULLPE )
		return ( (CRLEntry * )0 );

	result = parse_SECAF_CRLEntry (pe, 1, NULLIP, NULLVP, &ret);

	return (result ? (CRLEntry * )0 : ret);
}


/***************************************************************
 *
 * Procedure print_crlentry
 *
 ***************************************************************/
#ifdef __STDC__

RC print_crlentry(
	PS		  ps,
	CRLEntry	 *parm,
	int		  format
)

#else

RC print_crlentry(
	ps,
	parm,
	format
)
PS		  ps;
CRLEntry	 *parm;
int		  format;

#endif

{
	int       int_serial;
	char	* proc = "print_crlentry";

	/* Transform "OctetString" representation of serialnumber into "int" representation */
	int_serial = aux_OctetString2int(parm->serialnumber);
	if(int_serial < 0){
		aux_add_error(EINVALID, "aux_OctetString2int failed", CNULL, 0, proc);
		return(- 1);
	}
	ps_printf(ps, "%d#", int_serial);
	ps_printf(ps, "%s#", parm->revocationDate);
}


/***************************************************************
 *
 * Procedure aux_crlentry_cmp
 *
 ***************************************************************/
#ifdef __STDC__

int aux_crlentry_cmp(
	CRLEntry	 *a,
	CRLEntry	 *b
)

#else

int aux_crlentry_cmp(
	a,
	b
)
CRLEntry	 *a;
CRLEntry	 *b;

#endif

{
	/* It is assumed that a and b are contained in the same revocation list, 
	 * that is, have the same issuer.
	 */

	if ( a->serialnumber > b->serialnumber )
		return (1);
	if ( a->serialnumber < b->serialnumber )
		return (-1);

	return (0);
}


/*******************************************************************************
 *
 *      Attribute Syntax for SEQUENCE of Revoked Certificates
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure crlentryseq_enc
 *
 ***************************************************************/
#ifdef __STDC__

PE crlentryseq_enc(
	SEQUENCE_OF_CRLEntry	 *parm
)

#else

PE crlentryseq_enc(
	parm
)
SEQUENCE_OF_CRLEntry	 *parm;

#endif

{
	PE pe;

	(void) build_SECAF_CRLEntrySequence (&pe, 0, 0, NULLCP, parm);
	return (pe);
}


/***************************************************************
 *
 * Procedure crlentryseq_dec
 *
 ***************************************************************/
#ifdef __STDC__

SEQUENCE_OF_CRLEntry *crlentryseq_dec(
	PE	  pe
)

#else

SEQUENCE_OF_CRLEntry *crlentryseq_dec(
	pe
)
PE	  pe;

#endif

{
	SEQUENCE_OF_CRLEntry * ret;
	int	result;

	if ( pe == NULLPE )
		return ( (SEQUENCE_OF_CRLEntry * )0 );

	result = parse_SECAF_CRLEntrySequence (pe, 1, NULLIP, NULLVP, &ret);

	return (result ? (SEQUENCE_OF_CRLEntry * )0 : ret);
}


/*******************************************************************************
 *
 *      Attribute Syntax for CRL (revocation list according to PEM)
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure crl_enc
 *
 ***************************************************************/
#ifdef __STDC__

PE crl_enc(
	CRL	 *parm
)

#else

PE crl_enc(
	parm
)
CRL	 *parm;

#endif

{
	PE pe;

	if ( parm == (CRL * )0 )
		return (NULLPE);

	(void) build_SECAF_CRL (&pe, 0, 0, NULLCP, parm);
	return (pe);
}


/***************************************************************
 *
 * Procedure crl_dec
 *
 ***************************************************************/
#ifdef __STDC__

CRL *crl_dec(
	PE	  pe
)

#else

CRL *crl_dec(
	pe
)
PE	  pe;

#endif

{
	CRL * ret;
	int	result;

	if ( pe == NULLPE )
		return ( (CRL * )0 );

	result = parse_SECAF_CRL (pe, 1, NULLIP, NULLVP, &ret);

	return (result ? (CRL * )0 : ret);
}


/***************************************************************
 *
 * Procedure crltbs_enc
 *
 ***************************************************************/
#ifdef __STDC__

PE crltbs_enc(
	CRLTBS	 *parm
)

#else

PE crltbs_enc(
	parm
)
CRLTBS	 *parm;

#endif

{
	PE pe;

	(void) build_SECAF_TBSCRL (&pe, 0, 0, NULLCP, parm);
	return (pe);
}


/***************************************************************
 *
 * Procedure crltbs_dec
 *
 ***************************************************************/
#ifdef __STDC__

CRLTBS *crltbs_dec(
	PE	  pe
)

#else

CRLTBS *crltbs_dec(
	pe
)
PE	  pe;

#endif

{
	CRLTBS * ret;
	int	    result;

	if ( pe == NULLPE )
		return ( (CRLTBS * )0 );

	result = parse_SECAF_TBSCRL (pe, 1, NULLIP, NULLVP, &ret);

	return (result ? (CRLTBS * )0 : ret);
}


/***************************************************************
 *
 * Procedure str2crl
 *
 ***************************************************************/
#ifdef __STDC__

CRL *str2crl(
	char	 *str
)

#else

CRL *str2crl(
	str
)
char	 *str;

#endif

{
	CRL		 * result;
	SEQUENCE_OF_CRLEntry 	 * seq;
	SEQUENCE_OF_CRLEntry   * save_seq;
	struct alg_id 		   alg;
	char			 * ptr;
	int			   first_elem = 1;
	PE 			   pe;
	DN			   dn;
	OIDentifier		 * oid;
	int		           int_serial;
	char			 * proc = "str2crl";

	/*    OID = ObjId*
	 *    s. /isode/share/include/psap.h
 	 */

	/*    dn_enc() from  /isode/share/src/dsap/common/dn_str.c
	 */

	if ( !(result = (CRL * )malloc(sizeof(CRL))) )
		return( (CRL * )0 );

	result->tbs_DERcode = (OctetString * )0;

	result->tbs = (CRLTBS * )malloc(sizeof(CRLTBS));
	if (!result->tbs)
		return ((CRL * ) 0);

	result->sig = (Signature * )malloc(sizeof(Signature));
	if (!result->sig)
		return( (CRL * )0 );

	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("Algorithm not present", NULLCP);
		free( (CRL * )result );
		return( (CRL * )0 );
	}
	*ptr = '\0';
	ptr++;

	oid = name2oid(str);
	if ( oid == NULLOID) {
		parse_error("Bad algorithm identifier", NULLCP);
		free( (CRL * )result );
		return( (CRL * )0 );
	}

	alg.algorithm = oid;

	str = ptr;
	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("Parameters not present", NULLCP);
		free( (CRL * )result );
		return( (CRL * )0 );
	}
	*ptr = '\0';
	ptr++;

	str2alg(str, &alg);
	encode_AF_AlgorithmIdentifier(&pe, 0, 0, NULLCP, &alg);
	result->sig->signAI = AlgId_dec(pe);

	str = ptr;
	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("Signature not present", NULLCP);
		aux_free_AlgId( &(result->sig->signAI) );
		free( (CRL * )result );
		return( (CRL * )0 );
	}
	*ptr = '\0';
	ptr++;

	str2encrypted (str, &(result->sig->signature.bits), &(result->sig->signature.nbits));

	str = ptr;
	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("Algorithm not present", NULLCP);
		aux_free_Signature( &(result->sig) );
		free( (CRL * )result );
		return( (CRL * )0 );
	}
	*ptr = '\0';
	ptr++;

	oid = name2oid(str);
	if (oid == NULLOID ) {
		parse_error("Bad algorithm identifier", NULLCP);
		aux_free_Signature( &(result->sig) );
		free( (CRL * )result );
		return( (CRL * )0 );
	}

	alg.algorithm = oid;

	str = ptr;
	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("Parameters not present", NULLCP);
		aux_free_Signature( &(result->sig) );
		free( (CRL * )result );
		return( (CRL * )0 );
	}
	*ptr = '\0';
	ptr++;

	str2alg(str, &alg);
	encode_AF_AlgorithmIdentifier(&pe, 0, 0, NULLCP, &alg);
	result->tbs->signatureAI = AlgId_dec(pe);

	str = ptr;
	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("Issuer not present", NULLCP);
		aux_free_AlgId( &(result->tbs->signatureAI) );
		aux_free_Signature( &(result->sig) );
		free( (CRL * )result );
		return( (CRL * )0 );
	}
	*ptr = '\0';
	ptr++;

	dn = str2dn(str);
	pe = dn_enc(dn);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &result->tbs->issuer);
	pe_free(pe);
	dn_free(dn);

	str = ptr;
	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("Lastupdate time not present", NULLCP);
		aux_free_DName( &(result->tbs->issuer) );
		aux_free_AlgId( &(result->tbs->signatureAI) );
		aux_free_Signature( &(result->sig) );
		free( (CRL * )result );
		return( (CRL * )0 );
	}
	*ptr = '\0';
	ptr++;

	result->tbs->lastUpdate = aux_cpy_String(str);

	str = ptr;
	ptr = strchr(str, '#');
	if ( ptr == NULLCP ) {
		parse_error("Nextupdate time not present", NULLCP);
		aux_free_DName( &(result->tbs->issuer) );
		aux_free_AlgId( &(result->tbs->signatureAI) );
		aux_free_Signature( &(result->sig) );
		free( (CRL * )result );
		return( (CRL * )0 );
	}
	*ptr = '\0';
	ptr++;
	/* This may be the end of the string */

	result->tbs->nextUpdate = aux_cpy_String(str);

	while ( str = ptr, ((ptr = strchr(str, '#')) != NULLCP) ) {

		*ptr = '\0';
		ptr++;

		if ( !(seq = (SEQUENCE_OF_CRLEntry * )malloc(sizeof(SEQUENCE_OF_CRLEntry))) ) {
			free( (char *)result->tbs->lastUpdate );
			aux_free_DName( &(result->tbs->issuer) );
			aux_free_AlgId( &(result->tbs->signatureAI) );
			aux_free_Signature( &(result->sig) );
			free( (CRL * )result );
			return( (CRL * )0 );
		}

		if ( first_elem ) {
			result->tbs->revokedCertificates = seq;
			first_elem = 0;
		} else
			save_seq->next = seq;

		save_seq = seq;
		seq->next = (SEQUENCE_OF_CRLEntry * )0;

		if ( !(seq->element = (CRLEntry * )malloc(sizeof(CRLEntry))) ) {
			aux_free_CRL(&result);
			return( (CRL * )0 );
		}

		int_serial = atoi(str);
		seq->element->serialnumber = aux_int2OctetString(int_serial);
		if(!seq->element->serialnumber){
			aux_add_error(EINVALID, "aux_int2OctetString failed", CNULL, 0, proc);
			aux_free_CRL(&result);
			return( (CRL * )0 );
		}

		str = ptr;
		ptr = strchr(str, '#');
		if ( ptr == NULLCP ) {
			parse_error("Revocation date not present", NULLCP);
			free( (CRLEntry * )seq->element );
			seq->element = (CRLEntry * ) 0;
			aux_free_CRL(&result);
			return( (CRL * )0 );
		}
		*ptr = '\0';
		ptr++;
		/* This may be the end of the string */

		seq->element->revocationDate = aux_cpy_String(str);

	}    /* while */

	if ( first_elem ) {		
		/* Black list is empty, otherwise first_elem would have been set to null! */
		result->tbs->revokedCertificates = (SEQUENCE_OF_CRLEntry * )0;
	}

	return (result);
}


/* The  *tbs_DERcode-component within CRL is constructed in the
	 * DECODER-parse-section of pepsy:
	 *
	 *      parm->tbs_DERcode = aux_PE2OctetString($$)                                
	 */


/* The  *tbs_DERcode-component within CRLEntry is constructed in 
	 * the DECODER-parse-section of pepsy:
	 *
	 *	parm->tbs_DERcode = aux_PE2OctetString($$)
	 */


/***************************************************************
 *
 * Procedure printcrl
 *
 ***************************************************************/
#ifdef __STDC__

RC printcrl(
	PS	  ps,
	CRL	 *parm,
	int	  format
)

#else

RC printcrl(
	ps,
	parm,
	format
)
PS	  ps;
CRL	 *parm;
int	  format;

#endif

{
	struct alg_id 		* alg;
	PE 			  pe;
	DN 			  dn;
	SEQUENCE_OF_CRLEntry  * seq;
	int			  result;
	char		  	* proc = "printcrl";

	/*  Abfrage in calling routine
 	 */

	pe = AlgId_enc(parm->sig->signAI);
	result = decode_AF_AlgorithmIdentifier (pe, 0, NULLIP, NULLVP, &alg);
	print_algid(ps, alg, format);
	pe_free(pe);
	free( (struct alg_id *)alg );
	alg = (struct alg_id *) 0;

	print_encrypted(ps, parm->sig->signature.bits, parm->sig->signature.nbits, 
	    format);

	pe = AlgId_enc(parm->tbs->signatureAI);
	result = decode_AF_AlgorithmIdentifier (pe, 0, NULLIP, NULLVP, &alg);
	print_algid(ps, alg, format);
	pe_free(pe);
	free( (struct alg_id *)alg );
	alg = (struct alg_id *) 0;

	build_SECIF_Name(&pe, 1, 0, NULLCP, parm->tbs->issuer);
	dn = dn_dec(pe);
	dn_print(ps, dn, EDBOUT);
	ps_printf(ps, "#");
	pe_free(pe);
	dn_free(dn);

	ps_printf(ps, "%s#", parm->tbs->lastUpdate);
	ps_printf(ps, "%s#", parm->tbs->nextUpdate);

	seq = parm->tbs->revokedCertificates;

	while ( seq ) {
		result = print_crlentry(ps, seq->element, format);
		if(result < 0){
			AUX_ADD_ERROR;
			return(- 1);
		}
		seq = seq->next;
	}
}


/***************************************************************
 *
 * Procedure aux_crl_cmp
 *
 ***************************************************************/
#ifdef __STDC__

int aux_crl_cmp(
	CRL	 *a,
	CRL	 *b
)

#else

int aux_crl_cmp(
	a,
	b
)
CRL	 *a;
CRL	 *b;

#endif

{
	int ret;

	ret = aux_cmp_DName(a->tbs->issuer, b->tbs->issuer);
	if ( ret != 0 )
		return (ret);

	return(strcmp(a->tbs->lastUpdate, b->tbs->lastUpdate));
}


/***************************************************************
 *
 * Procedure crl_syntax
 *
 ***************************************************************/
#ifdef __STDC__

int crl_syntax(
)

#else

int crl_syntax(
)

#endif

{
	(void) add_attribute_syntax(
	    "pemCRL_syntax",
	    (IFP) crl_enc,       (IFP) crl_dec,
	    (IFP) str2crl,       (IFP) printcrl,
	    (IFP) aux_cpy_CRL ,  (IFP) aux_crl_cmp,
	    NULLIFP, 		  NULLCP,
	    NULLIFP, 	          TRUE);
}


#else


/***************************************************************
 *
 * Procedure af_sntx_dummy
 *
 ***************************************************************/
#ifdef __STDC__

int af_sntx_dummy(
)

#else

int af_sntx_dummy(
)

#endif

{
	return(0);
}

#endif

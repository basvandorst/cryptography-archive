/* prim2num.c - presentation element to integer */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/prim2num.c,v 10.0 1993/02/18 20:13:47 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/prim2num.c,v 10.0 1993/02/18 20:13:47 awy Rel $
 *
 *
 * $Log: prim2num.c,v $
 * Revision 10.0  1993/02/18  20:13:47  awy
 * Release IC-R1
 *
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <stdio.h>
#include "asn1/asn1.h"

/*  */

integer	prim2num (pe)
register PE	pe;
{
    register integer    i;
    register PElementData dp,
			  ep;

    if (pe -> pe_form != PE_FORM_PRIM || pe -> pe_prim == NULLPED)
	return pe_seterr (pe, PE_ERR_PRIM, NOTOK);
    if (pe -> pe_len > sizeof (i))
	return pe_seterr (pe, PE_ERR_OVER, NOTOK);

    pe -> pe_errno = PE_ERR_NONE;/* in case integer is NOTOK-valued */
    dp = pe -> pe_prim;
    if (pe -> pe_len > 1 && ((*dp == 0 && ((*(dp+1)) & 0x80) == 0) ||
	(*dp == 0xff && ((*(dp + 1)) & 0x80) == 0x80)))
	return pe_seterr (pe, PE_ERR_SYNTAX, NOTOK);
    i = ((*dp) & 0x80) ? (-1) : 0;
    for (ep = dp + pe -> pe_len; dp < ep;)
	i = (i << 8) | (*dp++ & 0xff);

    return i;
}

/* set_add.c - add member to set */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/set_add.c,v 10.0 1993/02/18 20:14:30 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/set_add.c,v 10.0 1993/02/18 20:14:30 awy Rel $
 *
 *
 * $Log: set_add.c,v $
 * Revision 10.0  1993/02/18  20:14:30  awy
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

int	set_add (pe, r)
register PE	pe,
		r;
{
    register int     pe_id;
    register PE	    *p,
		    q;

    if (r == NULLPE)
	return pe_seterr (pe, PE_ERR_NMEM, NOTOK);

    pe_id = PE_ID (r -> pe_class, r -> pe_id);
    for (p = &pe -> pe_cons; q = *p; p = &q -> pe_next)
	if (PE_ID (q -> pe_class, q -> pe_id) == pe_id)
	    return pe_seterr (pe, PE_ERR_DUPLICATE, NOTOK);

    *p = r;
    return OK;
}

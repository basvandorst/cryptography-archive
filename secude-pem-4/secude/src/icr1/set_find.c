/* set_find.c - find member of a set */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/set_find.c,v 10.0 1993/02/18 20:14:35 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/set_find.c,v 10.0 1993/02/18 20:14:35 awy Rel $
 *
 *
 * $Log: set_find.c,v $
 * Revision 10.0  1993/02/18  20:14:35  awy
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

PE	set_find (pe, class, id)
register PE	pe;
register PElementClass class;
register PElementID id;
{
    register int    pe_id;
    register PE	    p;
    PE pfound = NULLPE;

    pe_id = PE_ID (class, id);
    for (p = pe -> pe_cons; p; p = p -> pe_next)
	if (PE_ID (p -> pe_class, p -> pe_id) == pe_id) {
	    if (pfound == NULLPE)
		pfound = p;
	    else 
		return pe_seterr (pe, PE_ERR_DUPLICATE, NULLPE);
	}
    return pfound;
}

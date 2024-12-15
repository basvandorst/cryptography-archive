/* bit2prim.c - bit string to presentation element */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/bit2prim.c,v 10.0 1993/02/18 20:12:40 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/bit2prim.c,v 10.0 1993/02/18 20:12:40 awy Rel $
 *
 *
 * $Log: bit2prim.c,v $
 * Revision 10.0  1993/02/18  20:12:40  awy
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


static PE	bit2prim_aux ();

/*  */

PE	bit2prim (pe)
register PE	pe;
{
    if (pe == NULLPE)
	return NULLPE;

    switch (pe -> pe_form) {
	case PE_FORM_PRIM:
	    if (pe -> pe_prim == NULLPED) {
		if ((pe -> pe_prim = PEDalloc (1)) == NULLPED)
		    return NULLPE;
		pe -> pe_len = 1;
		pe -> pe_nbits = 0;
	    }
	    /* and fall */

	case PE_FORM_CONS:
	    if (bit2prim_aux (pe) == NULLPE)
		return NULLPE;
	    break;
    }

    return pe;
}

/*  */

static PE  bit2prim_aux (pe)
register PE	pe;
{
    int	    i;
    register PE	    p;

    if (pe == NULLPE)
	return NULLPE;

    switch (pe -> pe_form) {
	case PE_FORM_PRIM:
	    if (pe -> pe_prim && pe -> pe_len) {
		if ((i = (((pe -> pe_len - 1) * 8) - pe -> pe_nbits)) > 7)
		    return pe_seterr (pe, PE_ERR_BITS, NULLPE);
		pe -> pe_prim[0] = i & 0xff;

	    }
	    break;

	case PE_FORM_CONS:
	    for (p = pe -> pe_cons; p; p = p -> pe_next)
		if (bit2prim (p) == NULLPE)
		    return NULLPE;
	    break;
    }

    return pe;
}

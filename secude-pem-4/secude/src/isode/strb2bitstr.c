/* strb2bitstr.c - string of bits to bit string */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/strb2bitstr.c,v 10.2 1993/07/27 07:00:04 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/strb2bitstr.c,v 10.2 1993/07/27 07:00:04 awy Rel $
 *
 *
 * $Log: strb2bitstr.c,v $
 * Revision 10.2  1993/07/27  07:00:04  awy
 * Update IC-R1p0v5
 *
 * Revision 10.1  1993/06/01  17:07:43  awy
 * Avoid referencing past end of string.
 *
 * Revision 10.0  1993/02/18  20:14:51  awy
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

PE	strb2bitstr (cp, len, class, id)
register char  *cp;
register int     len;
PElementClass class;
PElementID id;
{
    register int    i,
		    j,
                    bit,
		    mask;
    register PE	    p;

    if ((p = pe_alloc (class, PE_FORM_PRIM, id)) == NULLPE)
	return NULLPE;

    p = prim2bit (p);
    if (len > 0 && bit_off (p, len - 1) == NOTOK) {
no_mem: ;
        pe_free (p);
        return NULLPE;
    }

    for (bit = (*cp & 0xff), i = 0, mask = 1 << (j = 7); i < len; /* i++ */) {
	if ((bit & mask) && bit_on (p, i) == NOTOK)
	    goto no_mem;
	if ((++i < len) && (j-- == 0))
	    bit = *++cp & 0xff, mask = 1 << (j = 7);
	else
	    mask >>= 1;
    }

    return p;
}

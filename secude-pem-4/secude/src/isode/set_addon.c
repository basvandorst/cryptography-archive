/* set_addon.c - add member to end of a set */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/set_addon.c,v 10.0 1993/02/18 20:14:32 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/set_addon.c,v 10.0 1993/02/18 20:14:32 awy Rel $
 *
 *
 * $Log: set_addon.c,v $
 * Revision 10.0  1993/02/18  20:14:32  awy
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

int	set_addon (pe, last, new)
PE	pe,
        last,
        new;
{
    if (pe == NULLPE)
	return NOTOK;
    if (last == NULLPE)
	return set_add (pe, new);
    new -> pe_offset = pe -> pe_cardinal++;
    last -> pe_next = new;

    return OK;
}

/* oid_cpy.c - copy an object identifier */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/oid_cpy.c,v 10.0 1993/02/18 20:13:13 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/oid_cpy.c,v 10.0 1993/02/18 20:13:13 awy Rel $
 *
 *
 * $Log: oid_cpy.c,v $
 * Revision 10.0  1993/02/18  20:13:13  awy
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

OID	oid_cpy (q)
register OID q;
{
    register unsigned int   i,
			   *ip,
			   *jp;
    register OID	oid;

    if (q == NULLOID)
	return NULLOID;
    if ((i = q -> oid_nelem) < 1)
	return NULLOID;
    if ((oid = (OID) malloc (sizeof *oid)) == NULLOID)
	return NULLOID;

    if ((ip = (unsigned int *) malloc ((unsigned) (i + 1) * sizeof *ip))
	    == NULL) {
	free ((char *) oid);
	return NULLOID;
    }

    oid -> oid_elements = ip, oid -> oid_nelem = i;

    for (i = 0, jp = q -> oid_elements; i < oid -> oid_nelem; i++, jp++)
	*ip++ = *jp;

    return oid;
}

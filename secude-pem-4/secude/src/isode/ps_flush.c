/* ps_flush.c - flush a presentation stream */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/ps_flush.c,v 10.0 1993/02/18 20:14:04 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/ps_flush.c,v 10.0 1993/02/18 20:14:04 awy Rel $
 *
 *
 * $Log: ps_flush.c,v $
 * Revision 10.0  1993/02/18  20:14:04  awy
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

int	ps_flush (ps)
register PS	ps;
{
    if (ps -> ps_flushP)
	return (*ps -> ps_flushP) (ps);

    return OK;
}

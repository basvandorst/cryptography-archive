/* ps_prime.c - prime a presentation stream */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/ps_prime.c,v 10.0 1993/02/18 20:14:11 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/ps_prime.c,v 10.0 1993/02/18 20:14:11 awy Rel $
 *
 *
 * $Log: ps_prime.c,v $
 * Revision 10.0  1993/02/18  20:14:11  awy
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

int	ps_prime (ps, waiting)
register PS	ps;
int	waiting;
{
    if (ps -> ps_primeP)
	return (*ps -> ps_primeP) (ps, waiting);

    return OK;
}

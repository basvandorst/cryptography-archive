/* qb_free.c - free a list of qbufs */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/qb_free.c,v 10.0 1993/02/18 20:14:15 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/qb_free.c,v 10.0 1993/02/18 20:14:15 awy Rel $
 *
 *
 * $Log: qb_free.c,v $
 * Revision 10.0  1993/02/18  20:14:15  awy
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

int	qb_free (qb)
register struct qbuf *qb;
{
    QBFREE (qb);

    free ((char *) qb);
}

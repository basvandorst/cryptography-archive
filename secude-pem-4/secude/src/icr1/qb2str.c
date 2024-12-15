/* qb2str.c - qbuf to string */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/compat/RCS/qb2str.c,v 10.0 1993/02/18 20:22:16 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/compat/RCS/qb2str.c,v 10.0 1993/02/18 20:22:16 awy Rel $
 *
 *
 * $Log: qb2str.c,v $
 * Revision 10.0  1993/02/18  20:22:16  awy
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
#include "compat/manifest.h"
#include "compat/general.h"

/*  */

char   *qb2str (q)
register struct qbuf *q;
{
    register int    len;
    register char  *b,
                   *d;
    register struct qbuf   *p;

    p = q -> qb_forw, len = 0;
    do {
	len += p -> qb_len;

	p = p -> qb_forw;
    }
    while (p != q);
    q -> qb_len = len;

    if ((b = d = malloc ((unsigned) (len + 1))) == NULL)
	return NULLCP;

    p = q -> qb_forw;
    do {
	bcopy (p -> qb_data, d, p -> qb_len);
	d += p -> qb_len;

	p = p -> qb_forw;
    }
    while (p != q);
    *d = NULL;

    return b;
}

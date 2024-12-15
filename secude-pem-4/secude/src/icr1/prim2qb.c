/* prim2qb.c - presentation element to qbuf */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/prim2qb.c,v 10.0 1993/02/18 20:13:51 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/prim2qb.c,v 10.0 1993/02/18 20:13:51 awy Rel $
 *
 *
 * $Log: prim2qb.c,v $
 * Revision 10.0  1993/02/18  20:13:51  awy
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

struct qbuf *prim2qb (pe)
register PE    pe;
{
    register struct qbuf *qb,
		         *qp;
    register PE	    p;
    register PElementClass class;
    register PElementID id;

    if ((qb = (struct qbuf *) malloc (sizeof *qb)) == NULL)
	return pe_seterr (pe, PE_ERR_NMEM, (struct qbuf *) NULL);
    qb -> qb_forw = qb -> qb_back = qb;
    qb -> qb_data = NULL, qb -> qb_len = 0;
	
    switch (pe -> pe_form) {
	case PE_FORM_PRIM:
	    if ((qp = str2qb ((char *) pe -> pe_prim, (int) pe -> pe_len, 0))
		    == NULL) {
		pe -> pe_errno = PE_ERR_NMEM;
		goto out;
	    }
	    qb -> qb_len = qp -> qb_len;
	    insque (qp, qb);
	    break;

	case PE_FORM_CONS:
	    if ((p = pe -> pe_cons) == NULLPE)
		break;
	    class = p -> pe_class, id = p -> pe_id;
	    for (p = pe -> pe_cons; p; p = p -> pe_next) {
		register struct qbuf *qpp,
				     *qbp;

		if ((p -> pe_class != class || p -> pe_id != id)
		        && (p -> pe_class != PE_CLASS_UNIV
				|| p -> pe_id != PE_PRIM_OCTS)) {
		    pe -> pe_errno = PE_ERR_TYPE;
		    goto out;
		}
		if ((qp = prim2qb (p)) == NULL) {
		    pe -> pe_errno = p -> pe_errno;
		    goto out;
		}

		for (qpp = qp -> qb_forw; qpp != qp; qpp = qbp) {
		    qbp = qpp -> qb_forw;

		    remque (qpp);
		    insque (qpp, qb -> qb_back);

		    qb -> qb_len += qpp -> qb_len;
		}
		free ((char *) qp);
	    }
	    break;
    }

    return qb;

out: ;
    qb_free (qb);

    return NULL;
}

/* pe_free.c - free a presentation element */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/pe_free.c,v 10.0 1993/02/18 20:13:36 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/pe_free.c,v 10.0 1993/02/18 20:13:36 awy Rel $
 *
 *
 * $Log: pe_free.c,v $
 * Revision 10.0  1993/02/18  20:13:36  awy
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
#ifdef DEBUG
#include "compat/tailor.h"
#endif

/*  */

int	pe_free (pe)
register PE	pe;
{
    register PE	    p,
		    q;
    register struct qbuf *qb,
                         *qp;

    if (!pe)
        abort ();

    if( pe->pe_refcnt < 0) {
#ifdef DEBUG
      DLOG (psap_log, LLOG_DEBUG,
	    ("WARNING: duplicately free'd pe 0x%x!", pe));
#endif
      return;
    }

    if (pe -> pe_refcnt-- > 0)
	return;

    switch (pe -> pe_form) {
	case PE_FORM_PRIM: 
	case PE_FORM_ICONS: 
	    if (pe -> pe_prim && !pe -> pe_inline)
		PEDfree (pe -> pe_prim);
            else
		if (pe -> pe_inline && Hqb) {
		    if(Fqb && (--Qbrefs == 0)) {
			for (qb = Fqb; qb && (qb != Hqb); qb = qp) {
			    qp = qb -> qb_forw;
			    free ((char *) qb);
			}
			if (!qb)
			    abort ();
			Fqb = Hqb = NULL;
		    }
		}
	    break;

	case PE_FORM_CONS: 
	    for (p = pe -> pe_cons; p; p = q) {
		q = p -> pe_next;
		pe_free (p);
	    }
	    break;

        default:
            abort();
	    /* NOTREACHED */
    }

    if (pe -> pe_realbase)
	free (pe -> pe_realbase);
	
    pe_frees++;
    pe -> pe_next = pe_list;
    pe_list = pe;
#ifdef	DEBUG
    if (psap_log -> ll_events & LLOG_DEBUG) {
	PE     *pep;

	for (pep = &pe_active; p = *pep; pep = &p -> pe_link)
	    if (p == pe) {
		*pep = p -> pe_link;
		break;
	    }
	if (!p)
	    DLOG (psap_log, LLOG_DEBUG,
		  ("WARNING: free'd pe (0x%x) not on active list", pe));
    }
#endif
}

/* prim2str.c - presentation element to octet string */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/prim2str.c,v 10.0 1993/02/18 20:13:56 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/prim2str.c,v 10.0 1993/02/18 20:13:56 awy Rel $
 *
 *
 * $Log: prim2str.c,v $
 * Revision 10.0  1993/02/18  20:13:56  awy
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

/* Similar to pe_pullup.  Returns a newly allocated string, composed of
   of any sub-elements in pe, whereas pe_pullup always reverts "pe" to
   a primitive.  The string is null-terminated, though pe_len specifically
   does NOT reflect this. */

char   *prim2str (pe, len)
register PE	pe;
register int   *len;
{
    register int    i,
                    k;
    int     j;
    register char  *dp,
                   *ep,
                   *fp;
    register PElementClass class;
    register PElementID id;
    register PE	    p;

    *len = 0;
    switch (pe -> pe_form) {
	case PE_FORM_PRIM: 
	    if ((dp = malloc ((unsigned) ((i = pe -> pe_len) + 1))) == NULLCP)
		return pe_seterr (pe, PE_ERR_NMEM, NULLCP);
	    bcopy ((char *) pe -> pe_prim, dp, i);
	    break;

	case PE_FORM_CONS: 
	    if ((p = pe -> pe_cons) == NULLPE) {
		if ((dp = malloc ((unsigned) ((i = 0) + 1))) == NULLCP)
		    return pe_seterr (pe, PE_ERR_NMEM, NULLCP);
		break;
	    }
	    dp = NULLCP, i = 0;
	    class = p -> pe_class, id = p -> pe_id;
	    for (p = pe -> pe_cons; p; p = p -> pe_next) {
		if ((p -> pe_class != class || p -> pe_id != id)
		        && (p -> pe_class != PE_CLASS_UNIV
			    	|| p -> pe_id != PE_PRIM_OCTS)) {
		    if (dp)
			free (dp);
		    return pe_seterr (pe, PE_ERR_TYPE, NULLCP);
		}

		if ((ep = prim2str (p, &j)) == NULLCP) {
		    if (dp)
			free (dp);
		    return pe_seterr (pe, PE_ERR_NMEM, NULLCP);
		}
		if (dp) {
		    if ((fp = realloc (dp, (unsigned) ((k = i + j) + 1)))
			    == NULLCP) {
			free (dp);
			return pe_seterr (pe, PE_ERR_NMEM, NULLCP);
		    }
		    bcopy (ep, fp + i, j);
		    dp = fp, i = k;
		}
		else
		    dp = ep, i += j;
	    }
	    break;
    }

    if (dp)
	dp[*len = i] = NULL;

    return dp;
}

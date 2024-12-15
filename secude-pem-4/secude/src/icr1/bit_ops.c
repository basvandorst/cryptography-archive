/* bit_opts.c - operations on bit strings */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/bit_ops.c,v 10.0 1993/02/18 20:12:42 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/bit_ops.c,v 10.0 1993/02/18 20:12:42 awy Rel $
 *
 *
 * $Log: bit_ops.c,v $
 * Revision 10.0  1993/02/18  20:12:42  awy
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


/* the first octet indicates how many unused bits are in the last octet --

	prim2bit  -	presentation element to bit string
	bit2prim  -	bit string to presentation element
	bit_on	  -	turn a bit ON
	bit_off	  -	turn a bit OFF
	bit_test  -	test a bit
 */


static PElementData ffb (), ffb_aux ();
static PE	ffb_pe ();

/*  */

int	bit_on (pe, i)
register PE	pe;
register int	i;
{
    int	    mask;
    register PElementData bp;

    if ((bp = ffb (pe, i, &mask, 1)) == NULLPED)
	return pe_seterr (pe, PE_ERR_NMEM, NOTOK);

    *bp |= mask;

    return OK;
}

/*  */

int	bit_off (pe, i)
register PE	pe;
register int	i;
{
    int	    mask;
    register PElementData bp;

    if ((bp = ffb (pe, i, &mask, 1)) == NULLPED)
	return pe_seterr (pe, PE_ERR_NMEM, NOTOK);

    *bp &= ~mask;

    return OK;
}

/*  */

int	bit_test (pe, i)
register PE	pe;
register int	i;
{
    int	    mask;
    register PElementData bp;

    if ((bp = ffb (pe, i, &mask, 0)) == NULLPED)
	return pe_seterr (pe, PE_ERR_BIT, NOTOK);

    return (*bp & mask ? 1 : 0);
}

/*  */

static PElementData ffb (pe, n, mask, xtnd)
register PE	pe;
register int	n,
	       *mask,
		xtnd;
{
    register int    len,
		    i;
    int     j;
    register PElementData bp;
    register PE    *p,
		    q,
		    r;

    i = (j = n) / 8 + 1;
    if ((bp = ffb_aux (pe, &j, mask)) != NULLPED || !xtnd)
	return bp;

    if (pe -> pe_form == PE_FORM_CONS)
	pe = ffb_pe (pe);

    switch (pe -> pe_form) {
	case PE_FORM_PRIM: 
	    if (pe -> pe_len < (PElementLen) (len = i + 1)) {
		if ((bp = PEDalloc (len)) == NULLPED)
		    return NULLPED;
		bzero ((char *) bp, len);
		if (pe -> pe_prim) {
		    PEDcpy (pe -> pe_prim, bp, pe -> pe_len);
		    if (pe -> pe_inline)
			pe -> pe_inline = 0;
		    else
			PEDfree (pe -> pe_prim);
		}
		pe -> pe_prim = bp, pe -> pe_len = len;
	    }
	    pe -> pe_nbits = n + 1;
	    *mask = 1 << (7 - (n % 8));
	    return (pe -> pe_prim + i);

	case PE_FORM_CONS: 
	    if ((r = pe_alloc (pe -> pe_class, PE_FORM_PRIM, pe -> pe_id))
		    == NULLPE)
		return NULLPED;
	    if ((r -> pe_prim = PEDalloc (len = r -> pe_len = j / 8 + 2))
		    == NULLPED) {
		pe_free (r);
		return NULLPED;
	    }
	    bzero ((char *) r -> pe_prim, len);
	    r -> pe_nbits = j + 1;
	    *mask = 1 << (7 - (j % 8));
	    for (p = &pe -> pe_cons; q = *p; p = &q -> pe_next)
		continue;
	    *p = r;
	    return (r -> pe_prim + len - 1);

	default:
	    return NULLPED;
    }
}

/*  */

static PElementData ffb_aux (pe, n, mask)
register PE	pe;
register int   *n,
	       *mask;
{
    register int    i,
		    nbits;
    register PElementData bp;
    register PE	    p;

    switch (pe -> pe_form) {
	case PE_FORM_PRIM: 
	    if ((nbits = pe -> pe_nbits) > (i = *n)) {
		*mask = 1 << (7 - (i % 8));
		return (pe -> pe_prim + i / 8 + 1);
	    }
	    *n -= nbits;
	    break;

	case PE_FORM_CONS: 
	    for (p = pe -> pe_cons; p; p = p -> pe_next)
		if ((bp = ffb_aux (p, n, mask)) != NULLPED)
		    return bp;
	    break;
    }

    return NULLPED;
}

/*  */

static PE	ffb_pe (pe)
register PE	pe;
{
    register PE	    p,
		    q;

    for (p = pe -> pe_cons, q = NULLPE; p; q = p, p = p -> pe_next)
	continue;

    if (q != NULLPE)
	switch (q -> pe_form) {
	    case PE_FORM_PRIM: 
		return q;

	    case PE_FORM_CONS: 
		return ffb_pe (q);
	}

    return pe;
}

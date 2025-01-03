/* pe_alloc.c - allocate a presentation element */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/pe_alloc.c,v 10.0 1993/02/18 20:13:26 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/pe_alloc.c,v 10.0 1993/02/18 20:13:26 awy Rel $
 *
 *
 * $Log: pe_alloc.c,v $
 * Revision 10.0  1993/02/18  20:13:26  awy
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


/* A Presentation Element (or PElement) is an internal representation for
   a presentation type from ISO8825.  The fields of the structure are:

	pe_class:	UNIVersal, APPLication, CONText, or PRIVate
	pe_form:	PRIMative, CONStructor, InlineCONStructor
	pe_id:		identifier

	pe_len:		if a PRIMative, then the length of pe_prim,
			else a scratch value; "indefinite" length elements
			have a pe_len of -1 (PE_LEN_INDF)
	pe_ilen:	if an InlineCONStructor, then the offset to the real
			data portion

	pe_prim:	if a PRIMative or an Inline CONStructor, the
			byte-string
	pe_cons:	if a CONStructor, the first element in the
			singly-linked list of elements

	pe_next:	if the immediate parent is a constructor, the
			next element in the singly-linked list of elements

	pe_cardinal:	if a LIST (SET or SEQ CONStructor), the cardinality
			of the list
	pe_offset:	if a member of a SEQ LIST, the offset in the SEQUENCE

	pe_nbits:	if a BITSTRING, the number of bits in the string

	pe_refcnt:	a hack for ANYs in pepy
 */

#define PE_LIST_CNT 100


int	pe_allocs;
int	pe_frees;
int	pe_most;

PE	pe_list = NULLPE;
#ifdef	DEBUG
PE	pe_active = NULLPE;
#endif

/*  */

PE	pe_alloc (class, form, id)
PElementClass class;
PElementForm  form;
PElementID id;
{
    register int    i;
    register PE	    pe;

    if (pe = pe_list)
        pe_list = pe -> pe_next;
    else {
        pe_list = (pe = (PE) calloc (PE_LIST_CNT, sizeof *pe));
        if (pe == NULLPE)
	    return NULLPE;

        for (i = 0; i < (PE_LIST_CNT - 1); i++, pe++)
	    pe -> pe_next = pe + 1;

        pe = pe_list;
        pe_list = pe -> pe_next;
    }

    bzero ((char *)pe, sizeof *pe);
    pe -> pe_class = class;
    pe -> pe_form = form;
    pe -> pe_id = id;

    if ((i = ++pe_allocs - pe_frees) > pe_most)
        pe_most = i;
#ifdef	DEBUG
    if (psap_log -> ll_events & LLOG_DEBUG) {
	pe -> pe_link = pe_active;
	pe_active = pe;
    }
#endif

    return pe;
}

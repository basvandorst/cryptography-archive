/* pe_error.c - presentation element error to string */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/pe_error.c,v 10.0 1993/02/18 20:13:31 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/pe_error.c,v 10.0 1993/02/18 20:13:31 awy Rel $
 *
 *
 * $Log: pe_error.c,v $
 * Revision 10.0  1993/02/18  20:13:31  awy
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

static char *pe_errorlist[] = {
    "Error 0",
    "Overflow",
    "Out of memory",
    "No such bit",
    "Malformed universal timestring",
    "Malformed generalized timestring",
    "No such member",
    "Not a primitive form",
    "Not a constructor form",
    "Class/ID mismatch in constructor",
    "Malformed object identifier",
    "Malformed bitstring",
    "Type not supported",
    "Signed integer not expected",
    "Duplicate member of set",
    "Syntax coding error",
};

static int pe_maxerror = sizeof pe_errorlist / sizeof pe_errorlist[0];

/*  */

char   *pe_error (c)
int	c;
{
    register char  *bp;
    static char buffer[30];

    if (c < pe_maxerror && (bp = pe_errorlist[c]))
	return bp;

    (void) sprintf (buffer, "Error %d", c);
    return buffer;
}

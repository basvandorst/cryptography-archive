/* pl_tables.c - tables for presentation lists */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/pl_tables.c,v 10.0 1993/02/18 20:13:28 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/pl_tables.c,v 10.0 1993/02/18 20:13:28 awy Rel $
 *
 *
 * $Log: pl_tables.c,v $
 * Revision 10.0  1993/02/18  20:13:28  awy
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

/*    DATA */

char   *pe_classlist[] = {
    "UNIV",
    "APPL",
    "CONT",
    "PRIV"
};

int	pe_maxclass = sizeof pe_classlist / sizeof pe_classlist[0];


char   *pe_univlist[] = {
    "EOC",
    "BOOL",
    "INT",
    "BITS",
    "OCTS",
    "NULL",
    "OID",
    "ODE",
    "EXTN",
    "REAL",
    "ENUM",
    "ENCR",
    NULLCP,
    NULLCP,
    NULLCP,
    NULLCP,
    "SEQ",
    "SET",
    "NUMS",
    "PRTS",
    "T61S",
    "VTXS",
    "IA5S",
    "UTCT",
    "GENT",
    "GFXS",
    "VISS",
    "GENS",
    "CHRS",
};

int	pe_maxuniv = sizeof pe_univlist / sizeof pe_univlist[0];


int	pe_maxappl = 0;
char  **pe_applist = NULL;

int	pe_maxpriv = 0;
char  **pe_privlist = NULL;

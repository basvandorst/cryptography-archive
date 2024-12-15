/* serror.c - get system error */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/compat/RCS/serror.c,v 10.0 1993/02/18 20:21:53 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/compat/RCS/serror.c,v 10.0 1993/02/18 20:21:53 awy Rel $
 *
 *
 * $Log: serror.c,v $
 * Revision 10.0  1993/02/18  20:21:53  awy
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
#include "compat/general.h"
#include "compat/manifest.h"

/*    DATA */

extern	int sys_nerr;
extern  char *sys_errlist[];

/*  */

char   *sys_errname (i)
int	i;
{
    static char buffer[30];

    if (0 < i && i < sys_nerr)
	return sys_errlist[i];
    (void) sprintf (buffer, "Error %d", i);

    return buffer;
}

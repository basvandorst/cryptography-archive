/* smalloc.c - error checking malloc */

#ifndef lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/compat/RCS/smalloc.c,v 10.0 1993/02/18 20:21:56 awy Rel $";
#endif

/*
 * $Header: /home/isode-consortium/isode/src/lib/compat/RCS/smalloc.c,v 10.0 1993/02/18 20:21:56 awy Rel $
 *
 *
 * $Log: smalloc.c,v $
 * Revision 10.0  1993/02/18  20:21:56  awy
 * Release IC-R1
 *
 */

/*
 *                                NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include "compat/general.h"
#include "compat/manifest.h"

/*  */

static void default_smalloc_handler ()
{
	abort ();
}

static VFP smalloc_handler = default_smalloc_handler;

VFP set_smalloc_handler (fnx)
VFP fnx;
{
	VFP savefnx = smalloc_handler;

	if (fnx)
		smalloc_handler = fnx;
	else	smalloc_handler = default_smalloc_handler;
	return savefnx;
}

char *
smalloc(size)
int	size;
{
	register char *ptr;

	if ((ptr = malloc((unsigned) size)) == (char *)0) {
		(*smalloc_handler) ();
		_exit(1);	/* just in case */
	}

	return(ptr);
}

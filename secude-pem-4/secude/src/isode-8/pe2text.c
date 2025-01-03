#ifdef DEBUG

/* pe2text.c - write a PE thru a debug filter */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap/RCS/pe2text.c,v 9.0 1992/06/16 12:25:44 isode Rel $";
#endif

/* 
 * $Header: /xtel/isode/isode/psap/RCS/pe2text.c,v 9.0 1992/06/16 12:25:44 isode Rel $
 *
 *
 * $Log: pe2text.c,v $
 * Revision 9.0  1992/06/16  12:25:44  isode
 * Release 8.0
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
#include "psap.h"
#include "logger.h"

/*  */

/* logfile-backed abstract for PStreams */

/* ARGSUSED */

static int  ll_pswrite (ps, data, n, in_line)
PS	ps;
PElementData data;
PElementLen n;
int	in_line;
{
    register LLog    *lp = (LLog *) ps -> ps_addr;

    if (lp -> ll_stat & LLOGTTY) {
	(void) fflush (stdout);

	(void) fwrite ((char *) data, sizeof *data, (int) n, stderr);
	(void) fflush (stderr);
    }

    if (lp -> ll_fd == NOTOK) {
	if ((lp -> ll_stat & (LLOGERR | LLOGTTY)) == (LLOGERR | LLOGTTY))
	    return ((int) n);
	if (ll_open (lp) == NOTOK)
	    return NOTOK;
    }
    else
	if (ll_check (lp) == NOTOK)
	    return NOTOK;

    return write (lp -> ll_fd, (char *) data, (int) n);
}

/*  */

static int  ll_psopen (ps)
register PS ps;
{
    ps -> ps_writeP = ll_pswrite;

    return OK;
}

#define	ll_psetup(ps, lp)	((ps) -> ps_addr = (caddr_t) (lp), OK)

/*  */

void	pe2text (lp, pe, rw, cc)
register LLog *lp;
register PE pe;
int	rw,
	cc;
{
    register char   *bp;
    char   buffer[BUFSIZ];
    register PS ps;

    bp = buffer;
    (void) sprintf (bp, "%s PE", rw ? "read" : "wrote");
    bp += strlen (bp);
    if (pe -> pe_context != PE_DFLT_CTX) {
	(void) sprintf (bp, ", context %d", pe -> pe_context);
	bp += strlen (bp);
    }
    if (cc != NOTOK) {
	(void) sprintf (bp, ", length %d", cc);
	bp += strlen (bp);
    }
    LLOG (lp, LLOG_ALL, ("%s", buffer));

    if ((ps = ps_alloc (ll_psopen)) != NULLPS) {
	if (ll_psetup (ps, lp) != NOTOK)
	    (void) pe2pl (ps, pe);

	ps_free (ps);
    }

    (void) ll_printf (lp, "-------\n");

    (void) ll_sync (lp);
}








#else

int pe2text_dummy(
)

{
	return(0);
}

#endif

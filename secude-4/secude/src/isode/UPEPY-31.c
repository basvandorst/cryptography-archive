/* automatically generated by pepy 8.0 #11 (tiber), do not edit! */

#include "psap.h"

#define	advise	PY_advise

void	advise ();

/* Generated from module UNIV */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/pepsy/RCS/UNIV.py,v 9.0 1992/06/16 12:24:03 isode Rel $";
#endif

#ifndef PEPYPARM
#define PEPYPARM char *
#endif /* PEPYPARM */
extern PEPYPARM NullParm;

/* ARGSUSED */

int	build_UNIV_GraphicString (pe, explicit, len, buffer, parm)
register PE     *pe;
int	explicit;
int	len;
char   *buffer;
PEPYPARM parm;
{
    register char *p9;
    int p9_len;

    p9 = buffer;
    if ((p9_len = len) == 0)
        p9_len = strlen (p9);
    if (p9 == NULLCP) {
        advise (NULLCP, "GraphicString %s", PEPY_ERR_INIT_FAILED);
        return NOTOK;
    }
    if (((*pe) = str2prim (p9, p9_len, PE_CLASS_UNIV, 25)) == NULLPE) {
        advise (NULLCP, "GraphicString: %s", PEPY_ERR_NOMEM);
        return NOTOK;
    }

#ifdef DEBUG
    (void) testdebug ((*pe), "UNIV.GraphicString");
#endif


    return OK;
}

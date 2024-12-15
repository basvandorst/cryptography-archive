/* rcmd_srch.c: search a lookup table: return string value */

#ifndef lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/compat/RCS/rcmd_srch.c,v 10.0 1993/02/18 20:21:48 awy Rel $";
#endif

/*
 * $Header: /home/isode-consortium/isode/src/lib/compat/RCS/rcmd_srch.c,v 10.0 1993/02/18 20:21:48 awy Rel $
 *
 *
 * $Log: rcmd_srch.c,v $
 * Revision 10.0  1993/02/18  20:21:48  awy
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

#include "compat/manifest.h"
#include "compat/cmd_srch.h"

/*  */

char   *rcmd_srch(val, cmd)
register int   val;
register CMD_TABLE *cmd;
{
	for(;cmd->cmd_key != NULLCP; cmd++)
		if(val == cmd->cmd_value)
			return(cmd->cmd_key);
	return(NULLCP);
}

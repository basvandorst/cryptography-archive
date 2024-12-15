/* cmd_srch.h - command search structure */

/*
 * $Header: /home/isode-consortium/isode/src/h/compat/RCS/cmd_srch.h,v 10.0 1993/02/18 20:24:53 awy Rel $
 *
 *
 * $Log: cmd_srch.h,v $
 * Revision 10.0  1993/02/18  20:24:53  awy
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


#ifndef _CMD_SRCH_
#define _CMD_SRCH_

typedef struct  cmd_table {
	char    *cmd_key;
	int     cmd_value;
} CMD_TABLE;


struct  cm_args {
	char    *cm_key;
	char    *cm_value;
};

int     cmd_srch ();
char   *rcmd_srch ();

#endif

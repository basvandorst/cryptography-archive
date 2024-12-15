/*********************************************************************
 * Filename:      conf.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Tue May 13 02:12:54 1997
 * Modified by:   Tage Stabell-Kulo <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/


#include "version.h"


/*
 * The PGP_VERSION is on which sources (parts of) this library was based.
 */

char *LIB_VERSION = PACKAGE " version " VERSION;
char *PGP_VERSION = "Version: 2.6.3ia";
char *PGP_COMMENT = "Comment: Requires PGP version 2.6 or later.";

/*
 * The global "errno"
 */

int PGPliberrno;

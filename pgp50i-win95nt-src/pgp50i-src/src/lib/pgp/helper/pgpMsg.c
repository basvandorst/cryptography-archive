/*
* pgpMsg.c -- a way to obtain the list of PGP Messages
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpMsg.c,v 1.3.2.1 1997/06/07 09:50:11 mhw Exp $
*/

	#include <stdio.h>		/* For sprintf() */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define PGPMSG(msg, string) { msg, string },

/* List of all message codes */
struct pgpmsg_t {
	int msg;
	char const *string;
};

static struct pgpmsg_t const pgpmsglist[] = {
#include "pgpMsg.h"
	{ 0, 0 }	/* End-of-list placeholder */
};

/* Get rid of warning from gcc about missing prototype */
char const PGPExport *pgpmsgString (int code);

char const *
pgpmsgString (int code)
	{
			unsigned i;
			static char buf[] = "Unknown msg code xxxxxxxxx";
			/* 1234567890123456789 */

			for (i = 0; i < sizeof (pgpmsglist)/sizeof (*pgpmsglist); i++) {
				 if (pgpmsglist[i].msg == code)
				 	return pgpmsglist[i].string;
			}
			sprintf (buf+19, "%+d", code);
			return buf;
	}

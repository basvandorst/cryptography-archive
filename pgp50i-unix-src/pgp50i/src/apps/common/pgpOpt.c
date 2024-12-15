/*
 * pgpOpt.c -- a different sort of getopt()
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * Added support for creation of ~/.pgp if it doesn't exist - BAT
 *
 * $Id: pgpOpt.c,v 1.2.2.2 1997/06/07 09:49:06 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpOpt.h"

void
pgpOptStart (struct PgpOptContext *opt, int argc, char **argv)
{
	opt->optarg = 0;
	opt->optargv = argv;
	opt->optargc = argc;
	opt->state = 0;
}

/*
 * Returns the option character of the next option, or 0 if there
 * is none, or EOF on the end of the command line.
 * opt->optarg is the flag's argument (this may be NULL at the
 * end of the command line).  To "claim" it, assign zero to the
 * field in the OptContext structure.
 *
 * If 0 is returned, you do not need to explicitly claim the optarg
 * pointer; that is assumed.  (Although it's harmless to do so.)
 *
 * "--" is not returned; it turns off option recognition from there forward.
 * (opt->state is set to -2)
 */
int
pgpOptNext (struct PgpOptContext *opt)
{
	char c;

	/*
	 * if state < 0 or the argument was claimed, get a new one
	 */
	if (opt->state < 0 || !opt->optarg ||
	    (opt->state > 0 && !*opt->optarg)) {
		if (--opt->optargc <= 0)
			return EOF;
		opt->optarg = *++opt->optargv;
		if (opt->state == -2)
			return 0;
		opt->state = 0;
	}
	/*
	 * Check to see if this is the start of a flag word or a
	 * "boring" argument.  Anything not beginning with '-' is
	 * boring.  "-" by itself is considered boring.  "--" is
	 * a special case that results in everything following
	 * bring considered boring.
	 */
	if (opt->state == 0) {	/* Beginning a new word */
		opt->state = -1;
		/* Check for not '-' or lonely "-" */
		if (opt->optarg[0] != '-' || opt->optarg[1] == '\0')
			return 0;
		/* Check for -- */
		if (opt->optarg[1] == '-' && opt->optarg[2] == '\0') {
			if (--opt->optargc <= 0)
				return EOF;
			opt->optarg = *++opt->optargv;
			opt->state = -2;
			return 0;
		}
		opt->optarg++;	/* Skip leading - */
		opt->state = 1;	/* We're parsing flags */
	}
	/* Now opt->state = 1 and we have an optarg pointer */
	c = *opt->optarg++;
	if (!*opt->optarg) {
		if (--opt->optargc <= 0)
			opt->optarg = 0;
		else
			opt->optarg = *++opt->optargv;
		opt->state = 0;
	}
	return (unsigned char)c;	/* Always return > 0 */
}


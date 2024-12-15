/*
 # $Id: log.c,v 1.2 1998/04/10 10:29:58 fbm Exp fbm $
 # Copyright (C) 1997-1999 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"
#include "callback.h"
#include "msgs.h"

static FILE	*fp = NULL;
static int	msg_level = 0;

void
Log_done(void)
{
	if (fp) {
		fflush(fp);
		if (fp != stdout)
			fclose(fp);
	}
	fp = NULL;
}

void
Log_init(char *filename)
{
	Log_done();
	if (filename == NULL || filename[0] == '\0')
		;
	else if (strcmp(filename, "-") == 0)
		fp = stdout;
	else {
		fp = fopen(filename, "a");
		if (fp == NULL)
			report(R_FATAL, Msgs_get("MLOGOPEN"), filename, syserr());
	}
}

int
Log_enabled(void)
{
	return (fp != NULL);
}

int
Log_toStdout(void)
{
	return (fp == stdout);
}

void
Log_level(int level)
{
	msg_level = level;
}

void
Log(int level, const char *format, ...)
{
	va_list args;

	if (fp && level <= msg_level) {
		va_start(args, format);
		vfprintf(fp, format, args);
		va_end(args);
	}
}

void
Log_multi(int level, const char *msg, const char *prefix)
{
	const char	*sol, *eol;
	const char	*end = msg + strlen(msg);

	if (fp && level <= msg_level) {
		for (sol = msg; sol < end; sol = eol) {
			fputs(prefix, fp);
			eol = strchr(sol, '\n');
			if (eol)
				eol++;
			else
				eol = end;
			fwrite(sol, sizeof(char), eol - sol, fp);
		}
	}
}

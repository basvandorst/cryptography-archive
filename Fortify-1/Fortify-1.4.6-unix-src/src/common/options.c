/*
 # $Id: options.c,v 1.11 1998/04/10 10:27:18 fbm Exp fbm $
 # Copyright (C) 1997-2000 Farrell McKay
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

#include "includes.h"
#include "options.h"
#include "misc.h"

static int	noprompts;
static int	nowrite;
static int	noreadonly;

static char	*logFile;

void
options_setLogFile(const char *lf)
{
	if (logFile) {
		free(logFile);
		logFile = NULL;
	}
	if (lf) {
		int len;
		while (isspace(*lf))
			lf++;
		for (len = strlen(lf); len > 0 && isspace(lf[len-1]); )
			len--;
		if (len > 0)
			logFile = f_strndup(lf, len);
	}
}

char *
options_getLogFile(void)
{
	return logFile;
}

void
options_setNoPrompts(int value)
{
	noprompts = value;
}

int
options_noprompts(void)
{
	return noprompts;
}

void
options_setNoWrite(int value)
{
	nowrite = value;
}

int
options_nowrite(void)
{
	return nowrite;
}

/*
 * A Win32 specific option that controls whether or not the "read-only"
 * file attribute is programmatically overidden during execution.
 */
void
options_setNoReadOnly(int value)
{
	noreadonly = value;
}

int
options_noreadonly(void)
{
	return noreadonly;
}

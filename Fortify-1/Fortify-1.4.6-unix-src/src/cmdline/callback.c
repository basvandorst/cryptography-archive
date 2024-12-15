/*
 # $Id: callback.c,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
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

#include "misc.h"
#include "options.h"
#include "callback.h"
#include "log.h"
#include "msgs.h"

#define MSGSIZE		8192

static char		msg[MSGSIZE];

int
confirm(const char *format, ...)
{
	int	len;
        char    buf[1024];
	va_list args;

	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);

        for (;;) {
		if (!Log_toStdout()) {
			Log(0, "t0>> ");
			Log(0, msg);
		}
		fputs(msg, stdout);

		if (options_noprompts()) {
			strcpy(buf, Msgs_get("MYES"));
			printf("%s\n", buf);
		}
		else {
			fgets(buf, sizeof(buf), stdin);
			len = strlen(buf);
			while (len > 0 && (isspace(buf[len-1])))
				buf[--len] = '\0';
		}

		if (!Log_toStdout())
			Log(0, "%s\n", buf);

                if (f_stricmp(buf, Msgs_get("MQUIT")) == 0)
                        exit(0);
                if (f_stricmp(buf, Msgs_get("MNO")) == 0)
                        return 0;
                if (f_stricmp(buf, Msgs_get("MYES")) == 0)
                        return 1;
        }
}

void
report(int severity, const char *format, ...)
{
        va_list		args;

        va_start(args, format);
        vsprintf(msg, format, args);
        va_end(args);

	if (!Log_toStdout()) {
		if (msg[0] != '\n')
			Log(0, "t0>> ");
		Log(0, msg);
	}
        fputs(msg, stdout);

        if (severity == R_FATAL)
                exit(1);
}

char *
syserr(void)
{
#ifdef SYS_ERRLIST
	extern char *sys_errlist[];

	return sys_errlist[errno];		/* SunOS has no strerror() function */
#else
	return strerror(errno);
#endif
}

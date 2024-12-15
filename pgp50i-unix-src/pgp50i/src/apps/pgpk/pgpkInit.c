/*
 * pgpkInit.c initialize the pgpk application.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpkInit.c,v 1.1.2.2 1997/06/07 09:49:13 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>				/* for sbrk() */
#endif

#include "pgpDebug.h"
#include "pgpLeaks.h"
#include "pgpkExit.h"
#include "pgpkInit.h"

int
pgpInitApp (void *stacktop, int exit_prog)
{
	void *heaptop;


#if defined(UNIX) || defined(__BORLANDC__)
		/* Find the top of the heap */
		heaptop = (void *) sbrk(0);
#else
		heaptop = NULL;
#endif

		/* setup the exit routines */
		exitSetup (stacktop, heaptop, exit_prog);

		return 0;
}

#ifdef MACINTOSH

int
main()
{
		static char buf[256];
		static char *argv[32];
		int argc = 0;
		int result;
		char *p;
	
		fputs("Command -> ", stdout);
		pgpTtyGetString(buf, sizeof(buf), NULL);
		argc = 0;
		p = buf;
		while (argc < sizeof(argv) / sizeof(argv[0])
		&& (argv[argc] = strtok(p, " "))) {
			p = 0;
			argc++;
		}
		result = appMain(argc, argv);
		exitCleanup(result);
		return result;
}

#else

int
main(int argc, char *argv[])
{
	int result;
	
#if DEBUG
		pgpLeaksBeginSession("main");
#endif
	result = appMain(argc, argv);
		exitCleanup(result);
		return result;
}

#endif

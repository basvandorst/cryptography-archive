/*
 * pgpScrUnix.c -- Get the screen size, and clear it (UNIX version).
 * Not much, but there are so *many* wonderful ways to do it.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpScrUnix.c,v 1.2.2.1 1997/06/07 09:52:03 mhw Exp $
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pgpPOSIX.h"	/* For ioctl() */
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include <sys/types.h>

#ifdef sco
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

#ifdef SVR2	/* SVR2 has wierd headers */

#ifndef NOTERMIO
#include <termio.h>	/* TIOCGWINSZ should be here */
#endif

#else	/* Normal system: look in <termios.h>, <sys/ioctl.h> and <sgtty.h> */

#ifndef NOTERMIO
#include <termios.h>	/* TIOCGWINSZ should be here */
#endif
#if !defined(TIOCGWINSZ) && !defined(TIOCGSIZE) && !defined(WIOCGETD)
#include <sys/ioctl.h>	/* TIOCG... should be here */
#endif
#if !defined(TIOCGWINSZ) && !defined(TIOCGSIZE) && !defined(WIOCGETD)
#include <sgtty.h>	/* WIOCGETD should be in here */
#endif

#endif /* not SVR2 */

#include "pgpUsuals.h"
#include "pgpScreen.h"

/*
 * Get the screen size for 'more'.
 * The environment variables $LINES and $COLUMNS will be used if they exist.
 * If not, then the TIOCGWINSZ call to ioctl() is used (if it is defined).
 * If not, then the TIOCGSIZE call to ioctl() is used (if it is defined).
 * If not, then the WIOCGETD call to ioctl() is used (if it is defined).
 * If not, then get the info from terminfo/termcap (if it is there).
 * Otherwise, assume the following: we have a 24x80 vt100 or similar.
 */
#define DEFAULT_ROWS	24
#define DEFAULT_COLS	80

/*
 * Try to access terminfo through the termcap-interface in the curses
 * library (which requires linking with -lcurses) or use termcap directly
 * (which requires linking with -ltermcap)
 */
#if !defined(USE_TERMCAP) && (defined(USE_TERMINFO) || defined(USE_CURSES))
#define USE_TERMCAP 1
#endif

#ifdef USE_TERMCAP
#define TERMBUFSIZ 1024
#define UNKNOWN_TERM "unknown"
#define DUMB_TERMBUF "dumb:co#80:hc:"

extern int tgetent(), tgetnum();
#endif

/* Return the screen size */
void
screenSizeGet(unsigned *rowsp, unsigned *colsp)	/* Rot bilong kargo */
{
	char const *env;
	long t;

#if defined(TIOCGWINSZ)
	struct winsize windowInfo;
#elif defined(TIOCGSIZE)
	struct ttysize windowInfo;
#elif defined(WIOCGETD)
	struct uwdata windowInfo;
#endif

	/* Make sure that we're outputting to a terminal */
	if (!isatty(fileno(stderr))) {
		*rowsp = DEFAULT_ROWS;
		*colsp = DEFAULT_COLS;
		return;
	}

	*rowsp = *colsp = 0;

	/* LINES & COLUMNS environment variables override everything else */
	env = getenv("LINES");
	if (env != NULL && (t = atol(env)) > 0 )
		*rowsp = (unsigned)t;

	env = getenv("COLUMNS");
	if (env != NULL && (t = atol(env)) > 0 )
		*colsp = (unsigned)t;

	if (*rowsp && *colsp)
		return;

	/* See what ioctl() has to say (overrides terminfo & termcap) */
#if defined(TIOCGWINSZ)
	if (ioctl(fileno(stderr), TIOCGWINSZ, &windowInfo) != -1) {
		if (!*rowsp && windowInfo.ws_row != 0)
			*rowsp = (unsigned)windowInfo.ws_row;

		if (!*colsp && windowInfo.ws_col != 0 )
			*colsp = (unsigned)windowInfo.ws_col;

		if (*rowsp && *colsp)
			return;
	}
#elif defined(TIOCGSIZE)
	if (ioctl(fileno(stderr), TIOCGSIZE, &windowInfo) != -1) {
		if (!*rowsp && windowInfo.ts_lines != 0)
			*rowsp = (unsigned)windowInfo.ts_lines;

		if (!*colsp && windowInfo.ts_cols != 0)
			*colsp = (unsigned)windowInfo.ts_cols;

		if (*rowsp && *colsp)
			return;
	}
#elif defined(WIOCGETD)
	if (ioctl(fileno(stderr), WIOCGETD, &windowInfo) != -1) {
		if (!*rowsp && windowInfo.uw_height != 0)
			*rowsp = windowInfo.uw_height / windowInfo.uw_vs;

		if (!*colsp && windowInfo.uw_width != 0)
			*colsp = windowInfo.uw_width / windowInfo.uw_hs;

		if (*rowsp && *colsp)
			return;
	}
#endif /* You are in a twisty little maze of standards, all different */

#ifdef USE_TERMCAP
	/* See what terminfo/termcap has to say */
	if (!*rowsp || !*colsp) {
		char termBuffer[TERMBUFSIZ], *termInfo;

		if ((termInfo = getenv("TERM")) == (char *)NULL)
 			termInfo = UNKNOWN_TERM;

 		if ((tgetent(termBuffer, termInfo) <= 0))
			strcpy(termBuffer, DUMB_TERMBUF);

		if (!*rowsp && (t = tgetnum("li")) > 0)
				*rowsp = (unsigned)t;

		if (!*colsp && (t = tgetnum("co")) > 0)
				*colsp = (unsigned)t;

		if (*rowsp && *colsp)
			return;
	}
#endif

	if (*rowsp == 0)		/* nothing worked, use defaults */
		*rowsp = DEFAULT_ROWS;
	if (*colsp == 0)
		*colsp = DEFAULT_COLS;
}

/*
 * Clear the screen and home the cursor.
 * Uses a near-universal ANSI sequence, followed by an attempt to
 * erase the mess it would make if it didn't work.
 */
void
screenClear(FILE *f)
{
	fputs("\33[2J\33[H\r           \r", f);
}

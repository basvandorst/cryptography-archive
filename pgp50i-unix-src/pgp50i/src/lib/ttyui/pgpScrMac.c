/*
 * pgpScrMac.c -- get the screen size, and clear it (Mac version).
 * Not much, but there are so *many* wonderful ways to do it.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpScrMac.c,v 1.2.2.1 1997/06/07 09:52:03 mhw Exp $
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Return the screen size */
void
screenSizeGet(unsigned *rowsp, unsigned *colsp)	/* Rot bilong kargo */
{
	*rowsp = DEFAULT_ROWS;
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

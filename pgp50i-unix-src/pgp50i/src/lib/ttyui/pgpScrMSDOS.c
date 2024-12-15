/*
 * pgpScrMSDOS.c -- Get the screen size, and clear it (MS-DOS version).
 * Not much, but there are so *many* wonderful ways to do it.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpScrMSDOS.c,v 1.2.2.1 1997/06/07 09:52:03 mhw Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __BORLANDC__
/* Borland C 3.1's <dos.h> won't compile in ANSI mode without this... */
#define far __far
#endif
#ifndef __GO32__ && !defined(_WIN32)
#include <dos.h>	 /* For int86 and union REGS */
#endif

/*
 * Some DOS Compilers are reluctant to define MK_FP in ANSI mode, as if
 * it were not in the namespace reserved by ANSI for the <dos.h> header
 * file. <dos.h> is not defined by ANSI, so obviously this is a
 * hallucination due to drinking too much Jolt. Go figure.
 */
#ifndef MK_FP
#ifndef _MK_FP
/* Create using Borland C extensions. */
#define _MK_FP(seg,off) ((void __seg *)(seg) + (void __near *)(off))
#endif /* !_MK_FP */
#define MK_FP _MK_FP
#endif /* !MK_FP */

/*
 * Return the screen size
 *
 * The number of columns is returned from int 0x10, ah = 0xF, in ah.
 * The low 7 bits of al are the screen mode. If it is 80 column mono
 * or colour, then try int 0x10, ah = 0x12, bl = 0x10 (Get Information)
 * and have a look at bl. If it has changed (the allowed return values
 * are 0, 1, 2, and 3), we have a modern video BIOS, which stores the
 * number of display rows at 0040:0084. I wish I had a PC BIOS reference
 * to see if there are simpler ways to get the data. In particular,
 * how reliable are the BIOS data fields at 0040:004A (columns per
 * line in current video mode) and 0040:0084 (rows-1)?  The latter
 * is apparently only used on video adapters with their own BIOS,
 * EGA and up.
 *
 * This technique is primarily based on code in the Borland C 3.1
 * run-time library, backed with information from Frank van Gilluwe's
 * The Undocumented PC and the Programmer's Guide to the AMIBIOS.
 */
void
screenSizeGet(unsigned *rowsp, unsigned *colsp)	/* Rot bilong kargo */
{
#ifdef _WIN32
	/* No DOS tricks */
	*rowsp = 25;
	*colsp = 80;
#else
	char const *env;
	long t;
	union REGS regs;

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

	/* Otherwise, figure it out the hard way. */
	regs.h.ah = 0xF;	/* Read current video mode */
	int86(0x10, &regs, &regs);
	if (!*colsp) {
		*colsp = regs.h.ah;
		if (*rowsp)
			return;
	}

	*rowsp = 25;		/* DOS default */
	/* If Colour or B&W 80-column mode, it's safe to ask about EGA */
	if ((regs.h.al & 0x7E) == 2) {
		regs.h.ah = 0x12;
		regs.h.bl = 0x10;
		int86(0x10, &regs, &regs);
		if (regs.h.bl != 0x10)	/* EGA installed  - read BIOS */
		*rowsp = *(unsigned char const *)MK_FP(0x40, 0x84) + 1;
	}
#endif
}

/*
 * Clear the screen and home the cursor.
 * The only way to do this reliably under MS-DOS is via the BIOS.
 * This involves finding the screen size (using the above function),
 * finding the active video page (int 0x10, ah = 0xF, returned in bh),
 * finding the attribute to clear with (taken from the current cursor
 * position, int 0x10, ah = 8, bh = current page, attribute returned
 * in ah), using the "scroll window" command to clear the active page
 * screen (int 0x10, ah = 6, al = 0 for clear screen, (cl,ch) the top
 * left, (dl,dh) the bottom right of the window (zero-based (x,y)),
 * and bh the attribute to clear with), and then finally resetting the
 * cursor to (0,0) with int 0x10, ah = 2. bh = current page, (dl,dh)
 * is the position to set it to.
 */
void
screenClear(FILE *f)
{
#ifdef _WIN32
	/* Not sure how to do it */
#else
	unsigned rows, cols;
	union REGS regs;
	unsigned char page;

	(void)f;
	screenSizeGet(&rows, &cols);

	regs.h.ah = 0xF;	/* Read current video mode */
	int86(0x10, &regs, &regs);

	page = regs.h.bh;	/* Active page */
	regs.h.ah = 8;		/* Get attribute */
	/* bh is already set */
	int86(0x10, &regs, &regs);

	regs.h.bh = regs.h.ah;	/* Attribute for screen clear */
	regs.x.ax = 0x600;	/* ah=6, scroll up, ah=0, clear */
	regs.x.cx = 0;		/* Start at (0,0) */
	regs.h.dl = cols-1;
	regs.h.dh = rows-1;	/* End at (cols-1, rows-1) */
	int86(0x10, &regs, &regs);

	regs.h.ah = 2;		/* Home the cursor */
	regs.h.bh = page;
	regs.x.dx = 0;		/* Go to (0,0) */
	int86(0x10, &regs, &regs);
#endif
}

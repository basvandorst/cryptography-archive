/*
* pgpKBMac.c - Mac stdio keyboard input routines.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpKBMac.c,v 1.1.2.1 1997/06/07 09:52:00 mhw Exp $
*/
#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>

#include "pgpKB.h"

#include "pgpRndPool.h"

#ifndef STDIN_FILENO
#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#endif

static int kbFd = STDIN_FILENO;

void
kbCbreak(void)
{
}

void
kbNorm(void)
{
}

int
kbGet(void)
{
		int result;
		char c;

		result = getchar();
		if (result == EOF)
			return -1;
		c = result;
		pgpRandPoolKeystroke(c);
		return (unsigned char)c;
}

/*
 * Flush any pending input. If "thorough" is set, tries to be more
 * thorough about it. Ideally, wait for 1 second of quiet, but we
 * may do something more primitive.
 *
 * kbCbreak() has the side effect of flushing the inout queue, so this
 * is not too critical.
 */
void
kbFlush(int thorough)
{
	/* XXX: Not implemented on Mac */
	(void)thorough;		/* Avoid warning */
}

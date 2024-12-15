/*
 * pgpkExit.h -- Header file for Exit routines
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpkExit.h,v 1.2.2.1 1997/06/07 09:49:12 mhw Exp $
 */

#ifndef attribute
#if __GNUC__
#define attribute(x) __attribute__(x)
#else
#define attribute(x) /*nothing*/
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

void exitCleanup (int code);
void exitSetup (void *stacktop, void *heaptop, int prog);
void exitArgError (char const *fmt, ...);
void exitUsage (int code);

#ifdef __cplusplus
}
#endif

/* These are defined to be equivalent to Colin's original code */
#define PGPEXIT_OK		0
#define PGPEXIT_ARGS		7
#define PGPEXIT_VERSION		9
#define PGPEXIT_BREAK		15
#define PGPEXIT_SIGNAL		16
#define PGPEXIT_NOMEM		17
#define PGPEXIT_INTERNAL	18

#define EXIT_PROG_PGPM	1
#define EXIT_PROG_PGPK	2


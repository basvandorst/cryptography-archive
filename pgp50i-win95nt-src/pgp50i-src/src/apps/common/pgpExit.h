/*
 * pgpExit.h -- Header file for Exit routines
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpExit.h,v 1.2.2.3.2.2 1997/07/15 21:26:17 quark Exp $
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

struct PgpEnv;

void exitCleanup (int code);
void exitSetup (struct PgpEnv *env, void *stacktop, void *heaptop, int prog,
		int newver);
void SetProgramName(int prog);
void exitExpiryCheck (struct PgpEnv *env);
void exitUsage (int code);
void exitVersion(void);

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

#define EXIT_PROG_PGPE	1
#define EXIT_PROG_PGPS  2
#define EXIT_PROG_PGPV  4
#define EXIT_PROG_PGPO  8
#define EXIT_PROG_PGPK	16

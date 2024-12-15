/*
* spgpinit.c -- Simple PGP API helper -- Initialize for a SPGP function.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Based on PGP3 initapp.c & appfile.c, which were
* written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: spgpinit.c,v 1.7.2.1 1997/06/07 09:51:55 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <assert.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "pgpDebug.h"
#include "spgp.h"
#include "spgpint.h"

#include "pgpKeyDB.h"

/*
* Initialize for SPGP functionality, reading environment variables and
* setting up PGP environment based on config file.
*/

int
spgpInit (PgpEnv **envp, RingPool **ringpoolp)
{
	extern PGPKDBExport PgpEnv *pgpEnv;
	extern PGPKDBExport RingPool *pgpRingPool;
	PGPError error;

	error = pgpLibInit();
	if (error)
		return error;
	*envp = pgpEnv;
	*ringpoolp = pgpRingPool;
	return error;
}

void
spgpFinish(PgpEnv *env0, RingPool *ringpool, int code)
{
	(void)env0;
	(void)ringpool;

	pgpRandPoolKeystroke (code);
	pgpLibCleanup();
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/

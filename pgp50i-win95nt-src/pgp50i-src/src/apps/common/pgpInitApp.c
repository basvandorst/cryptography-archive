/*
 * pgpInitApp.c initialize a PGP application.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * Added support for creation of ~/.pgp if it doesn't exist - BAT
 *
 * $Id: pgpInitApp.c,v 1.6.2.5.2.6 1997/07/08 19:32:25 quark Exp $
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
#include <unistd.h>		/* for sbrk() */
#endif

#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpLeaks.h"
#include "pgpConf.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpFileNames.h"
#include "pgpRndSeed.h"
#include "pgpUserIO.h"

#include "pgpExit.h"
#include "pgpAppFile.h"
#include "pgpInitApp.h"

#ifdef UNIX
#include "pgpPrefUnix.h"
#endif

#include "pgpOutput.h"

Boolean RngSeeded = FALSE;  /*Set to TRUE when we load a randseed.bin
			     *or collect entropy from the user.
			     */

int
pgpInitApp (struct PgpEnv **envp, void *stacktop,
	    struct PgpUICb const *ui, void *ui_arg, int exit_prog,
	    int newvers)
{
	struct PgpEnv *env;
	void *heaptop;
	FILE *rs, *sc;
	char *fn, *confdir, *upath = NULL;
	char const *pc, *pgppath;
	PGPError err = PGPERR_OK;

#if defined(UNIX) || defined(__BORLANDC__)
	/* Find the top of the heap */
	heaptop = (void *) sbrk(0);
#else
	heaptop = NULL;
#endif

	pgpAssert (envp);

	env = *envp = pgpenvCreate ();
	if (!env)
		exitCleanup (PGPEXIT_NOMEM);

	/* setup the exit routines */
	exitSetup (env, stacktop, heaptop, exit_prog, newvers);

	/* Default verbosity level */
	pgpenvSetInt(env, PGPENV_VERBOSE, 1, PGPENV_PRI_PRIVDEFAULT);

	/* Try to find the PGPPATH */
	if ((pc = getenv ("PGPPATH")) != NULL)
		pgpenvSetString (env, PGPENV_PGPPATH, pc,
				 PGPENV_PRI_PUBDEFAULT);
#ifdef UNIX
	else if ((pc = getenv ("HOME")) != NULL) {
	    fn = fileNameBuild (pc, ".pgp", NULL);
	    CheckAndCreatePGPDir(fn);
	    pgpenvSetString (env, PGPENV_PGPPATH, fn,
			     PGPENV_PRI_PUBDEFAULT);
	    pgpMemFree (fn);
	}
#endif
	pgppath = pgpenvGetString (env, PGPENV_PGPPATH, NULL, NULL);

	/* Ok, PGPPATH is a PATH of directories.  Find the first one.. */
	confdir = fileNameNextDirectory (pgppath, NULL);
	pgpAssert (confdir);

	fn = fileNameBuild (confdir, "pubring.pkr", NULL);
	pgpenvSetString (env, PGPENV_PUBRING, fn, PGPENV_PRI_PRIVDEFAULT);
	pgpMemFree (fn);

	fn = fileNameBuild (confdir, "secring.skr", NULL);
	pgpenvSetString (env, PGPENV_SECRING, fn, PGPENV_PRI_PRIVDEFAULT);
	pgpMemFree (fn);

	fn = fileNameBuild (confdir, "language50.txt", NULL);
	pgpenvSetString (env, PGPENV_LANGUAGEFILE, fn, PGPENV_PRI_PRIVDEFAULT);

	fn = fileNameBuild (confdir, "randseed.bin", NULL);
	pgpenvSetString (env, PGPENV_RANDSEED, fn, PGPENV_PRI_PRIVDEFAULT);

	/* Read in the randseed file */
	rs = fopen (fn, "rb");
	if (rs) {
		pgpRandSeedRead (rs, NULL);
		fclose (rs);
		RngSeeded = TRUE;
	} else
		fprintf (stderr, "No randseed file found.\n");
	pgpMemFree (fn);
	
	pgpenvSetString (env, PGPENV_TMP,
#ifdef VMS
			 getenv ("SYS$SCRATCH"),
#else
			 getenv ("TMP"),
#endif
			 PGPENV_PRI_PUBDEFAULT);

	upath = getenv ("PGPUPATH");
#ifdef PGPSYSDIR
	/* Read the system config file */
	fn = fileNameBuild (PGPSYSDIR, "pgp.cfg", NULL);
	/* No error if doesn't exist */
	sc = fopen (fn, "r");
	if (sc) {
		fclose(sc);
		pgpConfigFileProcess (ui, ui_arg, env, fn, PGPENV_PRI_SYSCONF);
	}
	pgpMemFree (fn);
	upath = fileNameExtendPath (upath, PGPSYSDIR, 0);
#endif
	/*
	 * If PGPSYSDIR is defined in the environment, read config.txt
	 * from that directory.
	 */
	pc = getenv ("PGPSYSDIR");
	if (pc) {
		fn = fileNameBuild (pc, "pgp.cfg", NULL);
		pgpConfigFileProcess (ui, ui_arg, env, fn,
				      PGPENV_PRI_SYSCONF+1);
		pgpMemFree (fn);
		upath = fileNameExtendPath (upath, pc, 0);
	}

	/* Now set the Untrusted Path in the environment */
	pgpenvSetString (env, PGPENV_UPATH, upath, PGPENV_PRI_PUBDEFAULT);

	/* Read the user's config file */
	fn = fileNameBuild (confdir, "pgp.cfg", NULL);
	pgpConfigFileProcess (ui, ui_arg, env, fn, PGPENV_PRI_CONFIG);
	pgpMemFree (fn);

	pgpMemFree (confdir);

	/*Load the language strings*/
	InitializeOutputHandlesToDefaults();
	if(((err = LoadLanguageStrings(env))) != PGPERR_OK) {
	    fprintf(stderr, "\
Unable to load language file or use internal copy!\nError %d:  %s\n",
		    err,
		    pgperrString(err));
	}

	return err;
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
	
#if DEBUG
	pgpLeaksBeginSession("main");
#if MACINTOSH
	atexit(pgpLeaksEndSession);
#endif
#endif
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
#if MACINTOSH
	atexit(pgpLeaksEndSession);
#endif
#endif

	result = appMain(argc, argv);
	
	exitCleanup(result);
	return result;
}

#endif





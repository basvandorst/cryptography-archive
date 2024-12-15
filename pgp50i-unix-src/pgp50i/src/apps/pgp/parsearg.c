/*
 * parsearg.c
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: parsearg.c,v 1.1.2.3.2.4 1997/07/15 21:26:18 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpUsuals.h"
#include "pgpExit.h"
#include "pgpUI.h"
#include "pgpOpt.h"
#include "pgpConf.h"
#include "pgpEnv.h"
#include "pgpUserIO.h"
#include "pgpPassCach.h"

#include "parsearg.h"
#include "pass.h"

#include "pgpOutput.h"

#include "pgpLicense.h"

void mainParseEncryptArgs(struct PgpUICb *ui,
			  struct UIArg *ui_arg,
			  struct PgpEnv *env,
			  int *argcp,
			  char *argv[],
			  struct Flags *flags)
{
    struct PgpOptContext opt;
    int c, argc = *argcp;

    pgpOptStart (&opt, argc, argv);
    argc = 0;

    while ((c = pgpOptNext (&opt)) != EOF) {
	switch (c) {
	    case 0:
		if (opt.optarg[0] != '+' || opt.optarg[1] == '\0') {
		    if (flags->encrypt)
			flags->recips[flags->numrecips++] =
			    opt.optarg;
		    else
			argv[argc++] = opt.optarg;
		} else
		    if (pgpConfigLineProcess (ui,
					      ui_arg,
					      env,
					      opt.optarg + 1,
					      PGPENV_PRI_CMDLINE)) {
			ErrorOutput(TRUE,
				    LEVEL_CRITICAL,
				    "UNRECOGNIZED_OPTION_STRING",
				    opt.optarg);
			exitCleanup(PGPEXIT_ARGS);
		    }
		break;
		
	    case '-':
		/* This accepts --foo */
		/* -- is special-cased, so "-- foo" won't do. */
		if(strcmp(opt.optarg, "version") == 0) {
		    exitVersion();
		}
		else {
		    if (pgpConfigLineProcess(ui, ui_arg, env, opt.optarg,
					     PGPENV_PRI_CMDLINE)) {
			ErrorOutput(TRUE,
				    LEVEL_CRITICAL,
				    "UNRECOGNIZED_OPTION_STRING_DASH",
				    opt.optarg - 1);
			exitCleanup(PGPEXIT_ARGS);
		    }
		    opt.optarg = NULL;
		}
		break;
				
	    case 'a':
		flags->doarmor = 2;
		pgpenvSetInt (env, PGPENV_ARMOR, 2,
			      PGPENV_PRI_CMDLINE);
		break;
		
	    case 'c':
		flags->conventional = 2;
		break;
		
	    case 'f':
		flags->filtermode = 2;
		break;
		
	    case 'h':
		exitUsage (PGPEXIT_OK);
		/*NOTREACHED*/
		
	    case 'o':
		/* XXX This doesn't do multiple output files, right now.*/
		if (!opt.optarg) {
		    ErrorOutput(TRUE,
				LEVEL_CRITICAL,
				"NO_OUTPUT_FILENAME");
		    exitCleanup(PGPEXIT_ARGS);
		}
		if(*opt.optarg == '-' && *(opt.optarg + 1) == '\0')
		    flags->filtermode = 2;
		else
		    flags->outfile = opt.optarg;
		opt.optarg = NULL;
		break;
		
	    case 'q':
		ui_arg->arg.verbose = 0;
		pgpenvSetInt(env, PGPENV_VERBOSE,
			     0, PGPENV_PRI_CMDLINE);
		break;
		
	    case 'r':
		if (!opt.optarg) {
		    ErrorOutput(TRUE,
				LEVEL_CRITICAL,
				"NO_RECIPIENT_SPECIFIED");
		    exitCleanup(PGPEXIT_ARGS);
		}
		flags->recips[flags->numrecips++] = opt.optarg;
		opt.optarg = NULL;
		break;

	    case 's':
		flags->sign = 2;
		break;
		
	    case 't':
		pgpenvSetInt (env, PGPENV_TEXTMODE, 2,
			      PGPENV_PRI_CMDLINE);
		break;
		
	    case 'u':
		/* XXX Check for duplicate args */
		if (!opt.optarg) {
		    ErrorOutput(TRUE,
				LEVEL_CRITICAL,
				"NO_USERID_SPECIFIED");
		    exitCleanup(PGPEXIT_ARGS);
		}
		pgpenvSetString (env, PGPENV_MYNAME, opt.optarg,
				 PGPENV_PRI_CMDLINE);
		opt.optarg = 0;
		break;
		
	    case 'v':
		pgpenvSetInt(env, PGPENV_VERBOSE,
			     ++ui_arg->arg.verbose, PGPENV_PRI_CMDLINE);
		break;
		
#ifdef WIPE_SUPPORTED
	    case 'w':
		flags->wipe = 2;
		break;
#endif
		
	    case 'z':
		if (!opt.optarg) {
		    ErrorOutput(TRUE,
				LEVEL_CRITICAL,
				"NO_PASSPHRASE_SPECIFIED_IN_BATCHMODE");
		    exitCleanup(PGPEXIT_ARGS);
		}
		pgpPassCacheAdd (passcache, opt.optarg,
				 strlen (opt.optarg));
		/* Record passphrase for -c */
		strncpy(phrase, opt.optarg, sizeof(phrase));
		opt.optarg = 0;
		break;
		
	    default:
		ErrorOutput(TRUE,
			    LEVEL_CRITICAL,
			    "UNRECOGNIZED_OPTION_CHAR", c);
		exitCleanup(PGPEXIT_ARGS);
	}
	/*XXX This is horrible, but I need to get it in and don't have time
	 *to do it right.  We want to set the headers option as soon as
	 *possible so that, if there is an error with the command line
	 *args, we use the headers on output.
	 */
	SetHeaders(pgpenvGetInt(env, PGPENV_HEADERS, NULL, NULL));
    }
    *argcp = argc;

    if(pgpenvGetInt(env, PGPENV_LICENSE, NULL, NULL)) {
	display_license();
	exitCleanup (PGPEXIT_ARGS);
    }
    if(!flags->numrecips && flags->conventional != 2) {
	ErrorOutput(TRUE, LEVEL_CRITICAL, "MUST_SPECIFY_A_RECIPIENT");
	exitCleanup (PGPEXIT_ARGS);
    }
    return;
}

void mainParseSignArgs(struct PgpUICb *ui,
		       struct UIArg *ui_arg,
		       struct PgpEnv *env,
		       int *argcp,
		       char *argv[],
		       struct Flags *flags)
{
    struct PgpOptContext opt;
    int c, argc = *argcp;

    flags->sign = 2;
		
    pgpOptStart (&opt, argc, argv);
    argc = 0;

    while ((c = pgpOptNext (&opt)) != EOF) {
	switch (c) {
	    case 0:
		if (opt.optarg[0] != '+' || opt.optarg[1] == '\0') {
		    if (flags->encrypt)
			flags->recips[flags->numrecips++] =
			    opt.optarg;
		    else
			argv[argc++] = opt.optarg;
		} else
		    if (pgpConfigLineProcess (ui,
					      ui_arg,
					      env,
					      opt.optarg + 1,
					      PGPENV_PRI_CMDLINE)) {
			
			ErrorOutput(TRUE,
				    LEVEL_CRITICAL,
				    "UNRECOGNIZED_OPTION_STRING",
				    opt.optarg);
			exitCleanup (PGPEXIT_ARGS);
		    }
		break;
		
	    case '-':
		/* This accepts --foo */
		/* -- is special-cased, so "-- foo" won't do. */
		if(strcmp(opt.optarg, "version") == 0) {
		    exitVersion();
		}
		else {
		    if (pgpConfigLineProcess(ui, ui_arg, env, opt.optarg,
					     PGPENV_PRI_CMDLINE)) {
			ErrorOutput(TRUE,
				    LEVEL_CRITICAL,
				    "UNRECOGNIZED_OPTION_STRING_DASH",
				    opt.optarg - 1);
			exitCleanup (PGPEXIT_ARGS);
		    }
		    opt.optarg = NULL;
		}
		break;
		
	    case 'a':
		flags->doarmor = 2;
		pgpenvSetInt (env, PGPENV_ARMOR, 2,
			      PGPENV_PRI_CMDLINE);
		break;
		
	    case 'b':
		flags->sepsig = 2;
		break;
		
	    case 'f':
		flags->filtermode = 2;
		break;
		
	    case 'h':
		exitUsage (PGPEXIT_OK);
		/*NOTREACHED*/
		
	    case 'o':
		if (!opt.optarg) {
		    ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_OUTPUT_FILENAME");
		    exitCleanup (PGPEXIT_ARGS);
		}
		if(*opt.optarg == '-' && *(opt.optarg + 1) == '\0')
		    flags->filtermode = 2;
		else
		    flags->outfile = opt.optarg;
		opt.optarg = NULL;
		break;
		
	    case 'q':
		ui_arg->arg.verbose = 0;
		pgpenvSetInt(env, PGPENV_VERBOSE,
			     0, PGPENV_PRI_CMDLINE);
		break;
		
	    case 't':
		pgpenvSetInt (env, PGPENV_TEXTMODE, 2,
			      PGPENV_PRI_CMDLINE);
		break;
		
	    case 'u':
		/* XXX Check for duplicate args */
		if (!opt.optarg) {
		    ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_USERID_SPECIFIED");
		    exitCleanup (PGPEXIT_ARGS);
		}
		pgpenvSetString (env, PGPENV_MYNAME, opt.optarg,
				 PGPENV_PRI_CMDLINE);
		opt.optarg = 0;
		break;
		
	    case 'v':
		pgpenvSetInt(env, PGPENV_VERBOSE,
			     ++ui_arg->arg.verbose, PGPENV_PRI_CMDLINE);
		break;
		
#ifdef WIPE_SUPPORTED
	    case 'w':
		flags->wipe = 2;
		break;
#endif

	    case 'z':
		if (!opt.optarg) {
		    ErrorOutput(TRUE,
				LEVEL_CRITICAL,
				"NO_PASSPHRASE_SPECIFIED_IN_BATCHMODE");
		    exitCleanup (PGPEXIT_ARGS);
		}
		pgpPassCacheAdd (passcache, opt.optarg,
				 strlen (opt.optarg));
		/* Record passphrase for -c */
		strncpy(phrase, opt.optarg, sizeof(phrase));
		opt.optarg = 0;
		break;
		
	    default:
		ErrorOutput(TRUE,
			    LEVEL_CRITICAL,
			    "UNRECOGNIZED_OPTION_CHAR", c);
		exitCleanup (PGPEXIT_ARGS);
	}
	/*XXX This is horrible, but I need to get it in and don't have time
	 *to do it right.  We want to set the headers option as soon as
	 *possible so that, if there is an error with the command line
	 *args, we use the headers on output.
	 */
	SetHeaders(pgpenvGetInt(env, PGPENV_HEADERS, NULL, NULL));
    }
    if(pgpenvGetInt(env, PGPENV_LICENSE, NULL, NULL)) {
	display_license();
	exitCleanup (PGPEXIT_ARGS);
    }
    *argcp = argc;
    return;
}

void mainParseVerifyArgs(struct PgpUICb *ui,
			 struct UIArg *ui_arg,
			 struct PgpEnv *env,
			 int *argcp,
			 char *argv[],
			 struct Flags *flags)
{
    struct PgpOptContext opt;
    int c, argc = *argcp;

    pgpOptStart (&opt, argc, argv);
    argc = 0;

    while ((c = pgpOptNext (&opt)) != EOF) {
	switch (c) {
	    case 0:
		if (opt.optarg[0] != '+' || opt.optarg[1] == '\0') {
		    if (flags->encrypt)
			flags->recips[flags->numrecips++] =
			    opt.optarg;
		    else
			argv[argc++] = opt.optarg;
		} else
		    if (pgpConfigLineProcess (ui,
					      ui_arg,
					      env,
					      opt.optarg + 1,
					      PGPENV_PRI_CMDLINE)) {
			ErrorOutput(TRUE,
				    LEVEL_CRITICAL,
				    "UNRECOGNIZED_OPTION_STRING", opt.optarg);
			exitCleanup (PGPEXIT_ARGS);
		    }
		break;
		
	    case '-':
		/* This accepts --foo */
		/* -- is special-cased, so "-- foo" won't do. */
		if(strcmp(opt.optarg, "version") == 0) {
		    exitVersion();
		}
		else {
		    if (pgpConfigLineProcess(ui, ui_arg, env, opt.optarg,
					     PGPENV_PRI_CMDLINE)) {
			ErrorOutput(TRUE,
				    LEVEL_CRITICAL,
				    "UNRECOGNIZED_OPTION_STRING_DASH",
				    opt.optarg - 1);
			exitCleanup (PGPEXIT_ARGS);
		    }
		    opt.optarg = NULL;
		}
		break;

#if 0				
	    case 'd':
		flags->decrypt = 2;
		break;
#endif
		
	    case 'f':
		flags->filtermode = 2;
		break;
		
	    case 'h':
		exitUsage (PGPEXIT_OK);
		/*NOTREACHED*/
		
	    case 'm':
		flags->moreflag = 2;
		break;
		
	    case 'o':
		if (!opt.optarg) {
		    ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_OUTPUT_FILENAME");
		    exitCleanup (PGPEXIT_ARGS);
		}
		if(*opt.optarg == '-' && *(opt.optarg + 1) == '\0')
		    flags->filtermode = 2;
		else
		    flags->outfile = opt.optarg;
		opt.optarg = NULL;
		break;
		
	    case 'q':
		ui_arg->arg.verbose = 0;
		pgpenvSetInt(env, PGPENV_VERBOSE,
			     0, PGPENV_PRI_CMDLINE);
		break;
		
	    case 'v':
		pgpenvSetInt(env, PGPENV_VERBOSE,
			     ++ui_arg->arg.verbose, PGPENV_PRI_CMDLINE);
		break;
		
#ifdef WIPE_SUPPORTED
	    case 'w':
		flags->wipe = 2;
		break;
#endif
		
	    case 'z':
		if (!opt.optarg) {
		    ErrorOutput(TRUE,
				LEVEL_CRITICAL,
				"NO_PASSPHRASE_SPECIFIED_IN_BATCHMODE");
		    exitCleanup (PGPEXIT_ARGS);
		}

		pgpPassCacheAdd (passcache, opt.optarg,
				 strlen (opt.optarg));
		/* Record passphrase for -c */
		strncpy(phrase, opt.optarg, sizeof(phrase));
		opt.optarg = 0;
		break;
		
	    default:
		ErrorOutput(TRUE,
			    LEVEL_CRITICAL,
			    "UNRECOGNIZED_OPTION_CHAR", c);
		exitCleanup (PGPEXIT_ARGS);
	}
	/*XXX This is horrible, but I need to get it in and don't have time
	 *to do it right.  We want to set the headers option as soon as
	 *possible so that, if there is an error with the command line
	 *args, we use the headers on output.
	 */
	SetHeaders(pgpenvGetInt(env, PGPENV_HEADERS, NULL, NULL));
    }
    if(pgpenvGetInt(env, PGPENV_LICENSE, NULL, NULL)) {
	display_license();
	exitCleanup (PGPEXIT_ARGS);
    }
    *argcp = argc;
    return;
}

/*XXX This should probably be a seperate app so that people can rename it
 *PGP if they need to.
 */
void mainParse262Args(struct PgpUICb *ui,
		      struct UIArg *ui_arg,
		      struct PgpEnv *env,
		      int *argcp,
		      char *argv[],
		      struct Flags *flags)
{
    struct PgpOptContext opt;
    int c, argc = *argcp;

    pgpOptStart (&opt, argc, argv);
    argc = 0;

    while ((c = pgpOptNext (&opt)) != EOF) {
	switch (c) {
	    case 0:
		if (opt.optarg[0] != '+' || opt.optarg[1] == '\0') {
		    if (flags->encrypt)
			flags->recips[flags->numrecips++] =
			    opt.optarg;
		    else
			argv[argc++] = opt.optarg;
		} else
		    if (pgpConfigLineProcess (ui,
					      ui_arg,
					      env,
					      opt.optarg + 1,
					      PGPENV_PRI_CMDLINE)) {
			ErrorOutput(TRUE,
				    LEVEL_CRITICAL,
				    "UNRECOGNIZED_OPTION_STRING", opt.optarg);
			exitCleanup (PGPEXIT_ARGS);
		    }
		break;
		
	    case '-':
		/* This accepts --foo */
		/* -- is special-cased, so "-- foo" won't do. */
		if(strcmp(opt.optarg, "version") == 0) {
		    exitVersion();
		}
		else {
		    if (pgpConfigLineProcess(ui, ui_arg, env, opt.optarg,
					     PGPENV_PRI_CMDLINE)) {
			ErrorOutput(TRUE,
				    LEVEL_CRITICAL,
				    "UNRECOGNIZED_OPTION_STRING_DASH",
				    opt.optarg - 1);
			exitCleanup (PGPEXIT_ARGS);
		    }
		}
		opt.optarg = NULL;
		break;

#if 0				
	    case 'd':
		flags->decrypt = 2;
		break;
#endif
		
	    case 'f':
		flags->filtermode = 2;
		break;
		
	    case 'h':
		exitUsage (PGPEXIT_OK);
		/*NOTREACHED*/
		
	    case 'm':
		flags->moreflag = 2;
		break;
		
	    case 'o':
		if (!opt.optarg) {
		    ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_OUTPUT_FILENAME");
		    exitCleanup (PGPEXIT_ARGS);
		}
		if(*opt.optarg == '-' && *(opt.optarg + 1) == '\0')
		    flags->filtermode = 2;
		else
		    flags->outfile = opt.optarg;
		opt.optarg = NULL;
		break;
		
	    case 'q':
		ui_arg->arg.verbose = 0;
		pgpenvSetInt(env, PGPENV_VERBOSE,
			     0, PGPENV_PRI_CMDLINE);
		break;
		
	    case 'v':
		pgpenvSetInt(env, PGPENV_VERBOSE,
			     ++ui_arg->arg.verbose, PGPENV_PRI_CMDLINE);
		break;
		
#ifdef WIPE_SUPPORTED
	    case 'w':
		flags->wipe = 2;
		break;
#endif
		
	    case 'z':
		if (!opt.optarg) {
		    ErrorOutput(TRUE,
				LEVEL_CRITICAL,
				"NO_PASSPHRASE_SPECIFIED_IN_BATCHMODE");
		    exitCleanup (PGPEXIT_ARGS);
		}

		pgpPassCacheAdd (passcache, opt.optarg,
				 strlen (opt.optarg));
		/* Record passphrase for -c */
		strncpy(phrase, opt.optarg, sizeof(phrase));
		opt.optarg = 0;
		break;
		
	    default:
		ErrorOutput(TRUE,
			    LEVEL_CRITICAL,
			    "UNRECOGNIZED_OPTION_CHAR", c);
		exitCleanup (PGPEXIT_ARGS);
	}
	/*XXX This is horrible, but I need to get it in and don't have time
	 *to do it right.  We want to set the headers option as soon as
	 *possible so that, if there is an error with the command line
	 *args, we use the headers on output.
	 */
	SetHeaders(pgpenvGetInt(env, PGPENV_HEADERS, NULL, NULL));
    }
    if(pgpenvGetInt(env, PGPENV_LICENSE, NULL, NULL)) {
	display_license();
	exitCleanup (PGPEXIT_ARGS);
    }
    *argcp = argc;
    return;
}

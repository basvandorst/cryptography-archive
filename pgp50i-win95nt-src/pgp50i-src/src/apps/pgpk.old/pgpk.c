/*
 * pgpk.c -- PGP Key Management Main Routine!
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 * Mark H. Weaver <mhw@netris.org>
 * Chris Harman <charman@pgp.com>
 *
 * $Id: pgpk.c,v 1.1.2.5 1997/06/07 09:49:18 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>		/* for sbrk() */
#endif
#ifdef UNIX
#include <sys/stat.h>		 /* for umask() */
#endif

#include "pgpDebug.h"
#include "pgpArmor.h"
#include "pgpFileMod.h"
#include "pgpFIFO.h"
#include "pgpKeySpec.h"
#include "pgpMem.h"
#include "pgpPassCach.h"
#include "pgpConf.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpFile.h"
#include "pgpUI.h"
#include "pgpPipeline.h"
#include "pgpPubKey.h"
#include "pgpRndom.h"
#include "pgpRndPool.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpUserIO.h"
#include "pgpUsuals.h"
#include "pgpRingUI.h"
#include "pgpRngMnt.h"
#include "pgpSigSpec.h"
#include "pgpTrust.h"
#include "pgpTrstPkt.h"

#include "pgpExit.h"
#include "pgpAppFile.h"
#include "pgpKeyRings.h"
#include "pgpInitApp.h"
#include "pgpOpt.h"

#define MAXARGS 2

struct Flags {
	char args[MAXARGS];
	int argc;
	char const *outfile;
	char opt;
	byte doarmor;
};

static struct PgpPassCache *passcache = NULL;
static struct RingPool *ringpool = NULL;
static struct RingSet *allkeys = NULL;
static struct PgpRandomContext *rng = NULL;

#define PASSLEN 256

static void
setOptArgs (char const *args, struct PgpOptContext *opt, struct Flags *flags)
{
	while (*opt->optarg && strchr (args, *opt->optarg))
		flags->args[flags->argc++] = *(opt->optarg++);
}

static void
setOpt (int opt, struct PgpOptContext *optp, struct Flags *flags)
{
	if (!flags->opt) {
		flags->opt = opt;

		if (optp->state == 1 && optp->optarg)
			switch (opt) {
			case 'k':
				setOptArgs ("s", optp, flags);
				break;
			case 'l':
				setOptArgs ("l", optp, flags);
				break;
			case 'r':
				setOptArgs ("su", optp, flags);
				break;
			case 'x':
				setOptArgs ("a", optp, flags);
				break;
			}

		return;
	}

	exitArgError ("Cannot use -%c and -%c together\n", flags->opt, opt);
}

static void
mainParseArgs (struct PgpUICb *ui, void *ui_arg, struct PgpEnv *env,
	int *argcp, char *argv[], struct Flags *flags)
{
	struct PgpOptContext opt;
	int c, argc = *argcp;

	pgpOptStart (&opt, argc, argv);
	argc = 0;

	while ((c = pgpOptNext (&opt)) != EOF) {
		switch (c) {
		case 0:
	if (opt.optarg[0] != '+' || opt.optarg[1] == '\0')
				argv[argc++] = opt.optarg;
			else if (pgpConfigLineProcess (ui, ui_arg, env,
						opt.optarg + 1,
						PGPENV_PRI_CMDLINE))
				exitArgError ("Unrecognized option %s",
					opt.optarg);
			break;
		case '-':
			/* This accepts --foo */
			/* -- is special-cased, so "-- foo" won't do. */
			if (pgpConfigLineProcess(ui, ui_arg, env, opt.optarg,
						PGPENV_PRI_CMDLINE))
				exitArgError (
				"Unrecognized option -%s",
				opt.optarg - 1);
			opt.optarg = NULL;
			break;
		case 'a':	/* Add */
		case 'c':	/* Check */
		case 'd': /* Disable */
		case 'e':	/* Edit */
		case 'g':	/* Generate */
		case 'k':	/* Revo(k)e */
		case 'l':	/* List */
		case 'r':	/* Remove */
		case 's':	/* Sign */
		case 'x':	/* eXtract */
			setOpt (c, &opt, flags);
			break;
		case 'h':	/* Help */
			exitUsage (PGPEXIT_OK);
			/*NOTREACHED*/
		case 'o':	/* Output */
			/* XXX check for duplicate args */
			if (!opt.optarg)
				exitArgError (
				"-o option requires a userid argument");
			flags->outfile = opt.optarg;
			opt.optarg = 0;
			break;
		case 'u':	/* Username (myname) */
			/* XXX Check for duplicate args */
			if (!opt.optarg)
				exitArgError (
				"-u option requires a userid argument");
			pgpenvSetString (env, PGPENV_MYNAME, opt.optarg,
					PGPENV_PRI_CMDLINE);
			opt.optarg = 0;
			break;
		case 'v':	/* Verbose */
			pgpenvSetInt(env, PGPENV_VERBOSE, 1+
			pgpenvGetInt(env, PGPENV_VERBOSE,
					NULL, NULL), PGPENV_PRI_CMDLINE);
			break;
		default:
			exitArgError ("Unrecognized option -%c", c);
		}
	}
	*argcp = argc;
	return;
}

/* Two functions to write out extracted key files */
static int
mainWriteSet (FILE *fp, int flags, struct RingSet const *set)
{
	struct PgpFile *pfp = pgpFileWriteOpen (fp, NULL);
	int rslt = ringSetWrite (set, pfp, NULL, PGPVERSION_3,
				flags);
	pgpFileClose(pfp);
	return rslt;
}

/* Like mainWriteSet, but for armored files */
static int
mainWriteArmoredSet (FILE *fp, int flags, struct RingSet const *set,
struct PgpEnv *env)
{
	struct PgpFile *pfp;
	struct PgpPipeline *ppl = NULL;
	int rslt;
	
	pfp = pgpFileWriteOpen(fp, NULL);
	if (!pfp) {
		fclose(fp);
		return PGPERR_NOMEM;
	}
	if (!pgpFileWriteCreate (&ppl, pfp, 1)) {
		pgpFileClose(pfp);
		return PGPERR_NOMEM;
	}
	if (!rng)
		rng = pgpRandomCreate ();
	if (!pgpArmorWriteCreate(&ppl, env, &pgpByteFifoDesc,
				rng, PGPVERSION_2_6, PGP_ARMOR_NORMAL)) {
		ppl->teardown(ppl);
		return PGPERR_NOMEM;
	}
	pfp = pgpFilePipelineOpen(ppl);
	if (!pfp) {
		ppl->teardown(ppl);
		return PGPERR_NOMEM;
	}
	rslt = ringSetWrite (set, pfp, NULL, PGPVERSION_2_6, flags);
	pgpFileClose(pfp);
	return rslt;
}

#if 0
/* Unused at present. fp must be a seekable stream. */
static struct RingSet const *
mainOpenKeyring (FILE *fp, int trusted)
{
	struct RingFile *ring;
	struct PgpFile *pfp;
	int error = 0;

	pfp = pgpFileReadOpen (fp, NULL, NULL);
	ring = ringFileOpen (ringpool, pfp, trusted, &error);
	if (error)
		fprintf (stderr, "ringFileOpen returned %d: %s\n",
			error, pgperrString (error));
	if (ring)
		return ringFileSet (ring);
	return NULL;
}
#endif

static int
setPassword (struct PgpEnv *env, struct PgpSecKey *seckey, void *ui_arg)
{
	char pass[PASSLEN];
	char pass2[PASSLEN];
	int error;

	pgpAssert (! pgpSecKeyIslocked (seckey));

	do {
		memset (pass, 0, sizeof (pass));
		memset (pass2, 0, sizeof (pass2));

		pgpTtyGetPass (ui_arg, pass, sizeof(pass));
		fputs ("Enter same pass phrase again\n", stderr);
		pgpTtyGetPass (ui_arg, pass2, sizeof(pass2));

		if (!strcmp (pass, pass2))
			break;
		fprintf (stderr,
			"Error: Pass phrases were different.  Try again.\n");
	} while (1);

	if (!strlen (pass))
		return 0;

	memset (pass2, 0, sizeof (pass2));
	error = pgpSecKeyChangeLock (seckey, env, rng, pass, strlen (pass));
	memset (pass, 0, sizeof (pass));
	return error;
}

/* Perform the maintenance pass and get trust info for interesting
keys. */

static const char checking[] = "Checking %d signatures...\n";

static int
doKeyEditTrust(union RingObject *obj, struct RingSet *set, int firsttime,
	FILE *log); /* forward ref */

/*
 * Callback function for displaying sig progress.  We just show a count on
 * every 10th sig check
 */
static void
mainShowSigCheckProgress(void *arg, struct RingIterator *iter, int checkok)
{
	int sigcheckcount = *(int *)arg;
	(void)iter;
	(void)checkok;
	++sigcheckcount;
	*(int *)arg = sigcheckcount;
	if (sigcheckcount % 10 == 0)
		fprintf (stderr, "%6d    \r", sigcheckcount);
}

static int
mainDoMaint (void *ui_arg, struct RingSet *keys, int sigcheck,
			struct RingSet *sigs)
	{
		FILE *fp = ((struct PgpTtyUI *) ui_arg)->fp;
		int numsigs;
		struct RingSet *dest;
		unsigned count = 0;
		int err;
		int sigcheckcount;
#if 0
union RingObject *key, *name;
size_t len;
struct RingIterator *iter;
int rc;
char const *namestring;
#endif
int trust_changed = 0;

/* Check sigs as required */

switch (sigcheck) {
case 1:
/* Only check signatures in sigs that are not
marked as having been checked. */
numsigs = ringPoolCheckCount (sigs, keys, 0);
if (numsigs > 0)
fprintf (fp, checking, numsigs);
sigcheckcount = 0;
	if ((err = ringPoolCheck (sigs, keys, 0, mainShowSigCheckProgress,
			(void *) &sigcheckcount)) != 0)
		fprintf (stderr, "Error %d checking signatures\n", err);
		if (sigcheckcount > 0)
			fprintf (stderr, "\n");
		break;
		case 2:
		/* Check all signatures as part of a full maintenance
		pass */
		numsigs = ringPoolCheckCount (keys, keys, 1);
		if (numsigs > 0)
		fprintf (fp, checking, numsigs);
		sigcheckcount = 0;
		if ((err = ringPoolCheck (keys, keys, 1, mainShowSigCheckProgress,
		(void *) &sigcheckcount)) != 0)
		fprintf (stderr, "Error %d checking signatures\n", err);
		if (sigcheckcount > 0)
			fprintf (stderr, "\n");
		break;
		default:
		fprintf (fp, "No signatures to check...\n");
		/* Don't check any sigs */
		}

		/* Loop around performing maintenance passes until no more
		interesting keys are returned. */

		do {
		dest = ringSetCreate (ringpool);
		if (!dest)
			return ringPoolError(ringpool)->error;
		fprintf (fp, "Computing validity of name/key bindings...\n");
		ringMnt (keys, dest, time ((time_t *) NULL));
#if 0
ringSetFreeze (dest);
if (pgpenvGetInt (((struct PgpTtyUI *) ui_arg)->env, PGPENV_BATCHMODE,
		NULL, NULL))
		count = 0; /* ignore interesting keys in batchmode */
		else
		ringSetCount (dest, &count, 1);
		if (count > 0) {
		iter = ringIterCreate (dest);
		if (!iter) {
			ringSetDestroy (dest);
			return ringPoolError(ringpool)->error;
		}

		/* Ask the user to define the trust for each key returned in dest.
			For the new trust model, we must locate the names with undefined
			trust.
			@@@ May want indication of why key is interesting in future,
			and switch based on the reason. */

		while (ringIterNextObject (iter, 1) > 0) {
		PgpTrustModel pgptrustmodel = pgpTrustModel(ringpool);
		key = ringIterCurrentObject (iter, 1);
		pgpAssert (key && ringObjectType (key) == RINGTYPE_KEY);
		if (pgptrustmodel == PGPTRUST0) {
			fprintf (fp, "Trust in the following key is undefined:\n");
			ringTtyShowKey (ui_arg, key, keys, 0);
			if (doKeyEditTrust (key, keys, 1, fp))
				trust_changed = 1;
			else
				/* So we don't ask again */
				ringKeySetTrust (keys, key, PGP_KEYTRUST_UNKNOWN);
		} else { /* new trust model */
			ringTtyShowKey (ui_arg, key, keys, 0);
			while ((rc = ringIterNextObject (iter, 2)) > 0) {
				name = ringIterCurrentObject (iter, 2);
				if (name && ringObjectType (name) == RINGTYPE_NAME) {
					if (ringNameConfidenceUndefined (keys, name)) {
						fprintf (fp,
"\nConfidence in the following name is undefined:\n");
						namestring = ringNameName (keys, name, &len);
						ringTtyPutString (namestring, len, len+2, fp, '"',
						                    '"');
						putc ('\n', fp);
						if (doKeyEditTrust (name, keys, 1, fp)) {
							trust_changed = 1;
						} else {
							/*
							* This is required to force 'undefined trust' to
							* 'no trust', so we don't ask again.
							*/
							ringNameSetConfidence (keys, name, 0);
												}
										}
								}
						}
				}
		ringIterDestroy (iter);
}
#endif
	ringSetDestroy (dest);
} while (count > 0 && trust_changed);
return 0;
}
	
/*
* Makes sure that the global <allkeys> is loaded,
* and returns an error code or 0 if successful.
* If <canFail> is 0, then an error will cause it to exit.
	*/
static int
loadAllKeys(struct PgpEnv *env, int canFail, int trusted_only)
{
	if (!allkeys) {
		int error = mainOpenKeyrings(env, ringpool, trusted_only,
					&allkeys);

		if (error) {
			fprintf(stderr,
				"mainOpenKeyrings failed (%d): %s\n",
				error, pgperrString (error));
			if (canFail)
				return error;
			else
				exitUsage(error);
		}
	}
	return 0;
}

/*
* Interface to loadAllKeys which makes sure <allkeys> will be reloaded.
* Sometimes we want to load only untrusted keyrings but <allkeys> may
* hold trusted ones.
	*/
static int
reloadAllKeys(struct PgpEnv *env, int canFail, int trusted_only)
{
	if (allkeys) {
		ringSetDestroy(allkeys);
		allkeys = 0;
	}
	return loadAllKeys(env, canFail, trusted_only);
}


/*
* Open key rings and return sets for pubring and secring.
	*/
static void
mainOpenPubSec(struct PgpEnv *env, char const **pub,
	struct RingSet **pubring, char const **sec,
	struct RingSet **secring, FILE *log)
{
	if (pub && pubring) {
		*pub = pgpenvGetString (env, PGPENV_PUBRING, NULL, NULL);
		if (!*pub) {
			if (log)
				fprintf (log,
"Unknown pubring file, assuming \"pubring.pkr\"\n");
			*pub = "pubring.pkr";
		}
		*pubring = mainOpenRingfile (env, ringpool, *pub, "public",1);
	}

	if (sec && secring) {
		*sec = pgpenvGetString (env, PGPENV_SECRING, NULL, NULL);
		if (!*sec) {
			if (log)
				fprintf (log,
"Unknown secring file, assuming \"secring.skr\"\n");
			*sec = "secring.skr";
		}
		*secring = mainOpenRingfile (env, ringpool, *sec, "private",0);
	}
}


/*
* Searches <ring> for keys matching the substrings on the
* command line. <ring> must be immutable.
* If <argc>==0 and <defAll>, uses the entire <ring>.
* In either case, the *set returned is a valid, newly created, immutable
* set.
*
* Returns 1 if any keys were selected, else 0.
*/
static int
selectKeyArgs(struct PgpEnv *env, int argc, char *argv[],
		char const *ringName, struct RingSet const *ring,
		struct RingSet **set, int defAll)
{
	int anykeys = 0, i, result;

	(void)env;	/* Avoid warning */
	if (argc <= 0 && defAll) {
	*set = ringSetCopy(ring);
		if (!*set)
			exitUsage(ringPoolError(ringpool)->error);
		anykeys = 1;	/* XXX: Not necessarily, ring might be empty */
	} else {
		*set = ringSetCreate(ringpool);
		if (!*set)
			exitUsage(ringPoolError(ringpool)->error);
	for (i = 0; i < argc; i++) {
			result = ringSetFilterSpec(ring, *set, argv[i], 0);
			if (result < 0)
			exitUsage(result);
			else if (result > 0)
				anykeys = 1;
			else
				fprintf(stderr,
					"No key named \"%s\" in keyring \"%s\"\n",
						argv[i], ringName);
		}
	}
	return anykeys;
}

/*
* This selects one RingObject from the given immutable <set> for which
* (*filt)(env, obj, arg) returns non-zero. (*print)(env, file, obj, arg)
* should print a label for the appropriate object to file (with newline),
* suitable for a menu. If there is more than one candidate, and <print>
* is non-null, the user is presented with a menu to choose from. If
* there's more than one candidate and <print> is null, or if there are no
* candidates, 1 is returned. The object is returned in <obj>, or NULL if
* there was an error. Returns < 0 for a library error, 1 for other error,
* or 0 on success.
*
* The <print> function may assume that the global <allkeys> will be
* set correctly by the time it is called. This routine makes sure of
* that, the caller doesn't have to.
*/
static int
selectOneRingObj(struct PgpEnv *env, struct RingSet *set, int level,
		union RingObject **obj1, FILE *out, char const *header,
		void *arg,
		int (*filt)(struct PgpEnv *env, union RingObject const *obj,
				void *arg),
		void (*print)(struct PgpEnv *env, union RingObject *obj,
				FILE *file, void *arg))
{
	struct RingIterator *iter;
	union RingObject *obj, *objList[50];
	int maxObjs = sizeof(objList) / sizeof(objList[0]);
	int numObjs = 0;
	int notFirst = 0;
	int result;
	long choice;
	char *endNum, buf[32];
	int i;

	*obj1 = NULL;
	iter = ringIterCreate(set);
	if (!iter)
		return ringSetError(set)->error;

	/* Advance iter state */
	for (i=1; i<level; ++i)
		if ((result = ringIterNextObject(iter, i)) < 0)
			return result;
	
	for (;;) {
		result = ringIterNextObject(iter, level);
		if (result < 0)
			goto destroyAndExit;
		else if (result == 0)
			break;
		if (numObjs >= maxObjs) {
			fprintf(stderr, "Too many matches; aborting!\n");
			goto destroyAndError;
		}
		obj = ringIterCurrentObject(iter, level);
		pgpAssert(obj);
		if (filt(env, obj, arg)) {
			objList[numObjs++] = obj;
			if (notFirst) {
				if (!print || !out)
					goto destroyAndError;
				loadAllKeys(env, 0, 0);
				if (numObjs == 2) {
					fprintf(out, "%s\n 1) ", header);
					print(env, objList[0], out, arg);
				}
				fprintf(out, "%2d) ", numObjs);
				print(env, obj, out, arg);
			}
			else
				notFirst = 1;
		}
	}
	ringIterDestroy(iter);

	if (numObjs < 1)
		return 1;
	else if (numObjs == 1) {
		*obj1 = objList[0];
		return 0;
	}
	pgpAssert(out);
	fprintf(out, "Choose one of the above: ");
	pgpTtyGetString(buf, sizeof(buf), out);
	choice = strtol(buf, &endNum, 10);
	while (isspace(*endNum))
		endNum++;
	if (*endNum || choice < 1 || choice > numObjs) {
		fprintf(stderr, "Invalid choice\n");
		return 1;
	}
	*obj1 = objList[choice - 1];
	return 0;

destroyAndError:
	result = 1;
destroyAndExit:
	ringIterDestroy(iter);
	return result;
}

/* A filter for selectOneRingObj (above), which selects all keys */
static
int keyFilt(struct PgpEnv *env, union RingObject const *obj, void *arg)
{
	(void)env;	/* Avoid warnings */
	(void)arg;	

	return ringObjectType(obj) == RINGTYPE_KEY;
}

/* A filter to select the specified type */
static
int typeFilt(struct PgpEnv *env, union RingObject const *obj, void *arg)
{
	byte mtype = *(byte *)arg;

	(void)env;	/* Avoid warnings */
	return ringObjectType(obj) == mtype;
}

/*
* A filter for signatures that won't show revocations. We don't print
* those out explicitly so it is better not to try to choose them.
*/
static
int sigFilt(struct PgpEnv *env, union RingObject const *obj, void *arg)
{
	(void)env;
	(void)arg;
	if (ringObjectType(obj) == RINGTYPE_SIG) {
		int type = ringSigType(allkeys, (union RingObject *) obj);
		if (type != PGP_SIGTYPE_KEY_COMPROMISE &&
		type != PGP_SIGTYPE_KEY_UID_REVOKE)
			return 1; /* accept */
	}
	return 0;	 	/* reject */
}


/*
* A key printer for selectOneRingObj (above),
* which prints keys for a menu.
	*/
static void
keyPrint(struct PgpEnv *env, union RingObject *obj,
		FILE *file, void *arg)
{
struct PgpTtyUI ui;
	(void)env;	/* Avoid warnings */
	(void)arg;
	ui.fp = file;
	fputc('\n', file);
	pgpAssert(allkeys);
	ringTtyShowKey((void *) &ui, obj, allkeys, 0);
}

/*
* A name printer for selectOneRingObj (above),
* which prints keys for a menu.
	*/
static void
namePrint(struct PgpEnv *env, union RingObject *obj,
		FILE *file, void *arg)
{
	char const *namestring;
	size_t len;
	struct PgpTtyUI ui;

	(void)env;	/* Avoid warnings */
	(void)arg;

	ui.fp = file;
	fputc('\n', file);
	pgpAssert(allkeys);
	namestring = ringNameName (allkeys, obj, &len);
	ringTtyPutString (namestring, len, (unsigned) len, file, 0, 0);
	fputs("\n", file);
	ringTtyShowSigs ((void *) &ui, obj, allkeys, 2);
}

/*
* A sig printer for selectOneRingObj (above),
* which prints keys for a menu.
*/
static void
sigPrint(struct PgpEnv *env, union RingObject *obj,
		FILE *file, void *arg)
{
	(void)env;	/* Avoid warnings */
	(void)arg;	

	pgpAssert(allkeys);
	fputc('\n', file);
	ringTtyShowSig(obj, allkeys, file, 1);
}

static void
objPrint(struct PgpEnv *env, union RingObject *obj,
		FILE *file, void *arg)
{
	switch (ringObjectType(obj)) {
	case RINGTYPE_KEY:
		keyPrint(env, obj, file, arg);
		break;
	case RINGTYPE_NAME:
		namePrint(env, obj, file, arg);
		break;
	case RINGTYPE_SIG:
		sigPrint(env, obj, file, arg);
		break;
	}
}

/*
* Add an object with all its children to the dest key set.
* The src key set, which controls which children are taken, must be
* immutable. dest must be mutable.
* Return number of objects added, or negative on error.
	*/
static int
ringSetAddHierarchy(struct RingSet *dest, struct RingSet const *src,
		union RingObject *obj)
{
	struct RingIterator *iter;
	int level, initlevel;
	int nobjs;
	int err;

	/* First add the object */
	if ((err=ringSetAddObject(dest, obj)) < 0)
		return err;

	iter = ringIterCreate(src);
	if (!iter)
		return ringSetError(src)->error;

	nobjs = 1;
	initlevel=ringIterSeekTo(iter, obj);
	if (initlevel < 0)
		return initlevel;
	level = initlevel + 1;
	while (level > initlevel) {
		union RingObject *child;
		err = ringIterNextObject(iter, level);
		if (err < 0) {
			ringIterDestroy(iter);
			return err;
		}
		if (err > 0) {
			child = ringIterCurrentObject(iter, level);
			if (!child)
				return ringSetError(src)->error;
			if ((err=ringSetAddObject(dest, child)) < 0)
				return err;
			++nobjs;
			++level;
		} else {
			--level;
		}
	}
	ringIterDestroy(iter);
	return nobjs;
}

/*
* Service routine for selectRingObject, below. This one selects a user ID
* and optionally a signature from a RingSet composed of a single key
* and its descendants. See the comments for selectRingObject for more
* info on the parameters.
	*/
static int
selectChildObject(struct PgpEnv *env, struct RingSet *to_select,
		int selecttype, char const *prep, FILE *log,
		union RingObject **pobj)
{
	struct RingSet *to_select2;
	int err;
	byte type;
	char header[256];
	unsigned counts[RINGTYPE_MAX];

	pgpAssert (selecttype==RINGTYPE_NAME || selecttype==RINGTYPE_SIG);

	ringSetCountTypes(to_select, counts, RINGTYPE_MAX);
	if (selecttype == RINGTYPE_NAME)
		sprintf (header,
"Please select a user ID %s:", prep);
	else
		sprintf (header,
"Please select a user ID with a signature %s:", prep);
	type = RINGTYPE_NAME;
	err = selectOneRingObj(env, to_select, 2, pobj, log,
			header, &type, typeFilt, namePrint);
	if (err) {
		if (log)
			fprintf (log,
"No user ID's selected %s.\n", prep);
		ringSetDestroy(to_select);
		return 0;
	}
	/* Keep name and all its children */
	if (!(to_select2 = ringSetCreate(ringpool))) {
		ringSetDestroy(to_select);
		return ringPoolError(ringpool)->error;
	}
	ringSetAddHierarchy(to_select2, to_select, *pobj);
	ringSetFreeze(to_select2);

	if (selecttype == RINGTYPE_SIG) {
		ringSetCountTypes(to_select2, counts, RINGTYPE_MAX);
		pgpAssert(counts[RINGTYPE_KEY-1] == 1);
		pgpAssert(counts[RINGTYPE_NAME-1] == 1);
		sprintf (header,
"Please select a signature %s:", prep);
		err = selectOneRingObj(env, to_select2, 3, pobj, log,
				header, &type, sigFilt, sigPrint);
		if (err) {
			if (log)
				fprintf (log,
"No signatures selected %s.\n", prep);
			ringSetDestroy(to_select2);
			return 0;
		}
	}
	ringSetDestroy(to_select2);
	pgpAssert(counts[selecttype-1] >= 1);
	return counts[selecttype-1];	/* success */
}


/*
* Selection routine.
* Selects a key, userid or
* signature depending on the selecttype parameter, a RINGTYPE_ value.
* Returns pointer to the object in *pobj. Prep is a prepositional
* clause like "to be removed". Returns 0 if no keys selected, negative
* on error. If objects are selected returns the number of objects at
* that level, e.g. if a user ID is selected it returns the number of
* user ID's on that key; if a signature is selected it returns the
* number of sigs on that user ID. This will always be at least 1.
	*/
static int
selectRingObject(struct PgpEnv *env, int argc, char **argv, char const *pub,
		struct RingSet const *pubring, int selecttype,
		char const *prep, FILE *log, union RingObject **pobj)
{
	struct RingSet *to_select = NULL, *to_select2 = NULL;
	char namebuf[256];
	char header[256];
	size_t namelen;
	int anykeys = 0;
	int err;
	unsigned counts[RINGTYPE_MAX];

		/*
	* Protect header[] from being overfilled by the sprintf's to it.
	* prep is only set within the program to small strings, so this
	* should never happen.
		*/
	pgpAssert (strlen(prep) < 100);

	if (argc == 0) {
		pgpAssert(log);
		fprintf (log,
"A user ID is required to select the key you want %s.\n\
Enter the key's user ID: ", prep);
		namelen = pgpTtyGetString (namebuf, sizeof (namebuf), log);
		argv = (char **)pgpMemAlloc(sizeof(char *));
		argv[0] = namebuf;
		argc = 1;
	}

	/* Set up for return */
	ringSetCountTypes(pubring, counts, RINGTYPE_MAX);

	anykeys = selectKeyArgs (env, argc, argv, pub, pubring, &to_select, 0);
	if (anykeys) {
		ringSetFreeze(to_select);
		if (selecttype == RINGTYPE_NAME)
			sprintf (header,
"Please select a key with a user ID %s:", prep);
		else if (selecttype == RINGTYPE_SIG)
			sprintf (header,
"Please select a key with a signature %s:", prep);
		else
			sprintf (header,
"Please select a key %s:", prep);
		anykeys = !selectOneRingObj (env, to_select, 1, pobj, log,
					header, NULL, keyFilt, keyPrint);
	}
	if (!anykeys) {
		if (log)
			fprintf (log,
"No keys selected %s.\n", prep);
		ringSetDestroy(to_select);
		return 0;
	}

	/* Add key and all its children to a new ringset called to_select */
	to_select2 = ringSetCreate(ringpool);
	if (!to_select2) {
		ringSetDestroy(to_select);
		return ringPoolError(ringpool)->error;
	}
	ringSetAddHierarchy(to_select2, to_select, *pobj);
	ringSetDestroy(to_select);
	to_select = to_select2;
	ringSetFreeze(to_select);

	/* Handle signature and user ID selection */
	if (selecttype != RINGTYPE_KEY) {
		err = selectChildObject(env, to_select, selecttype,
					prep, log, pobj);
		ringSetDestroy(to_select);
		return err;
	}
					
	ringSetDestroy(to_select);
	pgpAssert(counts[selecttype-1] >= 1);
	return counts[selecttype-1];	/* success */
}


static int
doKeyAdd (struct PgpEnv *env, int argc, char *argv[], struct PgpTtyUI *ui_arg)
{
	struct RingSet *pubring = NULL, *secring = NULL;
	struct RingSet *newpub = NULL, *newsec = NULL;
	struct RingSet *tmpring, *keyfile = NULL;
	struct RingIterator *iter;
	char const *pub, *sec;
	int needwritesec=0;
	int newkeys = 0;
	int err = 0;

	mainOpenPubSec(env, &pub, &pubring, &sec, &secring, stderr);

	if (!argc) {
#if 0
		fprintf (stderr, "No input files -- reading from stdin\n");
		/* BUG. This won't work, requires fseek/ftell */
		keyfile = mainOpenKeyring (stdin, 0);
#else
		fprintf (stderr, "No key file specified for addition\n");
		goto cleanup;
#endif
	} else while (argc--) {
		struct RingSet *temp;
		FILE *fp;
		struct PgpFile *pfp;
		struct RingFile *ring;

		fprintf (stderr, "Opening ringfile \"%s\"...\n", argv[0]);
		fp = fopen (argv[0], "rb");
		pfp = pgpFileReadOpen (fp, NULL, NULL); /* chain errors */
		/* We will never close this ringfile */
		ring = ringFileOpen (ringpool, pfp, 0, &err); /* chain */
		temp = ringSetCopy(ringFileSet(ring)); /* chain */
		if (!temp) {
			fprintf (stderr, "Unable to open file \"%s\"\n",
				argv[0]);
		} else {
			fprintf (stderr,
"\nAdding keys:\n\n");
			ringTtyKeyView((void *)ui_arg, NULL, temp, argv[0], 0);
			newkeys++;
			if (keyfile) {
				struct RingSet *tring;
				tring = ringSetUnion(keyfile, temp);
				ringSetDestroy(keyfile);
				ringSetDestroy(temp);
				keyfile = tring;
			} else
				keyfile = temp;
		}
		fprintf (stderr, "\n");
		argv++;
	}

	if (!newkeys) {
		fprintf (stderr, "No keys to add\n");
		goto cleanup;
	}

	/* Add keys in keyfile to copy of pubring/secring as appropriate */
	newpub = ringSetCreate(ringpool);
	newsec = ringSetCreate(ringpool);
	if (!newpub || !newsec) {
		err = PGPERR_NOMEM;
		goto cleanup;
	}
	ringSetAddSet(newpub, pubring);
	ringSetAddSet(newsec, secring);
	ringSetDestroy(pubring);
	ringSetDestroy(secring);
	pubring = secring = NULL;

	ringSetFreeze(keyfile);
	iter = ringIterCreate(keyfile);
	if (!iter) {
		err = ringPoolError(ringpool)->error;
		goto cleanup;
	}
	while (ringIterNextObject(iter, 1) > 0) {
		union RingObject *obj = ringIterCurrentObject(iter, 1);
		pgpAssert(obj);
		pgpAssert (ringObjectType (obj) == RINGTYPE_KEY);
		/* Treat secret keys differently from public */
		if (ringKeyIsSec(keyfile, obj)) {
			struct RingSet *tring;
			struct RingSet *tring2;
			union RingObject *secobj;
			struct RingIterator *itersec;
			int level;

			/* Create tring holding just this key and children */
			tring = ringSetCreate(ringpool);
			tring2 = ringSetCreate(ringpool);
			if (!tring || !tring2) {
				err = ringPoolError(ringpool)->error;
				ringSetDestroy(tring);
				ringSetDestroy(tring2);
				goto cleanup;
			}
			ringSetAddHierarchy(tring, keyfile, obj);
			ringSetFreeze(tring);

			/* Make tring2 which holds tring minus signatures */
			ringSetAddSet(tring2, tring);
			itersec = ringIterCreate(tring);
			while ((level=ringIterNextObjectAnywhere(itersec))>0) {
				union RingObject *iterobj;
				iterobj = ringIterCurrentObject(itersec,level);
				if (ringObjectType(iterobj) == RINGTYPE_SIG)
					ringSetRemObject(tring2, iterobj);
			}

			/* Add set minus sigs to secret keyring */
			ringSetAddSet(newsec, tring2);

			/* Make another copy of tring, this one minus secret */
			ringSetAddSet(tring2, tring);
			ringIterRewind(itersec, 1);
			ringIterNextObject(itersec, 1);
			secobj = NULL;
			while (ringIterNextObject(itersec, 2) > 0) {
				secobj = ringIterCurrentObject(itersec, 2);
				if (ringObjectType(secobj) == RINGTYPE_SEC)
					break;
			}
			pgpAssert (secobj);
			ringIterDestroy(itersec);
			ringSetRemObject(tring2, secobj);
			ringObjectRelease(secobj);

			/* Add set without secret to pubring */
			ringSetAddSet(newpub, tring2);
			ringSetDestroy(tring2);
			ringSetDestroy(tring);

			needwritesec = 1;
		} else {
			/* Add to public keyring */
			ringSetAddHierarchy(newpub, keyfile, obj);
		}
	}
	ringIterDestroy(iter);
	ringSetDestroy(keyfile);
	keyfile = NULL;
	ringSetFreeze(newpub);
	ringSetFreeze(newsec);
	
	if (!pgpenvGetInt (env, PGPENV_FORCE, NULL, NULL)) {
	if (pgpenvGetInt (env, PGPENV_BATCHMODE, NULL, NULL)) {
	fprintf (stderr,
		"Use +force to add keys in batchmode\n\
Key addition cancelled.\n");
	goto cleanup;
	}
	if (needwritesec)
	fprintf (stderr,
"Do you want to add these keys to keyrings \"%s\" and\n"\
"\"%s\" (y/N)? ", pub, sec);
	else
	fprintf (stderr,
"Do you want to add these keys to keyring \"%s\" (y/N)? ", pub);
			if (!pgpTtyGetBool(0, stderr)) {
			fprintf (stderr, "Key addition cancelled.\n");
			err = 0;
			goto cleanup;
			}
		}

	/* Do maintenance pass on new rings. */
	reloadAllKeys (env, 0, 1); /* no untrusted keyrings */
	tmpring = ringSetUnion(allkeys, newpub);
	ringSetDestroy(allkeys);
	allkeys = ringSetUnion(tmpring, newsec);
	ringSetDestroy(tmpring);
	err = mainDoMaint ((void *) ui_arg, allkeys, 1, newpub);
	if (err < 0)
		goto cleanup;
	mainRingNewSet (pub, PGP_WRITETRUST_PUB, newpub);
	newpub = 0;	/* Don't destroy it */
	if (needwritesec) {
		mainRingNewSet (sec, PGP_WRITETRUST_SEC, newsec);
		newsec = 0;	/* Don't destroy it */
	}

	fprintf (stderr,
"Keys added successfully.\n");
	err = 0;
cleanup:
	if (pubring)
		ringSetDestroy(pubring);
	if (secring)
		ringSetDestroy(secring);
	if (newpub)
		ringSetDestroy(newpub);
	if (newsec)
		ringSetDestroy(newsec);
	if (keyfile)
		ringSetDestroy(keyfile);
	return err;
}

static int
doKeyExtract (struct PgpEnv *env, struct Flags *flags, int argc, char *argv[],
	struct PgpTtyUI *ui_arg)
{
	struct RingSet *pubring = NULL;
	struct RingSet *keyfile = NULL;
	char const *pub;
	byte doarmor = flags->doarmor;
	int err;

	(void) ui_arg;

	/* Check the command-line args for armor */
	if (!doarmor && flags->argc && strchr (flags->args, 'a'))
		doarmor++;

	mainOpenPubSec(env, &pub, &pubring, NULL, NULL, stderr);
	if (!pubring) {
		fprintf (stderr, "No keys in pubring\n");
		return 0;
	}

	err = selectKeyArgs (env, argc, argv, pub, pubring, &keyfile, 0);
	if (err < 0)
		return err;
	if (err == 0) {
		fprintf (stderr, "No keys were selected for extraction.\n");
		return 0;
	}
	ringSetFreeze (keyfile);

	ringSetDestroy(pubring);

	if (flags->outfile &&
	pgpTtyCheckOverwrite ((void *) ui_arg, flags->outfile))
	return PGPERR_NO_FILE;
	if (doarmor) {
		FILE *fp = flags->outfile ? fopen(flags->outfile, "w") :
									stdout;
		if (!fp)
			return PGPERR_NO_FILE;
		err = mainWriteArmoredSet (fp, 0, keyfile, env);
	} else if (flags->outfile) {
		FILE *fp = fopen(flags->outfile, "wb");
		if (!fp)
			return PGPERR_NO_FILE;
		err = mainWriteSet (fp, 0, keyfile);
	} else {
		err = mainWriteSet (stdout, 0, keyfile);
	}
	ringSetDestroy(keyfile);
	return err;
}

static int
keygenProgress (void *arg, int c)
{
	(void)arg;		/* make the compiler happy */
	putc (c, stderr);
	fflush (stderr);
	return 0;
}

/* Optional Arguments: name keybits */
static int
doKeyGenerate (struct PgpEnv *env, int argc, char *argv[], void *ui_arg)
{
	struct RingSet *pubring = NULL, *secring = NULL;
	struct RingSet *newpub = NULL, *newsec = NULL;
	struct PgpKeySpec *keyspec = NULL;
	struct PgpSecKey *seckey = NULL, *subseckey = NULL;
	struct PgpPkAlg const *pkalg, *subkalg;
	char const *pub = NULL, *sec = NULL; /* make the compiler happy */
	char *name = NULL, namebuf[256];
	size_t namelen = 0;
	unsigned keybits = 0;
	int keytype;
	unsigned entropy;
	int error = 0;
	unsigned len;
	char buf[20];

	fprintf (stderr,
		"Choose the type of your public key:\n"
		"  1) DSS/Diffie-Hellman - New for PGP 5!  "
									"Separate signing and encryption keys\n"
		"  2) RSA                - Compatible with old versions of PGP\n"
		"  3) RSA pair           - Separate signing and encryption using RSA\n"
		"Choose 1, 2, or 3: ");
	len = pgpTtyGetString (buf, sizeof(buf), stderr);
	if (!len) {
		fprintf (stderr, "No key type was chosen\n");
		error = -1;
		goto cleanup;
	}
	keytype = atoi(buf);
	switch (keytype) {
	case 1:
		pkalg = pgpPkalgByNumber(PGP_PKALG_DSA);
		subkalg = pgpPkalgByNumber(PGP_PKALG_ELGAMAL);
		break;
	case 2:
		pkalg = pgpPkalgByNumber(PGP_PKALG_RSA);
		subkalg = 0;
		break;
	case 3:
		pkalg = pgpPkalgByNumber(PGP_PKALG_RSA);
		subkalg = pgpPkalgByNumber(PGP_PKALG_RSA);
		break;
	default:
		/* Allow unforeseen key types */
		pkalg = pgpPkalgByNumber(keytype);
		subkalg = 0;
		break;
	}
	if (!pkalg) {
		fprintf (stderr, "No legal key type was chosen\n");
		error = -1;
		goto cleanup;
	}

	/* Need to get userid, keysizes, etc. */
	if (argc) {
		name = argv[0];
		namelen = strlen (name);
		argv++;
		argc--;
	}
	if (argc) {
		keybits = atoi (argv[0]);
		argv++;
		argc--;
	}

	if (!keybits) {
		fputs (
"\nPick your public/private keypair key size:\n\
\n***** TEMPORARY LABELS, TO BE DETERMINED LATER!! *****\n\n\
    1)   768 bits- Commercial grade, probably not currently breakable\n\
    2)  1024 bits- High commercial grade, secure for many years\n\
    3)  2048 bits- \"Military\" grade, secure for the forseeable future\n\
    4)  3072 bits- Archival grade, slow, highest security\n\
Choose 1, 2, 3, or 4, or enter desired number of bits: ", stderr);
		fflush (stderr);
		len = pgpTtyGetString (buf, sizeof(buf), stderr);
		if (!len) {
			fprintf (stderr, "Keygen error\n");
			error = -1;
			goto cleanup;
		}
		keybits = atoi (buf);
		switch (keybits) {
			case 1: keybits = 768; break;
			case 2: keybits = 1024; break;
			case 3: keybits = 2048; break;
			case 4: keybits = 3072; break;
			default:
				if (keybits < 512) {
					fprintf (stderr,
"Minimum key size is 512 bits\n");
					error = -1;
					goto cleanup;
				}
				if (keybits > 4096) {
					fprintf (stderr,
"Maximum key size is 4096 bits\n");
					error = -1;
					goto cleanup;
				}
				break;
		}
	}

	if (!name) {
		name = namebuf;
		fputs (
"\nYou need a user ID for your public key.  The desired form for this\n\
user ID is your name, followed by your E-mail address enclosed in\n\
<angle brackets>, if you have an E-mail address.\n\
For example:  John Q. Smith <12345.6789@compuserve.com>\n\
Enter a user ID for your public key: ", stderr);
		fflush (stderr);
		namelen = pgpTtyGetString (namebuf, sizeof (namebuf), stderr);
	}

	if (!namelen) {
		fprintf (stderr, "Keygen error\n");
		error = -1;
		goto cleanup;
	}

	/* Open the existing keyrings */
	mainOpenPubSec(env, &pub, &pubring, &sec, &secring, stderr);

	/* Copy pubring and secring */
	newpub = ringSetCreate (ringpool);
	ringSetAddSet (newpub, pubring);
	ringSetDestroy(pubring);
	newsec = ringSetCreate (ringpool);
	ringSetAddSet (newsec, secring);
	ringSetDestroy(secring);

	if (!newpub || !newsec) {
		error = PGPERR_NOMEM;
		goto cleanup;
	}

	/* Generate Randomness */
	if (!rng)
		rng = pgpRandomCreate();

	/* Need to ask for randomness */
	entropy = pgpSecKeyEntropy (pkalg, keybits, FALSE);
	if (subkalg)
		entropy += pgpSecKeyEntropy (subkalg, keybits, FALSE);
	pgpTtyRandAccum (ui_arg, entropy);
	
	/* Generate the SecKey */
	seckey = pgpSecKeyGenerate (pkalg, keybits, FALSE, rng, keygenProgress,
				NULL, &error);

	pgpRandomStir (rng);
	if (!seckey && !error)
		error = PGPERR_NOMEM;
	if (error)
		goto cleanup;

	/* Need to lock the SecKey */
	fprintf(stderr, "\n\
You need a pass phrase to protect your %s private key.\n\
Your pass phrase can be any sentence or phrase and may have many\n\
words, spaces, punctuation, or any other printable characters.\n",
		(subkalg ? "signature" : "RSA"));
	error = setPassword (env, seckey, ui_arg);
	if (error)
		goto cleanup;

	/* Generate the keyring objects */

	keyspec = pgpKeySpecCreate (env);
	if (!keyspec) {
		error = PGPERR_NOMEM;
		goto cleanup;
	}

	error = ringCreateKeypair (env, seckey, keyspec, name, namelen, rng,
				newpub, newsec);
	if (error)
		goto cleanup;

	/* Make subkey if requested */
	if (subkalg) {
		/* Generate the subseckey */
		subseckey = pgpSecKeyGenerate (subkalg, keybits, FALSE, rng,
					keygenProgress, NULL, &error);
		pgpRandomStir (rng);
		if (!subseckey && !error)
			error = PGPERR_NOMEM;
		if (error)
			goto cleanup;

		/* Need to lock the encryption SubSecKey */
		fprintf(stderr, "\n\
You also need a pass phrase to protect your encryption private key.\n\
Your pass phrase can be any sentence or phrase and may have many\n\
words, spaces, punctuation, or any other printable characters.\n");
		error = setPassword (env, subseckey, ui_arg);
		if (error)
			goto cleanup;

		error = ringCreateSubkeypair (env, seckey, subseckey, keyspec,
					rng, newpub, newsec);
		if (error)
			goto cleanup;
	}

	/* Freeze the sets */
	ringSetFreeze (newpub);
	ringSetFreeze (newsec);

	/* Write out newpub and newsec */
	mainRingNewSet (sec, PGP_WRITETRUST_SEC, newsec);
	mainRingNewSet (pub, PGP_WRITETRUST_PUB, newpub);
	newsec = newpub = 0;	/* Don't destroy these */

	cleanup:
	if (newpub)
		ringSetDestroy (newpub);
	if (newsec)
		ringSetDestroy (newsec);
	if (seckey)
		pgpSecKeyDestroy (seckey);
	if (subseckey)
		pgpSecKeyDestroy (subseckey);
	if (keyspec)
		pgpKeySpecDestroy (keyspec);
	putc ('\n', stderr);
	if (!error)
		fprintf (stderr,
"Keypair created successfully.\n\
Public Key written to %s\n\
Private Key written to %s\n", pub, sec);

	return error;
}

static int
doKeyDisable (struct PgpEnv *env, int argc, char *argv[],
	struct PgpTtyUI *ui_arg)
{
	struct RingSet *pubring = NULL;
	struct RingSet *keys = NULL;
	char const *pub;
	union RingObject *obj;
#if 0
	struct RingIterator *iter;
#endif
	int disabled;
	int done_something = 0;
	int err;
	(void) ui_arg;

	mainOpenPubSec(env, &pub, &pubring, NULL, NULL, stderr);
	if (!pubring) {
		fprintf (stderr, "No keys in pubring\n");
		return 0;
	}

#if 0
	selectKeyArgs (env, argc, argv, pub, pubring, &keys, 0);
	ringSetFreeze (keys);
#else
	/* Select object to remove */
	err = selectRingObject(env, argc, argv, pub, pubring, RINGTYPE_KEY,
			"to disable or enable", stderr, &obj);
	keys = pubring;
	if (err <= 0) {
		return 0;
	}
	pgpAssert(ringObjectType(obj) == RINGTYPE_KEY);
#endif

	disabled = ringKeyDisabled (keys, obj);
	fprintf (stderr, "\n");
	ringKeyPrint (stderr, keys, obj, 1);
	fprintf (stderr, "\n");
	if (disabled) {
	fprintf (stderr, "Re-enable this key? (y/N) ");
		if (pgpTtyGetBool (0, stderr)) {
		ringKeyEnable (pubring, obj);
			done_something = 1;
			fprintf (stderr, "\nKey re-enabled.\n");
		}
	}
	else {
	fprintf (stderr, "Disable this key? (y/N) ");
		if (pgpTtyGetBool (0, stderr)) {
		ringKeyDisable (pubring, obj);
			done_something = 1;
			fprintf (stderr,"\nKey disabled.\n");
		}
	}
	ringSetDestroy (keys);
	/* Uncomment if disabled keys cannot participate in trust computation
	if (done_something) {
		reloadAllKeys (env, 0, 1); [* no untrusted keyrings *]
	return mainDoMaint ((void *) ui_arg, allkeys, 2, allkeys);
	}
*/
	return 0;
}

static int
doKeyCheck (struct PgpEnv *env, struct Flags *flags, int argc, char *argv[],
	struct PgpTtyUI *ui_arg)
{
	struct RingSet *pubring = NULL;
	struct RingSet *keys;
	char const *pub;

	(void)flags;
	mainOpenPubSec(env, &pub, &pubring, NULL, NULL, stderr);

	if (!pubring) {
		fprintf (stderr, "No keys in pubring\n");
		return 0;
	}
	
	reloadAllKeys (env, 0, 1); /* no untrusted keyrings */
	fprintf (stderr, "\n");
	if (argc > 0) {
		selectKeyArgs (env, argc, argv, pub,
				pubring, &keys, 0);
		ringSetFreeze (keys);
		ringTtyKeyView ((void *) ui_arg, keys, allkeys, NULL, 3);
		ringSetDestroy (keys);
	} else {
		int err = mainDoMaint ((void *) ui_arg, allkeys, 2, NULL);
		if (err < 0) {
			ringSetDestroy(pubring);
			return err;
		}
		ringTtyKeyView ((void *) ui_arg, allkeys, allkeys, NULL, 4);
	}
	ringSetDestroy(pubring);
	return 0;
}


static int
doKeyList (struct PgpEnv *env, struct Flags *flags, int argc, char *argv[],
	struct PgpTtyUI *ui_arg)
{
	/* struct RingSet *pubring = NULL; */
	struct RingSet *keys;
	char const *pub = NULL;
	int list_mode = 0;

	/*	mainOpenPubSec(env, &pub, &pubring, NULL, NULL, stderr); */
	loadAllKeys (env, 0, 0);
	selectKeyArgs (env, argc, argv, pub, allkeys, &keys, 1);
	/*	ringSetDestroy(pubring); */
	ringSetFreeze (keys);

	if (flags->argc > 0 && flags->args[0] == 'l')
	list_mode = 2; /* verbose */
	fprintf (stderr, "\n");
	ringTtyKeyView ((void *) ui_arg, keys, allkeys, NULL, list_mode);

	ringSetDestroy (keys);
	return 0;
}

/*
* Give the user an opportunity to edit the trust of object obj.
* Return 1 if he changes it, 0 if he does not.
* firsttime means that we have not previously had a trust value for
* this key, which changes the wording of the questions somewhat.
* If PGPTRUSTMODEL is 0, obj should be a key, else it should be a name.
* The key and/or name should have been printed just before this routine
* is called.
*/
static int
doKeyEditTrust(union RingObject *obj, struct RingSet *set, int firsttime,
	FILE *log)
{
	byte keytrust;
	word16 oldtrust;
	unsigned long confvalue;
	unsigned long trustcode;
	int len;
	PgpTrustModel pgptrustmodel;
	char buf[8];		/* enough for 7 digit response */

	if (firsttime) {
		fprintf (log,
"Do you want to change your estimate of this key owner's reliability\n"\
"as an introducer of other keys (y/N)? ");
		if (!pgpTtyGetBool(0, log)) {
			fprintf (log,
"No changes made.\n");
			return 0;
		}
		fprintf (log,
"\n"\
"Make a determination in your own mind whether this key actually\n"\
"belongs to the person whom you think it belongs to, based on available\n"\
"evidence.  If you think it does, then based on your estimate of\n"\
"that person's integrity and competence in key management, answer\n"\
"the following question:\n");
	}

	pgptrustmodel = pgpTrustModel(ringpool);
	if (pgptrustmodel == PGPTRUST0) {
		fprintf (log,
"\nWould you trust this key owner\n"\
"to act as an introducer and certify other people's public keys to you?\n"\
"(1=I don't know. 2=No. 3=Usually. 4=Yes, always.) ? ");
		len = pgpTtyGetString (buf, 2, log);
		if (!len) {
			fprintf (log,
	"No changes made.\n");
			return 0;
		}
		trustcode = atoi(buf);
		switch (trustcode) {
		case 1:
			keytrust = PGP_KEYTRUST_UNKNOWN;
			break;
		case 2:
			keytrust = PGP_KEYTRUST_NEVER;
			break;
		case 3:
			keytrust = PGP_KEYTRUST_MARGINAL;
			break;
		case 4:
			keytrust = PGP_KEYTRUST_COMPLETE;
			break;
		default:
			fprintf (log,
	"Unrecognized response.\n");
			fprintf (log,
	"No changes made.\n");
			return 0;
		}	
		ringKeySetTrust(set, obj, keytrust);
	} else {					/* New trust model */
		fprintf (log,
"\nDescribe the confidence you have in this person as an introducer.\n"\
"What are the odds that this key owner is going to be wrong about\n"\
"a key which he has signed as an introducer?\n");
		if (!firsttime) {
			oldtrust = ringNameConfidence (set, obj);
			if (oldtrust==PGP_TRUST_INFINITE)
				fprintf (log,
"(Currently he is listed as having essentially zero chance"\
" of being wrong.)\n");
			else if (oldtrust==0)
				fprintf (log,
"(Currently he is listed as not having any confidence as an introducer.)\n");
			else {
				int d = oldtrust-PGP_TRUST_DECADE-PGP_TRUST_OCTAVE;
				int i;
				unsigned long l;
				fprintf (log,
"(Currently he is listed as having a one in ");
				if (d < 0)
					d = 0;
				d -= d % PGP_TRUST_DECADE;
				i = d / PGP_TRUST_DECADE;
				l = ringTrustToInt(oldtrust - d);
				if (i)
					fprintf(log, "%lu%0*u", l, i, 0);
				else
					fprintf(log, "%lu", l);
				fprintf (log,
						" chance of being wrong.)\n");
			}
		}
		fprintf (log,
"Enter a number from 1 to 2 million");
		if (!firsttime)
			fprintf (log,
", or hit return to leave unchanged.");
		fprintf (log,
"\nHe will be wrong one time in: ");
		len = pgpTtyGetString (buf, sizeof(buf), log);
		if (!len) {
			fprintf (log,
"No changes made.\n");
			return 0;
		}
		trustcode = strtoul(buf, NULL, 0);
		confvalue = ringIntToTrust(trustcode);
		ringNameSetConfidence(set, obj, confvalue);
	}
	return 1;	 	/* made a change */
}


/* Helper routine for doKeyEdit, when the key being edited is someone else's */
static int
doKeyEditOthers(struct PgpEnv *env, struct Flags *flags, union RingObject *obj,
		const char *pub, struct RingSet *pubring, const char *sec,
		struct RingSet *secring, struct PgpTtyUI *ui_arg)
{
	struct RingSet *tring;
	union RingObject *name;
	int err;
	PgpTrustModel pgptrustmodel;

	(void) flags;
	(void) pub;
	(void) sec;
	(void) secring;

	pgptrustmodel = pgpTrustModel(ringpool);
	if (pgptrustmodel == PGPTRUST0) {
		err = doKeyEditTrust(obj, pubring, 0, stderr);
	} else {
		/* New trust model works on a per user ID basis. Must select one. */
		tring = ringSetCreate(ringpool);
		if (!tring) {
			return ringPoolError(ringpool)->error;
		}
		ringSetAddHierarchy(tring, pubring, obj);
		ringSetFreeze(tring);

		err = selectChildObject(env, tring, RINGTYPE_NAME, "to edit", stderr,
					&name);
		ringSetDestroy(tring);
		if (err < 0)
			return err;

		fprintf (stderr, "\n");
		ui_arg->fp = stderr;
		ringTtyShowKey(ui_arg, obj, pubring, 0);
		ringObjectRelease(obj);

		err = doKeyEditTrust(name, pubring, 0, stderr);
	}
	if (err < 0)
		return err;
	if (err == 0)
		return 0;	/* No changes made */
	fprintf (stderr,
"Public keyring updated.\n");

	reloadAllKeys (env, 0, 1);
	err = mainDoMaint ((void *) ui_arg, allkeys, 0, NULL);
	ringSetDestroy(pubring);
	ringSetDestroy(secring);
	return err;
}

/*
* Helper routine for doKeyEditSelf, to change a pass phrase.
* If parent is non-NULL, it is a subkey pass phrase.
* secobj is the object getting its pass phrase changed, with seckey
* having been made from it.
*/
static int
doKeyChangePassphrase(struct PgpEnv *env, struct PgpTtyUI *ui_arg,
	struct RingSet *set, struct PgpSecKey *seckey,
	union RingObject *secobj, union RingObject *parent)
{
	word32 validity;
	union RingObject *newsecobj;
	struct PgpKeySpec *keyspec = NULL;
	PgpVersion version;
	int err;

	fprintf (stderr, "\n");
	if (!rng)
		rng = pgpRandomCreate ();
	err = setPassword (env, seckey, ui_arg);
	if (err < 0)
		return err;
	keyspec = pgpKeySpecCreate (env);
	if (!keyspec)
		return PGPERR_NOMEM;
	/* We need to make this keyspec just like the existing one */
	/* XXX This could be put into the ABI */
	pgpKeySpecSetCreation(keyspec,
			ringKeyCreation(allkeys, secobj));
	/* Fix "version bug", don't change version from earlier one. */
	version = ringSecVersion(allkeys, secobj);
	pgpKeySpecSetVersion(keyspec, version);
	validity = ringKeyExpiration(allkeys, secobj);
	if (validity != 0) {
		validity -= ringKeyCreation(allkeys, secobj);
		validity /= 3600*24;
	}
	pgpKeySpecSetValidity(keyspec, validity);

	newsecobj = ringCreateSec (set, parent, seckey, keyspec,
				seckey->pkAlg);
	pgpKeySpecDestroy(keyspec);
	if (!newsecobj)
		return ringSetError(set)->error;
	return 0;
}


/* Helper routine for doKeyEdit, when the key being edited belongs to us */
static int
doKeyEditSelf(struct PgpEnv *env, struct Flags *flags, union RingObject *obj,
	const char *pub, struct RingSet *pubring, const char *sec,
	struct RingSet *secring, struct PgpTtyUI *ui_arg)
{
	union RingObject *subobj;
	struct RingSet *pubset=NULL, *secset=NULL;
	struct PgpSigSpec *sigspec = NULL;
	struct PgpSecKey *seckey = NULL, *subkey = NULL;
	int needwritesec=0, needwritepub=0;
	int err = 0;

	(void)flags;		/* make the compiler happy */
	loadAllKeys(env, 1, 0);
	
	pub = pgpenvGetString (env, PGPENV_PUBRING, NULL, NULL);
	if (!pub) {
		fprintf (stderr,
			"Unknown pubring file, assuming \"pubring.pkr\"\n");
		pub = "pubring.pkr";
	}
	pubring = mainOpenRingfile (env, ringpool, pub, "public", 1);

	/* Create copies of pub and secret sets */
	secset = ringSetCreate (ringpool);
	pubset = ringSetCreate (ringpool);
	if (!secset || !pubset) {
		err = ringPoolError(ringpool)->error;
		goto cleanup;
	}
	ringSetAddSet(secset, secring);
	ringSetAddSet(pubset, pubring);

	seckey = ringSecSecKey(allkeys, obj, 0);
	ui_arg->ringset = allkeys;
	err = pgpTtyUnlockSeckey (ui_arg, obj, seckey,
"\nYou need a pass phrase to unlock your private key.\n");
	if (err < 0)
		goto cleanup;

	if (!ringKeyAxiomatic(allkeys, obj)) {
		fprintf (stderr,
"\nUse this key as an ultimately-trusted introducer (y/N)? ");
		if (pgpTtyGetBool (0, stderr)) {
			ringKeySetAxiomatic (allkeys, obj);
			fprintf (stderr,
"\nKey has been marked as having ultimate confidence.\n");
			needwritepub = 1;
		}
	}
	fprintf (stderr,
"\nDo you want to add a new user ID (y/N)? ");
	if (pgpTtyGetBool (0, stderr)) {
		union RingObject *nameobj;
		char namebuf[256];
		int namelen;

		fprintf (stderr,
"Enter the new user ID: ");
		namelen = pgpTtyGetString (namebuf, sizeof (namebuf), stderr);
		if (!namelen) {
			fprintf (stderr,
"No name entered.\n");
			goto cleanup;
		}
		nameobj = ringCreateName (secset, obj, namebuf, namelen);
		if (!nameobj) {
			err = ringSetError(secset)->error;
			goto cleanup;
		}
		ringSetAddObject(pubset, nameobj);
		/* Setting key axiomatic sets name trust as well */
		ringKeySetAxiomatic(pubset, obj);

		fprintf (stderr,
"Make this user ID the primary user ID for this key (y/N)? ");
		if (pgpTtyGetBool(0, stderr)) {
			ringRaiseName(secset, nameobj);
			ringRaiseName(pubset, nameobj);
		}

		sigspec = pgpSigSpecCreate (env, seckey,
					PGP_SIGTYPE_KEY_GENERIC);
		if (!rng)
			rng = pgpRandomCreate();
		if (!sigspec || !rng) {
			err = PGPERR_NOMEM;
			goto cleanup;
		}
		err = ringSignObject (pubset, nameobj, sigspec, rng);
		pgpSigSpecDestroy(sigspec);
		sigspec = NULL;
		if (err < 0)
			goto cleanup;
		needwritesec = needwritepub = 1;
	}

	subobj = ringKeySubkey(allkeys, obj);
	if (subobj)
		fprintf (stderr,
"\nDo you want to change your signature key pass phrase (y/N)? ");
	else
		fprintf (stderr,
"\nDo you want to change your pass phrase (y/N)? ");
	if (pgpTtyGetBool(0, stderr)) {
		err = doKeyChangePassphrase(env, ui_arg, secset,
					seckey, obj, NULL);
		if (err < 0)
			goto cleanup;
		needwritesec = 1;
	}
	
	if (subobj && ringKeyIsSec(secset, subobj)) {
		fprintf (stderr,
"\nDo you want to change your encryption key pass phrase (y/N)? ");
		if (pgpTtyGetBool(0, stderr)) {
			subkey = ringSecSecKey(allkeys, subobj, 0);
			pgpAssert(subkey);
			err = pgpTtyUnlockSeckey (ui_arg, obj, subkey,
"\nYou need a pass phrase to unlock your private encryption key.\n");
		if (err < 0)
				goto cleanup;
			err = doKeyChangePassphrase(env, ui_arg, secset,
						subkey, subobj, obj);
			if (err < 0)
				goto cleanup;
			needwritesec = 1;
		}
	}


	/* Write out new results */
	if (needwritesec) {
		ringSetFreeze (secset);
		mainRingNewSet (sec, PGP_WRITETRUST_SEC, secset);
		secset = 0;	/* Don't destroy this */
		fprintf (stderr,
"Private keyring updated.\n");
	}
	if (needwritepub) {
		ringSetFreeze (pubset);
		mainRingNewSet (pub, PGP_WRITETRUST_PUB, pubset);
		pubset = 0;	/* Don't destroy this */
		fprintf (stderr,
"Public keyring updated.\n");
	}
cleanup:
	if (seckey)
		pgpSecKeyDestroy(seckey);
	if (subkey)
		pgpSecKeyDestroy(subkey);
	if (pubset)
		ringSetDestroy(pubset);
	if (secset)
		ringSetDestroy(secset);
	if (pubring)
		ringSetDestroy(pubring);
	if (secring)
		ringSetDestroy(secring);
	return err;
}


static int
doKeyEdit (struct PgpEnv *env, struct Flags *flags, int argc, char *argv[],
	struct PgpTtyUI *ui_arg)
{
	struct RingSet *pubring = NULL, *secring = NULL;
	union RingObject *obj;
	char const *pub, *sec;
	char const *prep = "to edit";
	int err;

	(void)flags;		/* make the compiler happy */

	mainOpenPubSec(env, &pub, &pubring, &sec, &secring, stderr);

	if (!pubring || !secring) {
		fprintf (stderr, "No keys in pubring or secring\n");
		return 0;
	}

	loadAllKeys(env, 1, 0);
	
	/* Select object to sign */
	err = selectRingObject(env, argc, argv, pub, pubring, RINGTYPE_KEY,
			prep, stderr, &obj);
	if (err <= 0)
		return err;

	fprintf (stderr, "\n");
	ringKeyPrint (stderr, pubring, obj, 1);
	fprintf (stderr, "\n");

	if (ringKeyIsSec(allkeys, obj))
		return doKeyEditSelf(env, flags, obj, pub, pubring,
				sec, secring, ui_arg);
	else
		return doKeyEditOthers(env, flags, obj, pub, pubring,
				sec, secring, ui_arg);
}


/*
* This handles key, user ID (with -ru), or signature (with -rs) removal.
*/
static int
doKeyRemove (struct PgpEnv *env, struct Flags *flags, int argc, char *argv[],
	struct PgpTtyUI *ui_arg)
{
	struct RingSet *pubring = NULL;
	struct RingSet *secring = NULL;
	struct RingSet *keyfile = NULL, *seckeyfile = NULL;
	union RingObject *obj;
	char const *pub, *sec;
	int numseckeys = 0;
	int err = 0;
	int removelevel;
	const char *prep = "to be removed";
	const char *typename;
	int removed = 0;

	/* Distinguish what we are to remove */
	if (flags->argc && strchr(flags->args, 's')) {
		removelevel = RINGTYPE_SIG; /* signature */
		typename = "signature";
	} else if (flags->argc && strchr(flags->args, 'u')) {
		removelevel = RINGTYPE_NAME; /* user id */
		typename = "user ID";
	} else {
		removelevel = RINGTYPE_KEY; /* key */
		typename = "key";
	}

	/* Open pub and sec key rings */
	mainOpenPubSec(env, &pub, &pubring, &sec, &secring, stderr);
	if (!pubring) {
		fprintf (stderr,
"No keys in pubring\n");
		goto cleanup;
	}
	loadAllKeys(env, 1, 0);

	/* Select object to remove */
	fprintf (stderr, "\n");
	err = selectRingObject(env, argc, argv, pub, pubring, removelevel,
			prep, stderr, &obj);
	if (err <= 0)
		goto cleanup;

	/* err holds number of possible objects at that level */
	if (removelevel == RINGTYPE_NAME && err == 1) {
		err = 0;
		fprintf (stderr,
"Selected key has only one user ID, can't be selected %s\n", prep);
		goto cleanup;
	}

	/* Confirm removal */
	fprintf(stderr,
"\nThe following %s has been selected %s:\n", typename, prep);
	objPrint(env, obj, stderr, NULL);

	/* See if key is on private ring too */
	numseckeys = secring ? ringSetIsMember(secring, obj) : 0;
	if (numseckeys) {
		fprintf(stderr,
"\nThis %s is also present in the private keyring.\n", typename);
		fprintf(stderr,
"Removing it will remove it from both the public and private keyrings.\n"\
"Do you want to remove it from both keyrings (y/N)? ");
	} else {
		fprintf (stderr,
"\nAre you sure you want this %s %s (y/N)? ", typename, prep);
	}
	if (!pgpTtyGetBool(0, stderr)) {
		fprintf (stderr,
"Removal cancelled.\n");
		err = 0;
		goto cleanup;
	}
	fprintf (stderr, "\n");

	/* Perform removal, but don't write back yet */
	keyfile = ringSetCreate (ringpool);
	if (!keyfile) {
		err = ringPoolError(ringpool)->error;
		goto cleanup;
	}
	err = ringSetAddSet(keyfile, pubring);
	if (err < 0)
		goto cleanup;
	err = ringSetRemObject(keyfile, obj);
	if (err < 0)
		goto cleanup;
	ringSetFreeze (keyfile);
	removed = 1;

	if (numseckeys) {
		/* Remove from private key ring, don't write back yet */
		seckeyfile = ringSetCreate (ringpool);
		if (!seckeyfile) {
			err = ringPoolError(ringpool)->error;
			goto cleanup;
		}
		err = ringSetAddSet(seckeyfile, secring);
		if (err < 0)
			goto cleanup;
		err = ringSetRemObject(seckeyfile, obj);
		if (err < 0)
			goto cleanup;
		ringSetFreeze (seckeyfile);
	}
	ringObjectRelease(obj);
	
	/* Write out results */
	mainRingNewSet (pub, PGP_WRITETRUST_PUB, keyfile);
	keyfile = NULL;		/* Don't destroy this */
	if (err < 0) {
		fprintf (stderr,
"Error - unable to update key file!\n");
		goto cleanup;
	}
	if (numseckeys) {
		mainRingNewSet (sec, PGP_WRITETRUST_SEC, seckeyfile);
		seckeyfile = NULL;	/* Don't destroy this */
		if (err < 0) {
			fprintf (stderr,
"Error - unable to update private key file!\n");
			goto cleanup;
		}
	}
	/* success */
	err = 0;

cleanup:
	if (seckeyfile)
		ringSetDestroy(seckeyfile);
	if (keyfile)
		ringSetDestroy(keyfile);
	if (pubring)
		ringSetDestroy(pubring);
	if (secring)
		ringSetDestroy(secring);

	/*  If no error and something was removed, do a maintenance pass.
	This causes the (new) public keyring to be reopened and re-read,
	but gives us the correct community of keys to be validated. */

	if (err == 0 && removed) {
		/* Close and write out current keyrings before reopening */
		ringSetDestroy(allkeys);
		allkeys = 0;
		mainCloseKeyrings(1 /*update*/, 1 /*newvers*/);
		
		reloadAllKeys (env, 0, 1);
		err = mainDoMaint ((void *) ui_arg, allkeys, 0, NULL);
		fprintf (stderr,
"This %s has now been removed from the public key ring.\n", typename);
		if (numseckeys) {
			fprintf (stderr,
"This %s has also been removed from the private key ring.\n", typename);
		}
	}
	return err;
}


/*
* Issue a signature on a userID.
*/
static int
doKeySign (struct PgpEnv *env, struct Flags *flags, int argc, char *argv[],
	struct PgpTtyUI *ui_arg)
{
	struct RingSet *pubring = NULL;
	struct RingSet *newpub = NULL;
	struct RingSet *tmpring;
	struct RingIterator *iter;
	struct PgpSigSpec *sigspec = NULL;
	struct PgpSecKey *secretKey = NULL;
	union RingObject *obj=NULL, *ringobj = NULL;
	char const *pub;
	int level;
	int err;
	const char *prep = "to be signed";
	byte keyid[8];
	byte pkalg;
	int sigtype = PGP_SIGTYPE_KEY_GENERIC;
	char const *myname = pgpenvGetString (env, PGPENV_MYNAME,
						NULL, NULL);
	(void)flags;

	/* Open pub key ring */
	mainOpenPubSec(env, &pub, &pubring, NULL, NULL, stderr);
	if (!pubring) {
		fprintf (stderr,
"No keys in pubring\n");
		return 0;
	}
	loadAllKeys(env, 1, 0);

	/* Find secret key, make sure it signs */
	ringobj = ringLatestSecret (allkeys, myname, time(NULL),
				PGP_PKUSE_SIGN);
	if (!ringobj) {
		fprintf (stderr,
"Cannot find private key suitable for signing: %s\n", myname);
		ringSetDestroy(pubring);
		return 0;
	}
	secretKey = ringSecSecKey (allkeys, ringobj, PGP_PKUSE_SIGN);
	if (!secretKey) {
		fprintf (stderr, "Cannot convert to private key\n");
		ringSetDestroy(pubring);
		return 0;
	}
	if (!secretKey->sign) {
		fprintf (stderr, "Private Key is not a signature key\n");
		ringSetDestroy(pubring);
		return 0;
	}
	/* Data to check for duplicate sigs below */
	memcpy(keyid, secretKey->keyID, sizeof(keyid));
	pkalg = secretKey->pkAlg;

	/* Select object to sign */
	err = selectRingObject(env, argc, argv, pub, pubring, RINGTYPE_NAME,
			prep, stderr, &obj);
	if (err <= 0)
		return err;

	/* Make sure it doesn't already have a sig on it from this key */
	iter = ringIterCreate(pubring);
	level = ringIterSeekTo(iter, obj);
	if (level < 0) {
		ringSetDestroy(pubring);
		return level;
	}
	++level;
	while (ringIterNextObject(iter, level) > 0) {
		byte sigid[8];
		byte sigpkalg;
		union RingObject *sig = ringIterCurrentObject(iter, level);
		pgpAssert(sig);
		if (ringObjectType(sig) != RINGTYPE_SIG)
			continue;
		ringSigID8(pubring, sig, &sigpkalg, sigid);
		if (sigpkalg==pkalg &&
		memcmp(sigid, keyid, sizeof(sigid))==0 &&
		ringSigType(pubring, sig) == sigtype) {
			/* Duplicate */
			fprintf (stderr,
"User ID is already signed by your private key\n");
			ringIterDestroy(iter);
			ringSetDestroy(pubring);
			return 0;
		}
	}
	ringIterDestroy(iter);

	/* Confirm signature */
	objPrint(env, obj, stderr, NULL);
	fprintf(stderr,
"\n\n"\
"READ CAREFULLY:  Based on your own direct first-hand knowledge, are\n"\
"you absolutely certain that you are prepared to solemnly certify that\n"\
"the above public key actually belongs to the user specified by the\n"\
"above user ID (y/N)? ");

	if (!pgpTtyGetBool(0, stderr)) {
		fprintf (stderr,
"Key sign operation cancelled.\n");
		ringSetDestroy(pubring);
		return 0;
	}

	ui_arg->ringset = allkeys;
	err = pgpTtyUnlockSeckey (ui_arg, ringobj, secretKey,
"\nYou need a pass phrase to unlock your private key.\n");
	ringObjectRelease (ringobj);
	if (err) {
		fprintf (stderr, "Cannot unlock private key\n");
		ringSetDestroy(pubring);
		return 0;
	}
	
	
	/* Actually do the signature */
	sigspec = pgpSigSpecCreate (env, secretKey, sigtype);
	if (!sigspec) {
		ringSetDestroy(pubring);
		return PGPERR_NOMEM;
	}

	newpub = ringSetCreate (ringpool);
	if (!newpub) {
		pgpSigSpecDestroy(sigspec);
		ringSetDestroy(pubring);
		return PGPERR_NOMEM;
	}
	ringSetAddSet (newpub, pubring);
	ringSetDestroy(pubring);
	pubring = NULL;

	if (!rng)
		rng = pgpRandomCreate ();

	err = ringSignObject (newpub, obj, sigspec, rng);
	pgpSigSpecDestroy(sigspec);
	ringSetFreeze(newpub);
	if (err < 0)
		return err;

	/* Do maintenance pass */
	fprintf (stderr, "\n");
	reloadAllKeys (env, 0, 1);
	tmpring = ringSetUnion(allkeys, newpub);
	ringSetDestroy(allkeys);
	allkeys = tmpring;
	err = mainDoMaint ((void *) ui_arg, allkeys, 1, allkeys);
	if (err < 0)
		return err;

	/* Write out results */
	mainRingNewSet (pub, PGP_WRITETRUST_PUB, newpub);
	newpub = NULL;		/* Don't destroy this */
	fprintf (stderr,
"Key signature certificate added.\n");

	return 0;
}

/*
* Return true if a key has been revoked.
*/
static int
mainKeyRevoked (struct RingSet const *set, union RingObject *obj)
{
	struct RingIterator *iter;
	byte keyid[8], keypkalg;
	int level;

	pgpAssert (ringObjectType(obj) == RINGTYPE_KEY);
	ringKeyID8(set, obj, &keypkalg, keyid);

	iter = ringIterCreate(set);
	level = ringIterSeekTo(iter, obj);
	if (level < 0) {
		ringIterDestroy(iter);
		return level;
	}
	++level;
	while (ringIterNextObject(iter, level) > 0) {
		byte sigid[8];
		byte sigpkalg;
		int sigtype;
		union RingObject *sig = ringIterCurrentObject(iter, level);
		pgpAssert(sig);
		if (ringObjectType(sig) != RINGTYPE_SIG)
			continue;
		ringSigID8(set, sig, &sigpkalg, sigid);
		if (memcmp(keyid,sigid,sizeof(keyid))!=0 ||
		sigpkalg != keypkalg)
			continue;
		sigtype = ringSigType(set, sig);
		if (sigtype == PGP_SIGTYPE_KEY_COMPROMISE) {
			ringIterDestroy(iter);
			return 1; /* yes, it is revoked */
		}
	}
	ringIterDestroy(iter);
	return 0;	 	/* no, it is not revoked */
}


/*
* Given a userID signature, return true if it has been revoked.
* @@@ Better to worry more about validity here?
*/
static int
mainSigRevoked (struct RingSet const *set, union RingObject *obj)
{
	struct RingIterator *iter;
	byte keyid[8], pkalg;
	int level;

	pgpAssert (ringObjectType(obj) == RINGTYPE_SIG);
	ringSigID8(set, obj, &pkalg, keyid);
	
	/* Prepare to iterate */
	iter = ringIterCreate(set);
	if (!iter)
		return PGPERR_NOMEM;
	level = ringIterSeekTo(iter, obj);
	if (level < 0) {
		ringIterDestroy(iter);
		return level;
	}

	/* Loop over all sigs on the name */
	ringIterRewind(iter, level);
	while ((level = ringIterNextObject (iter, level)) > 0) {
		union RingObject *sig = ringIterCurrentObject(iter, level);
		byte sigpkalg, sigid[8];
		pgpAssert(sig);
		if (ringObjectType(sig) != RINGTYPE_SIG)
			continue;
		ringSigID8(set, sig, &sigpkalg, sigid);
		if (sigpkalg==pkalg &&
		memcmp(sigid, keyid, sizeof(sigid))==0) {
			int type = ringSigType(set, sig);
			if (type == PGP_SIGTYPE_KEY_UID_REVOKE) {
				ringIterDestroy(iter);
				ringObjectRelease(sig);
				return 1; /* revocation found */
			}
		}
	}
	ringIterDestroy(iter);
	return 0;	 	/* no revocation found */
}

/*
* Revoke a key or a signature
*/
static int
doKeyRevoke (struct PgpEnv *env, struct Flags *flags, int argc, char *argv[],
	struct PgpTtyUI *ui_arg)
{
	struct RingSet *pubring = NULL;
	struct RingSet *newpub = NULL;
	struct RingSet *tmpring = NULL;
	struct RingIterator *iter;
	struct PgpSigSpec *sigspec = NULL;
	struct PgpSecKey *secretKey = NULL;
	union RingObject *obj=NULL, *seckeyobj = NULL;
	char const *pub;
	int level;
	int err;
	const char *prep = "to be revoked";
	int revokelevel;
	int sigtype;

	/* Distinguish what we are to revoke */
	if (flags->argc && strchr(flags->args, 's')) {
		revokelevel = RINGTYPE_SIG; /* signature on a name */
		sigtype = PGP_SIGTYPE_KEY_UID_REVOKE;
	} else {
		revokelevel = RINGTYPE_KEY; /* key */
		sigtype = PGP_SIGTYPE_KEY_COMPROMISE;
	}

	/* Open pub key ring */
	mainOpenPubSec(env, &pub, &pubring, NULL, NULL, stderr);
	if (!pubring) {
		fprintf (stderr,
"No keys in pubring\n");
		return 0;
	}
	loadAllKeys(env, 1, 0);

	/* Select object to sign */
	err = selectRingObject(env, argc, argv, pub, pubring, revokelevel,
			prep, stderr, &obj);
	if (err <= 0) {
		ringSetDestroy(pubring);
		return err;
	}

	/* Display it for him */
	objPrint(env, obj, stderr, NULL);

	if (revokelevel == RINGTYPE_KEY) {
		/* Make sure it is our key */
		seckeyobj = obj;
		secretKey = ringSecSecKey (allkeys, obj, 0);
		if (secretKey == NULL) {
			fprintf (stderr,
"You don't have the private key corresponding to that key\n");
			ringSetDestroy(pubring);
			return 0;
		}
		/* Make sure it hasn't already been revoked. */
		if (mainKeyRevoked(pubring, obj)) {
			fprintf (stderr,
"That key has already been revoked\n");
			ringSetDestroy(pubring);
			return 0;
		}
	} else {
		/* Make sure we have a secret key for this sig */
		byte keyid[8], pkalg;
		ringSigID8(pubring, obj, &pkalg, keyid);
		seckeyobj = ringKeyById8(allkeys, pkalg, keyid);
		if (seckeyobj == NULL) {
			fprintf (stderr,
"\nYou don't have any key information for that signature.\n");
			ringSetDestroy(pubring);
			return 0;
		}
		secretKey = ringSecSecKey (allkeys, seckeyobj, PGP_PKUSE_SIGN);
		if (!secretKey) {
			fprintf (stderr,
"\nYou don't have the private key which made that signature.\n");
			ringSetDestroy(pubring);
			return 0;
		}
		/* Make sure it hasn't already been revoked */
		if (mainSigRevoked(pubring, obj)) {
			fprintf (stderr,
"That signature has already been revoked.\n"\
"Are you sure you want to add another revocation certificate (y/N)? ");
			if (!pgpTtyGetBool(0, stderr)) {
				fprintf (stderr, "\
Signature revocation cancelled.\n");
				ringSetDestroy(pubring);
				return 0;
			}
		}
	}

	if (!secretKey->sign) {
		fprintf (stderr,
"Private key is not a signature key\n");
		ringSetDestroy(pubring);
		return 0;
	}

	/* Confirm signature */
	if (revokelevel == RINGTYPE_KEY) {
		fprintf(stderr,
"\n"\
"Do you want to permanently revoke your public key\n"\
"by issuing a secret key compromise certificate on this key (y/N)? ");
	} else {
		fprintf(stderr,
"\n"\
"Do you want to revoke this signature (y/N)? ");
	}

	if (!pgpTtyGetBool(0, stderr)) {
		fprintf (stderr,
"Revoke cancelled.\n");
		ringSetDestroy(pubring);
		return 0;
	}

	if (revokelevel == RINGTYPE_SIG) {
		/* Get parent of signature object to receive new sig */
		iter = ringIterCreate(pubring);
		if (iter == NULL) {
			ringSetDestroy(pubring);
			return PGPERR_NOMEM;
		}
		level = ringIterSeekTo(iter, obj);
		pgpAssert(level > 0);
		obj = ringIterCurrentObject(iter, level-1);
		pgpAssert(obj);
		ringIterDestroy(iter);
	}

	ui_arg->ringset = allkeys;
	err = pgpTtyUnlockSeckey (ui_arg, seckeyobj, secretKey,
"\nYou need a pass phrase to unlock your private key.\n");
	ringObjectRelease (seckeyobj);
	if (err) {
		fprintf (stderr, "Cannot unlock private key\n");
		ringSetDestroy(pubring);
		return 0;
	}
	fprintf (stderr, "\n");
	
	/* Actually do the signature */
	sigspec = pgpSigSpecCreate (env, secretKey, sigtype);
	if (!sigspec) {
		ringSetDestroy(pubring);
		return PGPERR_NOMEM;
	}

	newpub = ringSetCreate (ringpool);
	if (!newpub) {
		pgpSigSpecDestroy(sigspec);
		ringSetDestroy(pubring);
		return PGPERR_NOMEM;
	}
	ringSetAddSet (newpub, pubring);
	ringSetDestroy(pubring);
	pubring = NULL;

	if (!rng)
		rng = pgpRandomCreate ();

	err = ringSignObject (newpub, obj, sigspec, rng);
	pgpSigSpecDestroy(sigspec);
	ringSetFreeze(newpub);
	if (err < 0)
		return err;

	/* Do maintenance pass */
	reloadAllKeys (env, 0, 1);
	tmpring = ringSetUnion(allkeys, newpub);
	ringSetDestroy(allkeys);
	allkeys = tmpring;
	err = mainDoMaint ((void *) ui_arg, allkeys, 1, allkeys);
	if (err < 0)
		return err;

	/* Write out results */
	mainRingNewSet (pub, PGP_WRITETRUST_PUB, newpub);
	newpub = 0;	/* Don't destroy this */
	fprintf (stderr,
"Key revocation certificate added.\n");

	return 0;
}

static int
mainProcessFlags (struct Flags *flags, struct PgpEnv *env, int argc,
		char *argv[], PgpTtyUI *ui_arg)
{
	switch (flags->opt) {
	case 'a':
		return doKeyAdd (env, argc, argv, ui_arg);
		break;
	case 'c':
	return doKeyCheck (env, flags, argc, argv, ui_arg);
		break;
	case 'd':
	return doKeyDisable (env, argc, argv, ui_arg);
		break;
	case 'g':
		return doKeyGenerate (env, argc, argv, ui_arg);
		break;
	case 'k':
		return doKeyRevoke (env, flags, argc, argv, ui_arg);
		break;
	case 'l':
		return doKeyList (env, flags, argc, argv, ui_arg);
		break;
	case 'r':
		return doKeyRemove (env, flags, argc, argv, ui_arg);
		break;
	case 's':
		return doKeySign (env, flags, argc, argv, ui_arg);
		break;
	case 'x':
		return doKeyExtract (env, flags, argc, argv, ui_arg);
		break;
	case 'e':
		return doKeyEdit (env, flags, argc, argv, ui_arg);
		break;
	default:
		exitUsage(0);
	}
	/* NOTREACHED */
	return 0;
}

/* Clean up allocated space (like the pipeline and the rng) on exit */
static void
mainExit (void)
{
	if (passcache)
		pgpPassCacheDestroy (passcache);
	passcache = NULL;

	if (ringpool)
		ringPoolDestroy (ringpool);
	ringpool = NULL;

	if (rng)
		pgpRandomDestroy (rng);
	rng = NULL;

	return;
}

int
appMain (int argc, char *argv[])
{
	struct PgpUICb ui;
	struct PgpTtyUI ui_arg;
	struct Flags flags;
	struct PgpEnv *env;
	int retval;
	
	atexit(mainExit);

	/* Setup the UI callback functions */
ui.message = pgpTtyMessage;
ui.doCommit = pgpTtyDoCommit;
ui.newOutput = pgpTtyNewOutput;
ui.needInput = pgpTtyNeedInput;
	ui.sigVerify = pgpTtySigVerify;
	ui.eskDecrypt = pgpTtyEskDecrypt;
ui.annotate = NULL;

	/* Setup the TTY UI argument */
ui_arg.verbose = 1;
ui_arg.fp = stdout;
	ui_arg.ringset = NULL;
	ui_arg.showpass = 0;
	ui_arg.commits = -1;

	/* Initialize the application from config files, etc. */
	retval = pgpInitApp (&env, &argc, &ui, &ui_arg, EXIT_PROG_PGPK, 1);
	if (retval) {
		fprintf (stderr, "pgpInitApp failed: %d\n", retval);
		goto error;
	}

	/* Now initialize the values that need initialization */
	ui_arg.env = env;
	ui_arg.passcache = passcache = pgpPassCacheCreate (env);

	/* Check the exiration date on the application */
	exitExpiryCheck (env);

#if defined(UNIX) || defined(VMS)
	umask(077); /* Make files default to private */
#endif

	memset (&flags, 0, sizeof (flags));
	mainParseArgs (&ui, &ui_arg, env, &argc, argv, &flags);

	/*
	* Set variables which may have been set on the command line.
	*/
	ui_arg.showpass = pgpenvGetInt (env, PGPENV_SHOWPASS, NULL, NULL);
	flags.doarmor = pgpenvGetInt (env, PGPENV_ARMOR, NULL, NULL);

	/* Open the keyrings and process the flags */
	ringpool = ringPoolCreate (env);
	if (!ringpool) {
		fprintf (stderr, "Cannot create RingPool\n");
		retval = PGPERR_NOMEM;
		goto error;
	}

	retval = mainProcessFlags (&flags, env, argc, argv, &ui_arg);
	if (retval) {
		fprintf (stderr, "Error: %s\n", pgperrString (retval));
	}

	if (allkeys) {
		ringSetDestroy(allkeys);
		allkeys = 0;
	}
	
error:
	return retval;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/

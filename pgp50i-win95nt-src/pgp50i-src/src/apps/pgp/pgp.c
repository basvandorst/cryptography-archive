/*
 * pgp.c - PGP Main routine
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * Extensively rewritten by:  Brett A. Thomas (bat@pgp.com, quark@baz.com)
 *
 * Others involved include Hal Finney, Colin Plumb and Mark Weaver.
 *
 * $Id: pgp.c,v 1.1.2.4.2.9 1997/07/15 21:26:19 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

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
#include <sys/stat.h>		/* for umask() */
#endif
#if _MSC_VER>=1000
#include <limits.h>		/* for PATH_MAX */
#endif
#include <ctype.h>

#include "pgpDebug.h"
#include "pgpArmrFil.h"
#include "pgpBufMod.h"
#include "pgpCipher.h"
#include "pgpConvKey.h"
#include "pgpDecPipe.h"
#include "pgpEncPipe.h"
#include "pgpFIFO.h"
#include "pgpFileMod.h"
#include "pgpFileNames.h"
#include "pgpFileType.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpPassCach.h"
#include "pgpPipeline.h"
#include "pgpConf.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpFile.h"
#include "pgpUI.h"
#include "pgpPubKey.h"
#include "pgpRndom.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpSigSpec.h"
#include "pgpOutput.h"
#include "pgpUserIO.h"
#include "pgpRingUI.h"
#include "pgpUsuals.h"
#include "pgpTrstPkt.h"
#include "pgpTrust.h"

#include "pgpExit.h"
#include "pgpAppFile.h"
#include "pgpKeyRings.h"
#include "pgpInitApp.h"
#include "pgpOpt.h"
#include "pgpRndPool.h"

#define PGP_MAIN /*So globals get assigned*/
#include "parsearg.h"
#include "pass.h"


typedef enum _PGPInvokedType {
    Unknown = 0,
    PGPencrypt,
    PGPsign,
    PGPverify,
    PGPold,
    PGPraw,
    PGPkeys
}PGPInvokedType;

static PGPInvokedType getInvokedType(char *Argv0);
static void exitInvokedRaw(void);

static struct PgpPipeline *head = NULL;
static struct PgpRandomContext *rng = NULL;
static struct PgpSecKey *secretKey = NULL;
static struct PgpPubKey *publicKey = NULL;
static struct RingPool *ringpool = NULL;

static struct PgpFile *
openFile (void *arg, void const *base, unsigned num)
{
	FILE *fp;
	char filename[PATH_MAX+1];
	char n[20];

	memcpy (filename, base, strlen ((char *)base)+1);
#ifdef MSDOS
	if (num > 1) {
		char *name = fileNameContract(filename);
		strcpy (filename, name);
		pgpMemFree(name);
		if (num < 10)
			sprintf (n, ".as%d", num);
		else if (num < 100)
			sprintf (n, ".a%d", num);
		else {
			pgpAssert (num < 1000);
			sprintf (n, ".%d", num);
		}
		strcat (filename, n);
	}
#else
	if (num) {
		sprintf (n, ".%d", num);
		strcat (filename, n);
	}
#endif
	/* Do check for overwrite on single part file, or first part of
	   multi-part file only. */
	if (num <= 1 && pgpTtyCheckOverwrite (arg, filename))
	        return NULL;
	StatusOutput(TRUE, "CREATING_OUTPUT_FILE", filename);
	fp = fopen (filename, "wb");
	if (!fp)
		return NULL;
	return pgpFileWriteOpen (fp, NULL);
}

typedef struct AlgVotes
{
	int		numAlgs;
	int		numVoters;

	/* Arrays indexed by algorithm number */
	int *	algOkay;	/* Number of voters who can accept indexed alg */
	int *	algVotes;	/* Preference voting, lower is better */
} AlgVotes;

static PGPError
InitAlgVotes(AlgVotes *algVotes)
{
	size_t	arraySize;

	algVotes->numAlgs = 1;
	algVotes->numVoters = 0;

	while (pgpCipherByNumber(algVotes->numAlgs))
		algVotes->numAlgs++;
	arraySize = sizeof(int) * algVotes->numAlgs;

	algVotes->algOkay = (int *)pgpAlloc(arraySize);
	if (algVotes->algOkay == NULL)
		return PGPERR_NOMEM;

	algVotes->algVotes = (int *)pgpAlloc(arraySize);
	if (algVotes->algVotes == NULL)
	{
		pgpFree(algVotes->algOkay);
		return PGPERR_NOMEM;
	}

	pgpClearMemory(algVotes->algOkay, arraySize);
	pgpClearMemory(algVotes->algVotes, arraySize);

	return PGPERR_OK;
}

static void
CleanupAlgVotes(AlgVotes *algVotes)
{
	pgpFree(algVotes->algOkay);
	pgpFree(algVotes->algVotes);
}

static void
AccumAlgVotes(AlgVotes *algVotes, byte const *prefAlgs, size_t numPrefAlgs)
{
	size_t	i;

	algVotes->numVoters++;
	if (prefAlgs && numPrefAlgs > 0)
	{
		for (i = 0; i < numPrefAlgs; i++)
			if (prefAlgs[i] < algVotes->numAlgs)
			{
				algVotes->algVotes[prefAlgs[i]] += i;
				algVotes->algOkay[prefAlgs[i]]++;
			}
	}
	else
	{
		algVotes->algOkay[PGP_CIPHER_IDEA]++;
	}
}

/* Returns voted algorithm, or 0 if none is acceptable */
static byte
ChooseAlgFromVotes(AlgVotes const *algVotes)
{
	int		i;
	int		bestAlg = 0;
	int		bestVote = INT_MAX;

	for (i = 1; i < algVotes->numAlgs; i++)
	{
		if (algVotes->algOkay[i] == algVotes->numVoters &&
				algVotes->algVotes[i] < bestVote)
		{
			bestVote = algVotes->algVotes[i];
			bestAlg = i;
		}
	}
	return bestAlg;
}

/* Call pgpk to add keys */
static int
mainAddKeys (void *arg, char const *filename)
{
	static char const *prefix = "pgpk -a ";
	struct PgpTtyUI *ui_arg = (struct PgpTtyUI *)arg;
	struct RingSet *tset;
	char *buf;
	int err;

	(void)arg;

	buf = pgpMemAlloc(strlen(prefix) + strlen(filename) + 1);
	if (!buf)
		return PGPERR_NOMEM;
	
	/*
	 * Run pgpk with keyrings closed.  This is for two reasons.
	 * First, some OS's won't allow renaming and such of open files
	 * (e.g. MSDOS) and so we must close ours so pgpk can add keys.
	 * Second, this way when we re-open our keyrings we will get
	 * the new ones with the keys added, which may help in further
	 * processing the input.
	 */
	mainCloseKeyrings (1, 0);
	
	StatusOutput(TRUE, "COPYING_KEYFILE_AND_RUNNING_PGPK", filename);
	strcpy(buf, prefix);
	strcat(buf, filename);
	fprintf(stderr, "%s\n", buf);
	err = system(buf);
	pgpMemFree(buf);

	/* Re-open keyrings */
	(void)mainOpenKeyrings (ui_arg->env, ringpool, 0, &tset);
	ui_arg->ringset = tset;

	if (err)
		return PGPERR_GENERIC;

	return 0;
}

static size_t
mainGetPass (char *passp, struct UIArg *ui_arg)
{
	char pass[PASSLEN];
	char pass2[PASSLEN];
	int showpass = 0;

	showpass = pgpenvGetInt(ui_arg->arg.env, PGPENV_SHOWPASS, NULL, NULL);
	
	InteractionOutput(TRUE, "NEED_PASSPHRASE");
	if (pgpenvGetInt (ui_arg->arg.env, PGPENV_BATCHMODE, NULL, NULL))
	        return 0;
	do {
		memset (pass, 0, sizeof (pass));
		memset (pass2, 0, sizeof (pass2));

		pgpTtyGetPass (showpass, pass, sizeof(pass));
		InteractionOutput(TRUE, "NEED_PASSPHRASE_AGAIN");
		pgpTtyGetPass (showpass, pass2, sizeof(pass2));

		if (!strcmp (pass, pass2))
			break;
		InteractionOutput(TRUE, "PASSPHRASES_DIFFERENT");
	} while (1);

	if (!strlen (pass)) {
		ErrorOutput(TRUE, LEVEL_CRITICAL, "ZERO_LEN_PASSPHRASE");
		return 0;
	}
	memcpy (passp, pass, sizeof (pass));

	memset (pass2, 0, sizeof (pass2));
	memset (pass, 0, sizeof(pass));

	return strlen ((char *)passp);
}

/*
 * Setup a pipeline based upon the arguments in the flags
 */
static struct PgpPipeline **
mainSetupPipeline (struct PgpPipeline **pipehead,
		   struct PgpEnv *env, struct Flags *flags,
		   struct PgpRandomContext *rng_p, char const *in_name,
		   struct PgpFileRead *readhead, struct UIArg *ui_arg)
{
	struct PgpPipeline *mod = NULL, **tail = &mod;
	struct PgpConvKey convkey, *convkeys = NULL;
	struct PgpSigSpec *sigspec = NULL;
	struct PgpLiteralParams lP, *literalParams = &lP;
	byte const *ptr;
	size_t len;
	extern Boolean RngSeeded;

	if (flags->moreflag)
		literalParams->filename = "_CONSOLE";
	else
		literalParams->filename = in_name;

	if (flags->conventional) {
		size_t passlen;

		/* Use passphrase given with -z, else ask for one */
		if (phrase[0])
			passlen = strlen(phrase);
		else
			passlen = mainGetPass (phrase, ui_arg);

		if (!passlen)
			exitCleanup (-1);

		convkey.next = NULL;
		convkey.stringToKey = 0;
		convkey.pass = phrase;
		convkey.passlen = passlen;
		convkeys = &convkey;
	}

	/*
	 * Check the file to see if this is text or binary file.  If
	 * it binary, then force binary mode and check for
	 * compressibility.  If it is not compressible, turn off
	 * compression, too.
	 *
	 * If it is a PGP file, ask the user if it should be treated
	 * as a PGP file.  If it should, then ignore the literal packet
	 * type.
	 */
	ptr = pgpFileReadPeek (readhead, &len);
	pgpRandomAddBytes (rng_p, ptr, len);

	if (pgpFileTypeBinary (pgpenvGetString (env, PGPENV_LANGUAGE,
						NULL, NULL),
			       ptr, len)) {
		struct PgpFileType const *ft;

		pgpenvSetInt (env, PGPENV_TEXTMODE, 0, PGPENV_PRI_FORCE);

		ft = pgpFileType (ptr, len);
		if (ft && !ft->compressible)
			pgpenvSetInt (env, PGPENV_COMPRESS, 0,
				      PGPENV_PRI_FORCE);

		if (pgpFileTypePGP (ptr, len)) {

			if (!pgpenvGetInt (ui_arg->arg.env, PGPENV_BATCHMODE,
					   NULL, NULL)) {
				InteractionOutput(TRUE, "TREAT_AS_PGP");
				if (pgpTtyGetBool (0, 1))
				        /* No literal if this is a PGP msg */
				        literalParams = NULL;
			}
		}
	}

	if (flags->sign) {
		union RingObject *ringobj;
		char const *myname = pgpenvGetString (env, PGPENV_MYNAME,
						      NULL, NULL);
		int err, arg;

		if (!secretKey) {
			ringobj = ringLatestSecret (ui_arg->arg.ringset,
						    myname, time(NULL),
						    PGP_PKUSE_SIGN);
			if (!ringobj) {
				ErrorOutput(TRUE,
							LEVEL_CRITICAL,
							"PRIVATE_KEY_MISSING",
							myname);
				exitCleanup (-1);
			}

			secretKey = ringSecSecKey (ui_arg->arg.ringset,
						   ringobj, PGP_PKUSE_SIGN);
			ringObjectRelease (ringobj);
			if (!secretKey) {
				ErrorOutput(TRUE,
							LEVEL_CRITICAL,
							"CANNOT_CONVERT_TO_PRIVATE_KEY");
				exitCleanup (-1);
			}

			if (!secretKey->sign) {
				ErrorOutput(TRUE, LEVEL_CRITICAL, "PRIVATE_KEY_CANNOT_SIGN");
				exitCleanup (-1);
			}

			err = pgpTtyUnlockSeckey (&ui_arg->arg, ringobj,
									  secretKey,
									  "PRIVATE_KEY_NEEDED_FOR_SIGNATURE");
			if (err) {
				ErrorOutput(TRUE,
							LEVEL_CRITICAL,
							"CANNOT_UNLOCK_PRIVATE_KEY");
				exitCleanup (err);
			}
		}

		arg = pgpenvGetInt (env, PGPENV_TEXTMODE, NULL, NULL);
		arg = arg ? PGP_SIGTYPE_TEXT : PGP_SIGTYPE_BINARY;

		sigspec = pgpSigSpecCreate (env, secretKey, arg);

		if(!RngSeeded && !pgpRandPoolEntropy()) {
			pgpTtyRandAccum (ui_arg, 196);
			RngSeeded = TRUE;
		}
	}

	/* This will find *ALL* keys that match *ALL* recipients */
	if (flags->numrecips) {
		struct RingPool *		pool = ringSetPool (ui_arg->arg.ringset);
		int						i = flags->numrecips;
		int						e;
		struct RingSet *		ringset;
		struct RingIterator *	ringiter;
		union RingObject *		ringobj;
		struct PgpPubKey *		temp;
		AlgVotes				algVotes;
		byte const *			prefAlgs;
		size_t					numPrefAlgs;
		byte					bestAlg;

		if (!pool) {
			ErrorOutput(TRUE,
						LEVEL_CRITICAL,
						"NO_KEYRINGS");
			exitUsage (-1);
		}

		if (! publicKey) {
			char const *companyKey;

			ringset = ringSetCreate (pool);
			if (!ringset)
				exitUsage (ringPoolError (pool)->error);
			while (i--) {
				e = ringSetFilterSpec (ui_arg->arg.ringset,
						       ringset,
						       flags->recips[i],
						       PGP_PKUSE_ENCRYPT);
				if (e==0) {
					ErrorOutput(TRUE,
								LEVEL_CRITICAL,
								"NO_ENCRYPTION_KEYS_FOUND_FOR",
								flags->recips[i]);
					exitCleanup(-1);
				}
				if (e <= 0)
					exitUsage(e);

				if(e > 1)
				{
					WarningOutput(TRUE,
								  LEVEL_INFORMATION,
								  "MULTIPLE_RECIPIENTS_MATCHED",
								  e,
								  flags->recips[i]);
				}
			}
			if (pgpenvGetInt (env, PGPENV_ENCRYPTTOSELF,
					  NULL, NULL)) {
				char const *myname =
					pgpenvGetString (env, PGPENV_MYNAME,
							 NULL, NULL);
				/* Add myself to the list of recipients */

				ringobj = ringLatestSecret
					(ui_arg->arg.ringset, myname,
					 time(NULL), PGP_PKUSE_ENCRYPT);
				if (!ringobj) {
					ErrorOutput(TRUE,
								LEVEL_CRITICAL,
								"CANNOT_FIND_KEY",
								myname);
					exitCleanup (-1);
				}
				if (ringSetAddObject (ringset, ringobj)) {
					ErrorOutput(TRUE,
								LEVEL_CRITICAL,
								"CANNOT_ADD_MY_KEY");
					exitCleanup (-1);
				}
			}
			/*
			 * This is our version of "Commercial Key Escrow".
			 *
			 * A company can set the CompanyKey in the site-wide
			 * configuration file and it will be added to the
			 * recipient list for all encrypted messages.  Then
			 * again, the user can override the setting in their
			 * own pgp.cfg or on the command line.
			 */
			companyKey = pgpenvGetString (env, PGPENV_COMPANYKEY,
						      NULL, NULL);
			if (companyKey && *companyKey) {
				/* Add the company key to the recipient list */
				e = ringSetFilterSpec (ui_arg->arg.ringset,
						       ringset,
						       companyKey,
						       PGP_PKUSE_ENCRYPT);
				if (e <= 0)
					exitUsage(e);
			}
			ringSetFreeze (ringset);
			ringiter = ringIterCreate (ringset);
			if (!ringiter)
				exitUsage (ringPoolError (pool)->error);

			e = InitAlgVotes(&algVotes);
			if (e)
				exitUsage(e);

			while ((e = ringIterNextObject (ringiter, 1)) > 0) {
			  ringobj = ringIterCurrentObject(ringiter, 1);
			  if (ringObjectType (ringobj) != RINGTYPE_KEY) {
#if 0
			    fprintf (stderr,
				     "Got a ring object of type %d\n",
				     ringObjectType (ringobj));
#endif
			    continue;
			  }
			  /* Determine whether this key is OK to use
			     for encryption */
			  if (ringTtyKeyOKToEncrypt (ringset,
										 ringobj)) {
				prefAlgs = ringKeyFindSubpacket (ringobj, ringset,
							SIGSUB_PREFERRED_ALGS,
				            0, &numPrefAlgs, NULL, NULL, NULL, NULL, &e);
				if (prefAlgs == NULL && e != 0)
					exitUsage(e);
				AccumAlgVotes(&algVotes, prefAlgs, numPrefAlgs);

			    temp = ringKeyPubKey (ringset, ringobj,
						  PGP_PKUSE_ENCRYPT);
			    if (temp) {
			      temp->next = publicKey;
			      publicKey = temp;
			    }
			  }
			}
			bestAlg = ChooseAlgFromVotes(&algVotes);
			CleanupAlgVotes(&algVotes);
			if (bestAlg > 0)
			{
				pgpenvSetInt(env, PGPENV_CIPHER, bestAlg, PGPENV_PRI_FORCE);
			}
			else
			{
				ErrorOutput(TRUE,
							LEVEL_CRITICAL,
							"INCONSISTENT_RECIPIENT_SET");
				exitCleanup (-1);
			}

			ringIterDestroy (ringiter);
			ringSetDestroy (ringset);
			if (e < 0)
			  exitUsage(e);
		}
	}

	/*
	 * Don't proceed if couldn't encrypt for anyone, otherwise we send
	 * out plaintext.
	 */
	if (flags->numrecips && !publicKey) {
		ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_VALID_RECIPIENTS");
		exitCleanup (-1);
	}


	tail = pgpEncryptPipelineCreate (tail,
									 env,
									 NULL,
									 rng_p,
									 convkeys,
									 publicKey,
									 sigspec,
									 literalParams,
									 flags->sepsig);

	if (tail)
		*pipehead = mod;
	return tail;
}

/*
 * Process the flags that were set.  This builds the appropriate
 * pipelines and processes the input files.
 */
static int
mainProcessFlags (struct PgpUICb *ui, struct UIArg *ui_arg,
		  struct PgpEnv *env, int argc, char *argv[],
		  struct Flags *flags)
{
	struct PgpPipeline mod;
	struct PgpPipeline **tail = NULL;
	struct PgpEnv *tempenv = NULL;
	int retval = PGPERR_OK, err;
	int encrypting;
	int armoring;
	int filter = flags->filtermode;	/* Assign to 0 or 2 */
	
	if (flags->conventional && flags->numrecips) {
		ErrorOutput(TRUE,
					LEVEL_CRITICAL,
					"CANNOT_COMBINE_CONVENTIONAL_AND_PK");
		exitCleanup(PGPEXIT_ARGS);
	}

	if (!argc) {
		filter++;
		StatusOutput(TRUE, "USING_STDIN");
		argc = -1;
	}

	memset (&mod, 0, sizeof (mod));
	mod.name = "PGP Application";

	rng = pgpRandomCreate ();
	if (!rng)
		exitUsage (PGPERR_NOMEM);

	encrypting = (flags->numrecips || flags->conventional ||
		      (flags->doarmor && !flags->decrypt) ||
		      flags->sign); /* Encryption */
	armoring = pgpenvGetInt (env, PGPENV_ARMOR, NULL, NULL);

	/*
	 * Loop through input files.
	 */
	for (; argc; argc--, argv++) {
		FILE *input;
		struct PgpFileRead *context = NULL;
		char name[PATH_MAX+1];
		char *fn = NULL;
		char const *outfile = NULL;

		if (argc < 0) {
			pgpAssert (filter & 1);
			argc = 1;
			input = stdin;
		} else {
			input = fopen (argv[0], "rb");
			if (!input) {
				ErrorOutput(TRUE,
							LEVEL_CRITICAL,
							"CANNOT_OPEN_INPUT_FILE",
							argv[0]);
				continue;
			}
		}

		if (!tempenv)
			tempenv = pgpenvCopy (env);
		if (!tempenv) {
			retval = PGPERR_NOMEM;
			break;
		}			

		context = pgpFileReadCreate (input, ((filter & 1) ? 0 : 1));
		if (!context) {
			fclose (input);
			retval = PGPERR_NOMEM;
			break;
		}

		if (flags->decrypt) {
			if (flags->doarmor) {
				/* Want to just strip off ascii armor */
				ui_arg->arg.commits = 0;
			} else {
				/* Unwind just one layer of PGP */
				ui_arg->arg.commits = 1;
			}
		} else {
			/* Unwind all layers if decrypting */
			ui_arg->arg.commits = -1;
		}

		if (encrypting) {
			char const *in_name;

			if (filter & 1)
				in_name = "stdin";
			else
				in_name = argv[0];
			
			if (flags->outfile)
				outfile = flags->outfile;
			else if (filter) {
				outfile = NULL;
			} else {
				char *outname = argv[0];
#ifdef MSDOS
				outname = fileNameContract(outname);
#endif
				fn = fileNameExtend (outname,
						     (armoring ? ".asc" :
						      (flags->sign &&
						       flags->sepsig) ? ".sig"
						      : ".pgp"));
				strncpy (name, fn, sizeof (name));
				pgpMemFree (fn);
				outfile = name;
			}

			tail = mainSetupPipeline (&head, tempenv,
						  flags, rng, in_name,
						  context, ui_arg);
		} else {	/* Decryption */
			/* Choose an output name */
			if (!ui_arg->arg.protect_name) {
				/* Allow -o to override -f */
				if (flags->outfile) {
					ui_arg->arg.outname = flags->outfile;
				} else if (filter) {
					/* use stdout */
					ui_arg->arg.protect_name = -1;
				} else {
					fn = fileNameContract (argv[0]);
					strncpy (name, fn, sizeof (name));
					pgpMemFree (fn);
					ui_arg->arg.outname = name;
				}
			}
			if (!head) {
				tail = &head;
				if (filter & 2)	/* if -f is used */
					tail = pgpBufferModCreate (
						tail, &pgpFlexFifoDesc);
				tail = pgpDecryptPipelineCreate (tail, tempenv,
								 NULL, ui,
								 ui_arg);
				
				if (!head)
					exitCleanup (PGPERR_NOMEM);
			}
		}

		if (!tail) {
			ErrorOutput(TRUE,
						LEVEL_CRITICAL,
						"CANNOT_SETUP_PROCESSING_PIPELINE");
			exitCleanup (-1);
		}

		if (encrypting) {
			if (!outfile) {
				pgpAssert (filter);
				if (!pgpFileWriteCreate (tail,
							 pgpFileWriteOpen
							 (stdout, NULL), 0)) {
					head->teardown (head);
					head = NULL;
					continue;
				}
			} else {
				if (!pgpArmorFileWriteCreate (tail, openFile,
			  	          (void *) env, outfile)) {

					head->teardown (head);
					head = NULL;
					continue;
				}
			}
		}

		retval = pgpFileReadPump (context, head);
		if (retval)
			PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, retval);

		if (retval) {
			err = pgpFileReadClose (context, head);
			if (err) {
				PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, err);
			}
		}
		pgpFileReadDestroy (context);

		if (encrypting) {
			if (!retval) {
				retval = head->sizeAdvise (head, 0);
				if (retval)
					PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, retval);
			}
			head->teardown (head);
			head = NULL;
		}

		if (retval)
			break;
	}
	if (!retval && head) {
		retval = head->sizeAdvise (head, 0);
		if (retval)
			PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, retval);
	}
	if (head)
		head->teardown (head);
	head = NULL;

	if (rng)
		pgpRandomDestroy (rng);
	rng = NULL;

	if (tempenv)
		pgpenvDestroy (tempenv);
	tempenv = NULL;

	return retval;
}

/* Clean up allocated space (like the pipeline and the rng) on exit */
static void
mainExit (void)
{
	if (head)
		head->teardown (head);
	head = NULL;

	if (rng)
		pgpRandomDestroy (rng);
	rng = NULL;

	if (secretKey)
		pgpSecKeyDestroy (secretKey);
	secretKey = NULL;

	while (publicKey) {
		struct PgpPubKey *temp = publicKey->next;
		pgpPubKeyDestroy (publicKey);
		publicKey = temp;
	}

	if (ringpool)
		ringPoolDestroy (ringpool);
	ringpool = NULL;

	if (passcache)
		pgpPassCacheDestroy (passcache);
	passcache = NULL;

	memset (phrase, 0, sizeof (phrase));

	FreeLanguageStrings();
}

int
appMain (int argc, char *argv[])
{
	struct PgpUICb ui;
	struct UIArg ui_arg;
	struct Flags flags;
	struct PgpEnv *env;
	struct RingSet *tset;
	char const *p;
	int retval, prog_code = 0;
	PGPInvokedType invoked_type;

	/* Setup my exit function to clean myself up */
	atexit (mainExit);

	/* Setup the UI callback functions */
	ui.message = pgpTtyMessage;
	ui.doCommit = pgpTtyDoCommit;
	ui.newOutput = pgpTtyNewOutput;
	ui.needInput = pgpTtyNeedInput;
	ui.sigVerify = pgpTtySigVerify;
	ui.eskDecrypt = pgpTtyEskDecrypt;
	ui.annotate = NULL;

	/* Setup the TTY UI argument */
	ui_arg.arg.verbose = 1;
	ui_arg.arg.fp = stderr;
	ui_arg.arg.ringset = NULL;
	ui_arg.arg.showpass = 0;
	ui_arg.arg.commits = -1;
	ui_arg.arg.addKeys = mainAddKeys;
	ui_arg.arg.protect_name = 0;

	retval = pgpInitApp (&env, &argc, &ui, &ui_arg, prog_code, 0);
	if (retval) {
		/*XXX Is this message strictly neccessary?*/
		fprintf (stderr, "pgpInitApp failed: %d\n", retval);
		goto error;
	}
	/* Now initialize the values that need initialization */
	ui_arg.arg.env = env;
	ui_arg.arg.passcache = passcache = pgpPassCacheCreate (env);
	ui_arg.arg.verbose = pgpenvGetInt(env, PGPENV_VERBOSE, NULL, NULL);

	/* Check the exiration date on the application */
	exitExpiryCheck (env);

#if defined(UNIX) || defined(VMS)
	umask(077);             /* Make files default to private */
#endif

	/* Grab the pass phrase from the PGPPASS Env Var */
	p = getenv ("PGPPASS");
	if (p)
		pgpPassCacheAdd (passcache, p, strlen (p));

	/* XXX add PGPPASSFD here? */

	memset (&flags, 0, sizeof (flags));
	/* XXX This is never freed */
	flags.recips = (char **)malloc (argc * sizeof (*(flags.recips)));

	/* Initialize the application from config files, etc. */
	invoked_type = getInvokedType(argv[0]);
	switch(invoked_type) {
		case PGPencrypt:
			prog_code = EXIT_PROG_PGPE;
			break;
			
		case PGPsign:
			prog_code = EXIT_PROG_PGPS;
			break;

		case PGPverify:
			prog_code = EXIT_PROG_PGPV;
			break;

		case PGPold:
			prog_code = EXIT_PROG_PGPO;
			break;

	    case PGPkeys:
			/* XXX This shouldn't call exitArgError*/
			ErrorOutput(TRUE, LEVEL_CRITICAL, "PGPK_IS_SEPERATE");
			exitCleanup(PGPEXIT_ARGS);
			/*NOT EXECUTED*/
			break;

	    case PGPraw:
			exitInvokedRaw();
			/*NOT EXECUTED*/
			break;

	    default:
			/* XXX This shouldn't call exitArgError*/
			ErrorOutput(TRUE, LEVEL_CRITICAL, "UNKNOWN_SYMLINK");
			exitCleanup(PGPEXIT_ARGS);
			/*NOT EXECUTED*/
			break;
	}

	SetProgramName(prog_code);

	/* Now, read the argument list */
	switch(invoked_type) {
	    case PGPencrypt:
			mainParseEncryptArgs(&ui, &ui_arg, env, &argc, argv, &flags);
			break;

	    case PGPsign:
			mainParseSignArgs(&ui, &ui_arg, env, &argc, argv, &flags);
			break;

	    case PGPverify:
			mainParseVerifyArgs(&ui, &ui_arg, env, &argc, argv, &flags);
			break;

	    case PGPold:
			/*XXX this probably needs to be a seperate exe, but I'm not
			 *100% sure what to do about this, now.  -BAT
			 */
			mainParse262Args(&ui, &ui_arg, env, &argc, argv, &flags);
			break;

		case Unknown:
		case PGPraw:
		case PGPkeys:
			/*GCC's overly zealous preparser warns us of these, even though*/
			/*we can't get here...*/
			break;
	}

	ui_arg.arg.showpass = pgpenvGetInt (env, PGPENV_SHOWPASS, NULL, NULL);
	ui_arg.arg.moreflag = flags.moreflag;
	ui_arg.arg.pager = getenv ("PAGER");

	SetHeaders(pgpenvGetInt(env, PGPENV_HEADERS, NULL, NULL));

	if (!ringpool)
		ringpool = ringPoolCreate(env);
	if (!ringpool) {
		retval = PGPERR_NOMEM;
		goto error;
	}

	retval = mainOpenKeyrings (env, ringpool, 0, &tset);
	ui_arg.arg.ringset = tset;
	if (retval)
		goto error;

	/* Now process the flags */
	retval = mainProcessFlags (&ui, &ui_arg, env, argc, argv, &flags);
	
	exitCleanup (retval);
error:
	return (retval);
}

static PGPInvokedType getInvokedType(char *Argv0)
{
    char *p = NULL;
    PGPInvokedType invoked_type = Unknown;

    /*First of all, strip off any leading path info.  We don't care WHERE we
     *are, just WHAT we are:
     */
	invoked_type = PGPencrypt;

	/*Thanks to Chuck Bertsch, who pointed out that I'd written this wrong
	 *and that we should just look at the last character to see what we
	 *are, which enables you to call it pgp50e, for example.
	 */

	if((p = pgpFileNameTail(Argv0))) {
		p += strlen(p) - 1;
		switch(tolower(*p)) {
			case 'e':
				invoked_type = PGPencrypt;
				break;
					
			case 's':
				invoked_type = PGPsign;
				break;
				
			case 'v':
				invoked_type = PGPverify;
				break;
					
			case 'o':
				invoked_type = PGPold;
				break;
					
			case 'k':
				/*Huh?  They ran us as keys!*/
				invoked_type = PGPkeys;
				break;
					
			case 'p':
				invoked_type = PGPraw;
				break;
					
			default:
				invoked_type = Unknown;
		}
	}

    return(invoked_type);
}

static void exitInvokedRaw(void) {
	ErrorOutput(TRUE, LEVEL_CRITICAL, "DIFFERENT_EXES");
    exitCleanup(PGPEXIT_ARGS);
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

/*
 * pgpUserIO.c -- PGP TTY user IO interfaces
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU> and Colin Plumb
 * Modified by Brett A. Thomas <quark@baz.com> to use /dev/random
 *
 * $Id: pgpUserIO.c,v 1.5.2.10.2.5 1997/07/15 21:26:32 quark Exp $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#if !TIME_WITH_SYS_TIME
#include <time.h>
#endif
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h> /*for stat() to check for /dev/random*/
#endif

#include "pgpDebug.h"
#include "pgpFileRef.h"
#include "pgpAnnotate.h"
#include "pgpESK.h"
#include "pgpFileMod.h"
#include "pgpFixedKey.h"
#include "pgpMem.h"
#include "pgpPassCach.h"
#include "pgpErr.h"
#include "pgpFile.h"
#include "pgpMsg.h"
#include "pgpUI.h"
#include "pgpPipeline.h"
#include "pgpPubKey.h"
#include "pgpRndPool.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpSig.h"
#include "pgpTimeDate.h"
#include "pgpUsuals.h"
#include "pgpEnv.h"
#include "pgpKB.h"
#include "pgpUserIO.h"
#include "pgpOutput.h"
#include "pgpRingUI.h"
#include "pgpMoreMod.h"
#include "pgpTypes.h"

/* Filename for dumping data we don't want to keep */
#ifdef MSDOS
#define NULLNAME "NUL"
#else
#define NULLNAME "/dev/null"
#endif

static unsigned
pgpDevRandomAccum(int fd, unsigned count);

static Boolean
CheckDevRandom(char **RandomDevice, PgpTtyUI *ui);

static Boolean
VerifyRandomDevOSVersion(void);

/*
 * Get a string from the keyboard into the supplied buffer.
 * If echo is true, echos to the InteractionOutput stream.
 * Otherwise prints nothing (and beeps go to stderr).
 * Returns the number of characters read (always <= len-1).
 */
int
pgpTtyGetString(char *buf, int len, int echo)
{
    int c;
    int n = 0;

    kbCbreak();
#ifdef MACINTOSH
    echo = NULL;	/* XXX: For now, the Mac version echoes by itself */
#endif

    for (;;) {
	c = kbGet();
	
	if (c == '\r' || c == '\n') {
	    c = '\n';
	    break;
	}
	else
	    if (c == '\b' || c == 7 || c == 127) {
		if (n > 0) {
		    n--;
		    if (echo)
			InteractionOutputString(FALSE, "\b \b");
		}
		else
		    InteractionOutputString(FALSE, "\a");
	    }
	    else
		if (c < ' ' || c > 256) {
		    InteractionOutputString(FALSE, "\a");
		}
		else
		    if (n + 1 >= len) {
			InteractionOutputString(FALSE, "\a");
				kbFlush(0);
		    }
		    else {
				if (echo)
			    InteractionOutputString(FALSE, "%c", c);
				buf[n++] = (char)c;
		    }
    }
    if (echo) {
	InteractionOutputString(FALSE, "\n");
    }
    buf[n] = '\0';

    kbNorm();
    return n;
}

/*
 * Prompts the user for Y or N (case insensitive), and returns
 * the appropriate boolean value.  If nothing is entered, <def>
 * is returned.  This assumes that a prompt has already been printed.
 */
int
pgpTtyGetBool(int def, int echo)
{
	char buf[2];

	for (;;) {
		if (pgpTtyGetString(buf, sizeof (buf), echo) == 0)
			return def;
		switch (buf[0]) {
			case 'y':
			case 'Y':
				return 1;
			case 'n':
			case 'N':
				return 0;
			default:
				break;
		}
		InteractionOutput(TRUE,
				  "ENTER_Y_OR_N",
				  def ? 'Y' : 'N');
	}
}

static void
pgpTtyPutKeyID (byte const *keyid)
{
    InformationOutputString(FALSE,
			    "0x%2.2X%2.2X%2.2X%2.2X\n",
			    keyid[4],
			    keyid[5],
			    keyid[6],
			    keyid[7]);
}

/*
 * Ask the user for a filename.
 * string is the default print string that should be used.  filename is
 * the default that will be used.  buffer is of size buflen, and is
 * used to store the new filename.
 *
 * returns the length of the contents of buffer, which must be a real
 * filename (trailing white space must be stripped) */
static int
pgpTtyNeedFile (char *buffer, int buflen)
{
	char *p;

	*buffer = '\0';
	pgpTtyGetString (buffer, buflen, 1);

	p = buffer + strlen(buffer) - 1;
	while (p >= buffer && isspace (*p))
		*p-- = '\0';

	return (strlen (buffer));
}

static void
pgpTtySigResult (void *arg, union RingObject *key, word32 timestamp,
		 int status)
{
	struct PgpTtyUI *ui = (struct PgpTtyUI *) arg;
	struct RingSet const *set = ui->ringset;
	char timedate[21];

	pgpTimeString (timestamp, timedate);

	/* Status tells us whether the sig is good or bad.  Must also determine
	   if the signing key is valid. */

	if (status > 0) {
	    InformationOutput(TRUE, "GOOD_SIGNATURE", timedate);

	    ringKeyPrint (OUTPUT_INFORMATION, set, key, 1);
	    ringTtyKeyOKToSign(set, key);
	}
	else if (status == 0) {
	    InformationOutput(TRUE, "BAD_SIGNATURE", timedate);
	    ringKeyPrint (OUTPUT_INFORMATION, set, key, 1);
	}
	else {
	    char *ErrorString;
	    if(PGPErrCodeLoadString(&ErrorString, status) == PGPERR_OK) {
		InformationOutput(TRUE,
				  "ERROR_SIGNATURE",
				  status,
				  ErrorString);
		FreeString(&ErrorString);
	    }
	    PGPErrCodeOutput(TRUE, LEVEL_INFORMATION, status);
	}
}

int
pgpTtyGetPass (int showpass, char *buffer, int buflen)
{
	int i;

	InteractionOutput(TRUE, "ENTER_PASSPHRASE");
	i = pgpTtyGetString (buffer, buflen, showpass);
	InteractionOutputString(FALSE, "\n");
	return i;
}

/*
 * Responds to a commit request depending on the TTY UI member "commits"
 * If commits >= 0, then it will process that many levels of scopes.
 * if commits < 0, it will always recurse.
 */
int
pgpTtyDoCommit (void *arg, int scope)
{
	struct PgpTtyUI *ui = (struct PgpTtyUI *)arg;

	(void)scope;
	if (!ui->commits)
		return PGPANN_PARSER_PASSTHROUGH;
	ui->commits--;
	return PGPANN_PARSER_RECURSE;
}

/*
 * Performs an accumulation of random bits. As long as there are
 * fewer bits in the buffer than are needed, prompt for more.
 * (kbGet is known to call pgpRandPoolKeystroke() which increments
 * trueRandBits.)
 */
void
pgpTtyRandAccum (void *arg, unsigned count)
/* Get this many random bits ready */
{
	struct PgpTtyUI *ui = (struct PgpTtyUI *)arg;
	word32 randbits = pgpRandPoolEntropy ();
	Boolean HasDevRandom = FALSE, UseDevRandom = TRUE;
	char *RandomDevice = NULL;
	int fdDevRandom = -1, fdKB = -1, LargestFD = -1, SelectReturn = 0;
	fd_set AllDevicesSet;

	if (count > (unsigned)pgpRandPoolSize ())
		count = pgpRandPoolSize ();

	if (randbits >= count)
		return;

	/*Open the keyboard so that we can do a select on it:*/
	fdKB = kbOpenKbd(O_RDONLY); /*Can't fail*/
	FD_SET(fdKB, &AllDevicesSet);
	LargestFD = fdKB;

	if((HasDevRandom = CheckDevRandom(&RandomDevice, ui))) {
	    /*
	     *XXX Need to add support for ForceRandomDevice to pgp.cfg.
	     */
	    if(VerifyRandomDevOSVersion()) {
		if((fdDevRandom = open(RandomDevice, O_RDONLY)) >= 0) {
		    FD_SET(fdDevRandom, &AllDevicesSet);
		    if(fdDevRandom > LargestFD)
			LargestFD = fdDevRandom;
		    HasDevRandom = TRUE;
		}
		else
		    HasDevRandom = FALSE;

		}
		else {
		    UseDevRandom = FALSE;
		}
	}

	if(HasDevRandom) {
	    if(UseDevRandom) {
		InteractionOutput(TRUE,
				  "RANDOM_BITS_FROM_DEVICE",
				  count - (randbits), RandomDevice);
	    }
	    else {
		InteractionOutput(TRUE, "RANDOM_BITS_FROM_DEVICE_OLD_KERNEL");
	    }
	}

	if(!HasDevRandom || !UseDevRandom) {
	    InteractionOutput(TRUE,
			      "RANDOM_BITS_FROM_KEYBOARD",
			      count - (randbits));
	}

	kbCbreak ();

	do {
	    /* display counter to show progress */
	    InteractionOutputString(FALSE,
				    "\r%4u",
				    count-(unsigned)(randbits));

	    FD_ZERO(&AllDevicesSet);
	    FD_SET(fdKB, &AllDevicesSet);

	    if(HasDevRandom && UseDevRandom && fdDevRandom >= 0) {
		FD_SET(fdDevRandom, &AllDevicesSet);
	    }

	    kbFlush (0); /* Typeahead is illegal */

	    SelectReturn =
		select(LargestFD + 1, &AllDevicesSet, NULL, NULL, NULL);

	    /*If the waiting input is on stdin, or there was some kind of
	     *error, grab input from the keyboard:
	     */
	    if(SelectReturn < 0 ||
	       (SelectReturn >= 0 && FD_ISSET(fdKB, &AllDevicesSet))) {
		   (void)kbGet (); /* Wait for next char */
	    }
	    else { /*The waiting data is in /dev/random*/
		pgpDevRandomAccum(fdDevRandom, 1);
	    }
	    fputc (pgpRandPoolEntropy () == randbits ? '?' : '.', ui->fp);
	    randbits = pgpRandPoolEntropy ();

	} while (randbits < count);

    /* Do final display update */
    fputs ("\r   0 *", ui->fp);
    fputs ("\a -Enough, thank you.\n", ui->fp);

    /* Do an extra-thorough flush to absorb extra typing. */
    kbFlush (1);

    kbNorm ();

    if(RandomDevice) {
	pgpFree(RandomDevice);
    }

    if(fdDevRandom >= 0) {
	close(fdDevRandom);
	}

	kbCloseKbd(fdKB); /*Doesn't close if we're on stdin*/
}


int
pgpTtyNeedInput (void *arg, struct PgpPipeline *head)
{
    struct PgpTtyUI *ui = (struct PgpTtyUI *) arg;
    struct PgpFileRead *fileread;
    FILE *file = NULL;
    unsigned i, err;
    char namebuf[256], suggested_name[256];

    if (pgpenvGetInt (ui->env, PGPENV_BATCHMODE, NULL, NULL)) {
	if (ui->outname &&
	    (file = fopen (ui->outname, "rb")) != NULL) {
	    ui->outname = NULL;
	}
	else {
	    ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_INPUT_FILE_IN_BATCHMODE");
	    return PGPERR_NO_FILE;
	}
    } else {
	if (ui->outname) {
	    strncpy (suggested_name, ui->outname,
		     sizeof(suggested_name));
	}
	else {
	    suggested_name[0] = '\0';
	}
	
	for (;;) {
	    InteractionOutput(TRUE, "NEED_SIG_FILE", suggested_name);
	    i = pgpTtyNeedFile (namebuf, sizeof(namebuf));
	    if (!i) {
		if (suggested_name[0])
		    strcpy (namebuf, suggested_name);
		else
		    return PGPERR_NO_FILE;
	    }
	    file = fopen (namebuf, "rb");
	    if (file)
		break;
	    ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_OPEN_FILE", namebuf);
	    suggested_name[0] = 0;
	}
	/* Only use ui->outname once */
	if (strcmp (namebuf, ui->outname) == 0)
	    ui->outname = NULL;
    }

    fileread = pgpFileReadCreate (file, 1);
    if (!fileread) {
	ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_CREATE_READ_MODULE");
	fclose(file);
	return PGPERR_NOMEM;
    }

    err = pgpFileReadPump (fileread, head);
    pgpFileReadDestroy (fileread);
    return err;
}

struct CloseUI {
	struct PgpTtyUI *ui;
	struct PgpPipeline **output; /* No longer used */
	int closing;		/* No longer used */
	PGPFileRef *tmpFileRef;
};

static int
DoCloseKeyAdd (FILE *file, void *arg)
{
    struct CloseUI *ui = (struct CloseUI *)arg;
    int code = 0;

    if (fclose (file) != 0) {
	/* Error closing file */
	perror ("closing temp keyring file");
    } else if (ui->tmpFileRef != NULL) {
	char *filename = NULL;

	pgpFullPathFromFileRef(ui->tmpFileRef, &filename);
	
	if (filename == NULL) {
	    code = PGPERR_NOMEM;
	} else {
	    code = ui->ui->addKeys (ui->ui, filename);
	    pgpFree (filename);
	}
    }

    if (ui->tmpFileRef != NULL) {
	pgpDeleteFile (ui->tmpFileRef);
	pgpFreeFileRef (ui->tmpFileRef);
    }
    pgpMemFree (ui);
    return code;
}

#ifndef NO_POPEN

static int
DoClose (FILE *file, void *arg)
{
	struct CloseUI *ui = (struct CloseUI *)arg;
	int code;

	code = pclose (file);

	fputs ("\nDone...hit any key\n", ui->ui->fp);
	kbCbreak();
	kbGet();
	kbNorm();
	fputs ("\n", ui->ui->fp);
	/* XXX: Ask to save the file (if possible)?? How? */

	if (!code) {
		pgpMemFree (ui);
	}

	return code;
}

#endif /* ifndef NO_POPEN */


#ifndef NO_POPEN

static FILE *
openPager (struct PgpTtyUI const *ui)
{
	char const *pager = "more";

	if (ui->pager)
		pager = ui->pager;

	return popen (pager, "w");
}

#endif /* ifndef NO_POPEN */

int
pgpTtyNewOutput (void *arg, struct PgpPipeline **output, int type,
			char const *suggested_name)
{
	struct PgpTtyUI *ui = (struct PgpTtyUI *)arg;
	struct PgpFile *pgpf;
	struct CloseUI *cui;
	char const *name;
	char buffer[64];
	int len, moreflag = 0, use_stdout = 0, nullflag = 0;
	FILE *fp = NULL;	/* Initialized to shut up the compiler */

	pgpAssert(type >= 0);

	if (!output)
		return PGPERR_BADPARAM;

	name = ui->outname;	/* assume the name passed in */

	/* Send non-pgp portion to the bit bucket, for compat with old.*/
	if (type == PGPANN_NONPGP_BEGIN) {
	    name = NULLNAME;
	    nullflag = 1;
	
	    /* Filter Mode */
	    /* XXX This causes non-PGP output not to appear in filter mode*/
	    if (ui->protect_name < 0) {
		if(ui->moreflag) {
		    name = "stdout";
		    use_stdout++;
		}
	    }

	    type = PGP_LITERAL_TEXT;
	}
	else {
	    if (ui->protect_name < 0) {
		name = "stdout";
		use_stdout++;
	    }
	}


#if 0
	/* For debugging, describe what scope we are in */
	if (type > 255 && type != PGPANN_PGPKEY_BEGIN)
		fprintf (stderr, "pgpTtyNewOutput: type is scope: %d (%s)\n",
			 type, pgpScopeName (type));

	if (type >= 0 && type < 256 && type != PGP_LITERAL_TEXT &&
	    type != PGP_LITERAL_BINARY)
		fprintf (stderr,
 "pgpTtyNewOutput: type is a literal type, but not \'t\' or \'b\': %c (%d)\n",
			 (byte)type, type);
#endif

	/* if we want to use the name from the literal packet... */
	if (ui->protect_name > 0 && suggested_name &&
	    strcmp (suggested_name, "stdin"))
		name = suggested_name;


	/* Check for "for your eyes only" mode */
	if (/*type == PGP_LITERAL_TEXT && */
	    ((suggested_name && !strcmp (suggested_name, "_CONSOLE"))
	     || ui->moreflag)){
		/* output to pager */
		name = "_CONSOLE";
		moreflag++;
	}
	/* XXX: If suggested_name is _CONSOLE, ask the user whether to show */

	if (type == PGPANN_PGPKEY_BEGIN && ui->addKeys) {
		/* we have a means to add keys.. Use it */
		if (!ui->commits) {
			/* Just dearmor key file */
			type = PGP_LITERAL_BINARY;
		} else {
			name = "Temporary PGP Keyfile";/* won't be used */
			moreflag++;
		}
	}

	/* If all else fails, ask the user where to store the file.
	   If batchmode, use the suggested name if available, else
	   give up. */
	while (!name || (!moreflag && !use_stdout && !nullflag &&
			 pgpTtyCheckOverwrite(ui->env, name))) {
		if (pgpenvGetInt (ui->env, PGPENV_BATCHMODE, NULL, NULL)) {
			if (suggested_name)
				len = 0;
			else
		 		return PGPERR_NO_FILE;
		}
		else {
		    if(suggested_name)
			InteractionOutput(TRUE,
					  "NEED_FILE_TO_SAVE",
					  suggested_name);
		    else
			InteractionOutput(TRUE, "NEED_FILE_TO_SAVE", "");

		    len = pgpTtyNeedFile (buffer, sizeof (buffer));
		}

		if (len)
			name = buffer;
		else
			name = suggested_name;
	}


	if (!type) {
	    StatusOutput(TRUE, "UNKNOWN_FILE_TYPE");
	    type = PGP_LITERAL_TEXT;
	}
	StatusOutput(TRUE,
		     "OPENING_FILE_WITH_TYPE",
		     name,
		     type == PGP_LITERAL_TEXT ? "text" : "binary");

	if (! *output || moreflag || use_stdout) {
		if (!moreflag) {
			if (use_stdout)
				fp = stdout;
			else {
				fp = fopen (name, type == PGP_LITERAL_TEXT ?
						"w" : "wb");
				/* Only use name from UI once */
				if (name == ui->outname)
					ui->outname = NULL;
			}

			if (!fp) {
				/* XXX: Try to re-open file? */
				return PGPERR_NO_FILE;
			}
		}

		if (*output) {
			int ret;

			ret = (*output)->sizeAdvise (*output, 0);
			if (ret)
			    ErrorOutput(TRUE,
					LEVEL_SEVERE,
					"ERROR_CLOSING_OLD_FILE",
					ret);
			/* *output can be reset from under us in close() */
			if (*output)
				(*output)->teardown (*output);
			*output = NULL;
		}

		if (type == PGPANN_PGPKEY_BEGIN && ui->addKeys) {
			PGPError ret;

			cui = (struct CloseUI *)pgpMemAlloc (sizeof (*cui));
			if (!cui)
				return PGPERR_NOMEM;
			cui->ui = ui;
			fp = pgpStdIOOpenTempFile(&cui->tmpFileRef, &ret);
			if (!fp)
				return ret;
			fprintf(fp, "Wubba!\n");
			pgpf = pgpFileProcWriteOpen (fp, DoCloseKeyAdd, cui);

		} else if (moreflag) {
#ifdef NO_POPEN
			if (!pgpMoreModCreate(output)) {
				return PGPERR_NOMEM;
			}
			fputs ("\nPlaintext message follows...\n\
------------------------------\n\n", ui->fp);
			return 0;
#else
			cui = (struct CloseUI *)pgpMemAlloc (sizeof (*cui));
			if (!cui)
				return PGPERR_NOMEM;
			cui->ui = ui;
			cui->tmpFileRef = NULL;
			pgpf = pgpFileProcWriteOpen (openPager (ui), DoClose,
					cui);

			if (pgpf)
				fputs ("\nPlaintext message follows...\n\
------------------------------\n\n", ui->fp);
#endif
			} else {
				pgpf = pgpFileWriteOpen (fp, NULL);
				if (!pgpf && !use_stdout)
					fclose (fp);
			}
			if (!pgpf)
				return PGPERR_NOMEM;

			if (!pgpFileWriteCreate (output, pgpf,
			    (use_stdout && !moreflag) ? 0 : 1)) {
				return PGPERR_NOMEM;
			}
			return 0;

		/* XXX: How do we save the data in case the user wants to
		* save it to a file and moreflag is set and the name
		* isn't _CONSOLE?
		*/
	} else {
		/* Only use name from UI once */
		if (name == ui->outname)
			ui->outname = NULL;
		return (*output)->annotate (*output, *output,
					PGPANN_NEWFILE_START,
					(byte const *)name,
					strlen (name));
	}
}

/*
 * Given the signature structure, sig, verify it against the hash
 * to see if this signature is valid. This requires looking up the
 * public key in the keyring and validating the key.
 *
 * Returns 0 on success or an error code.
 */
int
pgpTtySigVerify (void *arg, struct PgpSig const *sig, byte const *hash)
{
	struct PgpTtyUI *ui = (struct PgpTtyUI *)arg;
	union RingObject *ringobj = NULL; /* shut up the compiler */
	struct PgpPubKey *pubkey;
	word32 timestamp = pgpSigTimestamp (sig);
	byte pkalg;
	byte const *keyid;
	int err;

	keyid = pgpSigId8 (sig);
	pkalg = pgpSigPKAlg (sig);
	pubkey = NULL;
#if 0 /*Obsolete - BAT 19970609*/
	if (!memcmp (keyid, fixedKeyID, 8) && pkalg == PGP_PKALG_RSA)
		pubkey = fixedKeyPub ();
	else
#endif
	if (ui->ringset) {
		ringobj = ringKeyById8 (ui->ringset, pkalg, keyid);
		if (ringobj) {
			pubkey = ringKeyPubKey (ui->ringset, ringobj, 0);
			ringObjectRelease (ringobj);
		}
	}


	if (pubkey && !pubkey->verify) {
		/* Make sure we can use this key */
		pgpPubKeyDestroy (pubkey);
		pubkey = NULL;
	}

	if (!pubkey) {
	    InformationOutput(TRUE,
			      "UNKNOWN_SIGNATURE");
	    pgpTtyPutKeyID (keyid);
	    return PGPERR_SIG_NOKEY;
	}

	err = pgpSigCheck (sig, pubkey, hash);

	if (err >= 0)
	  pgpTtySigResult (arg, ringobj, timestamp, err);
	else
	  pgpTtySigResult (arg, ringobj, timestamp, err);

	pgpPubKeyDestroy (pubkey);
	return 0;
}

int
pgpTtyUnlockSeckey (void *arg, union RingObject *key,
		struct PgpSecKey *seckey, char const *prompt)
{
	struct PgpTtyUI *ui = (struct PgpTtyUI *)arg;
	int i = 3;
	int err;

	if (!seckey)
		return -1;

	if (!pgpSecKeyIslocked (seckey))
		return 0;

	if (ui->passcache) {
		err = pgpPassCacheTryKey (ui->passcache, seckey);
		if (err > 0)
			return 0;
	}

	/*XXX Add code to detect if we're in batch mode without a passphrase*/

	if (prompt)
	    InteractionOutput(TRUE, prompt);

	InteractionOutput(TRUE, "NEED_PASSPHRASE_TO_DECRYPT_KEY");
	ringKeyPrint(OUTPUT_INTERACTION, ui->ringset, key, 1);
	while (i--) {
		char pass[256];
		int len;

		len = pgpTtyGetPass (ui->showpass, pass, sizeof (pass));
		if (!len)
			break;
		err = pgpSecKeyUnlock (seckey, ui->env, pass, len);

		if (err < 0) {
		    StatusOutput(TRUE, "KEY_CORRUPTED", err);
		    PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, err);
		    memset (pass, 0, sizeof (pass));
		    break;
		}

		if (err) {
		    StatusOutput(TRUE, "GOOD_PASSPHRASE");
		    if (ui->passcache)
			pgpPassCacheAdd (ui->passcache, pass, len);
		    memset (pass, 0, sizeof (pass));
		    return 0;
		}
		memset (pass, 0, sizeof (pass));
		StatusOutput(TRUE, "BAD_PASSPHRASE");
	}
	return -1;
}

/*
 * given a list of Encrypted Session Keys (esklist), try to decrypt
 * them to get the session key. Fills in keylen with the length of
 * the session key buffer.
 *
 * Returns 0 on success or PGPANN_PARSER_EATIT on failure.
 */
int
pgpTtyEskDecrypt (void *arg, struct PgpESK const *esklist, byte *key,
		size_t *keylen,
		int (*tryKey) (void *arg, byte const *key, size_t keylen),
		void *tryarg)
{
	struct PgpTtyUI *ui = (struct PgpTtyUI *)arg;
	struct PgpESK const *esk;
	union RingObject *ringobj = NULL; /* shut up the compiler */
	struct PgpSecKey *seckey;
	char pass[256];
	int i, err, len;
	byte pkalg;
	byte const *keyid;

	StatusOutput(TRUE, "MESSAGE_IS_ENCRYPTED");
	for (esk = esklist; esk; esk = pgpEskNext (esk)) {
		switch (pgpEskType (esk)) {
		case PGP_ESKTYPE_PASSPHRASE:
			if (ui->passcache) {
				err = pgpPassCacheTryEsk (ui->passcache,
							esk, tryKey, tryarg,
					 		key, keylen);
				if (err > 0)
					return 0; /* success */

				/* Ignore errors, err < 0 */
			}

			/*XXX Add code to get passphrase in batchmode*/
			i = 3;
			while (i--) {
			    len = pgpTtyGetPass(ui->showpass,
						pass,
						sizeof (pass));
				err = pgpEskConvDecrypt (esk, ui->env,
							 pass, len, key);
				*keylen = err;

				if (err >= 0)
					err = tryKey (tryarg, key, *keylen);

				if (!err) {
					if (ui->passcache)
						pgpPassCacheAdd (ui->passcache,
								pass, len);
					memset (pass, 0, sizeof (pass));
					return 0;
				}
				memset (pass, 0, sizeof (pass));
				InteractionOutput(TRUE,
						  "PASSPHRASE_INCORRECT");

				if (i) {
				    InteractionOutputString(FALSE, "  ");
				    InteractionOutput(FALSE, "TRY_AGAIN");
				}
				InteractionOutputString(FALSE, "\n");
			}
			break;

		case PGP_ESKTYPE_PUBKEY:
			keyid = pgpEskId8 (esk);
			pkalg = pgpEskPKAlg (esk);
#if 0 /*Obsolete - BAT 19970609*/
			if (!memcmp (keyid, fixedKeyID, 8))
				seckey = fixedKeySec ();
			else
#endif
			if (ui->ringset) {
				ringobj = ringKeyById8 (ui->ringset,
							pkalg,
							keyid);
				if (!ringobj)
					break;
				seckey = ringSecSecKey (ui->ringset,
							ringobj,
							PGP_PKUSE_ENCRYPT);
				ringObjectRelease (ringobj);
			}
			else
				break;

			if (!seckey)
				break;

			if (! seckey->decrypt) {
				/* Can't decrypt with this key */
				pgpSecKeyDestroy (seckey);
				break;
			}

			/* XXX need to make sure seckey is unlocked */
			err = pgpTtyUnlockSeckey (arg, ringobj, seckey, NULL);
			if (err) {
				pgpSecKeyDestroy (seckey);
				break;
			}

			err = pgpEskPKdecrypt (esk, ui->env, seckey, key);
			*keylen = err;
			pgpSecKeyDestroy (seckey);

			if (err <= 0)
				break;

			err = tryKey (tryarg, key, *keylen);
			if (!err)
				return 0;

			break;

		default:
		    ErrorOutput(TRUE,
				LEVEL_INFORMATION,
				"UNKNOWN_ESK",
				pgpEskType(esk));
		    break;
		}
	}
	/*
	 * At this point, none of the PgpESKs have been decrypted, so let's
	 * inform the user that we failed.
	 */

	InformationOutput(TRUE, "CANNOT_DECRYPT");

	for (esk = esklist; esk; esk = pgpEskNext (esk)) {
		switch (pgpEskType (esk)) {
		case PGP_ESKTYPE_PASSPHRASE:
		    InformationOutputString(FALSE, "  ");
		    InformationOutput(FALSE, "A_PASSPHRASE");
			break;
		case PGP_ESKTYPE_PUBKEY:
			keyid = pgpEskId8 (esk);
			pkalg = pgpEskPKAlg (esk);
			if (ui->ringset) {
				ringobj = ringKeyById8 (ui->ringset, pkalg,
							keyid);
				if (ringobj) {
					ringKeyPrint(OUTPUT_INFORMATION,
						     ui->ringset,
						     ringobj, 1);
					ringObjectRelease (ringobj);
				} else {
					pgpTtyPutKeyID (keyid);
				}
			} else {
			    InformationOutputString(FALSE, "  ");
			    InformationOutput(FALSE, "KEY_ID");
			    pgpTtyPutKeyID (keyid);
			}
		}
	}
	InformationOutputString(FALSE, "\n");
	return PGPANN_PARSER_EATIT;
}

/* Check if file already exists, and whether it is OK to overwrite it.
   If +force, no check is made and any existing file will be overwritten.
   If +batchmode (but not +force), the default answer is always do not
   overwrite. */

int
pgpTtyCheckOverwrite (struct PgpEnv const *env, char const *name)
{
        FILE *fp;
	int status = 0;

	if (!pgpenvGetInt (env, PGPENV_FORCE, NULL, NULL)) {
	        fp = fopen (name, "r");
		if (fp) {
		        fclose (fp);

			if (pgpenvGetInt (env, PGPENV_BATCHMODE,
					  NULL, NULL)) {
			    WarningOutput(TRUE,
					  LEVEL_INFORMATION,
					  "USE_FORCE_TO_ALLOW_OVERWRITING");
			    status = PGPERR_NO_FILE;
			}
			else {
			    InteractionOutput(TRUE,
					      "FORCE_OVERWRITE",
					      name);
			    if (!pgpTtyGetBool (0, 1))
				  status = PGPERR_NO_FILE;
			}

		}
	}
	if (status)
	    ErrorOutput(TRUE,
			LEVEL_SEVERE,
			"UNABLE_TO_OVERWRITE_FILE",
			name);
	
	return status;
}

/*This checks for existance of /dev/random (or whatever is configured in the
 *appropriate PGP variable).  We place the name of the device we checked in
 *in RandomDevice (allocating, first); the caller is responsible for
 *deallocation with pgpFree().  Returns TRUE if /dev/random is there, FALSE
 *if it is not.  RandomDevice may be NULL if you don't care what the device
 *is, only if it exists.  Note also that this function will alert the user if
 *the requested device is world-writable.
 */
static Boolean CheckDevRandom(char **RandomDevice, PgpTtyUI *ui)
{
    char *DeviceToUse, *prefDevRandom;
    Boolean HasDevRandom = TRUE;
    FILE *fDev = NULL;
#ifdef HAVE_SYS_STAT_H
    /*This is the preferred method, but it may not be universally portable,
     *so I've implemented a less-than-desirable check below using definitely
     *available functions.
     */
    struct stat stat_buf;
#endif

    prefDevRandom = (char *)
	pgpenvGetString(ui->env, PGPENV_RANDOMDEVICE, NULL, NULL);

    if(!prefDevRandom)
	prefDevRandom = (char *) "/dev/random";

    if(strcmp(prefDevRandom, "/dev/random") != 0) {
	WarningOutput(TRUE,
		      LEVEL_INFORMATION,
		      "RANDOM_DEVICE_NOT_DEFAULT",
		      "/dev/random");
    }

    if(RandomDevice) {
	*RandomDevice = pgpAlloc(sizeof(char) * (strlen(prefDevRandom) + 1));
	strcpy(*RandomDevice, prefDevRandom);
    }

    if(RandomDevice && *RandomDevice) {
	DeviceToUse = *RandomDevice;
    }
    else {
	DeviceToUse = (char *) prefDevRandom;
    }

#ifdef HAVE_SYS_STAT_H
    if(stat(DeviceToUse, &stat_buf) == 0) {
	/*Check for world-writable /dev/random and warn the user:*/
	if(((stat_buf.st_mode & S_IRWXO) == S_IRWXO) ||
	   ((stat_buf.st_mode & S_IRWXG) == S_IRWXG)) {
	    WarningOutput(TRUE,
			  LEVEL_SEVERE,
			  "RANDOM_DEVICE_WRITABLE",
			  DeviceToUse);
	}
    }
    else {
	HasDevRandom = FALSE;
    }
#endif

    /*People who don't have sys/stat.h miss out on the check of permissions,
     *above. People who do have sys/stat.h still execute this code, to make
     *sure we can actually READ the random device. sys/stat.h people who
     *have random devices they can't read get a warning.
     *
     *People who don't have sys/stat.h are just generally in trouble.
     */
    if(HasDevRandom) {
	if((fDev = fopen(DeviceToUse, "r"))) {
	    fclose(fDev);
	    HasDevRandom = TRUE;
	}
	else {
#if HAVE_SYS_STAT_H
	    ErrorOutput(TRUE,
			LEVEL_INFORMATION,
			"RANDOM_DEVICE_UNREADABLE",
			DeviceToUse);
#endif
	    HasDevRandom = FALSE;
	}
    }

    return(HasDevRandom);
}

/* /dev/random did basically what /dev/urandom does now in Linux
 *kernels < 1.3.33. My best research to date indicates that the
 *modern /dev/random, with no bugs large enough to limit its
 *cryptographic utility, cames to be in this kernel version. Note
 *that I haven't examined every patch from 1.3.33 - 1.2.0, but
 *this is my current information (and I'd love to hear about it
 *if I'm incorrect).
 *
 *Basically, what we want to do is check what Linux version we're
 *running (from /proc/version).  I'm sure there is some more Linux-
 *specific way to do this, but checking /proc/version will work on
 *systems that aren't Linux (since the check will just fail).
 *
 */
static Boolean
VerifyRandomDevOSVersion(void)
{
    FILE *inf;
    Boolean VersionIsOK = TRUE;
    char VersionBuf[256], *p;
    unsigned short major = 0, minor = 0, release = 0;

    if((inf = fopen("/proc/version", "r"))) {
	fgets(VersionBuf, sizeof(VersionBuf), inf);
	/*This string is of the form "Linux version Major.Minor.Release.*/
	if(strncasecmp(VersionBuf, "Linux version ", 14) == 0) {

	    p = VersionBuf;
	    /*Skip the leading text:*/
	    while(*p && !isdigit(*p))
		++p;

	    major = atoi(p);

	    /*If major >= 2, we know we're OK, if it's 0, we know we're not.*/
	    if(major == 1) {
		/*Skip the major number and dot:*/
		while(*p && isdigit(*p))
		    ++p;

		if(*p) {
		    ++p;
		    minor = atoi(p);

		    /*If here, we know it's 1.x.y.  If x < 3, we know it's
		     *bad; if > 3, we know it's good.*/
		    if(minor == 3) {
			/*Ok, it's 1.3.y, so we have to actually find out the
			 *minor version to see if it's OK.*/
			/*Skip the minor number and dot:*/

			while(*p && isdigit(*p)) {
			    ++p;
			}
			if(*p) {
			    release = atoi(p);
			}
		    }
		}
	    }

	    if(major <= 1 && minor <= 3 && release <= 32) {
		VersionIsOK = FALSE;
	    }
	}
    }

    return(VersionIsOK);
}


/*This adds count bytes from RandomDevice (usually "/dev/random") to the
 *random pool.
 */
static unsigned
pgpDevRandomAccum(int fd, unsigned count)
{
    char RandBuf;
    unsigned short i = 0;

    pgpAssert(count);
    pgpAssert(fd >= 0);

    for(i = 0; i <= count; ++i) {
	RandBuf = read(fd, &RandBuf, count);
	pgpRandomAddBytes(&pgpRandomPool, (byte *)&RandBuf, sizeof(RandBuf));
	pgpRandPoolAddEntropy(256);
    }

    return(i);
}

/* Assumes the message string is in printf format */
int
pgpTtyMessage (void *arg, int type, int msg, unsigned numargs, ...)
{
	struct PgpTtyUI *ui = (struct PgpTtyUI *)arg;
	struct PgpUICbArg const *msgarg;
	char const *string, *ptr;
        va_list ap;

	if (type > ui->verbose)
		return 0;

	string = pgpmsgString (msg);
	va_start (ap, numargs);

	while ((ptr = strchr(string, '%')) != NULL) {
		/* Print part of string leading up to % */
		fwrite (string, 1, ptr-string, ui->fp);

		if (*++ptr == '%') {
			putc ('%', ui->fp);
			string = ptr+1;
			continue;
		}
		pgpAssert (numargs--);
		msgarg = va_arg (ap, struct PgpUICbArg const *);
		switch (*ptr++) {
		case 'd': /* int */
			pgpAssert (msgarg->type == PGP_UI_ARG_INT);
			fprintf (ui->fp, "%d", msgarg->val.i);
			break;
		case 'l': /* unsigned long (word32) */
			pgpAssert(*ptr == 'u');
			ptr++;
			pgpAssert (msgarg->type == PGP_UI_ARG_WORD32);
			fprintf (ui->fp, "%lu",
					(unsigned long)msgarg->val.w32);
			break;
		case 'u': /* unsigned */
			pgpAssert (msgarg->type == PGP_UI_ARG_UNSIGNED);
			fprintf (ui->fp, "%u", msgarg->val.u);
			break;
		case 's': /* string */
			pgpAssert (msgarg->type == PGP_UI_ARG_STRING);
			fprintf (ui->fp, "%s", msgarg->val.s);
			break;
		case '.': /* buffer (%.*s) */
			pgpAssert (*ptr == '*' && ptr[1] == 's');
			ptr += 2;
			pgpAssert (msgarg->type == PGP_UI_ARG_BUFFER);
			fprintf (ui->fp, "%.*s",
				 (int)msgarg->val.buf.len,
				 msgarg->val.buf.buf);
			break;
		default:
			pgpAssert (0);
		}
		string = ptr;
	}
	/* Print trailing segment of string */
	fputs (string, ui->fp);
	putc('\n', ui->fp);

	return 0;
}	


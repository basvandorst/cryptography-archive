/*
 * pgpkKeyGen.c -- Routines to generate a key
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * 970514 BAT  Created new file from MHW's routine from pgpk.c
 *             Broken up into smaller functions
 *             And a PGP file without a single goto, now!  Who'd have thunk
 *             it?
 *
 * $Id: pgpkKeyGen.c,v 1.1.2.8.2.6 1997/08/28 05:33:40 quark Exp $
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
#include <sys/stat.h>		/* for umask() */
#endif

#include "pgpDebug.h"
#include "pgpEnv.h"
#include "pgpMem.h"
#include "pgpKeyDB.h"
#include "pgpOutput.h"
#include "pgpkUI.h"
#include "pgpkInit.h"
#include "pgpkExit.h"
#include "pgpOpt.h"
#include "pgpUserIO.h"
#include "pgpRndPool.h"

#include "pgpkKeyGen.h"
#include "pass.h"
#include "url.h"
#include "keyserver.h"

#define MIN_KEY_SIZE  768

static int
keygenProgress (void *arg, int c);
static int
KeyGenGetPassphrase(Boolean GotArgs,
		    char *passphrase,
		    size_t passphraselen,
		    const PgpEnv *env);
static int
KeyGenGetValidity(Boolean GotArgs, int *validity);
static int
KeyGenGetName(Boolean GotArgs, char *namebuf, int *namelen);
static int
KeyGenGetKeySize(Boolean GotArgs, byte pkalg, int *keybits, int *subkeybits);
static int
KeyGenGetKeyType(Boolean GotArgs, byte *pkalg, byte *subkalg);
static int
ParseKeyGenArgs(int argc,
		char *argv[],
		byte *pkalg,
		byte *subkalg,
		int *keybits,
		int *subkeybits,
		int *namelen,
		char *namebuf,
		int *validity,
		char *passphrase,
		int passphraselen,
		char **keyserver);
static int KeyGenSendKey(Boolean GotArgs,
			 char *keyserver,
			 PGPKey *key,
			 struct PgpTtyUI *ui);

static int KeySizeTable[5] = {0, 768, 1024, 2048, 3072};

/* Optional Arguments: (if you specify one, you've got to specify
*them all, except passphrase)
*
*argv[1] Key type (DSS or RSA)
*argv[2] Key Size (768-4096)
*argv[3] UserID (Brett A. Thomas <quark@baz.com>)
*argv[4] Validity period in days (0 - 999)
*argv[5] Passphrase (if '?', user will be prompted).
#argv[6] Keyserver to send key to, if any
*/

int
doKeyGenerate (PGPKeySet *defaultset, int argc, char *argv[], void *ui_arg)
{
    int        error = 0;
    byte       namebuf[256] = "\0",
	       passphrase[PASSLEN] = "\0";
    int	       namelen = 0;
    int        keybits = 0, subkeybits = 0;
    int	       validity = 0;
    byte       pkalg, subkalg;
    PGPKey    *key;
    unsigned   entropy_needed;
    extern PgpEnv PGPKDBExport *pgpEnv;
    Boolean    GotArgs = FALSE;
    char       *keyserver = NULL;

    if(argc > 1) {
	GotArgs = TRUE;
	namelen = sizeof(passphrase);
	error = ParseKeyGenArgs(argc,
				argv,
				&pkalg,
				&subkalg,
				&keybits,
				&subkeybits,
				&namelen,
				(char *) namebuf,
				&validity,
				(char *) passphrase,
				sizeof(passphrase),
				&keyserver);
    }

    /*This is inefficient in that a bad arg up above will cause six error==
     *evaluations.  However, it does mean that, if we have a memory allocation
     *failure someplace, we don't then call all the other routines, and it
     *also means that we do this without a bunch of exits or gotos all over
     *the place.
     */

    if(error == PGPERR_OK) {
	error = KeyGenGetKeyType(GotArgs, &pkalg, &subkalg);
    }

    if(error == PGPERR_OK) {
	error = KeyGenGetKeySize(GotArgs, pkalg, &keybits, &subkeybits);
    }

    if(error == PGPERR_OK) {
	namelen = sizeof(namebuf);
	error = KeyGenGetName(GotArgs, (char *) namebuf, &namelen);
    }

    if(error == PGPERR_OK) {
	error = KeyGenGetValidity(GotArgs, &validity);
    }

    if(error == PGPERR_OK) {
	error = KeyGenGetPassphrase(GotArgs,
				    (char *) passphrase,
				    sizeof(passphrase),
				    pgpEnv);
    }

    if(error == PGPERR_OK) {
	/* Need to ask for randomness */
	entropy_needed = pgpKeyEntropyNeeded (pkalg, keybits);
	if (subkalg)
	    entropy_needed += pgpKeyEntropyNeeded (subkalg, subkeybits);
	if (entropy_needed > pgpRandPoolEntropy()) {
	    fprintf (stderr, "Collecting randomness for key...\n");
	    pgpTtyRandAccum (pgpEnv, 
			     entropy_needed);
	}

	pgpAssert(defaultset);

	error = pgpGenerateKey (defaultset,
				pkalg,
				keybits,
				validity,
				(char *) namebuf,
				namelen,
				(char *) passphrase,
				keygenProgress,
				ui_arg,
				&key);

	fprintf (stderr, "\n");

	if(subkalg && error == PGPERR_OK) {
	    error = pgpGenerateSubKey(key,
				      subkeybits,
				      validity,
				      (char *) passphrase,
				      (char *) passphrase,
				      keygenProgress,
				      ui_arg);
	    fprintf (stderr, "\n");
	}

	if (!error) {
	    fprintf (stderr, "Keypair created successfully.\n");
	    pgpCommitKeyRingChanges (defaultset);
	    KeyGenSendKey(GotArgs, keyserver, key, (struct PgpTtyUI *) ui_arg);
	}
    }

    if(keyserver)
	pgpFree(keyserver);

    return(error);
}

static int
ParseKeyGenArgs(int argc,
		char *argv[],
		byte *pkalg,
		byte *subkalg,
		int *keybits,
		int *subkeybits,
		int *namelen,
		char *namebuf,
		int *validity,
		char *passphrase,
		int passphraselen,
		char **keyserver)
{
    int error = PGPERR_OK;
    pgpAssert(argv && pkalg && subkalg && keybits && subkeybits && namelen);
    pgpAssert(namebuf && validity && passphrase && passphraselen);

    /*Avoid warning on subkeybits.  This is for future functionality, in case
     *we ever want to allow others to specify this.
     */

    (void) subkeybits;

    if(argc == 5 || argc == 6) {
	if(strcasecmp(argv[0], "DSS") == 0) {
	    *pkalg = PGP_PKALG_DSA;
	    *subkalg = PGP_PKALG_ELGAMAL;
	}
	else {
	    if(strcasecmp(argv[0], "RSA") == 0) {
#if (!NO_RSA_KEYGEN)
		*pkalg = PGP_PKALG_RSA;
		*subkalg = 0;
#else
		fprintf(stderr, "\
RSA Key generation not supported in this verion of PGP.\n");
		error = PGPERR_BADPARAM;
#endif	
	    }

	    else {
		fprintf(stderr, "Unknown algorithm %s.\n", argv[0]);
		error = PGPERR_BADPARAM;
	    }
	}
	
	if(error == PGPERR_OK) {
	    *keybits = atol(argv[1]);
	}
	
	if(error == PGPERR_OK) {
	    strncpy(namebuf, argv[2], *namelen);
	    *namelen = strlen(argv[2]);
	}
	
	if(error == PGPERR_OK) {
	    *validity = atoi(argv[3]);
	}
	
	if(error == PGPERR_OK) {
	    strncpy(passphrase, argv[4], passphraselen);
	}

	if(error == PGPERR_OK && argc == 6) {
	    /*They specified a keyserver*/
	    if((*keyserver = pgpAlloc(sizeof(char) * (strlen(argv[5]) + 1)))) {
		strcpy(*keyserver, argv[5]);
	    }
	    else
		error = PGPERR_NOMEM;
	}
    }
    else {
	fprintf(stderr, "\
Error!  If any command-line key generation options are specified, all\n\
options must be specified (with the exception of the server to send to).\n");
	error = PGPERR_BADPARAM;
    }

    return(error);
}

static int
KeyGenGetKeyType(Boolean GotArgs, byte *pkalg, byte *subkalg)
{
    int error = PGPERR_OK, keytype, len;
    char buf[256];
    extern PgpEnv *pgpEnv;

    pgpAssert(pkalg && subkalg);
    if(!GotArgs) {
#if(!NO_RSA_KEYGEN)
	do {
	    fprintf (stderr,
		     "\
Choose the type of your public key:\n\
  1)  DSS/Diffie-Hellman - New algorithm for 5.0 (default)\n\
  2)  RSA\n\
Choose 1 or 2: ");
	    len = pgpTtyGetString (buf, 
				   sizeof(buf), 
				   OUTPUT_INTERACTION, 
				   pgpEnv);
	    if (len) {
		keytype = atoi(buf);
	    }
	    else {
		keytype = 1;
	    }

	    switch (keytype) {
		case 1:
		    *pkalg = PGP_PKALG_DSA;
		    *subkalg = PGP_PKALG_ELGAMAL;
		    break;
		case 2:
		    *pkalg = PGP_PKALG_RSA;
		    *subkalg = 0;
		    break;
		default:
		    *pkalg = *subkalg = 0;
		    break;
	    }
	}while(!*pkalg);
#else
	fprintf(stderr, "Creating DSS/Diffie-Hellman key.\n");
	*pkalg = PGP_PKALG_DSA;
	*subkalg = PGP_PKALG_ELGAMAL;
#endif
    }

    return(error);
}

static int
KeyGenGetKeySize(Boolean GotArgs, byte pkalg, int *keybits, int *subkeybits)
{
    int len = 0, maxlen = 0, error = PGPERR_OK, maxchoice = 0;
    char buf[256];
    extern PgpEnv *pgpEnv;

    pgpAssert(pkalg && keybits && subkeybits);

    do {
	if(!GotArgs) {
	    switch(pkalg) {
		case PGP_PKALG_DSA:
		    fputs (
"\nPick your public/private keypair key size:\n\
(Sizes are Diffie-Hellman/DSS; Read the user's guide for more information)\n\
 1)   768/768  bits- Commercial grade, probably not currently breakable\n\
 2)  1024/1024 bits- High commercial grade, secure for many years\n\
 3)  2048/1024 bits- \"Military\" grade, secure for forseeable future\
(default)\n\
 4)  3072/1024 bits- Archival grade, slow, highest security\n\
Choose 1, 2, 3 or 4, or enter desired number of Diffie-Hellman bits\n\
(768 - 4096): ", stderr);
		    maxchoice = 4;
		    fflush (stderr);
		    break;

		case PGP_PKALG_RSA:
		    fputs (
"\nPick your public/private keypair key size:\n\
  1)   768 bits- Commercial grade, probably not currently breakable\n\
  2)  1024 bits- High commercial grade, secure for many years\n\
  3)  2048 bits- \"Military\" grade, secure for the forseeable future\n\
Choose 1, 2 or 3, or enter desired number of bits\n\
(768 - 2048): ", stderr);
		    maxchoice = 3;
		    fflush (stderr);
		    break;
	    }

	    len = pgpTtyGetString (buf, sizeof(buf), OUTPUT_INTERACTION, 
				   pgpEnv);
	    if(len) {
		*keybits = atoi(buf);
		if(*keybits > 0 && *keybits <= maxchoice) {
		    *keybits = KeySizeTable[*keybits];
		}
	    }
	    else {
		*keybits = 2048;
	    }
	}

	switch(pkalg) {
	    case PGP_PKALG_DSA:
		maxlen = 4096;
		break;

	    case PGP_PKALG_RSA:
		maxlen = 2048;
		break;
	}

	if(*keybits < MIN_KEY_SIZE) {
	    error = PGPERR_BADPARAM;
	    fprintf(stderr,
		    "Minimum key size is %u.\n",
		    MIN_KEY_SIZE);
	}
	else {
	    if(*keybits > maxlen) {
		error = PGPERR_BADPARAM;
		fprintf(stderr,
			"Maximum key size is %u.\n",
			maxlen);
	    }
	    else {
		*subkeybits = *keybits;
		if(pkalg == PGP_PKALG_DSA && (*keybits >= 1024)) {
		    *keybits = 1024;
		}
		if(!GotArgs) {
		    if(*keybits != *subkeybits && pkalg == PGP_PKALG_DSA) {
			fprintf(stderr,
				"\
(Producing a %i bit DSS and a %i bit Diffie-Hellman key)\n",
				*keybits,
				*subkeybits);
		    }
		    else
			fprintf(stderr, "\n");
		}
		error = PGPERR_OK;
	    }
	}
    }while(!GotArgs && error != PGPERR_OK);

    return(error);
}

static int
KeyGenGetName(Boolean GotArgs, char *namebuf, int *namelen)
{
    int error = PGPERR_OK, tmp_namelen;
    extern PgpEnv *pgpEnv;

    pgpAssert(namebuf);
    pgpAssert(namelen);
    pgpAssert(*namelen);

    /*If we got a name on the command line, we've got to assume it's OK and
     *the user knows what she's doing.
     */

    do {
	if(!GotArgs) {
	    fputs (
"\nYou need a user ID for your public key.  The desired form for this\n"
"user ID is your FULL name, followed by your E-mail address enclosed in\n"
"<angle brackets>, if you have an E-mail address.  For example:\n"
"  Joe Smith <user@domain.com>\n"
"If you violate this standard, you will lose much of the benefits of\n"
"PGP 5.0's keyserver and email integration.\n\n"
"Enter a user ID for your public key: ", stderr);
	    fflush (stderr);
	    tmp_namelen = pgpTtyGetString (namebuf, 
					   *namelen, 
					   OUTPUT_INTERACTION,
					   pgpEnv);

	    if (!tmp_namelen) {
		*namebuf = '\0';
	    }
	    else
		*namelen = tmp_namelen;
	}
	else
	    *namelen = strlen(namebuf);

	if(!*namebuf) {
	    error = PGPERR_BADPARAM;
	    fputs(
"\nYou must enter something for your ID; otherwise, PGP will be unable to "
"manipulate your key.\n", stderr);
	}
	else
	    error = PGPERR_OK;
    }while(!GotArgs && error != PGPERR_OK);

    return(error);
}

static int
KeyGenGetValidity(Boolean GotArgs, int *validity)
{
    int len, error = PGPERR_OK;
    char buf[256];
    extern PgpEnv *pgpEnv;

    pgpAssert(validity);

    do {
	if(!GotArgs) {
	    fputs ("\n"
"Enter the validity period of your key in days from 0 - 999\n"
"0 is forever (and the default): ",
		   stderr);
	    fflush (stderr);

	    len = pgpTtyGetString (buf, 
				   sizeof(buf), 
				   OUTPUT_INTERACTION, 
				   pgpEnv);
	
	    if (len) {
		*validity = atoi (buf);
	    }
	    else {
		*validity = 0;
	    }
	}

	if(*validity < 0 || *validity > 999) {
	    error = PGPERR_BADPARAM;
	    fprintf(stderr, "Validity must be between 0 and 999!\n");
	}
	else
	    error = PGPERR_OK;
    }while(!GotArgs && error != PGPERR_OK);

    return(error);
}

static int
KeyGenGetPassphrase(Boolean GotArgs,
		    char *passphrase,
		    size_t passphraselen,
		    const PgpEnv *env)
{
    int error = PGPERR_OK;
    char pass2[PASSLEN];

    pgpAssert(passphrase && passphraselen);

    if(!GotArgs ||
	(GotArgs && *passphrase == '?' && *(passphrase + 1) == '\0')) {
	do {
	    fprintf(stderr, "\n\
You need a pass phrase to protect your private key(s).\n\
Your pass phrase can be any sentence or phrase and may have many\n\
words, spaces, punctuation, or any other printable characters.\n");

	    pgpTtyGetPass(pgpenvGetInt(env, PGPENV_SHOWPASS, NULL, NULL),
			  passphrase, 
			  passphraselen,
			  env);
	    fprintf (stderr, "Enter again, for confirmation:\n");
	    pgpTtyGetPass (pgpenvGetInt(env, PGPENV_SHOWPASS, NULL, NULL), 
			   pass2, 
			   sizeof(pass2), env);
	    if (strcmp (passphrase, pass2) != 0) {
		fprintf (stderr,
			 "Passphrases are different; please re-enter:\n");
		error = PGPERR_BADPARAM;
	    }
	    else
		error = PGPERR_OK;
	}while(error != PGPERR_OK);
    }
    return error;
}

static int
keygenProgress (void *arg, int c)
{
	(void)arg;		/* make the compiler happy */
	putc (c, stderr);
	fflush (stderr);
	return 0;
}

static int KeyGenSendKey(Boolean GotArgs,
			 char *keyserver,
			 PGPKey *key,
			 struct PgpTtyUI *ui)
{
    int error = PGPERR_OK, len;
    char buf[1024];
    int port = 0;
    char server[1024], file[1024], protocol[16], url[1024] = "\0";
    extern PgpEnv *pgpEnv;

    pgpAssert(key);

    if(!GotArgs) {
	fputs ("\n\
If you wish to send this new key to a server, enter the URL of the server,\n\
below.  If not, enter nothing.\n", stderr);
	fflush (stderr);

	if((len = pgpTtyGetString (buf, 
				   sizeof(buf), 
				   OUTPUT_INTERACTION, 
				   pgpEnv))) {
	    strcpy(url, buf);
	}
    }
    else {
	if(keyserver) {
	    strcpy(url, keyserver);
	}
    }

    if(url[0]) {
	char *buffer = NULL;
	size_t length = 1;
	PGPKeySet *keyset;

	if(parse_url(protocol, server, file, &port, buf) == PGPERR_OK) {
	    if((keyset = pgpNewSingletonKeySet(key))) {
		if((buffer = pgpAlloc(sizeof(char) * length))) {
		    pgpExportKeyBuffer(keyset,
				       (unsigned char *) buffer,
				       &length);
		    pgpFree(buffer);
		    ++length; /*Make room for a null*/
		    if((buffer = pgpAlloc(sizeof(char) * length))) {
			error = pgpExportKeyBuffer(keyset,
						   (unsigned char *) buffer,
						   &length);
			if(error == PGPERR_OK) {
			    PutKeyInfoHkp(server, port, buffer);
			    pgpFree(buffer);
			}
		    }
		    else
			error = PGPERR_NOMEM;
		}
		pgpFreeKeySet(keyset);
	    }
	    else {
		error = PGPERR_NOMEM;
	    }
	}
	else {
	    fprintf(ui->fp, "\
Badly formed URL.  Use pgpk -xa -o <URL> to submit your key manually.\n");
	}
    }

    return(error);
}


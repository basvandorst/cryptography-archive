/*
 * pgpEnv.h -- The PGPlib Environment.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Keep track of what used to be "global" state; these values are valid
 * only within a working environment. Multiple environments can co-exist
 * within a single application, however each one would have its own unique
 * environment structure and should pass it on to its children.
 *
 * The main purpose of this structure is to gether together a lot of
 * little fiddly things that a user might want to adjust into a single
 * variable so code that doesn't want to fiddle with them doesn't have to
 * think about the issue. It makes passing a different value of an
 * environment variable to a function more awkward (you have to
 * copy a parent environment and set the value(s) of interest), but
 * it's quite possible if you want more detailed control.
 *
 * The big win is that to add an extra control parameter to a function
 * that already gets some parameters from an environment, you can just
 * add it to the environment and change the function to get its parmaeter
 * from the environment and all of the code in between can just be
 * relinked.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpEnv.h,v 1.20.2.5.2.7 1997/10/09 00:06:36 quark Exp $
 */

#ifndef PGPENV_H
#define PGPENV_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

#define PGPENV_BASE_INT	1000
enum PgpEnvInts {
	PGPENV_ARMOR=PGPENV_BASE_INT, PGPENV_ARMORLINES,
	PGPENV_CERTDEPTH, PGPENV_CIPHER, PGPENV_CLEARSIG,
	PGPENV_COMPLETES, PGPENV_COMPRESS, PGPENV_COMPRESSALG,
	PGPENV_COMPRESSQUAL, PGPENV_ENCRYPTTOSELF, PGPENV_HASH,
	PGPENV_MARGINALS, PGPENV_SHOWPASS, PGPENV_TEXTMODE,
	PGPENV_TRUSTED, PGPENV_TZFIX, PGPENV_VERBOSE,
	PGPENV_VERSION, PGPENV_BATCHMODE, PGPENV_FORCE,
	PGPENV_MAGIC, PGPENV_NOOUT, PGPENV_LICENSE, PGPENV_HEADERS,
	PGPENV_PGPMIME, 
	PGPENV_MAILWORDWRAPENABLE, PGPENV_MAILWORDWRAPWIDTH,
	PGPENV_MAILPASSCACHEENABLE, PGPENV_MAILPASSCACHEDURATION,
	PGPENV_MAILENCRYPTPGPMIME, PGPENV_MAILSIGNPGPMIME,
	PGPENV_FASTKEYGEN, PGPENV_SIGNCACHEENABLE, PGPENV_SIGNCACHEDURATION,
	PGPENV_NOBATCHINVALIDKEYS, PGPENV_WARNONMIXRSAELGAMAL,
	PGPENV_MAILENCRYPTDEFAULT, PGPENV_MAILSIGNDEFAULT,
	PGPENV_DATEOFLASTSPLASHSCREEN, PGPENV_MARGINALLYVALIDWARNING,
	PGPENV_PGPMIMEPARSEBODY, PGPENV_FIRSTKEYGENERATED,
	PGPENV_AUTOSERVERFETCH,
	PGPENV_OUTPUTPRIMARYFD, 
	PGPENV_OUTPUTINFORMATIONFD, PGPENV_OUTPUTINTERACTIONFD, 
	PGPENV_OUTPUTWARNINGFD, PGPENV_OUTPUTERRORFD, PGPENV_OUTPUTSTATUSFD,
	PGPENV_WARNONRSARECIPANDNONRSASIGNER, PGPENV_HTTPKEYSERVERPORT, 
	PGPENV_MAX_INT
};

#define PGPENV_BASE_STRING	2000
enum PgpEnvStrings {
	PGPENV_BAKRING=PGPENV_BASE_STRING, PGPENV_CHARSET,
	PGPENV_COMMENT, PGPENV_RANDOMDEVICE, 
	PGPENV_LANGUAGE, PGPENV_LANGUAGEFILE, PGPENV_MYNAME, 
	PGPENV_PGPPATH, PGPENV_PUBRING, PGPENV_RANDSEED, PGPENV_SECRING, 
	PGPENV_SYSDIR, PGPENV_TMP, PGPENV_UPATH, PGPENV_OWNERNAME, 
	PGPENV_COMPANYNAME, PGPENV_LICENSENUMBER, PGPENV_HTTPKEYSERVERHOST,
	PGPENV_MAX_STRING
};

#define PGPENV_BASE_POINTER	3000
enum PgpEnvPointers {
	PGPENV_CHARMAPTOLATIN1=PGPENV_BASE_POINTER,
	PGPENV_CHARMAPTOLOCAL,
	PGPENV_PUBRINGREF,
	PGPENV_PRIVRINGREF,
	PGPENV_RANDSEEDDATA,
	PGPENV_PGPKEYSMACMAINWINPOS,
	PGPENV_PGPAPPMACPRIVATEDATA,
	PGPENV_ENCRYPTIONCALLBACK,
	PGPENV_PGPMENUMACAPPSIGNATURES,
	PGPENV_MAX_POINTER
};

/* Configuration priorities */
#define PGPENV_PRI_FORCE		25
#define PGPENV_PRI_CMDLINE		20
#define PGPENV_PRI_CONFIG		15
#define PGPENV_PRI_PRIVDEFAULT	10
#define PGPENV_PRI_SYSCONF	5
#define PGPENV_PRI_PUBDEFAULT	0

/* Exported Functions */

/* create and initialize, copy, and destroy an environment */
struct PgpEnv PGPExport *pgpenvCreate (void);
struct PgpEnv PGPExport *pgpenvCopy (struct PgpEnv const *env);
void PGPExport pgpenvDestroy (struct PgpEnv *env);

/*
 * Set Integer and String configuration parameters.  Strings are
 * copied by the environment; the caller need not preserve them.
 * (They must, of course, be null-terminated.)
 */
int PGPExport pgpenvSetInt (struct PgpEnv *env, enum PgpEnvInts var, int num,
	int pri);
int PGPExport pgpenvSetString (struct PgpEnv *env, enum PgpEnvStrings var,
		char const *string, int pri);

/*
 * Retrieve Integer and String configuration parameters.
 */
int PGPExport pgpenvGetInt (struct PgpEnv const *env, enum PgpEnvInts var,
	int *pri, int *error);
char const PGPExport *pgpenvGetString (struct PgpEnv const *env,
	enum PgpEnvStrings var, int *pri, int *error);

/*
 * Set and get Pointer parameters -- these are not configuration values,
 * in general, so they are treated somewhat differently. Note:
 * the environment code does *not* copy the objects associated with the
 * pointers, since it doesn't know how large they are or what other
 * dependent objects they may contain. It is up to the caller to
 * ensure that the pointers remain valid as long as a copy of them
 * might exist, and to free them when a pointer no longer exists.
 */
int PGPExport pgpenvSetPointer (struct PgpEnv *env, enum PgpEnvPointers var,
	void *ptr, size_t size);
void PGPExport *pgpenvGetPointer (struct PgpEnv const *env,
	enum PgpEnvPointers var, size_t *size, int *error);

#ifdef __cplusplus
}
#endif

#endif /* PGPENV_H */

/*
 * pgpUserIO.h -- PGP TTY User IO Interfaces
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpUserIO.h,v 1.4.2.2.2.4 1997/08/19 21:00:10 quark Exp $
 */

#ifndef PGPUSERIO_H
#define PGPUSERIO_H

#include <stdarg.h>
#include <stdio.h>

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpSecKey;
#ifndef TYPE_PGPSECKEY
#define TYPE_PGPSECKEY 1
typedef struct PgpSecKey PgpSecKey;
#endif

struct RingSet;
#ifndef TYPE_RINGSET
#define TYPE_RINGSET 1
typedef struct RingSet RingSet;
#endif

struct PgpSig;
#ifndef TYPE_PGPSIG
#define TYPE_PGPSIG 1
typedef struct PgpSig PgpSig;
#endif

struct PgpESK;
#ifndef TYPE_PGPESK
#define TYPE_PGPESK 1
typedef struct PgpESK PgpESK;
#endif

struct PgpPassCache;
#ifndef TYPE_PGPPASSCACHE
#define TYPE_PGPPASSCACHE 1
typedef struct PgpPassCache PgpPassCache;
#endif

struct PgpTtyUI {
	int verbose;
	int showpass;
	int moreflag;
	int protect_name;
	int commits;
        FILE *fp;
	char const *outname;
	char const *pager;
	struct RingSet const *ringset;
	struct PgpPassCache *passcache;
	struct PgpEnv const *env;
	int (*addKeys) (void *arg, char const *filename);
};
#ifndef TYPE_PGPTTYUI
#define TYPE_PGPTTYUI 1
typedef struct PgpTtyUI PgpTtyUI;
#endif

/*
 * Get a string from the keyboard into the supplied buffer.
 * If echo is true, to the InteractionOutput stream.
 * Otherwise prints nothing (and beeps go to stderr).
 * Returns the number of characters read (always <= len-1).
 */
int PGPTTYExport pgpTtyGetString(char *buf, int len, int echo, 
				 const PgpEnv *env);

/*
 * Prompts the user for Y or N (case insensitive), and returns
 * the appropriate boolean value. If nothing is entered, <def>
 * is returned. This assumes that a prompt has already been printed.
 */
int PGPTTYExport
pgpTtyGetBool(int def, int echo, const PgpEnv *env);

/*
 * Get a passphrase from the user.  buffer is space to store
 * the passphrase, and buflen is the size of buffer.
 *
 * Fill in buffer with the passphrase. line endings must be stripped
 * from the string. returns the length of the passphrase.
 */
int PGPTTYExport pgpTtyGetPass (int showpass, char *buffer, int buflen, 
				const PgpEnv *env);

/*
 * A debugging committal callback. Returns one of:
 * 	ANN_PARSER_PASSTHROUGH
 *	ANN_PARSER_EATIT
 * 	ANN_PARSER_PROCESS
 *	ANN_PARSER_RECURSE
 */
int PGPTTYExport pgpTtyDebugCommit (void *arg, int scope);

/*
 * Will process commit requests silently, based upon the value of arg->commits
 *
 *     If commits > 0, it will recursively process that many scopes,
 * changing the value of commits each time through.
 *     If commits == 0, it will passthrough the ciphertext.
 *     If commits < 0, it will recursively process the whole message.
 */
int PGPTTYExport pgpTtyDoCommit (void *arg, int scope);

/*
 * Accumulate count bits of random data from the user utilizing keystroke
 * timings.
 */
void PGPTTYExport pgpTtyRandAccum (struct PgpEnv const *env, unsigned count);

int PGPTTYExport pgpTtyDebugAnnotate (void *arg, struct PgpPipeline *origin,
	int type, byte const *string, size_t size);

int PGPTTYExport pgpTtyNeedInput (void *arg, 
				  struct PgpPipeline *head);

int PGPTTYExport pgpTtyNewOutput (void *arg, struct PgpPipeline **output,
	int type, char const *suggested_name);
int PGPTTYExport pgpTtyMessageOld (void *arg, int type, int msg,
	unsigned numargs, ...);
int PGPTTYExport pgpTtyMessage (void *arg, int type, int msg,
	unsigned numargs, ...);
int PGPTTYExport pgpTtyUnlockSeckey (void *arg, union RingObject *key,
				     struct PgpSecKey *seckey, 
				     char const *prompt, const PgpEnv *env);
int PGPTTYExport pgpTtySigVerify (void *arg, struct PgpSig const *sig,
	byte const *hash);

int PGPTTYExport pgpTtyEskDecrypt (void *arg, struct PgpESK const *esklist,
	byte *key, size_t *keylen,
	int (*tryKey)(void *arg, byte const *key, size_t keylen),
	void *tryarg);

int PGPTTYExport pgpTtyCheckOverwrite (struct PgpEnv const *env,
				       char const *name);

int PGPTTYExport pgpTtyGetInitialBitsFromRandDevice(struct PgpEnv const *env,
						    unsigned count);
#ifdef __cplusplus
}
#endif

#endif /* PGPUSERIO_H */

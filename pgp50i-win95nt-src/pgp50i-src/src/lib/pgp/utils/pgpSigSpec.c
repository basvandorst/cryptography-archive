/*
 * pgpSigSpec.c -- Signature Specification
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Code to specify a PGP signature and signature attributes
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpSigSpec.c,v 1.8.2.1 1997/06/07 09:51:42 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpTimeDate.h"
#include "pgpUsuals.h"
#include "pgpSigSpec.h"

/* Maximum number of recovery keys */
#define NR 4

struct PgpSigSpec {
			struct PgpSigSpec *next;
			struct PgpSecKey *seckey;
			PgpVersion version;
			byte hashtype;
			byte extra[5];		 		/* I know this is 5 bytes now! */
			/* Next two are only used on key self-sigs */
			word32 keyExpiration;	/* Timestamp for key to expire */
			byte prefAlgs[20];			/* Preferred algorithms */
			byte havekrecovery[NR];			/* True if krecovery has been set */
			byte krecovery[NR][22];			/* Key recovery information */
	};

struct PgpSigSpec *
pgpSigSpecCreate (struct PgpEnv const *env, struct PgpSecKey *sec,
				 byte sigtype)
	{
			struct PgpSigSpec *ss;

if (!env || !sec)
	return NULL;

ss = (struct PgpSigSpec *)pgpMemAlloc (sizeof (*ss));
if (ss) {
	/*
					* XXX: This hash algorithm selection must be kept in sync
					* with pgpHashAlgUsedForSignatures (in pgpKeyMan.c)
					*/
					int tzFix = pgpenvGetInt (env, PGPENV_TZFIX, NULL, NULL);
					int hash = pgpenvGetInt (env, PGPENV_HASH, NULL, NULL);
					PgpVersion version = pgpenvGetInt (env, PGPENV_VERSION, NULL,
							 			NULL);
					/* Force SHA-1 hash with DSA */
					if (sec->pkAlg == PGP_PKALG_DSA)
						hash = PGP_HASH_SHA;
					memset (ss, 0, sizeof (*ss));
					ss->seckey = sec;
					if (pgpSigSpecSetHashtype (ss, hash) ||
					pgpSigSpecSetSigtype (ss, sigtype) ||
					pgpSigSpecSetTimestamp (ss, pgpTimeStamp (tzFix)) ||
					pgpSigSpecSetVersion (ss, version)) {
							pgpMemFree (ss);
							ss = NULL;
					}
			}
			return ss;
	}

struct PgpSigSpec *
pgpSigSpecCopy (struct PgpSigSpec const *spec)
{
			struct PgpSigSpec *ss;

			if (!spec)
				 return NULL;

			ss = (struct PgpSigSpec *)pgpMemAlloc (sizeof (*ss));
			if (ss) {
				 memcpy (ss, spec, sizeof (*ss));
				 ss->next = NULL;
			}
			return ss;
}

void
pgpSigSpecDestroy (struct PgpSigSpec *spec)
{
			struct PgpSigSpec *ss;

for (ss = spec; ss; ss = spec) {
				spec = ss->next;
					memset (ss, 0, sizeof (*ss));
					pgpMemFree (ss);
			}
	}

/* Lists... Add a spec to a list; get the next spec from the list */
int
pgpSigSpecAdd (struct PgpSigSpec **list, struct PgpSigSpec *spec)
{
	if (!list)
		return PGPERR_BADPARAM;

spec->next = *list;
*list = spec;
return 0;
}

struct PgpSigSpec *
pgpSigSpecNext (struct PgpSigSpec const *list)
{
			if (!list)
				 return NULL;

return list->next;
}

/*
 * Access functions and Modifier functions follow below
 */

/* The SecKey cannot be changed */
struct PgpSecKey *
pgpSigSpecSeckey (struct PgpSigSpec const *spec)
{
	return spec->seckey;
}

/* Set and get the hash */
int
pgpSigSpecSetHashtype (struct PgpSigSpec *spec, byte hashtype)
{
	pgpAssert (spec);

	if (!pgpHashByNumber(hashtype))
	return PGPERR_BAD_HASHNUM;

	spec->hashtype = hashtype;
	return 0;
}

struct PgpHash const *
pgpSigSpecHash (struct PgpSigSpec const *spec)
{
	return pgpHashByNumber(spec->hashtype);
}

byte
pgpSigSpecHashtype (struct PgpSigSpec const *spec)
{
	return spec->hashtype;
}

/* Set and get the version */
int
pgpSigSpecSetVersion (struct PgpSigSpec *spec, PgpVersion version)
{
	pgpAssert (spec);

spec->version = version;
return 0;
}

PgpVersion
pgpSigSpecVersion (struct PgpSigSpec const *spec)
{
	return spec->version;
}

/* Set the sigtype and timestamp; get the "extra" bytes which result */
int
pgpSigSpecSetSigtype (struct PgpSigSpec *spec, byte sigtype)
{
			pgpAssert (spec);

spec->extra[0] = sigtype;
return 0;
}

int
pgpSigSpecSetTimestamp (struct PgpSigSpec *spec, word32 timestamp)
{
			pgpAssert (spec);

			spec->extra[1] = (byte)(timestamp>>24);
			spec->extra[2] = (byte)(timestamp>>16);
			spec->extra[3] = (byte)(timestamp>>8);
			spec->extra[4] = (byte)timestamp;
			return 0;
}

/*
 * The timestamp is external for now because it is a part of the
 * SigParams structure from the environment. Everything else is
 * a part of the PgpSigSpec structure
 */
byte const *
pgpSigSpecExtra (struct PgpSigSpec const *spec, unsigned *extralen)
{
			if (extralen)
			*extralen = 5;

		return spec->extra;
}


/* Access functions for self-sig related signature subpackets */


/* Returns 0 if no expiration has been set */
word32
pgpSigSpecKeyExpiration (struct PgpSigSpec const *spec)
{
	return spec->keyExpiration;
}

int
pgpSigSpecSetKeyExpiration (struct PgpSigSpec *spec, word32 keyExpire)
{
	spec->keyExpiration = keyExpire;
	return 0;
}

/* Returns a pointer to a byte of 0, with *preflen = 0 if no algs set */
byte const *
pgpSigSpecPrefAlgs (struct PgpSigSpec const *spec, unsigned *preflen)
	{
			if (preflen) {
					unsigned i;
					for (i=0; i<sizeof(spec->prefAlgs); ++i) {
						if (spec->prefAlgs[i] == 0)
								break;
					}
					*preflen = i;
			}
			return spec->prefAlgs;
	}

int
pgpSigSpecSetPrefAlgs (struct PgpSigSpec *spec, byte const *algs, size_t len)
	{
			pgpAssert (len < sizeof(spec->prefAlgs));
			pgpCopyMemory (algs, spec->prefAlgs, len);
			spec->prefAlgs[len+1] = 0;
			return 0;
	}

/* Returns NULL if no recovery key has been set */
byte const *
pgpSigSpecRecoveryKey (struct PgpSigSpec const *spec, unsigned *preflen,
			int nth)
	{
			if (nth >= NR)
				 return NULL;
			if (!spec->havekrecovery[nth]) {
				 return NULL;
			}
			if (preflen) {
				 *preflen = sizeof(spec->krecovery[nth]);
			}
			return spec->krecovery[nth];
	}

int
pgpSigSpecSetRecoveryKey (struct PgpSigSpec *spec, byte *krinfo, size_t len,
			int nth)
	{
		if (nth >= NR)
			return PGPERR_GENERIC;
		pgpAssert (len == sizeof(spec->krecovery[nth]));
		pgpCopyMemory (krinfo, spec->krecovery[nth], len);
		spec->havekrecovery[nth] = 1;
		return 0;
	}

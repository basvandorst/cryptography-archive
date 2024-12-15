/*
 * pgpSigSpec.c -- Signature Specification
 *
 * Code to specify a PGP signature and signature attributes
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpSigSpec.c,v 1.32 1997/10/17 06:15:16 hal Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpHashPriv.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpPubKey.h"
#include "pgpTimeDate.h"
#include "pgpUsuals.h"
#include "pgpSigSpec.h"
#include "pgpContext.h"

/* Maximum number of additional decryption or revocation keys */
#define NR 4

/* Maximum size of regexp specification in characters */
#define MAXREGEXP	1024

struct PGPSigSpec {
	PGPContextRef	cdkContext;
	
	PGPSigSpec *next;
	PGPSecKey *seckey;
	PgpVersion version;
	PGPByte hashtype;
	PGPBoolean exportable;
	PGPBoolean revokable;
	PGPByte extra[5];			/* I know this is 5 bytes now! */
	PGPUInt32 sigExpiration;	/* Timestamp for sig to expire */
	PGPByte trustLevel;			/* 0 for regular, 1 for trust, > for meta */
	PGPByte trustValue;			/* if trustLevel > 0 */
	PGPBoolean hasRegExp;
	char regExp[MAXREGEXP];
	/* Next ones are only used on key self-sigs */
	PGPUInt32 keyExpiration;	/* Timestamp for key to expire */
	PGPByte prefAlgs[20];		/* Preferred algorithms */
	PGPByte havekdecryption[NR];	/* True if kdecryption has been set */
	PGPByte kdecryption[NR][22];	/* Key additional decryption information */
	PGPByte havekrevocation[NR];	/* True if krevocation has been set */
	PGPByte krevocation[NR][22];	/* Key revocation information */
	DEBUG_STRUCT_CONSTRUCTOR( PGPSigSpec )
};

PGPSigSpec *
pgpSigSpecCreate (PGPEnv const *env, PGPSecKey *sec,
		  PGPByte sigtype)
{
	PGPSigSpec *ss;
	PGPContextRef		cdkContext	= pgpenvGetContext( env );
	
	if (!env || !sec)
		return NULL;

	ss = (PGPSigSpec *)pgpContextMemAlloc( cdkContext,
		sizeof (*ss), kPGPMemoryFlags_Clear);
	if (ss) {
		/*
		 * XXX: This hash algorithm selection must be kept in sync
		 *      with pgpHashAlgUsedForSignatures (in pgpKeyMan.c)
		 */
		int tzFix = pgpenvGetInt (env, PGPENV_TZFIX, NULL, NULL);
		PGPByte hash = pgpenvGetInt (env, PGPENV_HASH, NULL, NULL);
		PgpVersion version = pgpenvGetInt (env, PGPENV_VERSION, NULL,
						   NULL);
		/* Force SHA-1 hash with DSA */
		if (sec->pkAlg == kPGPPublicKeyAlgorithm_DSA) {
			/* Use double width version if modulus size > 1024 */
			/* Don't have that info; use fact that sig consists of two MP
			 * numbers, each with 2 byte headers; sizes of 2*(20+2) can use
			 * SHA, else we need double width version.
			 */
			if (pgpSecKeyMaxsig(sec, kPGPPublicKeyMessageFormat_PGP) > 44) {
				hash = kPGPHashAlgorithm_SHADouble;
			} else {
				hash = kPGPHashAlgorithm_SHA;
			}
		}

		ss->cdkContext	= cdkContext;
		ss->seckey = sec;
		ss->exportable = TRUE;
		ss->revokable = TRUE;
		if (pgpSigSpecSetHashtype (ss, hash) ||
		    pgpSigSpecSetSigtype (ss, sigtype) ||
		    pgpSigSpecSetTimestamp (ss, pgpTimeStamp (tzFix)) ||
		    pgpSigSpecSetVersion (ss, version)) {
			pgpContextMemFree( cdkContext, ss);
			ss = NULL;
		}
	}
	return ss;
}

PGPSigSpec *
pgpSigSpecCopy (PGPSigSpec const *spec)
{
	PGPSigSpec *ss;

	if (!spec)
		return NULL;

	ss = (PGPSigSpec *)pgpContextMemAlloc( spec->cdkContext,
		sizeof (*ss), 0);
	if (ss) {
		memcpy (ss, spec, sizeof (*ss));
		ss->next = NULL;
	}
	return ss;
}

void
pgpSigSpecDestroy (PGPSigSpec *spec)
{
	PGPSigSpec *ss;

	for (ss = spec; ss; ss = spec)
	{
		PGPContextRef		cdkContext;
		
		cdkContext	= ss->cdkContext;
		spec = ss->next;
		pgpClearMemory( ss,  sizeof (*ss));
		pgpContextMemFree( cdkContext, ss);
	}
}

/* Lists...  Add a spec to a list; get the next spec from the list */
int
pgpSigSpecAdd (PGPSigSpec **list, PGPSigSpec *spec)
{
	if (!list)
		return kPGPError_BadParams;

	spec->next = *list;
	*list = spec;
	return 0;
}

PGPSigSpec *
pgpSigSpecNext (PGPSigSpec const *list)
{
	if (!list)
		return NULL;

	return list->next;
}

/*
 * Access functions and Modifier functions follow below
 */

/* The SecKey cannot be changed */
PGPSecKey *
pgpSigSpecSeckey (PGPSigSpec const *spec)
{
	return spec->seckey;
}

/* Set and get the hash */
int
pgpSigSpecSetHashtype (PGPSigSpec *spec, PGPByte hashtype)
{
	pgpAssert (spec);

	if (!pgpHashByNumber( (PGPHashAlgorithm)hashtype))
		return kPGPError_BadHashNumber;

	spec->hashtype = hashtype;
	return 0;
}

PGPHashVTBL const *
pgpSigSpecHash (PGPSigSpec const *spec)
{
	return pgpHashByNumber( (PGPHashAlgorithm) spec->hashtype);
}

PGPByte
pgpSigSpecHashtype (PGPSigSpec const *spec)
{
	return spec->hashtype;
}

/* Set and get the version */
int
pgpSigSpecSetVersion (PGPSigSpec *spec, PgpVersion version)
{
	pgpAssert (spec);

	spec->version = version;
	return 0;
}

PgpVersion
pgpSigSpecVersion (PGPSigSpec const *spec)
{
	return spec->version;
}

/* Make sure we have a version new enough for subpackets */
static void
sSubPacketVersion(PGPSigSpec *spec)
{
	if (spec->version < PGPVERSION_3)
		spec->version = PGPVERSION_3;
}

/* Set the sigtype and timestamp; get the "extra" bytes which result */
int
pgpSigSpecSetSigtype (PGPSigSpec *spec, PGPByte sigtype)
{
	pgpAssert (spec);

	spec->extra[0] = sigtype;
	return 0;
}

int
pgpSigSpecSetTimestamp (PGPSigSpec *spec, PGPUInt32 timestamp)
{
	pgpAssert (spec);

	spec->extra[1] = (PGPByte)(timestamp>>24);
	spec->extra[2] = (PGPByte)(timestamp>>16);
	spec->extra[3] = (PGPByte)(timestamp>>8);
	spec->extra[4] = (PGPByte)timestamp;
	return 0;
}

/*
 * The timestamp is external for now because it is a part of the
 * SigParams structure from the environment.  Everything else is
 * a part of the PGPSigSpec structure 
 */
PGPByte const *
pgpSigSpecExtra (PGPSigSpec const *spec, size_t *extralen)
{
	if (extralen)
		*extralen = 5;

	return spec->extra;
}


/* Returns 0 if no expiration has been set */
PGPUInt32
pgpSigSpecSigExpiration (PGPSigSpec const *spec)
{
	return spec->sigExpiration;
}

int
pgpSigSpecSetSigExpiration (PGPSigSpec *spec, PGPUInt32 sigExpire)
{
	spec->sigExpiration = sigExpire;
	if( sigExpire != 0 )
		sSubPacketVersion(spec);
	return 0;
}


/* Access functions for self-sig related signature subpackets */


/* Returns 0 if no expiration has been set */
PGPUInt32
pgpSigSpecKeyExpiration (PGPSigSpec const *spec)
{
	return spec->keyExpiration;
}

int
pgpSigSpecSetKeyExpiration (PGPSigSpec *spec, PGPUInt32 keyExpire)
{
	spec->keyExpiration = keyExpire;
	if( keyExpire != 0 )
		sSubPacketVersion(spec);
	return 0;
}

/* Returns a pointer to a byte of 0, with *preflen = 0 if no algs set */
PGPByte const *
pgpSigSpecPrefAlgs (PGPSigSpec const *spec, size_t *preflen)
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
pgpSigSpecSetPrefAlgs (PGPSigSpec *spec, PGPByte const *algs, size_t len)
{
	pgpAssert (len < sizeof(spec->prefAlgs));
	pgpCopyMemory (algs, spec->prefAlgs, len);
	spec->prefAlgs[len+1] = 0;
	sSubPacketVersion(spec);
	return 0;
}

/* Returns NULL if no additional decryption key has been set */
PGPByte const *
pgpSigSpecAdditionalRecipientRequest (PGPSigSpec const *spec, size_t *preflen,
	int nth)
{
	if (nth >= NR)
		return NULL;
	if (!spec->havekdecryption[nth]) {
		return NULL;
	}
	if (preflen) {
		*preflen = sizeof(spec->kdecryption[nth]);
	}
	return spec->kdecryption[nth];
}

int
pgpSigSpecSetAdditionalRecipientRequest  (PGPSigSpec *spec, PGPByte *krinfo,
	size_t len, int nth)
{
	if (nth >= NR)
		return kPGPError_TooManyARRKs;
	pgpAssert (len == sizeof(spec->kdecryption[nth]));
	pgpCopyMemory (krinfo, spec->kdecryption[nth], len);
	spec->havekdecryption[nth] = 1;
	sSubPacketVersion(spec);
	return 0;
}

/* Returns NULL if no revocation key has been set */
PGPByte const *
pgpSigSpecRevocationKey (PGPSigSpec const *spec, size_t *preflen,
	int nth)
{
	if (nth >= NR)
		return NULL;
	if (!spec->havekrevocation[nth]) {
		return NULL;
	}
	if (preflen) {
		*preflen = sizeof(spec->krevocation[nth]);
	}
	return spec->krevocation[nth];
}

int
pgpSigSpecSetRevocationKey  (PGPSigSpec *spec, PGPByte *krinfo, size_t len,
	int nth)
{
	if (nth >= NR)
		return kPGPError_TooManyARRKs;
	pgpAssert (len == sizeof(spec->krevocation[nth]));
	pgpCopyMemory (krinfo, spec->krevocation[nth], len);
	spec->havekrevocation[nth] = 1;
	sSubPacketVersion(spec);
	return 0;
}

void
pgpSigSpecSetExportable (PGPSigSpec *spec, PGPBoolean exportable)
{
	pgpAssert( spec );
	spec->exportable = exportable;
	if( !exportable)
		sSubPacketVersion(spec);
}

PGPBoolean
pgpSigSpecExportable (PGPSigSpec const *spec)
{
	pgpAssert( spec );
	return spec->exportable;
}

void
pgpSigSpecSetRevokable (PGPSigSpec *spec, PGPBoolean revokable)
{
	pgpAssert( spec );
	spec->revokable = revokable;
	if( !revokable)
		sSubPacketVersion(spec);
}

PGPBoolean
pgpSigSpecRevokable (PGPSigSpec const *spec)
{
	pgpAssert( spec );
	return spec->revokable;
}

void
pgpSigSpecSetTrustLevel (PGPSigSpec *spec, PGPByte trustLevel)
{
	pgpAssert( spec );
	spec->trustLevel = trustLevel;
	if( trustLevel > 0 )
		sSubPacketVersion(spec);
}

PGPByte
pgpSigSpecTrustLevel (PGPSigSpec const *spec)
{
	pgpAssert( spec );
	return spec->trustLevel;
}

void
pgpSigSpecSetTrustValue (PGPSigSpec *spec, PGPByte trustValue)
{
	pgpAssert( spec );
	spec->trustValue = trustValue;
	if( trustValue > 0 )
		sSubPacketVersion(spec);
}

PGPByte
pgpSigSpecTrustValue (PGPSigSpec const *spec)
{
	pgpAssert( spec );
	return spec->trustValue;
}

void
pgpSigSpecSetRegExp (PGPSigSpec *spec, char const *regExp)
{
	pgpAssert( spec );
	pgpAssert( strlen( regExp ) < MAXREGEXP );
	strcpy (spec->regExp, regExp);
	spec->hasRegExp = TRUE;
	sSubPacketVersion(spec);
}

char const *
pgpSigSpecRegExp (PGPSigSpec const *spec)
{
	pgpAssert( spec );
	if( !spec->hasRegExp )
		return NULL;
	else
		return spec->regExp;
}

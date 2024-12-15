/*
 * pgpSigSpec.c -- Signature Specification
 *
 * Code to specify a PGP signature and signature attributes
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpSigSpec.c,v 1.40 1998/08/05 05:37:26 heller Exp $
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
#include "pgpPktList.h"
#include "pgpPubKey.h"
#include "pgpRngPub.h"
#include "pgpTimeDate.h"
#include "pgpUsuals.h"
#include "pgpSigSpec.h"
#include "pgpContext.h"

struct PGPSigSpec {
	PGPContextRef	cdkContext;
	PGPSigSpec *next;
	PGPSecKey *seckey;
	PgpVersion version;
	PGPByte hashtype;
	PGPByte extra[5];			/* Sigtype, timestamp */

	PktList	*pkl;				/* Other subpackets */
	PktList	*pki;				/* Pointer into pkl for iteration */
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
		sizeof (*ss), kPGPMemoryMgrFlags_Clear);
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
			hash = kPGPHashAlgorithm_SHA;
		}

		ss->cdkContext	= cdkContext;
		ss->seckey = sec;
#if 0
		ss->exportable = TRUE;
		ss->revocable = TRUE;
#endif
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
	PktList *pki, *sspk, **psspk;

	if (!spec)
		return NULL;

	ss = (PGPSigSpec *)pgpContextMemAlloc( spec->cdkContext,
		sizeof (*ss), 0);
	if (ss) {
		memcpy (ss, spec, sizeof (*ss));
		ss->next = NULL;
	}
	/* Copy over packetlist */
	ss->pkl = ss->pki = NULL;
	psspk = &ss->pkl;	/* Point at "next" of last entry in ss->pkl */
	for (pki = spec->pkl; pki; pki = pki->next) {
		sspk = pgpPktListNew( spec->cdkContext, pki->type, pki->buf,
							  pki->len );
		if( IsNull( sspk ) )
			return NULL;	/* out of memory */
		*psspk = sspk;
		psspk = &sspk->next;
		/* Copy iterator position in new list */
		if (pki == spec->pki)
			ss->pki = sspk;
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
		pgpPktListFreeList( ss->pkl );
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
	if (spec->version < PGPVERSION_4)
		spec->version = PGPVERSION_4;
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


/* Remaining functions use packetlist */

static PktList *
sSearchPkts( PGPSigSpec const *spec, int type, int nth )
{
	PktList *pki;

	for( pki = spec->pkl; pki; pki = pki->next ) {
		if( (pki->type & kPGPSigFlags_Type) == type )
			if (nth-- == 0)
				break;
	}
	return pki;
}

#define sSigFlags(flags) \
	(((flags) & ~kPGPSigFlags_Type) | kPGPSigFlags_Present)


/* Get expiration */
PGPUInt32
pgpSigSpecSigExpiration (PGPSigSpec const *spec, PGPUInt32 *sigExpire)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_EXPIRATION, 0 );
	if( IsNull( pkt ) ) {
		*sigExpire = 0;
		return 0;
	}
	pgpAssert (pkt->len == sizeof (*sigExpire));
	*sigExpire = *(PGPUInt32 *)pkt->buf;
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetSigExpiration (PGPSigSpec *spec, PGPUInt32 flags,
							PGPUInt32 sigExpire)
{
	PktList *pkt;

	flags |= SIGSUB_EXPIRATION;
	pkt = pgpPktListNew( spec->cdkContext, flags, (PGPByte *)&sigExpire,
						 sizeof(sigExpire) );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}


PGPUInt32
pgpSigSpecExportable (PGPSigSpec const *spec, PGPBoolean *exportable)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_EXPORTABLE, 0 );
	if( IsNull( pkt ) ) {
		*exportable = 0;
		return 0;
	}
	pgpAssert (pkt->len == sizeof (*exportable));
	*exportable = *(PGPBoolean *)pkt->buf;
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetExportable (PGPSigSpec *spec, PGPUInt32 flags,
						 PGPBoolean exportable)
{
	PktList *pkt;

	flags |= SIGSUB_EXPORTABLE;
	pkt = pgpPktListNew( spec->cdkContext, flags, (PGPByte *)&exportable,
						 sizeof(exportable) );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}

PGPUInt32
pgpSigSpecRevocable (PGPSigSpec const *spec, PGPBoolean *revocable)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_REVOCABLE, 0 );
	if( IsNull( pkt ) ) {
		*revocable = 0;
		return 0;
	}
	pgpAssert (pkt->len == sizeof (*revocable));
	*revocable = *(PGPBoolean *)pkt->buf;
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetRevocable (PGPSigSpec *spec, PGPUInt32 flags,
						PGPBoolean revocable)
{
	PktList *pkt;

	flags |= SIGSUB_REVOCABLE;
	pkt = pgpPktListNew( spec->cdkContext, flags, (PGPByte *)&revocable,
						 sizeof(revocable) );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}

PGPUInt32
pgpSigSpecTrustLevel (PGPSigSpec const *spec, PGPByte *trustLevel,
					  PGPByte *trustValue)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_TRUST, 0 );
	if( IsNull( pkt ) ) {
		*trustLevel = 0;
		*trustValue = 0;
		return 0;
	}
	pgpAssert (pkt->len == sizeof (*trustLevel) + sizeof (*trustValue));
	*trustLevel = pkt->buf[0];
	*trustValue = pkt->buf[1];
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetTrustLevel (PGPSigSpec *spec, PGPUInt32 flags,
						 PGPByte trustLevel, PGPByte trustValue)
{
	PktList *pkt;
	PGPByte trustArray[2];

	flags |= SIGSUB_TRUST;
	trustArray[0] = trustLevel;
	trustArray[1] = trustValue;
	pkt = pgpPktListNew( spec->cdkContext, flags, trustArray,
						 sizeof(trustArray) );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}

PGPUInt32
pgpSigSpecRegExp (PGPSigSpec const *spec, char **regExp)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_REGEXP, 0 );
	if( IsNull( pkt ) ) {
		*regExp = 0;
		return 0;
	}
	*regExp = (char *)pkt->buf;
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetRegExp (PGPSigSpec *spec, PGPUInt32 flags, char const *regExp)
{
	PktList *pkt;
	PGPSize len;

	flags |= SIGSUB_REGEXP;
	len = strlen( regExp ) + 1;	/* include null */
	pkt = pgpPktListNew( spec->cdkContext, flags,
				(const PGPByte *) regExp, len );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}

/* Access functions for self-sig related signature subpackets */

PGPUInt32
pgpSigSpecKeyExpiration (PGPSigSpec const *spec, PGPUInt32 *keyExpire)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_KEY_EXPIRATION, 0 );
	if( IsNull( pkt ) ) {
		*keyExpire = 0;
		return 0;
	}
	pgpAssert (pkt->len == sizeof (*keyExpire));
	*keyExpire = *(PGPUInt32 *)pkt->buf;
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetKeyExpiration (PGPSigSpec *spec, PGPUInt32 flags,
							PGPUInt32 keyExpire)
{
	PktList *pkt;

	flags |= SIGSUB_KEY_EXPIRATION;
	pkt = pgpPktListNew( spec->cdkContext, flags, (PGPByte *)&keyExpire,
						 sizeof(keyExpire) );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}

PGPUInt32
pgpSigSpecPrefAlgs (PGPSigSpec const *spec, PGPByte **prefalgs,
					PGPSize *preflen)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_PREFERRED_ENCRYPTION_ALGS, 0 );
	if( IsNull( pkt ) ) {
		*prefalgs = 0;
		*preflen = 0;
		return 0;
	}
	*preflen = pkt->len;
	*prefalgs = pkt->buf;
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetPrefAlgs (PGPSigSpec *spec, PGPUInt32 flags,
					   PGPByte const *algs, size_t len)
{
	PktList *pkt;

	flags |= SIGSUB_PREFERRED_ENCRYPTION_ALGS;
	pkt = pgpPktListNew( spec->cdkContext, flags, algs, len );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}

PGPUInt32
pgpSigSpecAdditionalRecipientRequest (PGPSigSpec const *spec,
									  PGPByte **arr, size_t *arrlen,
									  int nth)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_KEY_ADDITIONAL_RECIPIENT_REQUEST, nth );
	if( IsNull( pkt ) ) {
		*arr = 0;
		*arrlen = 0;
		return 0;
	}
	*arrlen = pkt->len;
	*arr = pkt->buf;
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetAdditionalRecipientRequest  (PGPSigSpec *spec, PGPUInt32 flags,
										  PGPByte const *krinfo, PGPSize len)
{
	PktList *pkt;

	flags |= SIGSUB_KEY_ADDITIONAL_RECIPIENT_REQUEST;
	pkt = pgpPktListNew( spec->cdkContext, flags, krinfo, len );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}

PGPUInt32
pgpSigSpecRevocationKey (PGPSigSpec const *spec, PGPByte **rev,
						 PGPSize *revlen, int nth)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_KEY_REVOCATION_KEY, nth );
	if( IsNull( pkt ) ) {
		*rev = 0;
		*revlen = 0;
		return 0;
	}
	*revlen = pkt->len;
	*rev = pkt->buf;
	return sSigFlags( pkt->type );
}

PGPError
pgpSigSpecSetRevocationKey  (PGPSigSpec *spec, PGPUInt32 flags,
							 PGPByte const *krinfo, size_t len)
{
	PktList *pkt;

	flags |= SIGSUB_KEY_REVOCATION_KEY;
	pkt = pgpPktListNew( spec->cdkContext, flags, krinfo, len );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}

/* Unrecognized subpacket type which we will copy over blindly */
PGPUInt32
pgpSigSpecPacket (PGPSigSpec const *spec, PGPByte **extradata,
				  PGPSize *len, int nth)
{
	PktList *pkt;

	pkt = sSearchPkts( spec, SIGSUB_UNRECOGNIZED, nth );
	if( IsNull( pkt ) ) {
		*extradata = 0;
		*len = 0;
		return 0;
	}
	*len = pkt->len;
	*extradata = pkt->buf;
	return sSigFlags( pkt->type );
}	

/* Unrecognized subpacket type which we will copy over blindly */
PGPError
pgpSigSpecSetPacket (PGPSigSpec *spec, PGPUInt32 flags,
							 PGPByte const *extradata, size_t len)
{
	PktList *pkt;

	flags |= SIGSUB_UNRECOGNIZED;
	pkt = pgpPktListNew( spec->cdkContext, flags, extradata, len );
	if( IsNull( pkt ) )
		return kPGPError_OutOfMemory;
	pkt->next = spec->pkl;
	spec->pkl = pkt;
	sSubPacketVersion(spec);
	return kPGPError_NoErr;
}


/*
 * Search for and remove packet of specified type.  Returns flags associated
 * with packet, including kPGPSigFlags_Present; returns 0 if not found.
 */
PGPUInt32
pgpSigSpecRemove (PGPSigSpec *spec, PGPUInt32 type)
{
	PktList *pki;
	PktList **pkip;

	pkip = &spec->pkl;
	for( pki = spec->pkl; pki; pki = pki->next ) {
		if( (PGPUInt32)(pki->type & kPGPSigFlags_Type) == type )
			break;
		pkip = &pki->next;
	}
	if (!pki)
		return 0;
	*pkip = pki->next;
	type = pki->type;
	pgpPktListFreeOne( pki );
	return sSigFlags( type );
}

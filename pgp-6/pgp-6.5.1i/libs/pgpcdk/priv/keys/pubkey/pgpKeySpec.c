/*
 * $Id: pgpKeySpec.c,v 1.18 1998/06/11 18:28:19 hal Exp $
 */

#include "pgpConfig.h"

#include <time.h>
#include <string.h>

#include "pgpKeySpec.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpTimeDate.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

/*
 * The things in a public key that aren't part of the (algorithm-specific)
 * mathematical public key.  Personally, I'd prefer that this didn't exist!
 */
struct PGPKeySpec
{
	PGPContextRef	context;
	PgpVersion version;			/* Version of key to create */
	PGPUInt32 creation;			/* Creation date */
	PGPUInt16 expirationDays;			/* Validity period */
	DEBUG_STRUCT_CONSTRUCTOR( PGPKeySpec )
} ;

PGPKeySpec *
pgpKeySpecCreate(PGPEnv const *env)
{
	PGPKeySpec *ks;
	PGPContextRef		cdkContext	= pgpenvGetContext( env );

	ks = (PGPKeySpec *)pgpContextMemAlloc( cdkContext,
		sizeof(*ks), kPGPMemoryMgrFlags_Clear);
	if (ks) {
		ks->context	= cdkContext;
		/* Default values, based on environment */
		/*ks->version = pgpenvGetInt(env, PGPENV_VERSION, NULL, NULL);*/
		ks->version = PGPVERSION_3;	/* Default to this unless overridden */
		ks->creation = pgpTimeStamp(pgpenvGetInt(env, PGPENV_TZFIX,
		                                         NULL, NULL));
		ks->expirationDays = 0;	/* Forever */
	}
	return ks;
}

PGPKeySpec *
pgpKeySpecCopy(PGPKeySpec const *ks)
{
	PGPKeySpec *ks2;

	ks2 = (PGPKeySpec *)pgpContextMemAlloc( ks->context,
		sizeof(*ks2), kPGPMemoryMgrFlags_Clear);
	if (ks2)
		memcpy(ks2, ks, sizeof(*ks2));
	return ks2;
}

void
pgpKeySpecDestroy(PGPKeySpec *ks)
{
	PGPContextRef		cdkContext;
	
	pgpAssertAddrValid( ks, PGPKeySpec );
	cdkContext	= ks->context;
	
	pgpClearMemory( ks,  sizeof(*ks));
	pgpContextMemFree( cdkContext, ks);
}

PgpVersion
pgpKeySpecVersion(PGPKeySpec const *ks)
{
	return ks->version;
}

PGPUInt32
pgpKeySpecCreation(PGPKeySpec const *ks)
{
	return ks->creation;
}

PGPUInt16
pgpKeySpecValidity(PGPKeySpec const *ks)
{
	return ks->expirationDays;
}

int
pgpKeySpecSetVersion(PGPKeySpec *ks, PgpVersion ver)
{
	switch(ver) {
	  case PGPVERSION_2:
	  case PGPVERSION_3:
	  case PGPVERSION_4:
		ks->version = ver;
		return 0;
	}
	/* default */
	return kPGPError_UnknownKeyVersion;
}

int
pgpKeySpecSetCreation(PGPKeySpec *ks, PGPUInt32 creation)
{
	ks->creation = creation;
	return 0;
}

int
pgpKeySpecSetValidity(PGPKeySpec *ks, PGPUInt16 expirationDays )
{
	ks->expirationDays = expirationDays;
	return 0;
}

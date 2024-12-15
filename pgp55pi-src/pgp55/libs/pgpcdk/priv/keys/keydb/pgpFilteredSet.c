/*
 * pgpFilteredSet.c
 * Filtered variant of key sets
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
 * All rights reserved
 *
 * $Id: pgpFilteredSet.c,v 1.27 1997/09/14 20:46:37 lloyd Exp $
 */

#include "pgpConfig.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <string.h>
#include <ctype.h>

#include "pgpKDBInt.h"
#include "pgpDebug.h"
#include "pgpErrors.h"
#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpContext.h"


/* Private data for KeySet of type filter */
typedef struct FilteredSetPriv
{
	PGPKeySetRef	origSet;
	PGPFilterRef	filter;
} FilteredSetPriv;


	static PGPBoolean
sFilteredSetIsMember(
	PGPKeySetRef	keys,
	RingObject *	obj )
{
	FilteredSetPriv *	priv = (FilteredSetPriv *) keys->priv;

	pgpa((
		pgpaPGPKeySetValid( keys ),
		pgpaAddrValid( priv, FilteredSetPriv )));

	return priv->origSet->isMember( priv->origSet, obj )
		&& pgpKeyObjMatchesFilter( priv->filter,
								   pgpKeyDBRingSet( keys->keyDB ),
								   obj );
}	

	static void
sFilteredSetDestroy(
	PGPKeySet *		keys )
{
	PGPContextRef		context = PGPGetKeySetContext( keys );
	FilteredSetPriv *	priv = (FilteredSetPriv *)keys->priv;

	pgpa((
		pgpaAddrValid( keys, PGPKeySet ),
		pgpaAddrValid( priv, FilteredSetPriv )));

	PGPFreeKeySet( priv->origSet );
	PGPFreeFilter( priv->filter );
	pgpContextMemFree( context, priv);
}


/*
 * Create a filtered KeySet based on the same KeyDB as the specified
 * original.  The original must be unfiltered.  The filtering is based
 * on a name substring (not keyid).
 */
	PGPError
PGPFilterKeySet (
	PGPKeySetRef		origSet,
	PGPFilterRef		filter,
	PGPKeySetRef *		outSet )
{
	PGPContextRef		context	= PGPGetKeySetContext( origSet );
	PGPError			err		= kPGPError_NoErr;
	FilteredSetPriv *	priv	= NULL;
	PGPKeySetRef		newSet	= kPGPInvalidRef;

	PGPValidatePtr( outSet );
	*outSet	= kPGPInvalidRef;
	PGPValidateKeySet( origSet );
	PGPValidateFilter( filter );

	priv = (FilteredSetPriv *)pgpContextMemAlloc( context,
												  sizeof( *priv ),
												  kPGPMemoryFlags_Clear );
	if (priv == NULL)
		return kPGPError_OutOfMemory;

	err = pgpNewKeySetInternal( origSet->keyDB, &newSet );
	if ( IsPGPError( err ) )
	{
		pgpContextMemFree( context, priv );
		return err;
	}

	priv->origSet		= origSet;
	PGPIncKeySetRefCount(origSet);

	priv->filter		= filter;
	PGPIncFilterRefCount(filter);

	newSet->priv		= priv;
	newSet->isMember	= sFilteredSetIsMember;
	newSet->makeUnion	= NULL;
	newSet->destroy		= sFilteredSetDestroy;

	*outSet = newSet;
	pgpAssertErrWithPtr( err, *outSet );
	return err;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

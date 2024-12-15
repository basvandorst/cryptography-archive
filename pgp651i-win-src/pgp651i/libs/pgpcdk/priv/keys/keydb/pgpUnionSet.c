/*
 * pgpUnionSet.c
 * Arbitrary union variant of PGPKeySet.
 *
 * Copyright (C) 1997 Network Associates Inc. and affiliated companies.
 * All rights reserved
 *
 * $Id: pgpUnionSet.c,v 1.4 1999/03/10 02:59:39 heller Exp $
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


/* Private data for the arbitrary union variant of PGPKeySet */
typedef struct UnionSetPriv
{
	PGPKeySetRef	set1;
	PGPKeySetRef	set2;
} UnionSetPriv;


/*
 * Check to see if a key is a member of an enumerated subset
 */
	static PGPBoolean
IsMemberOfUnionSet(
	PGPKeySetRef		set,
	PGPKeyRef			key)
{
	UnionSetPriv *		priv = (UnionSetPriv *)set->priv;

	pgpa((
		pgpaPGPKeySetValid(set),
		pgpaAddrValid(priv, UnionSetPriv),
		pgpaAddrValid(key, VoidAlign)));

	return (priv->set1->isMember(priv->set1, key)
		 || priv->set2->isMember(priv->set2, key));
}

	static void
DestroyUnionSet(
	PGPKeySetRef		set)
{
	UnionSetPriv *		priv = (UnionSetPriv *)set->priv;

	pgpa((
		pgpaAddrValid(set, PGPKeySet),
		pgpaAddrValid(priv, UnionSetPriv)));

	PGPFreeKeySet(priv->set1);
	PGPFreeKeySet(priv->set2);
}

	PGPError
pgpGenericUnionOfKeySets(
	PGPKeySetRef		set1,
	PGPKeySetRef		set2,
	PGPKeySetRef *		newSet)
{
	PGPContextRef		context	= PGPGetKeySetContext(set1);
	UnionSetPriv *		priv;
	PGPKeySetRef		set;
	PGPError			err;

	*newSet = NULL;		/* In case there's an error */

	priv = (UnionSetPriv *)pgpContextMemAlloc(context,
								sizeof(*priv), kPGPMemoryMgrFlags_Clear);
	if (IsNull(priv))
		return kPGPError_OutOfMemory;

	priv->set1 = set1;
	priv->set2 = set2;

	err = pgpNewKeySetInternal(set1->keyDB, &set);
	if (IsPGPError(err))
		return err;
	
	pgpAssertAddrValid(set, PGPKeySet);

	set->priv = priv;
	set->isMember = IsMemberOfUnionSet;
	set->makeUnion = NULL;
	set->destroy = DestroyUnionSet;

	PGPIncKeySetRefCount(set1);
	PGPIncKeySetRefCount(set2);

	*newSet = set;
	return kPGPError_NoErr;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*
 * pgpEnumeratedSet.c
 * Arbitrary enumerated subset variant of pgpKeySet.
 *
 * Copyright (C) 1997 Network Associates, Inc. and its affiliates.
 * All rights reserved
 *
 * $Id: pgpEnumeratedSet.c,v 1.10.12.1 1998/11/12 03:21:51 heller Exp $
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
#include "pgpKeyIDPriv.h"
#include "pgpRngPub.h"


/* Private data for KeySet of type: enumerated subset */
typedef struct EnumeratedSubsetPriv
{
	RingObject **	keyArray;	/* Array of keys, sorted by keyid */
	long			numKeys;
} EnumeratedSubsetPriv;


static PGPError	NewEnumeratedSubsetInternal(PGPKeyDBRef		db,
											PGPKeySetRef *	newSet);


/* XXX Assumes that keyIDs are 8 bytes long */
	static int
compareRawKeyIDs(
	PGPKeyID const *	keyIDA,
	PGPKeyID const *	keyIDB)
{
#if 0
	int				i;

	for (i = 0; i < 8; i++)
	{
		if (keyIDA->bytes[i] > keyIDB->bytes[i])
			return 1;
		else if (keyIDA->bytes[i] < keyIDB->bytes[i])
			return -1;
	}
	return 0;
#else
	return( PGPCompareKeyIDs( keyIDA, keyIDB ) );
#endif
}

/*
 * Check to see if a key is a member of an enumerated subset
 */
	static PGPBoolean
IsMemberOfEnumeratedSubset(
	PGPKeySetRef			set,
	RingObject *			obj)
{
	EnumeratedSubsetPriv *	priv = (EnumeratedSubsetPriv *)set->priv;
	RingSet const *			ringSet = pgpKeyDBRingSet(set->keyDB);
	RingObject *			keyObj;
	PGPKeyID				keyID;
	long					lowerBound;
	long					upperBoundPlusOne;
	long					indexOfEqualKeyID;
	long					indexTry	= 0;
	RingObject *			keyObjTry;
	PGPKeyID				keyIDTry;
	int						comparison;

	pgpa((
		pgpaPGPKeySetValid(set),
		pgpaAddrValid(priv, EnumeratedSubsetPriv),
		pgpaAddrValid(obj, VoidAlign)));

	if (!ringSetIsMember(ringSet, obj))
		return FALSE;

	{
		RingIterator *		ringIter;

		/* First get the master key object (XXX this may not be necessary) */
		ringIter = ringIterCreate(ringSet);
		pgpAssert(ringIter != NULL);
		ringIterSeekTo(ringIter, obj);
		keyObj = ringIterCurrentObject(ringIter, 1);
		pgpAssert(keyObj != NULL);
		ringIterDestroy(ringIter);
	}

	/* Get the KeyID */
	ringKeyID8(ringSet, keyObj, NULL, &keyID);

	/* Use binary search to find a key in the array with the same KeyID */
	lowerBound = 0;
	upperBoundPlusOne = priv->numKeys;
	while (lowerBound < upperBoundPlusOne)
	{
		indexTry = (lowerBound + upperBoundPlusOne) / 2;
		keyObjTry = priv->keyArray[indexTry];
		if (keyObjTry == keyObj)
			return TRUE;	/* Found it! */

		ringKeyID8(ringSet, keyObjTry, NULL, &keyIDTry);
		comparison = compareRawKeyIDs(&keyID, &keyIDTry);
		if (comparison > 0)
			lowerBound = indexTry + 1;
		else if (comparison < 0)
			upperBoundPlusOne = indexTry;
		else
			break;
	}
	if (lowerBound >= upperBoundPlusOne)
		return FALSE;	/* Binary search failed */
	
	/*
	 * We found a key with the same KeyID, but it's not the same key.
	 * Now we search the adjacent keys of equal KeyID looking for the key.
	 */
	indexOfEqualKeyID = indexTry;

	/* First search lower indices */
	while (--indexTry >= lowerBound)
	{
		keyObjTry = priv->keyArray[indexTry];
		if (keyObjTry == keyObj)
			return TRUE;	/* Found it! */

		ringKeyID8(ringSet, keyObjTry, NULL, &keyIDTry);
		if (0 != compareRawKeyIDs(&keyID, &keyIDTry))
			break;
	}

	/* Then search higher indices */
	indexTry = indexOfEqualKeyID;
	while (++indexTry < upperBoundPlusOne)
	{
		keyObjTry = priv->keyArray[indexTry];
		if (keyObjTry == keyObj)
			return TRUE;	/* Found it! */

		ringKeyID8(ringSet, keyObjTry, NULL, &keyIDTry);
		if (0 != compareRawKeyIDs(&keyID, &keyIDTry))
			break;
	}

	/* Now we know for sure it's not in the set */
	return FALSE;
}

	static PGPError
MakeUnionOfEnumeratedSubsets(
	PGPKeySetRef		set1,
	PGPKeySetRef		set2,
	PGPKeySetRef *		newSet)
{
	PGPContextRef			context	= PGPGetKeySetContext(set1);
	RingSet const *			ringSet = pgpKeyDBRingSet(set1->keyDB);
	EnumeratedSubsetPriv *	priv1 = (EnumeratedSubsetPriv *)set1->priv;
	EnumeratedSubsetPriv *	priv2 = (EnumeratedSubsetPriv *)set2->priv;
	EnumeratedSubsetPriv *	priv;
	RingObject **			keyArray1 = priv1->keyArray;
	RingObject **			keyArray2 = priv2->keyArray;
	RingObject **			keyArray;
	PGPKeyID				keyID1;
	PGPKeyID				keyID2;
	PGPKeySetRef			set;
	long					in1;
	long					in2;
	long					out;
	int						comparison;
	PGPError				err	= kPGPError_NoErr;

	*newSet = NULL;			/* In case there's an error */

	err = NewEnumeratedSubsetInternal(set1->keyDB, &set);
	if (IsPGPError(err))
		return err;

	priv = (EnumeratedSubsetPriv *)set->priv;
	priv->numKeys = priv1->numKeys + priv2->numKeys;

	keyArray = (RingObject **)pgpContextMemAlloc(context,
							priv->numKeys * sizeof(RingObject *), 0);
	if (IsNull(keyArray))
	{
		PGPFreeKeySet(set);
		return kPGPError_OutOfMemory;
	}

	in1 = in2 = out = 0;

	/* Merge the two sorted arrays until we reach the end of one of them */
	while (in1 < priv1->numKeys && in2 < priv2->numKeys)
	{
		/*
		 * Check for duplicate keys.  Note that if keyIDs aren't unique
		 * then this won't necessarily remove duplicates.  This isn't
		 * a bug, because duplicates needn't be removed for correctness.
		 * It is merely an optimization.
		 */
		if (keyArray1[in1] == keyArray2[in2])
		{
			in2++;
			continue;
		}

		ringKeyID8(ringSet, keyArray1[in1], NULL, &keyID1);
		ringKeyID8(ringSet, keyArray2[in2], NULL, &keyID2);

		comparison = compareRawKeyIDs(&keyID1, &keyID2);
		if (comparison <= 0)
			keyArray[out++] = keyArray1[in1++];
		else if (comparison > 0)
			keyArray[out++] = keyArray2[in2++];
	}

	/*
	 * Copy the rest of the remaining array.
	 * (at most one of these loops will be non-empty)
	 */
	while (in1 < priv1->numKeys)
		keyArray[out++] = keyArray1[in1++];
	while (in2 < priv2->numKeys)
		keyArray[out++] = keyArray2[in2++];

	/* Shrink the resulting array, in case there were duplicates */
	if (out < priv->numKeys)
	{
		priv->numKeys = out;
		err = pgpContextMemRealloc(context, (void **)&keyArray,
									out * sizeof(RingObject *), 0);
		pgpAssert(IsntPGPError(err));	/* Shrinking should never fail */
	}

	priv->keyArray = keyArray;
	*newSet = set;
	return kPGPError_NoErr;
}

	static void
RemoveKeyFromEnumeratedSubset(
	PGPKeySetRef	set,
	PGPKeyRef		key)
{
	PGPContextRef			context	= PGPGetKeySetContext(set);
	EnumeratedSubsetPriv *	priv	= (EnumeratedSubsetPriv *)set->priv;
	RingObject *			keyObj	= key->key;
	RingObject **			end		= priv->keyArray + priv->numKeys;
	RingObject **			out;
	RingObject **			in;

	/* First a fast read-only scan to find the first occurrence of the key */
	in = priv->keyArray;
	while (in < end && *in != keyObj)
		in++;

	/*
	 * Then a slower read-write pass to move the rest of the keys
	 * down in memory, finding duplicates as well.
	 */
	out = in;
	for (; in < end; in++)
		if (*in != keyObj)
			*out++ = *in;

	if (out < end)
	{
		priv->numKeys = out - priv->keyArray;

		/* Shrinking should never fail, but if it does we don't care */
		pgpContextMemRealloc(context, (void **)&priv->keyArray,
							 priv->numKeys * sizeof(RingObject *), 0);
	}
}

	static void
DestroyEnumeratedSubset(
	PGPKeySetRef	set)
{
	EnumeratedSubsetPriv *	priv = (EnumeratedSubsetPriv *)set->priv;
	PGPContextRef			context	= PGPGetKeySetContext(set);

	if (IsntNull(priv->keyArray))
	{
		pgpContextMemFree(context, priv->keyArray);
		priv->keyArray = NULL;
	}
	priv->numKeys = 0;
	pgpContextMemFree(context, priv);
}

	static PGPError
NewEnumeratedSubsetInternal(
	PGPKeyDBRef			db,
	PGPKeySetRef *		newSet)
{
	PGPContextRef			context	= pgpGetKeyDBContext(db);
	EnumeratedSubsetPriv *	priv;
	PGPKeySetRef			set;
	PGPError				err	= kPGPError_NoErr;

	*newSet = NULL;		/* In case there's an error */

	priv = (EnumeratedSubsetPriv *)pgpContextMemAlloc(context,
							sizeof(*priv), kPGPMemoryMgrFlags_Clear);
	if (IsNull(priv))
		return kPGPError_OutOfMemory;

	priv->keyArray = NULL;
	priv->numKeys = 0;

	err = pgpNewKeySetInternal(db, &set);
	if (IsPGPError(err))
		return err;
	
	pgpAssertAddrValid(set, PGPKeySet);

	set->priv = priv;
	set->isMember = IsMemberOfEnumeratedSubset;
	set->makeUnion = MakeUnionOfEnumeratedSubsets;
	set->removeKey = RemoveKeyFromEnumeratedSubset;
	set->destroy = DestroyEnumeratedSubset;

	*newSet = set;
	return kPGPError_NoErr;
}

/*
 * Create an empty enumerated key set
 */
	PGPError
PGPNewEmptyKeySet(
	PGPKeySetRef		origSet,
	PGPKeySetRef *		newSet)
{
	PGPKeySetRef		set;
	PGPError			err	= kPGPError_NoErr;

	PGPValidatePtr( newSet );
	*newSet = NULL;	
	PGPValidateKeySet( origSet );

	err = NewEnumeratedSubsetInternal(origSet->keyDB, &set);
	if (IsPGPError(err))
		return err;

	*newSet = set;
	pgpAssertErrWithPtr( err, *newSet );
	return err;
}

/*
 * Create a singleton enumerated key set
 */
	PGPError
PGPNewSingletonKeySet(
	PGPKeyRef			key,
	PGPKeySetRef *		newSet)
{
	PGPContextRef			context	= PGPGetKeyContext(key);
	EnumeratedSubsetPriv *	priv;
	PGPKeySetRef			set;
	PGPError				err	= kPGPError_NoErr;

	PGPValidatePtr( newSet );
	*newSet = NULL;			/* In case there's an error */
	PGPValidateKey( key );

	err = NewEnumeratedSubsetInternal(key->keyDB, &set);
	if (IsPGPError(err))
		return err;

	priv = (EnumeratedSubsetPriv *)set->priv;
	priv->keyArray = (RingObject **)pgpContextMemAlloc(context,
											sizeof(RingObject *), 0);
	if (IsNull(priv->keyArray))
	{
		PGPFreeKeySet(set);
		return kPGPError_OutOfMemory;
	}
	priv->numKeys = 1;
	priv->keyArray[0] = key->key;

	*newSet = set;
	pgpAssertErrWithPtr( err, *newSet );
	return err;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

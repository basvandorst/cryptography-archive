/*
 * PGPKeySet implementation
 *
 * Copyright (C) 1996,1997 Network Associates Inc. and affiliated companies.
 * All rights reserved
 *
 * $Id: pgpKeySet.c,v 1.89.8.1 1999/06/04 00:28:55 heller Exp $
 */

#include "pgpConfig.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <string.h>
#include <ctype.h>

#include "pgpKDBInt.h"
#include "pgpTypes.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpTimeDate.h"
#include "pgpUsuals.h"
#include "pgpMemPool.h"
#include "pgpRngMnt.h"
#include "pgpRngPub.h"
#include "pgpContext.h"
#include "pgpKeyIDPriv.h"
#include "pgpOptionListPriv.h"


	static void
sGetKeyID(
	PGPKeyRef	key,
	PGPKeyID *	keyID )
{
	RingSet const *	ringset;
	
	ringset	= pgpKeyDBRingSet (key->keyDB);
	
	ringKeyID8( ringset, key->key, NULL, keyID );
}

/* XXX Assumes that keyIDs are 8 bytes long */
	static int
compareKeyIDs(
	PGPKeyID const *	keyIDA,
	PGPKeyID const *	keyIDB)
{
#if 0
	int				i;
	for (i = 4; i < 8; i++)
	{
		if (keyIDA->bytes[i] > keyIDB->bytes[i])
			return 1;
		else if (keyIDA->bytes[i] < keyIDB->bytes[i])
			return -1;
	}
	for (i = 0; i < 4; i++)
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

	static int
keyCompareByKeyID(void const *a, void const *b)
{
	PGPKey *	keyA = *(PGPKey **)a;
	PGPKey *	keyB = *(PGPKey **)b;
	
	PGPKeyID	rawA;
	PGPKeyID	rawB;
	
	int			result	= 1;

	sGetKeyID( keyA, &rawA );
	sGetKeyID( keyB, &rawB );

	result	= compareKeyIDs( &rawA, &rawB);
	
	return( result );
}

	static int
keyCompareByReverseKeyID(void const *a, void const *b)
{
	return -keyCompareByKeyID(a, b);
}

	PGPInt32
PGPCompareUserIDStrings(char const *a, char const *b)
{
	pgpAssert( IsntNull( a ) );
	pgpAssert( IsntNull( b ) );
	
	if ( IsNull( a ) || IsNull( b ) )
		return( 0 );
		
		
	for (;;)
	{
		while (*a && tolower(*a) == tolower(*b))
			a++, b++;
		while (*a && !isalnum((int) (*a)))
			a++;
		while (*b && !isalnum((int) (*b)))
			b++;
		if (!*a || tolower(*a) != tolower(*b))
			break;
		a++;
		b++;
	}
	return (uchar)tolower(*a) - (uchar)tolower(*b);
}

	static int
keyCompareByUserID(void const *a, void const *b)
{
	PGPKeyRef		keyA = *(PGPKey **)a;
	PGPKeyRef		keyB = *(PGPKey **)b;

	char			nameA[ kPGPMaxUserIDSize ];
	char			nameB[ kPGPMaxUserIDSize ];
	int				compareResult	= 0;
	PGPSize			actualLength;
	
	/* if we get an error, it's OK; we'll just end up comparing the first
		256 bytes */
	(void)PGPGetPrimaryUserIDNameBuffer( keyA,
		sizeof( nameA ), nameA, &actualLength );
	(void)PGPGetPrimaryUserIDNameBuffer( keyB,
			sizeof( nameB ), nameB, &actualLength );
			
	compareResult = PGPCompareUserIDStrings(nameA, nameB);
	
	if ( compareResult == 0 )
		compareResult	= keyCompareByKeyID(a, b);
		
	return compareResult;
}

	static int
keyCompareByReverseUserID(void const *a, void const *b)
{
	return -keyCompareByUserID(a, b);
}

	static int
keyCompareByValidity(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;

	PGPValidity		validityA;
	PGPValidity		validityB;
	PGPError		result;
	
	result = PGPGetPrimaryUserIDValidity(keyA, &validityA);
	pgpAssert(result == kPGPError_NoErr);
	result = PGPGetPrimaryUserIDValidity(keyB, &validityB);
	pgpAssert(result == kPGPError_NoErr);

	if (validityA < validityB)
		return 1;
	else if (validityA > validityB)
		return -1;
	else
		return keyCompareByKeyID(a, b);
}

	static int
keyCompareByReverseValidity(void const *a, void const *b)
{
	return -keyCompareByValidity(a, b);
}

	static int
keyCompareByTrust(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;

	PGPInt32		trustA;
	PGPInt32		trustB;
	PGPError		result;
	
	result = PGPGetKeyNumber(keyA, kPGPKeyPropTrust, &trustA);
	pgpAssert(result == kPGPError_NoErr);
	result = PGPGetKeyNumber(keyB, kPGPKeyPropTrust, &trustB);
	pgpAssert(result == kPGPError_NoErr);

	if (trustA < trustB)
		return 1;
	else if (trustA > trustB)
		return -1;
	else
		return keyCompareByKeyID(a, b);
}

	static int
keyCompareByReverseTrust(void const *a, void const *b)
{
	return -keyCompareByTrust(a, b);
}

	static int
keyCompareByEncryptKeySize(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;
	PGPSubKey *		subKeyA = NULL;
	PGPSubKey *		subKeyB = NULL;

	PGPInt32		keySizeA;
	PGPInt32		keySizeB;
	PGPError		err;
	
	err = pgpGetFirstSubKey(keyA, &subKeyA);
	if ( IsntPGPError( err ) )
		err = PGPGetSubKeyNumber(subKeyA, kPGPKeyPropBits, &keySizeA);
	else
		err = PGPGetKeyNumber(keyA, kPGPKeyPropBits, &keySizeA);
	pgpAssertNoErr( err );

	err = pgpGetFirstSubKey(keyB, &subKeyB);
	if ( IsntPGPError( err ) )
		err = PGPGetSubKeyNumber(subKeyB, kPGPKeyPropBits, &keySizeB);
	else
		err = PGPGetKeyNumber(keyB, kPGPKeyPropBits, &keySizeB);
	pgpAssertNoErr( err );
	
	if (keySizeA < keySizeB)
		return 1;
	else if (keySizeA > keySizeB)
		return -1;

	return keyCompareByKeyID(a, b);
}

	static int
keyCompareByReverseEncryptKeySize(void const *a, void const *b)
{
	return -keyCompareByEncryptKeySize(a, b);
}

	static int
keyCompareBySigKeySize(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;

	PGPInt32		keySizeA;
	PGPInt32		keySizeB;
	PGPError		result;
	
	result = PGPGetKeyNumber(keyA, kPGPKeyPropBits, &keySizeA);
	pgpAssert(result == kPGPError_NoErr);
	result = PGPGetKeyNumber(keyB, kPGPKeyPropBits, &keySizeB);
	pgpAssert(result == kPGPError_NoErr);

	if (keySizeA < keySizeB)
		return 1;
	else if (keySizeA > keySizeB)
		return -1;
	else
		return keyCompareByKeyID(a, b);
}

	static int
keyCompareByReverseSigKeySize(void const *a, void const *b)
{
	return -keyCompareBySigKeySize(a, b);
}

	static int
keyCompareByCreation(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;

	PGPTime			creationA;
	PGPTime			creationB;
	PGPError		result;
	
	result = PGPGetKeyTime(keyA, kPGPKeyPropCreation, &creationA);
	pgpAssert(result == kPGPError_NoErr);
	result = PGPGetKeyTime(keyB, kPGPKeyPropCreation, &creationB);
	pgpAssert(result == kPGPError_NoErr);

	if (creationA < creationB)
		return 1;
	else if (creationA > creationB)
		return -1;
	else
		return keyCompareByKeyID(a, b);
}

	static int
keyCompareByReverseCreation(void const *a, void const *b)
{
	return -keyCompareByCreation(a, b);
}

	static int
keyCompareByExpiration(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;

	PGPTime			expirationA;
	PGPTime			expirationB;
	PGPError		result;
	
	result = PGPGetKeyTime(keyA, kPGPKeyPropExpiration, &expirationA);
	pgpAssert(result == kPGPError_NoErr);
	result = PGPGetKeyTime(keyB, kPGPKeyPropExpiration, &expirationB);
	pgpAssert(result == kPGPError_NoErr);

	if (expirationA == expirationB)
		return keyCompareByKeyID(a, b);
	else if (expirationA == kPGPExpirationTime_Never)
		return -1;
	else if (expirationB == kPGPExpirationTime_Never)
		return 1;
	else if (expirationA < expirationB)
		return 1;
	else	/* expirationA > expirationB */
		return -1;
}

	static int
keyCompareByReverseExpiration(void const *a, void const *b)
{
	return -keyCompareByExpiration(a, b);
}

/*
 * The compare functions must all return non-ambiguous answers (>0,<0)
 * because the add-key functionality uses a binary search to install
 * new keys.  If things are ambiguous then the order can change if some
 * keys are tied under the main search.  This is accomplished by doing
 * a secondary search on keyid if there is a tie on the main search
 * field.
 */

typedef int (*CompareFunc)(void const *, void const *);

static const CompareFunc compareFunc[] = {
	NULL,
	NULL,
	keyCompareByUserID,
	keyCompareByReverseUserID,
	keyCompareByKeyID,
	keyCompareByReverseKeyID,
	keyCompareByValidity,
	keyCompareByReverseValidity,
	keyCompareByTrust,
	keyCompareByReverseTrust,
	keyCompareByEncryptKeySize,
	keyCompareByReverseEncryptKeySize,
	keyCompareBySigKeySize,
	keyCompareByReverseSigKeySize,
	keyCompareByCreation,
	keyCompareByReverseCreation,
	keyCompareByExpiration,
	keyCompareByReverseExpiration };
/* #if PGP_DEBUG */
static PGPKeyOrdering sNumCompareFuncs = (PGPKeyOrdering)
		(sizeof(compareFunc) /  sizeof(compareFunc[0]));
/* #endif */

	PGPInt32
PGPCompareKeys(PGPKey *a, PGPKey *b, PGPKeyOrdering order)
{
	pgpa((
		pgpaPGPKeyValid(a),
		pgpaPGPKeyValid(b),
		pgpaAssert(order > 0 && order < sNumCompareFuncs
					&& order != kPGPAnyOrdering)));

	if ( ! ( pgpKeyIsValid( a ) && pgpKeyIsValid( b ) ) )
		return( 0 );
		
	return (*compareFunc[order])(&a, &b);
}

	static void
sortKeyList(PGPKeyList *list)
{
	pgpa((
		pgpaPGPKeyListValid(list),
		pgpaAssert(list->order > 0 && list->order < sNumCompareFuncs)));

	if (list->order != kPGPAnyOrdering)
		qsort(list->keys, list->keyCount, sizeof(list->keys[0]),
				compareFunc[list->order]);
}

/*
 * Keep in mind that the comparison functions are not guaranteed to
 * be total orderings, and so even if an element of the list has a
 * perfect match with <key>, the index returned might not contain a
 * perfect match.
 */
	static long
binarySearchKeyList(PGPKeyList *list, PGPKey *key)
{
	long		lo;
	long		hi;
	long		i;
	int			result;
	int			(*compare)(void const *, void const *);

	pgpa((
		pgpaPGPKeyListValid(list),
		pgpaPGPKeyValid(key),
		pgpaAssert(list->order > 0 && list->order < sNumCompareFuncs)));

	if (list->order == kPGPAnyOrdering)
		return list->keyCount;

	compare = compareFunc[list->order];

	lo = 0;
	hi = list->keyCount;

	while (lo < hi)
	{
		i = (lo + hi) / 2;
		result = (*compare)(&key, &list->keys[i]);
		if (result > 0)
			lo = i + 1;
		else if (result < 0)
			hi = i;
		else
			return i;
	}
	return lo;
}

/*
 * WARNING: This dependency reference counting scheme breaks down
 *   in the case of cyclical dependencies.  For instance, if you have
 *   two KeyDBs which both add keys to each other, then neither KeyDB
 *   will ever be deallocated because they have each incremented the
 *   other's refCount.  We check below for cycles involving just two
 *   KeyDBs, but cycles larger than that will cause a leak.  This will
 *   be fixed in a future implementation.
 */
	static PGPError
pgpKeyDBAddDependency(PGPKeyDB *db, PGPKeyDB *dependency)
{
	PGPError	result	= kPGPError_NoErr;
	long		i;

	if (db == dependency)
		return kPGPError_NoErr;
	for (i = 0; i < db->numKeyDBDependencies; i++)
		if (db->keyDBDependencies[i] == dependency)
			return kPGPError_NoErr;

#if PGP_DEBUG
	/* Check for cycles involving two KeyDBs */
	for (i = 0; i < dependency->numKeyDBDependencies; i++)
		pgpAssert(dependency->keyDBDependencies[i] != db);
#endif

	if (db->numKeyDBDependencies >= db->numKeyDBDependenciesAllocated)
	{
		result = pgpContextMemRealloc( pgpGetKeyDBContext( db ),
							(void **)&db->keyDBDependencies,
							db->numKeyDBDependenciesAllocated * 2
									* sizeof(PGPKeyDB *),
							0 );
		if (IsPGPError(result))
			return result;

		db->numKeyDBDependenciesAllocated *= 2;
	}
	db->keyDBDependencies[db->numKeyDBDependencies++] = dependency;
	pgpIncKeyDBRefCount(dependency);
	return kPGPError_NoErr;
}

	static void
pgpKeyDBReleaseDependencies(PGPKeyDB *db)
{
	long		i;

	for (i = 0; i < db->numKeyDBDependencies; i++)
		pgpFreeKeyDB(db->keyDBDependencies[i]);
	db->numKeyDBDependencies = 0;
}


/* Creates a new empty key database */
	PGPKeyDB *
pgpKeyDBCreateInternal(PGPContextRef context)
{
	PGPKeyDB *	db;

	db = (PGPKeyDB *)pgpContextMemAlloc( context,
		sizeof(PGPKeyDB), kPGPMemoryMgrFlags_Clear);
	if (db == NULL)
		return NULL;

	db->priv = NULL;
	db->refCount = 1;
	db->firstSetInDB = NULL;
	db->context = context;

	db->numKeyDBDependenciesAllocated = 4;
	db->keyDBDependencies = (PGPKeyDB **)pgpContextMemAlloc( context,
								db->numKeyDBDependenciesAllocated
										* sizeof(PGPKeyDB *),
								0 );
	if (db->keyDBDependencies == NULL)
	{
		pgpContextMemFree( context, db );
		return NULL;
	}
	db->numKeyDBDependencies = 0;

	memPoolInit( context, &db->keyPool);
	db->numKeys = 0;
	db->firstKeyInDB = NULL;
	db->firstFreeKey = NULL;
	db->firstFreeUserID = NULL;
	db->firstFreeCert = NULL;
	db->keysByKeyID = NULL;

	return db;
}

/* Does any additional initialization necessary after DB is fully created */
	void
pgpKeyDBInitInternal(PGPKeyDB *db)
{
	/* Nothing for now */
	(void)db;
}

/* Does the final destruction of a key database structure */
	void
pgpKeyDBDestroyInternal(PGPKeyDB *db)
{
	PGPContextRef	context	= db->context;

	pgpAssert( pgpContextIsValid( context ) );
	
	pgpKeyDBReleaseDependencies( db );
	pgpContextMemFree( context, db->keyDBDependencies );

	memPoolEmpty( &db->keyPool );
	pgpClearMemory( db, sizeof(*db) );
	pgpContextMemFree ( context, db );
}

	static PGPKey *
allocKey(PGPKeyDB *db)
{
	PGPKey *	key;

	pgpa(pgpaPGPKeyDBValid(db));

	if (db->firstFreeKey != NULL)
	{
		key = db->firstFreeKey;
		db->firstFreeKey = key->nextKeyInDB;
	}
	else
		key = (PGPKey *)memPoolNew(&db->keyPool, PGPKey);
	
	if ( IsntNull( key ) )
	{
		pgpClearMemory(key, sizeof(*key));
		
#if PGP_OPTIONAL_MAGICS
		key->magic		= kPGPKeyMagic;
#endif
	}

	return key;
}

	static void
deallocKey(PGPKeyDB *db, PGPKey *key)
{
	pgpa((
		pgpaPGPKeyDBValid(db),
		pgpaAssert(db == key->keyDB)));

#if PGP_OPTIONAL_MAGICS
	key->magic		= ~key->magic;
#endif
	key->nextKeyInDB = db->firstFreeKey;
	db->firstFreeKey = key;
}

	static PGPError
addKeyToList(PGPKeyList *list, PGPKey *key)
{
	long			i;
	long			newKeyCount;
	PGPKey **		newKeys;
	PGPContextRef	context	= PGPGetKeyListContext( list );
	
	pgpa(pgpaPGPKeyListValid(list));

	i = binarySearchKeyList(list, key);

	newKeyCount = list->keyCount + 1;
	newKeys = (PGPKey **)pgpContextMemAlloc( context,
		newKeyCount * sizeof(PGPKey *), kPGPMemoryMgrFlags_Clear );
	if (!newKeys)
		return kPGPError_OutOfMemory;

	pgpCopyMemory(list->keys, newKeys, i * sizeof(PGPKey *));
	pgpCopyMemory(list->keys + i, newKeys + i + 1,
					(list->keyCount - i) * sizeof(PGPKey *));
	pgpContextMemFree( context, list->keys);

	list->keys = newKeys;
	list->keyCount = newKeyCount;

	newKeys[i] = key;
	pgpKeyIterAddKey(list, i);
	
	return kPGPError_NoErr;
}

	static PGPError
addKeyToLists(PGPKeyDB *db, PGPKey *key)
{
	PGPKeySet *		set;
	PGPKeyList *	list;
	PGPError		result;

	pgpa(pgpaPGPKeyDBValid(db));

	for (set = db->firstSetInDB; set; set = set->nextSetInDB)
	{
		pgpa(pgpaPGPKeySetValid(set));

		if (PGPKeySetIsMember( key, set))
			for (list = set->firstListInSet; list; list = list->nextListInSet)
				if ((result = addKeyToList(list, key)) != kPGPError_NoErr)
					return result;
	}
	return kPGPError_NoErr;
}

	static PGPError
removeKeyFromList(PGPKeyList *list, PGPKey *key)
{
	long			i;
	PGPContextRef	cdkContext	= PGPGetKeyListContext( list );

	pgpa(pgpaPGPKeyListValid(list));

	for (i = 0; i < list->keyCount; i++)
		if (list->keys[i] == key)
			break;
	
	if (i < list->keyCount)
	{
		pgpCopyMemory(list->keys + i + 1, list->keys + i,
						(list->keyCount - i - 1) * sizeof(PGPKey *));

		list->keyCount--;
		/*
		 * Reducing the size of a block cannot fail, and 
		 * if it did fail we don't care anyway.  Therefore,
		 * we can ignore errors from the following realloc.
		 */
		pgpContextMemRealloc(cdkContext, (void **)&list->keys,
							 list->keyCount * sizeof(PGPKey *), 0);

		pgpKeyIterRemoveKey(list, i);
	}
	
	return kPGPError_NoErr;
}

	static PGPError
removeKeyFromSets(PGPKeyDB *db, PGPKey *key)
{
	PGPKeySet *		set;
	PGPKeyList *	list;
	PGPError		result;

	pgpa(pgpaPGPKeyDBValid(db));

	for (set = db->firstSetInDB; set; set = set->nextSetInDB)
	{
		pgpa(pgpaPGPKeySetValid(set));

		set->removeKey(set, key);

		for (list = set->firstListInSet; list; list = list->nextListInSet)
			if ((result = removeKeyFromList(list, key)) != kPGPError_NoErr)
				return result;
	}
	return kPGPError_NoErr;
}

	PGPError
pgpReSortKeys(PGPKeyDB *db, RingSet *changed)
{
	PGPKeySet *		set;
	PGPKeyList *	list;
	PGPKey *		key;
	PGPKey **		movedKeys = NULL;
	long			numMovedKeys;
	long			movedKeysAlloc;
	long			i;
	int				(*compare)(void const *, void const *);
	PGPError		result = kPGPError_NoErr;
	PGPContextRef	context	= pgpGetKeyDBContext( db );

	pgpa(pgpaPGPKeyDBValid(db));

	movedKeysAlloc = 8;
	movedKeys = (PGPKey **)pgpContextMemAlloc( context,
		movedKeysAlloc * sizeof(PGPKey *), kPGPMemoryMgrFlags_Clear );
	if (!movedKeys)
	{
		result = kPGPError_OutOfMemory;
		goto done;
	}
	for (set = db->firstSetInDB; set; set = set->nextSetInDB)
	{
		pgpa(pgpaPGPKeySetValid(set));

		for (list = set->firstListInSet; list; list = list->nextListInSet)
		{
			pgpa((
				pgpaPGPKeyListValid(list),
				pgpaAssert(list->order > 0 && list->order < sNumCompareFuncs)));

			if (list->order != kPGPAnyOrdering)
			{
				compare = compareFunc[list->order];
				numMovedKeys = 0;
				for (i = 0; i < list->keyCount; i++)
					if (ringSetIsMember(changed, list->keys[i]->key))
					{
						if (numMovedKeys >= movedKeysAlloc)
						{
							void *vmovedKeys;

							movedKeysAlloc *= 2;
							vmovedKeys = movedKeys;
							result = pgpContextMemRealloc(
								context, &vmovedKeys,
								movedKeysAlloc * sizeof(PGPKey *),
								0 );
							movedKeys = (PGPKeyRef *)vmovedKeys;
								
							if ( IsPGPError( result ) )
								goto done;
						}
						key = movedKeys[numMovedKeys++] = list->keys[i];
						result = removeKeyFromList(list, key);
						if ( IsPGPError( result ) )
							goto done;
						i--;
					}
				for (i = 0; i < numMovedKeys; i++)
				{
					result = addKeyToList(list, movedKeys[i]);
					if (result)
						goto done;
				}
			}
		}
	}
done:
	if (movedKeys != NULL)
		pgpContextMemFree( context, movedKeys);
	return result;
}

/*
 * buildKeyPool can be used to either add keys or remove keys, but not both.
 * If you are removing keys, pass TRUE for <deleteFlag>, but if so there
 * better not be any new keys or else it'll do the wrong thing.  Likewise,
 * if you pass FALSE for <deleteFlag>, there better not be any keys missing.
 */
	PGPError
pgpBuildKeyPool(PGPKeyDB *db, PGPBoolean deleteFlag)
{
	RingIterator *	iter;
	RingObject *	obj;
	PGPKey *		key;
	PGPKey **		prevPtr;
	PGPError		result = kPGPError_NoErr;

	iter = ringIterCreate(pgpKeyDBRingSet(db));
	if (iter == NULL)
		return kPGPError_OutOfMemory;
	
	prevPtr = &db->firstKeyInDB;
	
	while (ringIterNextObject(iter, 1) > 0)
	{
		obj = ringIterCurrentObject(iter, 1);
		pgpAssertAddrValid(obj, VoidAlign);	/* XXX use better align check */

		key = *prevPtr;

		if (deleteFlag && key)
			while (key->key != obj)
			{
				pgpa(pgpaPGPKeyValid(key));

				removeKeyFromSets(db, key);

				*prevPtr = key->nextKeyInDB;
				pgpFreeKey(key);
				key = *prevPtr;

				pgpAssert(db->numKeys > 0);
				db->numKeys--;
			}

		if ((!key || key->key != obj) && !deleteFlag)
		{
			key = allocKey(db);
			if (key == NULL)
			{
				result = kPGPError_OutOfMemory;
				break;
			}

			key->refCount = 0;
			key->keyDB = db;
			key->key = obj;
			key->userVal = 0;
			key->subKeys.next = &key->subKeys;
			key->subKeys.prev = key->subKeys.next;
			key->userIDs.next = &key->userIDs;
			key->userIDs.prev = key->userIDs.next;
			key->nextKeyInDB = *prevPtr;
			pgpIncKeyRefCount (key);
			*prevPtr = key;

			db->numKeys++;

			addKeyToLists(db, key);
		}
		prevPtr = &key->nextKeyInDB;
	}

	/*  Reached end of RingSet.  If we're in delete mode, there
		may still be trailing PGPKey objects that need to be
		freed. */
	
	if (deleteFlag) {
	    key = *prevPtr;
		while (key != NULL) {
		    pgpa(pgpaPGPKeyValid(key));
			removeKeyFromSets(db, key);
			*prevPtr = key->nextKeyInDB;
			pgpFreeKey(key);
			key = *prevPtr;
			pgpAssert(db->numKeys > 0);
			db->numKeys--;
		}
	}

	*prevPtr = NULL;
	ringIterDestroy(iter);

	return result;
}

	void
pgpIncKeyDBRefCount(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	db->refCount++;
}

	void
pgpFreeKeyDB(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	db->refCount--;
	if (db->refCount <= 0)
	{
		if (db->keysByKeyID != NULL)
		{
			/*
			 * Move the refCount up by 2 while we destroy the keyList,
			 * so when pgpFreeKeyDB is called from PGPFreeKeySet from
			 * PGPFreeKeyList below, it does nothing but decrement the
			 * refCount.  Unfortunately this is more of a hack than I'd
			 * like.  refCounts don't work all that well with cycles.
			 */
			db->refCount += 2;
			PGPFreeKeyList(db->keysByKeyID);
			db->refCount--;
		}
		(*db->destroy)(db);
		pgpKeyDBDestroyInternal (db);
	}
}

	PGPBoolean
PGPKeySetIsMember(PGPKey *key, PGPKeySet *set)
{
	pgpa((
		pgpaPGPKeySetValid(set),
		pgpaPGPKeyValid(key)));
		
	if ( ! ( pgpKeySetIsValid( set ) && pgpKeyIsValid( key ) ) )
	{
		return( FALSE );
	}
	
	return set->isMember(set, key);
}

	PGPError
PGPUnionKeySets(PGPKeySetRef set1, PGPKeySetRef set2, PGPKeySetRef *newSet)
{
	PGPError	err	= kPGPError_NoErr;

	PGPValidatePtr( newSet );
	*newSet	= NULL;
	PGPValidateKeySet( set1 );
	PGPValidateKeySet( set2 );
	if (set1->keyDB != set2->keyDB)
		return kPGPError_KeyDBMismatch;

	if (set1->makeUnion == set2->makeUnion && IsntNull(set1->makeUnion))
		err = set1->makeUnion(set1, set2, newSet);
	else
		err = pgpGenericUnionOfKeySets(set1, set2, newSet);

	return err;
}

	static void
defaultRemoveKeyFromKeySet(
	PGPKeySetRef	set,
	PGPKeyRef		key)
{
	/* Do nothing */

	(void)set;		/* Avoid warnings */
	(void)key;
}

	PGPError
pgpNewKeySetInternal(
	PGPKeyDBRef		db,
	PGPKeySetRef *	newSet)
{
	PGPKeySet *		set;
	PGPContextRef	context	= pgpGetKeyDBContext(db);

	pgpa(pgpaPGPKeyDBValid(db));

	*newSet = NULL;		/* In case there's an error */

	set = (PGPKeySet *)pgpContextMemAlloc(context,
							sizeof(PGPKeySet), kPGPMemoryMgrFlags_Clear);
	if (IsNull(set))
		return kPGPError_OutOfMemory;

	pgpIncKeyDBRefCount(db);

	set->priv			= NULL;
	set->refCount		= 1;
	set->keyDB			= db;
	set->firstListInSet = NULL;
	set->magic			= kPGPKeySetMagic;

	set->prevSetInDB = NULL;
	set->nextSetInDB = db->firstSetInDB;
	if (set->nextSetInDB)
		set->nextSetInDB->prevSetInDB = set; 
	db->firstSetInDB = set;

	set->removeKey = defaultRemoveKeyFromKeySet;

	*newSet = set;
	return kPGPError_NoErr;
}

	static PGPBoolean
rootSetIsMember(PGPKeySet *set, PGPKeyRef key)
{
	pgpa((
		pgpaPGPKeySetValid(set),
		pgpaAddrValid(key, VoidAlign)));	/* XXX use better align check */

	return ringSetIsMember(pgpKeyDBRingSet(set->keyDB), key->key);
}

	static PGPError
rootSetMakeUnion(PGPKeySetRef set1, PGPKeySetRef set2, PGPKeySetRef *newSet)
{
	(void)set2;	/* Avoid warning */

	PGPIncKeySetRefCount(set1);
	*newSet = set1;
	return kPGPError_NoErr;
}

	static void
rootSetDestroy(PGPKeySet *set)
{
	(void)set;	/* Avoid warning */
}

	PGPKeySet *
pgpKeyDBRootSet(
	PGPKeyDB *		db)
{
	PGPKeySet *		set;

	pgpNewKeySetInternal(db, &set);
	if (IsntNull(set))
	{
		set->isMember = rootSetIsMember;
		set->makeUnion = rootSetMakeUnion;
		set->destroy = rootSetDestroy;
	}
	return set;
}

	RingSet const *
pgpKeyDBRingSet(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return (*db->getRingSet)(db);
}

/*
 * Return a ringset holding the keys (and subsidiary objects) in the
 * corresponding keyset.  This differs from pgpKeyDBRingSet in that it
 * only includes the keys in the keyset, not all those in the keydb.
 * The returned ringset must be deleted after being used.  If freezeit
 * is true, the ringset is returned frozen, else it is returned
 * mutable.
 */
	PGPError
pgpKeySetRingSet(PGPKeySet *keys, PGPBoolean freezeit, RingSet const **ringset)
{
	PGPKeyList			*list = NULL;
	PGPKeyIter			*iter = NULL;
	PGPKey				*key;
	RingSet const		*kdbSet;
	RingSet				*rset = NULL;
	PGPError			 err = kPGPError_NoErr;

	kdbSet = pgpKeyDBRingSet( keys->keyDB );
	rset = ringSetCreate( ringSetPool( kdbSet ) );
	if( IsNull( rset ) ) {
		err = ringSetError( kdbSet )->error;
		goto error;
	}

	if (keys->isMember == rootSetIsMember) {
		/* In simple case, can just make a copy of root set */
		ringSetAddSet( rset, kdbSet );
	} else {
		/* Else iterate over all keys in set */
		if( IsPGPError( err = PGPOrderKeySet( keys, kPGPAnyOrdering,
											  &list ) ) )
			goto error;
		if( IsPGPError( err = PGPNewKeyIter( list, &iter ) ) )
			goto error;
		while( IsntPGPError( PGPKeyIterNext( iter, &key ) ) ) {
			RingObject *keyobj = key->key;
			pgpAssert( IsntNull( keyobj ) );
			ringSetAddObjectChildren( rset, kdbSet, keyobj );
		}
		PGPFreeKeyIter( iter );
		PGPFreeKeyList( list );
	}
	if( freezeit )
		ringSetFreeze( rset );
	*ringset = (RingSet const *) rset;
	return kPGPError_NoErr;

error:
	if( IsntNull( iter ) )
		PGPFreeKeyIter( iter );
	if( IsntNull( list ) )
		PGPFreeKeyList( list );
	if( IsntNull( rset ) )
		ringSetDestroy( rset );
	return err;
}

	PGPBoolean
pgpKeyDBIsMutable(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return (*db->isMutable)(db);
}

	PGPBoolean
pgpKeyDBIsDirty(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return (*db->isDirty)(db);
}

/*
 * Call this when we have made a change to the keys in the changedkeys set,
 * to force those keys to be resorted in all lists depending on the db
 */
	PGPError
pgpKeyDBChanged(PGPKeyDB *db, RingSet *changedkeys)
{
	return db->changed(db, changedkeys);
}

	PGPError
pgpCommitKeyDB(PGPKeyDB *db)
{
	RingSet const *	rset;
	PGPError		error;
	int				count;

	pgpa(pgpaPGPKeyDBValid(db));

	if ( !pgpKeyDBIsMutable( db ) )
		return kPGPError_ItemIsReadOnly;

	/*
	 * Don't do the automatic sigcheck and trust propagation on in-memory
	 * db's.  This is not only unnecessary, but it clears the axiomatic bit
	 * on any keys which are in the memory set as public keys but not as
	 * secret keys.
	 */
	if( db->typeMagic != PGPKDBMEMMAGIC )
	{
		rset = pgpKeyDBRingSet (db);
		if (!rset)
			return kPGPError_OutOfMemory;
		/* XXX This can take a while, need a progress bar? */
		error = ringPoolCheck (rset, rset, FALSE, FALSE, 0, 0);
		if (error)
			return error;
		count = ringMnt (rset, 0, PGPGetTime());
		if (count < 0)
			return (PGPError) count;
	}
	error = db->commit(db);
	if (error)
		return error;
	pgpKeyDBReleaseDependencies(db);
	return kPGPError_NoErr;
}


	PGPError
pgpPropagateTrustKeyDB(PGPKeyDB *db)
{
	RingSet const   *rset;
	int			result;
	PGPError	err	= kPGPError_NoErr;

	pgpa(pgpaPGPKeyDBValid(db));

	rset = pgpKeyDBRingSet (db);
	result	= ringMnt (rset, 0, PGPGetTime());
	
	/* this is obviously extremely ugly rngMnt() should be fixed */
	if ( result < 0 )
		err	= (PGPError)result;
	else
		err	= kPGPError_NoErr;
		
	return( err );
}


/* Callback for checking function */
typedef struct PGPKeyCheckState {
	PGPContextRef			context;
	PGPEventHandlerProcPtr	progress;
	PGPUserValue			userValue;
	PGPUInt32				total;
	PGPUInt32				sofar;
} PGPKeyCheckState;

	static PGPError
checkKeyDBCallback(void *arg, RingIterator *iter, int status)
{
	PGPKeyCheckState		*s = (PGPKeyCheckState *)arg;
	PGPError				err = kPGPError_NoErr;
	PGPOptionListRef		newOptionList = NULL;

	(void) iter;
	(void) status;

	if (IsntNull (s->progress)) {
		err = pgpEventNull (s->context, &newOptionList,
							s->progress, s->userValue, ++s->sofar, s->total);
		if (IsntNull (newOptionList))
			pgpFreeOptionList (newOptionList);
	}
	return err;
}
	

	PGPError
pgpCheckKeyDB (
	PGPKeyDB				*dbToCheck,
	PGPKeyDB				*dbSigning,
	PGPBoolean				checkAll,
	PGPEventHandlerProcPtr	progress,
	PGPUserValue			userValue
	)
{
	RingSet const			*rsetToCheck,
							*rsetSigning;
	PGPKeyCheckState		 s;

	pgpa(pgpaPGPKeyDBValid(dbToCheck));
	pgpa(pgpaPGPKeyDBValid(dbSigning));
	
	rsetToCheck = pgpKeyDBRingSet (dbToCheck);
	rsetSigning = pgpKeyDBRingSet (dbSigning);
	if (!rsetToCheck || !rsetSigning)
		return kPGPError_OutOfMemory;
	pgpClearMemory (&s, sizeof(s));
	if( IsntNull( progress ) ) {
		s.context = dbToCheck->context;
		s.progress = progress;
		s.userValue = userValue;
		s.sofar = 0;
		s.total = ringPoolCheckCount(rsetToCheck, rsetSigning, checkAll,
									 FALSE);
	}
	return ringPoolCheck (rsetToCheck, rsetSigning, checkAll, FALSE,
						  checkKeyDBCallback, &s);
}


	PGPError
pgpRevertKeyDB(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return db->revert(db);
}

	PGPError
pgpReloadKeyDB(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return db->reload(db);
}

	PGPError
PGPCommitKeyRingChanges(PGPKeySet *keys)
{
	PGPValidateKeySet( keys );

	return pgpCommitKeyDB(keys->keyDB);
}


	PGPError
PGPPropagateTrust(PGPKeySet *keys)
{
	PGPValidateKeySet( keys );

	return pgpPropagateTrustKeyDB(keys->keyDB);
}

	PGPError
PGPRevertKeyRingChanges(PGPKeySet *keys)
{
	PGPValidateKeySet( keys );

	return pgpRevertKeyDB(keys->keyDB);
}

/* Check all sigs in keyset */
	PGPError
PGPCheckKeyRingSigs(
	PGPKeySetRef			keysToCheck,
	PGPKeySetRef			keysSigning,
	PGPBoolean				checkAll,
	PGPEventHandlerProcPtr	progress,
	PGPUserValue			userValue
	)
{
	RingSet const			*rsetToCheck = NULL,
							*rsetSigning = NULL;
	PGPKeyCheckState		 s;
	PGPError				 err = kPGPError_NoErr;

	PGPValidateKeySet( keysToCheck );
	PGPValidateKeySet( keysSigning );
	
	err = pgpKeySetRingSet (keysToCheck, TRUE, &rsetToCheck);
	if( IsPGPError( err ) )
		goto error;
	err = pgpKeySetRingSet (keysSigning, TRUE, &rsetSigning);
	if( IsPGPError( err ) )
		goto error;

	pgpClearMemory (&s, sizeof(s));
	if( IsntNull( progress ) ) {
		s.context = PGPGetKeySetContext( keysToCheck );
		s.progress = progress;
		s.userValue = userValue;
		s.sofar = 0;
		s.total = ringPoolCheckCount(rsetToCheck, rsetSigning, checkAll,
									 FALSE);
	}
	err = ringPoolCheck (rsetToCheck, rsetSigning, checkAll, FALSE,
						  checkKeyDBCallback, &s);

error:

	if( IsntNull( rsetToCheck ) )
		ringSetDestroy( (RingSet *)rsetToCheck );
	if( IsntNull( rsetSigning ) )
		ringSetDestroy( (RingSet *)rsetSigning );
	return err;
}

	PGPError
PGPReloadKeyRings(PGPKeySetRef keys)
{
	PGPValidateKeySet( keys );

	return pgpReloadKeyDB(keys->keyDB);
}

	PGPError
PGPIncKeySetRefCount(PGPKeySetRef keys)
{
	PGPValidateKeySet( keys );

	keys->refCount++;
	
	return( kPGPError_NoErr );
}

	PGPError
PGPNewKeySet(
	PGPContextRef	context,
	PGPKeySetRef   *newSetOut)
{
	PGPKeyDB	   *memdb;
	
	PGPValidatePtr( newSetOut );
	*newSetOut	= NULL;
	PGPValidateContext( context );
	
	memdb = pgpKeyDBCreate(context);
	if ( IsNull( memdb ) )
		return kPGPError_OutOfMemory;
		
	*newSetOut = pgpKeyDBRootSet (memdb);
	pgpFreeKeyDB( memdb );
	
	return kPGPError_NoErr;
}


/* Add all the objects in the second key set into the first. */
	PGPError
PGPAddKeys (
	PGPKeySetRef	keysToAdd,
	PGPKeySetRef	set )
{
	PGPKeyList	   *kladd = NULL;
	PGPKeyIter	   *kiadd = NULL;
	RingSet		   *tmpset = NULL;
	PGPError		err = kPGPError_NoErr;

	PGPValidateKeySet( set );
	PGPValidateKeySet( keysToAdd );

	if ( !PGPKeySetIsMutable( set ) )
	{
		err = kPGPError_ItemIsReadOnly;
		goto error;
	}

	err = pgpKeyDBAddDependency(set->keyDB, keysToAdd->keyDB);
	if ( IsPGPError( err ) )
		goto error;
	
	err = pgpKeySetRingSet (keysToAdd, FALSE, (RingSet const **)&tmpset);
	if( IsPGPError( err ) )
	{
		err = kPGPError_OutOfMemory;
		goto error;
	}
	err = pgpAddObjects (set->keyDB, tmpset);
	if ( IsPGPError( err ) )
		goto error;

error:
	if (tmpset)
		ringSetDestroy (tmpset);
	if (kiadd)
		PGPFreeKeyIter (kiadd);
	if (kladd)
		PGPFreeKeyList (kladd);
		
	return err;
}

/* Remove all objects in the second set from the first */
	PGPError
PGPRemoveKeys (
	PGPKeySetRef	keysToRemove,
	PGPKeySetRef	set )
{
	PGPKeyList	   *klrem = NULL;
	PGPKeyIter	   *kirem = NULL;
	PGPKey		   *key = NULL;
	RingObject	   *keyobj = NULL;
	PGPError		err = kPGPError_NoErr;

	PGPValidateKeySet( set );
	PGPValidateKeySet( keysToRemove );
	
	if ( !PGPKeySetIsMutable( set ) )
	{
		err = kPGPError_ItemIsReadOnly;
		goto error;
	}

	err	= PGPOrderKeySet (keysToRemove, kPGPAnyOrdering, &klrem );
	if ( IsPGPError( err ) )
		goto error;

	err = PGPNewKeyIter (klrem, &kirem );
	if ( IsPGPError( err ) ) 
		goto error;

	while ((err = PGPKeyIterNext( kirem, &key )) == kPGPError_NoErr )
	{
		keyobj = key->key;
		err = pgpRemoveObject (set->keyDB, keyobj);
		if ( IsPGPError( err ) )
			goto error;
	}
	pgpAssert( err == kPGPError_EndOfIteration );
	if ( err == kPGPError_EndOfIteration )
		err	= kPGPError_NoErr;

error:
	if (kirem)
		PGPFreeKeyIter (kirem);
	if (klrem)
		PGPFreeKeyList (klrem);
	return err;
}


	PGPError
PGPFreeKeySet(PGPKeySet *keys)
{
	PGPContextRef	context	= NULL;
	
	PGPValidateKeySet( keys );
	context	= PGPGetKeySetContext( keys );
	PGPValidateContext( context );
	

	keys->refCount--;
	if (keys->refCount <= 0)
	{
		(*keys->destroy)(keys);
		keys->magic	= ~keys->magic;	/* mark as invalid */
		
		if (keys->prevSetInDB)
			keys->prevSetInDB->nextSetInDB = keys->nextSetInDB;
		else
			keys->keyDB->firstSetInDB = keys->nextSetInDB;
		if (keys->nextSetInDB)
			keys->nextSetInDB->prevSetInDB = keys->prevSetInDB;

		pgpAssert(keys->firstListInSet == NULL);

		pgpFreeKeyDB (keys->keyDB);
		pgpContextMemFree( context, keys);
	}
	
	return( kPGPError_NoErr );
}

	PGPBoolean
PGPKeySetIsMutable(PGPKeySet *keys)
{
	pgpa(pgpaPGPKeySetValid(keys));
	
	if ( ! pgpKeySetIsValid( keys ) )
		return( FALSE );

	return pgpKeyDBIsMutable(keys->keyDB);
}

	PGPBoolean
PGPKeySetNeedsCommit(PGPKeySet *keys)
{
	pgpa(pgpaPGPKeySetValid(keys));
	if ( ! pgpKeySetIsValid( keys ) )
		return( FALSE );

	return pgpKeyDBIsDirty(keys->keyDB);
}

/*  Defines when a key should be expanded or collapsed.  If set to 1, 
	keys are expanded by pgpBuildKeyPool.  If set to 2, they are expanded
	when referenced by an iterator, or when the key refCount is explicitly 
	incremented by the app. */

	PGPError
pgpIncKeyRefCount(PGPKey *key)
{
	pgpa(pgpaAddrValid(key, PGPKey));
	
	if ( IsNull( key ) )
		return( kPGPError_BadParams );

	key->refCount++;
	if (key->refCount == 1) 
		return pgpExpandKey (key);
	return kPGPError_NoErr;
}

	PGPError
pgpFreeKey(PGPKey *key)
{
	PGPError	err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	PGPValidateKey( key );
	PGPValidateParam( key->refCount >= 1 );

	if (key->refCount == 1)
		err = pgpCollapseKey (key);
	key->refCount--;
	if (key->refCount <= 0)
		deallocKey(key->keyDB, key);
	return err;
}

	PGPError
PGPCountKeys(
	PGPKeySetRef	keys,
	PGPUInt32 *		numKeys )
{
	PGPKey *		key;
	PGPUInt32		count = 0;
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( numKeys );
	*numKeys	= 0;
	PGPValidateKeySet( keys );

	for (key = keys->keyDB->firstKeyInDB; key; key = key->nextKeyInDB)
	{
		pgpa(pgpaPGPKeyValid(key));
		if (PGPKeySetIsMember(key, keys))
			count++;
	}
	
	*numKeys = count;

	return( err );
}

	PGPError
PGPOrderKeySet(
	PGPKeySet *		keys,
	PGPKeyOrdering	order,
	PGPKeyListRef *	outRef )
{
	PGPKeyList *	list;
	PGPKey *		key;
	PGPUInt32		count;
	PGPUInt32		i;
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	context	= PGPGetKeySetContext( keys );
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeySet( keys );


/*	list = pgpNew(PGPKeyList); */
	list = (PGPKeyList *)pgpContextMemAlloc( context,
		sizeof(PGPKeyList), kPGPMemoryMgrFlags_Clear );
	if (list == NULL)
		return kPGPError_OutOfMemory;

	list->magic	= kPGPKeyListMagic;
	
	(void)PGPCountKeys(keys, &count );
	list->keyCount = count;
	list->keys = (PGPKey **)pgpContextMemAlloc( context,
		count * sizeof(PGPKey *), kPGPMemoryMgrFlags_Clear);
	if (list->keys == NULL)
	{
		pgpContextMemFree( context, list);
		return kPGPError_OutOfMemory;
	}
	
	list->refCount = 1;
	list->keySet = keys;
	list->order = order;
	list->prevListInSet = NULL;
	list->nextListInSet = keys->firstListInSet;

	if( IsntNull( list->nextListInSet ) )
		list->nextListInSet->prevListInSet = list;
		
	keys->firstListInSet = list;
	list->firstIterInList = NULL;

	PGPIncKeySetRefCount(keys);

	i = 0;
	for (key = keys->keyDB->firstKeyInDB; key; key = key->nextKeyInDB)
	{
		pgpa(pgpaPGPKeyValid(key));
		if (PGPKeySetIsMember( key, keys ))
		{
			pgpAssert(i < count);
			list->keys[i++] = key;
		}
	}
	pgpAssert(i == count);

	sortKeyList(list);

	*outRef	= list;
	return( err );
}

	PGPError
PGPIncKeyListRefCount(PGPKeyList *list)
{
	PGPValidateKeyList( list );

	list->refCount++;
	
	return( kPGPError_NoErr );
}

	PGPError
PGPFreeKeyList(PGPKeyList *list)
{
	PGPContextRef	context	= NULL;
	
	PGPValidateKeyList( list );
	
	context	= PGPGetKeyListContext( list );

	list->refCount--;
	if (list->refCount <= 0)
	{
		list->magic	= ~ list->magic;	/* mark as invalid */
		
		if (list->prevListInSet)
			list->prevListInSet->nextListInSet = list->nextListInSet;
		else
			list->keySet->firstListInSet = list->nextListInSet;
		if (list->nextListInSet)
			list->nextListInSet->prevListInSet = list->prevListInSet;

		pgpAssert(list->firstIterInList == NULL);

		pgpContextMemFree( context, list->keys);
		PGPFreeKeySet(list->keySet);
		pgpContextMemFree( context, list);
	}
	return( kPGPError_NoErr );
}


	static PGPError 
pgpGetKeyByKeyID(
	PGPKeySetRef	keys,
	PGPKeyID const *keyIDIn,
	PGPPublicKeyAlgorithm	pubKeyAlgorithm,
	PGPKeyRef *		outRef )
{
	PGPKeyDB *		db;
	RingSet const *	ringSet;
	long			lo;
	long			hi;
	long			i;
	PGPKey **		keyArray;
	PGPKeyID		keyIDABytes;
	PGPKeyID		keyIDBBytes;
	PGPKeyID		keyIDMasterBytes;
	PGPKeyID		keyIDBytes;
	int				comparison;
	
	PGPError	err	= kPGPError_ItemNotFound;
	PGPKeyRef	resultKey	= NULL;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeySet( keys );
	PGPValidatePtr( keyIDIn );

	PGPValidateParam( pubKeyAlgorithm == kPGPPublicKeyAlgorithm_Invalid ||
		(pubKeyAlgorithm >= kPGPPublicKeyAlgorithm_First &&
			pubKeyAlgorithm <= kPGPPublicKeyAlgorithm_Last ) );
	
	keyIDBytes	= *keyIDIn;
	
	db = keys->keyDB;
	ringSet	= pgpKeyDBRingSet(db);

	/* We may be called to match a subkey; if so replace keyid with master's */
	if( pubKeyAlgorithm != kPGPPublicKeyAlgorithm_Invalid )
	{
		RingObject const	*ringObj;

		if( ((PGPKeyIDPriv *) keyIDIn)->length == 4 )
		{
			ringObj = ringKeyById4 (ringSet, (PGPByte)pubKeyAlgorithm,
								pgpGetKeyBytes( &keyIDBytes ) );
		}
		else
		{
			ringObj = ringKeyById8 (ringSet, (PGPByte)pubKeyAlgorithm,
								pgpGetKeyBytes( &keyIDBytes ) );
		}
		
		if( IsNull( ringObj ) )
			return( kPGPError_ItemNotFound );
			
		if( ringKeyIsSubkey( ringSet, ringObj ) )
		{
			/* It's a subkey, replace it with the master key */
			ringObj = ringKeyMasterkey( ringSet, ringObj );
			ringKeyID8( ringSet, ringObj, NULL, &keyIDMasterBytes );
			keyIDBytes	= keyIDMasterBytes;
		}
	}

	if (db->keysByKeyID == NULL)
	{
		PGPKeySet *		rootSet;

		if ((rootSet = pgpKeyDBRootSet(db)) != NULL)
		{
			err = PGPOrderKeySet(rootSet, kPGPKeyIDOrdering, &db->keysByKeyID );
			if ( IsPGPError( err ) )
				return( err );
				
			/* rootSet will stick around until the keylist is freed */
			PGPFreeKeySet(rootSet);

			/*
			 * Undo the additional refCount created by the existence of
			 * rootSet.  Otherwise the keyDB will never be freed because
			 * of the cycle in the reference graph.  When the keyDB is
			 * actually freed, if keysByKeyID exists, refCount will be
			 * incremented again before freeing keysByKeyID.
			 */
			if (db->keysByKeyID != NULL)
				db->refCount--;
		}
		if (db->keysByKeyID == NULL)
			return kPGPError_OutOfMemory;
	}

	pgpa((
		pgpaPGPKeyListValid(db->keysByKeyID),
		pgpaAssert(db->keysByKeyID->order == kPGPKeyIDOrdering)));

	keyArray = db->keysByKeyID->keys;
	keyIDABytes	= keyIDBytes;

	if( ((PGPKeyIDPriv *) keyIDIn)->length == 4 )
	{
		/*
		** Search the list linearly in the 4-byte case because the list
		** is sorted by the full 8-byute ID
		*/
		
		for( i = 0; i < db->keysByKeyID->keyCount; i++ )
		{
			PGPKeyRef	key;
			
			key = keyArray[ i ];
			ringKeyID4( ringSet, key->key, NULL, &keyIDBBytes );

			comparison = compareKeyIDs( &keyIDABytes, &keyIDBBytes);
			if( comparison == 0 && PGPKeySetIsMember( key, keys) )
			{
				if( PGPKeyRefIsValid( resultKey ) )
				{
					/* Only one match allowed. Fail */
					resultKey = kInvalidPGPKeyRef;
					break;
				}
				else
				{
					resultKey = key;
				}
			}
		}
	}
	else
	{
		lo = 0;
		hi = db->keysByKeyID->keyCount;

		while (lo < hi)
		{
			PGPKeyRef	key;
			
			i = (lo + hi) / 2;
			
			key		= keyArray[ i ];
			ringKeyID8( ringSet, key->key, NULL, &keyIDBBytes );
			
			comparison = compareKeyIDs( &keyIDABytes, &keyIDBBytes);
			if (comparison > 0)
				lo = i + 1;
			else if (comparison < 0)
				hi = i;
			else if (PGPKeySetIsMember( keyArray[i], keys))
			{
				resultKey	= keyArray[i];
				break;
			}
			else
				break;
		}
	}
	
	if ( IsNull( resultKey ) )
	{
		err	= kPGPError_ItemNotFound;
	}
	else
	{
		err = kPGPError_NoErr;
	}
	
	*outRef	= resultKey;
	
	pgpAssert( ( IsntPGPError( err ) && IsntNull( *outRef ) ) ||
		( IsPGPError( err ) && IsNull( *outRef ) ));
	return( err );
}

	PGPError 
PGPGetKeyByKeyID(
	PGPKeySetRef			keys,
	PGPKeyID const *		keyIDIn,
	PGPPublicKeyAlgorithm	pubKeyAlgorithm,
	PGPKeyRef *				outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeySet( keys );
	PGPValidatePtr( keyIDIn );
	
	err	= pgpGetKeyByKeyID( keys, keyIDIn, pubKeyAlgorithm, outRef );
	if ( IsntPGPError( err ) )
	{
		pgpIncKeyRefCount( *outRef );
	}
	
	pgpAssertErrWithPtr( err, *outRef );
	return( err );
}


	PGPBoolean
pgpKeyDBIsValid( PGPKeyDB const *	keyDB)
{
	return( IsntNull( keyDB ) && keyDB->fixedMagic == kPGPKeyDBMagic );
}

	PGPBoolean
pgpKeySetIsValid( PGPKeySet const *	keySet)
{
	return( IsntNull( keySet ) && keySet->magic == kPGPKeySetMagic );
}

	PGPBoolean
pgpKeyListIsValid( PGPKeyList const *	keyList)
{
	return( IsntNull( keyList ) && keyList->magic == kPGPKeyListMagic );
}

	PGPBoolean
pgpKeyIterIsValid( PGPKeyIter const *	keyIter)
{
	return( IsntNull( keyIter ) &&
		keyIter->magic == kPGPKeyIterMagic );
}

	PGPBoolean
pgpKeyIsValid( PGPKey const *		key)
{
	PGPBoolean	isValid;
	
	isValid	= IsntNull( key ) &&
			pgpOptionalMagicMatches( key, kPGPKeyMagic ) &&
			key->refCount > 0 && 
			pgpKeyDBIsValid( key->keyDB );
			
	return( isValid );
}

	PGPBoolean
pgpSubKeyIsValid(
	PGPSubKey const *	subKey)
{
	PGPBoolean	isValid;
	
	isValid	= IsntNull( subKey ) &&
			pgpOptionalMagicMatches( subKey, kPGPSubKeyMagic ) &&
			pgpKeyIsValid( subKey->key );
			
	return( isValid );
}

	PGPBoolean
pgpUserIDIsValid(
	PGPUserID const *	userID)
{
	PGPBoolean	isValid;
	
	isValid	= IsntNull( userID ) &&
			pgpOptionalMagicMatches( userID, kPGPUserIDMagic ) &&
			pgpKeyIsValid( userID->key );
			
	return( isValid );
}

	PGPBoolean
pgpSigIsValid(
	PGPSig const *		cert)
{
	return( IsntNull( cert ) &&
		pgpOptionalMagicMatches( cert, kPGPCertMagic ) );
}



#if PGP_DEBUG	/* [ */

	PGPBoolean
pgpaInternalPGPKeyDBValid(
	pgpaCallPrefixDef,
	PGPKeyDB const *	keyDB,
	char const *		varName)
{
	pgpaAddrValid(keyDB, PGPKeyDB);
	pgpaFailIf(keyDB->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPKeyDBValid failed on %s (%p)", varName, keyDB));

	return pgpaFailed;
}

	PGPBoolean
pgpaInternalPGPKeySetValid(
	pgpaCallPrefixDef,
	PGPKeySet const *	keySet,
	char const *		varName)
{
	pgpaAddrValid(keySet, PGPKeySet);
	pgpaFailIf(keySet->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPKeySetValid failed on %s (%p)", varName, keySet));

	return pgpaFailed;
}

	PGPBoolean
pgpaInternalPGPKeyListValid(
	pgpaCallPrefixDef,
	PGPKeyList const *	keyList,
	char const *		varName)
{
	pgpaAddrValid(keyList, PGPKeyList);
	pgpaFailIf(keyList->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPKeyListValid failed on %s (%p)", varName, keyList));

	return pgpaFailed;
}

	PGPBoolean
pgpaInternalPGPKeyIterValid(
	pgpaCallPrefixDef,
	PGPKeyIter const *	keyIter,
	char const *		varName)
{
	pgpaAddrValid(keyIter, PGPKeyIter);
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPKeyIterValid failed on %s (%p)", varName, keyIter));

	return pgpaFailed;
}

	PGPBoolean
pgpaInternalPGPKeyValid(
	pgpaCallPrefixDef,
	PGPKey const *		key,
	char const *		varName)
{
	pgpaAddrValid(key, PGPKey);
	pgpaFailIf(key->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPKeyValid failed on %s (%p)", varName, key));

	return pgpaFailed;
}

	PGPBoolean
pgpaInternalPGPSubKeyValid(
	pgpaCallPrefixDef,
	PGPSubKey const *	subKey,
	char const *		varName)
{
	pgpaAddrValid(subKey, PGPSubKey);
/*	pgpaFailIf(subKey->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));	*/
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPSubKeyValid failed on %s (%p)", varName, subKey));

	return pgpaFailed;
}

	PGPBoolean
pgpaInternalPGPUserIDValid(
	pgpaCallPrefixDef,
	PGPUserID const *	userID,
	char const *		varName)
{
	pgpaAddrValid(userID, PGPUserID);
/*	pgpaFailIf(userID->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));	*/
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPUserIDValid failed on %s (%p)", varName, userID));

	return pgpaFailed;
}

	PGPBoolean
pgpaInternalPGPCertValid(
	pgpaCallPrefixDef,
	PGPSig const *		cert,
	char const *		varName)
{
	pgpaAddrValid(cert, PGPSig);
/*	pgpaFailIf(cert->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0")); 	*/
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPCertValid failed on %s (%p)", varName, cert));

	return pgpaFailed;
}

#endif /* ] PGP_DEBUG */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */


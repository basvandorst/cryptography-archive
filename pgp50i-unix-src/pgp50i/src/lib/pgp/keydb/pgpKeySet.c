/*
 * pgpKeySet.c -- PGPKeySet implementation
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpKeySet.c,v 1.46.2.2.2.1 1997/07/15 21:26:27 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <string.h>
#include <ctype.h>

#include "pgpKDBint.h"
#include "pgpTypes.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpTimeDate.h"
#include "pgpUsuals.h"
#include "pgpMemPool.h"
#include "pgpRngMnt.h"
#include "pgpRngPub.h"

static int
compareKeyIDs(uchar const *keyIDA, uchar const *keyIDB)
{
	int				i;

	for (i = 4; i < 8; i++)
	{
		if (keyIDA[i] > keyIDB[i])
			return 1;
		else if (keyIDA[i] < keyIDB[i])
			return -1;
	}
	for (i = 0; i < 4; i++)
	{
		if (keyIDA[i] > keyIDB[i])
			return 1;
		else if (keyIDA[i] < keyIDB[i])
			return -1;
	}
	return 0;
}

static int
keyCompareByKeyID(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;
	
	uchar	 		 keyIDA[8];
	uchar	 		 keyIDB[8];
	
	size_t			len;
	PGPError		result;

	len = sizeof(keyIDA);
	result = pgpGetKeyString(keyA, kPGPKeyPropKeyId, (char *)keyIDA, &len);
	pgpAssert(result == PGPERR_OK);

	len = sizeof(keyIDB);
	result = pgpGetKeyString(keyB, kPGPKeyPropKeyId, (char *)keyIDB, &len);
	pgpAssert(result == PGPERR_OK);

	return compareKeyIDs(keyIDA, keyIDB);
}

static int
keyCompareByReverseKeyID(void const *a, void const *b)
{
	return -keyCompareByKeyID(a, b);
}

int
pgpUserIDStrCompare(char const *a, char const *b)
{
	for (;;)
	{
		while (*a && tolower(*a) == tolower(*b))
			a++, b++;
		while (*a && !isalnum(*a))
			a++;
		while (*b && !isalnum(*b))
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
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;

	char			nameA[256];	/* XXX What's the maximum userID size? */
	char			nameB[256];
	size_t			len;
	int				compareResult;
	PGPError		result;

	len = sizeof(nameA) - 1;
	result = pgpGetPrimaryUserIDName(keyA, nameA, &len);
	pgpAssert(result == PGPERR_OK);
	nameA[len] = '\0';

	len = sizeof(nameB) - 1;
	result = pgpGetPrimaryUserIDName(keyB, nameB, &len);
	pgpAssert(result == PGPERR_OK);
	nameB[len] = '\0';

	compareResult = pgpUserIDStrCompare(nameA, nameB);
	return (compareResult!=0) ? compareResult :
			keyCompareByKeyID(a, b);
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

	long			validityA;
	long			validityB;
	PGPError		result;
	
	result = pgpGetPrimaryUserIDValidity(keyA, &validityA);
	pgpAssert(result == PGPERR_OK);
	result = pgpGetPrimaryUserIDValidity(keyB, &validityB);
	pgpAssert(result == PGPERR_OK);

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

	long			trustA;
	long			trustB;
	PGPError		result;
	
	result = pgpGetKeyNumber(keyA, kPGPKeyPropTrust, &trustA);
	pgpAssert(result == PGPERR_OK);
	result = pgpGetKeyNumber(keyB, kPGPKeyPropTrust, &trustB);
	pgpAssert(result == PGPERR_OK);

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
keyCompareByKeySize(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;

	long			keySizeA;
	long			keySizeB;
	PGPError		result;
	
	result = pgpGetKeyNumber(keyA, kPGPKeyPropBits, &keySizeA);
	pgpAssert(result == PGPERR_OK);
	result = pgpGetKeyNumber(keyB, kPGPKeyPropBits, &keySizeB);
	pgpAssert(result == PGPERR_OK);

	if (keySizeA < keySizeB)
		return 1;
	else if (keySizeA > keySizeB)
		return -1;
	else
		return keyCompareByKeyID(a, b);
}

static int
keyCompareByReverseKeySize(void const *a, void const *b)
{
	return -keyCompareByKeySize(a, b);
}

static int
keyCompareByCreation(void const *a, void const *b)
{
	PGPKey *		keyA = *(PGPKey **)a;
	PGPKey *		keyB = *(PGPKey **)b;

	PGPTime			creationA;
	PGPTime			creationB;
	PGPError		result;
	
	result = pgpGetKeyTime(keyA, kPGPKeyPropCreation, &creationA);
	pgpAssert(result == PGPERR_OK);
	result = pgpGetKeyTime(keyB, kPGPKeyPropCreation, &creationB);
	pgpAssert(result == PGPERR_OK);

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

/*
* The compare functions must all return non-ambiguous answers (>0,<0)
* because the add-key functionality uses a binary search to install
* new keys. If things are ambiguous then the order can change if some
* keys are tied under the main search. This is accomplished by doing
* a secondary search on keyid if there is a tie on the main search
* field.
*/

static int (*compareFunc[])(void const *, void const *) = {
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
	keyCompareByKeySize,
	keyCompareByReverseKeySize,
	keyCompareByCreation,
	keyCompareByReverseCreation };
static uint numCompareFuncs = sizeof(compareFunc) / sizeof(compareFunc[0]);

int
pgpCompareKeys(PGPKey *a, PGPKey *b, PGPKeyOrdering order)
{
	pgpa((
		pgpaPGPKeyValid(a),
		pgpaPGPKeyValid(b),
		pgpaAssert(order > 0 && order < numCompareFuncs
					&& order != kPGPAnyOrdering)));

	return (*compareFunc[order])(&a, &b);
}

static void
sortKeyList(PGPKeyList *list)
{
	pgpa((
		pgpaPGPKeyListValid(list),
		pgpaAssert(list->order > 0 && list->order < numCompareFuncs)));

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
		pgpaAssert(list->order > 0 && list->order < numCompareFuncs)));

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


/* Creates a new empty key database */
PGPKeyDB *
pgpKeyDBCreateInternal(void)
{
	PGPKeyDB *	db;

	/* VC++ doesn't like the following macro */
/*	db = pgpNew(PGPKeyDB); */
	db = (PGPKeyDB *)pgpAlloc(sizeof(PGPKeyDB));
	if (db == NULL)
		return NULL;
	
	memset(db, 0, sizeof(*db));

	db->private = NULL;
	db->refCount = 1;
	db->firstSetInDB = NULL;

	memPoolInit(&db->keyPool);
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
	memPoolEmpty (&db->keyPool);
	memset (db, 0, sizeof(*db));
	pgpMemFree (db);
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
		key = memPoolNew(&db->keyPool, PGPKey);
	
	pgpClearMemory(key, sizeof(*key));

	return key;
}

static void
deallocKey(PGPKeyDB *db, PGPKey *key)
{
	pgpa((
		pgpaPGPKeyDBValid(db),
		pgpaAssert(db == key->keyDB)));

	key->nextKeyInDB = db->firstFreeKey;
	db->firstFreeKey = key;
}

static PGPError
addKeyToList(PGPKeyList *list, PGPKey *key)
{
	long			i;
	long			newKeyCount;
	PGPKey **	 	newKeys;
	
	pgpa(pgpaPGPKeyListValid(list));

	i = binarySearchKeyList(list, key);

	newKeyCount = list->keyCount + 1;
	newKeys = pgpAlloc(newKeyCount * sizeof(PGPKey *));
	if (!newKeys)
		return PGPERR_NOMEM;

	pgpCopyMemory(list->keys, newKeys, i * sizeof(PGPKey *));
	pgpCopyMemory(list->keys + i, newKeys + i + 1,
					(list->keyCount - i) * sizeof(PGPKey *));
	pgpFree(list->keys);

	list->keys = newKeys;
	list->keyCount = newKeyCount;

	newKeys[i] = key;
	pgpKeyIterAddKey(list, i);
	
	return PGPERR_OK;
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

		if (pgpKeySetMember(set, key))
			for (list = set->firstListInSet; list; list = list->nextListInSet)
				if ((result = addKeyToList(list, key)) != PGPERR_OK)
					return result;
	}
	return PGPERR_OK;
}

static PGPError
removeKeyFromList(PGPKeyList *list, PGPKey *key)
{
	long			i;

	pgpa(pgpaPGPKeyListValid(list));

	for (i = 0; i < list->keyCount; i++)
		if (list->keys[i] == key)
			break;
	
	if (i < list->keyCount)
	{
		pgpCopyMemory(list->keys + i + 1, list->keys + i,
						(list->keyCount - i - 1) * sizeof(PGPKey *));

		list->keyCount--;
		pgpRealloc((void **)&list->keys, list->keyCount * sizeof(PGPKey *));

		pgpKeyIterRemoveKey(list, i);
	}
	
	return PGPERR_OK;
}

static PGPError
removeKeyFromLists(PGPKeyDB *db, PGPKey *key)
{
	PGPKeySet *		set;
	PGPKeyList *	list;
	PGPError		result;

	pgpa(pgpaPGPKeyDBValid(db));

	for (set = db->firstSetInDB; set; set = set->nextSetInDB)
{
		pgpa(pgpaPGPKeySetValid(set));

		for (list = set->firstListInSet; list; list = list->nextListInSet)
			if ((result = removeKeyFromList(list, key)) != PGPERR_OK)
				return result;
	}
	return PGPERR_OK;
}

PGPError
pgpReSortKeys(PGPKeyDB *db, RingSet *changed)
{
	PGPKeySet *		set;
	PGPKeyList *	list;
	PGPKey *		key;
	PGPKey **	 	movedKeys = NULL;
	long			numMovedKeys;
	long			movedKeysAlloc;
	long			i;
	int				(*compare)(void const *, void const *);
	PGPError		result = PGPERR_OK;

	pgpa(pgpaPGPKeyDBValid(db));

	movedKeysAlloc = 8;
	movedKeys = pgpAlloc(movedKeysAlloc * sizeof(PGPKey *));
	if (!movedKeys)
	{
		result = PGPERR_NOMEM;
		goto done;
	}
	for (set = db->firstSetInDB; set; set = set->nextSetInDB)
	{
		pgpa(pgpaPGPKeySetValid(set));

		for (list = set->firstListInSet; list; list = list->nextListInSet)
		{
			pgpa((
				pgpaPGPKeyListValid(list),
				pgpaAssert(list->order > 0 && list->order < numCompareFuncs)));

			if (list->order != kPGPAnyOrdering)
			{
				compare = compareFunc[list->order];
				numMovedKeys = 0;
				for (i = 0; i < list->keyCount; i++)
					if (ringSetIsMember(changed, list->keys[i]->key))
					{
						if (numMovedKeys >= movedKeysAlloc)
						{
							movedKeysAlloc *= 2;
							result = pgpRealloc((void **)&movedKeys,
													movedKeysAlloc
														* sizeof(PGPKey *));
							if (result)
								goto done;
						}
						key = movedKeys[numMovedKeys++] = list->keys[i];
						result = removeKeyFromList(list, key);
						if (result)
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
		pgpFree(movedKeys);
	return result;
}

/*
* buildKeyPool can be used to either add keys or remove keys, but not both.
* If you are removing keys, pass TRUE for <deleteFlag>, but if so there
* better not be any new keys or else it'll do the wrong thing. Likewise,
* if you pass FALSE for <deleteFlag>, there better not be any keys missing.
*/
PGPError
pgpBuildKeyPool(PGPKeyDB *db, Boolean deleteFlag)
{
	RingIterator *	iter;
	RingObject *	obj;
	PGPKey *		key;
	PGPKey **	 	prevPtr;
	PGPError		result = PGPERR_OK;

	iter = ringIterCreate(pgpKeyDBRingSet(db));
	if (iter == NULL)
		return PGPERR_NOMEM;
	
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

				removeKeyFromLists(db, key);

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
				result = PGPERR_NOMEM;
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

	/* Reached end of RingSet. If we're in delete mode, there
		may still be trailing PGPKey objects that need to be
		freed. */
	
	if (deleteFlag) {
	key = *prevPtr;
		while (key != NULL) {
		pgpa(pgpaPGPKeyValid(key));
			removeKeyFromLists(db, key);
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
			* so when pgpFreeKeyDB is called from pgpFreeKeySet from
			* pgpFreeKeyList below, it does nothing but decrement the
			* refCount. Unfortunately this is more of a hack than I'd
			* like. refCounts don't work all that well with cycles.
				*/
			db->refCount += 2;
			pgpFreeKeyList(db->keysByKeyID);
			db->refCount--;
		}
		(*db->destroy)(db);
		pgpKeyDBDestroyInternal (db);
	}
}

static Boolean
rootSetIsMember(PGPKeySet *set, RingObject const *obj)
{
	pgpa((
		pgpaPGPKeySetValid(set),
		pgpaAddrValid(obj, VoidAlign)));	/* XXX use better align check */

	return ringSetIsMember(pgpKeyDBRingSet(set->keyDB), obj);
}

static void
rootSetDestroy(PGPKeySet *set)
{
	(void)set;	/* Avoid warning */
}

Boolean
pgpKeySetMember(PGPKeySet *set, PGPKey *key)
{
	pgpa((
		pgpaPGPKeySetValid(set),
		pgpaPGPKeyValid(key)));
	
	return (*set->isMember)(set, key->key);
}

PGPKeySet *
pgpKeyDBRootSet(PGPKeyDB *db)
{
	PGPKeySet *		set;

	pgpa(pgpaPGPKeyDBValid(db));

/*	set = pgpNew(PGPKeySet); */
	set = (PGPKeySet *)pgpAlloc(sizeof(PGPKeySet));
	if (set != NULL)
	{
		pgpIncKeyDBRefCount(db);

		set->private = NULL;
		set->refCount = 1;
		set->keyDB = db;

		set->prevSetInDB = NULL;
		set->nextSetInDB = db->firstSetInDB;
		if (set->nextSetInDB)
		set->nextSetInDB->prevSetInDB = set;
		db->firstSetInDB = set;

		set->firstListInSet = NULL;

		set->isMember = rootSetIsMember;
		set->destroy = rootSetDestroy;
	}
	return set;
}

/*
* This shortcut version will not work once we implement filtering, but
* for now all keysets are the same, differing only in their keydb's.
*/
PGPKeySet *
pgpCopyKeySet (PGPKeySet *set)
{
	PGPKeyDB	*db;
	PGPKeySet	 *newset;

	db = set->keyDB;
	newset = pgpKeyDBRootSet (db);
	if (!newset)
		return NULL;
	newset->isMember = set->isMember;
	newset->destroy = set->destroy;
	return newset;
}

RingSet *
pgpKeyDBRingSet(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return (*db->getRingSet)(db);
}

Boolean
pgpKeyDBIsMutable(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return (*db->isMutable)(db);
}

Boolean
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
	return (*db->changed)(db, changedkeys);
}

PGPError
pgpCommitKeyDB(PGPKeyDB *db)
{
	RingSet		*rset;
	PGPError		error;
	int				count;

	pgpa(pgpaPGPKeyDBValid(db));

	rset = pgpKeyDBRingSet (db);
	if (!rset)
		return PGPERR_NOMEM;
	/* XXX This can take a while, need a progress bar? */
	error = ringPoolCheck (rset, rset, 0, 0, 0);
	if (error)
		return error;
	count = ringMnt (rset, 0, pgpGetTime());
	if (count < 0)
		return (PGPError) count;
	return (*db->commit)(db);
}

PGPError
pgpCheckKeyDB (PGPKeyDB *db)
{
	RingSet *rset;

	pgpa(pgpaPGPKeyDBValid(db));
	
	rset = pgpKeyDBRingSet (db);
	if (!rset)
		return PGPERR_NOMEM;
	/* XXX This can take a while, need a progress bar? */
	return ringPoolCheck (rset, rset, 1, 0, 0);
}


PGPError
pgpRevertKeyDB(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return (*db->revert)(db);
}

PGPError
pgpReloadKeyDB(PGPKeyDB *db)
{
	pgpa(pgpaPGPKeyDBValid(db));

	return (*db->reload)(db);
}

PGPError
pgpCommitKeyRingChanges(PGPKeySet *keys)
{
	pgpa(pgpaPGPKeySetValid(keys));

	return pgpCommitKeyDB(keys->keyDB);
}

PGPError
pgpRevertKeyRingChanges(PGPKeySet *keys)
{
	pgpa(pgpaPGPKeySetValid(keys));

	return pgpRevertKeyDB(keys->keyDB);
}

/* Check all sigs in keyset */
PGPError
pgpCheckKeyRingCertificates (PGPKeySet *keys)
{
	return pgpCheckKeyDB (keys->keyDB);
}

PGPError
pgpReloadKeyRings(PGPKeySet *keys)
{
	pgpa(pgpaPGPKeySetValid(keys));

	return pgpReloadKeyDB(keys->keyDB);
}

void
pgpIncKeySetRefCount(PGPKeySet *keys)
{
	pgpa(pgpaPGPKeySetValid(keys));

	keys->refCount++;
}

PGPKeySet *
pgpNewKeySet ()
{
	PGPKeyDB		*memdb;
	PGPKeySet		*set;

	memdb = pgpKeyDBCreate();
	if (!memdb)
		return NULL;
	set = pgpKeyDBRootSet (memdb);
	pgpFreeKeyDB(memdb);
	return set;
}

PGPKeySet *
pgpNewSingletonKeySet (PGPKey *key)
{
	PGPKeyDB		*memdb;
	PGPKeySet		*set = NULL;
	RingSet			*rset;
	RingSet			*tmpset = NULL;
	PGPError		error;

	memdb = pgpKeyDBCreate();
	if (!memdb)
		return NULL;
	rset = pgpKeyDBRingSet (key->keyDB);
	if (!rset)
		goto done;
	error = pgpCopyKey (rset, key->key, &tmpset);
	if (error)
		goto done;
	error = pgpAddObjects (memdb, tmpset);
	if (error)
		goto done;
	set = pgpKeyDBRootSet (memdb);
done:
	if (tmpset)
		ringSetDestroy (tmpset);
	pgpFreeKeyDB(memdb);
	return set;
}



/* Add all the objects in the second key set into the first. */
PGPError
pgpAddKeys (PGPKeySet *set, PGPKeySet *keysToAdd)
{
	PGPKeyList		*kladd = NULL;
	PGPKeyIter		*kiadd = NULL;
	RingSet			*tmpset = NULL;
	PGPKey			*key;
	RingSet			*srcset;
	PGPError		error = PGPERR_OK;

	kladd = pgpOrderKeySet (keysToAdd, kPGPAnyOrdering);
	if (!kladd) {
		error = PGPERR_NOMEM;
		goto err;
	}
	kiadd = pgpNewKeyIter (kladd);
	if (!kiadd) {
		error = PGPERR_NOMEM;
		goto err;
	}

	while ((key = pgpKeyIterNext (kiadd)) != 0) {
		srcset = pgpKeyDBRingSet (key->keyDB);
		if (!srcset) {
			error = PGPERR_NOMEM;
			goto err;
		}
		error = pgpCopyKey (srcset, key->key, &tmpset);
		if (error)
			goto err;
		error = pgpAddObjects (set->keyDB, tmpset);
		if (error)
			goto err;
		ringSetDestroy (tmpset);
		tmpset = NULL;
	}

err:
	if (tmpset)
		ringSetDestroy (tmpset);
	if (kiadd)
		pgpFreeKeyIter (kiadd);
	if (kladd)
		pgpFreeKeyList (kladd);
	return error;
}

/* Remove all objects in the second set from the first */
PGPError
pgpRemoveKeys (PGPKeySet *set, PGPKeySet *keysToRemove)
{
	PGPKeyList		*klrem = NULL;
	PGPKeyIter		*kirem = NULL;
	PGPKey			*key = NULL;
	RingObject		*keyobj = NULL;
	PGPError		error = PGPERR_OK;

	klrem = pgpOrderKeySet (keysToRemove, kPGPAnyOrdering);
	if (!klrem) {
		error = PGPERR_NOMEM;
		goto err;
	}
	kirem = pgpNewKeyIter (klrem);
	if (!kirem) {
		error = PGPERR_NOMEM;
		goto err;
	}
	while ((key = pgpKeyIterNext (kirem)) != 0) {
		keyobj = key->key;
		error = pgpRemoveObject (set->keyDB, keyobj);
		if (error)
			goto err;
	}

err:
	if (kirem)
		pgpFreeKeyIter (kirem);
	if (klrem)
		pgpFreeKeyList (klrem);
	return error;
}


void
pgpFreeKeySet(PGPKeySet *keys)
{
	pgpa (pgpaPGPKeySetValid(keys));

	keys->refCount--;
	if (keys->refCount <= 0)
	{
		(*keys->destroy)(keys);

		if (keys->prevSetInDB)
			keys->prevSetInDB->nextSetInDB = keys->nextSetInDB;
		else
			keys->keyDB->firstSetInDB = keys->nextSetInDB;
		if (keys->nextSetInDB)
			keys->nextSetInDB->prevSetInDB = keys->prevSetInDB;

		pgpAssert(keys->firstListInSet == NULL);

		pgpFreeKeyDB (keys->keyDB);
		pgpFree (keys);
	}
}

Boolean
pgpKeySetIsMutable(PGPKeySet *keys)
{
	pgpa(pgpaPGPKeySetValid(keys));

	return pgpKeyDBIsMutable(keys->keyDB);
}

Boolean
pgpKeySetIsDirty(PGPKeySet *keys)
{
	pgpa(pgpaPGPKeySetValid(keys));

	return pgpKeyDBIsDirty(keys->keyDB);
}

/* Defines when a key should be expanded or collapsed. If set to 1,
	keys are expanded by pgpBuildKeyPool. If set to 2, they are expanded
	when referenced by an iterator, or when the key refCount is explicitly
	incremented by the app. */
#define PGP_EXPANDKEY 1

PGPError
pgpIncKeyRefCount(PGPKey *key)
{

	pgpa(pgpaAddrValid(key, PGPKey));

	key->refCount++;
	if (key->refCount == PGP_EXPANDKEY)
		return pgpExpandKey (key);
	return PGPERR_OK;
}

PGPError
pgpFreeKey(PGPKey *key)
{
	PGPError	err = PGPERR_OK;

	pgpa(pgpaPGPKeyValid(key));

	if (key->refCount == PGP_EXPANDKEY)
		err = pgpCollapseKey (key);
	key->refCount--;
	if (key->refCount <= 0)
		deallocKey(key->keyDB, key);
	return err;
}

long
pgpCountKeys(PGPKeySet *keys)
{
	PGPKey *		key;
	long			count = 0;

	pgpa(pgpaPGPKeySetValid(keys));

	for (key = keys->keyDB->firstKeyInDB; key; key = key->nextKeyInDB)
	{
		pgpa(pgpaPGPKeyValid(key));
		if (pgpKeySetMember(keys, key))
			count++;
	}

	return count;
}

PGPKeyList *
pgpOrderKeySet(PGPKeySet *keys, PGPKeyOrdering order)
{
	PGPKeyList *	list;
	PGPKey *		key;
	long			i, count;

	pgpa(pgpaPGPKeySetValid(keys));

/*	list = pgpNew(PGPKeyList); */
	list = (PGPKeyList *)pgpAlloc(sizeof(PGPKeyList));
	if (list == NULL)
		return NULL;

	list->keyCount = count = pgpCountKeys(keys);
	list->keys = pgpAlloc(count * sizeof(PGPKey *));
	if (list->keys == NULL)
	{
		pgpFree(list);
		return NULL;
	}
	
	list->refCount = 1;
	list->keySet = keys;
	list->order = order;
	list->prevListInSet = NULL;
	list->nextListInSet = keys->firstListInSet;
	keys->firstListInSet = list;
	list->firstIterInList = NULL;

	pgpIncKeySetRefCount(keys);

	i = 0;
	for (key = keys->keyDB->firstKeyInDB; key; key = key->nextKeyInDB)
	{
		pgpa(pgpaPGPKeyValid(key));
		if (pgpKeySetMember(keys, key))
		{
			pgpAssert(i < count);
			list->keys[i++] = key;
		}
	}
	pgpAssert(i == count);

	sortKeyList(list);

	return list;
}

void
pgpIncKeyListRefCount(PGPKeyList *list)
{
	pgpa(pgpaPGPKeyListValid(list));

	list->refCount++;
}

void
pgpFreeKeyList(PGPKeyList *list)
{
	pgpa(pgpaPGPKeyListValid(list));

	list->refCount--;
	if (list->refCount <= 0)
	{
		if (list->prevListInSet)
			list->prevListInSet->nextListInSet = list->nextListInSet;
		else
			list->keySet->firstListInSet = list->nextListInSet;
		if (list->nextListInSet)
			list->nextListInSet->prevListInSet = list->prevListInSet;

		pgpAssert(list->firstIterInList == NULL);

		pgpFree(list->keys);
		pgpFreeKeySet(list->keySet);
		pgpFree(list);
	}
}

PGPKey *
pgpGetKeyByKeyID(PGPKeySet *keys, byte *keyID, size_t keyIDLength)
{
	PGPKeyDB *	db;
	long		lo;
	long		hi;
	long		i;
	PGPKey **	 keyArray;
	size_t		len;
	uchar	 	keyIDA[8];
	uchar	 	keyIDB[8];
	int			comparison;

	pgpa((
		pgpaPGPKeySetValid(keys),
		pgpaAddrValid(keyID, byte),
		pgpaAssert(keyIDLength == 8)));
	(void)keyIDLength;		/* Avoid warning */

	db = keys->keyDB;

	if (db->keysByKeyID == NULL)
	{
		PGPKeySet *		rootSet;

		if ((rootSet = pgpKeyDBRootSet(db)) != NULL)
		{
			db->keysByKeyID = pgpOrderKeySet(rootSet, kPGPKeyIDOrdering);

			/* rootSet will stick around until the keylist is freed */
			pgpFreeKeySet(rootSet);

				/*
			* Undo the additional refCount created by the existence of
			* rootSet. Otherwise the keyDB will never be freed because
			* of the cycle in the reference graph. When the keyDB is
			* actually freed, if keysByKeyID exists, refCount will be
			* incremented again before freeing keysByKeyID.
				*/
			if (db->keysByKeyID != NULL)
				db->refCount--;
		}
		if (db->keysByKeyID == NULL)
			return NULL;
	}

	pgpa((
		pgpaPGPKeyListValid(db->keysByKeyID),
		pgpaAssert(db->keysByKeyID->order == kPGPKeyIDOrdering)));

	keyArray = db->keysByKeyID->keys;
	pgpCopyMemory(keyID, keyIDA, 8);

	lo = 0;
	hi = db->keysByKeyID->keyCount;

	while (lo < hi)
	{
		i = (lo + hi) / 2;
		len = sizeof(keyIDB);
		pgpGetKeyString(keyArray[i], kPGPKeyPropKeyId, (char *)keyIDB, &len);
		comparison = compareKeyIDs(keyIDA, keyIDB);
		if (comparison > 0)
			lo = i + 1;
		else if (comparison < 0)
			hi = i;
		else if (pgpKeySetMember(keys, keyArray[i]))
			return keyArray[i];
		else
			break;
	}
	return NULL;
}

#if DEBUG	/* [ */

	Boolean
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

	Boolean
pgpaInternalPGPKeySetValid(
	pgpaCallPrefixDef,
	PGPKeySet const *	 keySet,
	char const *		varName)
{
	pgpaAddrValid(keySet, PGPKeySet);
	pgpaFailIf(keySet->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPKeySetValid failed on %s (%p)", varName, keySet));

	return pgpaFailed;
}

	Boolean
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

	Boolean
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

	Boolean
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

	Boolean
pgpaInternalPGPSubKeyValid(
	pgpaCallPrefixDef,
	PGPSubKey const *	 subKey,
	char const *		varName)
{
	pgpaAddrValid(subKey, PGPSubKey);
/*	pgpaFailIf(subKey->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));	*/
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPSubKeyValid failed on %s (%p)", varName, subKey));

	return pgpaFailed;
}

	Boolean
pgpaInternalPGPUserIDValid(
	pgpaCallPrefixDef,
	PGPUserID const *	 userID,
	char const *		varName)
{
	pgpaAddrValid(userID, PGPUserID);
/*	pgpaFailIf(userID->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0"));	*/
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPUserIDValid failed on %s (%p)", varName, userID));

	return pgpaFailed;
}

	Boolean
pgpaInternalPGPCertValid(
	pgpaCallPrefixDef,
	PGPCert const *		cert,
	char const *		varName)
{
	pgpaAddrValid(cert, PGPCert);
/*	pgpaFailIf(cert->refCount <= 0, (pgpaFmtPrefix, "refCount <= 0")); 	*/
	pgpaFmtMsg((pgpaFmtPrefix,
			"pgpaPGPCertValid failed on %s (%p)", varName, cert));

	return pgpaFailed;
}

#endif /* ] DEBUG */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/

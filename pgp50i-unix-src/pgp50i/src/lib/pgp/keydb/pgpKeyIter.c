/*
 * pgpKeyIter.c -- PGP Key Iteration functions
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpKeyIter.c,v 1.25.2.1 1997/06/07 09:50:25 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpKDBint.h"


/* Internal functions to maintain reference counts. Incrementing
   or decrementing a refcount on a sub-object will do the same to
   its parent object (in fact, all superior objects), as a subobject
   cannot exist without its parent.

   When an iterator moves off an object or subobject, must make sure
   all child objects are properly adjusted (the decIterxxx functions),
   as the iterator implicitly moves off these subobjects also. */

static void
decKeyRefCount (PGPKey *key)
{
	if (key)
		pgpFreeKey(key);
}

static void
decSubKeyRefCount (PGPSubKey *subkey)
{
	if (subkey) {
	subkey->refCount--;
		decKeyRefCount (subkey->key);
	}
}

static void
decUserIDRefCount (PGPUserID *userid)
{
	if (userid) {
		userid->refCount--;
		decKeyRefCount (userid->key);
	}
}

static void
decCertRefCount (PGPCert *cert)
{
	if (cert) {
		cert->refCount--;
		if (cert->type == uidcert)
		decUserIDRefCount (cert->up.userID);
		else
		decKeyRefCount (cert->up.key);
	}
}

static void
decIterUIDCert (PGPKeyIter *iter)
{
	decCertRefCount (iter->uidCert);
	iter->uidCert = NULL;
}

static void
decIterUserID (PGPKeyIter *iter)
{
	decIterUIDCert (iter);
	decUserIDRefCount (iter->userID);
	iter->userID = NULL;
}

static void
decIterSubKey (PGPKeyIter *iter)
{
	decSubKeyRefCount (iter->subKey);
	iter->subKey = NULL;
}

static void
decIterKey (PGPKeyIter *iter)
{
decIterSubKey (iter);
	decIterUserID (iter);
	decKeyRefCount (iter->key);
	iter->key = NULL;
}

static PGPError
incKeyRefCount (PGPKey *key)
{
	if (key)
		return pgpIncKeyRefCount(key);
	return PGPERR_OK;
}

static void
incSubKeyRefCount (PGPSubKey *subkey)
{
incKeyRefCount (subkey->key);
	subkey->refCount++;
}

static void
incUserIDRefCount (PGPUserID *userid)
{
	incKeyRefCount (userid->key);
	userid->refCount++;
}

static void
incCertRefCount (PGPCert *cert)
{
	if (cert->type == uidcert)
	incUserIDRefCount (cert->up.userID);
	else
	incKeyRefCount (cert->up.key);
	cert->refCount++;
}



/* Externally visible functions */

/* Functions to increment and decrement sub-object reference
	counts. */

void
pgpIncSubKeyRefCount (PGPSubKey *subkey)
{
pgpa(pgpaPGPSubKeyValid(subkey));
	incSubKeyRefCount (subkey);
}

void
pgpIncUserIDRefCount (PGPUserID *userid)
{
	pgpa(pgpaPGPUserIDValid(userid));
	incUserIDRefCount (userid);
}

void
pgpIncCertRefCount (PGPCert *cert)
{
pgpa(pgpaPGPCertValid(cert));
	incCertRefCount (cert);
}

void
pgpFreeSubKey (PGPSubKey *subkey)
{
pgpa(pgpaPGPSubKeyValid(subkey));
	decSubKeyRefCount (subkey);
}

void
pgpFreeUserID (PGPUserID *userid)
{
	pgpa(pgpaPGPUserIDValid(userid));
	decUserIDRefCount (userid);
}

void
pgpFreeCert (PGPCert *cert)
{
pgpa(pgpaPGPCertValid(cert));
	decCertRefCount (cert);
}


/* pgpNewKeyIter can be called with a NULL keylist to iterate within
	a key only. If this is done, pgpKeyIterSet must be called to supply
	the key. Obviously, if an iterator is created in this way,
	pgpKeyIterNext and its companions that find other keys cannot be
	used. */

PGPKeyIter PGPKDBExport *
pgpNewKeyIter (PGPKeyList *keys)
{
	PGPKeyIter		*newIter = NULL;

	pgpa(pgpaPGPKeyListValid(keys));

	newIter = pgpMemAlloc (sizeof (PGPKeyIter));
	if (!newIter)
		return NULL;

	if (keys) {
	newIter->nextIterInList = keys->firstIterInList;
		newIter->prevIterInList = NULL;
		if (newIter->nextIterInList)
		newIter->nextIterInList->prevIterInList = newIter;
		keys->firstIterInList = newIter;
		pgpIncKeyListRefCount(keys);
	}
	newIter->keyList = keys;

	newIter->keyIndex = -1;
	newIter->key = NULL;
	newIter->subKey = NULL;
	newIter->userID = NULL;
	newIter->uidCert = NULL;
	return newIter;
}


PGPKeyIter PGPKDBExport *
pgpCopyKeyIter (PGPKeyIter *orig)
{
	PGPKeyIter		*newIter = NULL;

	pgpa(pgpaPGPKeyIterValid(orig));
	newIter = pgpNewKeyIter (orig->keyList);
	if (newIter) {
		newIter->keyIndex = orig->keyIndex;

		newIter->key = orig->key;
		if (newIter->key)
			incKeyRefCount(newIter->key);

		newIter->subKey = orig->subKey;
		if (newIter->subKey)
		incSubKeyRefCount (newIter->subKey);

		newIter->userID = orig->userID;
		if (newIter->userID)
		incUserIDRefCount (newIter->userID);

		newIter->uidCert = orig->uidCert;
		if (newIter->uidCert)
		incCertRefCount (newIter->uidCert);
	}
	return newIter;
}


void PGPKDBExport
pgpFreeKeyIter (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));

	if (iter->keyList) {
	if (iter->prevIterInList)
		iter->prevIterInList->nextIterInList = iter->nextIterInList;
		else
		iter->keyList->firstIterInList = iter->nextIterInList;
		if (iter->nextIterInList)
		iter->nextIterInList->prevIterInList = iter->prevIterInList;
		pgpFreeKeyList (iter->keyList);
	}
	decIterKey (iter);
	pgpMemFree (iter);
}
		

long PGPKDBExport
pgpKeyIterIndex (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	return iter->keyIndex;
}


void PGPKDBExport
pgpKeyIterRewind (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	if (iter->keyList) {
		decIterKey (iter);
		iter->keyIndex = -1;
	}
}


long PGPKDBExport
pgpKeyIterSet (PGPKeyIter *iter, PGPKey *key)
{
	PGPKey		**keys;
	long		i;

	pgpa((pgpaPGPKeyIterValid(iter),pgpaPGPKeyValid(key)));
	decIterKey (iter);
	iter->keyIndex = -1;
	iter->key = key;
	incKeyRefCount(key);
	keys = iter->keyList->keys;
	if (iter->keyList && (keys = iter->keyList->keys) != NULL) {
	for (i = 0; i < iter->keyList->keyCount; i++) {
		if (keys[i] == key) {
			iter->keyIndex = i;
				break;
			}
		}
	}
	return iter->keyIndex;
}


PGPKey PGPKDBExport *
pgpKeyIterMove (PGPKeyIter *iter, long relOffset)
{
	long newOffset;
	PGPKeyList *keylist = NULL;

	pgpa(pgpaPGPKeyIterValid(iter));
	if (!iter->keyList)
	return iter->key;
	keylist = iter->keyList;
	newOffset = iter->keyIndex + relOffset;

	/* Check for removed key */
	if (iter->key && keylist->keys[iter->keyIndex] != iter->key) {
	/* key removed from keylist */
	if (relOffset == 0)
			return NULL;		/* XXX Is this correct? */
		else if (relOffset > 0)
		newOffset--;
	}

	decIterKey (iter);
	iter->keyIndex = newOffset;
	if (newOffset >= 0 && newOffset < keylist->keyCount)
	{
		iter->key = keylist->keys[newOffset];
		incKeyRefCount(iter->key);
	}
	else
		iter->key = NULL;
	return iter->key;
}


PGPKey PGPKDBExport *
pgpKeyIterNext (PGPKeyIter *iter)
{
	return pgpKeyIterMove(iter, 1);
}


PGPKey PGPKDBExport *
pgpKeyIterPrev (PGPKeyIter *iter)
{
	return pgpKeyIterMove(iter, -1);
}


Boolean PGPKDBExport
pgpKeyIterValid (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	return (iter->key != NULL);
}


PGPKey PGPKDBExport *
pgpKeyIterKey (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	return iter->key;
}

PGPSubKey PGPKDBExport *
pgpKeyIterSubKey (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	return iter->subKey;
}

PGPUserID PGPKDBExport *
pgpKeyIterUserID (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	return iter->userID;
}


PGPCert PGPKDBExport *
pgpKeyIterCert (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	return iter->uidCert;
}

PGPSubKey PGPKDBExport *
pgpKeyIterNextSubKey (PGPKeyIter *iter)
{
	PGPKey			*key;
	PGPSubKey *next = NULL;

	pgpa(pgpaPGPKeyIterValid(iter));
	if (!pgpKeyIterValid (iter))
		return NULL;
	key = iter->key;
	if (!iter->subKey)
		next = (PGPSubKey *) key->subKeys.next;
	else
		next = iter->subKey->next;

	while (next != (PGPSubKey *) &key->subKeys) {
		if (next->removed)
			next = next->next;
		else {
		decIterSubKey (iter);
		iter->subKey = next;
			incSubKeyRefCount (next);
			return next;
		}
	}
	return NULL;
}


PGPSubKey PGPKDBExport *
pgpKeyIterPrevSubKey (PGPKeyIter *iter)
{
	PGPKey		*key;
	PGPSubKey *prev;

	pgpa(pgpaPGPKeyIterValid(iter));
	if (!pgpKeyIterValid (iter))
		return NULL;
	key = iter->key;
	if (!iter->subKey)
		prev = (PGPSubKey *) key->subKeys.prev;
	else
		prev = iter->subKey->prev;

	while (prev != (PGPSubKey *) &key->subKeys) {
		if (prev->removed)
			prev = prev->prev;
		else {
		decIterSubKey (iter);
			iter->subKey = prev;
			incSubKeyRefCount (prev);
			return prev;
		}
	}
	return NULL;
}


void PGPKDBExport
pgpKeyIterRewindSubKey (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	if (pgpKeyIterValid (iter))
		decIterSubKey (iter);
}


PGPUserID PGPKDBExport *
pgpKeyIterNextUserID (PGPKeyIter *iter)
{
	PGPKey			*key;
	PGPUserID *next = NULL;

	pgpa(pgpaPGPKeyIterValid(iter));
	if (!pgpKeyIterValid (iter))
		return NULL;
	key = iter->key;
	if (!iter->userID)
		next = (PGPUserID *) key->userIDs.next;
	else
		next = iter->userID->next;

	while (next != (PGPUserID *) &key->userIDs) {
		if (next->removed)
			next = next->next;
		else {
		decIterUserID (iter);
		iter->userID = next;
			incUserIDRefCount (next);
			return next;
		}
	}
	return NULL;
}


PGPUserID PGPKDBExport *
pgpKeyIterPrevUserID (PGPKeyIter *iter)
{
	PGPKey		*key;
	PGPUserID *prev;

	pgpa(pgpaPGPKeyIterValid(iter));
	if (!pgpKeyIterValid (iter))
		return NULL;
	key = iter->key;
	if (!iter->userID)
		prev = (PGPUserID *) key->userIDs.prev;
	else
		prev = iter->userID->prev;

	while (prev != (PGPUserID *) &key->userIDs) {
		if (prev->removed)
			prev = prev->prev;
		else {
		decIterUserID (iter);
			iter->userID = prev;
			incUserIDRefCount (prev);
			return prev;
		}
	}
	return NULL;
}


void PGPKDBExport
pgpKeyIterRewindUserID (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	if (pgpKeyIterValid (iter))
		decIterUserID (iter);
}


PGPCert PGPKDBExport *
pgpKeyIterNextUIDCert (PGPKeyIter *iter)
{
	PGPKey			*key;
	PGPUserID	 	*userid;
	PGPCert *next;

	pgpa(pgpaPGPKeyIterValid(iter));
	if (!pgpKeyIterValid (iter) || !iter->userID || iter->userID->removed)
		return NULL;
	key = iter->key;
	userid = iter->userID;
	if (!iter->uidCert)
		next = (PGPCert *) userid->certs.next;
	else
		next = iter->uidCert->next;

	while (next != (PGPCert *) &userid->certs) {
		if (next->removed)
			next = next->next;
		else {
		decIterUIDCert (iter);
			iter->uidCert = next;
			incCertRefCount (next);;
			return next;
		}
	}
	return NULL;
}


PGPCert PGPKDBExport *
pgpKeyIterPrevUIDCert (PGPKeyIter *iter)
{
	PGPKey			*key;
	PGPUserID	 	*userid;
	PGPCert *prev;

	pgpa(pgpaPGPKeyIterValid(iter));
	if (!pgpKeyIterValid (iter) || !iter->userID || iter->userID->removed)
		return NULL;
	key = iter->key;
	userid = iter->userID;
	if (!iter->uidCert)
		prev = (PGPCert *) userid->certs.prev;
	else
		prev = iter->uidCert->prev;

	while (prev != (PGPCert *) &userid->certs) {
		if (prev->removed)
			prev = prev->prev;
		else {
		decIterUIDCert (iter);
			iter->uidCert = prev;
			incCertRefCount (prev);
			return prev;
		}
	}
	return NULL;
}


void PGPKDBExport
pgpKeyIterRewindUIDCert (PGPKeyIter *iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	if (pgpKeyIterValid (iter) && iter->userID && !iter->userID->removed)
		decIterUIDCert (iter);
}



/* Internal functions */

/* Adjust iterators when a key is added to or removed from a keylist */

void
pgpKeyIterAddKey (PGPKeyList *keys, long idx)
{
	PGPKeyIter		*iter;

	pgpa(pgpaPGPKeyListValid(keys));
	for (iter = keys->firstIterInList; iter; iter = iter->nextIterInList)
	{
		if (iter->key == keys->keys[idx])
			iter->keyIndex = idx;
		else if (iter->keyIndex > idx)
			iter->keyIndex++;
		else if (iter->keyIndex == idx && keys->order != kPGPAnyOrdering
					&& iter->key != NULL
					&& pgpCompareKeys(iter->key, keys->keys[idx],
										keys->order) > 0)
			iter->keyIndex++;
	}
}

void
pgpKeyIterRemoveKey (PGPKeyList *keys, long idx)
{
	PGPKeyIter		*iter;

	pgpa(pgpaPGPKeyListValid(keys));
	for (iter = keys->firstIterInList; iter; iter = iter->nextIterInList)
		if (iter->keyIndex > idx)
			iter->keyIndex--;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

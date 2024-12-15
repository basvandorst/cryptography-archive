/*
 * pgpKeyUpd.c -- Functions to update sub-objects of keys
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpKeyUpd.c,v 1.22.2.1 1997/06/07 09:50:28 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpKDBint.h"
#include "pgpDebug.h"
#include "pgpRngPub.h"
#include "pgpSigSpec.h"


static PGPUserID *
allocUserID (PGPKeyDB *keyDB)
{
	PGPUserID	*userid;

	if (keyDB->firstFreeUserID != NULL) {
		userid = keyDB->firstFreeUserID;
		keyDB->firstFreeUserID = userid->next;
	}
	else
		userid = memPoolNew (&keyDB->keyPool, PGPUserID);
	return userid;
}


static PGPSubKey *
allocSubKey (PGPKeyDB *keyDB)
{
	PGPSubKey	*subkey;

	if (keyDB->firstFreeSubKey != NULL) {
		subkey = keyDB->firstFreeSubKey;
		keyDB->firstFreeSubKey = subkey->next;
	}
	else
		subkey = memPoolNew (&keyDB->keyPool, PGPSubKey);
	return subkey;
}



static PGPCert *
allocCert (PGPKeyDB *keyDB)
{
	PGPCert		*cert;

	if (keyDB->firstFreeCert != NULL) {
		cert = keyDB->firstFreeCert;
		keyDB->firstFreeCert = cert->next;
	}
	else
		cert = memPoolNew (&keyDB->keyPool, PGPCert);
	return cert;
}


/*	Remove the PGPUserID from its doubly linked list and
	add it to the keydb free list. If it has PGPCerts
	attached, deallocate them also. */

void
pgpDeallocUserID (PGPUserID *userid)
{
		pgpa(pgpaAssert(userid->refCount == 0));
		userid->prev->next = userid->next;
	userid->next->prev = userid->prev;
	userid->next = userid->key->keyDB->firstFreeUserID;
	userid->key->keyDB->firstFreeUserID = userid;
	/* Free subordinate certs */
	while (userid->certs.next != &userid->certs)
		pgpDeallocCert ((PGPCert *) userid->certs.next);
}

/* Remove a subkey from its doubly linked list */

void
pgpDeallocSubKey (PGPSubKey *subkey)
{
		pgpa(pgpaAssert(subkey->refCount == 0));
		subkey->prev->next = subkey->next;
	subkey->next->prev = subkey->prev;
	subkey->next = subkey->key->keyDB->firstFreeSubKey;
	subkey->key->keyDB->firstFreeSubKey = subkey;
}

/*	Remove the PGPCert from its doubly linked list and
	add it to the keydb free list */

void
pgpDeallocCert (PGPCert *cert)
{
		pgpa(pgpaAssert(cert->refCount == 0));
	cert->prev->next = cert->next;
	cert->next->prev = cert->prev;
		if (cert->type == uidcert) {
			cert->next = cert->up.userID->key->keyDB->firstFreeCert;
				cert->up.userID->key->keyDB->firstFreeCert = cert;
		}
		else {
			/* no key certificates for now */
			pgpAssert (0);
		}
}


static PGPError
createNewUserID (PGPKey *key, PGPUserID *prev, union RingObject *nameobj)
{
	PGPUserID				*newuid = NULL;

	newuid = allocUserID (key->keyDB);
	if (!newuid)
		return PGPERR_NOMEM;

	newuid->next = prev->next;
	newuid->prev = newuid->next->prev;
	newuid->next->prev = newuid;
	newuid->prev->next = newuid;
	newuid->certs.next = &newuid->certs;
	newuid->certs.prev = newuid->certs.next;
	newuid->userID = nameobj;
	newuid->key = key;
	newuid->refCount = 0;
	newuid->removed = 0;
	newuid->userVal = 0;
	return PGPERR_OK;
}


static PGPError
createNewUidCert (PGPUserID *userid, PGPCert *prev, union RingObject *sigobj)
{
	PGPCert	*newcert = NULL;

	newcert = allocCert (userid->key->keyDB);
	if (!newcert)
		return PGPERR_NOMEM;

	newcert->next = prev->next;
	newcert->prev = newcert->next->prev;
	newcert->next->prev = newcert;
	newcert->prev->next = newcert;
		newcert->type = uidcert;
	newcert->cert = sigobj;
	newcert->up.userID = userid;
	newcert->refCount = 0;
	newcert->removed = 0;
	newcert->userVal = 0;
	return PGPERR_OK;
}


static PGPError
createNewSubKey (PGPKey *key, PGPSubKey *prev, union RingObject *subkeyobj)
{
	PGPSubKey		*newsubkey = NULL;

	newsubkey = allocSubKey (key->keyDB);
	if (!newsubkey)
		return PGPERR_NOMEM;

	newsubkey->next = prev->next;
	newsubkey->prev = newsubkey->next->prev;
	newsubkey->next->prev = newsubkey;
	newsubkey->prev->next = newsubkey;
	newsubkey->subKey = subkeyobj;
newsubkey->key = key;
newsubkey->refCount = 0;
newsubkey->removed = 0;
newsubkey->userVal = 0;
return PGPERR_OK;
}



/*	Expand the sub-objects of the specified key. This function should be
	called when the refCount of a key goes from 0 to 1.
	This function is also called to update an already expanded key with
	new userids and/or certs. */

PGPError
pgpExpandKey (PGPKey *key)
{
	PGPKeyDB						*keyDB;
	struct RingSet	 		*ringset = NULL;
	struct RingIterator	*iter;
	union RingObject		*nameobj, *sigobj, *subkeyobj;
	PGPUserID						*userid;
	PGPCert							*cert;
		PGPSubKey 			*subkey;
	int									sigtype;
	PGPError						err = PGPERR_OK;

	pgpa(pgpaPGPKeyValid(key));

	keyDB = key->keyDB;
	ringset = pgpKeyDBRingSet (keyDB);
	iter = ringIterCreate (ringset);
	if (!iter)
		return ringSetError(ringset)->error;
	if (ringIterSeekTo (iter, key->key) != 1)
		return PGPERR_BADPARAM;

	 /* Expand subkeys */
		subkey = (PGPSubKey *) key->subKeys.next;
		while (subkey != (PGPSubKey *) &key->subKeys && subkey->removed)
		subkey = subkey->next;
	while ((err = ringIterNextObject (iter, 2)) > 0) {
		subkeyobj = ringIterCurrentObject (iter, 2);
				if (ringObjectType (subkeyobj) != RINGTYPE_KEY)
					continue; /* secobj or nameobj */
				if (subkey == (PGPSubKey *) &key->subKeys ||
						subkeyobj != subkey->subKey) { /* not found */
						err = createNewSubKey (key, subkey->prev, subkeyobj);
						if (err)
								goto cleanup;
				}
				else {	/* found */
					subkey = subkey->next;
						while (subkey != (PGPSubKey *) &key->subKeys && subkey->removed)
				subkey = subkey->next;
		}
	}
	ringIterRewind (iter, 2); /* rewind to find names */

		/*	Expand UserIDs */
	userid = (PGPUserID *) key->userIDs.next;
		while (userid != (PGPUserID *) &key->userIDs && userid->removed)
				userid = userid->next;
	while ((err = ringIterNextObject (iter, 2)) > 0) {
		nameobj = ringIterCurrentObject (iter, 2);
				if (ringObjectType (nameobj) != RINGTYPE_NAME)
					continue; /* secobj or subkey */
				if (userid == (PGPUserID *) &key->userIDs ||
						nameobj != userid->userID) { /* not found */
						err = createNewUserID (key, userid->prev, nameobj);
							if (err)
									goto cleanup;
					}
	else {	/* found */
							userid = userid->next;
						while (userid != (PGPUserID *) &key->userIDs && userid->removed)
								userid = userid->next;
				}

				/*	Expand certifications on UserID (userid->prev) */
				cert = (PGPCert *) userid->prev->certs.next;
				while (cert != (PGPCert *) &userid->prev->certs && cert->removed)
						cert = cert->next;
				while ((err = ringIterNextObject (iter, 3)) > 0) {
						sigobj = ringIterCurrentObject (iter, 3);
						pgpAssert (ringObjectType (sigobj) == RINGTYPE_SIG);
						sigtype = ringSigType (ringset, sigobj);
						if (sigtype < PGP_SIGTYPE_KEY_GENERIC ||
								sigtype > PGP_SIGTYPE_KEY_POSITIVE)
								continue;
						if (cert == (PGPCert *) &userid->prev->certs ||
								sigobj != cert->cert) { /* not found */
								err = createNewUidCert (userid->prev, cert->prev, sigobj);
								if (err)
										goto cleanup;
						}
						else {
								cert = cert->next;
								while (cert != (PGPCert *) &userid->prev->certs &&
										cert->removed)
										cert = cert->next;
						}
				}
	}
cleanup:
	if (iter)
		ringIterDestroy (iter);
	return err;
}


/*	Collapse the expanded subobjects. Refcount of all subobjects
		must be zero. */

PGPError
pgpCollapseKey (PGPKey *key)
{
	pgpa(pgpaAddrValid(key, PGPKey));

		while (key->subKeys.next != &key->subKeys)
			pgpDeallocSubKey ((PGPSubKey *) key->subKeys.next);
	while (key->userIDs.next != &key->userIDs)
		pgpDeallocUserID ((PGPUserID *) key->userIDs.next);
	return PGPERR_OK;
}



/*	Remove a RingObject from the Key Database. */

PGPError
pgpRemoveObject (PGPKeyDB *keyDB, union RingObject *obj)
{
	int									type, sigtype = 0;
	struct RingSet				*ringset;
	struct RingIterator	*iter;
	union RingObject		*nameobj = NULL, *keyobj;
	PGPKey								*key;
		PGPSubKey 			*subkey;
	PGPUserID						*userid;
	PGPCert							*cert;
		int 				level;
	PGPError						err = PGPERR_OK;

	pgpa(pgpaPGPKeyDBValid(keyDB));
	type = ringObjectType (obj);

	/*	Get superior RingObjects before deleting the subject object */
		ringset = pgpKeyDBRingSet (keyDB);
	iter = ringIterCreate (ringset);
	if (!iter)
		return ringSetError(ringset)->error;
	level = ringIterSeekTo (iter, obj);
	keyobj = ringIterCurrentObject (iter, 1);
	if (type == RINGTYPE_SIG) {
			sigtype = ringSigType (ringset, obj);
		nameobj = ringIterCurrentObject (iter, 2);
		}

	/*	Remove from master ringset. If it's still a member of the ringset,
				then it could not be removed from at least one ringset that makes
				up the master ringset. In this case, we're done. */

	(*keyDB->remove) (keyDB, obj);
	ringset = pgpKeyDBRingSet (keyDB);
	if (ringSetIsMember (ringset, obj))
		return PGPERR_OK;

	/*	Locate PGPKey object */
	key = pgpGetKeyByRingObject (keyDB, keyobj);
	if (!key) {
		err = PGPERR_KEYDB_CORRUPT;
				goto cleanup;
	}

	if (type == RINGTYPE_KEY && level == 1)
				pgpBuildKeyPool (keyDB, 1);
		else if (type == RINGTYPE_KEY && level == 2) {
		subkey = (PGPSubKey *) key->subKeys.next;
				/*	Search for the PGPSubKey object that references obj */
				while (subkey != (PGPSubKey *) &key->subKeys && subkey->subKey != obj)
						subkey = subkey->next;
				if (subkey != (PGPSubKey *) &key->subKeys)
						subkey->removed = 1;
				else {
					err = PGPERR_KEYDB_CORRUPT;
						goto cleanup;
				}
		}
	else if (type == RINGTYPE_NAME) {
		userid = (PGPUserID *) key->userIDs.next;
				/*	Search for the PGPUserID object that references obj */
				while (userid != (PGPUserID *) &key->userIDs &&
						!(userid->userID == obj && !userid->removed)) {
						userid = userid->next;
				}
				if (userid != (PGPUserID *) &key->userIDs) {
						userid->removed = 1;
						/*	Mark all attached certs as removed also */
						for (cert = (PGPCert *) userid->certs.next; cert !=
								(PGPCert *) &userid->certs;
								cert = cert->next)
								cert->removed = 1;
				}
				else { /*	not found */
						err = PGPERR_KEYDB_CORRUPT;
						goto cleanup;
				}
		}
	else if (type == RINGTYPE_SIG && sigtype != PGP_SIGTYPE_KEY_UID_REVOKE &&
						sigtype != PGP_SIGTYPE_KEY_COMPROMISE) {
		userid = (PGPUserID *) key->userIDs.next;
				/*	Search for parent PGPUserID */
				while (userid != (PGPUserID *) &key->userIDs &&
						!(userid->userID == nameobj && !userid->removed)) {
						userid = userid->next;
				}
				if (userid == (PGPUserID *) &key->userIDs) {
						err = PGPERR_KEYDB_CORRUPT;
						goto cleanup;
				}
				cert = (PGPCert *) userid->certs.next;
				/*	Search for the PGPCert object that references obj */
				while (cert != (PGPCert *) &userid->certs &&
						!(cert->cert == obj && !cert->removed)) {
						cert = cert->next;
				}
				if (cert != (PGPCert *) &userid->certs)
						cert->removed = 1;
				else { /*	not found */
						err = PGPERR_KEYDB_CORRUPT;
						goto cleanup;
				}
	}

		/*	Re-sort the keylist */
		if (type != RINGTYPE_KEY) {
				RingSet *changedset = ringSetCreate (pgpRingPool);
				if (!changedset) {
						err = ringPoolError(pgpRingPool)->error;
						goto cleanup;
				}
				ringSetAddObject (changedset, obj);
				err = pgpKeyDBChanged (keyDB, changedset);
				ringSetDestroy (changedset);
		}
cleanup:
	if (iter)
		ringIterDestroy (iter);
	return err;
}


/*	Add PGPUserID and PGPCert sub-objects attached to a key. New
	objects are added in same the position at which they are found in
	the source RingSet. If the key being added does not exist, it
	is created. Order of objects in the RingSet must match those in
	the PGPUserID and PGPCert list.

	Note that <addset> can contain multiple keys.
*/
								
PGPError
pgpAddObjects (PGPKeyDB *keyDB, struct RingSet *addset)
{
	union RingObject		*keyobj = NULL;
	struct RingIterator	*iter = NULL;
	PGPKey				*key;
	PGPError				err = PGPERR_OK;

	pgpa(pgpaPGPKeyDBValid(keyDB));

	/*	Add the new objects to the key database. */
	ringSetFreeze (addset);
	(*keyDB->add) (keyDB, addset);
	err = pgpBuildKeyPool (keyDB, 0);
	if (err)
		return err;

	iter = ringIterCreate (addset);
	if (!iter)
		return ringSetError(addset)->error;

	/*	Loop for multiple keys to be added */
	while ((err = ringIterNextObject (iter, 1)) > 0) {
		keyobj = ringIterCurrentObject (iter, 1);
				pgpAssert (ringObjectType (keyobj) == RINGTYPE_KEY);
				key = pgpGetKeyByRingObject (keyDB, keyobj);
				if (!key) {
						err = PGPERR_KEYDB_CORRUPT;
						goto cleanup;
				}
				/*	if key expanded, update it */
				if (key->userIDs.next != &key->userIDs) {
						err = pgpExpandKey (key);
						if (err)
								goto cleanup;
				}
	}

		/*	Resort as necessary */
		err = pgpKeyDBChanged (keyDB, addset);

cleanup:
	if (iter)
		ringIterDestroy (iter);
	return err;
}



/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
*/

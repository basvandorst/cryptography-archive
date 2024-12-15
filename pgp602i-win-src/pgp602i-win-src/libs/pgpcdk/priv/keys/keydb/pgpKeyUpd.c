/*
 * pgpKeyUpd.c
 * Functions to update sub-objects of keys.
 *
 * Copyright (C) 1996,1997 Network Associates, Inc. and its affiliates.
 * All rights reserved
 *
 * $Id: pgpKeyUpd.c,v 1.15.10.1 1998/11/12 03:22:04 heller Exp $
 */

#include "pgpConfig.h"

#include "pgpContext.h"
#include "pgpKDBInt.h"
#include "pgpDebug.h"
#include "pgpRngPub.h"
#include "pgpSigSpec.h"


static PGPUserIDRef
allocUserID (PGPKeyDB *keyDB)
{
    PGPUserID	*userid;

    if (keyDB->firstFreeUserID != NULL) {
        userid = keyDB->firstFreeUserID;
        keyDB->firstFreeUserID = userid->next;
    }
    else
        userid = (PGPUserID *)memPoolNew (&keyDB->keyPool, PGPUserID);
        
#if PGP_OPTIONAL_MAGICS
	if ( IsntNull( userid ) )
	{
		userid->magic		= kPGPUserIDMagic;
	}
#endif

    return userid;
}


static PGPSubKeyRef 
allocSubKey (PGPKeyDB *keyDB)
{
    PGPSubKey	*subkey;

    if (keyDB->firstFreeSubKey != NULL) {
        subkey = keyDB->firstFreeSubKey;
        keyDB->firstFreeSubKey = subkey->next;
    }
    else
        subkey = (PGPSubKey *)memPoolNew (&keyDB->keyPool, PGPSubKey);
        
#if PGP_OPTIONAL_MAGICS
	if ( IsntNull( subkey ) )
	{
		subkey->magic		= kPGPSubKeyMagic;
	}
#endif

    return subkey;
}



static PGPSig *
allocCert (PGPKeyDB *keyDB)
{
    PGPSig		*cert;

    if (keyDB->firstFreeCert != NULL) {
        cert			= keyDB->firstFreeCert;
		keyDB->firstFreeCert = cert->next;
    }
    else
    {
        cert = (PGPSig *)memPoolNew (&keyDB->keyPool, PGPSig);
	}
	
#if PGP_OPTIONAL_MAGICS
	if ( IsntNull( cert ) )
	{
		cert->magic		= kPGPCertMagic;
	}
#endif
    return cert;
}


/*  Remove the PGPUserID from its doubly linked list and
    add it to the keydb free list.  If it has PGPCerts 
    attached, deallocate them also. */

void
pgpDeallocUserID (PGPUserID *userid)
{
	pgpa(pgpaAssert(userid->refCount == 0));
#if PGP_OPTIONAL_MAGICS
	userid->magic			= ~userid->magic;
#endif
	userid->prev->next = userid->next;
    userid->next->prev = userid->prev;
    userid->next = userid->key->keyDB->firstFreeUserID;
    userid->key->keyDB->firstFreeUserID = userid;
    /*  Free subordinate certs */
    while (userid->certs.next != &userid->certs)
        pgpDeallocCert ((PGPSig *) userid->certs.next);
}

/*  Remove a subkey from its doubly linked list */

void
pgpDeallocSubKey (PGPSubKeyRef subkey)
{
	pgpa(pgpaAssert(subkey->refCount == 0));
#if PGP_OPTIONAL_MAGICS
	subkey->magic		= ~subkey->magic;
#endif
	subkey->prev->next	= subkey->next;
    subkey->next->prev	= subkey->prev;
    subkey->next = subkey->key->keyDB->firstFreeSubKey;
    subkey->key->keyDB->firstFreeSubKey = subkey;
}

/*  Remove the PGPSig from its doubly linked list and
    add it to the keydb free list */

void
pgpDeallocCert (PGPSig *cert)
{
	pgpa(pgpaAssert(cert->refCount == 0));
#if PGP_OPTIONAL_MAGICS
	cert->magic			= ~cert->magic;
#endif
    cert->prev->next	= cert->next;
    cert->next->prev	= cert->prev;
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
    PGPUserID		*newuid = NULL;

    newuid = allocUserID (key->keyDB);
    if (!newuid)
        return kPGPError_OutOfMemory;

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
    return kPGPError_NoErr;
}


static PGPError
createNewUidCert (PGPUserID *userid, PGPSig *prev, union RingObject *sigobj)
{
    PGPSig	*newcert = NULL;

    newcert = allocCert (userid->key->keyDB);
    if (!newcert)
        return kPGPError_OutOfMemory;

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
    return kPGPError_NoErr;
}


static PGPError
createNewSubKey (PGPKey *key, PGPSubKeyRef prev, union RingObject *subkeyobj)
{
    PGPSubKey	*newsubkey = NULL;

    newsubkey = allocSubKey (key->keyDB);
    if (!newsubkey)
        return kPGPError_OutOfMemory;

    newsubkey->next = prev->next;
    newsubkey->prev = newsubkey->next->prev;
    newsubkey->next->prev = newsubkey;
    newsubkey->prev->next = newsubkey;
    newsubkey->subKey = subkeyobj;
    newsubkey->key = key;
    newsubkey->refCount = 0;
    newsubkey->removed = 0;
    newsubkey->userVal = 0;
    return kPGPError_NoErr;
}



/*  Expand the sub-objects of the specified key.  This function should be
    called when the refCount of a key goes from 0 to 1. 
    This function is also called to update an already expanded key with
    new userids and/or certs. */

PGPError
pgpExpandKey (PGPKey *key)
{
    PGPKeyDB			*keyDB;
    RingSet const		*ringset = NULL;
    RingIterator	*iter;
    union RingObject	*nameobj, *sigobj, *subkeyobj;
    PGPUserID			*userid;
    PGPSig				*cert;
	PGPSubKey           *subkey;
    int					sigtype;
    PGPError			err = kPGPError_NoErr;

    pgpa(pgpaPGPKeyValid(key));

    keyDB = key->keyDB;
    ringset = pgpKeyDBRingSet (keyDB);
    iter = ringIterCreate (ringset);
    if (!iter)
        return ringSetError(ringset)->error;
    if (ringIterSeekTo (iter, key->key) != 1)
        return kPGPError_BadParams;

	/*  Expand subkeys */
	subkey = (PGPSubKeyRef ) key->subKeys.next;
	while (subkey != (PGPSubKeyRef ) &key->subKeys && subkey->removed)
	    subkey = subkey->next;
    while ((err = (PGPError)ringIterNextObject (iter, 2)) > 0) {
        subkeyobj = ringIterCurrentObject (iter, 2);
		if (ringObjectType (subkeyobj) != RINGTYPE_KEY)
			continue;   /* secobj or nameobj */
		if (subkey == (PGPSubKeyRef ) &key->subKeys ||
			subkeyobj != subkey->subKey) {   /* not found */
			err = createNewSubKey (key, subkey->prev, subkeyobj);
			if (err)
				goto cleanup;
		}
		else {                              /* found */
			subkey = subkey->next;
			while (subkey != (PGPSubKeyRef ) &key->subKeys && subkey->removed)
				subkey = subkey->next;
		}
	}
	ringIterRewind (iter, 2);   /* rewind to find names */

	/*  Expand UserIDs */
    userid = (PGPUserID *) key->userIDs.next;
	while (userid != (PGPUserID *) &key->userIDs && userid->removed)
		userid = userid->next;
    while ((err = (PGPError)ringIterNextObject (iter, 2)) > 0) {
        nameobj = ringIterCurrentObject (iter, 2);
		if (ringObjectType (nameobj) != RINGTYPE_NAME)
			continue;   /* secobj or subkey */
		if (userid == (PGPUserID *) &key->userIDs ||
			nameobj != userid->userID) {   /* not found */
			err = createNewUserID (key, userid->prev, nameobj);
			if (err)
				goto cleanup;
		}
		else {                              /* found */
			userid = userid->next;
			while (userid != (PGPUserID *) &key->userIDs && userid->removed)
				userid = userid->next;
		}

		/*  Expand certifications on UserID (userid->prev) */
		cert = (PGPSig *) userid->prev->certs.next;
		while (cert != (PGPSig *) &userid->prev->certs && cert->removed)
			cert = cert->next;
		while ((err = (PGPError)ringIterNextObject (iter, 3)) > 0) {
			sigobj = ringIterCurrentObject (iter, 3);
			pgpAssert (ringObjectType (sigobj) == RINGTYPE_SIG);
			sigtype = ringSigType (ringset, sigobj);
			if (sigtype < PGP_SIGTYPE_KEY_GENERIC || 
				sigtype > PGP_SIGTYPE_KEY_POSITIVE)
				continue;
			if (cert == (PGPSig *) &userid->prev->certs ||
				sigobj != cert->cert) {     /* not found */
				err = createNewUidCert (userid->prev, cert->prev, sigobj);
				if (err)
					goto cleanup;
			}
			else {
				cert = cert->next;
				while (cert != (PGPSig *) &userid->prev->certs && 
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


/*  Collapse the expanded subobjects.  Refcount of all subobjects
	must be zero. */

PGPError
pgpCollapseKey (PGPKey *key)
{
    pgpa(pgpaAddrValid(key, PGPKey));

	while (key->subKeys.next != &key->subKeys)
	    pgpDeallocSubKey ((PGPSubKeyRef ) key->subKeys.next);
    while (key->userIDs.next != &key->userIDs) 
        pgpDeallocUserID ((PGPUserID *) key->userIDs.next);
    return kPGPError_NoErr;
}



/*  Remove a RingObject from the Key Database.  */

PGPError 
pgpRemoveObject (PGPKeyDB *keyDB, union RingObject *obj)
{
    int					type, sigtype = 0;
    RingSet	const	*ringset;
    RingIterator	*iter;
    union RingObject	*nameobj = NULL, *keyobj;
    PGPKey				*key;
	PGPSubKey           *subkey;
    PGPUserID			*userid;
    PGPSig				*cert;
	int                 level;
	RingPool			*pgpRingPool;
    PGPError			err = kPGPError_NoErr;
    
    pgpa(pgpaPGPKeyDBValid(keyDB));
    type = ringObjectType (obj);

    /*  Get superior RingObjects before deleting the subject object */
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

    /*  Remove from master ringset.  If it's still a member of the ringset,
		then it could not be removed from at least one ringset that makes 
		up the master ringset. In this case, we're done.  */

    err = keyDB->remove(keyDB, obj);
	if (err)
		return err;
    ringset = pgpKeyDBRingSet (keyDB);
    if (ringSetIsMember (ringset, obj))
        return kPGPError_NoErr;

    /*  Locate PGPKey object */
    key = pgpGetKeyByRingObject (keyDB, keyobj);
    if (!key) {
        err = kPGPError_FileCorrupt;
		goto cleanup;
    }

    if (type == RINGTYPE_KEY && level == 1)
		pgpBuildKeyPool (keyDB, 1);
	else if (type == RINGTYPE_KEY && level == 2) {
        subkey = (PGPSubKeyRef ) key->subKeys.next;
		/*  Search for the PGPSubKey object that references obj */
		while (subkey != (PGPSubKeyRef ) &key->subKeys && 
				(subkey->removed || subkey->subKey != obj) )
			subkey = subkey->next;
		if (subkey != (PGPSubKeyRef ) &key->subKeys)
			subkey->removed = 1;
		else {
		    err = kPGPError_FileCorrupt;
			goto cleanup;
		}
	}
    else if (type == RINGTYPE_NAME) {
        userid = (PGPUserID *) key->userIDs.next;
		/*  Search for the PGPUserID object that references obj */
		while (userid != (PGPUserID *) &key->userIDs &&
			   !(userid->userID == obj && !userid->removed)) {
			userid = userid->next;
		}
		if (userid != (PGPUserID *) &key->userIDs) {
			userid->removed = 1;
			/*  Mark all attached certs as removed also */
			for (cert = (PGPSig *) userid->certs.next; cert != 
				   (PGPSig *) &userid->certs;
				 cert = cert->next)
				cert->removed = 1;
		}
		else {   /* not found */
			err = kPGPError_FileCorrupt;
			goto cleanup;
		}
	}
    else if (type == RINGTYPE_SIG && sigtype != PGP_SIGTYPE_KEY_UID_REVOKE &&
			 sigtype != PGP_SIGTYPE_KEY_REVOKE) {
        userid = (PGPUserID *) key->userIDs.next;
		/* Search for parent PGPUserID */
		while (userid != (PGPUserID *) &key->userIDs && 
			   !(userid->userID == nameobj && !userid->removed)) {
			userid = userid->next;
		}
		if (userid == (PGPUserID *) &key->userIDs) {
			err = kPGPError_FileCorrupt;
			goto cleanup;
		}
		cert = (PGPSig *) userid->certs.next;
		/* Search for the PGPSig object that references obj */
		while (cert != (PGPSig *) &userid->certs &&
			   !(cert->cert == obj && !cert->removed)) {
			cert = cert->next;
		}
		if (cert != (PGPSig *) &userid->certs)
			cert->removed = 1;
		else {   /* not found */
			err = kPGPError_FileCorrupt;
			goto cleanup;
		}
    }

	/*  Re-sort the keylist */
	if (type != RINGTYPE_KEY) {
		RingSet *changedset;
		pgpRingPool = pgpContextGetRingPool( keyDB->context );
		changedset = ringSetCreate (pgpRingPool);
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


/*  Add PGPUserID and PGPSig sub-objects attached to a key.   New 
    objects are added in same the position at which they are found in 
    the source RingSet.  If the key being added does not exist, it
    is created.  Order of objects in the RingSet must match those in
    the PGPUserID and PGPSig list.

    Note that <addset> can contain multiple keys.
*/
				 
PGPError
pgpAddObjects (PGPKeyDB *keyDB, RingSet *addset)
{
    union RingObject	*keyobj = NULL;
    RingIterator	*iter = NULL;
    PGPKey			*key;
    PGPError		err = kPGPError_NoErr;

    pgpa(pgpaPGPKeyDBValid(keyDB));

    /* Add the new objects to the key database. */
    ringSetFreeze (addset);
    err = keyDB->add(keyDB, addset);
    if (err)
	{
		pgpBuildKeyPool (keyDB, 0);
        return err;
	}
    err = pgpBuildKeyPool (keyDB, 0);
    if (err)
        return err;

    iter = ringIterCreate (addset);
    if (!iter)
        return ringSetError(addset)->error;

    /* Loop for multiple keys to be added */
    while ((err = (PGPError)ringIterNextObject (iter, 1)) > 0) {
        keyobj = ringIterCurrentObject (iter, 1);
		pgpAssert (ringObjectType (keyobj) == RINGTYPE_KEY);
		key = pgpGetKeyByRingObject (keyDB, keyobj);
		if (!key) {
			err = kPGPError_FileCorrupt;
			goto cleanup;
		}
		/*  if key expanded, update it */
		if (key->userIDs.next != &key->userIDs) {
			err = pgpExpandKey (key);
			if (err)
				goto cleanup;
		}
    }

	/* Resort as necessary */
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

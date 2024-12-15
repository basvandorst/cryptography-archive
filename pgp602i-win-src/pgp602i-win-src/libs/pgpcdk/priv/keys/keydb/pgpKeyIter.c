/*
 * pgpKeyIter.c
 * PGP Key Iteration functions
 *
 * Copyright (C) 1996,1997 Network Associates, Inc. and its affiliates.
 * All rights reserved
 *
 * $Id: pgpKeyIter.c,v 1.36.12.1 1998/11/12 03:21:58 heller Exp $
*/

#include "pgpConfig.h"

#include "pgpKDBInt.h"
#include "pgpContext.h"


/* Internal functions to maintain reference counts.  Incrementing
   or decrementing a refcount on a sub-object will do the same to 
   its parent object (in fact, all superior objects), as a subobject
   cannot exist without its parent.   

   When an iterator moves off an object or subobject, must make sure 
   all child objects are properly adjusted (the decIterxxx functions),
   as the iterator implicitly moves off these subobjects also. */

static void
decKeyRefCount (PGPKeyRef key)
{
	if ( IsntNull( key ) )
	{
		pgpa( pgpaPGPKeyValid( key ) );
		pgpFreeKey(key);
	}
}

static void
decSubKeyRefCount (PGPSubKeyRef subkey)
{
    if ( IsntNull( subkey ) )
    {
		pgpa( pgpaPGPSubKeyValid( subkey ) );
		
	    subkey->refCount--;
		decKeyRefCount (subkey->key);
	}
}

static void
decUserIDRefCount (PGPUserIDRef userid)
{
	if ( IsntNull( userid ) )
	{
		pgpa( pgpaPGPUserIDValid( userid ) );
		userid->refCount--;
		decKeyRefCount (userid->key);
	}
}

static void
decCertRefCount (PGPSigRef cert)
{
	if ( IsntNull( cert ) )
	{
		pgpa( pgpaPGPCertValid( cert ) );
		
		cert->refCount--;
		if (cert->type == uidcert)
		    decUserIDRefCount (cert->up.userID);
		else
		    decKeyRefCount (cert->up.key);
	}
}

static void 
decIterUIDCert (PGPKeyIterRef iter)
{
	decCertRefCount (iter->uidCert);
	iter->uidCert = NULL;
}

static void 
decIterUserID (PGPKeyIterRef iter)
{
	decIterUIDCert (iter);
	decUserIDRefCount (iter->userID);
	iter->userID = NULL;
}

static void
decIterSubKey (PGPKeyIterRef iter)
{
    decSubKeyRefCount (iter->subKey);
	iter->subKey = NULL;
}

static void 
decIterKey (PGPKeyIterRef iter)
{
    decIterSubKey (iter);
	decIterUserID (iter);
	decKeyRefCount (iter->key);
	iter->key = NULL;
}

static PGPError
incKeyRefCount (PGPKeyRef key)
{
	if ( IsntNull( key ) )
		return pgpIncKeyRefCount(key);
	return kPGPError_NoErr;
}

static void
incSubKeyRefCount (PGPSubKeyRef subkey)
{
    incKeyRefCount (subkey->key);
	subkey->refCount++;
}

static void
incUserIDRefCount (PGPUserIDRef userid)
{
	incKeyRefCount (userid->key);
	userid->refCount++;
}

static void
incCertRefCount (PGPSigRef cert)
{
    if (cert->type == uidcert)
	    incUserIDRefCount (cert->up.userID);
	else
	    incKeyRefCount (cert->up.key);
	cert->refCount++;
}



/*  Externally visible functions */

/*  Functions to increment and decrement sub-object reference
	counts. */

PGPError  
pgpIncSubKeyRefCount (PGPSubKeyRef subkey)
{
    pgpa(pgpaPGPSubKeyValid(subkey));
	incSubKeyRefCount (subkey);
	
	return( kPGPError_NoErr );
}

PGPError  
pgpIncUserIDRefCount (PGPUserIDRef userid)
{
	PGPValidateUserID( userid );
	
	incUserIDRefCount (userid);
	
	return( kPGPError_NoErr );
}

PGPError  
pgpIncSigRefCount (PGPSigRef cert)
{
	PGPValidateCert( cert );
	
	incCertRefCount (cert);
	
	return( kPGPError_NoErr );
}

PGPError  
pgpFreeSubKey (PGPSubKeyRef subkey)
{
    pgpa(pgpaPGPSubKeyValid(subkey));
    
	PGPValidateSubKey( subkey );
	
	decSubKeyRefCount (subkey);
	return( kPGPError_NoErr );
}

PGPError  
pgpFreeUserID (PGPUserIDRef userid)
{
	PGPValidateUserID( userid );
	
	decUserIDRefCount (userid);
	return( kPGPError_NoErr );
}

PGPError  
pgpFreeSig (PGPSigRef cert)
{
	PGPValidateCert( cert );
	
	decCertRefCount (cert);
	return( kPGPError_NoErr );
}


	PGPError 
PGPNewKeyIter (
	PGPKeyListRef	keys,
	PGPKeyIterRef *	outRef)
{
	PGPKeyIter * newIter = NULL;
	PGPError	err	= kPGPError_NoErr;
	PGPContextRef	context	= PGPGetKeyListContext( keys );

	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyList( keys );

	newIter = (PGPKeyIter *)pgpContextMemAlloc(
		context, sizeof (PGPKeyIter), kPGPMemoryMgrFlags_Clear );
	if ( IsntNull( newIter ) )
	{
		newIter->magic	= kPGPKeyIterMagic;
	    newIter->nextIterInList = keys->firstIterInList;
		newIter->prevIterInList = NULL;
		if (newIter->nextIterInList)
		    newIter->nextIterInList->prevIterInList = newIter;
		keys->firstIterInList = newIter;
		PGPIncKeyListRefCount(keys);
		newIter->keyList = keys;

		newIter->keyIndex = -1;
		newIter->key = NULL;
		newIter->subKey = NULL;
		newIter->userID = NULL;
		newIter->uidCert = NULL;
		
		*outRef	= newIter;
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	pgpAssertErrWithPtr( err, *outRef );
	return( err );
}


	PGPError  
PGPCopyKeyIter (
	PGPKeyIterRef 	orig,
	PGPKeyIterRef *	outRef)
{
	PGPKeyIterRef	newIter = NULL;
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( orig );
	
	err	= PGPNewKeyIter (orig->keyList, &newIter );
	if ( IsntPGPError( err ) )
	{
		newIter->keyIndex = orig->keyIndex;

		newIter->key = orig->key;
		if (newIter->key) 
			incKeyRefCount (newIter->key);

		newIter->subKey = orig->subKey;
		if (newIter->subKey)
		    incSubKeyRefCount (newIter->subKey);

		newIter->userID = orig->userID;
		if (newIter->userID)
		    incUserIDRefCount (newIter->userID);

		newIter->uidCert = orig->uidCert;
		if (newIter->uidCert)
		    incCertRefCount (newIter->uidCert);

		*outRef = newIter;
	}
	
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}


PGPError 
PGPFreeKeyIter (PGPKeyIterRef iter)
{
	PGPContextRef	context	= NULL;
	
	PGPValidateKeyIter( iter );
	context	= PGPGetKeyIterContext( iter );

	if (iter->keyList) {
	    if (iter->prevIterInList)
		    iter->prevIterInList->nextIterInList = iter->nextIterInList;
		else
		    iter->keyList->firstIterInList = iter->nextIterInList;
		if (iter->nextIterInList)
		    iter->nextIterInList->prevIterInList = iter->prevIterInList;
		PGPFreeKeyList (iter->keyList);
	}
	decIterKey (iter);
	iter->magic		= ~ iter->magic;	/* mark as invalid */
	pgpContextMemFree( context, iter);
	
	return( kPGPError_NoErr );
}
		

PGPInt32 
PGPKeyIterIndex (PGPKeyIterRef iter)
{
   	pgpa(pgpaPGPKeyIterValid(iter));
   	
	if ( ! pgpKeyIterIsValid( iter ) )
		return( 0 );
		
	return iter->keyIndex;
}


PGPError 
PGPKeyIterRewind (PGPKeyIterRef iter)
{
	pgpa(pgpaPGPKeyIterValid(iter));
	PGPValidateKeyIter( iter );
	
	if (iter->keyList) {
		decIterKey (iter);
		iter->keyIndex = -1;
		iter->key = NULL;
	}
	
	return( kPGPError_NoErr );
}


PGPInt32 
PGPKeyIterSeek (PGPKeyIterRef iter, PGPKeyRef key)
{
	PGPKey		**keys;
	long		i;

	PGPValidateKeyIter( iter );
	PGPValidateKey( key );
	
	decIterKey (iter);
	iter->keyIndex = -1;
	iter->key = NULL;
	incKeyRefCount(key);
	keys = iter->keyList->keys;
	if (iter->keyList && (keys = iter->keyList->keys) != NULL) {
	    for (i = 0; i < iter->keyList->keyCount; i++) {
		    if (keys[i] == key) {
			    iter->keyIndex = i;
				iter->key = key;
				break;
			}
		}
	}
	return iter->keyIndex;
}


PGPError 
PGPKeyIterMove (
	PGPKeyIterRef	iter,
	PGPInt32		relOffset,
	PGPKeyRef *		outRef )
{
    long       		newOffset;
	PGPKeyListRef	keylist = NULL;
	PGPError		err	= kPGPError_NoErr;

	pgpa(pgpaPGPKeyIterValid(iter));
	pgpAssertAddrValid( outRef, PGPKeyRef );
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );

	
	if ( IsntNull( iter->keyList ) )
	{
		keylist		= iter->keyList;
		newOffset	= iter->keyIndex + relOffset;

		/*  Check for removed key */
	    if ( IsntNull( iter->key ) && iter->keyIndex >= 0 &&
				( iter->keyIndex >= keylist->keyCount ||
	    		  keylist->keys[iter->keyIndex] != iter->key ) )
	    {
		    /* key removed from keylist */ 
		    if (relOffset == 0) 
			{
				/* pgpFixBeforeShip( "this is probably wrong" ); */
				err	= kPGPError_EndOfIteration;
				return( err );
			}
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
		{
			iter->key = NULL;
		}
	}
	
	*outRef	= iter->key;
	if ( IsNull( iter->key ) )
		err	= kPGPError_EndOfIteration;
		
	return err;
}


PGPError 
PGPKeyIterNext (
	PGPKeyIterRef	iter,
	PGPKeyRef *		outRef)
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	err	= PGPKeyIterMove(iter, 1, outRef );
	
	pgpAssertErrWithPtr( err, *outRef );
	return( err );
}


PGPError 
PGPKeyIterPrev (
	PGPKeyIterRef	iter,
	PGPKeyRef *		outRef)
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	err	= PGPKeyIterMove( iter, -1, outRef);
	
	pgpAssertErrWithPtr( err, *outRef );
	return( err );
}



PGPError 
PGPKeyIterKey (
	PGPKeyIterRef	iter,
	PGPKeyRef *		outRef)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpa(pgpaPGPKeyIterValid(iter));
	pgpAssertAddrValid( outRef, PGPKeyRef );
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	/* pgpFixBeforeShip( "what if key has been deleted" ) */
	if ( IsNull( iter->key ) )
	{
		err	= kPGPError_EndOfIteration;
	}
	
	*outRef	= iter->key;
	return err;
}

PGPError 
PGPKeyIterSubKey (
	PGPKeyIterRef	iter,
	PGPSubKeyRef *	outRef)
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	if ( IsNull( iter->subKey ) )
	{
		err	= kPGPError_EndOfIteration;
	}
	
	*outRef	= iter->subKey;
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}    

PGPError 
PGPKeyIterUserID (
	PGPKeyIterRef	iter,
	PGPUserIDRef *	outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	if ( IsNull( iter->userID ) )
	{
		err	= kPGPError_EndOfIteration;
	}
	
	*outRef	= iter->userID;
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}


PGPError 
PGPKeyIterSig (
	PGPKeyIterRef	iter,
	PGPSigRef *	outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpa(pgpaPGPKeyIterValid(iter));
	pgpAssertAddrValid( outRef, PGPSigRef );
	 
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	if ( IsNull( iter->uidCert ) )
	{
		err	= kPGPError_EndOfIteration;
	}
	
	*outRef	= iter->uidCert;
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}

PGPError 
PGPKeyIterNextSubKey (
	PGPKeyIterRef	iter,
	PGPSubKeyRef *	outRef )
{
	PGPKey			*key;
	PGPSubKey       *next = NULL;
	PGPError		err	= kPGPError_EndOfIteration;

	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	*outRef	= NULL;
		
	key = iter->key;
	if (!iter->subKey)
		next = (PGPSubKeyRef ) key->subKeys.next;
	else
		next = iter->subKey->next;

	while (next != (PGPSubKeyRef ) &key->subKeys)
	{
		if (next->removed)
			next = next->next;
		else
		{
		    decIterSubKey (iter);
		    iter->subKey = next;
			incSubKeyRefCount (next);
			
			*outRef	= next;
			err	= kPGPError_NoErr;
			break;
		}
	}
    
    pgpAssertErrWithPtr( err, *outRef );
    return( err );
}


/*____________________________________________________________________________
	Get the first subkey on a key.
	
	If the key doesn't have a subkey, then return kPGPError_ItemNotFound.
____________________________________________________________________________*/
	PGPError
pgpGetFirstSubKey(
	PGPKeyRef		key,
	PGPSubKeyRef *	outSubKey )
{
	PGPSubKeyRef	subKey	= NULL;
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr(outSubKey);
	*outSubKey = NULL;

	subKey = (PGPSubKeyRef)key->subKeys.next;
	while (	subKey != (PGPSubKeyRef)&key->subKeys &&
			subKey->removed)
	{
		subKey = subKey->next;
	}

	if (subKey != (PGPSubKeyRef)&key->subKeys)
	{
		*outSubKey = subKey;
	}
	
	if ( IsNull( *outSubKey ) )
   		err	= kPGPError_ItemNotFound;
   		
	return err;
}

PGPError 
PGPKeyIterPrevSubKey (
	PGPKeyIterRef	iter,
	PGPSubKeyRef *	outRef )
{
	PGPKey		*key;
	PGPSubKey   *prev;
	PGPError	err	= kPGPError_EndOfIteration;

	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
		
	key = iter->key;
	if (!iter->subKey)
		prev = (PGPSubKeyRef ) key->subKeys.prev;
	else
		prev = iter->subKey->prev;

	while (prev != (PGPSubKeyRef ) &key->subKeys)
	{
		if (prev->removed)
			prev = prev->prev;
		else
		{
		    decIterSubKey (iter);
			iter->subKey = prev;
			incSubKeyRefCount (prev);
			*outRef	= prev;
			err	= kPGPError_NoErr;
			break;
		}
	}

    pgpAssertErrWithPtr( err, *outRef );
    return( err );
}


	PGPError 
PGPKeyIterRewindSubKey (PGPKeyIterRef iter)
{
	PGPValidateKeyIter( iter );
	
	decIterSubKey (iter);
	return( kPGPError_NoErr );
}


PGPError 
PGPKeyIterNextUserID (
	PGPKeyIterRef	iter,
	PGPUserIDRef *	outRef )
{
	PGPKey			*key;
	PGPUserID       *next = NULL;
	PGPError		err	= kPGPError_EndOfIteration;

	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
		
	key = iter->key;
	if (!iter->userID)
		next = (PGPUserIDRef ) key->userIDs.next;
	else
		next = iter->userID->next;

	while (next != (PGPUserIDRef ) &key->userIDs)
	{
		if (next->removed)
			next = next->next;
		else
		{
		    decIterUserID (iter);
		    iter->userID = next;
			incUserIDRefCount (next);
			*outRef	= next;
			err	= kPGPError_NoErr;
			break;
		}
	}

    pgpAssertErrWithPtr( err, *outRef );
	return( err );
}


PGPError 
PGPKeyIterPrevUserID (
	PGPKeyIterRef 	iter,
	PGPUserIDRef *	outRef)
{
	PGPKey		*key;
	PGPUserID   *prev;
	PGPError		err	= kPGPError_EndOfIteration;

	pgpa(pgpaPGPKeyIterValid(iter));
	pgpAssertAddrValid( outRef, PGPUserIDRef );
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
		
	key = iter->key;
	if (!iter->userID)
		prev = (PGPUserIDRef ) key->userIDs.prev;
	else
		prev = iter->userID->prev;

	while (prev != (PGPUserIDRef ) &key->userIDs)
	{
		if (prev->removed)
			prev = prev->prev;
		else
		{
		    decIterUserID (iter);
			iter->userID = prev;
			incUserIDRefCount (prev);
			*outRef	= prev;
			err	= kPGPError_NoErr;
			break;
		}
	}
	
    pgpAssertErrWithPtr( err, *outRef );
	return( err );
}


PGPError 
PGPKeyIterRewindUserID (PGPKeyIterRef iter)
{
	PGPValidateKeyIter( iter );
	
	decIterUserID (iter);
	
	return( kPGPError_NoErr );
}


PGPError 
PGPKeyIterNextUIDSig (
	PGPKeyIterRef	iter,
	PGPSigRef *	outRef )
{
	PGPKey			*key;
	PGPUserID		*userid;
	PGPSig         *next;
	PGPError		err	= kPGPError_EndOfIteration;

	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	if ( IsNull( iter->userID ) ||
			iter->userID->removed)
	{
		return kPGPError_BadParams;
	}
		
	key = iter->key;
	userid = iter->userID;
	if (!iter->uidCert)
		next = (PGPSigRef ) userid->certs.next;
	else
		next = iter->uidCert->next;

	while (next != (PGPSigRef ) &userid->certs)
	{
		if (next->removed)
			next = next->next;
		else
		{
		    decIterUIDCert (iter);
			iter->uidCert = next;
			incCertRefCount (next);;
			*outRef	= next;
			err	= kPGPError_NoErr;
			break;
		}
	}
	
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}


PGPError 
PGPKeyIterPrevUIDSig (
	PGPKeyIterRef	iter,
	PGPSigRef *	outRef )
{
	PGPKey			*key;
	PGPUserID		*userid;
	PGPSig         *prev;
	PGPError		err	= kPGPError_EndOfIteration;

	pgpa(pgpaPGPKeyIterValid(iter));
	pgpAssertAddrValid( outRef, PGPSigRef );
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeyIter( iter );
	
	if ( !iter->userID || iter->userID->removed)
		return kPGPError_BadParams;
		
	key = iter->key;
	userid = iter->userID;
	if (!iter->uidCert)
		prev = (PGPSigRef ) userid->certs.prev;
	else
		prev = iter->uidCert->prev;

	while (prev != (PGPSigRef ) &userid->certs)
	{
		if (prev->removed)
			prev = prev->prev;
		else
		{
		    decIterUIDCert (iter);
			iter->uidCert = prev;
			incCertRefCount (prev);
			err	= kPGPError_NoErr;
			*outRef	= prev;
			break;
		}
	}
	
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}


PGPError 
PGPKeyIterRewindUIDSig (PGPKeyIterRef iter)
{
	PGPValidateKeyIter( iter );
	
	if ( iter->userID && !iter->userID->removed)
		decIterUIDCert (iter);
	return( kPGPError_NoErr );
}



/*  Internal functions */

/*  Adjust iterators when a key is added to or removed from a keylist */ 

void
pgpKeyIterAddKey (PGPKeyListRef keys, long idx)
{
	PGPKeyIterRef iter;

	pgpa(pgpaPGPKeyListValid(keys));
	for (iter = keys->firstIterInList; iter; iter = iter->nextIterInList)
	{
		if (iter->key == keys->keys[idx])
			iter->keyIndex = idx;
		else if (iter->keyIndex > idx)
			iter->keyIndex++;
		else if (iter->keyIndex == idx && keys->order != kPGPAnyOrdering
					&& iter->key != NULL
					&& PGPCompareKeys(iter->key, keys->keys[idx],
										keys->order) > 0)
			iter->keyIndex++;
	}
}

void
pgpKeyIterRemoveKey (PGPKeyListRef keys, long idx)
{
	PGPKeyIterRef iter;

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


/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPassphraseCache.cp,v 1.3.8.1 1997/12/05 22:14:41 mhw Exp $
____________________________________________________________________________*/

#include "pgpMem.h"
#include "CPassphraseCache.h"



/*____________________________________________________________________________

	Function Name:		CPassphraseCache::CPassphraseCache
____________________________________________________________________________*/
CPassphraseCache::CPassphraseCache()
	: mCache(nil), mCount(0), mKeySet(kPGPInvalidRef)
{
}



/*____________________________________________________________________________

	Function Name:		CPassphraseCache::~CPassphraseCache
____________________________________________________________________________*/
CPassphraseCache::~CPassphraseCache()
{
	if (mKeySet != kPGPInvalidRef) {
		PGPFreeKeySet(mKeySet);
	}
	if (mCache != nil) {
		WipeCache();
		::DisposePtr((Ptr) mCache);
	}
}
	


/*____________________________________________________________________________

	Function Name:		CPassphraseCache::AddOrUpdateItem
____________________________________________________________________________*/
void
CPassphraseCache::AddOrUpdateItem(
	UInt32			inTickCount,
	PGPKeyRef		inKey,
	const char *	inPassphrase)
{
	SPassphraseCacheItem *	newCache = nil;
	PGPKeySetRef			keySet = kPGPInvalidRef;
	
	try {
		PGPError	pgpErr;
		
		SPassphraseCacheItem *	theItemP;
		
		theItemP = FindItem(inKey);
		if (theItemP != nil) {
			theItemP->tickCount = inTickCount;
			theItemP->passphrase->Assign(inPassphrase);
		} else {
			// Set the new item
			newCache = (SPassphraseCacheItem *) ::NewPtr((mCount + 1) *
						sizeof(SPassphraseCacheItem));
			PGPThrowIfMemFail_(newCache);
			newCache[mCount].tickCount = inTickCount;
			newCache[mCount].passphrase = new CString(inPassphrase);
			PGPThrowIfNil_(newCache[mCount].passphrase);
			
			// Add the key to the keyset
			pgpErr = PGPNewSingletonKeySet(inKey, &keySet);
			PGPThrowIfPGPErr_(pgpErr);
			if (mKeySet == kPGPInvalidRef) {
				pgpErr = PGPNewKeySet(PGPGetKeyContext(inKey), &mKeySet);
				PGPThrowIfPGPErr_(pgpErr);
			}
			pgpErr = PGPAddKeys(keySet, mKeySet);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPCommitKeyRingChanges(mKeySet);
			PGPThrowIfPGPErr_(pgpErr);
			newCache[mCount].key = FindKeyInOtherKeySet(inKey, mKeySet);
			PGPThrowIf_(newCache[mCount].key == kPGPInvalidRef);
			
			// Copy the old data to the new cache
			if (mCache != nil) {
				::BlockMoveData(	mCache,
									newCache,
									::GetPtrSize((Ptr) mCache));
				WipeCache();
				::DisposePtr((Ptr) mCache);
			}
			mCount++;
			mCache = newCache;
			newCache = nil;
		}
	}
	
	catch (...) {
	}
	
	if (newCache != nil) {
		::DisposePtr((Ptr) newCache);
	}
	if (keySet != kPGPInvalidRef) {
		PGPFreeKeySet(keySet);
	}
}



/*____________________________________________________________________________

	Function Name:		CPassphraseCache::RemoveItem
____________________________________________________________________________*/
void
CPassphraseCache::RemoveItem(
	PGPKeyRef	inKey)
{
	SInt32			index;
	PGPKeySetRef	keySet = kPGPInvalidRef;
	PGPError		pgpErr;
	
	try {
		index = FindItemIndex(inKey);
		if (index != -1) {
			delete mCache[index].passphrase;
			pgpErr = PGPNewSingletonKeySet(mCache[index].key, &keySet);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPRemoveKeys(keySet, mKeySet);
			PGPThrowIfPGPErr_(pgpErr);
			
			::BlockMoveData(	&mCache[index + 1],
								&mCache[index],
								(mCount - index - 1) *
									sizeof(SPassphraseCacheItem));
			pgpClearMemory( &mCache[mCount - 1], sizeof(SPassphraseCacheItem));
			mCount--;
			::SetPtrSize((Ptr) mCache, mCount * sizeof(SPassphraseCacheItem));
		}
	}
	
	catch (...) {
	}
	
	if (keySet != kPGPInvalidRef) {
		PGPFreeKeySet(keySet);
	}
}


	
/*____________________________________________________________________________

	Function Name:		CPassphraseCache::FindItem
____________________________________________________________________________*/
SPassphraseCacheItem *
CPassphraseCache::FindItem(
	PGPKeyRef	inKey)
{
	SPassphraseCacheItem *	result = nil;
	SInt32					index;
	
	index = FindItemIndex(inKey);
	if (index != -1) {
		result = &mCache[index];
	}
	
	return result;
}
	


/*____________________________________________________________________________

	Function Name:		CPassphraseCache::ClearCache
____________________________________________________________________________*/
void
CPassphraseCache::ClearCache()
{
	if (mCache != nil) {
		for (SInt32 i = 0; i < mCount; i++) {
			delete mCache[i].passphrase;
		}
	}
	if (mKeySet != kPGPInvalidRef) {
		PGPContextRef	context = PGPGetKeySetContext(mKeySet);
	
		PGPFreeKeySet(mKeySet);
		PGPNewKeySet(context, &mKeySet);
	}
	WipeCache();
	mCount = 0;
}



/*____________________________________________________________________________

	Function Name:		CPassphraseCache::WipeCache
____________________________________________________________________________*/
void
CPassphraseCache::WipeCache()
{
	if (mCache != nil) {
		pgpClearMemory(mCache, ::GetPtrSize((Ptr) mCache));
	}
}



/*____________________________________________________________________________

	Function Name:		CPassphraseCache::FindItemIndex
____________________________________________________________________________*/
SInt32
CPassphraseCache::FindItemIndex(
	PGPKeyRef	inKey)
{
	SInt32	result = -1;
	
	if (mCache != nil) {
		for (SInt32 i = 0; i < mCount; i++) {
			if (PGPCompareKeys(inKey, mCache[i].key, kPGPKeyIDOrdering) == 0) {
				result = i;
				break;
			}
		}
	}
	
	return result;
}



/*____________________________________________________________________________

	Function Name:		CPassphraseCache::FindKeyInOtherKeySet
____________________________________________________________________________*/
PGPKeyRef
CPassphraseCache::FindKeyInOtherKeySet(
	PGPKeyRef		inKey,
	PGPKeySetRef	inKeySet)
{
	PGPKeyRef		result = kPGPInvalidRef;
	PGPKeyListRef	keyList = kPGPInvalidRef;
	PGPKeyIterRef	keyIter = kPGPInvalidRef;
	PGPError		pgpErr;
	
	try {
		pgpErr = PGPOrderKeySet(	inKeySet,
									kPGPAnyOrdering,
									&keyList);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPNewKeyIter(	keyList,
								&keyIter);
		PGPThrowIfPGPErr_(pgpErr);
		while ((PGPKeyIterNext(keyIter, &result) == kPGPError_NoErr)
		&& (result != kPGPInvalidRef)) {
			if (PGPCompareKeys(inKey, result, kPGPKeyIDOrdering) == 0) {
				break;
			}
		}
	}
	
	catch (...) {
	}
	
	if (keyIter != kPGPInvalidRef) {
		PGPFreeKeyIter(keyIter);
	}
	if (keyList != kPGPInvalidRef) {
		PGPFreeKeyList(keyList);
	}
	
	return result;
}

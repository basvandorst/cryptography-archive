/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPassphraseCache.h,v 1.2.8.1 1997/12/05 22:14:42 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPKeys.h"
#include "CString.h"



struct SPassphraseCacheItem {
	UInt32			tickCount;
	PGPKeyRef		key;
	CString *		passphrase;
};



class CPassphraseCache {
public:
							CPassphraseCache();
	virtual					~CPassphraseCache();
	
	void					AddOrUpdateItem(UInt32 inTickCount, PGPKeyRef inKey,
									const char * inPassphrase);
	void					RemoveItem(PGPKeyRef inKey);
	SPassphraseCacheItem *	FindItem(PGPKeyRef inKey);
	
	void					ClearCache();

protected:
	SPassphraseCacheItem *	mCache;
	SInt32					mCount;
	PGPKeySetRef			mKeySet;
	
	void					WipeCache();
	
	SInt32					FindItemIndex(PGPKeyRef inKey);
	
	PGPKeyRef				FindKeyInOtherKeySet(PGPKeyRef inKey,
									PGPKeySetRef inKeySet);
};

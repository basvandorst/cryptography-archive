/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CPassphraseCache.h,v 1.2.18.1 1998/11/12 03:10:07 heller Exp $____________________________________________________________________________*/#pragma once #include "PGPKeys.h"#include "CString.h"struct SPassphraseCacheItem {	UInt32			tickCount;	PGPKeyRef		key;	CString *		passphrase;};class CPassphraseCache {public:							CPassphraseCache();	virtual					~CPassphraseCache();		void					AddOrUpdateItem(UInt32 inTickCount, PGPKeyRef inKey,									const char * inPassphrase);	void					RemoveItem(PGPKeyRef inKey);	SPassphraseCacheItem *	FindItem(PGPKeyRef inKey);		void					ClearCache();protected:	SPassphraseCacheItem *	mCache;	SInt32					mCount;	PGPKeySetRef			mKeySet;		void					WipeCache();		SInt32					FindItemIndex(PGPKeyRef inKey);		PGPKeyRef				FindKeyInOtherKeySet(PGPKeyRef inKey,									PGPKeySetRef inKeySet);};
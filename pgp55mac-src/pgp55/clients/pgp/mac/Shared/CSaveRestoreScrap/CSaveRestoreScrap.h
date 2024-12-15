/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSaveRestoreScrap.h,v 1.3.10.1 1997/12/05 22:15:06 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CTempFile.h"

class CSaveRestoreScrap {
public:
				CSaveRestoreScrap();
				~CSaveRestoreScrap();
			
	void		Save();
	void		Restore();
	
	void		SetRestore(Boolean inRestore) { mRestore = inRestore; }
	
protected:
	Boolean		mScrapLoaded;
	SInt32		mScrapSize;
	SInt16		mScrapState;
	CTempFile *	mSavedScrap;
	LFile *		mScrapFile;
	Boolean		mRestore;
};

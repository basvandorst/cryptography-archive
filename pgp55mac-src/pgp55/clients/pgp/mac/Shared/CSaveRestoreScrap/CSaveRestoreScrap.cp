/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSaveRestoreScrap.cp,v 1.4.10.1 1997/12/05 22:15:05 mhw Exp $
____________________________________________________________________________*/

#include "MacFiles.h"

#include "CSaveRestoreScrap.h"



CSaveRestoreScrap::CSaveRestoreScrap()
	: mSavedScrap(nil), mScrapFile(nil), mRestore(false)
{
	Save();
}



CSaveRestoreScrap::~CSaveRestoreScrap()
{
	if (mRestore) {
		Restore();
	}
	delete mSavedScrap;
	delete mScrapFile;
}



	void
CSaveRestoreScrap::Save()
{
	// Save the scrap state
	mScrapSize = ::LMGetScrapSize();
	mScrapState = ::LMGetScrapState();
	mScrapLoaded = (::LMGetScrapHandle() != nil);
	
	// Unload the scrap if it is loaded
	if (mScrapLoaded) {
		::UnloadScrap();
	}
	
	// Save the scrap in a temp file
	OSErr	err;
	SInt16	vRefNum;
	SInt32	parID;
	FSSpec	scrapSpec;
	FSSpec	savedScrapSpec;
	
	err = ::FindFolder(	kOnSystemDisk,
						kSystemFolderType,
						false,
						&vRefNum,
						&parID);
	PGPThrowIfOSErr_(err);
	
	::FSMakeFSSpec(	vRefNum,
					parID,
					::LMGetScrapName(),
					&scrapSpec);
	mScrapFile = new LFile(scrapSpec);
	PGPThrowIfNil_(mScrapFile);
	mSavedScrap = new CTempFile(mScrapFile, false);
	PGPThrowIfNil_(mSavedScrap);
	mSavedScrap->GetSpecifier(savedScrapSpec);
	err = FSpCopyFiles(&scrapSpec, &savedScrapSpec);
	PGPThrowIfOSErr_(err);
	
	// Reload the scrap if necessary
	if (mScrapLoaded) {
		::LoadScrap();
	}
	
	mRestore = true;
}



	void
CSaveRestoreScrap::Restore()
{
	FSSpec		scrapSpec;
	SInt16		refNum;
	OSStatus	err;
	SInt32		numBytes;
	
	::ZeroScrap();
	::UnloadScrap();
	
	mScrapFile->GetSpecifier(scrapSpec);	
	err = FSpFindOpenForkRef(	&scrapSpec,
								false,
								&refNum);
	PGPThrowIfOSErr_(err);
	mSavedScrap->OpenDataFork(fsRdPerm);
	err = ::GetEOF(mSavedScrap->GetDataForkRefNum(), &numBytes);
	PGPThrowIfOSErr_(err);
	err = ::SetEOF(refNum, numBytes);
	PGPThrowIfOSErr_(err);
	err = ::SetFPos(refNum, fsFromStart, 0);
	PGPThrowIfOSErr_(err);
	err = FSCopyBytes(	mSavedScrap->GetDataForkRefNum(),
						0,
						refNum,
						0,
						numBytes);
	
	::LMSetScrapSize(mScrapSize);
	::LMSetScrapState(0);
	if (mScrapLoaded) {
		::LoadScrap();
	}
	::LMSetScrapState(mScrapState);
}

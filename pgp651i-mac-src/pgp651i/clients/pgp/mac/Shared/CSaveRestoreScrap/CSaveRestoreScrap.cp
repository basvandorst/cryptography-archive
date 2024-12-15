/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: CSaveRestoreScrap.cp,v 1.8 1999/04/10 11:15:10 jason Exp $____________________________________________________________________________*/#include <Folders.h>#include <LowMem.h>#include <Scrap.h>#include "MacFiles.h"#include "CPGPException.h"#include "CSaveRestoreScrap.h"CSaveRestoreScrap::CSaveRestoreScrap()	: mSavedScrap(nil), mScrapFile(nil), mRestore(false),	mMemoryMgr(kInvalidPGPMemoryMgrRef){	Save();}CSaveRestoreScrap::~CSaveRestoreScrap(){	if (mRestore) {		Restore();	}		if( PGPMemoryMgrRefIsValid( mMemoryMgr ) )		PGPFreeMemoryMgr( mMemoryMgr );	delete mSavedScrap;	delete mScrapFile;}	voidCSaveRestoreScrap::Save(){	// Save the scrap state	mScrapSize = ::LMGetScrapSize();	mScrapState = ::LMGetScrapState();	mScrapLoaded = (::LMGetScrapHandle() != nil);		// Unload the scrap if it is loaded	if (mScrapLoaded) {		::UnloadScrap();	}		// Save the scrap in a temp file	OSErr		err;	PGPError	pgpErr;	SInt16		vRefNum;	SInt32		parID;	FSSpec		scrapSpec;	FSSpec		savedScrapSpec;		err = ::FindFolder(	kOnSystemDisk,						kSystemFolderType,						false,						&vRefNum,						&parID);	PGPThrowIfOSError_(err);		::FSMakeFSSpec(	vRefNum,					parID,					::LMGetScrapName(),					&scrapSpec);	mScrapFile = new LFile(scrapSpec);	PGPThrowIfNULL_(mScrapFile);	mSavedScrap = new CTempFile(mScrapFile, false);	PGPThrowIfNULL_(mSavedScrap);	mSavedScrap->GetSpecifier(savedScrapSpec);	pgpErr = PGPNewMemoryMgr( 0, &mMemoryMgr );	PGPThrowIfPGPError_( pgpErr );	err = FSpCopyFiles(mMemoryMgr, &scrapSpec, &savedScrapSpec);	PGPThrowIfOSError_(err);		// Reload the scrap if necessary	if (mScrapLoaded) {		::LoadScrap();	}		mRestore = true;}	voidCSaveRestoreScrap::Restore(){	FSSpec		scrapSpec;	SInt16		refNum;	OSStatus	err;	SInt32		numBytes;		::ZeroScrap();	::UnloadScrap();		mScrapFile->GetSpecifier(scrapSpec);		err = FSpFindOpenForkRef(	&scrapSpec,								false,								&refNum);	PGPThrowIfOSError_(err);	mSavedScrap->OpenDataFork(fsRdPerm);	err = ::GetEOF(mSavedScrap->GetDataForkRefNum(), &numBytes);	PGPThrowIfOSError_(err);	err = ::SetEOF(refNum, numBytes);	PGPThrowIfOSError_(err);	err = ::SetFPos(refNum, fsFromStart, 0);	PGPThrowIfOSError_(err);	err = FSCopyBytes(	mMemoryMgr,						mSavedScrap->GetDataForkRefNum(),						0,						refNum,						0,						numBytes);		::LMSetScrapSize(mScrapSize);	::LMSetScrapState(0);	if (mScrapLoaded) {		::LoadScrap();	}	::LMSetScrapState(mScrapState);}
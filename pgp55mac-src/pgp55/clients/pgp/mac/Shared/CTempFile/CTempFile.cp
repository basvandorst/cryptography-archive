/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CTempFile.cp,v 1.5.10.1 1997/12/05 22:15:09 mhw Exp $
____________________________________________________________________________*/

#include "CTempFile.h"


CTempFile::CTempFile(
	LFile * inFile,
	Boolean	inCreate)
{
	FSSpec	theSpec;
	FInfo	theFinderInfo;
	
	inFile->GetSpecifier(theSpec);
	::FSpGetFInfo(	&theSpec,
					&theFinderInfo);
	InitCTempFile(	theFinderInfo.fdCreator,
					theFinderInfo.fdType,
					theSpec.name,
					inCreate);
}



CTempFile::CTempFile(
	OSType	inCreator,
	OSType	inFileType,
	Str63	inName,
	Boolean	inCreate)
{
	StringPtr	name = "\pTemp";
	
	if (inName != nil) {
		name = inName;
	}
	InitCTempFile(	inCreator,
					inFileType,
					name,
					inCreate);
}



	void
CTempFile::InitCTempFile(
	OSType	inCreator,
	OSType	inFileType,
	Str63	inName,
	Boolean	inCreate)
{
	Int16			theVRef;
	Int32			theDirID;
	OSStatus		err = noErr;

	// Get the temporary folder
	err = ::FindFolder(	kOnSystemDisk,
						kTemporaryFolderType,
						true,
						&theVRef,
						&theDirID);
	PGPThrowIfOSErr_(err);
	
	TString<Str63>	theFileName = inName;
	SInt32			theCount = 0;
	
	// Create FSSpecs until we find one that doesn't exist
	while (err != fnfErr) {
		err = ::FSMakeFSSpec(	theVRef,
								theDirID,
								theFileName,
								&mMacFileSpec);
								
		theFileName = inName + TString<Str31>(theCount++);
	}
	
	PGPThrowIf_(err != fnfErr);
	
	if (inCreate) {
		CreateNewFile(	inCreator,
						inFileType);
	}
}





CTempFile::~CTempFile()
{
	CloseDataFork();
	CloseResourceFork();

	// Delete the file
	::FSpDelete(&mMacFileSpec);
}

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: StSaveCurResFile.cp,v 1.3.10.1 1997/12/05 22:15:23 mhw Exp $
____________________________________________________________________________*/

#include "StSaveCurResFile.h"


StSaveCurResFile::StSaveCurResFile(
	Boolean	inResLoad)
{
	Initialize(inResLoad);
}



StSaveCurResFile::StSaveCurResFile(
	SInt16	inRefNum,
	Boolean	inResLoad)
{
	Initialize(inResLoad);
	::UseResFile(inRefNum);
}



StSaveCurResFile::StSaveCurResFile(
	FSSpec	inSpec,
	SInt16	inPerm,
	Boolean	inResLoad)
		: mOpenedFile(inSpec)
{
	Initialize(inResLoad);
	mOpenedFile.OpenResourceFork(inPerm);
}
					


StSaveCurResFile::~StSaveCurResFile()
{
	::UseResFile(mSavedResFile);
	::LMSetResLoad(mResLoad);
}



	void
StSaveCurResFile::Initialize(
	Boolean	inResLoad)
{
	mSavedResFile = ::CurResFile();
	mResLoad = ::LMGetResLoad();
	::SetResLoad(inResLoad);
}

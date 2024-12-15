/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: StSaveCurResFile.h,v 1.3.10.1 1997/12/05 22:15:24 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "LFile.h"

class StSaveCurResFile {
public:
				StSaveCurResFile(Boolean inResLoad = true);
				StSaveCurResFile(SInt16 inRefNum, Boolean inResLoad = true);
				StSaveCurResFile(FSSpec inSpec, SInt16 inPerm = fsRdPerm,
						Boolean inResLoad = true);
					
	virtual		~StSaveCurResFile();
	
protected:
	SInt8		mResLoad;
	SInt16		mSavedResFile;
	LFile		mOpenedFile;
	
	void		Initialize(Boolean inResLoad = true);
};

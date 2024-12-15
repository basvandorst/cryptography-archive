/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuPutScrapPatch.h,v 1.1.10.1 1997/12/05 22:14:20 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "MacCursors.h"
#include "PGPmenuGlobals.h"
#include "CPutScrapPatcher.h"

class CmenuPutScrapPatch : public CPutScrapPatcher {
public:
						CmenuPutScrapPatch(SGlobals & inGlobals);
	virtual				~CmenuPutScrapPatch();
	
protected:
	SGlobals &			mGlobals;
	
	virtual long		NewPutScrap(long length, ResType theType,
							void * source);
};

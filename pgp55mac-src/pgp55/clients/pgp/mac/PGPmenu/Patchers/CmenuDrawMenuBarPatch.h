/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuDrawMenuBarPatch.h,v 1.1.10.1 1997/12/05 22:14:15 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPmenuGlobals.h"
#include "CDrawMenuBarPatcher.h"

class CmenuDrawMenuBarPatch : public CDrawMenuBarPatcher {
public:
					CmenuDrawMenuBarPatch(SGlobals & inGlobals);
	virtual			~CmenuDrawMenuBarPatch();
	
protected:
	SGlobals &		mGlobals;
	
	virtual void	NewDrawMenuBar();
};

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuSetMenuBarPatch.h,v 1.1.10.1 1997/12/05 22:14:21 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPmenuGlobals.h"
#include "CSetMenuBarPatcher.h"

class CmenuSetMenuBarPatch : public CSetMenuBarPatcher {
public:
					CmenuSetMenuBarPatch(SGlobals & inGlobals);
	virtual			~CmenuSetMenuBarPatch();
	
protected:
	SGlobals &		mGlobals;
	
	virtual void	NewSetMenuBar(Handle menuList);
};

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuInsertMenuPatch.h,v 1.1.10.1 1997/12/05 22:14:17 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPMenuGlobals.h"
#include "CInsertMenuPatcher.h"

class CmenuInsertMenuPatch : public CInsertMenuPatcher {
public:
					CmenuInsertMenuPatch(SGlobals & inGlobals);
	virtual			~CmenuInsertMenuPatch();
	
protected:
	SGlobals &		mGlobals;
	
	virtual void	NewInsertMenu(MenuRef theMenu, short beforeID);
};

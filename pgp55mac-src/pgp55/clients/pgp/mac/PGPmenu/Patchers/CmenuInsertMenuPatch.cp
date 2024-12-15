/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuInsertMenuPatch.cp,v 1.2.10.1 1997/12/05 22:14:17 mhw Exp $
____________________________________________________________________________*/

#include "CmenuInsertMenuPatch.h"



CmenuInsertMenuPatch::CmenuInsertMenuPatch(
	SGlobals &	inGlobals)
	: mGlobals(inGlobals)
{
}



CmenuInsertMenuPatch::~CmenuInsertMenuPatch()
{
}
	


	void
CmenuInsertMenuPatch::NewInsertMenu(
	MenuRef	theMenu,
	short	beforeID)
{
	try {
		// Keep us at the end
		if ((::GetMenuHandle(MENU_PGPmenu) != nil) && (beforeID >= 0)
		&& (::GetMenuHandle(beforeID) == nil)) {
			beforeID = MENU_PGPmenu;
		}

		// Note that we have been called so that the next DrawMenuBar will
		// insert our menu
		mGlobals.insertMenu = true;
	}
	
	catch (...) {
	}
	
	OldInsertMenu(	theMenu,
					beforeID);
}

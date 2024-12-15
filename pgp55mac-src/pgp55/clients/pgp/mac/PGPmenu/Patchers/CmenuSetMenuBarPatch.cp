/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuSetMenuBarPatch.cp,v 1.2.10.1 1997/12/05 22:14:21 mhw Exp $
____________________________________________________________________________*/

#include "CmenuSetMenuBarPatch.h"



CmenuSetMenuBarPatch::CmenuSetMenuBarPatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals)
{
}



CmenuSetMenuBarPatch::~CmenuSetMenuBarPatch()
{
}



	void
CmenuSetMenuBarPatch::NewSetMenuBar(
	Handle	menuList)
{
	// Set flag so that we will insert our menu on the next DrawMenuBar
	mGlobals.insertMenu = true;
	
	OldSetMenuBar(menuList);
}

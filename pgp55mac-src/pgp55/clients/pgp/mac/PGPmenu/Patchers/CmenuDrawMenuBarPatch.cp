/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuDrawMenuBarPatch.cp,v 1.3.10.1 1997/12/05 22:14:14 mhw Exp $
____________________________________________________________________________*/

#include "MacIcons.h"

#include "CmenuDrawMenuBarPatch.h"



// types
#pragma options align=mac68k
struct MenuRec {
	MenuRef		menuRef;
	SInt16		menuLeft;
};

struct MenuList {
	SInt16	lastMenu;
	SInt16	lastRight;
	SInt16	notUsed;
	MenuRec	menus[];
};
#pragma options align=reset


CmenuDrawMenuBarPatch::CmenuDrawMenuBarPatch(
	SGlobals & inGlobals)
		: mGlobals(inGlobals)
{
	OSErr	err = noErr;

	mGlobals.menuH = ::GetMenu(MENU_PGPmenu);
	PGPThrowIfResFail_(mGlobals.menuH);
	::DetachResource((Handle) mGlobals.menuH);
	
	Handle	tempHandle;
	
	err = ::GetIconSuite(	&tempHandle,
							ICNx_PGPmenu,
							svAllSmallData);
	PGPThrowIfOSErr_(err);
	
	err = DuplicateIconSuite(	tempHandle,
								&mGlobals.menuIconH);
	PGPThrowIfOSErr_(err);
	::DisposeIconSuite(tempHandle, true);
	
	(**mGlobals.menuH).menuData[1] = 0x01;
	*((Handle *) &(**mGlobals.menuH).menuData[2]) = mGlobals.menuIconH;
	
#if BETA || PGP_DEMO
	if (mGlobals.disableEncryptAndSign) {
		::DisableItem(	mGlobals.menuH,
						kSign);
		::DisableItem(	mGlobals.menuH,
						kEncrypt);
		::DisableItem(	mGlobals.menuH,
						kEncryptAndSign);
	}
#endif
}



CmenuDrawMenuBarPatch::~CmenuDrawMenuBarPatch()
{
	if (mGlobals.menuH != nil) {
		::DisposeHandle((Handle) mGlobals.menuH);
	}
	if (mGlobals.menuIconH != nil) {
		::DisposeIconSuite(mGlobals.menuIconH, true);
	}
}



	void
CmenuDrawMenuBarPatch::NewDrawMenuBar()
{
	try {
		Handle			menuList = ::LMGetMenuList();
		char			firstMenuChar = 0;
		
// Initialize the appleevents manager for the finder menu
#if PGP_FINDERMENU
		static Boolean	initialized = false;
		
		if (! initialized) {
			UAppleEventsMgr::Initialize();
			initialized = true;
		}
#endif
		
		// Get the first character from the first menu
		if ((**(MenuList **) menuList).lastMenu != 0) {
			MenuRef	theMenu = (**(MenuList **) menuList).menus[0].menuRef;
			
			firstMenuChar = (**theMenu).menuData[1];
		}
		
		// Only insert our menu if the apple menu exists
		if (mGlobals.insertMenu && (firstMenuChar == appleMark)
		&& (::GetMenuHandle(MENU_PGPmenu) == nil)) {
			::InsertMenu(mGlobals.menuH, 0);
		}
	}
	
	catch (...) {
	}
	OldDrawMenuBar();
}

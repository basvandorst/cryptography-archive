/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CInsertMenuPatcher.cp,v 1.2.10.1 1997/12/05 22:14:49 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CInsertMenuPatcher.h"



// Define statics
CInsertMenuPatcher *	CInsertMenuPatcher::sInsertMenuPatcher = nil;

CInsertMenuPatcher::CInsertMenuPatcher()
{
	// Current version will only install one patch
	if (sInsertMenuPatcher == nil) {
		Patch(_InsertMenu, kProcInfo, (ProcPtr) InsertMenuCallback);
		sInsertMenuPatcher = this;
	}
}



CInsertMenuPatcher::~CInsertMenuPatcher()
{
}



	void
CInsertMenuPatcher::NewInsertMenu(
	MenuRef	theMenu,
	short	beforeID)
{
	// default just calls through to the original
	OldInsertMenu(theMenu, beforeID);
}



	void
CInsertMenuPatcher::OldInsertMenu(
	MenuRef	theMenu,
	short	beforeID)
{
#if GENERATINGCFM
	::CallUniversalProc(	mOldRoutine,
							kProcInfo,
							theMenu,
							beforeID);
#else
	((InsertMenuProc) mOldRoutine)(	theMenu,
									beforeID);
#endif
}



	pascal void
CInsertMenuPatcher::InsertMenuCallback(
	MenuRef	theMenu,
	short	beforeID)
{
	EnterCallback();

	try {
 		sInsertMenuPatcher->NewInsertMenu(theMenu, beforeID);
	}
	
	catch(...) {
	}
	
	ExitCallback();
}

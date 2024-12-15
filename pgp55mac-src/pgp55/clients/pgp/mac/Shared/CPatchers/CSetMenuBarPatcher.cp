/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSetMenuBarPatcher.cp,v 1.2.10.1 1997/12/05 22:14:55 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CSetMenuBarPatcher.h"



// Define statics
CSetMenuBarPatcher *	CSetMenuBarPatcher::sSetMenuBarPatcher = nil;

CSetMenuBarPatcher::CSetMenuBarPatcher()
{
	// Current version will only install one patch
	if (sSetMenuBarPatcher == nil) {
		Patch(_SetMenuBar, kProcInfo, (ProcPtr) SetMenuBarCallback);
		sSetMenuBarPatcher = this;
	}
}



CSetMenuBarPatcher::~CSetMenuBarPatcher()
{
}



void
CSetMenuBarPatcher::NewSetMenuBar(
	Handle	menuList)
{
	// default just calls through to the original
	OldSetMenuBar(menuList);
}



void
CSetMenuBarPatcher::OldSetMenuBar(
	Handle	menuList)
{
#if GENERATINGCFM
	::CallUniversalProc(	mOldRoutine,
							kProcInfo,
							menuList);
#else
	((SetMenuBarProc) mOldRoutine)(menuList);
#endif
}



pascal void
CSetMenuBarPatcher::SetMenuBarCallback(
	Handle	menuList)
{
	EnterCallback();

	try {
		sSetMenuBarPatcher->NewSetMenuBar(menuList);
	}
	
	catch(...) {
	}
	
	ExitCallback();
}

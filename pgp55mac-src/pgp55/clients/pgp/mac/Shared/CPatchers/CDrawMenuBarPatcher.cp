/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CDrawMenuBarPatcher.cp,v 1.2.10.1 1997/12/05 22:14:45 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CDrawMenuBarPatcher.h"



// Define statics
CDrawMenuBarPatcher *	CDrawMenuBarPatcher::sDrawMenuBarPatcher = nil;

CDrawMenuBarPatcher::CDrawMenuBarPatcher()
{
	// Current version will only install one patch
	if (sDrawMenuBarPatcher == nil) {
		Patch(_DrawMenuBar, kProcInfo, (ProcPtr) DrawMenuBarCallback);
		sDrawMenuBarPatcher = this;
	}
}



CDrawMenuBarPatcher::~CDrawMenuBarPatcher()
{
}



	void
CDrawMenuBarPatcher::NewDrawMenuBar()
{
	// default just calls through to the original
	OldDrawMenuBar();
}



	void
CDrawMenuBarPatcher::OldDrawMenuBar()
{
#if GENERATINGCFM
	::CallUniversalProc(	mOldRoutine,
							kProcInfo);
#else
	((DrawMenuBarProc) mOldRoutine)();
#endif
}



	pascal void
CDrawMenuBarPatcher::DrawMenuBarCallback()
{
	EnterCallback();

	try {	
		sDrawMenuBarPatcher->NewDrawMenuBar();
	}
	
	catch(...) {
	}
	
	ExitCallback();
}

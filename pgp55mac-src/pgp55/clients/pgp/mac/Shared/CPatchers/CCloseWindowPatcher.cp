/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CCloseWindowPatcher.cp,v 1.2.10.1 1997/12/05 22:14:43 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CCloseWindowPatcher.h"



// Define statics
CCloseWindowPatcher *	CCloseWindowPatcher::sCloseWindowPatcher = nil;

CCloseWindowPatcher::CCloseWindowPatcher()
{
	// Current version will only install one patch
	if (sCloseWindowPatcher == nil) {
		Patch(_CloseWindow, kProcInfo, (ProcPtr) CloseWindowCallback);
		sCloseWindowPatcher = this;
	}
}



CCloseWindowPatcher::~CCloseWindowPatcher()
{
}



	void
CCloseWindowPatcher::NewCloseWindow(
	WindowRef			theWindow)
{
	// default just calls through to the original
	OldCloseWindow(theWindow);
}



	void
CCloseWindowPatcher::OldCloseWindow(
	WindowRef			theWindow)
{
#if GENERATINGCFM
	::CallUniversalProc(	mOldRoutine,
							kProcInfo,
							theWindow);
#else
	((CloseWindowProc) mOldRoutine)(theWindow);
#endif
}



	pascal void
CCloseWindowPatcher::CloseWindowCallback(
	WindowRef			theWindow)
{
	EnterCallback();

	try {
		sCloseWindowPatcher->NewCloseWindow(theWindow);
	}
	
	catch(...) {
	}
	
	ExitCallback();
}

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CEndUpdatePatcher.cp,v 1.2.10.1 1997/12/05 22:14:46 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CEndUpdatePatcher.h"



// Define statics
CEndUpdatePatcher *	CEndUpdatePatcher::sEndUpdatePatcher = nil;

CEndUpdatePatcher::CEndUpdatePatcher()
{
	// Current version will only install one patch
	if (sEndUpdatePatcher == nil) {
		Patch(_EndUpDate, kProcInfo, (ProcPtr) EndUpdateCallback);
		sEndUpdatePatcher = this;
	}
}



CEndUpdatePatcher::~CEndUpdatePatcher()
{
}



	void
CEndUpdatePatcher::NewEndUpdate(
	WindowRef			theWindow)
{
	// default just calls through to the original
	OldEndUpdate(theWindow);
}



	void
CEndUpdatePatcher::OldEndUpdate(
	WindowRef			theWindow)
{
#if GENERATINGCFM
	::CallUniversalProc(	mOldRoutine,
							kProcInfo,
							theWindow);
#else
	((EndUpdateProc) mOldRoutine)(theWindow);
#endif
}



	pascal void
CEndUpdatePatcher::EndUpdateCallback(
	WindowRef			theWindow)
{
	EnterCallback();
	

	try {
		sEndUpdatePatcher->NewEndUpdate(theWindow);
	}
	
	catch(...) {
	}
	
	ExitCallback();
}

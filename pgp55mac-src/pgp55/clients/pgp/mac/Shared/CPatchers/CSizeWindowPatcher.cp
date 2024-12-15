/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSizeWindowPatcher.cp,v 1.2.10.1 1997/12/05 22:14:58 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CSizeWindowPatcher.h"



// Define statics
CSizeWindowPatcher *	CSizeWindowPatcher::sSizeWindowPatcher = nil;

CSizeWindowPatcher::CSizeWindowPatcher()
{
	// Current version will only install one patch
	if (sSizeWindowPatcher == nil) {
		Patch(_SizeWindow, kProcInfo, (ProcPtr) SizeWindowCallback);
		sSizeWindowPatcher = this;
	}
}



CSizeWindowPatcher::~CSizeWindowPatcher()
{
}



	void
CSizeWindowPatcher::NewSizeWindow(
	WindowRef	theWindow,
	short		w,
	short		h,
	Boolean		fUpdate)
{
	// default just calls through to the original
	OldSizeWindow(theWindow, w, h, fUpdate);
}



	void
CSizeWindowPatcher::OldSizeWindow(
	WindowRef	theWindow,
	short		w,
	short		h,
	Boolean		fUpdate)
{
#if	GENERATINGCFM
	::CallUniversalProc(	mOldRoutine,
							kProcInfo,
							theWindow,
							w,
							h,
							fUpdate);
#else
	((SizeWindowProc) mOldRoutine)(	theWindow,
									w,
									h,
									fUpdate);
#endif
}



	pascal void
CSizeWindowPatcher::SizeWindowCallback(
	WindowRef	theWindow,
	short		w,
	short		h,
	Boolean		fUpdate)
{
	EnterCallback();

	try {
		sSizeWindowPatcher->NewSizeWindow(theWindow, w, h, fUpdate);
	}
	
	catch(...) {
	}
	
	ExitCallback();
}

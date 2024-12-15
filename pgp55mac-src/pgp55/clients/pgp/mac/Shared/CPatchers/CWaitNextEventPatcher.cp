/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CWaitNextEventPatcher.cp,v 1.2.10.1 1997/12/05 22:15:01 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CWaitNextEventPatcher.h"



// Define statics
CWaitNextEventPatcher *	CWaitNextEventPatcher::sWaitNextEventPatcher = nil;

CWaitNextEventPatcher::CWaitNextEventPatcher()
{
	// Current version will only install one patch
	if (sWaitNextEventPatcher == nil) {
		Patch(_WaitNextEvent, kProcInfo, (ProcPtr) WaitNextEventCallback);
		sWaitNextEventPatcher = this;
	}
}



CWaitNextEventPatcher::~CWaitNextEventPatcher()
{
}



	Boolean
CWaitNextEventPatcher::NewWaitNextEvent(
	EventMask		eventMask,
	EventRecord *	theEvent,
	UInt32			sleep,
	RgnHandle		mouseRgn)
{
	// default just calls through to the original
	return OldWaitNextEvent(eventMask, theEvent, sleep, mouseRgn);
}



	Boolean
CWaitNextEventPatcher::OldWaitNextEvent(
	EventMask		eventMask,
	EventRecord *	theEvent,
	UInt32			sleep,
	RgnHandle		mouseRgn)
{
#if	GENERATINGCFM
	return (Boolean) ::CallUniversalProc(	mOldRoutine,
											kProcInfo,
											eventMask,
											theEvent,
											sleep,
											mouseRgn);
#else
	return ((WaitNextEventProc) mOldRoutine)(	eventMask,
												theEvent,
												sleep,
												mouseRgn);
#endif
}



	pascal Boolean
CWaitNextEventPatcher::WaitNextEventCallback(
	EventMask		eventMask,
	EventRecord *	theEvent,
	UInt32			sleep,
	RgnHandle		mouseRgn)
{
	EnterCallback();

	Boolean	result = false;

	try {
		result = sWaitNextEventPatcher->NewWaitNextEvent(	eventMask,
															theEvent,
															sleep,
															mouseRgn);
	}
	
	catch(...) {
	}
	
	ExitCallback();
	return result;
}

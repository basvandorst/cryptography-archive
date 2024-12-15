/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMenuSelectPatcher.cp,v 1.2.10.1 1997/12/05 22:14:50 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CMenuSelectPatcher.h"



// Define statics
CMenuSelectPatcher *	CMenuSelectPatcher::sMenuSelectPatcher = nil;

CMenuSelectPatcher::CMenuSelectPatcher()
{
	// Current version will only install one patch
	if (sMenuSelectPatcher == nil) {
		Patch(_MenuSelect, kProcInfo, (ProcPtr) MenuSelectCallback);
		sMenuSelectPatcher = this;
	}
}



CMenuSelectPatcher::~CMenuSelectPatcher()
{
}



	long
CMenuSelectPatcher::NewMenuSelect(
	Point	startPt)
{
	// default just calls through to the original
	return OldMenuSelect(startPt);
}



	long
CMenuSelectPatcher::OldMenuSelect(
	Point	startPt)
{
#if GENERATINGCFM
	return (long) ::CallUniversalProc(	mOldRoutine,
										kProcInfo,
										startPt);
#else
	return ((MenuSelectProc) mOldRoutine)(startPt);
#endif
}



	pascal long
CMenuSelectPatcher::MenuSelectCallback(
	Point	startPt)
{
	EnterCallback();

	long	result = 0;
	
	try {
		result = sMenuSelectPatcher->NewMenuSelect(startPt);
	}
	
	catch(...) {
	}
	
	ExitCallback();
	return result;
}

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CGetKeysPatcher.cp,v 1.2.10.1 1997/12/05 22:14:47 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CGetKeysPatcher.h"



// Define statics
CGetKeysPatcher *	CGetKeysPatcher::sGetKeysPatcher = nil;

CGetKeysPatcher::CGetKeysPatcher()
{
	// Current version will only install one patch
	if (sGetKeysPatcher == nil) {
		Patch(_GetKeys, kProcInfo, (ProcPtr) GetKeysCallback);
		sGetKeysPatcher = this;
	}
}



CGetKeysPatcher::~CGetKeysPatcher()
{
}



	void
CGetKeysPatcher::NewGetKeys(
	KeyMap	theKeys)
{
	// default just calls through to the original
	OldGetKeys(theKeys);
}



	void
CGetKeysPatcher::OldGetKeys(
	KeyMap	theKeys)
{
#if GENERATINGCFM
	::CallUniversalProc(	mOldRoutine,
							kProcInfo,
							theKeys);
#else
	((GetKeysProc) mOldRoutine)(theKeys);
#endif
}



	pascal void
CGetKeysPatcher::GetKeysCallback(
	KeyMap	theKeys)
{
	EnterCallback();

	try {
		sGetKeysPatcher->NewGetKeys(theKeys);
	}
	
	catch(...) {
	}
	
	ExitCallback();
}

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSetWTitlePatcher.cp,v 1.2.10.1 1997/12/05 22:14:57 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CSetWTitlePatcher.h"



// Define statics
CSetWTitlePatcher *	CSetWTitlePatcher::sSetWTitlePatcher = nil;

CSetWTitlePatcher::CSetWTitlePatcher()
{
	// Current version will only install one patch
	if (sSetWTitlePatcher == nil) {
		Patch(_SetWTitle, kProcInfo, (ProcPtr) SetWTitleCallback);
		sSetWTitlePatcher = this;
	}
}



CSetWTitlePatcher::~CSetWTitlePatcher()
{
}



	void
CSetWTitlePatcher::NewSetWTitle(
	WindowRef			theWindow,
	ConstStr255Param	title)
{
	// default just calls through to the original
	OldSetWTitle(theWindow, title);
}



	void
CSetWTitlePatcher::OldSetWTitle(
	WindowRef			theWindow,
	ConstStr255Param	title)
{
#if	GENERATINGCFM
	::CallUniversalProc(	mOldRoutine,
							kProcInfo,
							theWindow,
							title);
#else
	((SetWTitleProc) mOldRoutine)(	theWindow,
									title);
#endif
}



	pascal void
CSetWTitlePatcher::SetWTitleCallback(
	WindowRef			theWindow,
	ConstStr255Param	title)
{
	EnterCallback();

	try {
		sSetWTitlePatcher->NewSetWTitle(theWindow, title);
	}
	
	catch(...) {
	}
	
	ExitCallback();
}

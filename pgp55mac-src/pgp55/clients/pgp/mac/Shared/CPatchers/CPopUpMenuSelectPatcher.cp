/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPopUpMenuSelectPatcher.cp,v 1.2.10.1 1997/12/05 22:14:53 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CPopUpMenuSelectPatcher.h"



// Define statics
CPopUpMenuSelectPatcher *	CPopUpMenuSelectPatcher::sPopUpMenuSelectPatcher
									= nil;

CPopUpMenuSelectPatcher::CPopUpMenuSelectPatcher()
{
	// Current version will only install one patch
	if (sPopUpMenuSelectPatcher == nil) {
		Patch(_PopUpMenuSelect, kProcInfo, (ProcPtr) PopUpMenuSelectCallback);
		sPopUpMenuSelectPatcher = this;
	}
}



CPopUpMenuSelectPatcher::~CPopUpMenuSelectPatcher()
{
}



	long
CPopUpMenuSelectPatcher::NewPopUpMenuSelect(
	MenuRef	menu,
	short	top,
	short	left,
	short	popUpItem)
{
	// default just calls through to the original
	return OldPopUpMenuSelect(	menu,
								top,
								left,
								popUpItem);
}



	long
CPopUpMenuSelectPatcher::OldPopUpMenuSelect(
	MenuRef	menu,
	short	top,
	short	left,
	short	popUpItem)
{
#if GENERATINGCFM
	return (long) ::CallUniversalProc(	mOldRoutine,
										kProcInfo,
										menu,
										top,
										left,
										popUpItem);
#else
	return ((PopUpMenuSelectProc) mOldRoutine)(	menu,
												top,
												left,
												popUpItem);
#endif
}



	pascal long
CPopUpMenuSelectPatcher::PopUpMenuSelectCallback(
	MenuRef	menu,
	short	top,
	short	left,
	short	popUpItem)
{
	EnterCallback();

	long	result = 0;
	
	try {
		result = sPopUpMenuSelectPatcher->NewPopUpMenuSelect(	menu,
																top,
																left,
																popUpItem);
	}
	
	catch(...) {
	}
	
	ExitCallback();
	return result;
}

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerSizeWindowPatch.cp,v 1.2.10.1 1997/12/05 22:13:56 mhw Exp $
____________________________________________________________________________*/

#include "StSaveCurResFile.h"
#include "CemailerSizeWindowPatch.h"



CemailerSizeWindowPatch::CemailerSizeWindowPatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals)
{
}



CemailerSizeWindowPatch::~CemailerSizeWindowPatch()
{
}



	void
CemailerSizeWindowPatch::NewSizeWindow(
	WindowRef	theWindow,
	short		w,
	short		h,
	Boolean		fUpdate)
{
	OldSizeWindow(theWindow, w, h, fUpdate);

	try {
		ArrayIndexT	theIndex = mGlobals.windowRefArray.FetchIndexOfWindowRef(
														theWindow);

		// Remove the SwindowRef
		if (theIndex != LArray::index_Bad) {
			StSaveCurResFile	saveResFile;	
			StColorPortState	savePortState(theWindow);
			SWindowRef			theItem;

			if (mGlobals.windowRefArray.FetchSWindowRefAt(
			theIndex, theItem)) {
				theItem.grafPortView->ResizeFrameTo(	w,
														h,
														true);

				theItem.grafPortView->ResizeImageTo(	w,
														h,
														false);
			}
		}

	}
	
	catch (...) {
	}
}




/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerCloseWindowPatch.cp,v 1.2.10.1 1997/12/05 22:13:52 mhw Exp $
____________________________________________________________________________*/

#include "StSaveCurResFile.h"
#include "CemailerCloseWindowPatch.h"



CemailerCloseWindowPatch::CemailerCloseWindowPatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals)
{
}



CemailerCloseWindowPatch::~CemailerCloseWindowPatch()
{
}



	void
CemailerCloseWindowPatch::NewCloseWindow(
	WindowRef			theWindow)
{
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
				delete theItem.grafPortView;
				
				mGlobals.windowRefArray.RemoveSWindowRefAt(theIndex);
			}
		}

	}
	
	catch (...) {
	}
	
	OldCloseWindow(theWindow);
}




/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerEndUpdatePatch.cp,v 1.2.10.1 1997/12/05 22:13:53 mhw Exp $
____________________________________________________________________________*/

#include "StSaveCurResFile.h"
#include "CemailerEndUpdatePatch.h"



CemailerEndUpdatePatch::CemailerEndUpdatePatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals)
{
}



CemailerEndUpdatePatch::~CemailerEndUpdatePatch()
{
}



	void
CemailerEndUpdatePatch::NewEndUpdate(
	WindowRef			theWindow)
{
	try {
		ArrayIndexT	theIndex = mGlobals.windowRefArray.FetchIndexOfWindowRef(
														theWindow);

		// Redraw the GrafPortView
		if (theIndex != LArray::index_Bad) {
			StSaveCurResFile	saveResFile;	
			StColorPortState	savePortState(theWindow);
			SWindowRef			theItem;

			if (mGlobals.windowRefArray.FetchSWindowRefAt(
			theIndex, theItem)) {
				LFile	thePlugFile(mGlobals.plugSpec);
				
				thePlugFile.OpenResourceFork(fsRdPerm);
				theItem.grafPortView->Draw(nil);
				thePlugFile.CloseResourceFork();
			}
		}

	}
	
	catch (...) {
	}
	
	OldEndUpdate(theWindow);
}




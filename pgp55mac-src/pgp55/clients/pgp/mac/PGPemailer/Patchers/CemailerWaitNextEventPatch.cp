/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerWaitNextEventPatch.cp,v 1.2.10.1 1997/12/05 22:13:57 mhw Exp $
____________________________________________________________________________*/

#include "StSaveCurResFile.h"
#include "CemailerWaitNextEventPatch.h"



CemailerWaitNextEventPatch::CemailerWaitNextEventPatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals), mInitialized(false)
{
}



CemailerWaitNextEventPatch::~CemailerWaitNextEventPatch()
{
}
	


	void
CemailerWaitNextEventPatch::DispatchEvent()
{
	// Do initialization that depends upon the app being set up
	if (! mInitialized) {
		InitializePlug();
	}
	
	// Iterate through the window list activating and deactivating
	// GrafPortViews appropriately
	SWindowRef		theItem;
	LArrayIterator	iterator(mGlobals.windowRefArray);
	
	while (iterator.Next(&theItem)) {
		if (IsWindowHilited(theItem.windowRef)) {
			theItem.grafPortView->Activate();
		} else {
			theItem.grafPortView->Deactivate();
		}
	}

	// Call inherited
	CWaitNextEventDispatcher::DispatchEvent();
}



	void
CemailerWaitNextEventPatch::InitializePlug()
{
	// Initialize AppleEvents
	UAppleEventsMgr::Initialize();			

	mInitialized = true;
}



	void
CemailerWaitNextEventPatch::HandleMouseDown()
{
	WindowRef	theWindow;
	ArrayIndexT	theIndex;
	
	// Find the window
	::FindWindow(	sEventParams.theEvent->where,
					&theWindow);
	theIndex = mGlobals.windowRefArray.FetchIndexOfWindowRef(theWindow);
	if (theIndex != LArray::index_Bad) {
		StSaveCurResFile	saveResFile;
		StColorPortState	savePortState(theWindow);
		SWindowRef			theItem;
		
		if (mGlobals.windowRefArray.FetchSWindowRefAt(theIndex, theItem)) {
			LFile	thePlugFile(mGlobals.plugSpec);
			
			thePlugFile.OpenResourceFork(fsRdPerm);
			theItem.grafPortView->ClickInContent(*sEventParams.theEvent);
			thePlugFile.CloseResourceFork();
		}
	}
}

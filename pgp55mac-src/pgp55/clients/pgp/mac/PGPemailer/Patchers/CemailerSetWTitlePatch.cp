/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerSetWTitlePatch.cp,v 1.3.10.1 1997/12/05 22:13:54 mhw Exp $
____________________________________________________________________________*/

#include <string.h>

#include "StSaveCurResFile.h"
#include "CPGPemailerView.h"
#include "CemailerSetWTitlePatch.h"



// Constants
const ResIDT	STRx_OutgoingMessage	=	1025;
const SInt16	kOutgoingMessageID		=	12;

const ResIDT	STRx_IncomingMessage	=	2200;
const SInt16	kIncomingMessageID		=	1;

const SInt16	PPob_OutgoingMessages	=	1000;
const SInt16	PPob_IncomingMessages	=	2000;


CemailerSetWTitlePatch::CemailerSetWTitlePatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals)
{
	StSaveCurResFile	switchToApp(mGlobals.appResForkRef);
	
	::GetIndString(	mOutgoingMessageString,
					STRx_OutgoingMessage,
					kOutgoingMessageID);
	::GetIndString(	mIncomingMessageString,
					STRx_IncomingMessage,
					kIncomingMessageID);
}



CemailerSetWTitlePatch::~CemailerSetWTitlePatch()
{
}
	


	void
CemailerSetWTitlePatch::NewSetWTitle(
	WindowRef			theWindow,
	ConstStr255Param	title)
{
	try {
		SInt16	thePPob	=	0;
	
		// Check to see if the window needs to be captured
		if (mGlobals.windowRefArray.FetchIndexOfWindowRef(theWindow)
		== LArray::index_Bad) {
			if (memcmp(&title[1], &mOutgoingMessageString[1],
			mOutgoingMessageString[0]) == 0) {
#if BETA || PGP_DEMO
				if (! mGlobals.disableEncryptAndSign) {
#endif
					thePPob = PPob_OutgoingMessages;
#if BETA || PGP_DEMO
				}
#endif
			} else if (memcmp(&title[1], &mIncomingMessageString[1],
			mIncomingMessageString[0]) == 0) {
				thePPob = PPob_IncomingMessages;
			}
		}
		
		// Capture the window
		if (thePPob != 0) {
			StColorPortState	savePortState(theWindow);
			StSaveCurResFile	saveCurResFile();
			SWindowRef			theItem = {theWindow, nil};
			LFile				thePlug(mGlobals.plugSpec);
			
			// Create the view
			LPane::SetDefaultView(nil);
			LView::OutOfFocus(nil);
			thePlug.OpenResourceFork(fsRdPerm);
			theItem.grafPortView = dynamic_cast<LGrafPortView *>
					(LGrafPortView::CreateGrafPortView(	thePPob,
														nil));
			PGPThrowIfNil_(theItem.grafPortView);
			UReanimator::LinkListenerToControls(
						dynamic_cast<CPGPemailerView *>(theItem.grafPortView),
						theItem.grafPortView,
						thePPob);
													
			dynamic_cast<CPGPemailerView *>(theItem.grafPortView)->SetGlobals(
				&mGlobals);
			thePlug.CloseResourceFork();
			
			// Deactivate view if we are not the front app
			ProcessSerialNumber	theFrontProcess = {0, 0};
			ProcessSerialNumber	theCurrentProcess = {0, 0};
			Boolean				sameProcess;
			
			::GetFrontProcess(&theFrontProcess);
			::GetCurrentProcess(&theCurrentProcess);
			::SameProcess(	&theFrontProcess,
							&theCurrentProcess,
							&sameProcess);
			if (!sameProcess) {
				theItem.grafPortView->Deactivate();
			}


			// Add the window to the windowrefarray
			mGlobals.windowRefArray.InsertSWindowRef(theItem);
		}
	}
	
	catch (...) {
	}
	
	OldSetWTitle(theWindow, title);
}

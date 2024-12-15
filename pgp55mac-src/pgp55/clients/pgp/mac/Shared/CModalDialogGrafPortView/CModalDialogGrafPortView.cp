/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CModalDialogGrafPortView.cp,v 1.4.10.1 1997/12/05 22:14:37 mhw Exp $
____________________________________________________________________________*/

#include "UPGPException.h"
#include "CModalDialogGrafPortView.h"



/*____________________________________________________________________________

	Notes:				The pointer to the object is stored in the dialog's
						refcon.

						The visibility of the DLOG and the GrafPortView must
						match in order for this to work correctly correctly.
____________________________________________________________________________*/
CModalDialogGrafPortView *
CModalDialogGrafPortView::CreateCModalDialogGrafPortView(
	ResIDT			inGrafPortView,
	LCommander *	inSuperCommander)
{
	StColorPortState			theSavePort(UQDGlobals::GetCurrentPort());
	GrafPtr						savedPort;
	DialogPtr					theDialog;
	CModalDialogGrafPortView *	theModalDialogGrafPortView;
	
	::GetPort(&savedPort);
	
	// Create the dialog and make it the current port
	theDialog = ::GetNewDialog(	inGrafPortView,
								nil,
								(WindowRef) -1);
	PGPThrowIfNil_(theDialog);
	
	::SetPort(theDialog);
	
	// Create the grafport view
	theModalDialogGrafPortView = (CModalDialogGrafPortView *)
									CreateGrafPortView(	inGrafPortView,
														inSuperCommander);
	PGPThrowIfNil_(theModalDialogGrafPortView);
	
	theModalDialogGrafPortView->mSavedPort = savedPort;
	
	// Adjust current colors
	::GetForeColor(&theModalDialogGrafPortView->mForeColor);
	::GetBackColor(&theModalDialogGrafPortView->mBackColor);
	
	// Store the object in the refcon for further use
	::SetWRefCon(theDialog, (SInt32) theModalDialogGrafPortView);
	
	return theModalDialogGrafPortView;
}
	


CModalDialogGrafPortView::CModalDialogGrafPortView(
	LStream *	inStream)
		: LGrafPortView(inStream), mUpdateWindows(sizeof(WindowRef))
{
	mFilterProc = nil;
}



CModalDialogGrafPortView::~CModalDialogGrafPortView()
{
	GrafPtr		finalPort	=	UQDGlobals::GetCurrentPort();
	
	OutOfFocus(nil);

	// Dispose of the dialog
	if (GetMacPort() != nil) {
		if (finalPort == GetMacPort()) {
			finalPort = mSavedPort;
		}
		DeleteAllSubPanes();
		::DisposeDialog(GetMacPort());
	}
	
	// Invalidate the windows that we captured Update events for
	LArrayIterator		iterator(mUpdateWindows);
	WindowRef			theWindow;

	while (iterator.Next(&theWindow)) {
		::SetPort(theWindow);
		::InvalRect(&theWindow->portRect);
	}
	::SetPort(finalPort);
}



	void
CModalDialogGrafPortView::DrawSelf()
{
	ApplyForeAndBackColors();
	
	LGrafPortView::DrawSelf();
}



	void
CModalDialogGrafPortView::Run(
	ModalFilterProcPtr	inFilterProc)
{
	mFilterProc = inFilterProc;
	
	SInt16			itemHit;
	ModalFilterUPP	filterUPP = NewModalFilterProc(ModalDialogFilterProc);
	
	PGPThrowIfNil_(filterUPP);
	mError.err = CComboError::kNoErr;
	mError.pgpErr = kPGPError_NoErr;
	do {
		::ModalDialog(filterUPP, &itemHit);
	} while (itemHit == 0);
	
	::DisposeRoutineDescriptor(filterUPP);
	if (mError.IsError()) {
		PGPThrowComboErr_(mError);
	}
}



	pascal Boolean
CModalDialogGrafPortView::ModalDialogFilterProc(
	DialogPtr		theDialog, 
	EventRecord *	theEvent,
	SInt16 *		itemHit)
{
	Boolean						result = true;
	CModalDialogGrafPortView *	theGrafPortView =
			(CModalDialogGrafPortView *) ::GetWRefCon(theDialog);

	try {
		StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());

		OutOfFocus(nil);
		*itemHit = 1;
		if ((theGrafPortView->mFilterProc == nil)
		|| (theGrafPortView->mFilterProc(theDialog, theEvent, itemHit))) {
			// Get the grafportview
			
			theGrafPortView->mExitDialog = false;
			
			// Give time to repeaters
			LPeriodical::DevoteTimeToRepeaters(*theEvent);
			switch (theEvent->what) {
				case mouseDown:
				{
					WindowPtr	theWindow;
					SInt16		thePart = ::FindWindow(	theEvent->where,
														&theWindow);
					
					switch (thePart) {
						case inContent:
						{
							if (theWindow == theGrafPortView->GetMacPort()) {
								::SetPort(theGrafPortView->GetMacPort());
								theGrafPortView->ClickInContent(*theEvent);
							}
						}
						break;
					}
				}
				break;
						
						
				case autoKey:
				case keyDown:
				{
					LCommander *	theTarget = LCommander::GetTarget();
					
					if (theTarget != nil) {
						theTarget->ProcessKeyPress(*theEvent);
					}
				}
				break;
				
				
				case updateEvt:
				{
					::BeginUpdate(((WindowPtr) theEvent->message));
					if (((WindowPtr) theEvent->message)
					== theGrafPortView->GetMacPort()) {
						theGrafPortView->Draw(nil);
					} else if (theGrafPortView->mUpdateWindows.FetchIndexOf(
					&theEvent->message) == LArray::index_Bad) {
						theGrafPortView->
								mUpdateWindows.InsertItemsAt(1,
										LArray::index_Last,
										&theEvent->message);
					}
					::EndUpdate(((WindowPtr) theEvent->message));
				}
				break;
				
				
				case activateEvt:
				{
					if (((WindowPtr) theEvent->message)
					== theGrafPortView->GetMacPort()) {
						if (theEvent->modifiers & activeFlag) {
							theGrafPortView->Activate();
						} else {
							theGrafPortView->Deactivate();
						}
					}
				}
				break;
				
				
				case nullEvent:
				{
					LPeriodical::DevoteTimeToIdlers(*theEvent);
				}
				break;
					
			}
			
			// Check to see if we need to exit modal dialog
			if (! theGrafPortView->mExitDialog) {
				*itemHit = 0;
			}
		}
	}
	
	catch (CComboError & comboError) {
		theGrafPortView->mError = comboError;
	}
	
	catch (...) {
		theGrafPortView->mError.pgpErr = kPGPError_UnknownError;
	}
	
	return result;
}



	void
CModalDialogGrafPortView::Show()
{
	::ShowWindow(mGrafPtr);
	if (mVisible == triState_Off) {
		StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());
	
		OutOfFocus(nil);
		mVisible = triState_On;
		ShowSelf();
		LView::Show();
	}
}

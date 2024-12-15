/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COutputDialog.cp,v 1.3.10.1 1997/12/05 22:14:26 mhw Exp $
____________________________________________________________________________*/

#include <LGAPushButton.h>

#include "COutputDialog.h"

// Constants
const PaneIDT	pane_Text					=	1000;
const PaneIDT	pane_OKButton				=	1001;
const PaneIDT	pane_CopyToClipboardButton	=	1002;

const MessageT	msg_CopyToClipboard			=	'Copy';



COutputDialog::COutputDialog(
	LStream *	inStream)
		: CModalDialogGrafPortView(inStream), mCopyToClipboard(false)
{
	::InitCursor();
}



COutputDialog::~COutputDialog()
{
}



	void
COutputDialog::FinishCreateSelf()
{
	LGAPushButton *	button = (LGAPushButton *) FindPaneByID(pane_OKButton);
	
	button->SetDefaultButton(true);
	button->AddListener(this);
	
	button = (LGAPushButton *) FindPaneByID(pane_CopyToClipboardButton);
	button->AddListener(this);
}



	Boolean
COutputDialog::HandleKeyPress(
	const EventRecord &	inKeyEvent)
{
	StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());
	OutOfFocus(nil);

	Boolean			keyHandled = true;
	Int16			theKey = inKeyEvent.message & charCodeMask;
	LGAPushButton *	okButton = (LGAPushButton *) FindPaneByID(pane_OKButton);
	
	if((inKeyEvent.modifiers & cmdKey) == 0) {
		switch (theKey) {
			case char_Return:
			case char_Enter:
			{
				okButton->SimulateHotSpotClick(kControlButtonPart);
			}
			break;
			
			
			default:
			{
				keyHandled = CModalDialogGrafPortView::HandleKeyPress(
								inKeyEvent);
			}
			break;
		}
	} else {
		keyHandled = CModalDialogGrafPortView::HandleKeyPress(inKeyEvent);
	}
	
	return keyHandled;
}



	void
COutputDialog::ListenToMessage(
	MessageT	inMessage,
	void *		ioParam)
{
	switch (inMessage) {
		case msg_OK:
		{
			mExitDialog = true;
		}
		break;
		
		
		case msg_CopyToClipboard:
		{
			mCopyToClipboard = true;
		}
		break;
	}
}


	void
COutputDialog::SetText(
	Handle	inTextH)
{
	StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());
	OutOfFocus(nil);
	
	LTextEdit *	theText = (LTextEdit *) FindPaneByID(pane_Text);

	theText->SetTextHandle(inTextH);
}

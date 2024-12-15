/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: COutputDialog.cp,v 1.6.8.1 1998/11/12 03:08:49 heller Exp $____________________________________________________________________________*/#include <UDrawingState.h>#include <PP_KeyCodes.h>#include <PP_Messages.h>#include <LTextEditView.h>#include <LPushButton.h>#include "COutputDialog.h"// Constantsconst PaneIDT	pane_Text					=	1000;const PaneIDT	pane_OKButton				=	1001;const PaneIDT	pane_CopyToClipboardButton	=	1002;const MessageT	msg_CopyToClipboard			=	'Copy';COutputDialog::COutputDialog(	LStream *	inStream)		: CModalDialogGrafPortView(inStream), mCopyToClipboard(false){	::InitCursor();}COutputDialog::~COutputDialog(){}	voidCOutputDialog::FinishCreateSelf(){	LPushButton *	button = (LPushButton *) FindPaneByID(pane_OKButton);		button->AddListener(this);		button = (LPushButton *) FindPaneByID(pane_CopyToClipboardButton);	button->AddListener(this);}	BooleanCOutputDialog::HandleKeyPress(	const EventRecord &	inKeyEvent){	StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());	OutOfFocus(nil);	Boolean			keyHandled = true;	Int16			theKey = inKeyEvent.message & charCodeMask;	LPushButton *	okButton = (LPushButton *) FindPaneByID(pane_OKButton);		if((inKeyEvent.modifiers & cmdKey) == 0) {		switch (theKey) {			case char_Return:			case char_Enter:			{				okButton->SimulateHotSpotClick(kControlButtonPart);			}			break;									default:			{				keyHandled = CModalDialogGrafPortView::HandleKeyPress(								inKeyEvent);			}			break;		}	} else {		keyHandled = CModalDialogGrafPortView::HandleKeyPress(inKeyEvent);	}		return keyHandled;}	voidCOutputDialog::ListenToMessage(	MessageT	inMessage,	void *		ioParam){	(void) ioParam;		switch (inMessage) {		case msg_OK:		{			mExitDialog = true;		}		break;						case msg_CopyToClipboard:		{			mCopyToClipboard = true;		}		break;	}}	voidCOutputDialog::SetText(	Handle	inTextH){	StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());	OutOfFocus(nil);		LTextEditView *	theText = (LTextEditView *) FindPaneByID(pane_Text);	theText->SetTextHandle(inTextH);}
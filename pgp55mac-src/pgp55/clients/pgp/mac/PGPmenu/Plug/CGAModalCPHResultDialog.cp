/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CGAModalCPHResultDialog.cp,v 1.2.10.1 1997/12/05 22:14:25 mhw Exp $
____________________________________________________________________________*/

#include <UGraphicsUtilities.h>
#include <UGAColorRamp.h>
#include <LGAPushButton.h>

#include "MacQuickdraw.h"

#include "CString.h"

#include "PGPMenuGlobals.h"
#include "CPGPmenu.h"
#include "CGAModalCPHResultDialog.h"

// Constants
const PaneIDT	kOKButtonPaneID			=	'bOK ';
const PaneIDT	kStatusPaneID			=	'Stat';
const PaneIDT	kSignerPaneID			=	'Sign';
const PaneIDT	kDatePaneID				=	'Date';
const PaneIDT	kValidityPaneID			=	'Vald';



CGAModalCPHResultDialog::CGAModalCPHResultDialog(
	LStream *	inStream)
		: CModalDialogGrafPortView(inStream), mValidSignature(false)
{
	::InitCursor();
}



CGAModalCPHResultDialog::~CGAModalCPHResultDialog()
{
}



	void
CGAModalCPHResultDialog::FinishCreateSelf()
{
	LGAPushButton *	okButton = (LGAPushButton *) FindPaneByID(kOKButtonPaneID);
	
	okButton->SetDefaultButton(true);
	okButton->AddListener(this);
}



	Boolean
CGAModalCPHResultDialog::HandleKeyPress(
	const EventRecord &	inKeyEvent)
{
	StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());

	Boolean			keyHandled = true;
	Int16			theKey = inKeyEvent.message & charCodeMask;
	LGAPushButton *	okButton = (LGAPushButton *) FindPaneByID(kOKButtonPaneID);
	
	OutOfFocus(nil);

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
CGAModalCPHResultDialog::ListenToMessage(
	MessageT	inMessage,
	void *		ioParam)
{
	switch (inMessage) {
		case msg_OK:
		{
			mExitDialog = true;
		}
		break;
	}
}


	void
CGAModalCPHResultDialog::SetStatusMessage(
	SStatusMessage &	inStatusMessage)
{
	StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());
	OutOfFocus(nil);

	PGPResultStatus	resultStatus;
	LCaption *		statusCaption	= (LCaption *) FindPaneByID(kStatusPaneID);
	LCaption *		signerCaption	= (LCaption *) FindPaneByID(kSignerPaneID);
	LCaption *		dateCaption		= (LCaption *) FindPaneByID(kDatePaneID);
	Str31			dateString;
	Str255			statusText;
	
	ThrowIfNil_(statusCaption);
	ThrowIfNil_(signerCaption);
	ThrowIfNil_(dateCaption);
	
	CPGPmenuPlug::GetResultStatusAndValidity(	&inStatusMessage.sigInfo,
												&resultStatus,
												&mValidity);
	CPGPmenuPlug::GetResultStatusString(	resultStatus,
											CString(	STRx_ResultsStrings,
														kVerifiedID),
											statusText);
	statusCaption->SetDescriptor(statusText);
	signerCaption->SetDescriptor(CString(inStatusMessage.sigInfo.userID));
	
	::IUDateString(	inStatusMessage.sigInfo.signingDate,
					shortDate,
					dateString);
	dateCaption->SetDescriptor(dateString);
	
	mValidSignature = true;
	switch(resultStatus) {
		case kPGPResultStatusInvalid:
		case kPGPResultStatusNotEncryptedOrSigned:
		{
			mValidSignature = false;
		}
		break;
		
			
		case kPGPResultStatusNotSigned:
		{
			mValidSignature = false;
		}
		break;
		
		case kPGPResultStatusNotVerified:
		{
			mValidSignature = false;
		}
		break;
			
		case kPGPResultStatusBadSignature:
		{
			mValidSignature = false;
		}
		break;
	}
}



	void
CGAModalCPHResultDialog::DrawSelf()
{
	if (mValidSignature) {
		LPane *			validityPane	=	(LPane *) FindPaneByID(
														kValidityPaneID);
		RGBColor		validityColor = { 0xCCCC, 0xCCCC, 0xFFFF };
		Rect			validityRect;
		Rect			frameRect;
		
		validityPane->CalcLocalFrameRect(frameRect);
		validityRect = frameRect;
		validityRect.bottom = validityRect.top + kValidityBarHeight;
		AlignRect(&frameRect, &validityRect, kAlignAtVerticalCenter );
		
		::RGBForeColor( &UGAColorRamp::GetBlackColor() );
		::FrameRect( &validityRect );
		::InsetRect( &validityRect, 1, 1 );

		::RGBForeColor( &validityColor );
		::PaintRect( &validityRect );
		
		validityRect.right = validityRect.left
					+ (( UGraphicsUtilities::RectWidth( validityRect ) * 
					mValidity ) / kMaxValidity );
					
		::RGBForeColor( &UGAColorRamp::GetColor( colorRamp_Gray9 ) );
		::PaintRect( &validityRect );

		::RGBForeColor( &UGAColorRamp::GetBlackColor() );
	}
}

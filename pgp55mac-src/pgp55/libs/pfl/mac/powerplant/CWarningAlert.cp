/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CWarningAlert.cp,v 1.7 1997/09/18 22:54:50 heller Exp $
____________________________________________________________________________*/
#include "pgpMem.h"

#include <LCaption.h>
#include <LControl.h>
#include <LIconPane.h>
#include <PP_Messages.h>
#include <UModalDialogs.h>
#include <UWindows.h>
#include "CPGPStDialogHandler.h"

#include "MacStrings.h"

#include "CWarningAlert.h"

const ResIDT	kWarningAlertStringsResID	= 31500;

const ResIDT	kStopIconResID			= 31500;
const ResIDT	kNoteIconResID			= 31501;
const ResIDT	kCautionIconResID		= 31502;

const PaneIDT	kOKButtonPaneID			= 31501;
const PaneIDT	kCancelButtonPaneID 	= 31502;
const PaneIDT	kMessageCaptionPaneID 	= 31503;
const PaneIDT	kIconPaneID 			= 31504;

enum
{
	kOKStrIndex	= 1,
	kCancelStrIndex,
	kYesStrIndex,
	kNoStrIndex
};



	static short
sGetIconForAlertType( WarningAlertType 	alertType )
{
	ResIDT		iconID;

	switch( alertType )
	{
		default:
			pgpDebugMsg( "WarningAlertString: Invalid alert type" );
			/* fall thru */
		case kWANoteAlertType:
			iconID = kNoteIconResID;
			break;

		case kWACautionAlertType:
			iconID = kCautionIconResID;
			break;

		case kWAStopAlertType:
			iconID = kStopIconResID;
			break;
	}

	return( iconID );
}


	Boolean
CWarningAlert::SetupDialog(
	WarningAlertType 	alertType,
	WarningAlertStyle 	alertStyle,
	ConstStr255Param 	baseMessage,
	ConstStr255Param	str1,
	ConstStr255Param	str2,
	ConstStr255Param	str3 )
{
	Boolean		valid = FALSE;

	pgpAssertAddrValid( this, CWarningAlert );
	pgpAssertAddrValid( baseMessage, uchar );

	if( alertType != kWACustomAlertType )
	{
		SetIcon( sGetIconForAlertType( alertType ) );
	}

	if( alertStyle != kWACustomAlertStyle )
	{
		short		okButtonStrIndex;
		short		cancelButtonStrIndex;
		Boolean		hideCancelButton;
		PaneIDT		defaultButtonPaneID;
		PaneIDT		cancelButtonPaneID;
		Str255		tempStr;

		okButtonStrIndex 		= kOKStrIndex;
		cancelButtonStrIndex	= kCancelStrIndex;
		hideCancelButton		= FALSE;
		defaultButtonPaneID		= kOKButtonPaneID;
		cancelButtonPaneID		= kCancelButtonPaneID;

		switch( alertStyle )
		{
			default:
				pgpDebugMsg( "WarningAlertString: Invalid alert style" );
				/* fall thru */
			case kWAOKStyle:
				hideCancelButton = TRUE;
				break;

			case kWACancelStyle:
				okButtonStrIndex	= kCancelStrIndex;
				hideCancelButton 	= TRUE;
				break;

			case kWAOKCancelStyle:
				// Nothing to do.
				break;

			case kWACancelOKStyle:
				defaultButtonPaneID = kCancelButtonPaneID;
				cancelButtonPaneID	= kOKButtonPaneID;
				break;

			case kWAYesNoStyle:
				okButtonStrIndex 		= kYesStrIndex;
				cancelButtonStrIndex	= kNoStrIndex;
				break;

			case kWANoYesStyle:
				defaultButtonPaneID 	= kCancelButtonPaneID;
				cancelButtonPaneID		= kOKButtonPaneID;
				okButtonStrIndex 		= kNoStrIndex;
				cancelButtonStrIndex	= kYesStrIndex;
				break;
		}

		GetIndString( tempStr, kWarningAlertStringsResID, okButtonStrIndex );
		SetButtonTitle( kOKButtonPaneID, tempStr );

		if( hideCancelButton )
		{
			HideCancelButton();
		}
		else
		{
			GetIndString( tempStr, kWarningAlertStringsResID,
				cancelButtonStrIndex );
			SetButtonTitle( kCancelButtonPaneID, tempStr );
		}

		if( defaultButtonPaneID != kOKButtonPaneID )
		{
			SetDefaultButton( defaultButtonPaneID );
		}

		if( cancelButtonPaneID != kCancelButtonPaneID )
		{
			SetCancelButton( cancelButtonPaneID );
		}
	}

	Str255		realMessage;
	InsertPStrings( baseMessage, str1, str2, str3, realMessage );
	SetMessage( realMessage );

	valid = TRUE;

	return( valid );

}




	MessageT
CWarningAlert::DisplayCustom(
	short				dialogResID,
	WarningAlertType 	alertType,
	WarningAlertStyle 	alertStyle,
	ConstStr255Param	message,
	ConstStr255Param	str1,
	ConstStr255Param	str2,
	ConstStr255Param	str3)
{
	CPGPStDialogHandler	dialogHandler( dialogResID, nil );
	CWarningAlert	*dialogObj;
	MessageT		result;

	pgpAssertAddrValid( message, uchar );

	dialogObj = (CWarningAlert *) dialogHandler.GetDialog();
	if( IsNull( dialogObj ) )
	{
		pgpDebugMsg( "WarningAlertString: Nil dialog object" );
		return( msg_Cancel );
	}

	if( dialogObj->SetupDialog( alertType, alertStyle,
			message, str1, str2, str3 ) )
	{
		dialogObj->Show();

		while (TRUE)
		{
			result = dialogHandler.DoDialog();
			if( result > 0 )
				break;
		}

		dialogObj->Hide();
	}
	else
	{
		result = msg_Cancel;
	}

	return( result );
}



	MessageT
CWarningAlert::Display(
	WarningAlertType 	alertType,
	WarningAlertStyle 	alertStyle,
	ConstStr255Param	message,
	ConstStr255Param	str1,
	ConstStr255Param	str2,
	ConstStr255Param	str3)
{
	return( DisplayCustom( kWarningAlertDialogResID, alertType,
				alertStyle, message, str1, str2, str3 ) );
}



	MessageT
CWarningAlert::Display(
	WarningAlertType 	alertType,
	WarningAlertStyle 	alertStyle,
	ResIDT				stringListResID,
	short				stringListIndex,
	ConstStr255Param	str1,
	ConstStr255Param	str2,
	ConstStr255Param	str3)
{
	Str255	message;

	GetIndString( message, stringListResID, stringListIndex );
	pgpAssert( message[0] != 0 );

	return( Display( alertType, alertStyle, message, str1, str2, str3) );
}



CWarningAlert::CWarningAlert(LStream *inStream)
	: LGADialogBox(inStream)
{
}

CWarningAlert::~CWarningAlert()
{
}


	void
CWarningAlert::SetMessage(ConstStr255Param message)
{
	LCaption	*captionObj;

	pgpAssertAddrValid( message, uchar );

	captionObj = (LCaption *) FindPaneByID( kMessageCaptionPaneID );
	pgpAssertAddrValid( captionObj, LCaption );

	if( IsntNull( captionObj ) )
	{
		Rect		oldCaptionRect;
		Rect		newCaptionRect;
		Rect		textRect;
		FontInfo	fontInfo;
		short		lineHeight;
		TERec		**textTEH;
		short		numTextLines;
		short		heightDiff;

		// Resize the dialog vertically to account for large messages

		FocusDraw();

		captionObj->CalcPortFrameRect( oldCaptionRect );

		// Calculate the height of the text by overlaying a TERecord on
		// top of the caption
		// pane and calculating the number of lines of text as calculated
		// by TECalText().
		// Then delete the TERecord and set the size of the dialog
		// appropriately.

		textRect		= oldCaptionRect;
		textRect.bottom = 32767;
		numTextLines	= 0;

		TextFont( 0 );
		TextSize( 0 );

		textTEH = TENew( &textRect, &textRect );
		AssertHandleIsValid( textTEH, "SetMessage" );

		if( IsntNull( textTEH ) )
		{
			TESetText( &message[1], message[0], textTEH );
			TECalText( textTEH );

			numTextLines = (**textTEH).nLines;

			TEDispose( textTEH );
		}

		if( numTextLines < 3 )
			numTextLines = 3;

		GetFontInfo( &fontInfo );
		lineHeight = fontInfo.leading + fontInfo.ascent + fontInfo.descent;

		newCaptionRect 			= oldCaptionRect;
		newCaptionRect.bottom	= newCaptionRect.top +
			(numTextLines * lineHeight );

		heightDiff = newCaptionRect.bottom - oldCaptionRect.bottom;
		if( heightDiff != 0 )
		{
			Rect	windowRect;

			windowRect 			=
				UWindows::GetWindowContentRect( mMacWindowP );
			windowRect.bottom 	+= heightDiff;

			DoSetBounds( windowRect );
		}

		captionObj->SetDescriptor( message );
	}
}


	void
CWarningAlert::SetButtonTitle(PaneIDT buttonPaneID, ConstStr255Param
descriptor)
{
	LControl	*buttonObj;

	pgpAssertAddrValid( descriptor, uchar );

	buttonObj = (LControl *) FindPaneByID( buttonPaneID );
	pgpAssertAddrValid( buttonObj, LControl );

	if( IsntNull( buttonObj ) )
		buttonObj->SetDescriptor( descriptor );
}


	void
CWarningAlert::HideCancelButton(void)
{
	LControl	*controlObj;

	controlObj = (LControl *) FindPaneByID( kCancelButtonPaneID );
	pgpAssertAddrValid( controlObj, LControl );

	if( IsntNull( controlObj ) )
		controlObj->Hide();
}


	void
CWarningAlert::SetIcon(ResIDT iconResID)
{
	LIconPane	*iconObj;

	iconObj = (LIconPane *) FindPaneByID( kIconPaneID );
	pgpAssertAddrValid( iconObj, LIconPane );

	if( IsntNull( iconObj ) )
		iconObj->SetIconID( iconResID );
}


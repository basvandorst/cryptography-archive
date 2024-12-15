/*
 * Copyright (C) 1994-1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 */
#include "MacStrings.h"
#include "pgpMem.h"

#include "WarningAlert.h"



const ResID	kOKAlertResID		= 9940;
const ResID	kCancelAlertResID	= 9941;
const ResID	kOKCancelAlertResID	= 9942;
const ResID	kCancelOKAlertResID	= 9943;
const ResID	kYesNoAlertResID	= 9944;
const ResID	kNoYesAlertResID	= 9945;

	static short
ShowWarningAlert(
	WarningAlertType	alertType,
	WarningAlertStyle	alertStyle,
	ConstStr255Param	message)
{
	short		itemHit = 1;
	short		resID;
	const uchar	kEmptyString[]	= "\p";
	GrafPtr		savePort;
	
	InitCursor();
	GetPort( &savePort );
	
	switch( alertStyle )
	{
		case kWAOKStyle:			resID	= kOKAlertResID;		break;
		case kWACancelStyle:		resID	= kCancelAlertResID;	break;
		case kWAOKCancelStyle:		resID	= kOKCancelAlertResID;	break;
		case kWACancelOKStyle:		resID	= kCancelOKAlertResID;	break;
		case kWAYesNoStyle:			resID	= kYesNoAlertResID;		break;
		case kWANoYesStyle:			resID	= kNoYesAlertResID;		break;
	}
	
	ParamText( message, kEmptyString, kEmptyString, kEmptyString );
	
	if ( alertType == kWANoteAlertType )
	{
		itemHit	= NoteAlert( resID, nil );
	}
	else if ( alertType == kWACautionAlertType )
	{
		itemHit	= CautionAlert( resID, nil );
	}
	else if ( alertType == kWAStopAlertType )
	{
		itemHit	= StopAlert( resID, nil );
	}
	
	
	SetPort( savePort );
	
	return( itemHit );
}



	short
WarningAlert(
	WarningAlertType	alertType,
	WarningAlertStyle	alertStyle,
	ConstStr255Param	message,
	ConstStr255Param	str1,
	ConstStr255Param	str2,
	ConstStr255Param	str3
	)
{
	Str255	finalMessage;
	
	InsertPStrings( message, str1, str2, str3, finalMessage );
	
	return( ShowWarningAlert( alertType, alertStyle, finalMessage ) );
}


	short
WarningAlert(
	WarningAlertType	alertType,
	WarningAlertStyle	alertStyle,
	ResID				stringListResID,
	short				stringIndex,
	ConstStr255Param	str1,
	ConstStr255Param	str2,
	ConstStr255Param	str3)
{
	Str255	message;
	
	GetIndString( message, stringListResID, stringIndex );
	
	return( WarningAlert( alertType, alertStyle, message, str1, str2, str3 ) );
}



















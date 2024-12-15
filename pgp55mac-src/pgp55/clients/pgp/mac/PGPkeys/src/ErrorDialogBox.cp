/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ErrorDialogBox.cp,v 1.2 1997/09/17 19:36:42 wprice Exp $
____________________________________________________________________________*/
#include "ErrorDialogBox.h"
#include "CPGPKeys.h"
#include "string.h"

#include "UModalDialogs.h"

#include "pgpMem.h"

ErrorDialogBox	*ErrorDialogBox::sErrorHandler = 0;

ErrorDialogBox::ErrorDialogBox(void):
	mShowing(false)
{
	mErrorMessageLength = 0;
	mSmartAdd 			= true;
}	


	//Quick messages
	void	
ErrorDialogBox::Report(const char *theError)
{
	Reset();
	Add(theError);
	Show();
}


	void
ErrorDialogBox::Report(UInt32	number)
{
	Reset();
	Add(number);
	
	Show();
}

	void	
ErrorDialogBox::Report(
	ResIDT	resource, 
	UInt32 number)
{
	Str255	errorString;
	::GetIndString(errorString, resource, number);
	pgpAssert(errorString[ 0 ] > 0);
	Report(errorString);
}


	void
ErrorDialogBox::Report(
	const char *theError, 
	UInt32 		number)
{
	SaveErrorDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theError);
	Add(number);
	
	Show();
}


	void
ErrorDialogBox::Report(
	const char *theError, 
	UInt32 		number, 
	UInt32 		number2)
{
	SaveErrorDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theError);
	Add(number);
	Add(number2);
	
	Show();
}


	void
ErrorDialogBox::Report(
	const char *theError, 
	UInt32 number, 
	UInt32 number2, 
	UInt32 number3)
{
	SaveErrorDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theError);
	Add(number);
	Add(number2);
	Add(number3);
	
	Show();
}
	
	
		void	
ErrorDialogBox::Report(const unsigned char *theError)
{
	Reset();
	Add(theError);
	Show();
}


	void
ErrorDialogBox::Report(
	const unsigned char *theError, 
	UInt32 		number)
{
	SaveErrorDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theError);
	Add(number);
	
	Show();
}


	void
ErrorDialogBox::Report(
	const unsigned char *theError, 
	UInt32 		number, 
	UInt32 		number2)
{
	SaveErrorDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theError);
	Add(number);
	Add(number2);
	
	Show();
}


	void
ErrorDialogBox::Report(
	const unsigned char *theError, 
	UInt32 number, 
	UInt32 number2, 
	UInt32 number3)
{
	SaveErrorDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theError);
	Add(number);
	Add(number2);
	Add(number3);
	
	Show();
}


	//Build messages
	void
ErrorDialogBox::Reset(void)
{
	mErrorMessageLength = 0;
}


	void
ErrorDialogBox::Add(const char *theError)
{
	UInt32	stringLength;
	
	pgpAssertAddrValid(mErrorMessage, char);
	pgpAssertAddrValid(theError, char);

	if ((false != mSmartAdd) && (mErrorMessageLength > 0))
	{
		mErrorMessage[mErrorMessageLength++] = ' ';
	}
	
	stringLength = strlen(theError);
	if (stringLength > kMaxMessageLength - mErrorMessageLength)
		stringLength = kMaxMessageLength - mErrorMessageLength;
	
	BlockMoveData(theError, mErrorMessage + mErrorMessageLength, stringLength);
	mErrorMessageLength += stringLength;
	pgpAssert(mErrorMessageLength <= kMaxMessageLength);
}


	void
ErrorDialogBox::Add(const unsigned char *theError)
{
	UInt32	stringLength;
	
	pgpAssertAddrValid(mErrorMessage, char);
	pgpAssertAddrValid(theError, char);

	if ((false != mSmartAdd) && (mErrorMessageLength > 0))
	{
		mErrorMessage[mErrorMessageLength++] = ' ';
	}
	
	stringLength = theError[ 0 ];
	if (stringLength > kMaxMessageLength - mErrorMessageLength)
		stringLength = kMaxMessageLength - mErrorMessageLength;
	
	BlockMoveData(theError + 1, mErrorMessage + mErrorMessageLength,
					stringLength);
	mErrorMessageLength += stringLength;
	pgpAssert(mErrorMessageLength <= kMaxMessageLength);
}


	void
ErrorDialogBox::Add(Int32 number)
{
	Str255	numberString;
	
	pgpAssertAddrValid(mErrorMessage, char);
	if ((false != mSmartAdd) && (mErrorMessageLength > 0))
	{
		mErrorMessage[mErrorMessageLength++] = ' ';
	}
	
	NumToString(number, numberString);
	BlockMoveData(numberString+1, mErrorMessage+mErrorMessageLength,
					numberString[0]);
	mErrorMessageLength += numberString[0];
	pgpAssert(mErrorMessageLength <= kMaxMessageLength);
}


	void
ErrorDialogBox::Show(void)
{
	StDialogHandler	theHandler(kErrorWindowID, this);
	MessageT		hitMessage;
	
	LWindow			*theDialog 	= theHandler.GetDialog();	
	pgpAssertAddrValid(theDialog, LWindow);

	LCaption 		*theField 	=
		(LCaption*)theDialog->FindPaneByID(kErrorTextID);	
	pgpAssertAddrValid(theField, LCaption);
	
	Str255	caption;
	pgpAssertAddrValid(mErrorMessage, char);
	pgpAssert(mErrorMessageLength <= kMaxMessageLength);
	
	caption[0] = (Uint8)mErrorMessageLength;
	BlockMoveData (mErrorMessage, caption+1, caption[0]);
	theField->SetDescriptor(caption);
	
	mDefaultButton = (LControl *)theDialog->FindPaneByID(kPushButtonID);	
	pgpAssertAddrValid(mDefaultButton, LControl);
	
	mShowing = true;
	theDialog->Show();
	SwitchTarget(theDialog);
	
	do
	{
		hitMessage = theHandler.DoDialog();
	}	while(hitMessage != msg_OK);
	
	mShowing = false;
}
	

	//Global error handler.  Always use this one.
	ErrorDialogBox&	
ErrorDialogBox::ErrorHandler(void)
{
	if (IsNull(sErrorHandler))
	{
		sErrorHandler = new ErrorDialogBox;
		// must reset supercommander because the errorhandler may stay
		// around longer than the default supercommander
		sErrorHandler->SetSuperCommander(CPGPKeys::TheApp());
	}
		
	pgpAssertAddrValid(sErrorHandler, VoidAlign);
	return *sErrorHandler;
}


	Boolean	
ErrorDialogBox::GetSmartAdd(void) const
{
	return mSmartAdd;
}


	void	
ErrorDialogBox::SetSmartAdd(Boolean newSmartAdd)
{
	mSmartAdd = newSmartAdd;
}


	Boolean
ErrorDialogBox::HandleKeyPress(const EventRecord& inKeyEvent)
{
	Boolean		keyHandled = false;
	LControl	*keyButton = nil;
	
	if (mShowing)
	{
		switch (inKeyEvent.message & charCodeMask) 
		{
			case char_Enter:
			case char_Return:
				mDefaultButton->SimulateHotSpotClick(kControlButtonPart);
				return true;
				break;
			default:
				return LCommander::HandleKeyPress(inKeyEvent);
		}
	}
	else
		return LCommander::HandleKeyPress(inKeyEvent);
}

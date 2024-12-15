/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ChoiceDialogBox.cp,v 1.2 1997/09/17 19:36:41 wprice Exp $
____________________________________________________________________________*/
#include "ChoiceDialogBox.h"
#include "CPGPKeys.h"
#include "string.h"

#include "pgpMem.h"

ChoiceDialogBox	*ChoiceDialogBox::sChoiceHandler = 0;

ChoiceDialogBox::ChoiceDialogBox(void):
	mShowing(false),
	LCommander(CPGPKeys::TheApp())
{
	mChoiceMessageLength 	= 0;
	mSmartAdd 				= true;
	mOKButtonTitle[ 0 ] 	= 0;
	mCancelButtonTitle[ 0 ] = 0;
}	


	//Quick messages
	Boolean	
ChoiceDialogBox::Ask(const char *theChoice)
{
	Reset();
	Add(theChoice);
	return Show();
}


	Boolean
ChoiceDialogBox::Ask(UInt32	number)
{
	Reset();
	Add(number);
	
	return Show();
}


	Boolean
ChoiceDialogBox::Ask(
	const char *theChoice, 
	UInt32 		number)
{
	SaveChoiceDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theChoice);
	Add(number);
	
	return Show();
}


	Boolean
ChoiceDialogBox::Ask(
	const char *theChoice, 
	UInt32 		number, 
	UInt32 		number2)
{
	SaveChoiceDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theChoice);
	Add(number);
	Add(number2);
	
	return Show();
}


	Boolean
ChoiceDialogBox::Ask(
	const char *theChoice, 
	UInt32 number, 
	UInt32 number2, 
	UInt32 number3)
{
	SaveChoiceDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theChoice);
	Add(number);
	Add(number2);
	Add(number3);
	
	return Show();
}
	
	
		Boolean	
ChoiceDialogBox::Ask(const unsigned char *theChoice)
{
	Reset();
	Add(theChoice);
	return Show();
}


		Boolean	
ChoiceDialogBox::Ask(ResIDT resource, UInt32 number)
{
	Str255	theString;
	::GetIndString(theString, resource, number);
	pgpAssert(theString[ 0 ] > 0);
	return Ask(theString);
}



	Boolean
ChoiceDialogBox::Ask(
	const unsigned	char *theChoice, 
	UInt32 		number)
{
	SaveChoiceDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theChoice);
	Add(number);
	
	return Show();
}


	Boolean
ChoiceDialogBox::Ask(
	const unsigned	char *theChoice, 
	UInt32 		number, 
	UInt32 		number2)
{
	SaveChoiceDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theChoice);
	Add(number);
	Add(number2);
	
	return Show();
}


	Boolean
ChoiceDialogBox::Ask(
	const unsigned	char *theChoice, 
	UInt32 number, 
	UInt32 number2, 
	UInt32 number3)
{
	SaveChoiceDialogSmartAdd	saveSmartAdd;
	SetSmartAdd(true);

	Reset();
	Add(theChoice);
	Add(number);
	Add(number2);
	Add(number3);
	
	return Show();
}
	

	//Build messages
	void
ChoiceDialogBox::Reset(void)
{
	mChoiceMessageLength = 0;
}


	void
ChoiceDialogBox::Add(const char *theChoice)
{
	UInt32	stringLength;
	
	pgpAssertAddrValid(mChoiceMessage, char);
	pgpAssertAddrValid(theChoice, char);

	if ((false != mSmartAdd) && (mChoiceMessageLength > 0))
	{
		mChoiceMessage[mChoiceMessageLength++] = ' ';
	}
	
	stringLength = strlen(theChoice);
	if (stringLength > kMaxMessageLength - mChoiceMessageLength)
		stringLength = kMaxMessageLength - mChoiceMessageLength;
	
	BlockMoveData(theChoice, mChoiceMessage + mChoiceMessageLength,
					stringLength);
	mChoiceMessageLength += stringLength;
	pgpAssert(mChoiceMessageLength <= kMaxMessageLength);
}

	void
ChoiceDialogBox::Add(const unsigned char *theChoice)
{
	UInt32	stringLength;
	
	pgpAssertAddrValid(mChoiceMessage, char);
	pgpAssertAddrValid(theChoice, char);

	if ((false != mSmartAdd) && (mChoiceMessageLength > 0))
	{
		mChoiceMessage[mChoiceMessageLength++] = ' ';
	}
	
	stringLength = theChoice[ 0 ];
	if (stringLength > kMaxMessageLength - mChoiceMessageLength)
		stringLength = kMaxMessageLength - mChoiceMessageLength;
	
	BlockMoveData(theChoice + 1, mChoiceMessage + mChoiceMessageLength,
					stringLength);
	mChoiceMessageLength += stringLength;
	pgpAssert(mChoiceMessageLength <= kMaxMessageLength);
}


	void
ChoiceDialogBox::Add(Int32 number)
{
	Str255	numberString;
	
	pgpAssertAddrValid(mChoiceMessage, char);
	if ((false != mSmartAdd) && (mChoiceMessageLength > 0))
	{
		mChoiceMessage[mChoiceMessageLength++] = ' ';
	}
	
	NumToString(number, numberString);
	BlockMoveData(numberString+1, mChoiceMessage+mChoiceMessageLength,
					numberString[0]);
	mChoiceMessageLength += numberString[0];
	pgpAssert(mChoiceMessageLength <= kMaxMessageLength);
}


	Boolean
ChoiceDialogBox::Show(void)
{
	StDialogHandler	theHandler(kChoiceWindowID, this);
	MessageT		hitMessage;
	
	LWindow			*theDialog 	= theHandler.GetDialog();	
	pgpAssertAddrValid(theDialog, LWindow);

	LCaption 		*theField 	=
		(LCaption*)theDialog->FindPaneByID(kChoiceTextID);	
	pgpAssertAddrValid(theField, LCaption);
	
	Str255	caption;
	pgpAssertAddrValid(mChoiceMessage, char);
	pgpAssert(mChoiceMessageLength <= kMaxMessageLength);
	
	caption[0] = (Uint8)mChoiceMessageLength;
	BlockMoveData (mChoiceMessage, caption+1, caption[0]);
	theField->SetDescriptor(caption);
	
	mOKButton = (LGAPushButton *)theDialog->FindPaneByID(kOKButtonID);	
	pgpAssertAddrValid(mOKButton, LGAPushButton);
	if(IsNull(mOKButtonTitle[ 0 ]))
	{
		Str255	title;
		::GetIndString(title, kStringListID, kOKButtonTitleID);
		mOKButton->SetDescriptor(title);
		BlockMoveData(title, mOKButtonTitle, title[ 0 ]);
	}
	else
		mOKButton->SetDescriptor(mOKButtonTitle);

	mCancelButton =
		(LGAPushButton *)theDialog->FindPaneByID(kCancelButtonID);	
	pgpAssertAddrValid(mCancelButton, LGAPushButton);
	if(IsNull(mCancelButtonTitle[ 0 ]))
	{
		Str255	title;
		::GetIndString(title, kStringListID, kCancelButtonTitleID);
		mCancelButton->SetDescriptor(title);
		BlockMoveData(title, mCancelButtonTitle, title[ 0 ]);
	}
	else
	{
		if(mCancelButtonTitle[0] > 6)
		{
			mCancelButton->ResizeFrameTo(170, 20, false);
			mCancelButton->MoveBy(-110, 0, false);
		}
		mCancelButton->SetDescriptor(mCancelButtonTitle);
	}
	
	mShowing = true;
	theDialog->Show();
	SwitchTarget(theDialog);
	
	do
	{
		hitMessage = theHandler.DoDialog();
	}	while((hitMessage != msg_OK) && (hitMessage != msg_Cancel));
	
	mShowing 				= false;
	
	mOKButtonTitle[ 0 ] 	= '\0';
	mCancelButtonTitle[ 0 ] = '\0';
	
	return  (hitMessage == msg_OK);
}
	

	//Global Choice handler.  Always use this one.
	ChoiceDialogBox&	
ChoiceDialogBox::ChoiceHandler(void)
{
	if (IsNull(sChoiceHandler))
	{
		sChoiceHandler = new ChoiceDialogBox;
		// must reset supercommander because the errorhandler may stay
		// around longer than the default supercommander
		sChoiceHandler->SetSuperCommander(CPGPKeys::TheApp());
	}

	pgpAssertAddrValid(sChoiceHandler, ChoiceDialogBox);
	return *sChoiceHandler;
}


	Boolean	
ChoiceDialogBox::GetSmartAdd(void) const
{
	return mSmartAdd;
}


	void	
ChoiceDialogBox::SetSmartAdd(Boolean newSmartAdd)
{
	mSmartAdd = newSmartAdd;
}


	void	
ChoiceDialogBox::SetOKButtonTitle(		const char *okString)
{
	pgpAssertAddrValid(okString, char);
		
	mOKButtonTitle[ 0 ] = strlen(okString);
	BlockMoveData(okString, mOKButtonTitle + 1, mOKButtonTitle[ 0 ]);
	
	mOKButton->SetDescriptor(mOKButtonTitle);
}


	void	
ChoiceDialogBox::SetCancelButtonTitle(	const char *cancelString)
{
	pgpAssertAddrValid(cancelString, char);
	
	mCancelButtonTitle[ 0 ] = strlen(cancelString);
	BlockMoveData(cancelString, mCancelButtonTitle + 1,
		mCancelButtonTitle[ 0 ]);
}


	void
ChoiceDialogBox::SetOKButtonTitle(		const unsigned char *okString)
{
	pgpAssertAddrValid(okString, char);
	BlockMoveData(okString, mOKButtonTitle, okString[ 0 ] + 1);
}


	void	
ChoiceDialogBox::SetOKButtonTitle(		ResIDT resource, UInt32 number)
{
	Str255	theString;
	::GetIndString(theString, resource, number);
	pgpAssert(theString[ 0 ] > 0);
	SetOKButtonTitle(theString);
}


	void	
ChoiceDialogBox::SetCancelButtonTitle(	ResIDT resource, UInt32 number)
{
	Str255	theString;
	::GetIndString(theString, resource, number);
	pgpAssert(theString[ 0 ] > 0);
	SetCancelButtonTitle(theString);
}


	void	
ChoiceDialogBox::SetCancelButtonTitle(	const unsigned char *cancelString)
{
	pgpAssertAddrValid(cancelString, char);
	BlockMoveData(cancelString, mCancelButtonTitle, cancelString[ 0 ] + 1);
}


	Boolean
ChoiceDialogBox::HandleKeyPress(const EventRecord& inKeyEvent)
{
	Boolean		keyHandled = false;
	LControl	*keyButton = nil;
	
	if (mShowing)
	{
		switch (inKeyEvent.message & charCodeMask) 
		{
			case char_Enter:
			case char_Return:
				pgpAssertAddrValid(mOKButton, VoidAlign);
				mOKButton->SimulateHotSpotClick(kControlButtonPart);
				return true;
				break;
				
			case char_Escape:
				pgpAssertAddrValid(mCancelButton, VoidAlign);
				if ((inKeyEvent.message & keyCodeMask) == vkey_Escape) 
					mCancelButton->SimulateHotSpotClick(kControlButtonPart);
				return true;
				break;
				
			default:
			{
				char	charCode = inKeyEvent.message & charCodeMask;
				char	okCode, cancelCode;
				
				if ((charCode >= 'a') && (charCode <= 'z'))
					charCode += 'A' - 'a';
				
				pgpAssert(mOKButtonTitle[ 0 ] > 0);
				okCode = mOKButtonTitle[ 1 ];
				if ((okCode >= 'a') && (okCode <= 'z'))
					okCode += 'A' - 'a';

				pgpAssert(mCancelButtonTitle[ 0 ] > 0);
				cancelCode = mCancelButtonTitle[ 1 ];
				if ((cancelCode >= 'a') && (cancelCode <= 'z'))
					cancelCode += 'A' - 'a';
				
				if ((charCode == okCode) && (charCode != cancelCode))
				{
					mOKButton->SimulateHotSpotClick(kControlButtonPart);
					return true;
				}
				else if ((charCode != okCode) && (charCode == cancelCode))
				{
					mCancelButton->SimulateHotSpotClick(kControlButtonPart);
					return true;
				}
				if (UKeyFilters::IsCmdPeriod(inKeyEvent)) 
				{
					mCancelButton->SimulateHotSpotClick(kControlButtonPart);
					return true;
				}
				else 
					return LCommander::HandleKeyPress(inKeyEvent);
			}
		}
	}
	else
		return LCommander::HandleKeyPress(inKeyEvent);
}

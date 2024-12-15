/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPGPGetConvPassphraseGrafPrt.cp,v 1.3 1997/09/20 01:36:20 heller Exp $
____________________________________________________________________________*/

#include <LCaption.h>
#include <LGACheckbox.h>
#include <LGAPopup.h>
#include <LGAPushButton.h>
#include <UGAColorRamp.h>

#include "MacStrings.h"

#include "pgpUserInterface.h"
#include "pgpUtilities.h"

#include "CPassphraseEdit.h"
#include "CPGPGetConvPassphraseGrafPrt.h"
#include "CProgressBar.h"
#include "CSecureMemory.h"
#include "PGPUILibDialogs.h"


const PaneIDT	kPassphraseEditFieldPaneID		= 'ePwd';
const PaneIDT	kConfirmationEditFieldPaneID	= 'eCnf';
const PaneIDT	kHideTypingCheckboxPaneID		= 'HTyp';
const PaneIDT	kPassphraseQualityBarPaneID		= 'Qual';

CPGPGetConvPassphraseGrafPort::CPGPGetConvPassphraseGrafPort(LStream *inStream)
	: CPGPLibGrafPortView(inStream)
{
}

CPGPGetConvPassphraseGrafPort::~CPGPGetConvPassphraseGrafPort()
{
}

	void
CPGPGetConvPassphraseGrafPort::AdjustButtons(void)
{
	if( mPassphraseEdit->GetPassphraseLength() > 0 )
	{
		mOKButton->Enable();
	}
	else
	{
		mOKButton->Disable();
	}
}

	void
CPGPGetConvPassphraseGrafPort::FinishCreateSelf()
{
	CPGPLibGrafPortView::FinishCreateSelf();
	
	mPassphraseEdit =
		(CPassphraseEdit *) FindPaneByID( kPassphraseEditFieldPaneID );
	mConfirmationEdit =
		(CPassphraseEdit *) FindPaneByID( kConfirmationEditFieldPaneID );
	mHideTypingCheckbox =
		(LGACheckbox *) FindPaneByID( kHideTypingCheckboxPaneID );
	mPassQualityProgressBar = (CProgressBar *)
		FindPaneByID( kPassphraseQualityBarPaneID );
	
	SwitchTarget( mPassphraseEdit );
	
	mPassphraseEdit->SetHideTyping( TRUE );
	mPassphraseEdit->AddListener( this );
	mConfirmationEdit->SetHideTyping( TRUE );

	mHideTypingCheckbox->SetValue( 1 );
	mHideTypingCheckbox->AddListener( this );

	AdjustButtons();
}

	void
CPGPGetConvPassphraseGrafPort::ListenToMessage(
	MessageT 	inMessage,
	void 		*ioParam)
{
	switch (inMessage)
	{
		case kPassphraseEditFieldPaneID:
		{
			CSecureCString256	passphrase;
			PGPUInt16			quality;
			
			mPassphraseEdit->GetDescriptor( (uchar *) ((char *) passphrase) );
			PToCString( (uchar *) ((char *) passphrase), passphrase );
			
			quality = PGPCalcPassphraseQuality( passphrase );
			mPassQualityProgressBar->SetPercentComplete( quality );

			AdjustButtons();
			
			break;
		}

		case kHideTypingCheckboxPaneID:
		{
			mPassphraseEdit->SetHideTyping(
						mHideTypingCheckbox->GetValue() == 1 );
			mConfirmationEdit->SetHideTyping(
						mHideTypingCheckbox->GetValue() == 1 );
			break;
		}
			
		default:
			inherited::ListenToMessage( inMessage, ioParam );
			break;
	}
}

	Boolean
CPGPGetConvPassphraseGrafPort::HandleKeyPress(const EventRecord& inKeyEvent)
{
	Boolean		keyHandled = TRUE;
	
	if( (inKeyEvent.modifiers & cmdKey) != 0 )
	{
		Int16	theKey = inKeyEvent.message & charCodeMask;

		switch(theKey)
		{
			case 'H':
			case 'h':
				mHideTypingCheckbox->SetValue(
							!mHideTypingCheckbox->GetValue());
				mPassphraseEdit->SetHideTyping(
							mHideTypingCheckbox->GetValue() == 1 );
				mConfirmationEdit->SetHideTyping(
							mHideTypingCheckbox->GetValue() == 1 );
				break;
			
			default:
				keyHandled = CPGPLibGrafPortView::HandleKeyPress( inKeyEvent );
				break;
		}
	}
	else
	{
		keyHandled = CPGPLibGrafPortView::HandleKeyPress( inKeyEvent );
	}

	return keyHandled;
}

	void
CPGPGetConvPassphraseGrafPort::GetPassphrase(
	char 	passphrase[256],
	char	confirmation[256])
{
	pgpAssertAddrValid( passphrase, char);
	MacDebug_FillWithGarbage( passphrase, 256 );
	pgpAssertAddrValid( confirmation, char);
	MacDebug_FillWithGarbage( confirmation, 256 );

	mPassphraseEdit->GetDescriptor( (unsigned char *) passphrase );
	PToCString( (unsigned char *) passphrase, passphrase );

	mConfirmationEdit->GetDescriptor( (unsigned char *) confirmation );
	PToCString( (unsigned char *) confirmation, confirmation );
}

	void
CPGPGetConvPassphraseGrafPort::SetPrompt(ConstStr255Param prompt)
{
	LCaption	*captionObj;
	
	pgpAssertAddrValid( prompt, char );

	captionObj = (LCaption *) FindPaneByID( 'cTxt' );
	pgpAssertAddrValid( captionObj, LCaption );
	
	captionObj->SetDescriptor( prompt );
}

	void
CPGPGetConvPassphraseGrafPort::ClearPassphrase(void)
{
	mPassphraseEdit->ClearPassphrase();
	mConfirmationEdit->ClearPassphrase();
	mPassQualityProgressBar->SetPercentComplete( 0 );
	
	SwitchTarget( mPassphraseEdit );
}

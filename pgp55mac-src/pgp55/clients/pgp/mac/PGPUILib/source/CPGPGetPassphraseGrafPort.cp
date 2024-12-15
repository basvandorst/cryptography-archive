/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPGPGetPassphraseGrafPort.cp,v 1.17 1997/10/10 00:59:07 heller Exp $
____________________________________________________________________________*/

#include <LCaption.h>
#include <LGACheckbox.h>
#include <LGAPopup.h>
#include <UGAColorRamp.h>

#include "MacStrings.h"

#include "pflPrefTypes.h"
#include "pgpClientPrefs.h"
#include "pgpUtilities.h"

#include "CPassphraseEdit.h"
#include "CPGPGetPassphraseGrafPort.h"
#include "CPrivateKeysPopup.h"
#include "CUserIDTable.h"
#include "PGPUILibDialogs.h"
#include "pgpMem.h"


const PaneIDT	kPassphraseEditFieldPaneID	= 'ePwd';
const PaneIDT	kHideTypingCheckboxPaneID	= 'HTyp';
const PaneIDT	kTextOutputCheckboxPaneID	= 'TOut';
const PaneIDT	kMacBinaryPopupPaneID		= 'MBin';
const PaneIDT	kDetachedSigCheckboxPaneID	= 'DSig';
const PaneIDT	kKeysPopupPaneID			= 'pKey';
const PaneIDT	kKeyCaptionPaneID			= 'cKey';
const PaneIDT	kRecipientUserIDTablePaneID	= 'tUsr';

enum
{
	kMacBinaryPopupNoneMenuItem	= 1,
	kMacBinaryPopupSmartMenuItem,
	kMacBinaryPopupYesMenuItem
};

CPGPGetPassphraseGrafPort::CPGPGetPassphraseGrafPort(LStream *inStream)
	: CPGPLibGrafPortView(inStream)
{
	mHaveFileOptions		= FALSE;
	mTextOutputCheckbox		= NULL;
	mMacBinaryPopup			= NULL;
	mDetachedSigCheckbox	= NULL;
}

CPGPGetPassphraseGrafPort::~CPGPGetPassphraseGrafPort()
{
	if( IsntNull( mRecipientTable ) )
	{
		/* Delete the backing store for the name list */
		PGPRecipientTableItem::DisposeNameData();
	}
}

	void
CPGPGetPassphraseGrafPort::FinishCreateSelf()
{
	CPGPLibGrafPortView::FinishCreateSelf();
	
	mPassphraseEdit =
		(CPassphraseEdit *) FindPaneByID( kPassphraseEditFieldPaneID );
	mHideTypingCheckbox =
		(LGACheckbox *) FindPaneByID( kHideTypingCheckboxPaneID );
	mKeysPopup =
		(CPrivateKeysPopup *) FindPaneByID( kKeysPopupPaneID );
	mRecipientTable =
		(CUserIDTable *) FindPaneByID( kRecipientUserIDTablePaneID );
		
	
	SwitchTarget( mPassphraseEdit );
	
	mPassphraseEdit->SetHideTyping( TRUE );
	mHideTypingCheckbox->SetValue( 1 );
	mHideTypingCheckbox->AddListener(this);
}

	void
CPGPGetPassphraseGrafPort::ResetTarget()
{
	RGBColor	backColor	= UGAColorRamp::GetColor( colorRamp_Gray2 );
	RGBColor	foreColor	= UGAColorRamp::GetBlackColor();
	
	mPassphraseEdit->ClearPassphrase();
	SwitchTarget( mPassphraseEdit );
	OutOfFocus(nil);
	FocusDraw();
	SetForeAndBackColors( &foreColor, &backColor );
	ApplyForeAndBackColors();
	
	Refresh();
}

	Boolean
CPGPGetPassphraseGrafPort::BuildKeyList(PGPKeySetRef allKeys)
{
	return( mKeysPopup->BuildKeyList( allKeys ) );
}

	void
CPGPGetPassphraseGrafPort::SetRecipients(
	PGPKeySetRef 	recipientSet,
	PGPUInt32		numMissingRecipients )
{
	if( IsntNull( mRecipientTable ) )
	{
		PGPError		err;
		PGPKeyListRef	keyList;
		
		mRecipientTable->InsertCols(1, 1, NULL, 0, TRUE);
		mRecipientTable->SetDisplayColumns( TRUE, FALSE, FALSE );
		mRecipientTable->DisallowSelections();
		
		if( numMissingRecipients > 0 )
		{
			Str255	userID;
			
			if( numMissingRecipients == 1 )
			{
				GetIndString( userID, kPGPLibDialogsStringListResID,
					kUnknownKeyStrIndex );
			}
			else
			{
				Str32	numStr;
				
				GetIndString( userID, kPGPLibDialogsStringListResID,
					kXUnknownKeysStrIndex );
				NumToString( numMissingRecipients, numStr );
				PrintPString( userID, userID, numStr );
			}
			
			PToCString( userID, (char *) userID );
			mRecipientTable->AddMissingUserID( (char *) userID, TRUE );
		}
		
		err = PGPOrderKeySet( recipientSet, kPGPAnyOrdering, &keyList );
		if( IsntPGPError( err ) )
		{
			PGPKeyIterRef	keyIterator;
			
			err = PGPNewKeyIter( keyList, &keyIterator );
			if( IsntPGPError( err ) )
			{
				PGPKeyRef	theKey;

				err = PGPKeyIterNext( keyIterator, &theKey );
				while( IsntPGPError( err ) )
				{
					mRecipientTable->AddKey( theKey, FALSE, TRUE );
					
					err = PGPKeyIterNext( keyIterator, &theKey );
				}
					
				PGPFreeKeyIter( keyIterator );
			}
			
			PGPFreeKeyList( keyList );
		}
	}
}

	void
CPGPGetPassphraseGrafPort::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
		case kHideTypingCheckboxPaneID:
			mPassphraseEdit->SetHideTyping(
						mHideTypingCheckbox->GetValue() == 1 );
			break;
			
		default:
			inherited::ListenToMessage( inMessage, ioParam );
			break;
	}
}

	Boolean
CPGPGetPassphraseGrafPort::HandleKeyPress(const EventRecord& inKeyEvent)
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
				break;
			
			case 'T':
			case 't':
				if( IsntNull( mTextOutputCheckbox )  )
				{
					mTextOutputCheckbox->SetValue(
								!mTextOutputCheckbox->GetValue());
				}
				break;

			case 'D':
			case 'd':
				if( IsntNull( mDetachedSigCheckbox )  )
				{
					mDetachedSigCheckbox->SetValue(
								!mDetachedSigCheckbox->GetValue());
				}
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
CPGPGetPassphraseGrafPort::GetPassphrase(char passphrase[256])
{
	pgpAssertAddrValid( passphrase, char);
	MacDebug_FillWithGarbage( passphrase, 256 );

	mPassphraseEdit->GetDescriptor( (unsigned char *) passphrase );
	PToCString( (unsigned char *) passphrase, passphrase );
}

	void
CPGPGetPassphraseGrafPort::SetPrompt(ConstStr255Param prompt)
{
	LCaption	*captionObj;
	
	pgpAssertAddrValid( prompt, char );

	captionObj = (LCaption *) FindPaneByID( 'cTxt' );
	pgpAssertAddrValid( captionObj, LCaption );
	
	captionObj->SetDescriptor( prompt );
}

	PGPKeyRef
CPGPGetPassphraseGrafPort::GetTargetKey(void)
{
	PGPKeyRef	targetKey = kInvalidPGPKeyRef;
	
	if( IsntNull( mKeysPopup ) )
	{
		targetKey = mKeysPopup->GetSigningKey();
	}
	
	return( targetKey );
}

	void
CPGPGetPassphraseGrafPort::SetTargetKey(PGPKeyRef signingKey)
{
	if (mKeysPopup != nil)
	{
		mKeysPopup->SetSigningKey( signingKey );
	}
	else
	{
		LCaption	*captionObj;
		PGPError	err;
		Str255		buf;
		PGPSize		len;
		
		captionObj = (LCaption *) FindPaneByID( kKeyCaptionPaneID );
		pgpAssertAddrValid( captionObj, LCaption );
		
		err = PGPGetPrimaryUserIDNameBuffer( (PGPKeyRef) signingKey,
			sizeof( buf ) - 1, (char *) &buf[1], &len );
		if ( IsntPGPError( err ) )
		{
			buf[0] = len;
			
			captionObj->SetDescriptor( buf );
		}
	}
}

	void
CPGPGetPassphraseGrafPort::SetSettings(
	PGPPrefRef					clientPrefsRef,
	PGPGetPassphraseSettings 	defaultSettings,
	PGPGetPassphraseOptions		dialogOptions)
{
	FocusDraw();
	
	if( ( dialogOptions & kPGPGetPassphraseOptionsHideFileOptions ) == 0 )
	{
		short	macBinaryMenuItem;

		mHaveFileOptions = TRUE;
	
		mDetachedSigCheckbox =
			 (LGACheckbox *) FindPaneByID( kDetachedSigCheckboxPaneID );
		mTextOutputCheckbox = 
			(LGACheckbox *) FindPaneByID( kTextOutputCheckboxPaneID );
		mMacBinaryPopup =
			(LGAPopup *) FindPaneByID( kMacBinaryPopupPaneID );

		mDetachedSigCheckbox->SetValue(
			( defaultSettings & kPGPGetPassphraseSettingsDetachedSig ) != 0 );

		mTextOutputCheckbox->SetValue(
			( defaultSettings & kPGPUISettingsTextOutput ) != 0 );

		/* MacBinary always on by default */
		macBinaryMenuItem = kMacBinaryPopupYesMenuItem;

		if ( ( defaultSettings & kPGPUISettingsNoMacBinary ) != 0 )
		{
			macBinaryMenuItem = kMacBinaryPopupNoneMenuItem;
		}
		else if ( ( defaultSettings & kPGPUISettingsSmartMacBinary ) != 0 )
		{
			macBinaryMenuItem = kMacBinaryPopupSmartMenuItem;
		}
		else if ( ( defaultSettings & kPGPUISettingsForceMacBinary ) != 0 )
		{
			macBinaryMenuItem = kMacBinaryPopupYesMenuItem;
		}
		else
		{
			PGPUInt32	macBinaryPref;
			
			if( IsntPGPError( PGPGetPrefNumber( clientPrefsRef,
					kPGPPrefMacBinaryDefault, &macBinaryPref ) ) )
			{
				switch( macBinaryPref )
				{
					case kPGPPrefMacBinaryOff:
						macBinaryMenuItem = kMacBinaryPopupNoneMenuItem;
						break;
					
					case kPGPPrefMacBinarySmart:
						macBinaryMenuItem = kMacBinaryPopupSmartMenuItem;
						break;

					case kPGPPrefMacBinaryOn:
					default:
						macBinaryMenuItem = kMacBinaryPopupYesMenuItem;
						break;
				}
			}
		}
		
		mMacBinaryPopup->SetValue( macBinaryMenuItem );
	}
	
	ApplyForeAndBackColors();
}

	PGPGetPassphraseSettings
CPGPGetPassphraseGrafPort::GetSettings(void)
{
	PGPGetPassphraseSettings	settings = 0;

	if( mHaveFileOptions )
	{
		UInt32	macBinarySetting;

		if( mTextOutputCheckbox->GetValue() == 1 )
			settings |= kPGPUISettingsTextOutput;

		if( mDetachedSigCheckbox->GetValue() == 1 )
			settings |= kPGPGetPassphraseSettingsDetachedSig;

		macBinarySetting = mMacBinaryPopup->GetValue();
		switch( macBinarySetting )
		{
			case kMacBinaryPopupNoneMenuItem:
				settings |= kPGPUISettingsNoMacBinary;
				break;

			case kMacBinaryPopupSmartMenuItem:
				settings |= kPGPUISettingsSmartMacBinary;
				break;

			case kMacBinaryPopupYesMenuItem:
				settings |= kPGPUISettingsForceMacBinary;
				break;
		}	
	}
	
	return( settings );
}

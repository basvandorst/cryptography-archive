/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPGPPassphraseGrafPortView.cp,v 1.9.10.1 1998/11/12 03:20:38 heller Exp $____________________________________________________________________________*/#include <LCheckBox.h>#include <LProgressBar.h>#include <LPushButton.h>#include <LStaticText.h>#include <PP_Messages.h>#include <UGAColorRamp.h>#include "MacEvents.h"#include "MacSecureMemory.h"#include "MacStrings.h"#include "pgpMem.h"#include "pgpUtilities.h"#include "CPassphraseEdit.h"#include "CPGPPassphraseGrafPortView.h"#include "CPrivateKeysPopup.h"#include "PGPsdkUILibDialogs.h"CPGPPassphraseGrafPortView::CPGPPassphraseGrafPortView(LStream *inStream)	: CPGPUIGrafPortView(inStream){	mPassphraseEdit 		= NULL;	mHideTypingCheckbox		= NULL;	mOptionsButton			= NULL;	mOptions				= NULL;	mCapsLockMessage		= NULL;	mShowingCapsLockMessage	= FALSE;}CPGPPassphraseGrafPortView::~CPGPPassphraseGrafPortView(){}	voidCPGPPassphraseGrafPortView::FinishCreateSelf(){	CPGPUIGrafPortView::FinishCreateSelf();		mPassphraseEdit =		(CPassphraseEdit *) FindPaneByID( kPassphraseEditFieldPaneID );	mHideTypingCheckbox =		(LCheckBox *) FindPaneByID( kHideTypingCheckboxPaneID );	mOptionsButton = 		(LPushButton *) FindPaneByID( kOptionsButtonPaneID );	mCapsLockMessage =		(LStaticText *) FindPaneByID( kCapsLockDownTextPaneID );			pgpAssertAddrValid( mPassphraseEdit, VoidAlign );	pgpAssertAddrValid( mHideTypingCheckbox, VoidAlign );	pgpAssertAddrValid( mOptionsButton, VoidAlign );		SwitchTarget( mPassphraseEdit );		mPassphraseEdit->SetHideTyping( TRUE );	mPassphraseEdit->AddListener( this );		mHideTypingCheckbox->SetValue( 1 );	mHideTypingCheckbox->AddListener( this );		if( IsntNull( mCapsLockMessage ) )		mCapsLockMessage->Hide();}	PGPErrorCPGPPassphraseGrafPortView::SetOptions(	PGPContextRef 					context,	CPGPPassphraseDialogOptions 	*options){	PGPError	err = kPGPError_NoErr;		err = CPGPUIGrafPortView::SetOptions( context, options );	if( IsntPGPError( err ) )	{		mOptions = options;				if( IsntNull( options->mPrompt ) )		{			Str255	pPrompt;						CToPString( options->mPrompt, pPrompt );			SetDescriptor( pPrompt );		}		// Hide the options button unless the client has asked for options				if( IsntNull( options->mDialogOptions ) &&			pgpGetOptionListCount( options->mDialogOptions ) > 0 )		{			LPane 		*positioningPane;			SPoint32	loc;						mOptionsButton->AddListener( this );						/* Move Hide Typing checkbox above the options button */						positioningPane = FindPaneByID( kHideTypingPositioningPaneID );			pgpAssertAddrValid( positioningPane, VoidAlign );						positioningPane->GetFrameLocation( loc );			mHideTypingCheckbox->PlaceInSuperFrameAt( loc.h, loc.v, FALSE );		}		else		{			mOptionsButton->Disable();			mOptionsButton->Hide();		}		AdjustButtons();	}		return( err );}	MessageTCPGPPassphraseGrafPortView::HandleMessage(MessageT theMessage){	if( theMessage == msg_OK )	{		PGPUInt32	passphraseLength;				pgpAssert( IsntNull( mOptions->mPassphrasePtr ) );				passphraseLength = mPassphraseEdit->GetPassphraseLength();				*mOptions->mPassphrasePtr = (char *) PGPNewSecureData(							PGPGetContextMemoryMgr( mContext ),							mPassphraseEdit->GetPassphraseLength() + 1, 0 );		if( IsntNull( *mOptions->mPassphrasePtr ) )		{			mPassphraseEdit->GetPassphrase( *mOptions->mPassphrasePtr );		}		else		{			mDialogError = kPGPError_OutOfMemory;		}	}		return( theMessage );}	voidCPGPPassphraseGrafPortView::AdjustButtons(void){	PGPBoolean			disableOKButton = FALSE;		pgpAssert( IsntNull( mOptions ) );	if( mOptions->mMinPassphraseLength > 0 &&			mPassphraseEdit->GetPassphraseLength() <				mOptions->mMinPassphraseLength )	{		disableOKButton = TRUE;	}		if( ! disableOKButton )	{		if( mOptions->mMinPassphraseQuality > 0 &&			mPassphraseEdit->EstimatePassphraseQuality() <				mOptions->mMinPassphraseQuality )		{			disableOKButton = TRUE;		}	}		if( disableOKButton )	{		mOKButton->Disable();	}	else	{		mOKButton->Enable();	}}	voidCPGPPassphraseGrafPortView::ListenToMessage(MessageT inMessage, void *ioParam){	switch (inMessage)	{		case msg_HideTyping:			mPassphraseEdit->SetHideTyping(						mHideTypingCheckbox->GetValue() == 1 );			break;		case msg_ChangedPassphrase:			AdjustButtons();			break;				case msg_Options:			DoOptionsDialog();			break;					default:			CPGPUIGrafPortView::ListenToMessage( inMessage, ioParam );			break;	}}	BooleanCPGPPassphraseGrafPortView::HandleKeyPress(const EventRecord& inKeyEvent){	Boolean		keyHandled = TRUE;		if( (inKeyEvent.modifiers & cmdKey) != 0 )	{		Int16	theKey = inKeyEvent.message & charCodeMask;		switch(theKey)		{			case 'H':			case 'h':			{				mHideTypingCheckbox->SetValue(							!mHideTypingCheckbox->GetValue());				mPassphraseEdit->SetHideTyping(							mHideTypingCheckbox->GetValue() == 1 );				break;			}						case 'O':			case 'o':			{				if( mOptionsButton->IsEnabled() )				{					mOptionsButton->SimulateHotSpotClick( kControlButtonPart );				}								break;			}			default:				keyHandled = CPGPUIGrafPortView::HandleKeyPress(									inKeyEvent );				break;		}	}	else	{		keyHandled = CPGPUIGrafPortView::HandleKeyPress( inKeyEvent );	}	return keyHandled;}	voidCPGPPassphraseGrafPortView::ClearPassphrase(void){	mPassphraseEdit->ClearPassphrase();		if( IsntNull( *mOptions->mPassphrasePtr ) )	{		PGPFreeData( *mOptions->mPassphrasePtr );		*mOptions->mPassphrasePtr = NULL;	}}	voidCPGPPassphraseGrafPortView::SetDescriptor(ConstStringPtr prompt){	LStaticText	*captionObj;		pgpAssertAddrValid( prompt, char );	captionObj = (LStaticText *) FindPaneByID( kPromptTextPaneID );	pgpAssertAddrValid( captionObj, VoidAlign );		captionObj->SetDescriptor( prompt );}	voidCPGPPassphraseGrafPortView::DoOptionsDialog(void){	pgpAssertAddrValid( mOptions->mDialogOptions, VoidAlign );		/*	** mOptions->dialogOptions is probably a volatile PGPOptionList i.e.	** passing it to PGPOptionsDialog() will dispose of it. Make a copy	** here for our purposes.	*/	(void) PGPOptionsDialog( mContext,					pgpCopyOptionList( mOptions->mDialogOptions ),					PGPOLastOption( mContext ) );}	voidCPGPPassphraseGrafPortView::DoIdle(const EventRecord &inMacEvent){	CPGPUIGrafPortView::DoIdle( inMacEvent );		if( IsntNull( mCapsLockMessage ) )	{		if( mPassphraseEdit->IsVisible() )		{			if( CapsLockIsDown() )			{				if( ! mShowingCapsLockMessage )				{					mShowingCapsLockMessage = TRUE;					mCapsLockMessage->Show();				}			}			else if( mShowingCapsLockMessage )			{				mShowingCapsLockMessage = FALSE;				mCapsLockMessage->Hide();			}		}		else if( mShowingCapsLockMessage )		{			mShowingCapsLockMessage = FALSE;			mCapsLockMessage->Hide();		}	}}
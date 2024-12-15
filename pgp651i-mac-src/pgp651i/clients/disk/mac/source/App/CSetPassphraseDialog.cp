/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CSetPassphraseDialog.cp,v 1.13 1999/05/22 07:08:27 heller Exp $____________________________________________________________________________*/#include <Sound.h>#include <LCheckBox.h>#include <LProgressBar.h>#include <PP_Messages.h>#include <UModalDialogs.h>#include "CPGPStDialogHandler.h"#include "pgpMacMemory.h"#include "pgpPassphraseUtils.h"#include "MacSecureMemory.h"#include "MacStrings.h"#include "PGPDisk.h"#include "PGPDiskResources.h"#include "PGPDiskPreferences.h"#include "PGPDiskUtils.h"#include "CPGPDiskApplication.h"#include "CSetPassphraseDialog.h"#include "CPassphraseEdit.h"#include "CWarningAlert.h"const short		kSetPassphraseDialogResID			= 5000;const PaneIDT	kConfirmationCaptionPaneID			= 5007;const PaneIDT	kConfirmationEditFieldPaneID		= 5008;const PaneIDT	kReadOnlyPassphraseCheckboxPaneID	= 5009;const PaneIDT	kMoveHideTypingToLocationPaneID		= 5010;const PaneIDT	kMinPasspharaseQualityCaptionPaneID	= 5011;const PaneIDT	kMaxPasspharaseQualityCaptionPaneID	= 5012;const PaneIDT	kPasspharaseQualityBarPaneID		= 5013;//	Presents the new passphrase dialog to the user until the user aborts or//	enters a passphrase of the correct format.	static OSStatusDoSetChangePassphraseDialog(	ConstStr255Param 	diskName,	short				promptIndex,	Boolean				showReadOnlyPassphraseCheckbox,	StringPtr 			passphrase,	Boolean				*addReadOnly){	CPGPStDialogHandler		dialogHandler( kSetPassphraseDialogResID, nil );	CSetPassphraseDialog	*dialogObj;	OSStatus				status;	Str255					prompt;		pgpAssertAddrValid( diskName, uchar );	pgpAssertAddrValid( passphrase, uchar );	pgpAssertAddrValid( addReadOnly, Boolean );		status = noErr;	dialogObj = (CSetPassphraseDialog *) dialogHandler.GetDialog();	if( IsNull( dialogObj ) )	{		pgpDebugMsg( "GetNewPassphraseForDisk: Nil dialog object" );		return( memFullErr );	}		GetIndString( prompt, kDialogStringListResID, promptIndex );	PrintPString( prompt, prompt, diskName );	dialogObj->SetPrompt( prompt );	dialogObj->ShowReadOnlyCheckbox( showReadOnlyPassphraseCheckbox );	dialogObj->Show();		while (TRUE)	{		MessageT	hitMessage = dialogHandler.DoDialog();		if( hitMessage == msg_Cancel ||			CPGPDiskApplication::mApplication->GetState() == programState_Quitting )		{			status = userCanceledErr;			break;		}		else if( hitMessage == msg_OK )		{			if( dialogObj->PassphraseIsConfirmed() )			{				dialogObj->GetPassphrase( passphrase );								if( passphrase[0] < kMinPassPhraseLength )				{					Str32	numStr;					Str255	msg;										GetIndString( msg, kErrorStringListResID,							kPassphraseToShortStrIndex );					NumToString( kMinPassPhraseLength, numStr );					PrintPString( msg, msg, numStr );					SysBeep( 1 );					CWarningAlert::Display( kWANoteAlertType, kWAOKStyle,							msg );				}				else				{						*addReadOnly = dialogObj->GetReadOnlyCheckboxStatus();					break;				}			}			else			{				SysBeep( 1 );				CWarningAlert::Display( kWANoteAlertType, kWAOKStyle,							kErrorStringListResID,							kPassphrasesMustMatchStrIndex );											dialogObj->ClearPassphraseFields();			}		}	}	dialogObj->Hide();		return( status );}//	Presents the new passphrase dialog to the user until the user aborts or//	enters a passphrase of the correct format.	OSStatusDoNewPassphraseDialog(	ConstStr255Param 	diskName,	StringPtr 			passphrase){	OSStatus	status;	Boolean		notUsed;		pgpAssertAddrValid( diskName, uchar );	pgpAssertAddrValid( passphrase, uchar );	status = DoSetChangePassphraseDialog( diskName,					kNewPassphraseDialogPromptStrIndex, FALSE, passphrase,					&notUsed );		return( status );}//	Presents the add passphrase dialog to the user until the user aborts or//	enters a passphrase of the correct format.	OSStatusDoAddPassphraseDialog(	ConstStr255Param 	diskName,	StringPtr 			passphrase,	Boolean				*addReadOnly){	OSStatus	status;		pgpAssertAddrValid( diskName, uchar );	pgpAssertAddrValid( passphrase, uchar );	pgpAssertAddrValid( addReadOnly, Boolean );	status = DoSetChangePassphraseDialog( diskName,						kAddPassphraseDialogPromptStrIndex,						TRUE, passphrase, addReadOnly );		return( status );}//	Presents the set passphrase dialog to the user until the user aborts or//	enters a passphrase of the correct format.	OSStatusDoSetPassphraseDialog(	ConstStr255Param 	diskName,	StringPtr 			passphrase){	OSStatus	status;	Boolean		notUsed;		pgpAssertAddrValid( diskName, uchar );	pgpAssertAddrValid( passphrase, uchar );	status = DoSetChangePassphraseDialog( diskName,						kSetPassphraseDialogPromptStrIndex,						FALSE, passphrase, &notUsed );		return( status );}CSetPassphraseDialog::CSetPassphraseDialog(){}CSetPassphraseDialog::CSetPassphraseDialog(LStream *inStream)	: CPassphraseDialog(inStream){}CSetPassphraseDialog::~CSetPassphraseDialog(){}	voidCSetPassphraseDialog::FinishCreateSelf(void){	CPassphraseDialog::FinishCreateSelf();		mConfirmationFieldObj =			(CPassphraseEdit *) FindPaneByID( kConfirmationEditFieldPaneID );	pgpAssertAddrValid( mConfirmationFieldObj, VoidAlign ); 	// Setup the confirmation field options	mConfirmationFieldObj->SetHideTyping( TRUE );	mConfirmationFieldObj->AddListener( this );		mPassphraseQualityBarObj =			(LProgressBar *) FindPaneByID( kPasspharaseQualityBarPaneID );	pgpAssertAddrValid( mPassphraseQualityBarObj, VoidAlign ); }	BooleanCSetPassphraseDialog::PassphraseIsConfirmed(void){	CSecurePString255 	passphrase;	CSecurePString255 	confirmation;	Boolean				confirmed;		pgpAssertAddrValid( mPassphraseFieldObj, VoidAlign );	pgpAssertAddrValid( mConfirmationFieldObj, VoidAlign );		mPassphraseFieldObj->GetDescriptor( passphrase );	mConfirmationFieldObj->GetDescriptor( confirmation );		confirmed = PStringsAreEqual( passphrase, confirmation );		return( confirmed );}	BooleanCSetPassphraseDialog::GetPassphrase(StringPtr passphrase){	if( ! PassphraseIsConfirmed() )	{		pgpDebugMsg("GetPassphrase(): Passphrase is not correctly confirmed");		return( FALSE );	}		return( CPassphraseDialog::GetPassphrase( passphrase ) );}	voidCSetPassphraseDialog::SetHideTyping(Boolean hideTyping){	pgpAssertAddrValid( mConfirmationFieldObj, VoidAlign );		CPassphraseDialog::SetHideTyping( hideTyping );		mConfirmationFieldObj->SetHideTyping( hideTyping );}	voidCSetPassphraseDialog::ShowReadOnlyCheckbox(Boolean showCheckbox){	if( showCheckbox )	{		LCheckBox	*control;		LPane		*placeHolder;		SPoint32	placeHolderLoc;				control =			(LCheckBox *) FindPaneByID( kReadOnlyPassphraseCheckboxPaneID );		pgpAssertAddrValid( control, VoidAlign );				control->Show();		control = (LCheckBox *) FindPaneByID( kHideTypingCheckboxPaneID );		pgpAssertAddrValid( control, VoidAlign );		placeHolder = FindPaneByID( kMoveHideTypingToLocationPaneID );		pgpAssertAddrValid( placeHolder, LPane );				placeHolder->GetFrameLocation( placeHolderLoc );		control->PlaceInSuperFrameAt( placeHolderLoc.h, placeHolderLoc.v,				TRUE );	}}	BooleanCSetPassphraseDialog::GetReadOnlyCheckboxStatus(void){	LCheckBox	*control;		control = (LCheckBox *) FindPaneByID( kReadOnlyPassphraseCheckboxPaneID );	pgpAssertAddrValid( control, VoidAlign );	return( control->GetValue() != 0 );}	voidCSetPassphraseDialog::ClearPassphraseFields(void){	mPassphraseFieldObj->ClearPassphrase();	mConfirmationFieldObj->ClearPassphrase();		SwitchTarget( mPassphraseFieldObj );		PassphraseHasChanged();}	voidCSetPassphraseDialog::PassphraseHasChanged(void){	CPassphraseDialog::PassphraseHasChanged();		UpdatePassphraseQuality();}	voidCSetPassphraseDialog::UpdatePassphraseQuality(void){	CSecurePString255	passphrase;	CSecureCString256	cPassphrase;	ushort				passphraseQuality;		mPassphraseFieldObj->GetDescriptor( passphrase );	PToCString( passphrase, cPassphrase );		passphraseQuality = pgpEstimatePassphraseQuality( cPassphrase );	pgpAssert( passphraseQuality <= 100 );		mPassphraseQualityBarObj->SetValue( passphraseQuality );}
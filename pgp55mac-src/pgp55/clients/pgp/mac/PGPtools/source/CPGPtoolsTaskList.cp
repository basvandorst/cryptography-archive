/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$ld$
_____________________________________________________________________________*/

#include <string.h>

#include <UDesktop.h>
#include <UModalDialogs.h>

#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpAdminPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpMem.h"
#include "pgpOpenPrefs.h"
#include "pgpUserInterface.h"
#include "pgpUtilities.h"
#include "pgpVersionHeader.h"
#include "CSecureMemory.h"
#include "CWrappers.h"
#include "CWarningAlert.h"

#include "MacInternet.h"
#include "MacStrings.h"

#include "CPGPStDialogHandler.h"

#include "CPGPtools.h"
#include "CPGPtoolsPassphraseList.h"
#include "CPGPtoolsTaskList.h"
#include "CPGPtoolsCryptoTask.h"
#include "CPGPtoolsWipeTask.h"
#include "CPGPtoolsTaskProgressDialog.h"
#include "CResultsWindow.h"
#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"


	static PGPLocalEncodingFlags
PGPUISettingsToLocalEncoding( PGPUISettings	settings )
{
	PGPLocalEncodingFlags	encodingFlags = kPGPLocalEncoding_None;
	
	if( ( settings & kPGPUISettingsSmartMacBinary ) != 0 )
	{
		encodingFlags = kPGPLocalEncoding_Auto;
	}
	else if( ( settings & kPGPUISettingsForceMacBinary ) != 0 )
	{
		encodingFlags = kPGPLocalEncoding_Force;
	}
	
	return( encodingFlags );
}


CPGPtoolsTaskList::CPGPtoolsTaskList(
	PGPtoolsOperation	operation)
{
	/* setup default prefs */
	pgpClearMemory( &mPrefs, sizeof( mPrefs ) );
	
	mPrefs.warnOnWipe		= TRUE;
	mPrefs.comment[ 0 ]		= 0;
	mPrefs.convAlgorithm	= kPGPCipherAlgorithm_None;
	mTaskListHasFileTasks	= FALSE;
	mOperation 				= operation;
	
	mTaskList = new TArray<CPGPtoolsTask *>;
	pgpAssertAddrValid( mTaskList, VoidAlign );
}

CPGPtoolsTaskList::~CPGPtoolsTaskList(void)
{
	if( IsntNull( mTaskList ) )
	{
		DeleteTasks();

		delete( mTaskList );
		mTaskList = nil;
	}
	
	if( IsntNull( mPrefs.allowedAlgorithms ) )
	{
		(void) pgpFree( mPrefs.allowedAlgorithms );
	}
}

	void
CPGPtoolsTaskList::DeleteTasks(void)
{
	if ( IsntNull( mTaskList ) )
	{
		LArrayIterator	iterator( *mTaskList );
		CPGPtoolsTask	*task;

		while( iterator.Next( &task ) )
		{
			delete( task );
		}
	}
}
	
	void
CPGPtoolsTaskList::AppendTask(CPGPtoolsTask *theTask)
{
	pgpAssertAddrValid( theTask, VoidAlign );
	
	if( IsntNull( mTaskList ) )
	{
		mTaskList->InsertItemsAt( 1, LArray::index_Last, &theTask );
	}
}

	CToolsError
CPGPtoolsTaskList::SetupEncryptionOptions(
	PGPContextRef			context,
	PGPKeySetRef			recipients,
	PGPRecipientSettings	settings,
	PGPRecipientOptions		options,
	PGPOptionListRef		encodeOptions)
{
	CToolsError	err;
	Boolean		haveFileOptions;
	
	haveFileOptions	= ( options & kPGPRecipientOptionsHideFileOptions ) == 0;
						
	// Check for conventional encryption first.
	
	if( ( settings & kPGPRecipientSettingsConvEncrypt ) != 0 )
	{
		CSecureCString256	passphrase;
		char				prompt[256];
		
		GetIndCString( prompt, kDialogStringsListResID,
				kPleaseEnterConventionalPassphrasePromptStrIndex );
				
		err.pgpErr = PGPGetConventionalEncryptionPassphraseDialog(
				context, prompt, passphrase );
		if( err.IsntError() )
		{
			err.pgpErr = PGPAppendOptionList( context, encodeOptions,
						PGPOConventionalEncrypt( context,
							PGPOPassphrase( context, passphrase ),
							PGPOLastOption( context ) ),
						mPrefs.convAlgorithm != kPGPCipherAlgorithm_None ?
							PGPOCipherAlgorithm( context,
								mPrefs.convAlgorithm) :
							PGPONullOption( context ),
						PGPOLastOption( context ) );
		}
	}
	else
	{
		PGPUInt32	numKeys	= 0;

		// Verify that there is at least one recipient
		
		err.pgpErr	= PGPCountKeys( recipients, &numKeys );
		if( err.IsntError( ) && numKeys == 0 )
		{
			err.pgpErr = kPGPError_UserAbort;
		}
		else
		{
			err.pgpErr = PGPAppendOptionList( context, encodeOptions,
					PGPOEncryptToKeySet( context, recipients ),
					PGPOLastOption( context ) );
		}
	}

	// Setup ASCII armoring
	if( haveFileOptions && err.IsntError() )
	{
		if( ( settings & kPGPUISettingsTextOutput ) != 0 )
		{
			err.pgpErr = PGPAppendOptionList( context, encodeOptions,
					PGPOArmorOutput( context, TRUE ),
					PGPOLastOption( context ) );
		}
	}
	
	// Setup preferred algorithms
	if( err.IsntError() && IsntNull( mPrefs.allowedAlgorithms ) )
	{
		err.pgpErr = PGPAppendOptionList( context, encodeOptions,
						PGPOPreferredAlgorithms( context,
								mPrefs.allowedAlgorithms,
								mPrefs.numAllowedAlgorithms ),
						PGPOLastOption( context ) );
	}
	
	return( err );
}

	CToolsError
CPGPtoolsTaskList::SetupSigningOptions(
	PGPContextRef				context,
	PGPGetPassphraseSettings	settings,
	PGPGetPassphraseOptions		options,
	PGPOptionListRef			encodeOptions)
{
	CToolsError	err;
	Boolean		haveFileOptions;
	
	haveFileOptions	= ( options &
						kPGPGetPassphraseOptionsHideFileOptions ) == 0;
						
	// Setup ASCII armoring
	if( haveFileOptions && err.IsntError() )
	{
		if( ( settings & kPGPUISettingsTextOutput ) != 0 )
		{
			err.pgpErr = PGPAppendOptionList( context, encodeOptions,
					PGPOArmorOutput( context, TRUE ),
					PGPOLastOption( context ) );
		}
	}
	
	return( err );
}

	CToolsError
CPGPtoolsTaskList::SetupCommonOptions(
	PGPContextRef		context,
	PGPOptionListRef	encodeOptions)
{
	CToolsError		err;

	if( err.IsntError() )
	{
		err.pgpErr = PGPAppendOptionList( context, encodeOptions,
					PGPOVersionString( context, pgpVersionHeaderString ),
					PGPOCommentString( context, mPrefs.comment ),
					PGPOLastOption( context ) );
	}
	
	return( err );
}


/*____________________________________________________________________________
	Return TRUE if at least one of the recipients has a corresponding private
	key.
____________________________________________________________________________*/
	Boolean
CPGPtoolsTaskList::RecipientsIncludeSelf(
	PGPKeySetRef	recipients)
{
	PGPError		err;
	Boolean			havePrivateKey	= FALSE;
	PGPKeyListRef	keyListRef;
	
	err = PGPOrderKeySet( recipients, kPGPAnyOrdering, &keyListRef );
	if( IsntPGPError( err ) )
	{
		PGPKeyIterRef	keyIterator;
	
		err = PGPNewKeyIter( keyListRef, &keyIterator );
		
		if( IsntPGPError( err ) )
		{
			PGPKeyRef	theKey;
			
			err = PGPKeyIterNext( keyIterator, &theKey );
			while( IsntErr( err ) )
			{
				PGPBoolean	isSecret;
				
				err	= PGPGetKeyBoolean( theKey,
						kPGPKeyPropIsSecret, &isSecret );
				if ( IsPGPError( err ) )
					break;
				if ( isSecret )
				{
					havePrivateKey	= TRUE;
					break;
				}
					
				err = PGPKeyIterNext( keyIterator, &theKey );
			}
			if( err == kPGPError_EndOfIteration )
				err = kPGPError_NoErr;
				
			PGPFreeKeyIter( keyIterator );
		}
		
		PGPFreeKeyList( keyListRef );
	}
	
	return( havePrivateKey );
}


	CToolsError
CPGPtoolsTaskList::ConfirmationDialogs(
	CEncodeParams const *	params )
{
	CToolsError	err;
	
	if ( ! mTaskListHasFileTasks )
		return( err );
		
	if ( err.IsntError() )
	{
		Boolean		macBinaryIsOff	=
			( params->encodingFlags & kPGPLocalEncoding_Force ) == 0;
		
		if ( macBinaryIsOff && ! params->detachedSignature )
		{
			err.pgpErr	= ConfirmMacBinary( );
		}
	}
	
	if ( mPrefs.warnOnWipe )
	{
		if ( err.IsntError() )
		{
			if ( params->wipeOriginal )
			{
				err.pgpErr = ConfirmWipe( params->encrypting,
						params->signing );
			}
		}
		
		if ( err.IsntError() )
		{
			if ( (! params->encryptingToSelf) &&
					params->wipeOriginal &&
					(! params->conventionalEncrypt) )
			{
				err.pgpErr	= ConfirmNotEncryptingToSelf();
			}
		}
	}
	
	return( err );
}


	void
CPGPtoolsTaskList::ProcessEncryptSignTasks(
	PGPContextRef				context,
	PGPKeySetRef				*allKeys,		/* I/O Parameter */
	CPGPtoolsTaskProgressDialog	*progressDialogObj)
{
	PGPKeySetRef				recipients	= kInvalidPGPKeySetRef;
	CToolsError					err;
	PGPRecipientSettings		recipientSettings;
	PGPRecipientOptions			recipientOptions;
	PGPGetPassphraseSettings	getPassphraseSettings;
	PGPGetPassphraseOptions		getPassphraseOptions;
	CEncodeParams				state;
	Boolean						haveEncodingFlags	= FALSE;
	
	pgpAssert( 	mOperation == kPGPtoolsEncryptOperation ||
				mOperation == kPGPtoolsSignOperation ||
				mOperation == kPGPtoolsEncryptSignOperation );
	
	recipientOptions 		= kPGPRecipientOptionsDefault;
	getPassphraseOptions	= kPGPGetPassphraseOptionsDefault;
	getPassphraseSettings	= 0;
	
	if( ! mTaskListHasFileTasks )
	{
		recipientOptions 		|= kPGPRecipientOptionsHideFileOptions;
		getPassphraseOptions	|= kPGPGetPassphraseOptionsHideFileOptions;
	}
	
	pgpClearMemory( &state, sizeof( state ) );
	
	state.progressDialog	= progressDialogObj;
	state.allKeys			= *allKeys;
	state.encrypting		= ( mOperation == kPGPtoolsEncryptOperation ||
								mOperation == kPGPtoolsEncryptSignOperation );
	state.signing			= ( mOperation == kPGPtoolsSignOperation ||
								mOperation == kPGPtoolsEncryptSignOperation );
	state.encodingFlags		= kPGPLocalEncoding_Force;

	err.pgpErr = PGPNewOptionList( context, &state.encodeOptions);
	if ( err.IsntError() )
	{
		if( state.encrypting)
		{
			PGPBoolean		didShowDialog;
			PGPKeySetRef	newKeys;
			
			// Hide the passphrase file options if we're encrypting
			getPassphraseOptions |= kPGPGetPassphraseOptionsHideFileOptions;

			// Show the PGPRecipientDialog
			UDesktop::Deactivate();
				err.pgpErr = PGPRecipientDialog( context, *allKeys, 0, nil,
							recipientOptions,
							kPGPRecipientSettingsDefault,
							&recipientSettings, &recipients, 
							&didShowDialog, &newKeys );
			UDesktop::Activate();
			
			if( err.IsntError() && PGPKeySetRefIsValid( newKeys ) )
			{
				if( didShowDialog )
				{
					PGPKeySetRef	importSet;
					char			prompt[256];
					
					GetIndCString( prompt, kDialogStringsListResID,
							kSelectiveImportUpdatedServerPromptStrIndex );
					
					err.pgpErr = PGPSelectiveImportDialog( context, prompt,
							newKeys, *allKeys, &importSet );
					if( err.IsntError() )
					{
						(void) PGPFreeKeySet( *allKeys );
						*allKeys = kInvalidPGPKeySetRef;
						
						err.pgpErr = PGPSharedAddKeysToDefaultKeyring(
									importSet );
						if( err.IsntError() )
						{
							err.pgpErr = PGPOpenDefaultKeyRings( context,
										0, allKeys );
							
							state.allKeys = *allKeys;
						}
						
						(void) PGPFreeKeySet( importSet );
					}
					else if( err.pgpErr == kPGPError_UserAbort )
					{
						err.pgpErr = kPGPError_NoErr;
					}
				}
				
				(void) PGPFreeKeySet( newKeys );
			}
			
			if( err.IsntError() )
			{
				state.encodingFlags	=
					PGPUISettingsToLocalEncoding( recipientSettings );
				haveEncodingFlags	= TRUE;
				
				state.conventionalEncrypt = ( recipientSettings &
							kPGPRecipientSettingsConvEncrypt ) != 0;
					
				state.wipeOriginal	=
				(recipientSettings & kPGPRecipientSettingsWipeOriginal) != 0;
					
				err = SetupEncryptionOptions( context, recipients,
							recipientSettings, recipientOptions,
							state.encodeOptions );
			}
			
			if ( err.IsntError() && ! state.conventionalEncrypt )
			{
				state.encryptingToSelf	= RecipientsIncludeSelf( recipients );
			}
		}
	}
	
	if( err.IsntError() && state.signing )
	{
		char	prompt[256];
		
		GetIndCString( prompt, kDialogStringsListResID,
				kPleaseEnterSigningPassphrasePromptStrIndex );

		UDesktop::Deactivate();
			err.pgpErr = PGPGetSigningPassphraseDialog( context,
						*allKeys, prompt,
						getPassphraseOptions,
						kPGPGetPassphraseSettingsDetachedSig,
						NULL,
						state.signingPassphrase,
						&getPassphraseSettings,
						&state.signingKey );
		UDesktop::Activate();
		
		if( err.IsntError() )
		{
			if ( ! haveEncodingFlags )
			{
				state.encodingFlags	=
					PGPUISettingsToLocalEncoding( getPassphraseSettings );
				haveEncodingFlags	= TRUE;
			}
					
			err = SetupSigningOptions( context, getPassphraseSettings,
						getPassphraseOptions, state.encodeOptions );
		}
	}
	
	if( err.IsntError() )
	{
		if( ( recipientSettings & kPGPUISettingsTextOutput ) != 0 ||
			( getPassphraseSettings & kPGPUISettingsTextOutput ) != 0 )
		{
			state.textOutput = TRUE;
		}
		
		if( (getPassphraseSettings &
				kPGPGetPassphraseSettingsDetachedSig ) != 0 )
		{
			pgpAssert( state.signing );
			state.detachedSignature = TRUE;
		}
		
		err = SetupCommonOptions( context, state.encodeOptions );
	}
	
	if ( err.IsntError() )
	{
		err	= ConfirmationDialogs( &state );
	}
	
	if( err.IsntError() )
	{
		UInt32	numTasks;
		
		progressDialogObj->SetProgressOperation( mOperation );

		numTasks = mTaskList->GetCount();
		for( UInt32 taskIndex = 1; taskIndex <= numTasks; taskIndex++ )
		{
			CPGPtoolsCryptoTask	*task;
			
			mTaskList->FetchItemAt( taskIndex, task );

			err = task->EncryptSign( context, &state );

			if( err.IsError() )
				break;
		}
	}

	if( ShouldReportError( err ) )
	{
		Str255	errorStr;
		
		GetErrorString( err, errorStr );
		
		SysBeep( 1 );
		CWarningAlert::Display( kWAStopAlertType, kWAOKStyle,
			kErrorStringListResID, kOperationCouldNotBeCompletedStrIndex,
			errorStr );
	}
		
	if( PGPKeySetRefIsValid( recipients ) )
		PGPFreeKeySet( recipients );
}

	void
CPGPtoolsTaskList::ProcessDecryptVerifyTasks(
	PGPContextRef				context,
	PGPKeySetRef 				allKeys,
	CPGPtoolsTaskProgressDialog	*progressDialogObj)
{
	CToolsError					err;
	UInt32						numTasks;
	CDecodeParams				state;
	CPGPtoolsPassphraseList		passphraseList;
	
	pgpAssert( mOperation == kPGPtoolsDecryptVerifyOperation );
	
	CResultsWindow::NewSummary();
	
	state.passphraseList 	= &passphraseList;
	state.progressDialog	= progressDialogObj;
	state.allKeys			= allKeys;
	
	numTasks = mTaskList->GetCount();
	for( UInt32 taskIndex = 1; taskIndex <= numTasks; taskIndex++ )
	{
		CPGPtoolsCryptoTask	*task;
		
		mTaskList->FetchItemAt( taskIndex, task );
		
		err = task->DecryptVerify( context, &state );

		if( err.IsError( ) )
			break;
	}
	
	if( ShouldReportError( err ) )
	{
		Str255	errorStr;
		
		GetErrorString( err, errorStr );
		
		SysBeep( 1 );
		CWarningAlert::Display( kWAStopAlertType, kWAOKStyle,
			kErrorStringListResID, kOperationCouldNotBeCompletedStrIndex,
			errorStr );
	}
}

	static void
GetConfirmWipeStrIndex(
	Boolean		encrypting,
	Boolean		signing,
	Boolean		multipleFiles,
	StringPtr	str )
{
	short	strIndex	= 0;
	
	if ( encrypting && signing )
	{
		strIndex	= multipleFiles ?
			kConfirmWipingItemsAfterEncryptingSigningStrIndex :
			kConfirmWipingItemAfterEncryptingSigningStrIndex;
	}
	else if ( encrypting )
	{
		strIndex	= multipleFiles ?
			kConfirmWipingItemsAfterEncryptingStrIndex :
			kConfirmWipingItemAfterEncryptingSigningStrIndex;
	}
	else if ( signing )
	{
		strIndex	= multipleFiles ?
			kConfirmWipingItemsAfterSigningStrIndex :
			kConfirmWipingItemAfterSigningStrIndex;
	}
	else
	{
		strIndex	= multipleFiles ?
			kConfirmWipingItemsStrIndex :
			kConfirmWipingItemStrIndex;
	}
	
	GetIndString( str, kDialogStringsListResID, strIndex );
}
	
/*____________________________________________________________________________
	The user has requested a wipe operation either directly via the wipe
	command or indirectly by encrypting or signing.
____________________________________________________________________________*/
	PGPError
CPGPtoolsTaskList::ConfirmWipe(
	Boolean	encrypting,
	Boolean	signing)
{
	PGPError	err	= kPGPError_NoErr;
	MessageT	alertResult;
	Str255		str;
			
	if ( mTaskList->GetCount() > 1 )
	{
		GetConfirmWipeStrIndex( encrypting, signing, TRUE, str );
		alertResult	= CWarningAlert::Display( kWACautionAlertType,
						kWAOKCancelStyle, str );
	}
	else
	{
		CPGPtoolsWipeTask *	task;
		Str255				itemName;
		
		mTaskList->FetchItemAt( 1, task );
		task->GetTaskItemName( itemName );
		
		GetConfirmWipeStrIndex( encrypting, signing, FALSE, str );
		
		alertResult	= CWarningAlert::Display( kWACautionAlertType,
						kWAOKCancelStyle, str, itemName );
	}
	
	err	= (alertResult == msg_Cancel) ? kPGPError_UserAbort : kPGPError_NoErr;
	
	return( err );
}


	PGPError
CPGPtoolsTaskList::ConfirmMacBinary(  )
{
	PGPError		err	= kPGPError_NoErr;
	MessageT		alertResult;
	
	alertResult	= CWarningAlert::Display( kWACautionAlertType,
					kWAOKCancelStyle,
					kDialogStringsListResID, kConfirmMacBinaryOffStrIndex );
	
	err	= (alertResult == msg_Cancel) ? kPGPError_UserAbort : kPGPError_NoErr;
	
	return( err );
}

/*____________________________________________________________________________
	Confirm that the user doesn't want to encrypt to himself.
	
	Should be called if the originals are being wiped.
____________________________________________________________________________*/
	PGPError
CPGPtoolsTaskList::ConfirmNotEncryptingToSelf(  )
{
	PGPError		err	= kPGPError_NoErr;
	MessageT		alertResult;
	
	alertResult	= CWarningAlert::Display( kWACautionAlertType, kWAOKCancelStyle,
		kDialogStringsListResID, kConfirmNotEncryptingToSelf );
	
	err	= (alertResult == msg_Cancel) ? kPGPError_UserAbort : kPGPError_NoErr;
	
	return( err );
}


	void
CPGPtoolsTaskList::ProcessWipeTasks(
	CPGPtoolsTaskProgressDialog	*progressDialogObj)
{
	CToolsError		err;
	
	pgpAssert( mOperation == kPGPtoolsWipeOperation );
	
	if( mPrefs.warnOnWipe )
		err.pgpErr	= ConfirmWipe( FALSE, FALSE );
	if ( err.IsntError() )
	{
		UInt32	numTasks	= 0;
		
		numTasks = mTaskList->GetCount();
		for( UInt32 taskIndex = 1; taskIndex <= numTasks; taskIndex++ )
		{
			CPGPtoolsWipeTask	*task;
			
			mTaskList->FetchItemAt( taskIndex, task );
			
			err = task->Wipe( progressDialogObj );

			if( err.IsError( ) )
				break;
		}
	}
}


	OSStatus
CPGPtoolsTaskList::CalcProgressBytes(
	PGPContextRef		context,
	ByteCount *			progressBytes)
{
	UInt32		numTasks;
	OSStatus	err	= noErr;
	
	*progressBytes	= 0;
	
	numTasks = mTaskList->GetCount();
	for( UInt32 taskIndex = 1; taskIndex <= numTasks; taskIndex++ )
	{
		CPGPtoolsTask	*task;
		
		mTaskList->FetchItemAt( taskIndex, &task );
		if( IsntNull( task ) )
		{
			ByteCount	count;
			
			err	= task->CalcProgressBytes( context, &count );
			if ( IsErr( err ) )
				break;
				
			*progressBytes += count;
		}
	}
	
	return( err );
}

	PGPError
CPGPtoolsTaskList::BeginTasks(
	PGPContextRef 	context)
{
	(void) context;

	return( kPGPError_NoErr );
}


	void
CPGPtoolsTaskList::LoadCurrentPrefs()
{
	PGPPrefRef		prefRef;
	CToolsError		err;
	PFLContextRef	pflContext;
	
	// Get comment string, if any
	err.pgpErr	= PFLNewContext( &pflContext );
	if ( err.IsntError() )
	{
	#if PGP_BUSINESS_SECURITY	// [
		if( IsntPGPError( PGPOpenAdminPrefs( pflContext, &prefRef ) ) )
		{
			// Comments are not always present. Don't check for errors.
			(void) PGPGetPrefStringBuffer( prefRef, kPGPPrefComments,
					sizeof( mPrefs.comment ), mPrefs.comment );

			(void) PGPClosePrefFile( prefRef );
		}
	#endif	// ] PGP_BUSINESS_SECURITY

		
		err.pgpErr = PGPOpenClientPrefs( pflContext, &prefRef );
		if( err.IsntError() )
		{
			if ( strlen( mPrefs.comment ) == 0 )
			{
				// Comments are not always present. Don't check for errors.
				(void) PGPGetPrefStringBuffer( prefRef, kPGPPrefComment,
						sizeof( mPrefs.comment ), mPrefs.comment );
			}
			
			err.pgpErr	= PGPGetPrefBoolean( prefRef,
				kPGPPrefWarnOnWipe, &mPrefs.warnOnWipe );

			if( err.IsntError() )
			{
				PGPUInt32	algorithm;
				
				err.pgpErr = PGPGetPrefNumber( prefRef,
							   kPGPPrefPreferredAlgorithm, &algorithm );

				mPrefs.convAlgorithm = (PGPCipherAlgorithm) algorithm;
			}

			if( err.IsntError() )
			{
				PGPSize 	prefSize;
				void 		*prefData;
				
				if( IsntPGPError( PGPGetPrefData( prefRef,
							   kPGPPrefAllowedAlgorithmsList,
							   &prefSize, &prefData ) ) &&
					prefSize > 0 )
				{
					mPrefs.allowedAlgorithms =
								(PGPCipherAlgorithm *) pgpAlloc( prefSize );
					if( IsntNull( mPrefs.allowedAlgorithms ) )
					{
						mPrefs.numAllowedAlgorithms = prefSize /
									sizeof( PGPCipherAlgorithm );
						
						pgpCopyMemory( prefData, mPrefs.allowedAlgorithms,
								prefSize );
					}
					else
					{
						err.pgpErr = kPGPError_OutOfMemory;
					}
					
					(void) PGPDisposePrefData( prefRef, prefData );
				}
			}

			(void) PGPClosePrefFile( prefRef );
		}
		
		PFLFreeContext( pflContext );
	}
}


	void
CPGPtoolsTaskList::ProcessTasks(PGPContextRef context)
{
	CToolsError	err;
	ByteCount	progressBytes;
	
	// Note: Since the error messages are context sensitive,
	// all errors are reported inside the respective task object.
	
	if( IsNull( mTaskList ) || mTaskList->GetCount() == 0 )
		return;
		
	CPGPStDialogHandler			dialogHandler( kProgressDialogResID, nil );
	CPGPtoolsTaskProgressDialog	*dialogObj;
	
	LoadCurrentPrefs();
	
	dialogObj = (CPGPtoolsTaskProgressDialog *)
				dialogHandler.GetDialog();
	dialogObj->SetDialogHandler( &dialogHandler );
	dialogObj->SetProgressOperation( mOperation );
	err.err	= CalcProgressBytes( context, &progressBytes);
	dialogObj->SetTotalOperations( progressBytes );
	
	if ( err.IsntError( ) )
	{
		if ( mOperation != kPGPtoolsWipeOperation )
		{
			PGPKeySetRef 	keySet;
			
			err.pgpErr = PGPOpenDefaultKeyRings( context, 0, &keySet );
			if( err.IsntError() )
			{
				err.err	= BeginTasks( context );
				if( err.IsntError() )
				{
					switch( mOperation )
					{
						case kPGPtoolsEncryptOperation:
						case kPGPtoolsSignOperation:
						case kPGPtoolsEncryptSignOperation:
							ProcessEncryptSignTasks( context,
								&keySet, dialogObj );
							break;
							
						case kPGPtoolsDecryptVerifyOperation:
							ProcessDecryptVerifyTasks( context,
								keySet, dialogObj );
							break;
							
						default:
							pgpDebugMsg( "ProcessTasks(): Invalid operation" );
							break;
					}
				}
				
				/* Could be NULL after calling ProcessEncryptSignTasks() */
				if( PGPKeySetRefIsValid( keySet ) )
					PGPFreeKeySet( keySet );
			}
			
			DeleteTasks();
			
			delete( mTaskList );
			mTaskList = NULL;
		}
		else
		{
			ProcessWipeTasks( dialogObj );
		}
	}
	
	dialogObj->SetPercentComplete( 100 );
	dialogObj->Hide();
}





	





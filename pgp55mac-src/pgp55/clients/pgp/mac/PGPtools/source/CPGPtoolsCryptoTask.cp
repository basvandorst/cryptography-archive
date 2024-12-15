/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <UModalDialogs.h>

#include "CPGPStDialogHandler.h"
#include "CSecureMemory.h"
#include "CWarningAlert.h"
#include "MacEvents.h"
#include "MacStrings.h"
#include "MacFiles.h"
#include "MacErrors.h"
#include "pgpKeys.h"

#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"
#include "PGPSharedEncryptDecrypt.h"
#include "pgpUtilities.h"
#include "PGPtoolsEncryptDecrypt.h"

#include "CPGPtoolsCryptoTask.h"
#include "CPGPtoolsTaskProgressDialog.h"
#include "CPGPtoolsPassphraseList.h"

CPGPtoolsCryptoTask::CPGPtoolsCryptoTask(PGPtoolsOperation operation)
	: CPGPtoolsTask( operation )
{
	mContext = NULL;
	
	mBadPassphraseCount			= 0;
	mDataType					= kPGPAnalyze_Unknown;
	mRecipientsKeySet			= kInvalidPGPKeySetRef;
	mNumMissingRecipientKeys	= 0;
	
	mSignatureDataValid = FALSE;
	pgpClearMemory( &mSignatureData, sizeof( mSignatureData ) );
}

CPGPtoolsCryptoTask::~CPGPtoolsCryptoTask(void)
{
	if( PGPKeySetRefIsValid( mRecipientsKeySet ) )
		(void) PGPFreeKeySet( mRecipientsKeySet );
}



	PGPError
CPGPtoolsCryptoTask::GenericEventHandler(
	PGPEvent *		event,
	CEventHandlerData *data)
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( event->type == kPGPEvent_NullEvent )
	{
		err	= NullEvent( event, data );
	}
	
	return( err );
}


	PGPError
CPGPtoolsCryptoTask::sEncodeEventHandlerProc(
	PGPContextRef	context,
	PGPEvent 		*event,
	PGPUserValue 	userValue)
{
	CEncodeEventHandlerData *	data = (CEncodeEventHandlerData *)userValue;
	
	(void)context;
	pgpAssert( IsntNull( data )  );
	pgpAssert( data->mMagic == CEncodeEventHandlerData::kMagic );
	
	return( data->mTask->EncodeEventHandler( event, data) );
}

	PGPError
CPGPtoolsCryptoTask::EncodeEventHandler(
	PGPEvent *					event,
	CEncodeEventHandlerData *	data)
{
	PGPError	err	= kPGPError_NoErr;
	
	err	= GenericEventHandler( event, data );
	
	return( err );
}


	PGPError
CPGPtoolsCryptoTask::NullEvent(
	PGPEvent *			event,
	CEventHandlerData *	data)
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( IsntNull( data->mParams->progressDialog ) )
	{
		OSStatus	macErr;
		PGPSize		bytesProcessed;
		PGPSize		totalBytes;
		
		bytesProcessed	= event->data.nullData.bytesWritten;
		totalBytes		= event->data.nullData.bytesTotal;
		
		macErr	= DoProgress( data->mParams->progressDialog,
					bytesProcessed, totalBytes );
		
		err	= MacErrorToPGPError( macErr );
	}
	
	return( err );
}
								

	PGPError
CPGPtoolsCryptoTask::DecodePassphraseEvent(
	PGPEvent *					event,
	CDecodeEventHandlerData	*	data)
{
	PGPError			err = kPGPError_NoErr;
	Boolean				askForPassphrase;
	CSecureCString256	passphrase;
	
	askForPassphrase = TRUE;
	
	// Try cached passphrases first
	if( IsntNull( data->passphraseIterator ) )
	{
		PassphraseListItem	passphraseItem;
		
		if( data->passphraseIterator->Next( &passphraseItem ) )
		{
			askForPassphrase = FALSE;
			CopyCString( passphraseItem.passphrase, passphrase );
		}
		else
		{
			data->passphraseIterator->ResetTo( 0 );
		}
	}
	
	if( askForPassphrase )
	{
		Str255		prompt;
		
		if( mBadPassphraseCount > 0 )
		{
			Str255	temp;
			
			SysBeep( 1 );
			
			GetIndString( prompt, kDialogStringsListResID,
					kIncorrectPassphrasePrefixStrIndex );
			GetDecryptPassphrasePromptString( temp );
			PrintPString( prompt, prompt, temp );
		}
		else
		{
			GetDecryptPassphrasePromptString( prompt );
		}

		PToCString( prompt, (char *) prompt );
		
		if( event->data.passphraseData.fConventional )
		{
			err = PGPGetConventionalDecryptionPassphraseDialog( mContext,
					(char const *) prompt, passphrase );
		}
		else
		{
			PGPKeyRef	whichKey;
			
			err = PGPSharedGetIndKeyFromKeySet(
							event->data.passphraseData.keyset,
							kPGPAnyOrdering, 1, &whichKey );
			/* 
			   If there aren't any keys in the key set, then return
			   the fact that the needed secret key cound not be found
			*/
			if( err == kPGPError_ItemNotFound )
				err = kPGPError_SecretKeyNotFound;
				
			if( IsntPGPError( err ) )
			{				
				err = PGPGetDecryptionPassphraseDialog( mContext,
							(char const *) prompt,
							mRecipientsKeySet, mNumMissingRecipientKeys,
							whichKey, passphrase, &whichKey );
			}
		}
		
		if( IsntPGPError( err ) )
		{
			CDecodeParams *params = (CDecodeParams *)data->mParams;
			
			params->passphraseList->RememberPassphrase( mContext, passphrase );
		}
		
		++mBadPassphraseCount;
	}
	
	if( IsntPGPError( err ) )
	{
		err = PGPAddJobOptions( event->job,
					PGPOPassphrase( mContext, passphrase ),
					PGPOLastOption( mContext ) );
	}

	return( err );
}

	PGPError
CPGPtoolsCryptoTask::DecodeRecipientsEvent(
	PGPEvent *					event,
	CDecodeEventHandlerData	*	data)
{
	PGPError	err 			= kPGPError_NoErr;
	PGPUInt32	numFoundKeys	= 0;
	
	(void) data;
	
	mRecipientsKeySet = event->data.recipientsData.recipientSet;
	if( PGPKeySetRefIsValid( mRecipientsKeySet ) )
	{
		err = PGPIncKeySetRefCount( mRecipientsKeySet );
		if( IsntPGPError( err ) )
		{
			err = PGPCountKeys( mRecipientsKeySet, &numFoundKeys );
		}
	}
	
	if( numFoundKeys > event->data.recipientsData.keyCount )
		numFoundKeys = event->data.recipientsData.keyCount;
		
	mNumMissingRecipientKeys = event->data.recipientsData.keyCount -
									numFoundKeys;
	
	return( err );
}

	PGPError
CPGPtoolsCryptoTask::DecodeSignatureEvent(
	PGPEvent 					*event,
	CDecodeEventHandlerData		*data)
{
	PGPError	err	= kPGPError_NoErr;
	
	(void) data;
	
	mSignatureData.data 				= event->data.signatureData;
	mSignatureData.signingKeyIDStr[0]	= 0;
	mSignatureDataValid 				= TRUE;
	
	(void)PGPGetKeyIDString( 
			&mSignatureData.data.signingKeyID,
			kPGPKeyIDString_Abbreviated,
			mSignatureData.signingKeyIDStr );
	
	return( kPGPError_NoErr );
}


	PGPError
CPGPtoolsCryptoTask::DecodeAnalyzeEvent(
	PGPEvent 					*event,
	CDecodeEventHandlerData		*data)
{
	(void) data;
	
	if( mDataType == kPGPAnalyze_Unknown &&
		event->data.analyzeData.sectionType != kPGPAnalyze_Unknown &&
		event->data.analyzeData.sectionType != kPGPAnalyze_Key )
	{
		mDataType = event->data.analyzeData.sectionType;
	}
	
	return( kPGPError_NoErr );
}

	PGPError
CPGPtoolsCryptoTask::DecodeEventHandler(
	PGPEvent *					event,
	CDecodeEventHandlerData *	data)
{
	PGPError		err = kPGPError_NoErr;
	CDecodeParams *	params	= (CDecodeParams *)data->mParams;
	
	switch( event->type )
	{
		default:
		{
			err	= GenericEventHandler( event, data );
			break;
		}
		
		case kPGPEvent_InitialEvent:
		{
			mBadPassphraseCount = 0;
			mDataType			= kPGPAnalyze_Unknown;
			
			if( IsntNull( params->passphraseList ) )
			{
				data->passphraseIterator =
					new LArrayIterator( *(params->passphraseList) );
			}

			break;
		}
		
		case kPGPEvent_AnalyzeEvent:
		{
			err = DecodeAnalyzeEvent( event, data );
			break;
		}

		case kPGPEvent_PassphraseEvent:
		{
			err = DecodePassphraseEvent( event, data );
			break;
		}

		case kPGPEvent_RecipientsEvent:
		{
			err = DecodeRecipientsEvent( event, data );
			break;
		}
		
		case kPGPEvent_SignatureEvent:
		{
			err = DecodeSignatureEvent( event, data );
			break;
		}

		
		case kPGPEvent_FinalEvent:
		{
			delete data->passphraseIterator;
			data->passphraseIterator	= NULL;
			break;
		}
	}
	
	return( err );
}

	PGPError
CPGPtoolsCryptoTask::sDecodeEventHandlerProc(
	PGPContextRef 	context,
	PGPEvent 		*event,
	PGPUserValue 	userValue)
{
	PGPError					err;
	CDecodeEventHandlerData *	data	= (CDecodeEventHandlerData *)userValue;
	
	(void)context;
	pgpAssert( IsntNull( data )  );
	pgpAssert( data->mMagic == CDecodeEventHandlerData::kMagic );
	
	err = data->mTask->DecodeEventHandler( event, data );
	
	return( err );
}





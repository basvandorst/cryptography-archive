/*____________________________________________________________________________	Copyright (C) 1997-1998 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CPGPtoolsCryptoTask.cp,v 1.32 1999/03/10 02:37:56 heller Exp $____________________________________________________________________________*/#include <Sound.h>#include <LArrayIterator.h>#include <UDesktop.h>#include <UModalDialogs.h>#include "CPGPStDialogHandler.h"#include "CSecureMemory.h"#include "CWarningAlert.h"#include "MacEvents.h"#include "MacStrings.h"#include "MacFiles.h"#include "MacErrors.h"#include "pgpKeys.h"#include "PGPtoolsResources.h"#include "PGPtoolsUtils.h"#include "PGPSharedEncryptDecrypt.h"#include "pgpUtilities.h"#include "PGPtoolsEncryptDecrypt.h"#include "pgpClientLib.h"#include "CPGPtoolsCryptoTask.h"#include "CPGPtoolsTaskProgressDialog.h"#include "CPGPtoolsPassphraseList.h"CPGPtoolsCryptoTask::CPGPtoolsCryptoTask(PGPtoolsOperation operation)	: CPGPtoolsTask( operation ){	mContext 	= kInvalidPGPContextRef;	mTLSContext	= kInvalidPGPtlsContextRef;		mBadPassphraseCount		= 0;	mDataType				= kPGPAnalyze_Unknown;	mSignatureDataValid 	= FALSE;		pgpClearMemory( &mSignatureData, sizeof( mSignatureData ) );}CPGPtoolsCryptoTask::~CPGPtoolsCryptoTask(void){}	PGPErrorCPGPtoolsCryptoTask::GenericEventHandler(	PGPEvent *		event,	CEventHandlerData *data){	PGPError	err	= kPGPError_NoErr;		if ( event->type == kPGPEvent_NullEvent )	{		err	= NullEvent( event, data );	}		return( err );}	PGPErrorCPGPtoolsCryptoTask::sEncodeEventHandlerProc(	PGPContextRef	context,	PGPEvent 		*event,	PGPUserValue 	userValue){	CEncodeEventHandlerData *	data = (CEncodeEventHandlerData *)userValue;		(void)context;	pgpAssert( IsntNull( data )  );	pgpAssert( data->mMagic == CEncodeEventHandlerData::kMagic );		return( data->mTask->EncodeEventHandler( event, data) );}	PGPErrorCPGPtoolsCryptoTask::EncodeEventHandler(	PGPEvent *					event,	CEncodeEventHandlerData *	data){	PGPError	err	= kPGPError_NoErr;		err	= GenericEventHandler( event, data );		return( err );}	PGPErrorCPGPtoolsCryptoTask::NullEvent(	PGPEvent *			event,	CEventHandlerData *	data){	PGPError	err	= kPGPError_NoErr;		if ( IsntNull( data->mParams->progressDialog ) )	{		OSStatus	macErr;		PGPSize		bytesProcessed;		PGPSize		totalBytes;				bytesProcessed	= event->data.nullData.bytesWritten;		totalBytes		= event->data.nullData.bytesTotal;				macErr	= DoProgress( data->mParams->progressDialog,					bytesProcessed, totalBytes );				err	= MacErrorToPGPError( macErr );	}		return( err );}									PGPErrorCPGPtoolsCryptoTask::DecodePassphraseEvent(	PGPEvent *					event,	CDecodeEventHandlerData	*	data){	PGPError			err = kPGPError_NoErr;	Boolean				askForPassphrase;	CSecureCString256	passphrase;		askForPassphrase = TRUE;	// Try cached passphrases first	if( IsntNull( data->passphraseIterator ) )	{		PassphraseListItem	passphraseItem;				if( data->passphraseIterator->Next( &passphraseItem ) )		{			askForPassphrase = FALSE;			CopyCString( passphraseItem.passphrase, passphrase );		}		else		{			data->passphraseIterator->ResetTo( 0 );		}	}		if( askForPassphrase )	{		uchar	*prompt = (uchar *) data->clientHandlerData.prompt;				if( mBadPassphraseCount > 0 )		{			Str255	temp;						SysBeep( 1 );						GetIndString( prompt, kDialogStringsListResID,					kIncorrectPassphrasePrefixStrIndex );			GetDecryptPassphrasePromptString( temp );			PrintPString( prompt, prompt, temp );		}		else		{			GetDecryptPassphrasePromptString( prompt );		}		PToCString( prompt, data->clientHandlerData.prompt );				UDesktop::Deactivate();			err = PGPClientDecodeEventHandler( mContext, event,						&data->clientHandlerData );		UDesktop::Activate();				if( IsntPGPError( err ) )		{			if( PGPGetPassBufferType( data->clientHandlerData.passBuffer ) ==						kPGPPassBufferType_Passphrase )			{				CDecodeParams 	*params = (CDecodeParams *)data->mParams;				char			*passphrase;								(void) PGPPassBufferGetPassphrasePtr(							data->clientHandlerData.passBuffer, &passphrase );				pgpAssert( IsntNull( passphrase ) );								params->passphraseList->RememberPassphrase( mContext,							passphrase );			}		}				++mBadPassphraseCount;	}	else	{		err = PGPAddJobOptions( event->job,					PGPOPassphrase( mContext, passphrase ),					PGPOLastOption( mContext ) );	}	return( err );}	PGPErrorCPGPtoolsCryptoTask::DecodeRecipientsEvent(	PGPEvent *					event,	CDecodeEventHandlerData	*	data){	PGPError	err = kPGPError_NoErr;		err = PGPClientDecodeEventHandler( mContext, event,				&data->clientHandlerData );		return( err );}	PGPErrorCPGPtoolsCryptoTask::DecodeSignatureEvent(	PGPEvent 					*event,	CDecodeEventHandlerData		*data){	PGPError	err;		err = PGPClientDecodeEventHandler( mContext, event,				&data->clientHandlerData );	if( IsntPGPError( err ) )	{		mSignatureData.data = data->clientHandlerData.signatureData;		mSignatureDataValid = data->clientHandlerData.signatureDataValid;				mSignatureData.signingKeyIDStr[0] = 0;				if( mSignatureDataValid )		{			(void)PGPGetKeyIDString( 					&mSignatureData.data.signingKeyID,					kPGPKeyIDString_Abbreviated,					mSignatureData.signingKeyIDStr );		}	}	return( err );}	PGPErrorCPGPtoolsCryptoTask::DecodeAnalyzeEvent(	PGPEvent 					*event,	CDecodeEventHandlerData		*data){	(void) data;		if( mDataType == kPGPAnalyze_Unknown &&		event->data.analyzeData.sectionType != kPGPAnalyze_Unknown &&		event->data.analyzeData.sectionType != kPGPAnalyze_Key )	{		mDataType = event->data.analyzeData.sectionType;	}		return( kPGPError_NoErr );}	PGPErrorCPGPtoolsCryptoTask::DecodeEventHandler(	PGPEvent *					event,	CDecodeEventHandlerData *	data){	PGPError		err = kPGPError_NoErr;	CDecodeParams *	params	= (CDecodeParams *)data->mParams;		switch( event->type )	{		default:		{			err = PGPClientDecodeEventHandler( mContext, event,						&data->clientHandlerData );			if( IsntPGPError( err ) )			{				err	= GenericEventHandler( event, data );			}						break;		}				case kPGPEvent_InitialEvent:		{			mBadPassphraseCount = 0;			mDataType			= kPGPAnalyze_Unknown;						if( IsntNull( params->passphraseList ) )			{				data->passphraseIterator =					new LArrayIterator( *(params->passphraseList) );			}			break;		}				case kPGPEvent_AnalyzeEvent:		{			err = DecodeAnalyzeEvent( event, data );			break;		}		case kPGPEvent_PassphraseEvent:		{			err = DecodePassphraseEvent( event, data );			break;		}		case kPGPEvent_RecipientsEvent:		{			err = DecodeRecipientsEvent( event, data );			break;		}				case kPGPEvent_SignatureEvent:		{			err = DecodeSignatureEvent( event, data );			break;		}				case kPGPEvent_FinalEvent:		{			delete data->passphraseIterator;			data->passphraseIterator	= NULL;			break;		}						case kPGPEvent_OutputEvent:		{			if( event->data.outputData.forYourEyesOnly )			{				data->isFYEO = true;			}						err = PGPClientDecodeEventHandler( mContext, event,						&data->clientHandlerData );			if( IsntPGPError( err ) )			{				err	= GenericEventHandler( event, data );			}						break;		}						case kPGPEvent_EndLexEvent:		{			if( data->clientHandlerData.outputBufferSize != 0 )			{				if( data->outputDataSize == 0 )				{					data->outputDataSize = data->clientHandlerData.outputBufferSize;					data->outputData = PGPNewSecureData(											PGPGetContextMemoryMgr( mContext ),											data->outputDataSize + 1,											kPGPMemoryMgrFlags_None );					if( IsNull( data->outputData ) )					{						err = kPGPError_OutOfMemory;					}					pgpCopyMemory( data->clientHandlerData.outputBuffer,								   data->outputData,								   data->outputDataSize );					( (char *) data->outputData )[data->outputDataSize] = 0;				}				else				{					PGPSize	newSize = data->outputDataSize +								data->clientHandlerData.outputBufferSize;										err = PGPReallocData( PGPGetContextMemoryMgr( mContext ),										  &data->outputData,										  newSize + 1,										  kPGPMemoryMgrFlags_None );					if( IsntPGPError( err ) )					{						pgpCopyMemory( data->clientHandlerData.outputBuffer,									   ( (char *) data->outputData) + data->outputDataSize,									   data->clientHandlerData.outputBufferSize );						( (char *) data->outputData )[newSize] = 0;						data->outputDataSize = newSize;					}				}			}						PGPError	clientHandlerErr;						clientHandlerErr = PGPClientDecodeEventHandler( mContext, event,								&data->clientHandlerData );			if( IsntPGPError( clientHandlerErr ) )			{				clientHandlerErr	= GenericEventHandler( event, data );			}						if ( IsntPGPError( err ) )			{				err = clientHandlerErr;			}						break;		}	}		return( err );}	PGPErrorCPGPtoolsCryptoTask::sDecodeEventHandlerProc(	PGPContextRef 	context,	PGPEvent 		*event,	PGPUserValue 	userValue){	PGPError					err;	CDecodeEventHandlerData *	data	= (CDecodeEventHandlerData *)userValue;		(void)context;	pgpAssert( IsntNull( data )  );	pgpAssert( data->mMagic == CDecodeEventHandlerData::kMagic );		err = data->mTask->DecodeEventHandler( event, data );		return( err );}
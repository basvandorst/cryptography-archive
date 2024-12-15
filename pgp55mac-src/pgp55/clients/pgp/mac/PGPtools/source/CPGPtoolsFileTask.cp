/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <LArray.h>
#include <LComparator.h>
#include <UDesktop.h>

#include "pgpUserInterface.h"
#include "pgpUtilities.h"

#include "MacErrors.h"
#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpMem.h"
#include "pgpUtilities.h"

#include "CPGPtoolsProcessingList.h"

#include "CPGPtoolsFileTask.h"
#include "CPGPtoolsTaskProgressDialog.h"
#include "CResultsWindow.h"
#include "PGPtoolsEncryptDecrypt.h"
#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"
#include "CPGPToolsWipeTask.h"

CPGPtoolsFileTask::CPGPtoolsFileTask(
	PGPtoolsOperation operation,
	const FSSpec *fileSpec) : CPGPtoolsCryptoTask( operation )
{
	OSStatus	err;
	CInfoPBRec	cpb;
	
	mWorkingFileSpec	= *fileSpec;
	mSourceSpec 		= *fileSpec;
	
	err	= FSpGetCatInfo( fileSpec, &cpb );
	pgpAssertNoErr( err );
	mSourceIsFolder	= cpbIsFolder( &cpb );
	
	mProcessingList	= new CPGPtoolsProcessingList();
	if ( IsntNull( mProcessingList ) )
	{
		err	= mProcessingList->BuildSourceTree( &mSourceSpec );
		pgpAssertMsg( IsntErr( err ), "error building source tree" );
		if ( IsErr( err ) )
		{
			delete mProcessingList;
			mProcessingList	= NULL;
		}
	}
}

CPGPtoolsFileTask::~CPGPtoolsFileTask(void)
{
	delete mProcessingList;
	mProcessingList	= NULL;
}

// MW cannot seem to compile this function without this option
#pragma global_optimizer on

	CToolsError
CPGPtoolsFileTask::EncryptSignFileInternal(
	CEncodeParams 			*params,
	const ProcessingInfo	*srcInfo,
	const FSSpec 			*destSpec )
{
	CToolsError			err;
	PGPOptionListRef	signingOptions	= kInvalidPGPOptionListRef;
	PGPContextRef		c	= mContext;
	
	pgpAssert( PGPContextRefIsValid( mContext ) );
	pgpAssertAddrValid( params, CEncodeParams );

	params->progressDialog->SetNewProgressItem( srcInfo->srcSpec.name );
	
	if( params->signing )
	{
		err.pgpErr = PGPBuildOptionList( c, &signingOptions, 
				PGPOSignWithKey( c, params->signingKey,
					PGPOPassphrase( c, params->signingPassphrase ),
					PGPOLastOption( c ) ),
				PGPOLastOption( c ) );
		
		if( err.IsntError() && params->detachedSignature )
		{
			err.pgpErr = PGPAppendOptionList( c, signingOptions,
						PGPODetachedSig( c,
							PGPOLastOption( c ) ),
						PGPOLastOption( c ) );
		
			/* text output should have alreday been set up */
		}
	}
	else
	{
		/* no signing options; create dummy list */
		err.pgpErr	= PGPBuildOptionList( c,
						&signingOptions, PGPOLastOption( c ) );
	}
	
	if( err.IsntError() )
	{
		CEncodeEventHandlerData	data( this, params );
		PGPLocalEncodingFlags	encodingFlags;
		
		encodingFlags = params->encodingFlags;
		if( encodingFlags == kPGPLocalEncoding_None &&
			srcInfo->dataSize == 0 &&
			srcInfo->resSize != 0 )
		{
			// MacBinary is off, but the file is a resource fork-only
			// file. This case is illogical. Turn on MacBinary for this file.
		
			encodingFlags = kPGPLocalEncoding_Force;
		}
		
		err.pgpErr = PGPEncode( c,
					PGPOInputFileFSSpec( c, &srcInfo->srcSpec ),
					PGPOOutputFileFSSpec( c, destSpec ),
					PGPODataIsASCII( c, srcInfo->fInfo.fdType == 'TEXT' ),
					signingOptions,
					PGPOptionListRefIsValid( params->encodeOptions ) ?
						params->encodeOptions : PGPONullOption( c ),
					PGPOEventHandler( c,
						sEncodeEventHandlerProc, &data ),
					PGPOSendNullEvents( c, TRUE ),
					PGPOLocalEncoding( c, encodingFlags ),
					PGPOLastOption( c  ) );
	}

	if ( PGPOptionListRefIsValid( signingOptions ) )
		PGPFreeOptionList( signingOptions );
	
	if( ShouldReportError( err ) )
	{
		err = ReportError( err,
			params->encrypting ? kFileCannotBeEncryptedStrIndex :
			kFileCannotBeSignedStrIndex );
	}

	return( err );
}

#pragma global_optimizer reset

	OSStatus
CPGPtoolsFileTask::GetOutputFolderSpec(
	Boolean			encryptSign,
	const FSSpec *	srcSpec,
	FSSpec *		destSpec )
{
	OSStatus	err	= noErr;
	const uchar	kPGPSuffix[]	= "\p PGP";
	
	*destSpec	= *srcSpec;
	
	if ( encryptSign )
	{
		if ( StrLength( destSpec->name ) +
				StrLength( kPGPSuffix ) > kMaxHFSFileNameLength )
		{
			destSpec->name[ 0 ]	=
				kMaxHFSFileNameLength - StrLength( kPGPSuffix );
		}
		AppendPString( kPGPSuffix, destSpec->name );
	}
	else
	{	/* decrypting/verifying */
		if ( PStringHasSuffix( srcSpec->name, kPGPSuffix, FALSE) )
		{
			/* strip the ".pgp" off */
			destSpec->name[ 0 ]	-= StrLength( kPGPSuffix );
		}
	}
	
	err	= FSpGetUniqueSpec( destSpec, destSpec );
	
	return( err );
}


/*____________________________________________________________________________
	Determine if the file should be processed.
	
	While it might make sense to not encrypt already encrypted items or
	encrypt detached sigs, there are also cases where you might want
	to do this. so don't prevent it.
	
	The same idea applies when signing already signed or encrypted items.
____________________________________________________________________________*/
	Boolean
CPGPtoolsFileTask::ShouldEncryptSignFile(
	Boolean			encrypting,
	const FSSpec *	spec )
{
	OSStatus		err;
	Boolean			shouldProcess	= TRUE;
	
	// FileKindInfo	info;
	// err	= GetPGPFileKindInfo( spec, &info );
	if ( encrypting )
	{
		/* without UI to ask the user, we can't reasonably forbid signing */
	}
	else
	{
		/* without UI to ask the user, we can't reasonably forbid signing */
	}
	
	if ( shouldProcess )
	{
		CInfoPBRec	cpb;
		
		err	= FSpGetCatInfo( spec, &cpb );
		if ( IsntErr( err ) )
		{
			/* don't do aliases or invisible files */
			if ( cpbFInfo( &cpb ).fdFlags & ( kIsInvisible | kIsAlias ) )
				shouldProcess	= FALSE;
		}
	}
	
	return( shouldProcess );
}
	

	CToolsError
CPGPtoolsFileTask::EncryptSignFolder(
	CEncodeParams *	params )
{
	CToolsError	err;
	FSSpec		destTreeSpec;
	
	(void)params;
	pgpAssert( IsntNull( mProcessingList ) );
	
	err.err	= mProcessingList->PrepareForIteration();
	if ( err.IsntError() && ! mProcessInPlace)
	{
		err.err	= GetOutputFolderSpec( TRUE,
					&mSourceSpec, &destTreeSpec );
		if ( err.IsntError() )
		{
			err.err	= mProcessingList->CreateDestTree( &destTreeSpec );
		}
	}
	
	if ( err.IsntError() )
	{
		UInt32	numFiles	= mProcessingList->GetNumFiles();
		
		for( UInt32 index = 0; index < numFiles; ++index )
		{
			ProcessingInfo	info;
			FSSpec			destSpec;
			
			mProcessingList->GetIndFile( index, &info );
			
			mWorkingFileSpec	= info.srcSpec;
			
			if ( mProcessInPlace )
			{
				destSpec.vRefNum	= info.srcSpec.vRefNum;
				destSpec.parID		= info.srcSpec.parID;
			}
			else
			{
				destSpec.vRefNum	= info.destVRefNum;
				destSpec.parID		= info.destParID;
			}
			if ( ShouldEncryptSignFile( params->encrypting,
					&info.srcSpec ) )
			{
				GetDefaultEncryptSignName( info.srcSpec.name,
					params->textOutput,
					params->detachedSignature, destSpec.name );
				pgpAssert( ! FSpEqual( &info.srcSpec, &destSpec ) );
				
				err	= EncryptSignFileInternal( params, &info,
					&destSpec );
			}
			else if ( ! mProcessInPlace )
			{
				CopyPString( info.srcSpec.name, destSpec.name );
				err.err	= FSpCopyFiles( &info.srcSpec, &destSpec );
				pgpAssertNoErr( err.err );
			}
			
			if ( err.IsError() )
				break;
		}
	}
	
	return( err );
}

 
	CToolsError
CPGPtoolsFileTask::EncryptSign(
	PGPContextRef	context,
	CEncodeParams *	params)
{
	CToolsError	err;

	mContext	= context;
	pgpAssert( (! params->wipeOriginal) || params->encrypting );
	
	/* send output to a duplicate folder hierarchy 
		unless we are doing detached sigs */
	mProcessInPlace	= params->detachedSignature;

	if ( mSourceIsFolder )
	{
		err	= EncryptSignFolder( params );
	}
	else
	{
		FSSpec			destSpec;
		ProcessingInfo	info;
		
		mProcessingList->GetIndFile( 0, &info );
			
		GetDefaultEncryptSignFSSpec( &info.srcSpec,
				params->textOutput,
				params->detachedSignature,
				&destSpec );

		(void)FSpGetUniqueSpec( &destSpec, &destSpec );

		err	= EncryptSignFileInternal( params, &info,
				&destSpec );
	}
	
	if ( err.IsntError() &&
		params->wipeOriginal )
	{
		CPGPtoolsWipeTask	task( &mSourceSpec );
		CPGPtoolsTaskProgressDialog * progress =
					params->progressDialog;
		ByteCount		progressBytes;
		
		progress->SetProgressOperation( kPGPtoolsWipeOperation );
		err.err	= task.CalcProgressBytes( context, &progressBytes);
		if ( err.IsntError() )
		{
			progress->SetTotalOperations( progressBytes );
			err	= task.Wipe( progress );
		}
	}

	mContext	= kInvalidPGPContextRef;
	return( err );
}




	Boolean
CPGPtoolsFileTask::ShouldDecryptVerifyFile(
	const FSSpec *	spec )
{
	OSStatus		err;
	Boolean			shouldProcess	= FALSE;
	FileKindInfo	info;
	
	err	= GetPGPFileKindInfo( spec, &info );
	
	if ( info.maybePGPFile || info.isPGPFile )
		shouldProcess	= TRUE;
	
	return( shouldProcess );
}

	CToolsError
CPGPtoolsFileTask::DecryptVerifyFileInternal(
	CDecodeParams *			params,
	const ProcessingInfo *	srcSpecInfo,
	const FSSpec *			destSpec )
{
	CToolsError			err;
	Boolean				forcePrompt	= FALSE;
	PGPOptionListRef	signingOptions	= kInvalidPGPOptionListRef;	
	
#if PGP_DEBUG
	pgpAssert( PGPContextRefIsValid( mContext ) );
	pgpAssertAddrValid( params, CEncodeParams );
	if ( IsntNull( destSpec ) )
		pgpAssert( ! FSpEqual( &srcSpecInfo->srcSpec, destSpec ) );
#endif

	params->progressDialog->SetNewProgressItem(
				srcSpecInfo->srcSpec.name );
	
	if ( ShouldDecryptVerifyFile( &srcSpecInfo->srcSpec ) )
	{
		CDecodeEventHandlerData	data( this, params );
		PGPContextRef			c	= mContext;
		PGPOptionListRef		outputOption	= kInvalidPGPOptionListRef;
		
		mWorkingFileSpec		= srcSpecInfo->srcSpec;
		
		// Assume both forks are verified
		mSignatureData.didNotVerifyResourceFork = FALSE;
		
		err.pgpErr	= PGPBuildOptionList( c, &outputOption,
						IsntNull( destSpec ) ?
							PGPOOutputFileFSSpec( c, destSpec ) :
							PGPONullOption( c ),
						PGPOLastOption( c ) );
		
		err.pgpErr = PGPDecode( c,
				PGPOInputFileFSSpec( c, &srcSpecInfo->srcSpec ),
				outputOption,
				PGPOKeySetRef( c, params->allKeys ),
				PGPOEventHandler( c, sDecodeEventHandlerProc,
							&data ),
				PGPOSendNullEvents( c, TRUE ),
				PGPOLastOption( c ) );
		
		if ( mSignatureDataValid &&
				mSignatureData.data.checked &&
				( ! mSignatureData.data.verified ) )
		{
			/*
			sig checked, but didn't verify.  Output should have been
			created. Try again with no output, existing passphrase,
			and local encoding off (controlled by 
			data.retryingSigVerification ).
			*/
			data.retryingSigVerification			= TRUE;
			mSignatureData.didNotVerifyResourceFork = TRUE;
			mSignatureDataValid						= FALSE;
			
			err.pgpErr = PGPDecode( c,
					PGPOInputFileFSSpec( c, &srcSpecInfo->srcSpec ),
					PGPODiscardOutput( c, TRUE ),
					PGPOKeySetRef( c, params->allKeys ),
					PGPOEventHandler( c, sDecodeEventHandlerProc, &data ),
					PGPOSendNullEvents( c, TRUE ),
					PGPOLastOption( c ) );
			pgpAssert( mSignatureDataValid );
		}
		
		if ( PGPOptionListRefIsValid( outputOption ) )
			PGPFreeOptionList( outputOption );
	}
	else
	{
		/* if it's a file job, don't copy it */
		/* but if it's a folder, copy non-relevant items */
		if ( mSourceIsFolder )
		{
			pgpAssert( IsntNull( destSpec ) );
			err.err	= FSpCopyFiles( &srcSpecInfo->srcSpec, destSpec );
		}
	}
	
	if( err.IsntError( ) )
	{
		if( mSignatureDataValid )
		{
			CResultsWindow::NewResult( srcSpecInfo->srcSpec.name,
						&mSignatureData);
		}
	}

	if( ShouldReportError( err ) )
	{
		err = ReportError( err, kFileCannotBeDecryptedVerifiedStrIndex );
	}

	return( err );
}


	CToolsError
CPGPtoolsFileTask::DecryptVerifyFolder(
	CDecodeParams *	params)
{
	CToolsError	err;
	FSSpec		destTreeSpec;
	
	(void)params;
	pgpAssert( IsntNull( mProcessingList ) );
	
	err.err	= mProcessingList->PrepareForIteration();
	if ( err.IsntError() && ! mProcessInPlace)
	{
		err.err	= GetOutputFolderSpec( FALSE,
					&mSourceSpec, &destTreeSpec );
		if ( err.IsntError() )
		{
			err.err	= mProcessingList->CreateDestTree( &destTreeSpec );
		}
	}
	
	if ( err.IsntError() )
	{
		UInt32	numFiles	= mProcessingList->GetNumFiles();
		
		for( UInt32 index = 0; index < numFiles; ++index )
		{
			ProcessingInfo	info;
			FSSpec			destSpec;
			
			mProcessingList->GetIndFile( index, &info );
			mWorkingFileSpec	= info.srcSpec;
			
			if ( mProcessInPlace )
			{
				destSpec.vRefNum	= info.srcSpec.vRefNum;
				destSpec.parID		= info.srcSpec.parID;
			}
			else
			{
				destSpec.vRefNum	= info.destVRefNum;
				destSpec.parID		= info.destParID;
			}
			if ( ShouldDecryptVerifyFile( &info.srcSpec ) )
			{
				GetDefaultDecryptVerifyName( info.srcSpec.name,
					destSpec.name );
				pgpAssert( ! FSpEqual( &info.srcSpec, &destSpec ) );
				
				err	= DecryptVerifyFileInternal( params,
						&info, &destSpec );
			}
			else if ( ! mProcessInPlace )
			{
				CopyPString( info.srcSpec.name, destSpec.name );
				err.err	= FSpCopyFiles( &info.srcSpec, &destSpec );
				pgpAssertNoErr( err.err );
			}
			
			if ( err.IsError() )
				break;
		}
	}
	
	return( err );
}

							
	CToolsError
CPGPtoolsFileTask::DecryptVerify(
	PGPContextRef		context,
	CDecodeParams		*params)
{
	CToolsError				err;
	
	pgpAssertAddrValid( params, CDecodeParams );
	
	mContext		= context;
	mProcessInPlace	= FALSE;
	
	if ( mSourceIsFolder )
	{
		err	= DecryptVerifyFolder( params );
		
	}
	else
	{
		ProcessingInfo	info;
		
		mProcessingList->GetIndFile( 0, &info );

		if( err.IsntError( ) )
		{
			err	= DecryptVerifyFileInternal( params, &info, NULL );
		}
	}

	mContext	= kInvalidPGPContextRef;
	return( err );
}

	void
CPGPtoolsFileTask::BuildErrorMessage(
	CToolsError 	err,
	short 		errorStrIndex,
	StringPtr	msg)
{
	Str255			errorStr;

	GetIndString( msg, kErrorStringListResID, errorStrIndex );

	PrintPString( msg, msg, mWorkingFileSpec.name );
	
	GetErrorString( err, errorStr );
	PrintPString( msg, msg, errorStr );
}


	OSStatus
CPGPtoolsFileTask::CalcProgressBytes(
	PGPContextRef	context,
	ByteCount *		progressBytes )
{
	OSStatus	err	= noErr;
	
	(void)context;
	pgpAssert( IsntNull( mProcessingList ) );
	if ( IsNull( mProcessingList ) )
		return( paramErr );
	*progressBytes	= 0;
	
	err	= mProcessingList->PrepareForIteration();
	if ( IsntErr( err ) )
	{
		UInt32	numFiles	= mProcessingList->GetNumFiles();
		
//		pgpFixBeforeShip( "get actual size from SDK call" );
		for( UInt32 index = 0; index < numFiles; ++index )
		{
			ProcessingInfo	info;
			
			mProcessingList->GetIndFile( index, &info );
			
			*progressBytes	+= info.resSize + info.dataSize;
		}
	}

	return( err );
}




	PGPError
CPGPtoolsFileTask::DecodeOutputEvent(
	PGPEvent *					event,
	CDecodeEventHandlerData	*	data)
{
	PGPError	err = kPGPError_NoErr;
	FSSpec		outSpec;

	(void)data;
		
	GetDefaultDecryptVerifyFSSpec( &mWorkingFileSpec, &outSpec );
	(void) FSpGetUniqueSpec( &outSpec, &outSpec );
	
	if( IsntPGPError( err ) )
	{
		err = PGPAddJobOptions( event->job,
					PGPOOutputFileFSSpec( mContext, &outSpec ),
					PGPOLastOption( mContext ) );
	}
	
	return( err );
}



	PGPError
CPGPtoolsFileTask::DecodeDetachedSignatureEvent(
	PGPEvent *					event,
	CDecodeEventHandlerData	*	data)
{
	PGPContextRef	c	= mContext;
	PGPError		err	= kPGPError_NoErr;
	
	/* see if there is an appropriately named source file */
	if( ! data->retryingSigVerification )
	{	/* caution: use just macErr here */
		OSStatus	macErr;
		CInfoPBRec	cpb;
	
		if ( GetDetachedSigFileSourceSpec( &mWorkingFileSpec,
			&mDetachedSigInputSpec ) )
		{
			macErr	= FSpGetCatInfo( &mDetachedSigInputSpec, &cpb );
			if ( IsntErr( macErr ) )
			{
				if ( cpbIsFolder( &cpb ) )
					macErr	= fnfErr;
			}
		}
		if ( IsErr( macErr ) )
		{
			macErr = PromptForSignatureSourceFile( &mWorkingFileSpec,
							&mDetachedSigInputSpec );
		}
		
		err = MacErrorToPGPError( macErr );
	}
	
	
	if ( IsntPGPError( err ) )
	{
		CInfoPBRec				cpb;
		PGPLocalEncodingFlags	encodingFlags;
		
		/* first we'll try MacBinary on (Force), then if we're doing
			a retry, we'll try it off */
		if ( data->retryingSigVerification )
			encodingFlags	= kPGPLocalEncoding_None;
		else
			encodingFlags	= kPGPLocalEncoding_Force;
			
		(void)FSpGetCatInfo( &mWorkingFileSpec, &cpb );
		
		/* require CRC to be valid, unless it's not created by us */
		/* at least one brain-dead program doesn't set the CRC */
		if ( cpbFileCreator( &cpb ) != kPGPMacFileCreator_Tools )
			encodingFlags	|= kPGPLocalEncoding_NoMacBinCRCOkay;
		
		(void)FSpGetCatInfo( &mDetachedSigInputSpec, &cpb );

		mSignatureData.haveResourceFork = ( cpbResForkSize( &cpb ) != 0 );
			
		err	= PGPAddJobOptions( event->job,
			PGPODetachedSig( c, 
				PGPOInputFileFSSpec( c, &mDetachedSigInputSpec ),
				PGPOLocalEncoding( c, encodingFlags),
				PGPOLastOption( c ) ),
			PGPOLastOption( c ) );
	}
	
	return( err );
}


	PGPError
CPGPtoolsFileTask::DecodeEventHandler(
	PGPEvent *					event,
	CDecodeEventHandlerData *	data)
{
	PGPError	err = kPGPError_NoErr;
	
	switch( event->type )
	{
		default:
		{
			err	= inherited::DecodeEventHandler( event, data );
			break;
		}
		
		case kPGPEvent_OutputEvent:
		{
			err = DecodeOutputEvent( event, data );
			break;
		}
		
		case  kPGPEvent_DetachedSignatureEvent:
		{
			err = DecodeDetachedSignatureEvent( event, data );
			break;
		}
	}
	
	return( err );
}



	void
CPGPtoolsFileTask::GetDecryptPassphrasePromptString(StringPtr prompt)
{
	GetIndString( prompt, kDialogStringsListResID,
						kGetPassphraseForFilePromptStrIndex );
	PrintPString( prompt, prompt, mWorkingFileSpec.name );
}

	void
CPGPtoolsFileTask::GetTaskItemName(StringPtr name)
{
	CopyPString( mWorkingFileSpec.name, name );
}

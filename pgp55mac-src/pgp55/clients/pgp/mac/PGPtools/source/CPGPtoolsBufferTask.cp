/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <UDesktop.h>

#include "CWarningAlert.h"
#include "MacMemory.h"
#include "MacStrings.h"

#include "CPGPtools.h"
#include "CPGPtoolsBufferTask.h"
#include "CPGPtoolsTaskProgressDialog.h"
#include "CResultsWindow.h"
#include "PGPtoolsEncryptDecrypt.h"
#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"
#include "pgpUserInterface.h"
#include "pgpUtilities.h"
#include "PGPSharedEncryptDecrypt.h"

CPGPtoolsBufferTask::CPGPtoolsBufferTask(PGPtoolsOperation operation) :
		CPGPtoolsCryptoTask( operation )
{
	mText 			= NULL;
	mTextLength 	= 0;
	mTextContext	= kInvalidPGPContextRef;
}

CPGPtoolsBufferTask::~CPGPtoolsBufferTask(void)
{
	if( IsntNull( mText ) )
	{
		PGPFreeData( mText );
		mText = NULL;
	}
	
	if( PGPContextRefIsValid( mTextContext ) )
	{
		(void) PGPFreeContext( mTextContext );
		mTextContext = kInvalidPGPContextRef;
	}
}


// MW cannot seem to compile this function without this option
#pragma global_optimizer on

	CToolsError
CPGPtoolsBufferTask::EncryptSign(
	PGPContextRef	context,
	CEncodeParams	*params)
{
	CToolsError	err;
	
	pgpAssert( PGPContextRefIsValid( context ) );
	pgpAssertAddrValid( params, CEncodeParams );
	
	mContext	= context;
	
	if( IsntNull( mText ) )
	{
		Str255			itemName;
		Boolean			done = FALSE;
		PGPContextRef	c	= context;
		
		GetTaskItemName( itemName );
		params->progressDialog->SetNewProgressItem( itemName );	

		PGPOptionListRef	signingOptions	= kInvalidPGPOptionListRef;
		void				*outputBuffer	= NULL;
		PGPSize				outputBufferSize;
		CEncodeEventHandlerData	data( this, params );
		
		outputBuffer 	= NULL;
		
		if( params->signing )
		{
			err.pgpErr = PGPBuildOptionList( c, &signingOptions,
					PGPOSignWithKey( c, params->signingKey,
						PGPOPassphrase( c, params->signingPassphrase ),
						PGPOLastOption( c ) ),
					PGPOClearSign( c, TRUE ),
					PGPOLastOption( c ) );
		}
		else
		{
			err.pgpErr	= PGPBuildOptionList( c, &signingOptions,
							PGPONullOption( c ),
							PGPOLastOption( c ));
		}
		
		if ( err.IsntError() )
		{
			err.pgpErr = PGPEncode( c,
				PGPOInputBuffer( c, mText, mTextLength ),
				PGPOAllocatedOutputBuffer( c, &outputBuffer,
						MAX_PGPSize, &outputBufferSize ),
				PGPOArmorOutput( c, TRUE ),
				PGPODataIsASCII( c, TRUE ),
				signingOptions,
				PGPOptionListRefIsValid( params->encodeOptions ) ?
					params->encodeOptions : PGPONullOption( c ),
				PGPOEventHandler( c, sEncodeEventHandlerProc,
							&data ),
				PGPOSendNullEvents( c, TRUE ),
				PGPOLastOption( c  ) );
		}
		/* can't happen; we verifed earlier that it was good */
		pgpAssert( err.pgpErr != kPGPError_BadPassphrase );

		if( PGPOptionListRefIsValid( signingOptions ) )
			PGPFreeOptionList( signingOptions );
		
		if( IsntNull( outputBuffer ) )
		{
			PGPFreeData( mText );
			
			mText 		= outputBuffer;
			mTextLength	= outputBufferSize;
		}
		
		if( err.IsntError( ) )
			err = ProcessOutputData();
	}
	

	return( err );
}

#pragma global_optimizer reset

	CToolsError
CPGPtoolsBufferTask::DecryptVerify(
	PGPContextRef	context,
	CDecodeParams	*params)
{
	CToolsError	err;

	mContext	= context;
	
	if( IsntNull( mText ) )
	{
		void			*outputBuffer;
		PGPSize			outputBufferSize;
		Str255			itemName;
		PGPKeySetRef	newKeysSet = kInvalidPGPKeySetRef;
		
		CDecodeEventHandlerData	data( this, params );
		
		pgpAssertAddrValid( params, CDecodeParams );
		GetTaskItemName( itemName );

		err.pgpErr = PGPNewKeySet( context, &newKeysSet );
		if( err.IsntError() )
		{
			err.pgpErr = PGPDecode( context,
						PGPOInputBuffer( context, mText, mTextLength ),
						PGPOAllocatedOutputBuffer( context, &outputBuffer,
									MAX_PGPSize, &outputBufferSize ),
						PGPOKeySetRef( context, params->allKeys ),
						PGPOPassThroughIfUnrecognized( context, TRUE ),
						PGPOEventHandler( context, sDecodeEventHandlerProc,
									&data ),
						PGPOSendNullEvents( context, TRUE ),
						PGPOImportKeysTo( context, newKeysSet ),
						PGPOLastOption( context ) );
		}
		
		if( err.IsntError( ) )
		{
			PGPFreeData( mText );
			
			mText 		= outputBuffer;
			mTextLength	= outputBufferSize;
			
			err = ProcessOutputData();
			
			if( mSignatureDataValid )
			{
				CResultsWindow::NewResult( itemName, &mSignatureData );
			}
			
			if( err.IsntError() )
			{
				PGPUInt32	numNewKeys;
			
				err.pgpErr = PGPCountKeys( newKeysSet, &numNewKeys );
				if( err.IsntError() && numNewKeys > 0 )
				{
					// Offer to import the keys
					PGPKeySetRef	importSet;
					char			prompt[256];
					
					GetIndCString( prompt, kDialogStringsListResID,
							kSelectiveImportFoundKeysPromptStrIndex );
					
					err.pgpErr = PGPSelectiveImportDialog( context, prompt,
							newKeysSet, params->allKeys, &importSet );
					if( err.IsntError() )
					{
						err.pgpErr = PGPSharedAddKeysToDefaultKeyring(
									importSet );
						
						(void) PGPFreeKeySet( importSet );
					}
				}
			}
		}
		
		if( PGPRefIsValid( newKeysSet ) )
			PGPFreeKeySet( newKeysSet );
	}
	
	return( err );
}

	PGPError
CPGPtoolsBufferTask::SetText(
	PGPByte 	*text,
	PGPSize		textLength)
{
	PGPError		err = kPGPError_NoErr;
	
	pgpAssertAddrValid( text, PGPByte );
	
	if( IsntNull( mText ) )
	{
		mTextLength = 0;
		PGPFreeData( mText );
	}
	
	pgpAssert( mTextContext == kInvalidPGPContextRef );
	
	err = PGPNewContext( kPGPsdkAPIVersion, &mTextContext );
	if( IsntPGPError( err ) )
	{
		mText = PGPNewData( mTextContext, textLength );
		if( IsntNull( mText ) )
		{
			BlockMoveData( text, mText, textLength );
			mTextLength = textLength;
		}
		else
		{
			pgpDebugMsg( "CPGPtoolsBufferTask::SetData(): Out of memory" );
			err = kPGPError_OutOfMemory;
		}
	}
	
	return( err );
}

	OSStatus
CPGPtoolsBufferTask::CalcProgressBytes(
	PGPContextRef		context,
	ByteCount *			progressBytes)
{
	(void) context;
	
	*progressBytes	= mTextLength;
	
	return( noErr );
}








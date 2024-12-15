/*____________________________________________________________________________
	CManualTranslator.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CManualTranslator.cp,v 1.34 1997/09/20 18:11:44 lloyd Exp $
____________________________________________________________________________*/
#include <string.h>

#include "pgpMem.h"
#include "MacMemory.h"
#include "MacFiles.h"
#include "pgpErrors.h"

#include "CManualTranslator.h"
#include "TranslatorUtils.h"
#include "PluginLibUtils.h"
#include "PassphraseCache.h"
#include "WarningAlert.h"
#include "TranslatorStrings.h"
#include "CWrappers.h"
#include "CSecureMemory.h"
#include "SignatureStatusMessage.h"
#include "PGPSharedParse.h"
#include "TranslatorPrefs.h"
#include "pgpWordWrap.h"
#include "pgpVersionHeader.h"
#include "PGPSharedEncryptDecrypt.h"




	CComboError
CManualDecryptVerifyTranslator::HandlePassphraseEvent(
	PGPEvent *			event,
	EventHandlerData *	data)
{
	return( HandleDecryptPassphraseEvent( event, data ) );
}




	PluginError
CManualDecryptVerifyTranslator::TranslateFile(
	long			transContext,
	ConstemsMIMETypeHandle	inMIMEtypeHandle,
	const FSSpec *	inSpec,
	StringHandle**	addressesHandle,		// nil for manual translators
	emsMIMETypeHandle*	outMIMETypeHandlePtr,// nil for manual translators
	const FSSpec *	outSpec,
	StringHandle*	returnedMessage,		// nil for manual translators
	StringHandle*	errorMessage,
	long *			resultCode
	)
{
	CComboError		err;
	Boolean			reportedError	= false;
	DecryptVerifyEventHandlerData	eventData( this );
	
	(void)transContext;
	(void)inMIMEtypeHandle;
	(void)addressesHandle;
	
	InitPtrToNil( outMIMETypeHandlePtr );
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );
	
	if ( IsntNull( resultCode ) )
		*resultCode	= EMSR_OK;

	void *			inBuffer	= NULL;
	ulong			inBufferSize;
	ulong			blockStart;
	ulong			blockSize;
	
	Boolean			isEncrypted	= false;
	Boolean			isSigned	= false;
	
	err.err	= FSpReadFileIntoBuffer( inSpec, &inBuffer, &inBufferSize );
	if ( err.IsntError() )
	{
		if ( FindEncryptedBlock( inBuffer,
				inBufferSize, &blockStart, &blockSize ) )
		{
			isEncrypted	= true;
		}
		else if ( FindSignedBlock( inBuffer,
					inBufferSize, &blockStart, &blockSize ) )
		{
			isSigned	= true;
		}
		
		if ( ! ( isSigned || isEncrypted ) )
		{
			WarningAlert( kWANoteAlertType, kWAOKStyle,
				kTranslatorErrorStringsResID, kMessageIsNotEncryptedStrIndex);
			err.pgpErr	= kPGPError_UnknownError;
			reportedError	= true;
		}
	}
	if ( err.IsntError() )
	{
		err.pgpErr	= LoadDefaultKeySet( FALSE );
	}
	
	if ( err.IsntError() && isEncrypted )
	{
		void *			outputData	= NULL;
		PGPSize			outputDataSize;
		
		do
		{
			PGPContextRef	c	= mContext;
	
			err.pgpErr	= PGPDecode( c,	
					PGPOInputBuffer( c,
						&((const char *)inBuffer)[ blockStart ], blockSize),
					PGPOAllocatedOutputBuffer( c,
						&outputData, MAX_PGPSize, &outputDataSize ),
					PGPOOutputLineEndType( c, kPGPLineEnd_CR),
					PGPOEventHandler( c, sPGPEventHandler, &eventData ),
					PGPOSendNullEvents( c, 10 ),
					PGPOKeySetRef( c, mKeySet ),
					PGPOLastOption( c )
					);
		} while ( err.pgpErr == kPGPError_BadPassphrase );
		
		if ( err.IsntError() )
		{
			short	fileRef;
			
			// write out:
			//	- data preceeding encrypted block
			//	- signing information (if any)
			//	- decrypted data
			//	- data following encrypted block
			err.err	= FSpOpenDF( outSpec, fsRdWrPerm, &fileRef );
			if ( err.IsntError() )
			{
				long	count;

				count	= blockStart;
				err.err	= FSWrite( fileRef, &count, inBuffer );
				if ( err.IsntError()  &&
					eventData.mHaveSignatureData )
				{
					Str255	signatureStatusString;
					long	count;
					
					GetSignatureStatusMessage( &eventData.mSignatureData,
						signatureStatusString );
					AppendPString( "\p\r", signatureStatusString );
					
					count	= StrLength( signatureStatusString );
					err.err	= FSWrite( fileRef,
							&count, &signatureStatusString[ 1 ] );
				}
				if ( err.IsntError() )
				{
					count	= outputDataSize;
					err.err	= FSWrite( fileRef, &count, outputData );
				}
				if ( err.IsntError() )
				{
					const char *	data	=
						&((const char *)inBuffer)[ blockStart + blockSize ];
						
					count	= inBufferSize - ( blockStart + blockSize );
					err.err		= FSWrite( fileRef, &count, data );
				}
				
				FSClose( fileRef );
			}
			
			PGPFreeData( outputData );
			outputData	= NULL;
		}
	
		if ( err.IsntError()  )
			gEncryptionPassphraseCache->Remember( eventData.mPassphrase );
		else
			gEncryptionPassphraseCache->Forget( );
	}
	else if ( err.IsntError()  && isSigned )
	{
		PGPContextRef	c	= mContext;
	
		err.pgpErr	= PGPDecode( c,	
				PGPOInputBuffer( c,
					&((const char *)inBuffer)[ blockStart ], blockSize),
				PGPODiscardOutput( c, TRUE ),
				PGPOOutputLineEndType( c, kPGPLineEnd_CR),
				PGPOEventHandler( c, sPGPEventHandler, &eventData ),
				PGPOSendNullEvents( c, 10 ),
				PGPOKeySetRef( c, mKeySet ),
				PGPOLastOption( c )
				);
		
		if ( err.IsntError() )
		{
			short	fileRef;
			
			pgpAssert( eventData.mHaveSignatureData );
			
			// write out:
			//	- our verification info
			//	- original data
			err.err	= FSpOpenDF( outSpec, fsRdWrPerm, &fileRef );
			if ( err.IsntError() )
			{
				Str255		signingMessage;
				long		count;
				
				GetSignatureStatusMessage( &eventData.mSignatureData,
							signingMessage );
				AppendPString( "\p\r", signingMessage );
				
				count	= StrLength( signingMessage );
				err.err	= FSWrite( fileRef, &count, &signingMessage[ 1 ] );
				if ( err.IsntError( ) )
				{
					long	count	= inBufferSize;
					
					err.err	= FSWrite( fileRef, &count, inBuffer );
				}
				FSClose( fileRef );
			}
		}
	}
	
	if ( IsntNull( inBuffer ) )
	{
		pgpFreeMac( inBuffer );
		inBuffer	= nil;
	}
		
	// important: if we got an error, we must make sure that the output
	// is the same as the input or Eudora will grab whatever crap
	// we put into it.
	if ( err.IsError() ) 
	{
		(void)FSpCopyFiles( inSpec, outSpec );
	}

	
	if ( ! reportedError )
	{
		ReportError( err );
	}
		
	return( CComboErrorToEudoraError( err ) );
}



#pragma mark -



	PluginError
CManualAddKeyTranslator::TranslateFile(
	long				transContext,
	ConstemsMIMETypeHandle	inMIMEtypeHandle,
	const FSSpec *		inSpec,
	StringHandle**		addressesHandle,		// nil for manual translators
	emsMIMETypeHandle*	outMIMETypeHandlePtr,	// nil for manual translators
	const FSSpec *		outSpec,
	StringHandle*		returnedMessage,		// nil for manual translators
	StringHandle*		errorMessage,
	long *				resultCode
	)
{
	CComboError		err;
	PluginError		emsrErr	= EMSR_OK;
	Boolean			reportedError	= false;
	
	(void)transContext;
	(void)inMIMEtypeHandle;
	(void)addressesHandle;
	(void)outMIMETypeHandlePtr;
	(void)returnedMessage;
	(void)errorMessage;
	(void)resultCode;
	
	// preserve the data; we aren't going to change anything
	err.err	= FSpCopyFiles( inSpec, outSpec );
	if ( err.IsntError() )
	{
		PGPUInt32		numKeysAdded = 0;
		PGPUInt32		numKeysFound = 0;
		PGPContextRef	c	= mContext;
		PGPBoolean		cantOpenMutable	= FALSE;
		PGPKeySetRef	keysFound;
		
		err.pgpErr	= PGPImportKeySet( c,
					&keysFound,
					PGPOInputFileFSSpec(c, inSpec),
					PGPOLastOption( c )
					);
		if ( err.IsntError() )
		{
			err.pgpErr	= PGPCountKeys( keysFound, &numKeysFound );
		}
		
		if ( err.IsntError() && numKeysFound != 0 )
		{
			PGPKeySetRef	keysToImport	= kPGPInvalidRef;
			Str255			pPrompt;
			char			cPrompt[ sizeof( pPrompt ) ];
			
			GetIndString( pPrompt, kTranslatorStringsRID,
				kSelectiveImportPromptStrIndex );
			PToCString( pPrompt, cPrompt );
			
			err.pgpErr	= PGPSelectiveImportDialog( c,
					cPrompt,
					keysFound,
					mKeySet,
					&keysToImport );
			if ( err.IsntError() )
			{
				err.pgpErr	= PGPCountKeys( keysToImport, &numKeysAdded );
				if ( numKeysAdded != 0 )
				{
					/* we're about to change it, so get rid of it */
					if ( IsntNull( mKeySet ) )
					{
						PGPFreeKeySet( mKeySet );
						mKeySet	= NULL;
					}
					
					err.pgpErr	=
						PGPSharedAddKeysToDefaultKeyring( keysToImport );
				}
				PGPFreeKeySet( keysToImport );
			}
			PGPFreeKeySet( keysFound );
		}
		
		reportedError	= true;	// most cases do, see below
		if ( numKeysAdded != 0 )
		{
			if ( numKeysAdded == 1 )
			{
				WarningAlert( kWANoteAlertType, kWAOKStyle,
					kTranslatorErrorStringsResID,
					kKeyAddedSuccessfullyStrIndex );
			}
			else
			{
				Str255	numKeysStr;
				NumToString( numKeysAdded, numKeysStr );
				
				WarningAlert( kWANoteAlertType, kWAOKStyle,
					kTranslatorErrorStringsResID,
					kKeysAddedSuccessfullyStrIndex, numKeysStr );
			}
		}
		else if ( err.pgpErr == kPGPError_FilePermissions ||
					cantOpenMutable)
		{
			WarningAlert( kWAStopAlertType, kWAOKStyle,
				kTranslatorErrorStringsResID,
				kCantAddKeyAddItInPGPkeysInsteadStrIndex );
		}
		else if ( err.IsCancelError() )
		{
			reportedError	= true;
		}
		else if ( err.HavePGPError()  )
		{
			Str255	errorString;
			
			PGPGetErrorString( err.pgpErr,
				sizeof( errorString ), (char *)errorString);
			CToPString( (char *)errorString, errorString );

			WarningAlert( kWANoteAlertType, kWAOKStyle,
				kTranslatorErrorStringsResID,
				kErrorWhileAddingKeyStrIndex, errorString );
		}
		else if ( numKeysAdded == 0 )
		{
			WarningAlert( kWAStopAlertType, kWAOKStyle,
				kTranslatorErrorStringsResID, kNoKeysPresentStrIndex );
		}
		else
		{
			reportedError	= false;
		}
				
		// we handle all our own error reporting
		err.err		= noErr;
		err.pgpErr	= noErr;
	}
	
	if ( ! reportedError )
	{
		ReportError( err );
	}
	
	emsrErr	= CComboErrorToEudoraError( err );
	
	return( emsrErr );
}








CManualEncryptSignTranslator::CManualEncryptSignTranslator(
	PGPContextRef	context,
	emsProgress		progressHook,
	Boolean			doSign )
	: CManualTranslator( context, progressHook)
{
	mDoSign = doSign;
}


#pragma global_optimizer on


	PluginError
CManualEncryptSignTranslator::TranslateFile(
	long			transContext,
	ConstemsMIMETypeHandle	inMIMEtypeHandle,
	const FSSpec *	inSpec,
	StringHandle**	addressesHandle,			// nil for manual translators
	emsMIMETypeHandle*	outMIMETypeHandlePtr,	// nil for manual translators
	const FSSpec *	outSpec,
	StringHandle*	returnedMessage,			// nil for manual translators
	StringHandle*	errorMessage,
	long *			resultCode
	)
{
	CComboError			err;
	PGPOptionListRef	options	= kPGPInvalidRef;
	PGPContextRef		c	= mContext;

	(void)transContext;
	(void)inMIMEtypeHandle;
	(void)addressesHandle;
	(void)outMIMETypeHandlePtr;
	(void)returnedMessage;
	(void)errorMessage;
	(void)resultCode;
		
	pgpAssert( IsNull( addressesHandle ) );
	InitPtrToNil( outMIMETypeHandlePtr );
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );

	DebugCopyToRAMDisk( inSpec, "\pManualEncryptIn" );
	
	
	err.pgpErr	= PGPBuildOptionList( c, &options,
				PGPOOutputLineEndType( c, kPGPLineEnd_CR),
				PGPOLastOption( c ) );
	
	if ( err.IsntError() )
	{
		err	= HandleEncryptSign( inSpec, outSpec,
				addressesHandle, mDoSign, options );
	}
	
	PGPFreeOptionList( options );

	// important: if we got an error, we must make sure that the output
	// is the same as the input or Eudora will grab whatever crap we put
	// into it.
	if ( err.IsError() ) 
	{
		(void)FSpCopyFiles( inSpec, outSpec );
	}
	
	ReportError( err );
	
	return( CComboErrorToEudoraError( err ) );
}




	PluginError
CManualSignTranslator::TranslateFile(
	long			transContext,
	ConstemsMIMETypeHandle	inMIMEtypeHandle,
	const FSSpec *	inSpec,
	StringHandle**	addressesHandle,		// nil for manual translators
	emsMIMETypeHandle*	outMIMETypeHandlePtr,// nil for manual translators
	const FSSpec *	outSpec,
	StringHandle*	returnedMessage,		// nil for manual translators
	StringHandle*	errorMessage,
	long *			resultCode
	)
{
	CComboError		err;
	PGPKeyRef		signingKey	= kPGPInvalidRef;
	
	CSecureCString256		signingPassphrase;
	Boolean					reportedError	= false;


	(void)transContext;
	(void)inMIMEtypeHandle;
	(void)addressesHandle;
	(void)outMIMETypeHandlePtr;
	(void)returnedMessage;
	(void)errorMessage;
	(void)resultCode;
	
	pgpAssert( IsNull( addressesHandle ) );
	InitPtrToNil( outMIMETypeHandlePtr );
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );
	
	DebugCopyToRAMDisk( inSpec, "\pManualSignIn" );
	
	ulong	wordWrapLength;
	Boolean	shouldWordWrap	= PrefShouldBreakLines( &wordWrapLength );
	FSSpec	tempSpec;
		
	if ( shouldWordWrap )
	{
		err.err	= FSpGetUniqueSpec( outSpec, &tempSpec );
		if ( err.IsntError() )
		{
			PFLContextRef	context	= NULL;
			
			err.pgpErr	= PFLNewContext( &context );
			if ( err.IsntError() )
			{
				err.pgpErr	= pgpWordWrapFileFSSpec( context, inSpec,
					&tempSpec, wordWrapLength, "\r");
				PFLFreeContext( context );
			}
		}
	}
	else
	{
		tempSpec	= *inSpec;
	}
	
	if ( err.IsntError() )
	{
		// prompt for passphrase if none was entered
		err		= GetSigningPassphrase( signingPassphrase, &signingKey);

		if ( err.IsntError() )
		{
			PGPContextRef				c = mContext;
			char						comment[ 256 ];
				
			PrefGetComment( comment );
			EncryptSignEventHandlerData	data(this);
			
			err.pgpErr	= PGPEncode( c,
				PGPOInputFileFSSpec( c, &tempSpec ),
				PGPOOutputFileFSSpec( c, outSpec ),
				PGPOSignWithKey( c, signingKey,
					PGPOPassphrase(c, signingPassphrase),
					PGPOLastOption(c) ),
				PGPOLocalEncoding( c, kPGPLocalEncoding_None ),
				PGPOArmorOutput(c, TRUE),
				PGPOOutputLineEndType( c, kPGPLineEnd_CR),
				PGPOClearSign(c, TRUE ),
				PGPODataIsASCII( c, TRUE ),
				PGPOEventHandler( c, sPGPEventHandler, &data ),
				PGPOSendNullEvents( c, TRUE ),
				PGPOVersionString( c, pgpVersionHeaderString ),
				PGPOCommentString( c, comment ),
				PGPOLastOption(c)
				);
		
			if ( shouldWordWrap )
			{
				// delete our intermediate file
				(void)FSpDelete( &tempSpec );
			}
			
			gSigningPassphraseCache->Remember( signingPassphrase,
				signingKey );
			gEncryptionPassphraseCache->Remember( signingPassphrase );
		}
		DebugCopyToRAMDisk( outSpec, "\pManualEncryptSignOut" );
	}

	// important: if we got an error, we must make sure that the output
	// is the same as the input or Eudora will grab whatever crap we put
	// into it.
	if ( err.IsError() ) 
	{
		(void)FSpCopyFiles( inSpec, outSpec );
	}
	
	if ( ! reportedError )
	{
		ReportError( err );
	}
	
	return( CComboErrorToEudoraError( err ) );
}



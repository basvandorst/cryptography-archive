/*____________________________________________________________________________	CManualTranslator.cp		Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CManualTranslator.cp,v 1.52 1999/04/26 09:40:05 heller Exp $____________________________________________________________________________*/#include <string.h>#include <TextUtils.h>#include "pgpMem.h"#include "MacFiles.h"#include "pgpErrors.h"#include "CManualTranslator.h"#include "TranslatorUtils.h"#include "PluginLibUtils.h"#include "PassphraseCache.h"#include "WarningAlert.h"#include "TranslatorStrings.h"#include "CWrappers.h"#include "CSecureMemory.h"#include "SignatureStatusMessage.h"#include "PGPSharedParse.h"#include "TranslatorPrefs.h"#include "pgpWordWrap.h"#include "pgpVersionHeader.h"#include "PGPSharedEncryptDecrypt.h"#include "pgpClientLib.h"#include "pgpClientErrors.h"	PGPErrorCManualDecryptVerifyTranslator::PGPDecodeEventHandler(	PGPContextRef					context,	PGPEvent *						event,	DecryptVerifyEventHandlerData *	data){		CComboError	err;		switch( event->type )	{		case kPGPEvent_OutputEvent:		{			if( event->data.outputData.forYourEyesOnly && ( data->mFYEOData == nil ) )			{				SInt32	eof;								err.err = ::SetFPos( data->mOutputFileRef, fsFromStart, 0 );				if ( err.IsntError() )				{					err.err = ::GetEOF( data->mOutputFileRef, &eof );				}								if ( err.IsntError() )				{					data->mFYEOData = (char *) PGPNewSecureData( PGPGetContextMemoryMgr( context ),																 eof + 1,																 kPGPMemoryMgrFlags_None );					if ( IsNull( data->mFYEOData ) )					{						err.pgpErr = kPGPError_OutOfMemory;					}				}								if ( err.IsntError() )				{					data->mFYEOData[eof] = 0;					err.err = ::FSRead( data->mOutputFileRef, &eof, data->mFYEOData );				}			}		}		break;						case kPGPEvent_EndLexEvent:		{			if( data->mClientHandlerData.outputBufferSize != 0 )			{				long	count;				Str255	str;				short	strIndex;				if( data->mClientHandlerData.signatureDataValid )				{					GetSignatureStatusMessage(&data->mSignatureData,								str );										count = StrLength( str );					if ( IsntNull( data->mFYEOData ) )					{						PGPSize	newSize = strlen( data->mFYEOData ) + count + 1;												err.pgpErr = PGPReallocData( PGPGetContextMemoryMgr( context ),													 &data->mFYEOData,													 newSize,													 kPGPMemoryMgrFlags_None );						if ( err.IsntError() )						{							strncat( data->mFYEOData, (char * ) &str[1], count );							data->mFYEOData[newSize - 1] = 0;						}					}					else					{						err.err = FSWrite( data->mOutputFileRef,								&count, &str[1] );					}					if( err.IsntError() )					{						if( data->mClientHandlerData.sectionType ==									kPGPAnalyze_Encrypted )						{							strIndex = kBeginDecryptedVerifiedMsgStrIndex;						}						else						{							strIndex = kBeginVerifiedMsgStrIndex;						}												GetIndString( str, kTranslatorStringsRID,								strIndex );												count = StrLength( str );						if ( IsntNull( data->mFYEOData ) )						{							PGPSize	newSize = strlen( data->mFYEOData ) + count + 1;														err.pgpErr = PGPReallocData( PGPGetContextMemoryMgr( context ),														 &data->mFYEOData,														 newSize,														 kPGPMemoryMgrFlags_None );							if ( err.IsntError() )							{								strncat( data->mFYEOData, (char *) &str[1], count );								data->mFYEOData[newSize - 1] = 0;							}						}						else						{							err.err = FSWrite( data->mOutputFileRef,									&count, &str[1] );						}					}				}								if( err.IsntError() )				{					count = data->mClientHandlerData.outputBufferSize;									if ( IsntNull( data->mFYEOData ) )					{						PGPSize	newSize = strlen( data->mFYEOData ) + count + 1;												err.pgpErr = PGPReallocData( PGPGetContextMemoryMgr( context ),													 &data->mFYEOData,													 newSize,													 kPGPMemoryMgrFlags_None );						if ( err.IsntError() )						{							strncat( data->mFYEOData, (char *) data->mClientHandlerData.outputBuffer,								count );							data->mFYEOData[newSize - 1] = 0;						}					}					else					{						err.err = FSWrite( data->mOutputFileRef, &count,									data->mClientHandlerData.outputBuffer );					}				}				if( err.IsntError() &&					data->mClientHandlerData.signatureDataValid )				{					if( data->mClientHandlerData.sectionType ==								kPGPAnalyze_Encrypted )					{						strIndex = kEndDecryptedVerifiedMsgStrIndex;					}					else					{						strIndex = kEndVerifiedMsgStrIndex;					}											GetIndString( str, kTranslatorStringsRID,							strIndex );										count = StrLength( str );					if ( IsntNull( data->mFYEOData ) )					{						PGPSize	newSize = strlen( data->mFYEOData ) + count + 1;												err.pgpErr = PGPReallocData( PGPGetContextMemoryMgr( context ),													 &data->mFYEOData,													 newSize,													 kPGPMemoryMgrFlags_None );						if ( err.IsntError() )						{							strncat( data->mFYEOData, (char *) &str[1], count );							data->mFYEOData[newSize - 1] = 0;						}					}					else					{						err.err = FSWrite( data->mOutputFileRef,								&count, &str[1] );					}				}			}						break;		}	}		if( err.IsntError() )	{		err.pgpErr = CManualTranslator::PGPDecodeEventHandler( context,					event, data );	}		return( err.ConvertToPGPError() );}	PluginErrorCManualDecryptVerifyTranslator::TranslateFile(	emsTranslatorP 		trans,	emsDataFileP 		inFile,    emsDataFileP 		outFile,    emsResultStatusP 	transStatus){	CComboError		err;	Boolean			reportedError	= false;	DecryptVerifyEventHandlerData	eventData( this );	PGPKeySetRef	newKeysSet = kInvalidPGPKeySetRef;		(void) trans;	(void) transStatus;		err.pgpErr = PGPNewKeySet( mContext, &newKeysSet );	if ( err.IsntError() )	{		err.pgpErr = LoadDefaultKeySet( FALSE );	}		if ( err.IsntError() )	{		PGPContextRef	c	= mContext;		err.err	= FSpOpenDF( &outFile->file, fsRdWrPerm, &eventData.mOutputFileRef );		if ( err.IsntError() )		{			err.pgpErr	= PGPDecode( c,						PGPOInputFileFSSpec( c, &inFile->file ),					PGPOOutputLineEndType( c, kPGPLineEnd_CR),					PGPOEventHandler( c, sPGPDecodeEventHandler, &eventData ),					PGPOSendNullEvents( c, 10 ),					PGPOKeySetRef( c, mKeySet ),					PGPOImportKeysTo( c, newKeysSet ),					PGPOPassThroughIfUnrecognized( c, TRUE ),					PGPOLastOption( c )					);					FSClose( eventData.mOutputFileRef );			eventData.mOutputFileRef = -1;		}				if ( err.IsntError() )		{			if( err.IsntError() &&				PGPKeySetRefIsValid( eventData.mClientHandlerData.newKeySet ) )			{				err.pgpErr = PGPAddKeys(eventData.mClientHandlerData.newKeySet,									newKeysSet );				if( err.IsntError() )				{					err.pgpErr = PGPCommitKeyRingChanges( newKeysSet );				}			}		}			if ( err.IsntError() )		{			RememberDecryptionPassBuffer(					eventData.mClientHandlerData.passBuffer,					eventData.mClientHandlerData.decryptionKey );		}		else		{			gDecryptionPassphraseCache->Forget( );		}				// FYEO		if ( err.IsntError() && ( eventData.mFYEOData != nil ) )		{			err.pgpErr = PGPForYourEyesOnlyDialog( c , eventData.mFYEOData );		}		if ( eventData.mFYEOData != nil )		{			pgpClearMemory( eventData.mFYEOData, strlen( eventData.mFYEOData ) );			PGPFreeData( eventData.mFYEOData );		}	}	// important: if we got an error, we must make sure that the output	// is the same as the input or Eudora will grab whatever crap	// we put into it. We also do this if the message was FYEO	if ( err.IsError() || ( eventData.mFYEOData != nil ) ) 	{		(void)FSpCopyFiles( PGPGetContextMemoryMgr( mContext ),					&inFile->file, &outFile->file );	}	if( err.IsntError() )	{		PGPUInt32	numNewKeys;		PGPError	tempErr;				tempErr = PGPCountKeys( newKeysSet, &numNewKeys );		if( IsntPGPError( tempErr ) && numNewKeys > 0 )		{			// Offer to import the keys			PGPKeySetRef	importSet;			char			prompt[256];						GetIndCString( prompt, kTranslatorStringsRID,					kSelectiveImportAfterSendPromptStrIndex );						tempErr = PGPSelectKeysDialog( mContext,					kPGPSelectKeysImportVariation, prompt,					newKeysSet, mKeySet, &importSet );						if( IsntPGPError( tempErr ) )			{				(void) PGPSharedAddKeysToDefaultKeyring( importSet );				(void) PGPFreeKeySet( importSet );			}		}	}	if ( ! reportedError )	{		ReportError( err );	}		if( PGPKeySetRefIsValid( newKeysSet ) )		PGPFreeKeySet( newKeysSet );			return( CComboErrorToEudoraError( err ) );}#pragma mark -	PluginErrorCManualAddKeyTranslator::TranslateFile(	emsTranslatorP 		trans,	emsDataFileP 		inFile,    emsDataFileP 		outFile,    emsResultStatusP 	transStatus){	CComboError		err;	PluginError		emsrErr	= EMSR_OK;	Boolean			reportedError	= false;		(void) trans;	(void) transStatus;		// preserve the data; we aren't going to change anything	err.err	= FSpCopyFiles( PGPGetContextMemoryMgr( mContext ), &inFile->file,						&outFile->file );	if ( err.IsntError() )	{		PGPUInt32		numKeysAdded = 0;		PGPUInt32		numKeysFound = 0;		PGPContextRef	c	= mContext;		PGPBoolean		cantOpenMutable	= FALSE;		PGPKeySetRef	keysFound;				err.pgpErr	= PGPImportKeySet( c,					&keysFound,					PGPOInputFileFSSpec(c, &inFile->file),					PGPOLastOption( c )					);		if ( err.IsntError() )		{			err.pgpErr	= PGPCountKeys( keysFound, &numKeysFound );		}				if ( err.IsntError() && numKeysFound != 0 )		{			PGPKeySetRef	keysToImport	= kInvalidPGPKeySetRef;			Str255			pPrompt;			char			cPrompt[ sizeof( pPrompt ) ];						GetIndString( pPrompt, kTranslatorStringsRID,				kSelectiveImportPromptStrIndex );			PToCString( pPrompt, cPrompt );						err.pgpErr	= PGPSelectKeysDialog( c,					kPGPSelectKeysImportVariation,					cPrompt,					keysFound,					mKeySet,					&keysToImport );			if ( err.IsntError() )			{				err.pgpErr	= PGPCountKeys( keysToImport, &numKeysAdded );				if ( numKeysAdded != 0 )				{					/* we're about to change it, so get rid of it */					if ( IsntNull( mKeySet ) )					{						PGPFreeKeySet( mKeySet );						mKeySet	= NULL;					}										err.pgpErr	=						PGPSharedAddKeysToDefaultKeyring( keysToImport );				}				PGPFreeKeySet( keysToImport );			}			PGPFreeKeySet( keysFound );		}				reportedError	= true;	// most cases do, see below		if ( numKeysAdded != 0 )		{			if ( numKeysAdded == 1 )			{				WarningAlert( kWANoteAlertType, kWAOKStyle,					kTranslatorErrorStringsResID,					kKeyAddedSuccessfullyStrIndex );			}			else			{				Str255	numKeysStr;				NumToString( numKeysAdded, numKeysStr );								WarningAlert( kWANoteAlertType, kWAOKStyle,					kTranslatorErrorStringsResID,					kKeysAddedSuccessfullyStrIndex, numKeysStr );			}		}		else if ( err.pgpErr == kPGPError_FilePermissions ||					cantOpenMutable)		{			WarningAlert( kWAStopAlertType, kWAOKStyle,				kTranslatorErrorStringsResID,				kCantAddKeyAddItInPGPkeysInsteadStrIndex );		}		else if ( err.IsCancelError() )		{			reportedError	= true;		}		else if ( err.HavePGPError()  )		{			Str255	errorString;						PGPGetClientErrorString( err.pgpErr,				sizeof( errorString ), (char *)errorString);			CToPString( (char *)errorString, errorString );			WarningAlert( kWANoteAlertType, kWAOKStyle,				kTranslatorErrorStringsResID,				kErrorWhileAddingKeyStrIndex, errorString );		}		else if ( numKeysAdded == 0 )		{			WarningAlert( kWAStopAlertType, kWAOKStyle,				kTranslatorErrorStringsResID, kNoKeysPresentStrIndex );		}		else		{			reportedError	= false;		}						// we handle all our own error reporting		err.err		= noErr;		err.pgpErr	= noErr;	}		if ( ! reportedError )	{		ReportError( err );	}		emsrErr	= CComboErrorToEudoraError( err );		return( emsrErr );}CManualEncryptSignTranslator::CManualEncryptSignTranslator(	PGPContextRef		context,	PGPtlsContextRef	tlsContext,	emsProgress			progressHook,	Boolean				doSign )	: CManualTranslator( context, tlsContext, progressHook){	mDoSign = doSign;}#pragma global_optimizer on	PluginErrorCManualEncryptSignTranslator::TranslateFile(	emsTranslatorP 		trans,	emsDataFileP 		inFile,    emsDataFileP 		outFile,    emsResultStatusP 	transStatus){	CComboError			err;	PGPOptionListRef	options	= kInvalidPGPOptionListRef;	PGPContextRef		c	= mContext;	(void) trans;	(void) transStatus;		DebugCopyToRAMDisk( PGPGetContextMemoryMgr( mContext ), &inFile->file,				"\pManualEncryptIn" );			err.pgpErr	= PGPBuildOptionList( c, &options,				PGPOOutputLineEndType( c, kPGPLineEnd_CR),				PGPOLastOption( c ) );		if ( err.IsntError() )	{		err	= HandleEncryptSign( &inFile->file, &outFile->file,				inFile->header, mDoSign, options, false );	}		PGPFreeOptionList( options );	// important: if we got an error, we must make sure that the output	// is the same as the input or Eudora will grab whatever crap we put	// into it.	if ( err.IsError() ) 	{		(void)FSpCopyFiles( PGPGetContextMemoryMgr( mContext ), &inFile->file, &outFile->file );	}		ReportError( err );		return( CComboErrorToEudoraError( err ) );}	PluginErrorCManualSignTranslator::TranslateFile(	emsTranslatorP 		trans,	emsDataFileP 		inFile,    emsDataFileP 		outFile,    emsResultStatusP 	transStatus){	CComboError		err;	PGPKeyRef		signingKey	= kInvalidPGPKeyRef;	Boolean					reportedError	= false;	(void) trans;	(void) transStatus;		DebugCopyToRAMDisk( PGPGetContextMemoryMgr( mContext ), &inFile->file,				"\pManualSignIn" );		ulong	wordWrapLength;	Boolean	shouldWordWrap	= PrefShouldBreakLines( &wordWrapLength );	FSSpec	tempSpec;			if ( shouldWordWrap )	{		err.err	= FSpGetUniqueSpec( &outFile->file, &tempSpec );		if ( err.IsntError() )		{			PGPMemoryMgrRef	memoryMgr	= NULL;						err.pgpErr	= PGPNewMemoryMgr( 0, &memoryMgr );			if ( err.IsntError() )			{				err.pgpErr	= pgpWordWrapFileFSSpec( memoryMgr, &inFile->file,					&tempSpec, wordWrapLength, "\r");				PGPFreeMemoryMgr( memoryMgr );			}		}	}	else	{		tempSpec	= inFile->file;	}		if ( err.IsntError() )	{		PGPPassBufferRef	passBuffer;				// prompt for passphrase if none was entered		err = GetSigningPassBuffer( &passBuffer, &signingKey );		if ( err.IsntError() )		{			err.pgpErr = PGPGuaranteeMinimumEntropy( mContext );		}				if ( err.IsntError() )		{			PGPContextRef				c = mContext;			char						comment[ 256 ];							PrefGetComment( comment );			EncryptSignEventHandlerData	data(this);						err.pgpErr	= PGPEncode( c,				PGPOInputFileFSSpec( c, &tempSpec ),				PGPOOutputFileFSSpec( c, &outFile->file ),				PGPOSignWithKey( c, signingKey,					PGPOPassBuffer(c, passBuffer),					PGPOLastOption(c) ),				PGPOLocalEncoding( c, kPGPLocalEncoding_None ),				PGPOArmorOutput(c, TRUE),				PGPOOutputLineEndType( c, kPGPLineEnd_CR),				PGPOClearSign(c, TRUE ),				PGPODataIsASCII( c, TRUE ),				PGPOEventHandler( c, sPGPEncodeEventHandler, &data ),				PGPOSendNullEvents( c, TRUE ),				PGPOVersionString( c, pgpVersionHeaderString ),				PGPOCommentString( c, comment ),				PGPOLastOption(c)				);					if ( shouldWordWrap )			{				// delete our intermediate file				(void)FSpDelete( &tempSpec );			}						if( err.IsntError() )			{				RememberSigningPassBuffer( passBuffer, signingKey );								// remember it for encryption as well (but not vice versa)				RememberDecryptionPassBuffer( passBuffer, signingKey );			}			else			{				gSigningPassphraseCache->Forget();			}						PGPFreePassBuffer( passBuffer );		}				DebugCopyToRAMDisk( PGPGetContextMemoryMgr( mContext ), &outFile->file,					"\pManualEncryptSignOut" );	}	// important: if we got an error, we must make sure that the output	// is the same as the input or Eudora will grab whatever crap we put	// into it.	if ( err.IsError() ) 	{		(void)FSpCopyFiles( PGPGetContextMemoryMgr( mContext ), &inFile->file, &outFile->file );	}		if ( ! reportedError )	{		ReportError( err );	}		return( CComboErrorToEudoraError( err ) );}
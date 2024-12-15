/*____________________________________________________________________________	CVerifyTranslator.cp		Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CVerifyTranslator.cp,v 1.25 1999/04/26 09:40:18 heller Exp $____________________________________________________________________________*/#include <TextUtils.h>#include "MacStrings.h"#include "MacFiles.h"#include "MacIcons.h"#include "PGPUserInterface.h"#include "MyMIMEUtils.h"#include "TranslatorIDs.h"#include "PluginLibUtils.h"#include "TranslatorUtils.h"#include "TranslatorStrings.h"#include "SignatureStatusMessage.h"#include "PGPSharedParse.h"#include "PGPSharedEncryptDecrypt.h"#include "CVerifyTranslator.h"	PluginErrorCVerifyTranslator::CanTranslate(	emsTranslatorP 		trans,	emsDataFileP 		inTransData,   	emsResultStatusP 	transStatus){	PluginError	err	= EMSR_CANT_TRANS;		(void) trans;	(void) transStatus;		err	= CanTranslateMIMEType( inTransData->context, inTransData->mimeInfo,			"\pmultipart", "\psigned", "\papplication/pgp-signature");	return( err );}	PluginErrorCVerifyTranslator::TranslateFile(	emsTranslatorP 		trans,	emsDataFileP 		inFile,    emsDataFileP 		outFile,    emsResultStatusP 	transStatus){	CComboError		err;		(void) trans;		transStatus->code = EMSC_SIGUNKNOWN;		DebugCopyToRAMDisk( PGPGetContextMemoryMgr( mContext ), &inFile->file,				"\pVerifyInput" );			err.pgpErr	= LoadDefaultKeySet( FALSE );		FInfo	fInfo;	FSpGetFInfo( &outFile->file, &fInfo );	FSpDeleteResourceFork( &outFile->file );	FSpChangeFileType( &outFile->file, 'TEXT' );			if ( err.IsntError( ) )	{		PGPContextRef	c = mContext;		PGPKeySetRef	newKeysSet;				err.pgpErr = PGPNewKeySet( c, &newKeysSet );		if( err.IsntError() )		{			DecryptVerifyEventHandlerData	eventData(this);			err.pgpErr	= PGPDecode( c,				PGPOInputFileFSSpec( c, &inFile->file ),				PGPOOutputFileFSSpec( c, &outFile->file ),				PGPOOutputLineEndType( c, kPGPLineEnd_CR),				PGPOEventHandler( c, sPGPDecodeEventHandler, &eventData ),				PGPOSendNullEvents( c, TRUE ),				PGPOKeySetRef( c, mKeySet ),				PGPOImportKeysTo( c, newKeysSet ),				PGPOLastOption(c) );						if ( ! eventData.mHaveSignatureData )			{				transStatus->code	= EMSC_SIGBAD;			}			else			{				transStatus->code		=					SignatureDataToEudoraResult(						&eventData.mSignatureData.sigData );			}			if ( eventData.mHaveSignatureData )			{				Str255		statusMessage;								GetSignatureStatusMessage( &eventData.mSignatureData,					statusMessage );				transStatus->desc	= NewString( statusMessage );			}						if( err.IsntError() &&				PGPKeySetRefIsValid( eventData.mClientHandlerData.newKeySet ) )			{				err.pgpErr = PGPAddKeys(eventData.mClientHandlerData.newKeySet,									newKeysSet );				if( err.IsntError() )				{					err.pgpErr = PGPCommitKeyRingChanges( newKeysSet );				}			}			if( err.IsntError() )			{				PGPUInt32	numNewKeys;				PGPError	tempErr;								tempErr = PGPCountKeys( newKeysSet, &numNewKeys );				if( IsntPGPError( tempErr ) && numNewKeys > 0 )				{					// Offer to import the keys					PGPKeySetRef	importSet;					char			prompt[256];										GetIndCString( prompt, kTranslatorStringsRID,							kSelectiveImportAfterSendPromptStrIndex );										tempErr = PGPSelectKeysDialog( c,							kPGPSelectKeysImportVariation, prompt,							newKeysSet, mKeySet, &importSet );										if( IsntPGPError( tempErr ) )					{						(void) PGPSharedAddKeysToDefaultKeyring( importSet );						(void) PGPFreeKeySet( importSet );					}				}			}			PGPFreeKeySet( newKeysSet );		}	}		DebugCopyToRAMDisk( PGPGetContextMemoryMgr( mContext ), &outFile->file,				"\pVerifyOutput" );		// we deleted the resource fork.	// This makes Eudora unhappy.  Recreate one.	(void)FSpCreateResFile( &outFile->file,		fInfo.fdCreator, fInfo.fdType, smSystemScript );					ReportError( err );		return( CComboErrorToEudoraError( err ) );}
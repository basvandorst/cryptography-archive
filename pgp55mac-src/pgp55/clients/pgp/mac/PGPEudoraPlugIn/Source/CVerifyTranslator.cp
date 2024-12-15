/*____________________________________________________________________________
	CVerifyTranslator.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CVerifyTranslator.cp,v 1.18 1997/09/20 02:13:26 lloyd Exp $
____________________________________________________________________________*/

#include "MacStrings.h"
#include "MacFiles.h"
#include "MacIcons.h"
#include "MacMemory.h"
#include "PGPUserInterface.h"
#include "MyMIMEUtils.h"
#include "TranslatorIDs.h"
#include "PluginLibUtils.h"
#include "TranslatorUtils.h"
#include "SignatureStatusMessage.h"
#include "PGPSharedParse.h"

#include "CVerifyTranslator.h"



	PluginError
CVerifyTranslator::CanTranslate(
	long			transContext,
	emsMIMEtype **	inMimeTypeHandle,
	const FSSpec *	inFSp,
	StringHandle**	addressesHandle,
	long *			aprox_len,
	StringHandle *	errorMessage,
	long *			result_code)
{
	PluginError	err	= EMSR_CANT_TRANS;
	
	(void)inFSp;
	(void)addressesHandle;
	(void)aprox_len;
	(void)result_code;
	InitPtrToNil( errorMessage );
	
	err	= CanTranslateMIMEType( transContext, inMimeTypeHandle,
			"\pmultipart", "\psigned", "\papplication/pgp-signature");

	return( err );
}






	PluginError
CVerifyTranslator::TranslateFile(
	long					transContext,
	ConstemsMIMETypeHandle	inMIMEtypeHandle,
	const FSSpec *			inSpec,
	StringHandle**			addressesHandle,
	emsMIMETypeHandle *		outMIMETypeHandlePtr,
	const FSSpec *			outSpec,
	StringHandle*			returnedMessage,
	StringHandle*			errorMessage,
	long *					resultCode)
{
	CComboError		err;
	
	(void)transContext;
	(void)inMIMEtypeHandle;
	(void)addressesHandle;
	
	if ( IsntNull( resultCode ) )
	{
		*resultCode	= EMSC_SIGUNKNOWN;
	}
	
	InitPtrToNil( outMIMETypeHandlePtr );
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );

	DebugCopyToRAMDisk( inSpec, "\pVerifyInput" );
		
	err.pgpErr	= LoadDefaultKeySet( FALSE );
	
	FInfo	fInfo;
	FSpGetFInfo( outSpec, &fInfo );
	FSpDeleteResourceFork( outSpec );
	FSpChangeFileType( outSpec, 'TEXT' );
		
	if ( err.IsntError( ) )
	{
		PGPContextRef					c	= mContext;
		DecryptVerifyEventHandlerData	eventData(this);

		err.pgpErr	= PGPDecode( c,
			PGPOInputFileFSSpec( c, inSpec ),
			PGPOOutputFileFSSpec( c, outSpec ),
			PGPOOutputLineEndType( c, kPGPLineEnd_CR),
			PGPOEventHandler( c, sPGPEventHandler, &eventData ),
			PGPOSendNullEvents( c, TRUE ),
			PGPOKeySetRef( c, mKeySet ),
			PGPOLastOption(c) );
		
		pgpAssert( IsntNull( resultCode ) );
		if ( ! eventData.mHaveSignatureData )
		{
			*resultCode	= EMSC_SIGBAD;
		}
		else
		{
			*resultCode		=
				SignatureDataToEudoraResult(
					&eventData.mSignatureData.sigData );
		}

		if ( IsntNull( returnedMessage ) )
		{
			if ( eventData.mHaveSignatureData )
			{
				Str255		statusMessage;
				
				GetSignatureStatusMessage( &eventData.mSignatureData,
					statusMessage );
				*returnedMessage	= NewString( statusMessage );
			}
			else
			{
				*returnedMessage	= NULL;
			}
		}
	}
	
	DebugCopyToRAMDisk( outSpec, "\pVerifyOutput" );
	
	// we deleted the resource fork.
	// This makes Eudora unhappy.  Recreate one.
	(void)FSpCreateResFile( outSpec,
		fInfo.fdCreator, fInfo.fdType, smSystemScript );
		
		
	ReportError( err );
	
	return( CComboErrorToEudoraError( err ) );
}








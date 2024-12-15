/*____________________________________________________________________________
	CDecryptTranslator.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CDecryptTranslator.cp,v 1.17 1997/10/21 00:22:31 lloyd Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpErrors.h"
#include "MacMemory.h"
#include "MacFiles.h"
#include "CDecryptTranslator.h"
#include "TranslatorIDs.h"
#include "MyMIMEUtils.h"
#include "PluginLibUtils.h"
#include "TranslatorUtils.h"
#include "PassphraseCache.h"
#include "SignatureStatusMessage.h"




	CComboError
CDecryptTranslator::HandlePassphraseEvent(
	PGPEvent *			event,
	EventHandlerData *	data)
{
	return( HandleDecryptPassphraseEvent( event, data ) );
}




	PluginError
CDecryptTranslator::CanTranslate(
	long				transContext,
	emsMIMETypeHandle	inMimeTypeHandle,
	const FSSpec *		inFSp,
	StringHandle**		addressesHandle,
	long *				aprox_len,
	StringHandle *		errorMessage,
	long *				result_code)
{
	PluginError	err	= EMSR_CANT_TRANS;
	
	(void)inFSp;
	(void)addressesHandle;
	(void)aprox_len;
	(void)result_code;
	InitPtrToNil( errorMessage );
	
	err	= CanTranslateMIMEType( transContext, inMimeTypeHandle,
			"\pmultipart", "\pencrypted", "\papplication/pgp-encrypted");
	

	return( err );
}


	PluginError
CDecryptTranslator::TranslateFile(
	long				transContext,
	ConstemsMIMETypeHandle	inMIMETypeHandle,
	const FSSpec *		inSpec,
	StringHandle**		addressesHandle,
	emsMIMETypeHandle*	outMIMETypeHandlePtr,
	const FSSpec *		outSpec,
	StringHandle*		returnedMessage,
	StringHandle*		errorMessage,
	long *				resultCode)
{
	CComboError		err;
	FInfo			fInfo;

	(void)transContext;
	(void)inMIMETypeHandle;
	(void)addressesHandle;
	
	DecryptVerifyEventHandlerData	eventData(this);
	
	InitPtrToNil( outMIMETypeHandlePtr );
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );
	if ( IsntNull( resultCode ) )
		*resultCode	= EMSR_OK;
	
	FSpGetFInfo( outSpec, &fInfo );
	(void)FSpDelete( outSpec );
	
	err.pgpErr	= LoadDefaultKeySet( FALSE );
	if ( err.IsntError() )
	{
		PGPContextRef	c	= mContext;
	
		err.pgpErr	= PGPDecode( c,
				PGPOInputFileFSSpec( c, inSpec ),
				PGPOOutputFileFSSpec( c, outSpec ),
				PGPOEventHandler( c, sPGPEventHandler, &eventData ),
				PGPOSendNullEvents( c, TRUE ),
				PGPOKeySetRef( c, mKeySet ),
				PGPOLastOption(c)
				);
		
		if ( err.IsError() )
		{
			gEncryptionPassphraseCache->Forget();
		}
	}
	
	// we deleted the resource fork.
	// This makes Eudora unhappy.  Recreate one.
	(void)FSpCreateResFile( outSpec,
		fInfo.fdCreator, fInfo.fdType, smSystemScript );

	DebugCopyToRAMDisk( inSpec, "\pDecryptIn" );
	DebugCopyToRAMDisk( outSpec, "\pDecryptOut" );
	
	if ( err.IsntError() )
	{
		pgpAssert( IsntNull( resultCode ) );
		pgpAssert( IsntNull( returnedMessage ) );
		
		if ( ! eventData.mHaveSignatureData )
		{
			*resultCode	= EMSR_OK;
		}
		else
		{
			PGPEventSignatureData const *	sigData;
			
			sigData	= &eventData.mSignatureData.sigData;
			*resultCode	= SignatureDataToEudoraResult( sigData );

			if ( IsntNull( returnedMessage ) )
			{
				Str255		statusMessage;
				
				GetSignatureStatusMessage( &eventData.mSignatureData,
					statusMessage );
				*returnedMessage	= NewString( statusMessage );
			}
		}
	}
	
	// parse the output to figure out what the MIME type is
	pgpAssertAddrValid( outMIMETypeHandlePtr, emsMIMETypeHandle );
	ParseFileForMIMEType( outSpec, outMIMETypeHandlePtr );
	
	
	if ( err.IsntError() )
	{
		gEncryptionPassphraseCache->Remember( eventData.mPassphrase );
	}
	else
	{
		gEncryptionPassphraseCache->Forget();
		ReportError( err );
	}
		
	
	return( CComboErrorToEudoraError( err ) );
}












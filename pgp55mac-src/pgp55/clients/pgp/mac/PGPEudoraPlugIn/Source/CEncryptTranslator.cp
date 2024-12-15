/*____________________________________________________________________________
	CEncryptTranslator.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CEncryptTranslator.cp,v 1.20 1997/09/11 19:32:57 lloyd Exp $
____________________________________________________________________________*/
#include <string.h>

#include "pgpErrors.h"
#include "pgpKeys.h"
#include "MacStrings.h"
#include "MacMemory.h"
#include "MacFiles.h"
#include "MyMIMEUtils.h"
#include "TranslatorIDs.h"
#include "TranslatorUtils.h"
#include "CPGPFree.h"
#include "TranslatorFactory.h"
#include "pgpVersionHeader.h"

#include "CEncryptTranslator.h"


CEncryptTranslator::CEncryptTranslator(
	PGPContextRef		context,
	emsProgress			progressHook,
	CipherOutputType	outputType, 
	Boolean				doSign )
	: CTranslator( context, progressHook)
{
	mOutputType	= outputType;
	
	mDoSign		= doSign;
}




#pragma global_optimizer on


	PluginError
CEncryptTranslator::TranslateFile(
	long					transContext,
	ConstemsMIMETypeHandle	inMIMEtypeHandle,
	const FSSpec *			inSpec,
	StringHandle**			addressesHandle,
	emsMIMETypeHandle *		outMIMEtypeHandlePtr,
	const FSSpec *			outSpec,
	StringHandle*			returnedMessage,
	StringHandle*			errorMessage,
	long *					resultCode)
{
	CComboError				err;
	Boolean					usePGPMime	= ( mOutputType == kUsePGPMIME );
	char					mimeDelimiter[ kPGPMimeSeparatorSize ];
	PGPOptionListRef		options	= kPGPInvalidRef;
	PGPContextRef			c	= mContext;

	InitPtrToNil( outMIMEtypeHandlePtr );
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );
	
	(void)transContext;
	(void)resultCode;
	
	DebugCopyToRAMDisk( inSpec, "\pEncryptIn" );
			
	if ( ! usePGPMime )
	{
		// use PGPMIME if we're processing anything other than text
		if ( ! MatchMIMEType( inMIMEtypeHandle, "\ptext", "\pplain" ) )
		{
			usePGPMime	= true;
		}
	}
	
	
	if ( (! usePGPMime ) &&
		MatchMIMEType( inMIMEtypeHandle, "\ptext", "\pplain" ) )
	{
		// only eliminate it for plain text, not text/enriched
		err.err	= EliminateMIMEHeader( inSpec );
	}
	
	
	err.pgpErr	= PGPBuildOptionList( c, &options,
		PGPOOutputLineEndType( c, kPGPLineEnd_CRLF),
		PGPOLastOption(c)
		);
		
	if ( err.IsntError() && usePGPMime )
	{
		PGPSize		mimeBodyOffset;
		
		err.pgpErr	= PGPAppendOptionList( c, options, 
				PGPOPGPMIMEEncoding( c, TRUE,
					&mimeBodyOffset, mimeDelimiter ),
				PGPOOmitMIMEVersion( c, TRUE),
				PGPOLastOption(c) );
	}
	
	if ( err.IsntError() )
	{
		err	= HandleEncryptSign( inSpec, outSpec, addressesHandle,
				mDoSign, options );
	}
			
	if ( PGPRefIsValid( options ) )
		PGPFreeOptionList( options );
	
	DebugCopyToRAMDisk( outSpec, "\pEncryptFinalOut" );
		
	// finish up by creating appropriate MIME types and parameters
	// and by putting output data into appropriate format
	if ( err.IsntError() )
	{
		ConstStringPtr	mimeParamValueString	= nil;
		
		// put output data into appropriate format
		if ( usePGPMime )
		{
			err.err	= BuildEncryptedPGPMIMEType( outMIMEtypeHandlePtr,
					mimeDelimiter );
		}
		else
		{
			err.err	= CreateMIMEType( "\ptext", "\pplain",
					outMIMEtypeHandlePtr );
		}
	}


	ReportError( err );
	
	return( CComboErrorToEudoraError( err ) );
}







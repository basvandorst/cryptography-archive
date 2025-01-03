/*____________________________________________________________________________
	CSignTranslator.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSignTranslator.cp,v 1.29 1997/10/17 17:40:49 heller Exp $
____________________________________________________________________________*/
#include <string.h>

#include "pgpMem.h"


#include "pgpErrors.h"

#include "MacMemory.h"
#include "MacStrings.h"
#include "MacFiles.h"

#include "PGPUserInterface.h"
#include "MyMIMEUtils.h"
#include "TranslatorIDs.h"
#include "PluginLibUtils.h"
#include "CPGPFree.h"
#include "CFSCloser.h"
#include "TranslatorUtils.h"
#include "PassphraseCache.h"
#include "TranslatorStrings.h"
#include "PGPKeys.h"
#include "pgpHash.h"
#include "pgpEncode.h"
#include "pgpVersionHeader.h"
#include "TranslatorPrefs.h"
#include "CSecureMemory.h"

#include "CSignTranslator.h"





CSignTranslator::CSignTranslator(
	PGPContextRef		context,
	emsProgress			progress,
	CipherOutputType	outputType )
	: CTranslator( context, progress)
{
	mOutputType	= outputType;
}



#pragma global_optimizer on



	PluginError
CSignTranslator::TranslateFile(
	long					transContext,
	ConstemsMIMETypeHandle	inMIMETypeHandle,
	const FSSpec *			inSpec,
	StringHandle**			addressesHandle,
	emsMIMETypeHandle *		outMIMEtypeHandlePtr,
	const FSSpec *			outSpec,
	StringHandle*			returnedMessage,
	StringHandle*			errorMessage,
	long *					resultCode)
{
	CComboError		err;
	PluginError		emsrErr	= EMSR_OK;
	CSecureCString256	signingPassphrase;
	Boolean			usePGPMime	= ( mOutputType == kUsePGPMIME );
	
	(void)transContext;
	(void)addressesHandle;
	(void)resultCode;
	
	InitPtrToNil( outMIMEtypeHandlePtr );
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );
	
	if ( ! usePGPMime )
	{
		// use PGPMIME if we're processing anything other than text
		if ( ! MatchMIMEType( inMIMETypeHandle, "\ptext", "\pplain" ) )
		{
			usePGPMime	= true;
		}
	}
	
	const Boolean		useTextOutput	= true;
	FSSpec				signatureSpec;
	
	if ( err.IsntError() )
	{
		PGPKeyRef		signingKey	= kPGPInvalidRef;
		PGPContextRef	c	= mContext;
		char			mimeDelimiter[ kPGPMimeSeparatorSize ];
	
		DebugCopyToRAMDisk( inSpec, "\pSignClearText" );
		
		if ( (! usePGPMime ) &&
			MatchMIMEType( inMIMETypeHandle, "\ptext", "\pplain" ) )
		{
			// only eliminate it for plain text, not text/enriched or other
			err.err	= EliminateMIMEHeader( inSpec );
		}
		
		if ( err.IsntError()  )
		{
			err.err	= FSpGetUniqueSpec( outSpec, &signatureSpec );
		}
	
		
		err	= GetSigningPassphrase( signingPassphrase,
				&signingKey);
		if ( err.IsntError()  )
		{
			PGPOptionListRef	optionList	= kPGPInvalidRef;
			EncryptSignEventHandlerData	data(this);
			PGPSize				mimeBodyOffset;
			char				comment[ 256 ];
			Boolean	signatureOnly	= usePGPMime;
		
			PrefGetComment( comment );
		
			// all these options are the same, regardless of whether it's
			// PGP/MIME or clear signed
			err.pgpErr	= PGPBuildOptionList( c, &optionList,
					PGPOInputFileFSSpec( c, inSpec ),
					PGPOOutputFileFSSpec( c, outSpec ),
					PGPOSignWithKey( c, signingKey,
						PGPOPassphrase(c, signingPassphrase),
						PGPOLastOption(c) ),
					PGPOLocalEncoding( c, kPGPLocalEncoding_None ),
					PGPOArmorOutput(c, TRUE),
					PGPOOutputLineEndType( c, kPGPLineEnd_CRLF),
					PGPOEventHandler( c, sPGPEventHandler, &data ),
					PGPOSendNullEvents( c, TRUE ),
					PGPOClearSign(c, TRUE),
					PGPOVersionString( c, pgpVersionHeaderString ),
					PGPOCommentString( c, comment ),
					PGPOLastOption(c)
					);
			
			if ( err.IsntError() )
			{
				if ( usePGPMime )
				{
					err.err	= PGPEncode( c,
							optionList,
							PGPOPGPMIMEEncoding( c, TRUE,
								&mimeBodyOffset, mimeDelimiter ),
							PGPOOmitMIMEVersion( c, TRUE),
							PGPOLastOption(c) );
				}
				else
				{
					/* clear sign */
					err.err	= PGPEncode( c, optionList, PGPOLastOption(c) );
				}
			}
			
			if ( optionList != kPGPInvalidRef )
				PGPFreeOptionList( optionList );
		}
		
		DebugCopyToRAMDisk( outSpec, "\pSignedOut" );
		
		// finish up by creating appropriate MIME types and parameters
		// and by putting output data into appropriate format
		if ( err.IsntError() )
		{
			if ( ! usePGPMime )
			{
				// signatureSpec contains the original data + the signature
				err.err	= FSpExists( outSpec ) ? kPGPError_NoErr :
							kPGPError_FileNotFound;
			}
			
			// this information already in output, but Eudora forces us to
			// put it in a Eudora-defined data structure
			if ( err.IsntError() )
			{
				// create appropriate MIME types for Eudora
				pgpAssert( IsntNull( outMIMEtypeHandlePtr ) );

				if ( usePGPMime )
				{
					err.err	= BuildSignedPGPMIMEType( outMIMEtypeHandlePtr,
							mimeDelimiter );
				}
				else
				{
					err.err	= CreateMIMEType( "\ptext", "\pplain",
							outMIMEtypeHandlePtr );
				}

				if ( err.IsntError() )
				{
					err.err	= AddMIMEParam( *outMIMEtypeHandlePtr,
							kPGPAlreadyProcessedParamName,
							kPGPAlreadyProcessedParamValue);
				}
			}
		}
		
		DebugCopyToRAMDisk( outSpec, "\pSignFinalOut" );
		
		if ( err.IsntError() )
		{
			gSigningPassphraseCache->Remember( signingPassphrase,
				signingKey );
			// remember it for encryption as well (but not vice versa)
			gEncryptionPassphraseCache->Remember( signingPassphrase );
		}
		else
		{
			gSigningPassphraseCache->Forget();
			
			ReportError( err );
		}
	}
	
					
	return( CComboErrorToEudoraError( err ) );
}



	void
CSignTranslator::GetErrorString(
	CComboError	err,
	StringPtr	msg )
{
	if ( err.HavePGPError() )
	{
		switch( err.pgpErr )
		{
			default:
				inherited::GetErrorString( err, msg );
				break;
			
			case kPGPError_SecretKeyNotFound:
			case kPGPError_AdditionalRecipientRequestKeyNotFound:
				GetIndString( msg,
					kTranslatorErrorStringsResID, kNoDefaultKeyStrIndex );
				break;
		}
	}
	else
	{
		inherited::GetErrorString( err, msg );
	}
}










/*____________________________________________________________________________
	TranslatorFactory.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorFactory.cp,v 1.3 1997/08/02 21:08:16 lloyd Exp $
____________________________________________________________________________*/

#include "TranslatorFactory.h"
#include "CEncryptTranslator.h"
#include "CDecryptTranslator.h"
#include "CSignTranslator.h"
#include "CVerifyTranslator.h"
#include "CManualTranslator.h"
#include "TranslatorPrefs.h"

#if PGP_DEMO
	#include "PGPDemo.h"
#endif

#if BETA
	#include "WarningAlert.h"
#endif


	CTranslator *
CreateTranslator(
	PGPContextRef	context,
	emsProgress		progressHook,
	TranslatorID	id )
{
	CTranslator *		translator	= nil;
	Boolean				betaExpired	= FALSE;
	Boolean				demoExpired	= FALSE;

#if BETA
	betaExpired = BetaExpired();
#endif

#if PGP_DEMO
	demoExpired = (GetDemoState() != kDemoStateActive);
#endif

	if( betaExpired || demoExpired )
	{
		Boolean		dontAllow	= false;
		switch ( id )
		{
			case kEncryptTranslatorID:
			case kEncryptAndSignTranslatorID:
			case kSignTranslatorID:
			case kManualAddKeyTranslatorID:
			case kManualEncryptTranslatorID:
			case kManualSignTranslatorID:
			case kManualEncryptSignTranslatorID:
				dontAllow	= true;
				break;
		}
		if ( dontAllow )
		{
		return( nil );
		}
	}

	switch ( id )
	{
		default:
			pgpDebugMsg( "unknown TranslatorID" );
			translator	= nil;
			break;
			
		case kEncryptTranslatorID:
		{
			CipherOutputType	outputType;
			
			outputType	= PrefShouldUsePGPMimeEncrypt( ) ?
							kUsePGPMIME : kUseOldPGPFormat;
			
			translator	= new CEncryptTranslator( context,
							progressHook, outputType, false);
			break;
		}
			
		case kEncryptAndSignTranslatorID:
		{
			CipherOutputType	outputType;
			
			outputType	= PrefShouldUsePGPMimeEncrypt( ) ?
							kUsePGPMIME : kUseOldPGPFormat;
			translator	= new CEncryptTranslator( context,
							progressHook, outputType, true);
			break;
		}
			
		case kSignTranslatorID:
		{
			CipherOutputType	outputType;
			
			outputType	= PrefShouldUsePGPMimeSign( ) ?
							kUsePGPMIME : kUseOldPGPFormat;
			translator	= new CSignTranslator( context,
								progressHook, outputType );
			break;
		}

			
		case kDecryptTranslatorID:
			translator	= new CDecryptTranslator( context, progressHook );
			break;
			
		case kVerifyTranslatorID:
			translator	= new CVerifyTranslator( context, progressHook);
			break;
			
		case kManualDecryptVerifyTranslatorID:
			translator	= new CManualDecryptVerifyTranslator( context,
							progressHook);
			break;
			
		case kManualAddKeyTranslatorID:
			translator	= new CManualAddKeyTranslator( context, progressHook);
			break;
			
		case kManualEncryptSignTranslatorID:
			translator	= new CManualEncryptSignTranslator( context,
							progressHook, true );
			break;
			
		case kManualEncryptTranslatorID:
			translator	= new CManualEncryptSignTranslator( context,
							progressHook, false );
			break;
			
		case kManualSignTranslatorID:
			translator	= new CManualSignTranslator( context, progressHook);
			break;
	}
	
	return( translator );
}









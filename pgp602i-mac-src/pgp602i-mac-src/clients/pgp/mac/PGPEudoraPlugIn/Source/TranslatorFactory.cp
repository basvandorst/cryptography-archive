/*____________________________________________________________________________	TranslatorFactory.cp		Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: TranslatorFactory.cp,v 1.4.8.1 1998/11/12 03:06:48 heller Exp $____________________________________________________________________________*/#include "TranslatorFactory.h"#include "CEncryptTranslator.h"#include "CDecryptTranslator.h"#include "CSignTranslator.h"#include "CVerifyTranslator.h"#include "CManualTranslator.h"#include "TranslatorPrefs.h"#if PGP_DEMO	#include "PGPDemo.h"#endif#if BETA	#include "WarningAlert.h"#endif	CTranslator *CreateTranslator(	PGPContextRef		context,	PGPtlsContextRef 	tlsContext,	emsProgress			progressHook,	TranslatorID		id ){	CTranslator *		translator	= nil;	Boolean				betaExpired	= FALSE;	Boolean				demoExpired	= FALSE;#if BETA	betaExpired = BetaExpired();#endif#if PGP_DEMO	demoExpired = (GetDemoState() != kDemoStateActive);#endif	if( betaExpired || demoExpired )	{		Boolean		dontAllow	= false;		switch ( id )		{			case kEncryptTranslatorID:			case kEncryptAndSignTranslatorID:			case kSignTranslatorID:			case kManualAddKeyTranslatorID:			case kManualEncryptTranslatorID:			case kManualSignTranslatorID:			case kManualEncryptSignTranslatorID:				dontAllow	= true;				break;		}		if ( dontAllow )		{		return( nil );		}	}	switch ( id )	{		default:			pgpDebugMsg( "unknown TranslatorID" );			translator	= nil;			break;					case kEncryptTranslatorID:		{			CipherOutputType	outputType;						outputType	= PrefShouldUsePGPMimeEncrypt( ) ?							kUsePGPMIME : kUseOldPGPFormat;						translator	= new CEncryptTranslator( context, tlsContext,									progressHook, outputType, false);			break;		}					case kEncryptAndSignTranslatorID:		{			CipherOutputType	outputType;						outputType	= PrefShouldUsePGPMimeEncrypt( ) ?							kUsePGPMIME : kUseOldPGPFormat;			translator	= new CEncryptTranslator( context, tlsContext,							progressHook, outputType, true);			break;		}					case kSignTranslatorID:		{			CipherOutputType	outputType;						outputType	= PrefShouldUsePGPMimeSign( ) ?							kUsePGPMIME : kUseOldPGPFormat;			translator	= new CSignTranslator( context, tlsContext,								progressHook, outputType );			break;		}					case kDecryptTranslatorID:			translator	= new CDecryptTranslator( context, tlsContext,									progressHook );			break;					case kVerifyTranslatorID:			translator	= new CVerifyTranslator( context, tlsContext,									progressHook);			break;					case kManualDecryptVerifyTranslatorID:			translator	= new CManualDecryptVerifyTranslator( context,									tlsContext, progressHook);			break;					case kManualAddKeyTranslatorID:			translator	= new CManualAddKeyTranslator( context, tlsContext,									progressHook);			break;					case kManualEncryptSignTranslatorID:			translator	= new CManualEncryptSignTranslator( context,									tlsContext, progressHook, true );			break;					case kManualEncryptTranslatorID:			translator	= new CManualEncryptSignTranslator( context,									tlsContext, progressHook, false );			break;					case kManualSignTranslatorID:			translator	= new CManualSignTranslator( context, tlsContext,									progressHook);			break;	}		return( translator );}
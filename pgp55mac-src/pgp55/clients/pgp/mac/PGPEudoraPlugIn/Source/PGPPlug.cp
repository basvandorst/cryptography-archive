/*____________________________________________________________________________
	PGPPlug.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPPlug.cp,v 1.10 1997/09/18 01:31:30 lloyd Exp $
____________________________________________________________________________*/
#include <Icons.h>

#include "MacStrings.h"
#include "MacIcons.h"
#include "MacFiles.h"

#include "PGPPlug.h"
#include "CTranslator.h"
#include "CDecryptTranslator.h"
#include "TranslatorFactory.h"
#include "PassphraseCache.h"
#include "TranslatorPrefs.h"
#include "PluginProcInfo.h"
#include "TranslatorStrings.h"
#include "TranslatorUtils.h"
#include "WarningAlert.h"
#include "pgpMacCustomContextAlloc.h"

#if PGP_DEMO
#include "PGPDemo.h"
#endif

// same as used in the BNDL resource
const SInt16	kAboutPGPPluginIconID				=	128;

// obtained 3/17/97, also obtained 881 thru 884
const short		kModuleID	 = 880;



#pragma export on

// these routines are called by the plugin code resource
RoutineDescriptor	PlugInInitRD		=
	BUILD_ROUTINE_DESCRIPTOR( uppPlugInInitProcInfo, PlugInInit );
RoutineDescriptor	PlugInFinishRD		=
	BUILD_ROUTINE_DESCRIPTOR( uppPluginFinishProcInfo, PlugInFinish );
RoutineDescriptor	GetTranslatorInfoRD =
	BUILD_ROUTINE_DESCRIPTOR( uppGetTranslatorInfoProcInfo, GetTranslatorInfo );
RoutineDescriptor	CanTranslateFileRD	=
	BUILD_ROUTINE_DESCRIPTOR( uppCanTranslateFileProcInfo, CanTranslateFile );
RoutineDescriptor	TranslateFileRD		=
	BUILD_ROUTINE_DESCRIPTOR( uppTranslateFileProcInfo, TranslateFile );

#pragma export off




static Boolean		sInited	= false;

	static CComboError
InitMyWorld( void )
{
	CComboError	err;
	
	if ( ! sInited )
	{
		PGPContextRef	tempContext	= NULL;
		
		sInited	= true;
	
		InitPassphraseCaches();
	}
	
	return( err );
}

	static void
DisposeMyWorld( void )
{
	if ( sInited )
	{
		// nothing yet
		sInited	= false;
	}
	
	DisposePassphraseCaches();
}

#pragma mark -



	PluginError
PlugInInit(
	emsUserGlobals **/*globalsHandle*/,
	short*			module_count,
	StringHandle*	descriptionHandlePtr,
	short*			module_id,
	Handle*			iconHandlePtr)
{
	const uchar *			sModuleDescription = "\pEudora PGP Plugin";
	CComboError				err;  
	PluginError				eudoraErr	= EMSR_OK;
	
	pgpLeaksBeginSession( "main" );
	
	err	= InitMyWorld( );
	if ( err.IsntError() )
	{
		
		// Set the number of translators in this module
		if ( IsntNull( module_count ) )
		{
			*module_count = kPGPNumTranslators;
		}
		
		MacLeaks_Suspend();	// Eudora keeps the stuff we're about to allocate
		
		// Set the module's description
		if ( IsntNull( descriptionHandlePtr ) )
		{
			Handle	stringHandle;
		
			err.err	= PtrToHand( &sModuleDescription[ 0 ], &stringHandle,
						StrSize( sModuleDescription ));
			
			*descriptionHandlePtr = (StringHandle)stringHandle;
		}
		
		// Set the module's icon
		if ( IsntNull( iconHandlePtr ) )
		{
			Handle tempSuite = nil;
			
			err.err = GetIconSuite(	&tempSuite, kAboutPGPPluginIconID,
					svAllAvailableData);
			if ( err.IsntError() && IsntNull( tempSuite ))
			{
				err.err = DuplicateIconSuite( tempSuite, iconHandlePtr);
				DisposeIconSuite( tempSuite, true);
			}
		}
		
		MacLeaks_Resume();	// Eudora keeps the stuff we just allocated

		// Set the module ID
		if( IsntNull( module_id ) )
		{
			*module_id = kModuleID;
		}
	}
	
	if ( err.IsError( ) )
	{
		pgpDebugPStr( "\pfailure in PlugInInit" );
		eudoraErr	= EMSR_UNKNOWN_FAIL;
	}
	else
	{
		eudoraErr	= EMSR_OK;
	}
	

#if BETA
	static Boolean	sNotified	= false;
	if ( BetaExpired() && ! sNotified )
	{
	sNotified	= true;
	WarningAlert( kWANoteAlertType, kWAOKStyle,
		"\pThis BETA version of PGP Eudora Plugin has expired."
		"You may decrypt and verify, but not encrypt or sign.  "
		"Please obtain the latest version from the PGP web site." );
	}
#endif

#if PGP_DEMO
	if( err.IsntError() )
	{
		if( ShowDemoDialog( FALSE ) == kDemoStateExpired )
		{
			eudoraErr = EMSR_UNKNOWN_FAIL;
		}
	}
#endif

	return( eudoraErr );
}



	PluginError
PlugInFinish( emsUserGlobals **	/*globalsHandle*/)
{
	DisposeMyWorld( );
	
	// pgpLeaksEndSession();
	
	return( EMSR_OK );
}




	PluginError
GetTranslatorInfo(
	emsUserGlobals **/*globalsHandle*/,
	short			translatorID,
	long *			type,
	long *			subtype,
	unsigned long *	flags,
	StringHandle *	trans_desc,
	Handle *		iconHandlePtr)
{
	CComboError		err;
	PluginError		eudoraErr	= EMSR_OK;
	
	err	= InitMyWorld( );
	if ( err.IsntError() )
	{
		// Make sure that we are given a valid ID
		if( (translatorID >= kFirstTranslatorID ) &&
			(translatorID <= kLastTranslatorIDPlusOne - 1) )
		{
			eudoraErr	= CTranslator::GetInfo( (TranslatorID)translatorID,
					type, subtype, flags, trans_desc, iconHandlePtr );
		}
		else
		{
		pgpDebugPStr( "\pillegal translator ID" );
		}
	}
	
	if ( err.IsError() )
		eudoraErr	= CComboErrorToEudoraError( err );
		
	return( eudoraErr );
}



	PluginError
CanTranslateFile(
	emsUserGlobals **/*globalsHandle*/,
	long			transContext,
	short			translatorID,
	emsMIMEtype **	inMimeTypeHandle,
	const FSSpec *	inFSp,	// may get removed in next version
	StringHandle**	addressesHandle,
	long *			aprox_len,
	StringHandle*	errorMessage,
	long *			result_code)
{
	CComboError	err;
	PluginError	eudoraErr	= EMSR_OK;
	
	err	= InitMyWorld( );
	if ( err.IsntError() )
	{
		CTranslator *	translator	= nil;
		
		InitPtrToNil( errorMessage );
		
		translator	=
			CreateTranslator( NULL, NULL, (TranslatorID)translatorID );
		
		if ( IsntNull( translator ) )
		{
			eudoraErr	= translator->CanTranslate( transContext,
				inMimeTypeHandle,
				inFSp, addressesHandle,
				aprox_len, errorMessage, result_code);
		
			delete translator;
		}
		else
		{
			eudoraErr = EMSR_INVALID_TRANS;
		}
	}
	
	if ( err.IsError() )
		eudoraErr	= CComboErrorToEudoraError( err );
		
	return( eudoraErr );
}


	PluginError
TranslateFile(
	emsUserGlobals **/*globalsHandle*/,
	long			transContext,
	short			translatorID,
	emsMIMEtype **	inMIMETypeHandle,
	const FSSpec *	inSpec,
	StringHandle**	addressesHandle,
	emsProgress		progress,
	emsMIMEtype ***	outMIMETypeHandlePtr,
	const FSSpec *	outSpec,
	Handle *		returnedIcon,
	StringHandle*	returnedMessage,
	StringHandle*	errorMessage,
	long *			resultCode)
{
	CInfoPBRec		cpb;
	ulong			secs;
	PGPContextRef	context;
	CComboError		err;
	PluginError		eudoraErr	= EMSR_OK;

	
	err.pgpErr	= pgpNewContextCustomMacAllocators( &context );

	if ( err.IsError() )
	{
		*resultCode	= EMSR_TRANS_FAILED;
		return( CComboErrorToEudoraError( err ) );
	}
	
	(void)FSpGetCatInfo( outSpec, &cpb );	// info needed below

	// convenient to do this here, globally
	secs	= PrefGetSigningPassphraseCacheSeconds( );
	gSigningPassphraseCache->SetCacheSeconds( secs );
	
	secs	= PrefGetEncryptionPassphraseCacheSeconds( );
	gEncryptionPassphraseCache->SetCacheSeconds( secs );
	
	InitPtrToNil( returnedIcon );
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );

	// note: 'returnedIcon' is not implemented by Eudora, so we won't both
	// passing it to our translators
	
	err	= InitMyWorld( );
	if ( err.IsntError() )
	{
		const ulong		kMinHeapSpace	= 300 * 1024UL;
		
		if ( FreeMem() < kMinHeapSpace )
		{
			Str255	msg;
			
			GetIndString( msg,
				kTranslatorErrorStringsResID, kOutOfMemoryStrIndex );
			WarningAlert( kWANoteAlertType, kWAOKStyle, msg );
			eudoraErr	= CComboErrorToEudoraError( err );
			*resultCode	= EMSR_TRANS_FAILED;
		}
	}
	
	
	if ( err.IsntError() )
	{
		CTranslator *	translator	= nil;
		
		translator	= CreateTranslator( context,
					progress, (TranslatorID)translatorID );
		if ( IsntNull( translator ) )
		{
			err	= translator->PrepareToTranslate();
			if ( err.IsntError() )
			{
				eudoraErr	= translator->TranslateFile( transContext,
					inMIMETypeHandle,
					inSpec, addressesHandle, outMIMETypeHandlePtr,
					outSpec, returnedMessage, errorMessage, resultCode);
			}
			
			delete translator;
		}
		else
		{
			eudoraErr = EMSR_INVALID_TRANS;
		}
	}
	
	// Eudora doesn't like it if there is no output file, even if
	// we're returning an error.
	if ( ! FSpExists( outSpec ) )
	{
		(void)FSpCreate( outSpec, cpbFileCreator( &cpb ), cpbFileType( &cpb ),
				smSystemScript );
	}
	
	PGPFreeContext( context );

	if ( err.IsError() )
	{
		eudoraErr	= CComboErrorToEudoraError( err );
	}
	
	return eudoraErr;
}






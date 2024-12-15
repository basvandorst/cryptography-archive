/*____________________________________________________________________________
	PGPPlugGlue.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPPlugGlue.cp,v 1.3 1997/07/30 04:45:58 lloyd Exp $
____________________________________________________________________________*/

#include <CodeFragments.h>
 
#include "EudoraTypes.h"

#include "PGPPlug.h"
#include "MacFiles.h"
#include "MacEnvirons.h"
#include "MacStrings.h"

#include "PluginProcInfo.h"




#if GENERATING68K
	#define	CALL_TO_CFM		1
#else
	#define	CALL_TO_CFM		0
#endif

#if CALL_TO_CFM	// [


typedef struct MyRoutineDescriptors
{
	UniversalProcPtr	init;
	UniversalProcPtr	finish;
	UniversalProcPtr	getTranslatorInfo;
	UniversalProcPtr	canTranslateFile;
	UniversalProcPtr	translateFile;
}  MyRoutineDescriptors;

static CFragConnectionID sMyConnectionID	=
							kUnresolvedCFragSymbolAddress;
							
static MyRoutineDescriptors		sMyRoutines	= { nil, };

	static CFragConnectionID
GetMyConnectionID()
{
	return( sMyConnectionID );
}

	static OSErr
GetSymbol(
	ConstStringPtr		symbolName,
	Ptr	*				symbol )
{
	OSErr				err	= noErr;
	CFragSymbolClass	theSymClass;
		
	err = FindSymbol( GetMyConnectionID(), symbolName, symbol, &theSymClass);
	AssertNoErr( err, "GetSymbol" );
		
	return( err );
}


	static OSErr
LoadMyLibrary( )
{
#if PGP_DEBUG
	const uchar			kMy68KLibraryName[]	= "\pPGPplugin/Eudora68DBG.Lib";
#else
	const uchar			kMy68KLibraryName[]	= "\pPGPplugin/Eudora68K.Lib";
#endif
	Ptr					mainAddr;
	Str255				errorName;
	OSStatus			err	= noErr;
	CFragConnectionID	connectionID;
	short				curResFile = CurResFile();
	FSSpec				spec;

	GetSpecFromRefNum( curResFile, &spec);
	if ( ! RunningOnPowerPC() )
	{
		// 68K has its library stored separately
		CopyPString( kMy68KLibraryName, spec.name );
	}
	
	err	= GetDiskFragment( &spec,
			0UL, kCFragGoesToEOF, nil, kNewCFragCopy,
			&connectionID, &mainAddr, errorName);
	AssertNoErr( err, "LoadMyLibrary" );

	if ( IsntErr( err ) )
	{
		sMyConnectionID	= connectionID;
		
		err = GetSymbol( "\pPlugInInitRD", (Ptr *)&sMyRoutines.init);
		AssertNoErr( err, "LoadMyLibrary: can't find symbol" );
		
		err = GetSymbol( "\pPlugInFinishRD", (Ptr *)&sMyRoutines.finish);
		AssertNoErr( err, "LoadMyLibrary: can't find symbol" );
		
		err = GetSymbol( "\pGetTranslatorInfoRD",
				(Ptr *)&sMyRoutines.getTranslatorInfo);
		AssertNoErr( err, "LoadMyLibrary: can't find symbol" );
		
		err = GetSymbol( "\pCanTranslateFileRD",
				(Ptr *)&sMyRoutines.canTranslateFile);
		AssertNoErr( err, "LoadMyLibrary: can't find symbol" );
		
		err = GetSymbol( "\pTranslateFileRD",
				(Ptr *)&sMyRoutines.translateFile);
		AssertNoErr( err, "LoadMyLibrary: can't find symbol" );
	}
	AssertNoErr( err, "LoadMyLibrary" );
	
	return( err );
}


	static void
UnloadMyLibrary(  )
{
	OSErr	err	= noErr;
	
	if ( sMyConnectionID != 0 )
	{
		err	= CloseConnection( &sMyConnectionID );
		pgpAssert( sMyConnectionID == 0 );
		AssertNoErr( err, "UnloadMyLibrary" );
	}
}
	
#endif	// ]

	pascal long
ems_plugin_version(
	short*	api_version)
{

	if( IsntNull( api_version ) )
	{
		*api_version = EMS_VERSION;
	}
	
	return(EMSR_OK);
}

	
	pascal long
ems_plugin_init(
	Handle		globalsHandle,
	short*		module_count,
	StringPtr**	descriptionHandlePtr,
	short*		module_id,
	Handle*		iconHandlePtr)
{
	long	err	= noErr;
	
	pgpAssert( sizeof( int ) == 4 );
	if ( sizeof( int ) != 4 )
	{
		SysError( 444 );
	}
	
#if CALL_TO_CFM
	err	= LoadMyLibrary();
	if ( IsntErr( err ) )
	{
		err	= (*sMyRoutines.init)(  globalsHandle, module_count,
				descriptionHandlePtr, module_id, iconHandlePtr );
	}
#else
	err	= PlugInInit( (emsUserGlobals **)globalsHandle, module_count,
		descriptionHandlePtr, module_id, iconHandlePtr);
#endif
	
	AssertNoErr( err, "ems_plugin_init" );
	return( err );
}


	pascal long
ems_plugin_finish(
	Handle	globalsHandle)
{
	long	err	= noErr;

#if CALL_TO_CFM
	err	= (*sMyRoutines.finish)(  globalsHandle );
	UnloadMyLibrary();
#else
	err	= PlugInFinish( (emsUserGlobals **)globalsHandle );
#endif
	return( err );
}



	pascal long
ems_translator_info(
	Handle			globalsHandle,
	short			trans_id,
	long *			type,
	long *			subtype,
	unsigned long *	flags,
	StringPtr **	trans_desc,
	Handle *		iconHandlePtr)
{
	long	err	= noErr;

#if CALL_TO_CFM
	err	= (*sMyRoutines.getTranslatorInfo)(  globalsHandle, trans_id,
			type, subtype, flags, trans_desc, iconHandlePtr );
#else
	err	= GetTranslatorInfo( (emsUserGlobals **)globalsHandle,
			trans_id, type, subtype, flags, trans_desc, iconHandlePtr);
#endif
	return( err );
}



pascal long
ems_can_translate_file(
	Handle			globalsHandle,
	long			transContext,
	short			trans_id,
	emsMIMEtype **	inMimeTypeHandle,
	FSSpec *		inFSp,				// may get removed in next version
	StringPtr ***	addressesHandle,
	long *			aprox_len,
	StringPtr **	errorMessage,
	long *			result_code)
{
	long	err;
	
#if CALL_TO_CFM
	err	= (*sMyRoutines.canTranslateFile)( (emsUserGlobals **)globalsHandle,
			transContext, trans_id, inMimeTypeHandle, inFSp,
			addressesHandle, aprox_len, errorMessage, result_code);
#else
	err	= CanTranslateFile( (emsUserGlobals **)globalsHandle, transContext,
			trans_id, inMimeTypeHandle, inFSp, addressesHandle, aprox_len,
			errorMessage, result_code);
#endif
		
	return( err );
}


#include "MacIcons.h"

	pascal long
ems_translate_file(
	Handle			globalsHandle,
	long			transContext,
	short			trans_id,
	emsMIMEtype **	InMIMEtypeHandle,
	FSSpec *		inFSp,
	StringPtr ***	addressesHandle,
	emsProgress		progress,
	emsMIMEtype ***	OutMIMEtypeHandlePtr,
	FSSpec *		outFSp,
	Handle *		returnedIcon,
	StringPtr **	returnedMessage,
	StringPtr **	errorMessage,
	long *			resultCode)
{
	long	err;
	
#if CALL_TO_CFM
	err	= (*sMyRoutines.translateFile)( (emsUserGlobals **)globalsHandle,
			transContext, trans_id, InMIMEtypeHandle, inFSp, addressesHandle,
			progress, OutMIMEtypeHandlePtr, outFSp, returnedIcon,
			returnedMessage, errorMessage, resultCode);
#else
	err	= TranslateFile( (emsUserGlobals **)globalsHandle, transContext,
			trans_id, InMIMEtypeHandle, inFSp, addressesHandle, progress,
			OutMIMEtypeHandlePtr, outFSp, returnedIcon, returnedMessage,
			errorMessage, resultCode);
#endif
	
	return( err );
}









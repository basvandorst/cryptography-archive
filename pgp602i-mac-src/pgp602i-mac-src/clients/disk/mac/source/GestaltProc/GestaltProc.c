/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: GestaltProc.c,v 1.5.8.1 1998/11/12 03:05:52 heller Exp $____________________________________________________________________________*/#include <A4Stuff.h>#include <SetUpA4.h>#include "pgpMem.h"#include "MacResources.h"#include "PGPDiskGlobals.h"#include "PGPDiskGestalt.h"#include "PGPDiskPatches.h"static Boolean			sPatchesInstalled = FALSE;static Handle			sPatchesCode		= nil;static UInt32			sPatchesCodeSize	= 0;static PGPDiskGlobals	sGlobals;	static pascal OSErrGestaltProc(OSType selector, long *response){	OSErr	err = noErr;		EnterCallback();	pgpAssert( selector == gestaltPGPDisk );	pgpAssertAddrValid( response, long );		if( selector == gestaltPGPDisk )	{		*response = (long) &sGlobals;	}	else	{		err = gestaltUndefSelectorErr;	}		ExitCallback();	return( err );}	static BooleanPatchesInstalled(void){	// The patch proc is given the address of sPatchesInstalled when the	// patches are installed. Calling our semaphore routine TickCount() will	// set sPatchesInstalled to TRUE if the patches are installed in this	// context.		sPatchesInstalled = FALSE;		(void) TickCount();		return( sPatchesInstalled );}	static voidIdleProc(void){	OSStatus	status = noErr;	EnterCallback();#if INSTALL_PATCHES	// [	if( ( ! PatchesInstalled() ) && IsntNull( sPatchesCode ) )	{		THz		applicationZone;		UInt32	heapSize;				AssertHandleIsValid( sPatchesCode, "IdleProc" );				// Install the patches using a copy of the patches code residing in		// the application heap. This copy will get it's own global state		// and will be nuked automatically when the application quits.		// The patch is installed only if the heap is at least 64K in size			applicationZone	= ApplicationZone();				heapSize = (UInt32) StripAddress( applicationZone->bkLim ) - 					(UInt32) StripAddress( &applicationZone->heapData );		if( heapSize > 64 * 1024L )		{			THz		saveZone;			Ptr		patchesCopy;						saveZone = GetZone();			SetZone( applicationZone );						patchesCopy = NewPtr( sPatchesCodeSize );			if( IsntNull( patchesCopy ) )			{				PGPDiskPatchesProcPtr	patchesProc;								BlockMove( *sPatchesCode, patchesCopy, sPatchesCodeSize );								patchesProc = (PGPDiskPatchesProcPtr) patchesCopy;								status = (*patchesProc)( &sPatchesInstalled );			}			else			{				status = memFullErr;			}			SetZone( saveZone );		}	}#endif	// ]	AssertNoErr( status, "IdleProc" );	ExitCallback();}	static OSStatusRealMain(void){	OSStatus		status 					= noErr;	static Boolean	sGestaltProcInstalled 	= FALSE;	if( ! sGestaltProcInstalled )	{		THz		saveZone;				// This is the first invocation of this proc by the application.		// Install the Gestalt proc and fetch the patch code. The application		// resource file is the current resource file.				saveZone = GetZone();		SetZone( SystemZone() );		#if INSTALL_PATCHES	// [		sPatchesCode = Get1Resource( kPatchesProcResType, kPatchesProcResID );		if( IsntNull( sPatchesCode ) )		{			DetachResource( sPatchesCode );						sPatchesCodeSize = GetHandleSize( sPatchesCode );		}		else		{					// It is OK to fail here. If the patches code resource is not			// present, then we assume no patches are needed. This "feature"			// allows for external modification of the app if necessary		}#endif	// ]				PrepareCallback();				pgpClearMemory( &sGlobals, sizeof( sGlobals ) );				sGlobals.magic			= kPGPDiskGlobalsMagic;		sGlobals.globalsVersion	= kPGPDiskGlobalsVersion;		sGlobals.idleProc 		= IdleProc;				status = NewGestalt( gestaltPGPDisk, GestaltProc );		AssertNoErr( status, "GestaltProcMain" );				if( IsntErr( status ) )		{			sGestaltProcInstalled = TRUE;		}		SetZone( saveZone );	}		IdleProc();		return( status );}	OSStatusmain(void){	OSStatus	status;#if PGP_DEBUG	PGPDiskGestaltInstallProcPtr	testProcPtr = main;#endif	EnterCodeResource();			status = RealMain();			ExitCodeResource();		return( status );}
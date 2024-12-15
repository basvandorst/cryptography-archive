/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: PatchesProc.c,v 1.7.8.1 1998/11/12 03:05:54 heller Exp $____________________________________________________________________________*/#if INSTALL_PATCHES	// [#include <A4Stuff.h>#include <SetUpA4.h>#include "MacDriverUtils.h"#include "pgpMem.h"#include "MacStrings.h"#include "MacTraps.h"#include "PGPDiskPatches.h"#include "DriverAPI.h"static Boolean			*sPatchesInstalledPtr = nil;static UniversalProcPtr	sOldTickCountAddress;static UniversalProcPtr	sOldUnmountVolAddress;	static asm voidTickCountPatch(void){	clr.l	-(sp)		// Jump to address	movem.l	d0-d2/a0/a1/a4, -(sp)	jsr		SetUpA4		move.l	sPatchesInstalledPtr, a0	move.b	#1, (a0)		move.l	sOldTickCountAddress, 0x18(sp)	movem.l	(sp)+, d0-d2/a0/a1/a4		rts}#pragma parameter CallUnmountVol(__A1, __A0, __D1)	static asm voidCallUnmountVol(	UniversalProcPtr 	oldAddress,	const ParamBlockRec *callerPB,	ushort 				trapword){	// a0 and d1 are set correctly at this point by the #pragma parameter	// directive for this function.		move.l	a4, -(sp)	// Preserve A4 in case routine we're calling whacks it	jsr		(a1)	move.l	(sp)+, a4	rts}	static BooleanDriverIsPGPDiskDriver(short driverRefNum){	Boolean	isPGPDiskDriver	= false;	OSErr	err	= noErr;	Str255	driverName;		pgpAssert( driverRefNum < 0 );		err	= GetDriverNameUsingRefNum( driverRefNum, driverName );	if ( IsntErr( err ) && DriverNameIsPGPDisk( driverName ) )	{		isPGPDiskDriver = TRUE;	}			return( isPGPDiskDriver );}	static voidDoUnmountVolPatch(const ParamBlockRec *callerPB, ushort trapword){	HParamBlockRec	pb;	Str255			volumePath;	OSErr			err;		// Capture the information about the volume we're unmounting		pgpClearMemory( &pb, sizeof( pb ) );		pb.volumeParam.ioVRefNum = callerPB->volumeParam.ioVRefNum;		if( IsntNull( callerPB->volumeParam.ioNamePtr ) )	{		CopyPString( callerPB->volumeParam.ioNamePtr, volumePath );		pb.volumeParam.ioNamePtr = volumePath;	}		err = PBHGetVInfoSync( &pb );		// Call through to unmount the volume	CallUnmountVol( sOldUnmountVolAddress, callerPB, trapword );		// Send the unmounted volume information to the driver	if( IsntErr( err ) &&		IsntErr( callerPB->volumeParam.ioResult ) &&		DriverIsPGPDiskDriver( pb.volumeParam.ioVDRefNum ) )	{		ProcessSerialNumber	curAppPSN;				GetCurrentProcess( &curAppPSN );				(void) PGPDiskDriveUnmountedInProcess( pb.volumeParam.ioVDRefNum,							pb.volumeParam.ioVDrvInfo, curAppPSN );	}}	static asm voidUnmountVolPatch(void){	movem.l	a0/a4, -(sp)	jsr		SetUpA4		move.w	d1, -(sp)		// Trapword	move.l	a0, -(sp)		// Parameter block		jsr		DoUnmountVolPatch	addq.l	#6, sp		movem.l	(sp)+, a0/a4				// Restore A4 and paramater block		move.w	struct(ParamBlockRec.ioParam.ioResult)(a0), d0	// Set result code	rts}		OSStatusmain(Boolean *patchesInstalledResultPtr){	OSStatus	status = noErr;#if PGP_DEBUG	PGPDiskPatchesProcPtr	testProcPtr = main;#endif	EnterCodeResource();	PrepareCallback();		pgpAssertAddrValid( patchesInstalledResultPtr, Boolean );		sPatchesInstalledPtr = patchesInstalledResultPtr;	(void) PFLPatchTrap( _TickCount, (UniversalProcPtr) TickCountPatch,				&sOldTickCountAddress);	(void) PFLPatchTrap( _UnmountVol, (UniversalProcPtr) UnmountVolPatch,				&sOldUnmountVolAddress );		ExitCodeResource();		return( status );}#endif	// ]
/*____________________________________________________________________________
	MacDebugPatches.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDebugPatches.cp,v 1.6 1997/09/18 01:34:26 lloyd Exp $
____________________________________________________________________________*/

#include <Traps.h>

#include "MacDebug.h"
#include "MacTraps.h"
#include "pgpMem.h"

#include "MacDebugPatches.h"

#include "SetupA5.h"


#if USE_MAC_DEBUG_PATCHES	// [


enum
	{
	uppDisposePtrProcInfo	= kRegisterBased
	 | REGISTER_ROUTINE_PARAMETER( 1, kRegisterD0, SIZE_CODE(sizeof(short)))
	 | REGISTER_ROUTINE_PARAMETER( 2, kRegisterA0, SIZE_CODE(sizeof(Ptr)))
	};
	
enum
	{
	uppDisposeHandleProcInfo	= kRegisterBased
	 | REGISTER_ROUTINE_PARAMETER( 1, kRegisterD0, SIZE_CODE(sizeof(short)))
	 | REGISTER_ROUTINE_PARAMETER( 2, kRegisterA0, SIZE_CODE(sizeof(Handle)))
	};
	

typedef UniversalProcPtr	DisposeHandleUPP;
typedef UniversalProcPtr	DisposePtrUPP;

#define NewDisposeHandleProc( userRoutine ) \
	NewRoutineDescriptor((ProcPtr)(userRoutine),\
	uppDisposeHandleProcInfo, GetCurrentArchitecture())
	
#define NewDisposePtrProc( userRoutine ) \
	NewRoutineDescriptor((ProcPtr)(userRoutine),\
	uppDisposePtrProcInfo, GetCurrentArchitecture())
	

static DisposePtrUPP	sRealDisposePtrPatch	= nil;
static DisposeHandleUPP	sRealDisposeHandlePatch	= nil;

// Filling a block which is itself a heap zone will cause a crash. This
// function attempts to guess that a block is a heap zone.

	static Boolean
BlockMayBeHeapZone(LogicalAddress address, UInt32 blockSize)
{
	Boolean		mayBeHeapZone = FALSE;
	
	if( blockSize > sizeof( Zone ) )
	{
		THz		zoneHeader 	= (THz) address;
		char	*zoneStart	= (char *) &zoneHeader->heapData;
		char	*zoneEnd	= (char *) address + blockSize - 1;
		
		// Try to guess that the block is a heap zone by validating
		// some fields in the "zone header" WITHOUT dereferencing
		// any pointers. We're declaring it a zone iff:
		//
		// 1) bkLim is within 32 bytes of the end of the block.
		// 2) purgePtr is within the block or is nil.
		// 3) hFstFree is within the block or is nil.
		// 4) zcbFree is less than the size of the block.
	
		if( ( zoneHeader->bkLim > zoneEnd - 32 ) &&		
			( zoneHeader->bkLim < zoneEnd ) &&
			
			( IsNull( zoneHeader->purgePtr ) ||
			( ( zoneHeader->purgePtr > zoneStart ) && 
			  ( zoneHeader->purgePtr < zoneEnd ) ) ) &&
			  
			( IsNull( zoneHeader->hFstFree ) ||
			( ( zoneHeader->hFstFree > zoneStart ) && 
			  ( zoneHeader->hFstFree < zoneEnd ) ) ) &&
			
			( zoneHeader->zcbFree < blockSize ) )
		{
			mayBeHeapZone = TRUE;
		}
	}

	return( mayBeHeapZone );
}

// called by "glue" routine
	static pascal void
DisposeHandlePatchImpl(
	short	/*trapWord*/,
	Handle	h)
	{
	#if ! GENERATINGCFM
	static ulong	kMagicTest	= 'DSPS';
	pgpDebugMsgIf( kMagicTest != 'DSPS',
		"DisposeHandlePatchImpl: A5/A4 is not be set up" );
	#endif

	if ( IsntNull( h ) && IsntNull( *h ) )
		{
		if( DebugTraps_HandleZone( h ) != SystemZone() )
			{
			Size	hSize	= DebugTraps_GetHandleSize( h );
		
			if( ! BlockMayBeHeapZone( *h, hSize ) )
				{
				MacDebug_FillWithGarbage( *h, hSize );
				}
			}
		}
	}
	

// called by "glue" routine
	static pascal void
DisposePtrPatchImpl(
	short	/*trapWord*/,
	Ptr		p)
	{
	#if ! GENERATINGCFM
	static ulong	kMagicTest	= 'DSPS';
	pgpDebugMsgIf( kMagicTest != 'DSPS',
		"DisposePtrPatchImpl: A5/A4 is not be set up" );
	#endif
	
	if ( IsntNull( p ) )
		{
		if( DebugTraps_PtrZone( p ) != SystemZone() )
			{
			Size	ptrSize	= DebugTraps_GetPtrSize( p );
		
			if( ! BlockMayBeHeapZone( p, ptrSize ) )
				{
				MacDebug_FillWithGarbage( p, ptrSize );
				}
			}
		}
	}



#if GENERATINGCFM	// [

	static pascal void
DisposeHandlePatchGlue(
	short	trapWord,
	Handle	h)
	{
	DisposeHandlePatchImpl( trapWord, h );
	CallOSTrapUniversalProc( sRealDisposeHandlePatch,
		uppDisposeHandleProcInfo, trapWord, h);
	}
	
	
	static pascal void
DisposePtrPatchGlue(
	short	trapWord,
	Ptr		p)
	{
	DisposePtrPatchImpl( trapWord, p );
	CallOSTrapUniversalProc( sRealDisposePtrPatch,
		uppDisposePtrProcInfo, trapWord, p);
	}
	
#else	// ] [




	static pascal asm void
DisposeHandlePatchGlue()
	{
	movem.l	D1/A0, -(sp)
	
	jsr		SetUpA5		
	// A5 now valid, save old value on stack
	move.l	D0, -(sp)
	
	move.w	d1, -(sp)
	move.l	A0, -(sp)
	jsr		DisposeHandlePatchImpl
	
	move.l	sRealDisposeHandlePatch, D0
	move.l	(sp)+, A5
	movem.l	(sp)+, D1/A0
	
	move.l	D0, -(sp)
	rts
	}
	
	static pascal asm void
DisposePtrPatchGlue()
	{
	movem.l	D1/A0, -(sp)
	
	jsr		SetUpA5		
	// A5 now valid, save old value on stack
	move.l	D0, -(sp)
	
	move.w	d1, -(sp)
	move.l	A0, -(sp)
	jsr		DisposePtrPatchImpl
	
	move.l	sRealDisposePtrPatch, D0
	move.l	(sp)+, A5
	movem.l	(sp)+, D1/A0
	
	move.l	D0, -(sp)
	rts
	}
	
#endif	// ]


	Boolean
DebugPatches_DisposeTrapsPatched( )
{
	return( IsntNull( sRealDisposePtrPatch ) );
}


	void
DebugPatches_PatchDisposeTraps()
	{
	if ( ! DebugPatches_DisposeTrapsPatched() )
		{
		DisposeHandleUPP	disposeHandlePatch;
		Boolean				patchedOK;
		
		PrepareCallbackA5();
		
		disposeHandlePatch	= NewDisposeHandleProc( DisposeHandlePatchGlue );
		patchedOK 			= PFLPatchTrap( _DisposeHandle,
							disposeHandlePatch, &sRealDisposeHandlePatch);
		pgpAssert( patchedOK );

		DisposeHandle( NewHandle( 10 ) );	// test it right now
		
		DisposePtrUPP		disposePtrPatch;
		disposePtrPatch	= NewDisposePtrProc( DisposePtrPatchGlue );
		patchedOK 		= PFLPatchTrap( _DisposePtr,
							disposePtrPatch, &sRealDisposePtrPatch);
		pgpAssert( patchedOK );
		
		DisposePtr( NewPtr( 10 ) );	// test it right now
		}
	}
	
#else

#define DisposeTrapsPatched()		( FALSE )

#endif	// ]


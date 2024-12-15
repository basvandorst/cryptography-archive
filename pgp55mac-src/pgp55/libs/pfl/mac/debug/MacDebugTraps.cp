/*____________________________________________________________________________
	MacDebugTraps.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDebugTraps.cp,v 1.14 1997/09/18 01:34:27 lloyd Exp $
____________________________________________________________________________*/

#include <AEObjects.h>
#include <AEPackObject.h>
#include <LowMem.h>

#include "MacDebug.h"
#include "pgpMem.h"

#include "MacDebugPatches.h"
#include "MacDebugTrapsOff.h"

#include <Icons.h>

#if USE_MAC_DEBUG_TRAPS	// [

#include "MacDebugLeaks.h"

#define kHStatePurgeableMask	0x40
#define kHStateLockedMask		0x80
#define kHStateResourceMask		0x20

inline Boolean HStateIsPurgeable( SInt8 hState )
	{ return( ( hState & kHStatePurgeableMask ) != 0 ); }
	
inline Boolean HStateIsLocked( SInt8 hState )
	{ return( ( hState & kHStateLockedMask ) != 0 ); }
	
inline Boolean HStateIsResource( SInt8 hState )
	{ return( ( hState & kHStateResourceMask ) != 0 ); }
	
inline Boolean HandleIsPurged( Handle theHandle )
	{ return( IsNull( *( theHandle ) ) ); }


/*__________________________________________________________________________
	Errors such as 'noErr' and 'memFullErr' are normal because they can occur
	in normal usage with no adverse consequences.
	Other errors, which indicate a bad handle, heap, etc are not normal.
____________________________________________________________________________*/
	static Boolean
MemErrIsNormal( OSErr	err )
	{
	return( IsntErr( err ) || err == memFullErr );
	}


	static void
AssertMemErrIsNormal( const char * msg )
	{
	OSErr	err;
	
	err	= LMGetMemErr();
	if ( ! MemErrIsNormal( err ) )
		{
		AssertNoErr( err, msg );
		}
	}



	static Boolean
HandleIsPurgeable( Handle	h )
	{
	SInt8	hState;
	
	hState	= HGetState( h );
	
	return( HStateIsPurgeable( hState ) );
	}


	static Boolean
HandleIsLocked( Handle	h )
	{
	SInt8	hState;
	#define kHStatePurgeableMask	0x40
	#define kHStateLockedMask		0x80
	
	hState	= HGetState( h );
	
	return( HStateIsLocked( hState ) );
	}


	static Boolean
HandleIsPurgeableAndNotLocked( Handle h )
	{
	return( HandleIsPurgeable( h ) && ! HandleIsLocked( h ) );
	}



	static Boolean
IsValidMemoryAllocationSize( Size	theSize )
	{
	// choose the highest value that you anticipate
	#define kMaxReasonableAllocation		(128 * 1024UL * 1024UL )

	return( theSize >= 0 && theSize <= kMaxReasonableAllocation );
	}


	static void
AssertValidMemoryAllocationSize(
	Size			byteCount,
	const char *	msg )
	{
	if ( ! IsValidMemoryAllocationSize( byteCount ) )
		{
		Str255	out;
		
		pgpFormatPStr( out, sizeof( out ), TRUE,
			"%s: invalid memory allocation size: %ld bytes",
			msg, ( long )byteCount );
		pgpDebugPStr( out );
		}
	}


#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Memory Manager ---
#endif


	pascal void
DebugTraps_DisposeHandle( Handle h )
	{
	AssertHandleIsValid( h, "DebugTraps_DisposeHandle");
	
	UInt32	hSize;
	SInt8	hState;
	
	MacLeaks_ForgetHandle( h, kMacLeaks_DisposeHandleDeallocType );
	
	hState	= HGetState( h );
	pgpAssertMsg( ! HStateIsResource( hState ),
		"DebugTraps_DisposeHandle: this Handle is a resource. "
		"Use ReleaseResource");
	
	// fill the handle with junk prior to disposing it
	hSize	= GetHandleSize( h );
	if ( MemError() == noErr )
		{
		if ( ! DebugPatches_DisposeTrapsPatched() )
			{
			MacDebug_FillWithGarbage( *h, hSize );
			}
		}
	
	DisposeHandle( h );
	AssertNoErr( LMGetMemErr(), "DebugTraps_DisposeHandle");
	}

	
#define kMaxReasonableBlockMove		(64 * 1024UL * 1024UL )

	pascal void
DebugTraps_BlockMove(
	const void	*srcPtr,
	void		*destPtr,
	Size		byteCount )
	{
	const void	*sysZone;
	
	pgpAssertAddrValidMsg( srcPtr, VoidAlign, "DebugTraps_BlockMove");
	pgpAssertAddrValidMsg( destPtr, VoidAlign, "DebugTraps_BlockMove");
	
	pgpAssertMsg( byteCount >= 0,
		"DebugTraps_BlockMove: negative PGPByte count");
	pgpAssertMsg( byteCount <= kMaxReasonableBlockMove,
		"DebugTraps_BlockMove: count is huge...is this correct?");
	
	sysZone	= SystemZone();
	pgpAssertMsg( srcPtr >= ( void *)sysZone,
		"DebugTraps_BlockMove: srcPtr below system zone");
	pgpAssertMsg( destPtr >= ( void *)sysZone,
	"DebugTraps_BlockMove: destPtr below system zone");
	
	
	BlockMove( srcPtr, destPtr, byteCount );
	}


	pascal void
DebugTraps_BlockMoveData(
	const void	*srcPtr,
	void		*destPtr,
	Size		byteCount )
	{
	const void	*sysZone;
	
	pgpAssertAddrValidMsg( srcPtr, VoidAlign, "DebugTraps_BlockMoveData");
	pgpAssertAddrValidMsg( destPtr, VoidAlign, "DebugTraps_BlockMoveData");
	
	pgpAssertMsg( byteCount >= 0,
		"DebugTraps_BlockMoveData: negative PGPByte count");
	pgpAssertMsg( byteCount <= kMaxReasonableBlockMove,
		"DebugTraps_BlockMoveData: count is huge...is this correct?");

	sysZone	= SystemZone();
	pgpAssertMsg( srcPtr >= ( void *)sysZone,
		"DebugTraps_BlockMoveData: srcPtr below system zone");
	pgpAssertMsg( destPtr >= ( void *)sysZone,
		"DebugTraps_BlockMoveData: destPtr below system zone");
	
	
	BlockMoveData( srcPtr, destPtr, byteCount );
	}


	pascal Handle
DebugTraps_NewHandle(
	Size		byteCount,
	MDT_FileAndLineParams )
	{
	Handle	h;
	
	pgpAssertMsg( byteCount >= 0, "DebugTraps_NewHandle: negative count");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_NewHandle: MEMORY_ALLOC_MUST_BE_APP");
#elif MEMORY_ALLOC_MUST_BE_SYS
	pgpAssertMsg( GetZone() == SystemZone(),
		"DebugTraps_NewHandle: MEMORY_ALLOC_MUST_BE_SYS");
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	pgpAssertMsg( GetZone() == ApplicationZone() ||
		GetZone() == SystemZone(),
		"DebugTraps_NewHandle: MEMORY_ALLOC_MUST_BE_APP_OR_SYS");
#endif
		
	h	= NewHandle( byteCount );
	AssertMemErrIsNormal( "DebugTraps_NewHandle" );
	
	// byteCount is tested as well, because the memory manager has a bug that
	// causes it to succeed in creating a valid handle for small
	// negative sizes.  This bug may be fixed in 7.5.3 update 3.0
	if ( IsntNull( h ) && byteCount > 0 && LMGetMemErr() == noErr )
		{
		MacDebug_FillWithGarbage( *h, byteCount );
		
		MacLeaks_RememberHandle( h,
			kMacLeaks_NewHandleAllocType, srcFile_, srcLine_);
		}
	
	return( h );
	}




	pascal Handle
DebugTraps_NewHandleClear(
	Size		byteCount,
	MDT_FileAndLineParams	)
	{
	Handle	h;
	
	AssertValidMemoryAllocationSize( byteCount, "DebugTraps_NewHandleClear");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_NewHandleClear: MEMORY_ALLOC_MUST_BE_APP");
#elif MEMORY_ALLOC_MUST_BE_SYS
	pgpAssertMsg( GetZone() == SystemZone(),
		"DebugTraps_NewHandleClear: MEMORY_ALLOC_MUST_BE_SYS");
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	pgpAssertMsg( GetZone() == ApplicationZone() || GetZone() == SystemZone(),
		"DebugTraps_NewHandleClear: MEMORY_ALLOC_MUST_BE_APP_OR_SYS");
#endif
		
	h	= NewHandleClear( byteCount );
	AssertMemErrIsNormal( "DebugTraps_NewHandleClear" );
	
	if ( IsntNull( h ) )
	{	
		MacLeaks_RememberHandle( h,
			kMacLeaks_NewHandleClearAllocType, srcFile_, srcLine_);
	}
	
	return( h );
	}

	pascal Handle
DebugTraps_NewHandleSys(
	Size		byteCount,
	MDT_FileAndLineParams	)
	{
	Handle	h;
	
	AssertValidMemoryAllocationSize( byteCount, "DebugTraps_NewHandleSys");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_NewHandleSys: MEMORY_ALLOC_MUST_BE_APP");
#endif
		
	h	= NewHandleSys( byteCount );
	AssertMemErrIsNormal( "DebugTraps_NewHandleSys" );
	
	if ( IsntNull( h ) && LMGetMemErr() == noErr )
		{
		MacDebug_FillWithGarbage( *h, byteCount );
		
		MacLeaks_RememberHandle( h,
			kMacLeaks_NewHandleSysAllocType, srcFile_, srcLine_);
		}
	
	return( h );
	}


	pascal Handle
DebugTraps_NewHandleSysClear(
	Size		byteCount,
	MDT_FileAndLineParams	)
	{
	Handle	h;
	
	AssertValidMemoryAllocationSize( byteCount,
		"DebugTraps_NewHandleSysClear");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_NewHandleSysClear: MEMORY_ALLOC_MUST_BE_APP");
#endif
		
	h	= NewHandleSysClear( byteCount );
	AssertMemErrIsNormal( "DebugTraps_NewHandleSysClear" );
	
	if ( IsntNull( h ) )
	{
		MacLeaks_RememberHandle( h, kMacLeaks_NewHandleSysClearAllocType,
			srcFile_, srcLine_);
	}
	
	return( h );
	}


	pascal Size
DebugTraps_GetHandleSize(
	Handle h )
	{
	Size	theSize;
	OSErr	err;
	
	AssertHandleIsValid( h, "DebugTraps_GetHandleSize");
	
	theSize	= GetHandleSize( h );
	
	err	= LMGetMemErr();
	if ( err == nilHandleErr && IsNull( *h ) )
		{
		// normal to get this on purged Handle
		}
	else
		{
		AssertNoErr( err, "DebugTraps_GetHandleSize");
		}
	
	return( theSize );
	}



	pascal OSErr
DebugTraps_PtrToHand(
	const void	*srcPtr,
	Handle		*dstHndl,
	long		size,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	
	pgpAssertAddrValidMsg( srcPtr, VoidAlign, "DebugTraps_PtrToHand");
	pgpAssertAddrValidMsg( dstHndl, Ptr, "DebugTraps_PtrToHand");
	pgpAssertMsg( IsValidMemoryAllocationSize( size ), "DebugTraps_PtrToHand");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_PtrToHand: MEMORY_ALLOC_MUST_BE_APP");
#elif MEMORY_ALLOC_MUST_BE_SYS
	pgpAssertMsg( GetZone() == SystemZone(),
		"DebugTraps_PtrToHand: MEMORY_ALLOC_MUST_BE_SYS");
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	pgpAssertMsg( GetZone() == ApplicationZone() || GetZone() == SystemZone(),
		"DebugTraps_PtrToHand: MEMORY_ALLOC_MUST_BE_APP_OR_SYS");
#endif
	
	err	= PtrToHand( srcPtr, dstHndl, size );
	AssertNoErr( err, "DebugTraps_PtrToHand");
	
	if ( IsntErr( err ) && IsntNull( *dstHndl ) )
		{
		MacLeaks_RememberHandle( *dstHndl,
			kMacLeaks_PtrToHandAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}


	pascal Ptr
DebugTraps_NewPtr(
	Size		byteCount,
	MDT_FileAndLineParams	)
	{
	Ptr	newPtr;
	
	AssertValidMemoryAllocationSize( byteCount, "DebugTraps_NewPtr");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_NewPtr: MEMORY_ALLOC_MUST_BE_APP");
#elif MEMORY_ALLOC_MUST_BE_SYS
	pgpAssertMsg( GetZone() == SystemZone(),
		"DebugTraps_NewPtr: MEMORY_ALLOC_MUST_BE_SYS");
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	pgpAssertMsg( GetZone() == ApplicationZone() || GetZone() == SystemZone(),
		"DebugTraps_NewPtr: MEMORY_ALLOC_MUST_BE_APP_OR_SYS");
#endif

	newPtr	= NewPtr( byteCount );
	AssertMemErrIsNormal( "DebugTraps_NewPtr" );
	
	if ( IsntNull( newPtr ) && LMGetMemErr() == noErr )
		{
		MacDebug_FillWithGarbage( newPtr, byteCount );
		
		MacLeaks_RememberPtr( newPtr, kMacLeaks_NewPtrAllocType,
			byteCount, srcFile_, srcLine_);
		}
	
	return( newPtr );
	}


	pascal Ptr
DebugTraps_NewPtrClear(
	Size		byteCount,
	MDT_FileAndLineParams	)
	{
	Ptr	newPtr;
	
	AssertValidMemoryAllocationSize( byteCount, "DebugTraps_NewPtrClear");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_NewPtrClear: MEMORY_ALLOC_MUST_BE_APP");
#elif MEMORY_ALLOC_MUST_BE_SYS
	pgpAssertMsg( GetZone() == SystemZone(),
		"DebugTraps_NewPtrClear: MEMORY_ALLOC_MUST_BE_SYS");
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	pgpAssertMsg( GetZone() == ApplicationZone() || GetZone() == SystemZone(),
		"DebugTraps_NewPtrClear: MEMORY_ALLOC_MUST_BE_APP_OR_SYS");
#endif
		
	newPtr	= NewPtrClear( byteCount );
	AssertMemErrIsNormal( "DebugTraps_NewPtrClear" );
	
	if ( IsntNull( newPtr ) )
	{
		MacLeaks_RememberPtr( newPtr, kMacLeaks_NewPtrClearAllocType,
			byteCount, srcFile_, srcLine_);
	}
	
	return( newPtr );
	}


	pascal Ptr
DebugTraps_NewPtrSys(
	Size		byteCount,
	MDT_FileAndLineParams	)
	{
	Ptr	newPtr;
	
	AssertValidMemoryAllocationSize( byteCount, "DebugTraps_NewPtrSys");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_NewPtrSys: MEMORY_ALLOC_MUST_BE_APP");
#endif

	newPtr	= NewPtrSys( byteCount );
	AssertMemErrIsNormal( "DebugTraps_NewPtrSys" );
	
	if ( IsntNull( newPtr ) && LMGetMemErr() == noErr )
		{
		MacDebug_FillWithGarbage( newPtr, byteCount );
		
		MacLeaks_RememberPtr( newPtr, kMacLeaks_NewPtrSysAllocType,
			byteCount, srcFile_, srcLine_);
		}
	
	return( newPtr );
	}


	pascal Ptr
DebugTraps_NewPtrSysClear(
	Size		byteCount,
	MDT_FileAndLineParams	)
	{
	Ptr	newPtr;
	
	AssertValidMemoryAllocationSize( byteCount, "DebugTraps_NewPtrSysClear");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_NewPtrSysClear: MEMORY_ALLOC_MUST_BE_APP");
#endif
	
	newPtr	= NewPtrSysClear( byteCount );
	
	AssertMemErrIsNormal( "DebugTraps_NewPtrSysClear" );
	
	if ( IsntNull( newPtr ) )
	{
		MacLeaks_RememberPtr( newPtr, kMacLeaks_NewPtrSysClearAllocType,
			byteCount, srcFile_, srcLine_);
	}
	
	return( newPtr );
	}


	pascal Size
DebugTraps_GetPtrSize( Ptr newPtr )
	{
	Size	theSize;
	
	theSize	= GetPtrSize( newPtr );
	AssertNoErr( LMGetMemErr(), "DebugTraps_GetPtrSize" );
	
	return( theSize );
	}


	pascal void
DebugTraps_SetPtrSize(
	Ptr		thePtr,
	Size	newSize )
	{
	AssertValidMemoryAllocationSize( newSize, "DebugTraps_SetPtrSize");
	
	SetPtrSize( thePtr, newSize );
	
	AssertMemErrIsNormal( "DebugTraps_SetPtrSize" );
	}


	pascal THz
DebugTraps_PtrZone( Ptr thePtr )
	{
	THz	zone;
	
	zone	= PtrZone( thePtr );
	AssertMemErrIsNormal( "DebugTraps_PtrZone" );
	
	return( zone );
	}


	pascal void
DebugTraps_DisposePtr( Ptr thePtr )
	{
	UInt32	ptrSize;
	
	pgpAssertAddrValidMsg( thePtr, long, "DebugTraps_DisposePtr");
	
	// fill the handle with junk prior to disposing it
	ptrSize	= GetPtrSize( thePtr );
	if ( LMGetMemErr() == noErr )
		{
		if ( ! DebugPatches_DisposeTrapsPatched() )
			{
			MacDebug_FillWithGarbage( thePtr, ptrSize );
			}
		}
	
	DisposePtr( thePtr );
	MacLeaks_ForgetPtr( thePtr, kMacLeaks_DisposePtrDeallocType );
	
	AssertNoErr( LMGetMemErr(), "DebugTraps_DisposePtr");
	}




	pascal void
DebugTraps_SetHandleSize(
	Handle	h,
	Size	newSize )
	{
	Size	oldSize;
	SInt8	hState;
	OSErr	err	= noErr;
	Boolean	success	= FALSE;
	
	AssertValidMemoryAllocationSize( newSize, "DebugTraps_SetHandleSize");
	AssertHandleIsValid( h, "DebugTraps_SetHandleSize");
	
	hState	= DebugTraps_HGetState( h );
#if 0
	pgpAssertMsg( ( hState & kHStateLockedMask ) == 0,
		"DebugTraps_SetHandleSize: "
		"changing size of locked Handle is a bug on System 8");
#endif

	oldSize	= GetHandleSize( h );
	
	SetHandleSize( h, newSize );
	err	= LMGetMemErr();
	AssertMemErrIsNormal( "DebugTraps_SetHandleSize" );
	
	success	= GetHandleSize( h ) == newSize;
	
	// we should either have the correct size Handle or an error
	pgpAssertMsg( IsErr( err ) || success,
		"DebugTraps_SetHandleSize: inconsistent results");
	
	if ( success && ( newSize > oldSize ) )
		{
		MacDebug_FillWithGarbage( (( char *)*h ) + oldSize,
			newSize - oldSize );
		}
	}


	pascal void
DebugTraps_ReallocateHandle(
	Handle	h,
	Size	byteCount )
	{
	AssertValidMemoryAllocationSize( byteCount, "DebugTraps_ReallocateHandle");
	AssertHandleIsValid( h, "DebugTraps_ReallocateHandle");
	
	ReallocateHandle( h, byteCount );
	
	if ( LMGetMemErr() == noErr )
		{
		MacDebug_FillWithGarbage( *h, byteCount );
		}
	else
		{
		AssertMemErrIsNormal( "DebugTraps_ReallocateHandle" );
		}
	}


	pascal void
DebugTraps_HLock(
	Handle h )
	{
	AssertHandleIsValid( h, "stdHLock");
	pgpAssertMsg( ! HandleIsPurged( h ),
		"DebugTraps_HLock: locking purged Handle");
	
	HLock( h );
	AssertNoErr( LMGetMemErr(), "DebugTraps_HLock");
	}


	pascal void
DebugTraps_HUnlock(
	Handle h )
	{
	AssertHandleIsValid( h, "stdHUnlock");
	
	HUnlock( h );
	AssertNoErr( LMGetMemErr(), "DebugTraps_HUnlock");
	}


	pascal SInt8
DebugTraps_HGetState(
	Handle h )
	{
	SInt8	hState;
	OSErr	err;
	
	AssertHandleIsValid( h, "DebugTraps_HGetState");
	
	hState	= HGetState( h );
	
	err	= LMGetMemErr();
	if ( err == nilHandleErr && IsNull( *h ) )
		{
		// normal to get this on purged Handle
		}
	else
		{
		AssertNoErr( err, "DebugTraps_HGetState");
		}
	
	return( hState );
	}


	pascal void
DebugTraps_HSetState(
	Handle	h,
	SInt8	flags )
	{
	OSErr	err;
	
	AssertHandleIsValid( h, "DebugTraps_HSetState");
	
	HSetState( h, flags );
	
	err	= LMGetMemErr();
	if ( err == nilHandleErr && IsNull( *h ) )
		{
		// normal to get this on purged Handle
		}
	else
		{
		AssertNoErr( err, "DebugTraps_HSetState");
		}
	}


	pascal OSErr
DebugTraps_HandAndHand(
	Handle	hand1,
	Handle	hand2)
	{
	OSErr	err;
	
	AssertHandleIsValid( hand1, "DebugTraps_HandAndHand");
	AssertHandleIsValid( hand2, "DebugTraps_HandAndHand");
	pgpAssertMsg( ! HandleIsPurgeableAndNotLocked( hand1 ),
		"DebugTraps_HandAndHand: purgeable hand1");
	pgpAssertMsg( ! HandleIsPurgeableAndNotLocked( hand2 ),
		"DebugTraps_HandAndHand: purgeable hand2");
	
	err	= HandAndHand( hand1, hand2 );
	AssertMemErrIsNormal( "DebugTraps_HandAndHand" );
	
	return( err );
	}


	pascal OSErr
DebugTraps_HandToHand(
	Handle		*theHandlePtr,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	Handle	temp;
	
	AssertHandleIsValid( *theHandlePtr, "DebugTraps_HandToHand");
	pgpAssertMsg( ! HandleIsPurgeableAndNotLocked( *theHandlePtr ),
		"DebugTraps_HandToHand: attempt to duplicate purgeable Handle");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"DebugTraps_HandToHand: MEMORY_ALLOC_MUST_BE_APP");
#elif MEMORY_ALLOC_MUST_BE_SYS
	pgpAssertMsg( GetZone() == SystemZone(),
		"DebugTraps_HandToHand: MEMORY_ALLOC_MUST_BE_SYS");
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	pgpAssertMsg( GetZone() == ApplicationZone() || GetZone() == SystemZone(),
		"DebugTraps_HandToHand: MEMORY_ALLOC_MUST_BE_APP_OR_SYS");
#endif
	
	temp	= *theHandlePtr;
	err	= HandToHand( theHandlePtr );
	AssertMemErrIsNormal( "DebugTraps_HandToHand" );
	
	if ( IsntErr( err ) )
	{
		pgpAssertMsg( GetHandleSize( temp ) == GetHandleSize( *theHandlePtr ),
			"DebugTraps_HandToHand: different size");
			
		pgpAssertMsg( temp != *theHandlePtr,
			"DebugTraps_HandToHand: handle the same");
		
		MacLeaks_RememberHandle( *theHandlePtr,
			kMacLeaks_HandToHandAllocType, srcFile_, srcLine_);
	}
	
	return( err );
	}


	pascal void
DebugTraps_HPurge(
	Handle h )
	{
	OSErr	err;
	
	AssertHandleIsValid( h, "DebugTraps_HPurge");
	
	HPurge( h );
	
	err	= LMGetMemErr();
	if ( err == nilHandleErr && IsNull( *h ) )
		{
		// normal to get error on purged Handle
		}
	else
		{
		AssertNoErr( err, "DebugTraps_HPurge");
		}
	}


	pascal void
DebugTraps_EmptyHandle(
	Handle	h )
	{
	OSErr	err;
	UInt32	hSize;
	
	AssertHandleIsValid( h, "DebugTraps_EmptyHandle");
	
	hSize	= GetHandleSize( h );
	if ( LMGetMemErr() == noErr )
		{
		pgpAssertMsg( IsntNull( *h ),
			"DebugTraps_EmptyHandle: master pointer is nil");
		MacDebug_FillWithGarbage( *h, hSize );
		}
	
	EmptyHandle( h );
	err	= LMGetMemErr();
	AssertNoErr( err, "DebugTraps_EmptyHandle");
	}


	pascal void
DebugTraps_HNoPurge(
	Handle h )
	{
	AssertHandleIsValid( h, "DebugTraps_HNoPurge");
	pgpAssertMsg( ! HandleIsPurged( h ),
		"DebugTraps_HNoPurge: handle already purged");
	
	HNoPurge( h );
	AssertNoErr( LMGetMemErr(), "DebugTraps_HNoPurge");
	}


	pascal THz
DebugTraps_HandleZone(
	Handle h )
	{
	THz		zone;
	AssertHandleIsValid( h, "DebugTraps_HandleZone");
	
	zone	= HandleZone( h );
	AssertNoErr( LMGetMemErr(), "DebugTraps_HandleZone");
	pgpAssertMsg( IsntNull( zone ), "DebugTraps_HandleZone");
	
	return( zone );
	}


	pascal Handle
DebugTraps_RecoverHandle( Ptr thePtr )
	{
	Handle	h;
	
	pgpAssertAddrValidMsg( thePtr, long, "DebugTraps_RecoverHandle");
	
	h	= RecoverHandle( thePtr );
	
	AssertNoErr( LMGetMemErr(), "DebugTraps_RecoverHandle");
	AssertHandleIsValid( h, "DebugTraps_RecoverHandle");
	
	return( h );
	}


	pascal Handle
DebugTraps_RecoverHandleSys( Ptr thePtr )
	{
	Handle	h;
	
	pgpAssertAddrValidMsg( thePtr, long, "DebugTraps_RecoverHandleSys");
	
	h	= RecoverHandleSys( thePtr );
	
	AssertNoErr( LMGetMemErr(), "DebugTraps_RecoverHandleSys");
	AssertHandleIsValid( h, "DebugTraps_RecoverHandleSys");
	
	return( h );
	}


	pascal void
DebugTraps_SetZone( THz hz )
	{
	pgpAssertAddrValidMsg( hz, long, "DebugTraps_SetZone");
	
	SetZone( hz );
	AssertNoErr( LMGetMemErr(), "DebugTraps_SetZone");
	}


	pascal THz
DebugTraps_GetZone()
	{
	THz	hz;
	
	hz	= GetZone();
	AssertNoErr( LMGetMemErr(), "DebugTraps_GetZone");
	
	return( hz );
	}


	pascal Handle
DebugTraps_TempNewHandle(
	Size		logicalSize,
	OSErr		*resultCode,
	MDT_FileAndLineParams	)
	{
	Handle	h;
	
	AssertValidMemoryAllocationSize( logicalSize, "DebugTraps_TempNewHandle");
#if MEMORY_ALLOC_MUST_BE_APP
	pgpDebugPStr( "\pDebugTraps_TempNewHandle: MEMORY_ALLOC_MUST_BE_APP");
#elif MEMORY_ALLOC_MUST_BE_SYS
	pgpDebugPStr( "\pDebugTraps_TempNewHandle: MEMORY_ALLOC_MUST_BE_SYS");
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	pgpDebugPStr( "\pDebugTraps_TempNewHandle: "
	"MEMORY_ALLOC_MUST_BE_APP_OR_SYS");
#endif
	
	pgpAssertAddrValidMsg( resultCode, OSErr, "DebugTraps_TempNewHandle");
	
	h	= TempNewHandle( logicalSize, resultCode );

	if ( ! MemErrIsNormal( *resultCode ) )
		{
		AssertNoErr( *resultCode, "DebugTraps_TempNewHandle");
		}
	
	if ( IsntNull( h ) && *resultCode == noErr )
		{
		MacDebug_FillWithGarbage( *h, logicalSize );
		
		MacLeaks_RememberHandle( h,
			kMacLeaks_TempNewHandleAllocType, srcFile_, srcLine_);
		}
	
	return( h );
	}


	pascal void
DebugTraps_TempDisposeHandle(
	Handle	h,
	OSErr	*err )
	{
	AssertHandleIsValid( h , "DebugTraps_TempDisposeHandle");
	pgpAssertAddrValid( err, OSErr );
	
	TempDisposeHandle( h, err );
	AssertNoErr( *err, "DebugTraps_TempDisposeHandle");
	
	MacLeaks_ForgetHandle( h, kMacLeaks_DisposeHandleDeallocType );
	}


	pascal void
DebugTraps_MoveHHi(
	Handle h )
	{
	AssertHandleIsValid( h , "DebugTraps_MoveHHi");
	
	MoveHHi( h );
	
	AssertNoErr( LMGetMemErr(), "DebugTraps_MoveHHi");
	}


	pascal OSErr
DebugTraps_PtrAndHand(
	const void	*ptr,
	Handle		hand,
	long		size )
	{
	OSErr	err;
	
	AssertHandleIsValid( hand , "DebugTraps_PtrAndHand");
	pgpAssertAddrValidMsg( ptr , VoidAlign, "DebugTraps_PtrAndHand");
	AssertValidMemoryAllocationSize( size, "DebugTraps_PtrAndHand");
	
	err	= PtrAndHand( ptr, hand, size );
	
	if ( ! MemErrIsNormal( err ) )
		{
		AssertNoErr( err, "DebugTraps_PtrAndHand");
		}
	
	return( err );
	}





//____________________________Resource Manager _______________________________

#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Resource Manager ---
#endif


	pascal void
DebugTraps_CloseResFile( short refNum )
	{
	AssertFileRefNumIsValid( refNum, "DebugTraps_CloseResFile");
	
	CloseResFile( refNum );
	}


	pascal void
DebugTraps_UseResFile( short refNum )
	{
	AssertFileRefNumIsValid( refNum, "DebugTraps_UseResFile");
	
	UseResFile( refNum );
	}
	
	
	pascal short
DebugTraps_HomeResFile(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource , "DebugTraps_HomeResFile");
	
	short	refNum;
	
	refNum	= HomeResFile( theResource );
	return( refNum );
	}
	
	
	pascal void
DebugTraps_CreateResFile( ConstStr255Param fileName )
	{
	pgpAssertAddrValidMsg( fileName, uchar, "DebugTraps_CreateResFile");
	
	CreateResFile( fileName );
	}
	
	
	pascal short
DebugTraps_OpenResFile( ConstStr255Param fileName )
	{
	pgpAssertAddrValidMsg( fileName, uchar, "DebugTraps_OpenResFile");
	
	short refNum	= OpenResFile( fileName );
	
	return( refNum );
	}
	
	
	pascal void
DebugTraps_GetIndType(
	ResType *theType,
	short	index )
	{
	pgpAssertAddrValidMsg( theType, ResType, "DebugTraps_GetIndType");
	pgpAssertMsg( index > 0, "DebugTraps_GetIndType: illegal index");
	
	GetIndType( theType, index );
	}
	
	
	pascal void
DebugTraps_Get1IndType(
	ResType *theType,
	short	index )
	{
	pgpAssertAddrValidMsg( theType, ResType, "DebugTraps_GetIndType");
	pgpAssertMsg( index > 0, "DebugTraps_GetIndType: illegal index");
	
	Get1IndType( theType, index );
	}
	
	
	pascal Handle
DebugTraps_GetIndResource(
	ResType theType,
	short	index )
	{
	pgpAssertMsg( index > 0, "DebugTraps_GetIndResource: illegal index");
	
	Handle	theResource	= GetIndResource( theType, index );
	return( theResource );
	}
	
	
	pascal Handle
DebugTraps_Get1IndResource(
	ResType theType,
	short	index )
	{
	pgpAssertMsg( index > 0, "DebugTraps_Get1IndResource: illegal index");
	
	Handle	theResource	= Get1IndResource( theType, index );
	return( theResource );
	}
	
	
	pascal Handle
DebugTraps_GetResource(
	ResType	theType,
	short	theID )
	{
	Handle	theResource	= GetResource( theType, theID );
	return( theResource );
	}
	
	
	pascal Handle
DebugTraps_Get1Resource(
	ResType		theType,
	short		theID )
	{
	Handle	theResource	= Get1Resource( theType, theID );
	return( theResource );
	}
	
	
	pascal Handle
DebugTraps_GetNamedResource(
	ResType				theType, 
	ConstStr255Param	name )
	{
	pgpAssertAddrValidMsg( name, uchar, "DebugTraps_GetNamedResource");
	
	Handle	theResource	= GetNamedResource( theType, name );
	return( theResource );
	}
	
	
	pascal Handle
DebugTraps_Get1NamedResource(
	ResType				theType,
	ConstStr255Param	name )
	{
	pgpAssertAddrValidMsg( name, uchar, "DebugTraps_Get1NamedResource");
	
	Handle	theResource	= Get1NamedResource( theType, name );
	return( theResource );
	}
	
	
	pascal void
DebugTraps_LoadResource(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource, "DebugTraps_LoadResource");
	
	LoadResource( theResource );
	}
	
	pascal void
DebugTraps_ReleaseResource(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource , "DebugTraps_ReleaseResource");
	
	SInt8	hState	= HGetState( theResource );
	pgpAssertMsg( HStateIsResource( hState ) || IsNull( *theResource ),
		"DebugTraps_ReleaseResource: not a resource");
	
	ReleaseResource( theResource );
	
	AssertNoErr( LMGetResErr(), "DebugTraps_ReleaseResource");
	}

	
	
	pascal void
DebugTraps_DetachResource(
	Handle 			theResource,
	MDT_FileAndLineParams	)
	{
	AssertHandleIsValid( theResource, "DebugTraps_DetachResource");
	pgpAssertMsg( ! HandleIsPurgeableAndNotLocked( theResource ),
		"DebugTraps_DetachResource: "
		"detaching a purgeable, non-locked resource");
	
	if ( IsntNull( theResource ) )
	{	
		MacLeaks_RememberHandle( theResource,
			kMacLeaks_DetachResourceAllocType, srcFile_, srcLine_);
	}
	
	DetachResource( theResource );
	}
	
	
	
	pascal short
DebugTraps_GetResAttrs(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource, "DebugTraps_GetResAttrs");
	
	short	attrs	= GetResAttrs( theResource );
	return( attrs );
	}
	
	
	pascal void
DebugTraps_GetResInfo(
	Handle		theResource,
	short *		theID,
	ResType *	theType,
	Str255		name )
	{
	AssertHandleIsValid( theResource, "DebugTraps_GetResInfo");
	pgpAssertAddrValidMsg( theID, short, "DebugTraps_GetResInfo");
	pgpAssertAddrValidMsg( theType, ResType, "DebugTraps_GetResInfo");
	pgpAssertAddrValidMsg( name, uchar, "DebugTraps_GetResInfo");
	
	GetResInfo( theResource, theID, theType, name );
	}
	
	
	pascal void
DebugTraps_SetResInfo(
	Handle				theResource,
	short				theID,
	ConstStr255Param	name )
	{
	AssertHandleIsValid( theResource, "DebugTraps_SetResInfo");
	pgpAssertAddrValidMsg( name, uchar, "DebugTraps_GetResInfo");
	
	SetResInfo( theResource, theID, name );
	}
	
	
	pascal void
DebugTraps_AddResource(
	Handle				theData,
	ResType				theType,
	short				theID,
	ConstStr255Param	name )
	{
	AssertHandleIsValid( theData, "DebugTraps_AddResource");
	pgpAssertAddrValidMsg( name, uchar, "DebugTraps_AddResource");
	
	AddResource( theData, theType, theID, name );
	if ( LMGetResErr() == noErr )
		{
		#if USE_MAC_DEBUG_LEAKS
		if ( MacLeaks_ItemIsRemembered( theData ) )
			{
			MacLeaks_ForgetHandle( theData,
				kMacLeaks_DisposeHandleDeallocType);
			}
		#endif
		}
	}
	
	
	pascal long
DebugTraps_GetResourceSizeOnDisk(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource, "DebugTraps_GetResourceSizeOnDisk");
	
	long	resSize	= GetResourceSizeOnDisk( theResource );
	return( resSize );
	}
	
	
	pascal long
DebugTraps_GetMaxResourceSize(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource, "DebugTraps_GetMaxResourceSize");
	
	long	resSize	= GetMaxResourceSize( theResource );
	return( resSize );
	}
	
	
	pascal long
DebugTraps_RsrcMapEntry(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource, "DebugTraps_RsrcMapEntry");
	
	long	offset	= RsrcMapEntry( theResource );
	return( offset );
	}
	
	
	pascal void
DebugTraps_SetResAttrs(
	Handle	theResource,
	short	attrs )
	{
	AssertHandleIsValid( theResource, "DebugTraps_SetResAttrs");
	
	SetResAttrs( theResource, attrs );
	}
	
	
	pascal void
DebugTraps_ChangedResource(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource, "DebugTraps_ChangedResource");
	
	ChangedResource( theResource );
	}
	
	
	pascal void
DebugTraps_RemoveResource(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource, "DebugTraps_RemoveResource");
	
	RemoveResource( theResource );
	}
	
	
	pascal void
DebugTraps_UpdateResFile( short refNum )
	{
	AssertFileRefNumIsValid( refNum, "DebugTraps_UpdateResFile");
	
	UpdateResFile( refNum );
	}
	
	
	pascal void
DebugTraps_WriteResource(
	Handle	theResource )
	{
	AssertHandleIsValid( theResource, "DebugTraps_WriteResource");
	
	WriteResource( theResource );
	}
	
	
	pascal short
DebugTraps_GetResFileAttrs( short refNum )
	{
	AssertFileRefNumIsValid( refNum, "DebugTraps_GetResFileAttrs");
	
	short	attrs	= GetResFileAttrs( refNum );
	return( attrs );
	}
	
	
	pascal void
DebugTraps_SetResFileAttrs(
	short	refNum,
	short	attrs )
	{
	AssertFileRefNumIsValid( refNum, "DebugTraps_SetResFileAttrs");
	
	SetResFileAttrs( refNum, attrs );
	}
	
	
	pascal short
DebugTraps_OpenRFPerm(
	ConstStr255Param	fileName,
	short				vRefNum,
	SInt8				permission )
	{
	pgpAssertAddrValidMsg( fileName, uchar, "DebugTraps_OpenRFPerm");
	
	short	refNum	= OpenRFPerm( fileName, vRefNum, permission );
	
	return( refNum );
	}
	
	
	
	pascal short
DebugTraps_HOpenResFile(
	short				vRefNum,
	long				dirID,
	ConstStr255Param	fileName,
	SInt8				permission )
	{
	pgpAssertAddrValidMsg( fileName, uchar, "DebugTraps_HOpenResFile");
	
	short	refNum	= HOpenResFile( vRefNum, dirID, fileName, permission );
	return( refNum );
	}
	
	
	pascal void
DebugTraps_HCreateResFile(
	short				vRefNum,
	long				dirID,
	ConstStr255Param	fileName )
	{
	pgpAssertAddrValidMsg( fileName, uchar, "DebugTraps_HCreateResFile");
	
	HCreateResFile( vRefNum, dirID, fileName );
	}
	
	
	pascal short
DebugTraps_FSpOpenResFile(
	const FSSpec *	spec,
	SignedByte		permission )
	{
	AssertSpecIsValid( spec, "DebugTraps_FSpOpenResFile");
	
	short	refNum	= FSpOpenResFile( spec, permission );
	return( refNum );
	}
	
	
	pascal void
DebugTraps_FSpCreateResFile(
	const FSSpec *	spec,
	OSType			creator,
	OSType 			fileType,
	ScriptCode		scriptTag )
	{
	AssertSpecIsValid( spec, "DebugTraps_FSpCreateResFile");
	
	FSpCreateResFile( spec, creator, fileType, scriptTag );
	}
	
	
	pascal void
DebugTraps_ReadPartialResource(
	Handle	theResource,
	long 	offset,
	void *	buffer,
	long	count )
	{
	AssertHandleIsValid( theResource, "DebugTraps_ReadPartialResource");
	pgpAssertAddrValidMsg( buffer, VoidAlign,
		"DebugTraps_ReadPartialResource");
	pgpAssertMsg( count > 0, "DebugTraps_ReadPartialResource: count <= 0");
	pgpAssertMsg( offset >= 0, "DebugTraps_ReadPartialResource: offset < 0");
	
	ReadPartialResource( theResource, offset, buffer, count );
	}
	
	
	pascal void
DebugTraps_WritePartialResource(
	Handle			theResource,
	long 			offset,
	const void *	buffer,
	long			count )
	{
	AssertHandleIsValid( theResource, "DebugTraps_WritePartialResource");
	pgpAssertAddrValidMsg( buffer, VoidAlign,
		"DebugTraps_WritePartialResource");
	pgpAssertMsg( count > 0, "DebugTraps_WritePartialResource: count <= 0");
	pgpAssertMsg( offset >= 0, "DebugTraps_WritePartialResource: offset < 0");
	
	WritePartialResource( theResource, offset, buffer, count );
	}
	
	
	pascal void
DebugTraps_SetResourceSize(
	Handle	theResource,
	long	newSize )
	{
	AssertHandleIsValid( theResource, "DebugTraps_SetResourceSize");
	pgpAssertMsg( newSize >= 0, "DebugTraps_SetResourceSize: newSize < 0");
	
	SetResourceSize( theResource, newSize );
	}
	
	
	pascal Handle
DebugTraps_GetNextFOND( Handle	fondHandle )
	{
	AssertHandleIsValid( fondHandle, "DebugTraps_GetNextFOND");
	
	Handle	nextFOND	= GetNextFOND( fondHandle );
	return( nextFOND );
	}



#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Quickdraw ---
#endif



	pascal RgnHandle
DebugTraps_NewRgn( MDT_FileAndLineParams	)
	{
	RgnHandle	theRgn	= NewRgn();
	
	if ( IsntNull( theRgn ) )
	{	
		MacLeaks_RememberHandle( (Handle )theRgn,
			kMacLeaks_NewRgnAllocType, srcFile_, srcLine_);
	}
	
	return( theRgn );
	}


	pascal void
DebugTraps_DisposeRgn( RgnHandle	theRgn )
	{
	AssertHandleIsValid( theRgn, "Debug_TrapsDisposeRgn" );
	
	MacLeaks_ForgetHandle( (Handle)theRgn,
		kMacLeaks_DisposeRgnDeallocType );
		
	DisposeRgn( theRgn );
	}


#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Menu Mgr ---
#endif

	pascal MenuRef
DebugTraps_NewMenu(
	short				menuID,
	ConstStr255Param	menuTitle,
	MDT_FileAndLineParams	)
	{
	MenuHandle	theMenu	= nil;
	pgpAssertAddrValidMsg( menuTitle, uchar, "DebugTraps_NewMenu");
	
	theMenu	= NewMenu( menuID, menuTitle );
	DebugMsgIf( IsNull( theMenu ), "DebugTraps_NewMenu: NewMenu failed");
	
	if ( IsntNull( theMenu ) )
	{	
		MacLeaks_RememberHandle( (Handle )theMenu,
			kMacLeaks_NewMenuAllocType, srcFile_, srcLine_);
	}
	
	return( theMenu );
	}



	pascal Handle
DebugTraps_GetMenuBar( MDT_FileAndLineParams	)
	{
	Handle	theMenuBar	= nil;
	
	theMenuBar	= GetMenuBar( );
	DebugMsgIf( IsNull( theMenuBar ), "DebugTraps_GetMenuBar: failed");
	
	if ( IsntNull( theMenuBar ) )
	{	
		MacLeaks_RememberHandle( (Handle)theMenuBar,
			kMacLeaks_GetMenuBarAllocType, srcFile_, srcLine_);
	}
	
	return( theMenuBar );
	}
	
	
	

	pascal Handle
DebugTraps_GetNewMBar(
	short			menuBarID,
	MDT_FileAndLineParams	)
	{
	Handle	theMenuBar	= nil;
	
	theMenuBar	= GetNewMBar( menuBarID );
	DebugMsgIf( IsNull( theMenuBar ), "DebugTraps_GetNewMBar: failed");
	
	if ( IsntNull( theMenuBar ) )
	{	
		MacLeaks_RememberHandle( (Handle )theMenuBar,
			kMacLeaks_GetNewMBarAllocType, srcFile_, srcLine_);
	}
	
	return( theMenuBar );
	}




#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Icons ---
#endif

	pascal OSErr
DebugTraps_NewIconSuite(
	Handle *		theIconSuite,
	MDT_FileAndLineParams	)
	{
	OSErr	err	= noErr;
	
	err	= NewIconSuite( theIconSuite );
	DebugMsgIf( IsNull( *theIconSuite ), "DebugTraps_NewIconSuite: failed");
	
	if ( IsntNull( *theIconSuite ) )
	{	
		MacLeaks_RememberHandle( (Handle )*theIconSuite,
			kMacLeaks_NewIconSuiteAllocType,
			srcFile_, srcLine_);
	}
	
	return( err );
	}


	pascal OSErr
DebugTraps_GetIconSuite(
	Handle *			theIconSuite,
	SInt16				theResID,
	IconSelectorValue	selector,
	MDT_FileAndLineParams)
	{
	OSErr	err	= noErr;
	
	err	= GetIconSuite( theIconSuite, theResID, selector );
	AssertNoErr( err, "DebugTraps_GetIconSuite" );
	
	if ( IsntErr( err ) )
	{	
		MacLeaks_RememberHandle( (Handle )*theIconSuite,
			kMacLeaks_GetIconSuiteAllocType, srcFile_, srcLine_);
	}
	
	return( err );
	}


	pascal OSErr
DebugTraps_DisposeIconSuite(
	Handle theIconSuite, Boolean disposeData )
	{
	OSErr	err	= noErr;
	
	AssertHandleIsValid( theIconSuite, "DebugTraps_DisposeIconSuite" );
	
	MacLeaks_ForgetHandle( theIconSuite,
		kMacLeaks_DisposeIconSuiteDeallocType );
	
	err	= DisposeIconSuite( theIconSuite, disposeData );
	AssertNoErr( err, "DebugTraps_DisposeIconSuite" );
	
	return( err );
	}



#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Alias Mgr ---
#endif



	pascal OSErr
DebugTraps_NewAlias(
	ConstFSSpecPtr	fromFile,
	const FSSpec *	target,
	AliasHandle *	alias,
	MDT_FileAndLineParams	)
	{
#if PGP_DEBUG
	if ( IsntNull( fromFile ) )
		{
		AssertSpecIsValid( fromFile, "DebugTraps_NewAlias");
		}
	AssertSpecIsValid( target, "DebugTraps_NewAlias");
	pgpAssertAddrValidMsg( alias, AliasHandle, "DebugTraps_NewAlias");
#endif

	OSErr	err	= noErr;
	
	err	= NewAlias( fromFile, target, alias );
	AssertNoErr( err, "DebugTraps_NewAlias");
	
	if ( IsntNull( *alias ) )
	{	
		MacLeaks_RememberHandle( (Handle)*alias,
			kMacLeaks_NewAliasAllocType,
			srcFile_, srcLine_);
	}
	
	return( err );
	}
	
	
	pascal OSErr
DebugTraps_NewAliasMinimal(
	const FSSpec *	target,
	AliasHandle *	alias,
	MDT_FileAndLineParams	)
	{
	AssertSpecIsValid( target, "DebugTraps_NewAliasMinimal");
	pgpAssertAddrValidMsg( alias, AliasHandle, "DebugTraps_NewAliasMinimal");
	
	OSErr	err	= noErr;
	
	err	= NewAliasMinimal( target, alias );
	AssertNoErr( err, "DebugTraps_NewAliasMinimal");
	
	if ( IsntNull( *alias ) )
	{	
		MacLeaks_RememberHandle( (Handle)*alias,
			kMacLeaks_NewAliasMinimalAllocType,
			srcFile_, srcLine_);
	}
	
	return( err );
	}
	
	
	pascal OSErr
DebugTraps_NewAliasMinimalFromFullPath(
	short 			fullPathLength,
	const void *	fullPath,
	ConstStr32Param zoneName,
	ConstStr31Param serverName,
	AliasHandle *	alias,
	MDT_FileAndLineParams	)
	{
	pgpa((
		pgpaAssert( fullPathLength >= 1 ),
		pgpaAddrValid( fullPath, VoidAlign ),
		pgpaAddrValid( alias, AliasHandle ),
		pgpaMsg(( pgpaFmtPrefix, "DebugTraps_NewAliasMinimalFromFullPath")) ) );
	
	AssertAddrNullOrValid( zoneName, uchar,
		"DebugTraps_NewAliasMinimalFromFullPath");
	AssertAddrNullOrValid( serverName, uchar,
		"DebugTraps_NewAliasMinimalFromFullPath");
	
	OSErr	err	= noErr;
	
	err	= NewAliasMinimalFromFullPath( fullPathLength, fullPath, zoneName,
			serverName, alias );
	AssertNoErr( err, "DebugTraps_NewAliasMinimal");
	
	if ( IsntNull( *alias ) )
	{	
		MacLeaks_RememberHandle( (Handle)*alias,
			kMacLeaks_NewAliasMinimalFromFullPathAllocType,
			srcFile_, srcLine_);
	}
	
	return( err );
	}
	
	
#pragma mark -



// __________________________ Apple Event Related ____________________________

#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Apple Event related ---
#endif



	pascal OSErr
DebugTraps_AECreateDesc(
	DescType	typeCode,
	const void *dataPtr,
	Size		dataSize,
	AEDesc *	result,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	
	err	= AECreateDesc( typeCode, dataPtr, dataSize, result );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( result,
			kMacLeaks_AECreateDescAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
	
		
	pascal OSErr
DebugTraps_AECoercePtr(
	DescType		typeCode,
	const void *	dataPtr,
	Size			dataSize,
	DescType		toType,
	AEDesc *		result,
	MDT_FileAndLineParams )
	{
	OSErr	err;
	
	err	= AECoercePtr( typeCode, dataPtr, dataSize, toType, result );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( result,
			kMacLeaks_AECoercePtrAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}

		
	pascal OSErr
DebugTraps_AECoerceDesc(
	const AEDesc *		theAEDesc,
	DescType			toType,
	AEDesc *			result,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	
	err	= AECoerceDesc( theAEDesc, toType, result );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( result,
			kMacLeaks_AECoerceDescAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
	
				
	pascal OSErr
DebugTraps_AEDisposeDesc( AEDesc *		theAEDesc )
	{
	OSErr	err;
	
	MacLeaks_ForgetAEDesc( theAEDesc, kMacLeaks_DisposeAEDescDeallocType);
		
	err	= AEDisposeDesc( theAEDesc );
	AssertNoErr( err, "DebugTraps_AEDisposeDesc" );
	
	return( err );
	}
	
	
	pascal OSErr
DebugTraps_AEDuplicateDesc(
	const AEDesc *	theAEDesc,
	AEDesc *		result,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	
	err	= AEDuplicateDesc( theAEDesc, result );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( result,
			kMacLeaks_AEDuplicateDescAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
	

	pascal OSErr
DebugTraps_AECreateList(
	const void *	factoringPtr,
	Size			factoredSize,
	Boolean			isRecord,
	AEDescList *	resultList,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	
	err	= AECreateList( factoringPtr, factoredSize, isRecord, resultList );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( resultList,
			kMacLeaks_AECreateListAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
	

	pascal OSErr
DebugTraps_AEGetNthDesc(
	const AEDescList *	theAEDescList,
	long				index,
	DescType			desiredType,
	AEKeyword *			theAEKeyword,
	AEDesc *			result,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	
	err	= AEGetNthDesc( theAEDescList,
		index, desiredType, theAEKeyword, result );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( result,
			kMacLeaks_AEGetNthDescAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
	

	pascal OSErr
DebugTraps_AEGetAttributeDesc(
	const AppleEvent *	theAppleEvent,
	AEKeyword			theAEKeyword,
	DescType 			desiredType,
	AEDesc *			result,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	
	err	= AEGetAttributeDesc( theAppleEvent,
				theAEKeyword, desiredType, result );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( result,
			kMacLeaks_AEGetAttributeDescAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
	

	pascal OSErr
DebugTraps_AECreateAppleEvent(
	AEEventClass			theAEEventClass,
	AEEventID				theAEEventID,
	const AEAddressDesc *	target,
	short					returnID,
	long					transactionID,
	AppleEvent *			result,
	MDT_FileAndLineParams	)
	{
	AssertAEDescIsValid( target, "DebugTraps_AECreateAppleEvent");
	
	OSErr	err;
	
	err	= AECreateAppleEvent( theAEEventClass,
			theAEEventID, target, returnID, transactionID, result );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( result,
			kMacLeaks_AECreateAppleEventAllocType, srcFile_, srcLine_);
		}
		
	return( err );
	}
	

	pascal OSErr
DebugTraps_AESend(
	const AppleEvent *	theAppleEvent,
	AppleEvent *		reply,
	AESendMode			sendMode,
	AESendPriority		sendPriority,
	long				timeOutInTicks,
	AEIdleUPP			idleProc,
	AEFilterUPP			filterProc )
	{
	AssertAEDescIsValid( theAppleEvent, "DebugTraps_AESend");
	
	OSErr	err;
	
	err	= AESend( theAppleEvent, reply, sendMode,
			sendPriority, timeOutInTicks, idleProc, filterProc );
	
	return( err );
	}


	pascal OSErr
DebugTraps_AEGetTheCurrentEvent(
	AppleEvent *	theAppleEvent,
	MDT_FileAndLineParams	)
	{
	OSErr	err;
	
	err	= AEGetTheCurrentEvent( theAppleEvent );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( theAppleEvent,
			kMacLeaks_AEGetTheCurrentEventAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}


	pascal OSErr
DebugTraps_AEResolve(
	const AEDesc *	objectSpecifier,
	short			callbackFlags,
	AEDesc *		theToken,
	MDT_FileAndLineParams	)
	{
	AssertAEDescIsValid( objectSpecifier, "DebugTraps_AEResolve");
	AssertAEDescIsValid( theToken, "DebugTraps_AEResolve");
	
	OSErr	err;
	
	err	= AEResolve( objectSpecifier, callbackFlags, theToken );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( theToken,
			kMacLeaks_AEResolveAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
	

	pascal OSErr
DebugTraps_CreateOffsetDescriptor(
	long		theOffset,
	AEDesc *	theDescriptor,
	MDT_FileAndLineParams	)
	{
	AssertAEDescIsValid( theDescriptor, "DebugTraps_CreateOffsetDescriptor");
	
	OSErr	err;
	
	err	= CreateOffsetDescriptor( theOffset, theDescriptor );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( theDescriptor, 
			kMacLeaks_CreateOffsetDescriptorAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
		
					
	pascal OSErr
DebugTraps_CreateCompDescriptor( DescType comparisonOperator,
	AEDesc *	operand1,
	AEDesc *	operand2,
	Boolean 	disposeInputs,
	AEDesc *	theDescriptor,
	MDT_FileAndLineParams	)
	{
	AssertAEDescIsValid( operand1, "DebugTraps_CreateCompDescriptor");
	AssertAEDescIsValid( operand2, "DebugTraps_CreateCompDescriptor");
	AssertAEDescIsValid( theDescriptor, "DebugTraps_CreateCompDescriptor");
	
	OSErr	err;
	
	err	= CreateCompDescriptor( comparisonOperator,
			operand1, operand2, disposeInputs, theDescriptor );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( theDescriptor,
			kMacLeaks_CreateCompDescriptorAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
		
					
	pascal OSErr
DebugTraps_CreateLogicalDescriptor(
	AEDescList *	theLogicalTerms,
	DescType		theLogicOperator,
	Boolean			disposeInputs,
	AEDesc *		theDescriptor,
	MDT_FileAndLineParams	)
	{
	AssertAEDescIsValid( theDescriptor, "DebugTraps_CreateLogicalDescriptor");
	
	OSErr	err;
	
	err	= CreateLogicalDescriptor( theLogicalTerms,
			theLogicOperator, disposeInputs, theDescriptor );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( theDescriptor,
			kMacLeaks_CreateLogicalDescriptorAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
		
					
	pascal OSErr
DebugTraps_CreateObjSpecifier( DescType desiredClass,
	AEDesc *	theContainer,
	DescType	keyForm,
	AEDesc *	keyData,
	Boolean		disposeInputs,
	AEDesc *	objSpecifier,
	MDT_FileAndLineParams	)
	{
	AssertAEDescIsValid( theContainer, "DebugTraps_CreateObjSpecifier");
	AssertAEDescIsValid( keyData, "DebugTraps_CreateObjSpecifier");
	AssertAEDescIsValid( objSpecifier, "DebugTraps_CreateObjSpecifier");
	
	OSErr	err;
	
	err	= CreateObjSpecifier( desiredClass,
			theContainer, keyForm, keyData, disposeInputs, objSpecifier );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( objSpecifier,
			kMacLeaks_CreateObjSpecifierAllocType, srcFile_, srcLine_);
		}
	
	if ( disposeInputs )
		{
		// forget the inputs MacLeaks_ForgetAEDesc(...)
		}
	
	return( err );
	}


	pascal OSErr
DebugTraps_CreateRangeDescriptor(
	AEDesc *	rangeStart,
	AEDesc *	rangeStop,
	Boolean		disposeInputs,
	AEDesc *	theDescriptor,
	MDT_FileAndLineParams	)
	{
	AssertAEDescIsValid( rangeStart, "DebugTraps_CreateRangeDescriptor");
	AssertAEDescIsValid( rangeStop, "DebugTraps_CreateRangeDescriptor");
	AssertAEDescIsValid( theDescriptor, "DebugTraps_CreateRangeDescriptor");
	
	OSErr	err;
	
	err	= CreateRangeDescriptor( rangeStart,
			rangeStop, disposeInputs, theDescriptor );
	
	if ( IsntErr( err ) )
		{
		MacLeaks_RememberAEDesc( theDescriptor,
			kMacLeaks_CreateRangeDescriptorAllocType, srcFile_, srcLine_);
		}
	
	return( err );
	}
	
	
	
// _____________________________ Window Mgr _________________________________

#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Window Mgr ---
#endif


	pascal WindowRef
DebugTraps_GetNewWindow(
	short			windowID,
	void *			wStorage,
	WindowRef		behind,
	MDT_FileAndLineParams	)
	{
	AssertAddrNullOrValid( wStorage, VoidAlign, "DebugTraps_GetNewWindow" );
	
	WindowRef	theRef;
	
	theRef	= GetNewWindow( windowID, wStorage, behind );
	pgpAssertMsg( IsntNull( theRef ), "DebugTraps_GetNewWindow: failure");
	
	if ( IsntNull( theRef ) )
		{
		if ( IsNull( wStorage ) )
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_GetNewWindowAllocType, 0, srcFile_, srcLine_);
		else
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_GetNewWindowStorageAllocType, 0, srcFile_, srcLine_);
		}
	
	return( theRef );
	}


	pascal WindowRef
DebugTraps_NewWindow(
	void *				wStorage,
	const Rect *		boundsRect,
	ConstStr255Param	title,
	Boolean				visible,
	short				theProc,
	WindowRef			behind,
	Boolean				goAwayFlag,
	long				refCon,
	MDT_FileAndLineParams	)
	{
	AssertAddrNullOrValid( wStorage, VoidAlign, "DebugTraps_NewWindow" );
	AssertAddrNullOrValid( title, uchar, "DebugTraps_NewWindow" );
	pgpAssertAddrValid( boundsRect, Rect );
	
	WindowRef	theRef;
	
	theRef	= NewWindow( wStorage, boundsRect,
			title,visible, theProc, behind, goAwayFlag, refCon );
	pgpAssertMsg( IsntNull( theRef ), "DebugTraps_NewWindow: failure");

	if ( IsntNull( theRef ) )
		{
		if ( IsNull( wStorage ) )
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_NewWindowAllocType, 0, srcFile_, srcLine_);
		else
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_NewWindowStorageAllocType, 0, srcFile_, srcLine_);
		}
	
	return( theRef );
	}


	

	pascal WindowRef
DebugTraps_GetNewCWindow(
	short		windowID,
	void *		wStorage,
	WindowRef	behind,
	MDT_FileAndLineParams	)
	{
	AssertAddrNullOrValid( wStorage, VoidAlign, "DebugTraps_GetNewCWindow" );
	
	WindowRef	theRef;
	
	theRef	= GetNewCWindow( windowID, wStorage, behind );
	pgpAssertMsg( IsntNull( theRef ), "DebugTraps_GetNewCWindow: failure");

	if ( IsntNull( theRef ) )
		{
		if ( IsNull( wStorage ) )
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_GetNewCWindowAllocType, 0, srcFile_, srcLine_);
		else
			MacLeaks_RememberPtr( theRef,
			kMacLeaks_GetNewCWindowStorageAllocType, 0, srcFile_, srcLine_);
		}
		
	return( theRef );
	}


	

	pascal WindowRef
DebugTraps_NewCWindow(
	void *				wStorage,
	const Rect *		boundsRect,
	ConstStr255Param	title,
	Boolean				visible,
	short				procID,
	WindowRef			behind,
	Boolean				goAwayFlag,
	long 				refCon,
	MDT_FileAndLineParams	)
	{
	AssertAddrNullOrValid( wStorage, VoidAlign, "DebugTraps_NewCWindow" );
	AssertAddrNullOrValid( title, VoidAlign, "DebugTraps_NewCWindow" );
	
	WindowRef	theRef;
	
	theRef	= NewCWindow( wStorage, boundsRect,
		title,visible, procID, behind, goAwayFlag, refCon );
	pgpAssertMsg( IsntNull( theRef ), "DebugTraps_NewCWindow: failure");
	
	if ( IsntNull( theRef ) )
		{
		if ( IsNull( wStorage ) )
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_NewCWindowAllocType, 0, srcFile_, srcLine_);
		else
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_NewCWindowStorageAllocType, 0, srcFile_, srcLine_);
		}
	
	return( theRef );
	}


	pascal void
DebugTraps_DisposeWindow(WindowRef theWindow)
	{
	pgpAssertAddrValidMsg( theWindow, WindowRecord, "DebugTraps_DisposeWindow");
	
	MacLeaks_ForgetPtr( theWindow, kMacLeaks_DisposeWindowDeallocType );
	
	DisposeWindow( theWindow );
	}
	
	
	pascal void
DebugTraps_CloseWindow(WindowRef theWindow)
	{
	pgpAssertAddrValidMsg( theWindow, WindowRecord, "DebugTraps_DisposeWindow");
	
	MacLeaks_ForgetPtr( theWindow, kMacLeaks_CloseWindowDeallocType );
	
	CloseWindow( theWindow );
	}


// _____________________________ Dialog Mgr _________________________________

#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#pragma mark --- Dialog Mgr ---
#endif


	pascal DialogRef
DebugTraps_NewDialog(
	void *				wStorage,
	const Rect *		boundsRect,
	ConstStr255Param	title,
	Boolean				visible,
	short				procID,
	WindowRef			behind,
	Boolean				goAwayFlag,
	long				refCon,
	Handle				itmLstHndl,
	MDT_FileAndLineParams	)
	{
	AssertAddrNullOrValid( wStorage, VoidAlign, "DebugTraps_NewDialog" );
	AssertAddrNullOrValid( title, uchar, "DebugTraps_NewDialog" );
	
	DialogRef	theRef;
	
	theRef	= NewDialog( wStorage, boundsRect, title,
				visible, procID, behind, goAwayFlag, refCon, itmLstHndl );
	pgpAssertMsg( IsntNull( theRef ), "DebugTraps_NewDialog: failure");
	
	if ( IsntNull( theRef ) )
		{
		if ( IsNull( wStorage )  )
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_NewDialogAllocType, 0, srcFile_, srcLine_);
		else
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_NewDialogStorageAllocType, 0, srcFile_, srcLine_);
		}
	
	return( theRef );
	}
	
	
						
	pascal DialogRef
DebugTraps_GetNewDialog(
	short 			dialogID,
	void *			dStorage,
	WindowRef		behind,
	MDT_FileAndLineParams	)
	{
	AssertAddrNullOrValid( dStorage, uchar, "DebugTraps_GetNewDialog" );
	
	DialogRef	theRef;
	
	theRef	= GetNewDialog( dialogID, dStorage, behind );
	pgpAssertMsg( IsntNull( theRef ), "DebugTraps_GetNewDialog: failure");
	
	if ( IsntNull( theRef ) )
		{
		if ( IsNull( dStorage )  )
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_GetNewDialogAllocType, 0, srcFile_, srcLine_);
		else
			MacLeaks_RememberPtr( theRef,
				kMacLeaks_GetNewDialogStorageAllocType, 0, srcFile_, srcLine_);
		}
	
	return( theRef );
	}
	
	


	pascal void
DebugTraps_ModalDialog(
	ModalFilterUPP	modalFilter,
	short *			itemHit)
	{
	AssertUPPIsNullOrValid( modalFilter, "DebugTraps_ModalDialog" );
	pgpAssertAddrValid( itemHit, short );
	
	ModalDialog( modalFilter, itemHit );
	}





	pascal void
DebugTraps_DisposeDialog( DialogRef theDialog)
	{
	pgpAssertAddrValidMsg( theDialog, DialogRecord,
		"DebugTraps_DisposeDialog");
	
	MacLeaks_ForgetPtr( theDialog, kMacLeaks_DisposeDialogDeallocType );
		
	DisposeWindow( theDialog );
	}
	
	
	pascal void
DebugTraps_CloseDialog( DialogRef theDialog)
	{
	pgpAssertAddrValidMsg( theDialog, DialogRecord, "DebugTraps_DisposeDialog");
	
	MacLeaks_ForgetPtr( theDialog, kMacLeaks_CloseDialogDeallocType );
		
	DisposeWindow( theDialog );
	}


	pascal short
DebugTraps_Alert(
	short			alertID,
	ModalFilterUPP	modalFilter)
	{
	AssertUPPIsNullOrValid( modalFilter, "DebugTraps_Alert" );
	pgpAssert( IsntNull( GetResource( 'ALRT', alertID ) ) );
	
	short itemHit	= Alert( alertID, modalFilter );
	return( itemHit );
	}



	pascal short
DebugTraps_StopAlert(
	short			alertID,
	ModalFilterUPP	modalFilter)
	{
	AssertUPPIsNullOrValid( modalFilter, "DebugTraps_StopAlert" );
	pgpAssert( IsntNull( GetResource( 'ALRT', alertID ) ) );
	
	short itemHit	= StopAlert( alertID, modalFilter );
	return( itemHit );
	}


	pascal short
DebugTraps_NoteAlert(
	short			alertID,
	ModalFilterUPP	modalFilter)
	{
	AssertUPPIsNullOrValid( modalFilter, "DebugTraps_NoteAlert" );
	pgpAssert( IsntNull( GetResource( 'ALRT', alertID ) ) );
	
	short itemHit	= NoteAlert( alertID, modalFilter );
	return( itemHit );
	}


	pascal short
DebugTraps_CautionAlert(
	short			alertID,
	ModalFilterUPP	modalFilter)
	{
	AssertUPPIsNullOrValid( modalFilter, "DebugTraps_CautionAlert" );
	pgpAssert( IsntNull( GetResource( 'ALRT', alertID ) ) );
	
	short itemHit	= CautionAlert( alertID, modalFilter );
	return( itemHit );
	}
	


// _____________________________ File Mgr _________________________________

	pascal OSErr
DebugTraps_FSpCreate(
	const FSSpec *	spec,
	OSType			creator,
	OSType			fileType,
	ScriptCode		scriptTag )
{
	OSErr	err;
	
	AssertSpecIsValid( spec, "DebugTraps_FSpCreate");
	
	err	= FSpCreate( spec, creator, fileType, scriptTag );
	return( err );
}





// _____________________________ Graphics _________________________________

	pascal void
DebugTraps_DisposeGDevice( GDHandle	device )
{
	AssertHandleIsValid( device, "DebugTraps_DisposeGDevice" );
	
	MacLeaks_ForgetHandle( device, kMacLeaks_DisposeHandleDeallocType );
	MacLeaks_ForgetHandle( (**device).gdITable,
		kMacLeaks_DisposeHandleDeallocType );

	
	DisposeGDevice( device );
}

	pascal void
DebugTraps_DisposeCTable(CTabHandle cTable)
{
	AssertHandleIsValid( cTable, "DebugTraps_DisposeCTable" );
	MacLeaks_ForgetHandle( cTable, kMacLeaks_DisposeHandleDeallocType );
	
	DisposeCTable( cTable );
}


#endif // ] USE_MAC_DEBUG_TRAPS





































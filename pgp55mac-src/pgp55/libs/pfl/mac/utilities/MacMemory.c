/*____________________________________________________________________________
	MacMemory.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacMemory.c,v 1.10 1997/09/18 01:34:50 lloyd Exp $
____________________________________________________________________________*/

#include "MacDebug.h"
#include "MacMemory.h"
#include "pgpMem.h"

#if MEMORY_ALLOC_MUST_BE_APP

	#if MEMORY_ALLOC_MUST_BE_SYS
	#error illegal combination of flags
	#endif
	
	#if MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	#error illegal combination of flags
	#endif
	
#elif MEMORY_ALLOC_MUST_BE_SYS
	#if MEMORY_ALLOC_MUST_BE_APP
		#error illegal combination of flags
	#endif
	
	#if MEMORY_ALLOC_MUST_BE_APP_OR_SYS
		#error illegal combination of flags
	#endif
		#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	#if MEMORY_ALLOC_MUST_BE_APP
		#error illegal combination of flags
	#endif
	
	#if MEMORY_ALLOC_MUST_BE_SYS
		#error illegal combination of flags
	#endif
#endif


#define MIN(a, b )		( (a) <= (b) ? (a) : (b) )


/*____________________________________________________________________________
	Allocate a Handle, based on desired flags.
	Flags have an implicit order as follows:
		- temp mem ( kMacMemory_PreferTempMem )
		- current heap
		- app heap
		- temp mem
		- system heap
____________________________________________________________________________*/
	Handle
pgpNewHandle(
	const size_t				requestedSize,
	const MacMemoryAllocFlags	flags )
	{
	Handle	theHandle	= nil;
	OSErr	err	= noErr;
	THz		saveZone	= GetZone();
	
	#define HaveFlag( flag )	( ((flags) & flag) == flag )
	
	if ( HaveFlag( kMacMemory_PreferTempMem ) )
		{
		theHandle	= TempNewHandle ( requestedSize, &err );
		}
	
	if ( IsNull( theHandle ) && HaveFlag( kMacMemory_UseCurrentHeap ))
		{
		theHandle	= NewHandle( requestedSize );
		}
	
	if ( IsNull( theHandle ) && HaveFlag( kMacMemory_UseApplicationHeap ))
		{
		SetZone( ApplicationZone() );
		theHandle	= NewHandle( requestedSize );
		SetZone( saveZone );
		}
	
	if ( IsNull( theHandle ) && HaveFlag( kMacMemory_UseTempMem ))
		{
		theHandle	= TempNewHandle( requestedSize, &err);
		}
	
	if ( IsNull( theHandle ) && HaveFlag( kMacMemory_UseSystemHeap ))
		{
		theHandle	= NewHandleSys( requestedSize );
		}
	
	if ( IsntNull( theHandle ) && HaveFlag( kMacMemory_ClearBytes ) )
		{
		pgpFillMemory( *theHandle, requestedSize, 0 );
		}
	
	return( theHandle );
	}
	


	void
pgpDisposeHandle( Handle	theHandle)
	{
	DisposeHandle( theHandle );
	}



/*____________________________________________________________________________
	Allocate a pointer, based on desired flags.
	Flags have an implicit order as
	follows:
		- temp mem ( kMacMemory_PreferTempMem )
		- current heap
		- app heap
		- temp mem
		- system heap
	
	Because the memory may be allocated via either NewPtr or TempNewHandle,
	we must prepend a header to it so we know how to dispose of it.
____________________________________________________________________________*/

#define kMacMemHeaderMagic	0x89ABCDEF
typedef struct MacMemHeader
	{
	UInt32	magic;
	Handle	tempHandle;
	} MacMemHeader;
#define UserPtrToMacMemHeader( u )		( ((MacMemHeader *)u) - 1 )
#define MacMemHeaderToUserPtr( m )		( ((MacMemHeader *)m) + 1 )

	void *
pgpAllocMac(
	const size_t				requestedSize,
	const MacMemoryAllocFlags	userFlags )
	{
	MacMemHeader *	memHeader		= nil;
	void *		result	= nil;
	OSErr		err	= noErr;
	THz			saveZone	= GetZone();
	size_t		totalSize	 = sizeof( MacMemHeader ) + requestedSize;
	Handle		tempHandle = nil;
	MacMemoryAllocFlags	flags = userFlags;

#if PGP_DEBUG
const MacMemoryAllocFlags	kAllFlags =
	kMacMemory_UseCurrentHeap | kMacMemory_UseApplicationHeap |
	kMacMemory_UseSystemHeap |
	kMacMemory_UseTempMem | kMacMemory_PreferTempMem | kMacMemory_ClearBytes;
	
	pgpAssertMsg( ( userFlags & ~kAllFlags ) == 0,
		"pgpAllocMac: unknown flag" );
	pgpAssert( ( requestedSize & 0x80000000) == 0 );
#endif
	
	#define HaveFlag( flag )	( ((flags) & flag) == flag )

#if MEMORY_ALLOC_MUST_BE_APP
	flags = flags & ~( kMacMemory_UseSystemHeap | kMacMemory_UseTempMem |
			kMacMemory_PreferTempMem );
	
	if( HaveFlag( kMacMemory_UseCurrentHeap ) )
	{
		pgpAssert( GetZone() == ApplicationZone() );
	}
	else if( ! HaveFlag( kMacMemory_UseApplicationHeap ) )
	{
		pgpDebugPStr( "\ppgpAllocMac: "
		"Unable to satisfy MEMORY_ALLOC_MUST_BE_APP" );
	}
#elif MEMORY_ALLOC_MUST_BE_SYS
	flags = flags & ~( kMacMemory_UseApplicationHeap |
			kMacMemory_UseTempMem | kMacMemory_PreferTempMem );
	
	if( HaveFlag( kMacMemory_UseCurrentHeap ) )
	{
		pgpAssert( GetZone() == SystemZone() );
	}
	else if( ! HaveFlag( kMacMemory_UseSystemHeap ) )
	{
		pgpDebugPStr( "\ppgpAllocMac: "
			"Unable to satisfy MEMORY_ALLOC_MUST_BE_SYS" );
	}
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	flags = flags & ~( kMacMemory_UseTempMem | kMacMemory_PreferTempMem );
	
	if( HaveFlag( kMacMemory_UseCurrentHeap ) )
	{
		pgpAssert( GetZone() == ApplicationZone() ||
			GetZone() == SystemZone() );
	}
	else if( ( ! HaveFlag( kMacMemory_UseSystemHeap ) ) &&
			( ! HaveFlag( kMacMemory_UseApplicationHeap ) ) )
	{
		pgpDebugPStr( "\ppgpAllocMac: "
		"Unable to satisfy MEMORY_ALLOC_MUST_BE_APP_OR_SYS" );
	}
#endif	
	
	if ( HaveFlag( kMacMemory_PreferTempMem ) )
		{
		/*
			allocate a Handle with a special header, lock it, and use the
			dereferenced Handle as our pointer
		*/
		tempHandle	= TempNewHandle( totalSize, &err);
		if ( IsntNull( tempHandle ) )
			{
			HLock( tempHandle );
			memHeader	= (MacMemHeader *)*tempHandle;
			}
		}
	
	if ( IsNull( memHeader ) && HaveFlag( kMacMemory_UseCurrentHeap ))
		{
		memHeader	= (MacMemHeader *)NewPtr( totalSize );
		}
	
	if ( IsNull( memHeader ) && HaveFlag( kMacMemory_UseApplicationHeap ))
		{
		SetZone( ApplicationZone() );
		memHeader	= (MacMemHeader *)NewPtr( totalSize );
		SetZone( saveZone );
		}
	
	if ( IsNull( memHeader ) &&
			HaveFlag( kMacMemory_UseTempMem ) &&
			! HaveFlag( kMacMemory_PreferTempMem ) )
		{
		/*
			allocate a Handle with a special header, lock it, and use the
			dereferenced Handle as our pointer
		*/
		tempHandle	= TempNewHandle( totalSize, &err);
		if ( IsntNull( tempHandle ) )
			{
			HLock( tempHandle );
			memHeader	= (MacMemHeader *)*tempHandle;
			}
		}
	
	if ( IsNull( memHeader ) && HaveFlag( kMacMemory_UseSystemHeap ))
		{
		memHeader	= (MacMemHeader *)NewPtrSys( totalSize );
		}
	
	if ( IsntNull( memHeader ) )
		{
		pgpClearMemory( memHeader, sizeof( *memHeader ) );
		memHeader->magic		= kMacMemHeaderMagic;
		memHeader->tempHandle	= tempHandle;
		
		/* point just beyond our header for user bytes */
		result	= MacMemHeaderToUserPtr( memHeader );
		
		if ( HaveFlag( kMacMemory_ClearBytes ) )
			{
			pgpClearMemory( result, requestedSize );
			}
		}
	
	return( result );
	}

	void
pgpFreeMac( void *	memory )
	{
	const MacMemHeader *memHeader	= UserPtrToMacMemHeader( memory );
	
	pgpAssertAddrValidMsg( memHeader, MacMemHeader, "pgpFreeMac" );
	pgpAssertMsg( memHeader->magic == kMacMemHeaderMagic, "pgpFreeMac" );
			
	if ( memHeader->magic == kMacMemHeaderMagic )
		{
		if ( IsntNull( memHeader->tempHandle ) )
			{
			AssertHandleIsValid( memHeader->tempHandle, "pgpFreeMac" );
			DisposeHandle( memHeader->tempHandle );
			}
		else
			{
			DisposePtr( (Ptr)memHeader );
			}
		}
	}


	UInt32
pgpMaxBlock( MacMemoryAllocFlags userFlags )
	{
	THz		saveZone	= GetZone();
	UInt32	tempMax		= 0;
	UInt32	maxSoFar	= 0;
	MacMemoryAllocFlags flags = userFlags;
	
#if MEMORY_ALLOC_MUST_BE_APP
	flags = flags & ~( kMacMemory_UseSystemHeap | kMacMemory_UseTempMem |
			kMacMemory_PreferTempMem );
	
	if( HaveFlag( kMacMemory_UseCurrentHeap ) )
	{
		pgpAssert( GetZone() == ApplicationZone() );
	}
	else if( ! HaveFlag( kMacMemory_UseApplicationHeap ) )
	{
		pgpDebugPStr( "\ppgpMaxBlock: "
		"Unable to satisfy MEMORY_ALLOC_MUST_BE_APP" );
	}
#elif MEMORY_ALLOC_MUST_BE_SYS
	flags = flags & ~( kMacMemory_UseApplicationHeap |
			kMacMemory_UseTempMem | kMacMemory_PreferTempMem );
	
	if( HaveFlag( kMacMemory_UseCurrentHeap ) )
	{
		pgpAssert( GetZone() == SystemZone() );
	}
	else if( ! HaveFlag( kMacMemory_UseSystemHeap ) )
	{
		pgpDebugPStr( "\ppgpMaxBlock: "
		"Unable to satisfy MEMORY_ALLOC_MUST_BE_SYS" );
	}
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	flags = flags & ~( kMacMemory_UseTempMem | kMacMemory_PreferTempMem );
	
	if( HaveFlag( kMacMemory_UseCurrentHeap ) )
	{
		pgpAssert( GetZone() == ApplicationZone() ||
				GetZone() == SystemZone() );
	}
	else if( ( ! HaveFlag( kMacMemory_UseSystemHeap ) ) &&
			( ! HaveFlag( kMacMemory_UseApplicationHeap ) ) )
	{
		pgpDebugPStr( "\ppgpMaxBlock: "
		"Unable to satisfy MEMORY_ALLOC_MUST_BE_APP_OR_SYS" );
	}
#endif	

	if ( HaveFlag( kMacMemory_PreferTempMem ) ||
			HaveFlag( kMacMemory_UseTempMem ) )
		{
		long	dummyGrow;
		
		tempMax	= TempMaxMem( &dummyGrow );
		if ( tempMax > maxSoFar )
			maxSoFar	= tempMax;
		}
	
	if ( HaveFlag( kMacMemory_UseCurrentHeap ))
		{
		tempMax	= MaxBlock();
		if ( tempMax > maxSoFar )
			maxSoFar	= tempMax;
		}
	
	if ( HaveFlag( kMacMemory_UseApplicationHeap ))
		{
		SetZone( ApplicationZone() );
		tempMax	= MaxBlock();
		SetZone( saveZone );
		
		if ( tempMax > maxSoFar )
			maxSoFar	= tempMax;
		}
	
	if ( HaveFlag( kMacMemory_UseSystemHeap ))
		{
		tempMax	= MaxBlockSys();
		if ( tempMax > maxSoFar )
			maxSoFar	= tempMax;
		}
	
	return( maxSoFar );
	}


	void *
pgpNewPtrMost(
	const UInt32			desiredSize,
	const UInt32			minSize,
	MacMemoryAllocFlags	allocFlags,
	UInt32 *				actualSizePtr )
	{
	void *	memory	= nil;
	
	pgpAssert( desiredSize != 0 && minSize != 0 );
	
	if( desiredSize != 0 )
		{
		*actualSizePtr	= desiredSize;
			
		while ( IsNull( memory ) )
			{
			memory	= pgpAllocMac( desiredSize, allocFlags );
			if ( IsntNull( memory ) )
				break;
			
			/* reduce size by 1/8 */
			*actualSizePtr	-= *actualSizePtr / 8;
			if ( *actualSizePtr < minSize )
				break;
			}
		}
	
	return( memory );
	}






#if 0
	static void
TestFillMemory()
	{
	void *mem;
	
	const UInt32 kTestSize			= 20;
	const UInt32 kTestIterations	= 1 *1024UL * 1024UL;
	mem	= pgpAllocMac( kTestSize, kMacMemory_UseTempMem );
	if ( IsntNull( mem ) )
		{
		UInt32	startTicks	= TickCount();
		
		UInt32	iter	= kTestIterations;
		while ( iter-- != 0 )
			{
			pgpClearMemory( mem, kTestSize );
			pgpClearMemory( mem, kTestSize );
			pgpClearMemory( mem, kTestSize );
			pgpClearMemory( mem, kTestSize );
			}
		
		UInt32	elapsed	= TickCount() - startTicks;
		
		Str255	tempStr;
		NumToString( elapsed, tempStr );
		DebugStr( tempStr );
		}
	}
	
	pgpFreeMac( mem );
#endif

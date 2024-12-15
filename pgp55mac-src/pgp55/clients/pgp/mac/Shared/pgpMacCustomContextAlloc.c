/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpMacCustomContextAlloc.c,v 1.4 1997/09/14 03:16:26 lloyd Exp $
____________________________________________________________________________*/

#include "pgpMacCustomContextAlloc.h"
#include "pgpMem.h"



static void *	pgpMacCustomAllocator( PGPContextRef context,
					PGPSize allocationSize, PGPMemoryFlags flags,
					PGPUserValue userValue );

static PGPError	pgpMacCustomRealloc( PGPContextRef context,
					void **allocation, PGPSize newAllocationSize,
					PGPMemoryFlags flags, PGPUserValue userValue );

static PGPError	pgpMacCustomDealloc( PGPContextRef context,
					void *allocation, PGPUserValue userValue );
				
				
typedef struct BlockHeader
{
	/* we should keep this struct a multiple of 16 bytes.  For now,
	we use the extra space for extra magic fields */
	#define kMagic		'MACM'
	PGPUInt32		magic;
	// if temp-mem, then this is non-null
	Handle			tempMemHandle;
	PGPSize			userSize;
	PGPUInt32		magic2;
} BlockHeader;
#define BlockHeaderToData( hdr )	((void *)( ((BlockHeader *)hdr) + 1 ))
#define DataToBlockHeader( data )	\
	((BlockHeader *)(((char *)data) - sizeof( BlockHeader )))

#define IsTempMemBlock( hdr )		( IsntNull( (hdr)->tempMemHandle ) )

	static PGPBoolean
sHeaderIsValid( BlockHeader const *hdr )
{
	if ( IsNull( hdr ) )
		return( FALSE );
	return( hdr->magic == kMagic && hdr->magic2 == kMagic );
}

#define BlockIsValid( hdr )		( IsntNull( hdr ) && sHeaderIsValid( hdr ) )

/* anything larger than this will be allocated in temp mem */
#define kTempMemCutoff		( 4UL * 1024UL )


	static BlockHeader *
AllocateBlock( PGPSize requestSize )
{
	PGPSize			totalSize	= requestSize + sizeof( BlockHeader );
	BlockHeader *	header	= NULL;
	
	/* try getting a plain Ptr first, if that's what's desired */
	if ( requestSize < kTempMemCutoff )
	{
		header	= (BlockHeader *)NewPtr( totalSize );
		if ( IsntNull( header ) )
			header->tempMemHandle	= NULL;
	}
	
	if ( IsNull( header )  )
	{
		Handle	tempHandle;
		OSErr	err;

		tempHandle	= TempNewHandle( totalSize, &err );
		if ( IsntNull( tempHandle ) && IsntErr( err ) )
		{
			HLock( tempHandle );
			header	= (BlockHeader *)*tempHandle;
			header->tempMemHandle	= tempHandle;
		}
		
	}
	
	if ( IsntNull( header ) )
	{
		header->magic		= kMagic;
		header->magic2		= kMagic;
		header->userSize	= requestSize;
	}
	
	return( header );
}

	static void *
pgpMacCustomAllocator(
	PGPContextRef	context,
	PGPSize			allocationSize,
	PGPMemoryFlags	flags,
	PGPUserValue	userValue )
{
	BlockHeader *	header	= NULL;
	void *			mem	= NULL;
	
	(void)context;
	(void)flags;
	(void)userValue;
	
	header	= AllocateBlock( allocationSize );
	
	if ( IsntNull( header ) )
		mem	= BlockHeaderToData( header );
	else
		mem	= NULL;
	return( mem );
}



	static PGPError
pgpMacCustomDealloc(
	PGPContextRef	context,
	void *			allocation,
	PGPUserValue	userValue )
{
	PGPError		err	= kPGPError_NoErr;
	BlockHeader *	header	= NULL;
	
	(void)context;
	(void)userValue;
	if ( IsNull( allocation ) )
	{
		pgpDebugMsg( "pgpMacCustomDealloc: NULL block" );
		return( kPGPError_BadParams );
	}
	
	header	= DataToBlockHeader( allocation );
	if ( ! BlockIsValid( header ) )
	{
		pgpDebugMsg( "pgpMacCustomDealloc: bad block" );
		return( kPGPError_BadParams );
	}
	
	if ( IsTempMemBlock( header ) )
	{
		Handle	theHandle;
		
		theHandle	= header->tempMemHandle;
		pgpAssert( (Ptr)header == (Ptr)*theHandle );
		
		MacDebug_FillWithGarbage( *theHandle ,
			GetHandleSize( theHandle ) );
			
		DisposeHandle( theHandle );
	}
	else
	{
		MacDebug_FillWithGarbage( header,
			GetPtrSize( (Ptr)header ) );
		
		DisposePtr( (Ptr)header );
	}

	
	return( err );
}



	static PGPError
pgpMacCustomRealloc( 
	PGPContextRef	context,
	void **			allocationPtr,
	PGPSize			newAllocationSize,
	PGPMemoryFlags	flags,
	PGPUserValue	userValue )
{
	PGPError		err	= kPGPError_OutOfMemory;
	BlockHeader *	header	= NULL;
	PGPSize			totalSize;
	
	PGPValidatePtr( allocationPtr );
	header		= DataToBlockHeader( *allocationPtr );
	PGPValidateParam( BlockIsValid( header ) );
	if ( newAllocationSize == header->userSize )
		return( kPGPError_NoErr );
	
	totalSize	= newAllocationSize + sizeof(BlockHeader);
	
	/* CAUTION: if we fail, original must *not* move original */
	
	/* first, try resizing existing block in place */
	/* for temp mem, don't attempt to grow a locked block;
	  it may or may not be legal */
	err	= kPGPError_OutOfMemory;
	if ( IsTempMemBlock( header ))
	{
		if ( newAllocationSize <= header->userSize )
		{
			/* it's locked, but resizing may succeed */
			SetHandleSize( header->tempMemHandle, totalSize );
			if ( GetHandleSize( header->tempMemHandle ) == totalSize )
			{
				err	= kPGPError_NoErr;
			}
		}
	}
	else
	{
		SetPtrSize( (Ptr)header, totalSize );
		if ( GetPtrSize( (Ptr)header ) == totalSize )
		{
			err	= kPGPError_NoErr;
		}
	}
	
	if ( err == kPGPError_OutOfMemory )
	{
		/* resizing existing block in place failed */
		/* try allocating an all-new block */
		PGPSize		oldSize		= header->userSize;
		void *		newMem;
		
		newMem	= pgpMacCustomAllocator( context, newAllocationSize,
						flags, userValue );
		if ( IsntNull( newMem ) )
		{
			err	= kPGPError_NoErr;
			
			pgpCopyMemory( *allocationPtr, newMem,
				pgpMin( newAllocationSize, oldSize ) );
				
			pgpMacCustomDealloc( context, *allocationPtr, userValue );
			*allocationPtr	= newMem;
		}
	}
	
	if ( IsntPGPError( err ) )
	{
		header	= DataToBlockHeader( *allocationPtr );
		header->userSize	= newAllocationSize;
	}
	
	return( err );
}





	PGPError
pgpNewContextCustomMacAllocators(
	PGPContextRef *	context )
{
	PGPError				err	= kPGPError_NoErr;
	PGPNewContextStruct		custom;
	
	pgpClearMemory( &custom, sizeof( custom ) );
	custom.sizeofStruct		= sizeof( custom );
	custom.allocProc		= pgpMacCustomAllocator;
	custom.reallocProc		= pgpMacCustomRealloc;
	custom.deallocProc		= pgpMacCustomDealloc;
	custom.allocUserValue	= 0;
	
	err	= PGPNewContextCustom( kPGPsdkAPIVersion, &custom, context );
	
	return( err );
}

				




























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

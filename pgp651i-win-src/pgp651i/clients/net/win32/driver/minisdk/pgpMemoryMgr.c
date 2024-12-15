/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpMemoryMgr.c,v 1.4 1999/05/11 17:38:11 dgal Exp $
____________________________________________________________________________*/
#include "pgpPFLErrors.h"
#include "pgpMem.h"
#include "pgpRMWOLock.h"

#include "pgpMemoryMgrPriv.h"


/* if defined as 1, causes leaks to be tracked */
#define MEMORY_MGR_TRACK_LEAKS			PGP_DEBUG


#define TESTING		PGP_DEBUG

#if TESTING
static void		sTestMemoryMgr( PGPMemoryMgrRef mgr );
#else
#define sTestMemoryMgr( mgr )	/* nothing */
#endif

/*____________________________________________________________________________
	This structure is prepended to every allocated block.
	
	It is dynamically padded to keep it aligned to the alignment
	requirement.  See kRealHeaderSize.
____________________________________________________________________________*/
typedef struct DataHeader
{
#define kHeaderMagic	((PGPUInt16)0xABCD)
	PGPMemoryMgrRef	mgr;
	
	PGPUInt32		secureAlloc		: 1;
	PGPUInt32		nonPageable		: 1;
	PGPUInt32		requestSize	: 30;
	
#if MEMORY_MGR_TRACK_LEAKS
	struct DataHeader *	next;
	struct DataHeader *	prev;
#endif
	
	PGPUInt32		magic;
	#define kDataHeaderMagic		0x42554746  /*'BUGF'*/
} DataHeader;

#define kHeaderAlign		16
#define kOddHeaderBytes		( sizeof( DataHeader ) % kHeaderAlign )
#define kNeededHeaderBytes	( (kHeaderAlign - kOddHeaderBytes) % kHeaderAlign)
#define kRealHeaderSize		( sizeof( DataHeader ) + kNeededHeaderBytes )

#define HeaderToUserBlock( hdr )	\
	( (void *)( ((char *)(hdr)) + kRealHeaderSize) )
#define UserBlockToHeader( user )	\
	( (DataHeader *)(((char *)user) - kRealHeaderSize) )


	static PGPBoolean
sDataHeaderIsValid( const DataHeader *header )
{
	return( IsntNull( header ) &&
			PGPMemoryMgrIsValid( header->mgr ) &&
			header->magic == kDataHeaderMagic );
}

#if PGP_DEBUG
#define AssertBlockValid( block) \
	pgpAssert( IsntNull( block ) && \
	sDataHeaderIsValid( (const DataHeader *)\
		UserBlockToHeader( block ) ) )
#else
#define AssertBlockValid( block ) /* nothing */
#endif



struct PGPMemoryMgr
{
#define kPGPMemoryMgrMagic	0x4D4D6772	/* 'MMgr' */
	PGPUInt32		magic;				/* Always kPGPMemoryMgrMagic */
	PGPUserValue	customValue;		/* for custom memory mgrs */
	PGPBoolean		isDefault;
	
	PGPMemoryMgrAllocationProc		allocProc;
	PGPMemoryMgrReallocationProc	reallocProc;
	PGPMemoryMgrDeallocationProc	deallocProc;
	
	PGPMemoryMgrSecureAllocationProc	secureAllocProc;	/* may be NULL */
	PGPMemoryMgrSecureDeallocationProc	secureDeallocProc;	/* may be NULL */

#if MEMORY_MGR_TRACK_LEAKS
	PGPRMWOLock			leaksRMWOLock;
	DataHeader *		leaksList;
	DataHeader			dummyItem;
	PGPSize				numAllocations;
#endif
} ;



	PGPBoolean
PGPMemoryMgrIsValid( PGPMemoryMgrRef mgr )
{
	return( IsntNull( mgr ) &&
			mgr->magic == kPGPMemoryMgrMagic );
}



#if MEMORY_MGR_TRACK_LEAKS	/* [ */


	static void
sCheckListIntegrity( DataHeader const * list )
{
	DataHeader *		cur		= list->next;
	const DataHeader *	prev	= list;
	
	/* verify that item isn't already in list */
	while ( cur != list )
	{
		pgpAssert( cur->magic == kDataHeaderMagic );
		pgpAssert( cur->prev->magic == kDataHeaderMagic );
		pgpAssert( cur->next->magic == kDataHeaderMagic );
		pgpAssert( cur->next->prev == cur && cur->prev->next == cur);
		prev = cur;
		cur	= cur->next;
	}
}

	static void
sAddToLeaksList(
	PGPMemoryMgrRef	mgr,
	DataHeader *	item )
{
	DataHeader *	list;
	
	PGPRMWOLockStartWriting(&mgr->leaksRMWOLock);

	list = mgr->leaksList;

	sCheckListIntegrity( list );
	
	pgpAssert( IsntNull( list->next ) && IsntNull( list->prev ) );
	
#if PGP_DEBUG
	{
		DataHeader *	cur	= list->next;
		
		/* verify that item isn't already in list */
		while ( cur != list )
		{
			pgpAssert( cur != item );
			cur	= cur->next;
		}
	}
#endif

	/* insert new item into circular linked list */
	item->next			= list;
	item->prev			= list->prev;
	list->prev->next	= item;
	list->prev			= item;
	
	mgr->numAllocations	+= 1;
	
	sCheckListIntegrity( list );
	
	pgpAssert( IsntNull( item->next ) && IsntNull( item->prev ) );

	PGPRMWOLockStopWriting(&mgr->leaksRMWOLock);

}

	static void
sRemoveFromLeaksList(
	PGPMemoryMgrRef		mgr,
	DataHeader *		item )
{
	DataHeader *	next;
	DataHeader *	prev;
	
	PGPRMWOLockStartWriting(&mgr->leaksRMWOLock);

	next	= item->next;
	prev	= item->prev;

	sCheckListIntegrity( mgr->leaksList );
	
	pgpAssert( prev->next == item && next->prev == item );
	
	next->prev	= prev;
	prev->next	= next;
	
	item->next	= NULL;
	item->prev	= NULL;
	
	mgr->numAllocations	-= 1;
	
	sCheckListIntegrity( mgr->leaksList );

	PGPRMWOLockStopWriting(&mgr->leaksRMWOLock);
}

#else
#define sAddToLeaksList( mgr, item )		/* nothing */
#define sRemoveFromLeaksList( mgr, item )	/* nothing */

#endif	/* ] */

	static void
sInitDataHeader(
	PGPMemoryMgrRef	mgr,
	PGPSize			requestSize,
	DataHeader *	header )
{
	pgpClearMemory( header, sizeof(*header) );
	header->mgr				= mgr;
	header->secureAlloc		= FALSE;
	header->nonPageable		= FALSE;
	header->requestSize	= requestSize;
	
	header->magic	= kDataHeaderMagic;
	
#if MEMORY_MGR_TRACK_LEAKS
	header->next			= NULL;
	header->prev			= NULL;
	
#endif
}




/*____________________________________________________________________________
	Reallocate a secure block.
	
	To preserve the semantics of the block not changing if it shrinks,
	we take no action if the block is shrinking.  If it's growing, we
	always discard it and create a new one.
____________________________________________________________________________*/
	static PGPError
sSecureMemoryReallocProc(
	PGPMemoryMgrRef		mgr,
	void **				allocation,
	PGPSize 			newAllocationSize,
	PGPMemoryMgrFlags	flags,
	PGPSize				existingSize )
{
	PGPError		err		= kPGPError_NoErr;
	
	pgpAssert( IsntNull( *allocation ) );
	
#if 0
	if ( newAllocationSize <= existingSize )
	{
		PGPUInt32	excessByteCount;
		char *		dataToClear	= NULL;
		
		/* leave existing size intact, but clear excess bytes now
		to avoid leaving sensitive data in memory unnecessarily */
		excessByteCount	= existingSize - newAllocationSize;
		dataToClear	= ((char *)*allocation) + newAllocationSize;
		pgpClearMemory( dataToClear, excessByteCount );
		
		err	= kPGPError_NoErr;
	}
	else
#endif
	{	/* block is increasing in size */
		PGPBoolean	isNonPageable;
		void *		newBlock	= NULL;
		
		newBlock	= (mgr->secureAllocProc)( mgr, mgr->customValue,
			newAllocationSize, flags, &isNonPageable);
		if ( IsntNull( newBlock ) )
		{
			DataHeader *	oldHeader	= (DataHeader *)*allocation;

			pgpCopyMemory( *allocation, newBlock,
				pgpMin( existingSize, newAllocationSize ) );
			
			(mgr->secureDeallocProc)( mgr, mgr->customValue,
				*allocation, existingSize,
				(PGPBoolean)oldHeader->nonPageable );
			*allocation	= newBlock;
		}
		else
		{
			err	= kPGPError_OutOfMemory;
		}
	}
	
	return( err );
}




/*____________________________________________________________________________
	Allocate a new Memory Mgr using custom memory allocators.
____________________________________________________________________________*/
	PGPError
PGPNewMemoryMgrCustom(
	PGPNewMemoryMgrStruct const *	custom,
	PGPMemoryMgrRef *				newMgr )
{
	PGPError			err = kPGPError_NoErr;
	PGPMemoryMgrRef		mgr	= NULL;
	PGPUInt32			idx;
	
	PGPValidatePtr( newMgr );
	*newMgr	= NULL;
	PGPValidatePtr( custom );
	PGPValidatePtr( custom->allocProc );
	PGPValidatePtr( custom->reallocProc );
	PGPValidatePtr( custom->deallocProc );
	PGPValidatePtr( custom->secureAllocProc );
	PGPValidatePtr( custom->secureDeallocProc );
	PGPValidateParam( custom->sizeofStruct >= sizeof( *custom ) );
	PGPValidateParam( custom->reserved == NULL );
	
	#define kNumPads	sizeof( custom->pad ) / sizeof( custom->pad[ 0 ] )
	for( idx = 0; idx < kNumPads; ++idx )
	{
		PGPValidateParam( custom->pad[ idx ] == NULL );
	}
	
	/* NOTE: do not depend on kPGPMemoryMgrFlags_Clear here;
	 allocator is not required to support it */
	mgr = (PGPMemoryMgr *) (*(custom->allocProc))( NULL, custom->customValue,
				sizeof( *mgr ), 0 );
	if( IsntNull( mgr ) )
	{
		pgpClearMemory( mgr, sizeof( *mgr ) );
		mgr->magic				= kPGPMemoryMgrMagic;
		mgr->allocProc			= custom->allocProc;
		mgr->reallocProc		= custom->reallocProc;
		mgr->deallocProc		= custom->deallocProc;
		mgr->secureAllocProc	= custom->secureAllocProc;
		mgr->secureDeallocProc	= custom->secureDeallocProc;

		mgr->customValue		= custom->customValue;
		mgr->isDefault			= FALSE;
		
#if MEMORY_MGR_TRACK_LEAKS
		/* Create read many/write once mutex. */
		InitializePGPRMWOLock(&mgr->leaksRMWOLock);

		/* set up doubly-linked leaks list */
		mgr->leaksList			= &mgr->dummyItem;
		mgr->dummyItem.next		= &mgr->dummyItem;
		mgr->dummyItem.prev		= &mgr->dummyItem;
		mgr->dummyItem.magic	= kDataHeaderMagic;
		mgr->numAllocations		= 0;
#endif
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}
	
	*newMgr	= mgr;
	
#if TESTING
	if ( IsntPGPError( err ) )
	{
		sTestMemoryMgr( mgr );
	}
#endif
	
	return( err );
}


/*____________________________________________________________________________
____________________________________________________________________________*/

	PGPError
PGPNewMemoryMgr(
	PGPFlags			reserved,
	PGPMemoryMgrRef *	newMgr )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( newMgr );
	*newMgr	= NULL;
	PGPValidateParam( reserved == 0 );
	
	err	= pgpCreateStandardMemoryMgr( newMgr );
	if ( IsntPGPError( err ) )
	{
		(*newMgr)->isDefault	= TRUE;
	}
	
	return( err );
}



/*____________________________________________________________________________
	Delete an existing PFLContext and all resources associated with it.
____________________________________________________________________________*/
	PGPError
PGPFreeMemoryMgr(PGPMemoryMgrRef	mgr)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateParam( PGPMemoryMgrIsValid( mgr ) );

	if( PGPMemoryMgrIsValid( mgr ) )
	{
		PGPUserValue	customValue	= mgr->customValue;
		PGPBoolean		isDefault	= mgr->isDefault;
		
#if MEMORY_MGR_TRACK_LEAKS
		DeletePGPRMWOLock(&mgr->leaksRMWOLock);

		pgpAssertMsg( mgr->numAllocations == 0,
		"PGPFreeData(): mgr being freed with outstanding allocations" );
#endif
		
		(*mgr->deallocProc)( NULL, customValue, mgr, sizeof( *mgr ));
		
		if ( isDefault )
		{
			pgpDisposeStandardMemoryMgrUserValue( customValue );
		}
		
	}
	else
	{
		err = kPGPError_BadParams;
	}
	
	return( err );
}


	PGPError
PGPGetMemoryMgrCustomValue(
	PGPMemoryMgrRef 	mgr,
	PGPUserValue *		customValue)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidatePtr( customValue );
	*customValue	= NULL;
	PGPValidateMemoryMgr( mgr );
	/* illegal to change user value of default allocator */
	PGPValidateParam( ! mgr->isDefault );
	
	if( PGPMemoryMgrIsValid( mgr ) &&
		IsntNull( customValue ) )
	{
		*customValue = mgr->customValue;
	}
	else
	{
		err = kPGPError_BadParams;
	}
	
	return( err );
}

	PGPError
PGPSetMemoryMgrCustomValue(
	PGPMemoryMgrRef 	mgr,
	PGPUserValue 		customValue)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateMemoryMgr( mgr );
	/* illegal to change user value of default allocator */
	PGPValidateParam( ! mgr->isDefault );
	
	if( PGPMemoryMgrIsValid( mgr ) )
	{
		mgr->customValue = customValue;
	}
	else
	{
		err = kPGPError_BadParams;
	}
	
	return( err );
}





/*____________________________________________________________________________
	Allocate a block of memory using the allocator stored in a PFLContext.
____________________________________________________________________________*/

	void *
PGPNewData(
	PGPMemoryMgrRef 	mgr,
	PGPSize 			requestSize,
	PGPMemoryMgrFlags	flags)
{
	void	*allocation		= NULL;
	PGPSize	allocationSize	= requestSize;
	
	allocationSize	+= kRealHeaderSize;
	
	pgpAssert( PGPMemoryMgrIsValid( mgr ) );
	
	if( PGPMemoryMgrIsValid( mgr ) )
	{
		allocation = (*mgr->allocProc)( mgr, mgr->customValue,
						allocationSize, flags );
	}
	
	if ( IsntNull( allocation ) )
	{
		DataHeader *	header	= NULL;
	
		if ( (flags & kPGPMemoryMgrFlags_Clear ) != 0 )
		{
			pgpClearMemory( allocation, allocationSize );
		}
		else
		{
			pgpDebugWhackMemory( allocation, allocationSize );
		}
		
		header	= (DataHeader *)allocation;
		sInitDataHeader( mgr, requestSize, header );
		
		allocation		= HeaderToUserBlock( allocation );
		AssertBlockValid( allocation );
		
		sAddToLeaksList( mgr, header );
	}

	return( allocation );
}




/*____________________________________________________________________________
	Allocate a block of memory (secure or non-secure).
____________________________________________________________________________*/
	PGPError
PGPReallocData(
	PGPMemoryMgrRef 	mgr,
	void **				userPtr,
	PGPSize 			requestSize,
	PGPMemoryMgrFlags	flags)
{
	PGPError		err			= kPGPError_NoErr;
	
	PGPValidateParam( PGPMemoryMgrIsValid( mgr ) );
	PGPValidatePtr( userPtr );

	/* allow a NULL input  */
	if ( IsntNull( *userPtr ) )
	{
		DataHeader *	header	= UserBlockToHeader( *userPtr );
		PGPSize			oldRequestSize	= header->requestSize;
		
		sRemoveFromLeaksList( mgr, header );
			
		if ( header->secureAlloc )
		{
			AssertBlockValid( *userPtr );
			err = sSecureMemoryReallocProc( mgr,
						(void **) &header,
						kRealHeaderSize + requestSize,
						flags,
						kRealHeaderSize + header->requestSize );
			
		}
		else
		{
			AssertBlockValid( *userPtr );
			err = (*(mgr->reallocProc))( mgr, mgr->customValue,
						(void **) &header,
						kRealHeaderSize + requestSize,
						flags,
						kRealHeaderSize + header->requestSize );
		}
		
		sAddToLeaksList( mgr, header );
		
		if ( IsntPGPError( err ) )
		{
			*userPtr	= HeaderToUserBlock( header );
			header->requestSize	= requestSize;
			AssertBlockValid( *userPtr );
		}
		
		
		if ( IsntPGPError( err ) && requestSize > oldRequestSize )
		{
			if ( (flags & kPGPMemoryMgrFlags_Clear ) != 0 )
			{
				pgpClearMemory( ((char *)(*userPtr)) + oldRequestSize,
					requestSize - oldRequestSize );
			}
			else
			{
	#if PGP_DEBUG
				/* clear all newly allocated data to garbage */
				pgpDebugWhackMemory( ((char *)(*userPtr)) + oldRequestSize,
					requestSize - oldRequestSize );
	#endif
			}
		}
	}
	else
	{
		/* allocate a brand-new block */
		*userPtr	= PGPNewData( mgr, requestSize, flags );
		if ( IsNull( *userPtr ) )
		{
			err	= kPGPError_OutOfMemory;
		}
	}
	
	return( err );
}


/*____________________________________________________________________________
	Free a block of memory, whether secure or not.
____________________________________________________________________________*/
	PGPError
PGPFreeData( void * allocation )
{
	PGPError		err	= kPGPError_NoErr;
	PGPMemoryMgrRef	mgr	= NULL;
	DataHeader *	header;
	PGPSize			allocationSize;
	
	PGPValidatePtr( allocation );
	header	= UserBlockToHeader( allocation );
	PGPValidateParam( sDataHeaderIsValid( header ) );
	
	mgr				= header->mgr;
	allocationSize	= kRealHeaderSize + header->requestSize;
	
	sRemoveFromLeaksList( mgr, header );
	
	if ( header->secureAlloc )
	{
		pgpClearMemory( allocation, header->requestSize );
		
		err	= (*(mgr->secureDeallocProc))(
					mgr, mgr->customValue,
					header,
					allocationSize,
					(PGPBoolean)header->nonPageable );
	}
	else
	{
		pgpDebugWhackMemory( header, allocationSize );
		
		err	= (*(mgr->deallocProc))(
					mgr, mgr->customValue,
					header,
					allocationSize );
	}
	
	return( err );
}



	void *
PGPNewSecureData(
	PGPMemoryMgrRef 	mgr,
	PGPSize 			requestSize,
	PGPMemoryMgrFlags	flags )
{
	void *		allocation		= NULL;
	PGPSize		allocationSize	= requestSize;
	PGPBoolean	isNonPageable;
	
	allocationSize	+= kRealHeaderSize;
	
	if( PGPMemoryMgrIsValid( mgr ) )
	{
		allocation = (*mgr->secureAllocProc)( mgr, mgr->customValue,
						allocationSize, flags, &isNonPageable );
	}
	
	if ( IsntNull( allocation ) )
	{
		DataHeader *	header	= NULL;
	
		if ( (flags & kPGPMemoryMgrFlags_Clear ) != 0)
		{
			pgpClearMemory( allocation, allocationSize );
		}
		else
		{
			pgpDebugWhackMemory( allocation, allocationSize );
		}
		
		header	= (DataHeader *)allocation;
		sInitDataHeader( mgr, requestSize, header );
		header->secureAlloc	= TRUE;
		header->nonPageable	= isNonPageable;
		
		allocation		= HeaderToUserBlock( allocation );
		AssertBlockValid( allocation );
		
		sAddToLeaksList( mgr, header );
	}

	return( allocation );
}


	PGPFlags
PGPGetMemoryMgrDataInfo( void *allocation )
{
	PGPFlags		flags	= 0;
	DataHeader *	header;
	
	if ( IsNull( allocation ) )
		return( 0 );
		
	header	= UserBlockToHeader( allocation );
	if ( sDataHeaderIsValid( header ) )
	{
		flags	|= kPGPMemoryMgrBlockInfo_Valid;
		
		if ( header->secureAlloc )
			flags	|= kPGPMemoryMgrBlockInfo_Secure;
			
		if ( header->nonPageable )
			flags	|= kPGPMemoryMgrBlockInfo_NonPageable;
	}
	return( flags );
}

typedef struct MemoryMgrInfo
{
	struct MemoryMgrInfo	*next;
	PGPMemoryMgrRef			mgr;

} MemoryMgrInfo;


struct PGPDefaultMemoryMgr
{
#define kPGPDefaultMemoryMgrMagic	0x444D6772	/* 'DMgr' */

	PGPUInt32		magic;				/* Always kPGPDefaultMemoryMgrMagic */
	PGPMemoryMgrRef	defaultMemoryMgr;
	MemoryMgrInfo	*memoryMgrList;
	PGPRMWOLock		memoryMgrListLock;
	PGPMemoryMgrRef internalMemoryMgr;
};


PGPError
PGPNewDefaultMemoryMgr(PGPDefaultMemoryMgrRef *defaultRef)
{
	PGPMemoryMgrRef mgr = NULL;
	PGPDefaultMemoryMgrRef newDefault = NULL;
	PGPError err = kPGPError_NoErr;

	if (IsNull(defaultRef))
		return kPGPError_BadParams;

	*defaultRef = NULL;

	err = pgpCreateStandardMemoryMgr(&mgr);
	if (IsntPGPError(err))
	{
		newDefault = (PGPDefaultMemoryMgrRef) 
			PGPNewData(mgr, sizeof(PGPDefaultMemoryMgr), 
				kPGPMemoryMgrFlags_Clear);

		if (IsNull(newDefault))
		{
			err = kPGPError_OutOfMemory;
			PGPFreeMemoryMgr(mgr);
		}
	}

	if (IsntPGPError(err))
	{
		newDefault->magic				= kPGPDefaultMemoryMgrMagic;
		newDefault->defaultMemoryMgr	= kInvalidPGPMemoryMgrRef;
		newDefault->memoryMgrList		= NULL;
		newDefault->internalMemoryMgr	= mgr;
	}

	*defaultRef = newDefault;
	return err;
}


static PGPBoolean
sDefaultMemoryMgrIsValid( PGPDefaultMemoryMgrRef mgr )
{
	return( IsntNull( mgr ) &&
			mgr->magic == kPGPDefaultMemoryMgrMagic );
}


	PGPError
PGPGetDefaultMemoryMgr(PGPDefaultMemoryMgrRef defaultRef,
					   PGPMemoryMgrRef *memoryMgr)
{
	PGPError err = kPGPError_NoErr;
	
	if (IsNull(memoryMgr))
		return kPGPError_BadParams;

	*memoryMgr = kInvalidPGPMemoryMgrRef;

	if (!sDefaultMemoryMgrIsValid(defaultRef))
		return kPGPError_BadParams;

	if( IsntNull( defaultRef->memoryMgrList ) )
	{
		/*
		** If the list is non-null, then we have a mgr. We need to use a RMWO
		** lock because this is a global list
		*/
		
		PGPRMWOLockStartReading( &(defaultRef->memoryMgrListLock) );
			*memoryMgr = defaultRef->defaultMemoryMgr;
		PGPRMWOLockStopReading( &(defaultRef->memoryMgrListLock) );
	}
	else
	{
		*memoryMgr = kInvalidPGPMemoryMgrRef;
	}
	
	return( err );
}

	PGPError
PGPSetDefaultMemoryMgr(PGPDefaultMemoryMgrRef defaultRef,
					   PGPMemoryMgrRef memoryMgr)
{
	PGPError	err 			= kPGPError_NoErr;
	PGPBoolean	foundMemoryMgr	= FALSE;
	
	PGPValidateParam( PGPMemoryMgrRefIsValid( memoryMgr ) );

	if (!sDefaultMemoryMgrIsValid(defaultRef))
		return kPGPError_BadParams;
	
	if( IsNull( defaultRef->memoryMgrList ) )
	{
		/*
		** If there is no list, initialize the RMWO lock. This presents
		** a small window where the lock could get initialized twice,
		** however this is not a call likely to introduce race conditions.
		*/
	
		InitializePGPRMWOLock( &(defaultRef->memoryMgrListLock) );
	}

	PGPRMWOLockStartWriting( &(defaultRef->memoryMgrListLock) );

	if( IsntNull( defaultRef->memoryMgrList ) )
	{
		MemoryMgrInfo	*cur;
		
		/*
		** Check to see if this memory mgr is already in our list and use
		** it if it is
		*/
		
		cur = defaultRef->memoryMgrList;
		while( IsntNull( cur ) )
		{
			if( cur->mgr == memoryMgr )
			{
				foundMemoryMgr = TRUE;
				break;
			}
			
			cur = cur->next;
		}
	}

	if( ! foundMemoryMgr )
	{
		MemoryMgrInfo	*info;
		
		info = (MemoryMgrInfo *) PGPNewData( memoryMgr, sizeof( *info ), 0 );
		if( IsntNull( info ) )
		{
			info->mgr = memoryMgr;
			
			info->next					= defaultRef->memoryMgrList;
			defaultRef->memoryMgrList	= info;
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
	}

	if( IsntPGPError( err ) )
		defaultRef->defaultMemoryMgr = memoryMgr;
		
	PGPRMWOLockStopWriting( &(defaultRef->memoryMgrListLock) );
	
	return( err );
}


static void
sFreeDefaultMemoryMgrList(PGPDefaultMemoryMgrRef defaultRef)
{
	if( IsntNull( defaultRef->memoryMgrList ) )
	{
		MemoryMgrInfo	*cur;

		PGPRMWOLockStartWriting( &(defaultRef->memoryMgrListLock) );

		cur = defaultRef->memoryMgrList;
		while( IsntNull( cur ) )
		{
			MemoryMgrInfo	*next;
			PGPMemoryMgrRef	mgr;
			
			next 	= cur->next;
			mgr		= cur->mgr;
			
			PGPFreeData( cur );
			PGPFreeMemoryMgr( mgr );
			
			cur = next;
		}
		
		defaultRef->memoryMgrList = NULL;
		
		PGPRMWOLockStopWriting( &(defaultRef->memoryMgrListLock) );
		DeletePGPRMWOLock( &(defaultRef->memoryMgrListLock) );
	}
}


PGPError
PGPFreeDefaultMemoryMgr(PGPDefaultMemoryMgrRef defaultRef)
{
	PGPMemoryMgrRef mgr;

	if (!sDefaultMemoryMgrIsValid(defaultRef))
		return kPGPError_BadParams;
	
	sFreeDefaultMemoryMgrList(defaultRef);

	mgr = defaultRef->internalMemoryMgr;
	
	PGPFreeData(defaultRef);
	PGPFreeMemoryMgr(mgr);

	return kPGPError_NoErr;
}


#if TESTING

/* test reallocation, then dispose of the ptr */
	static void
sTestUsingPtr(
	PGPMemoryMgrRef	mgr,
	void *			data )
{
	PGPError		err	= kPGPError_NoErr;

	(void)mgr;
	err	= PGPReallocData( mgr, &data, 20000, 0 );
	pgpAssertNoErr( err );
	err	= PGPReallocData( mgr, &data, 500, 0 );
	pgpAssertNoErr( err );
	err	= PGPReallocData( mgr, &data, 0, 0 );
	pgpAssertNoErr( err );
	err	= PGPReallocData( mgr, &data, 20001, 0 );
	pgpAssertNoErr( err );
	
	PGPFreeData( data );
}

	static void
sTestMultiAllocations( PGPMemoryMgrRef	mgr)
{
#define kNumTestPtrs	50
	void *		plain[ kNumTestPtrs ];
	void *		secure[ kNumTestPtrs ];
	PGPUInt32	idx;
	void *		temp;
	PGPError	err	= kPGPError_NoErr;
	
	for( idx = 0; idx < kNumTestPtrs; ++idx )
	{
		PGPSize			ptrSize;
		PGPFlags		flags;
		
		ptrSize	= idx * 17;
		plain[ idx ]	= PGPNewData( mgr, ptrSize, 0);
		if ( IsntNull( plain[ idx ] ) )
		{
			flags	= PGPGetMemoryMgrDataInfo( plain[ idx ] );
			pgpAssert( ( flags & kPGPMemoryMgrBlockInfo_Valid ) != 0 );
			pgpAssert( ( flags & kPGPMemoryMgrBlockInfo_Secure ) == 0 );
			pgpAssert( ( flags & kPGPMemoryMgrBlockInfo_NonPageable ) == 0 );
		}
		
		secure[ idx ]	= PGPNewSecureData( mgr, ptrSize, 0 );
		if ( IsntNull( secure[ idx ] ) )
		{
			flags	= PGPGetMemoryMgrDataInfo( secure[ idx ] );
			pgpAssert( ( flags & kPGPMemoryMgrBlockInfo_Valid ) != 0 );
			pgpAssert( ( flags & kPGPMemoryMgrBlockInfo_Secure ) != 0 );
		}
	}
	
	/* free even numbered items */
	for( idx = 0; idx < kNumTestPtrs; idx += 2 )
	{
		PGPFreeData( plain[ idx ] );
		PGPFreeData( secure[ idx ] );
	}
	
	/* free odd numbered items */
	for( idx = 1; idx < kNumTestPtrs; idx += 2 )
	{
		PGPFreeData( plain[ idx ] );
		PGPFreeData( secure[ idx ] );
	}

	/* test realloc case with NULL ptr */
	temp	= NULL;
	err	= PGPReallocData( mgr, &temp, 1000, 0 );
	pgpAssertNoErr( err );
	pgpAssert( IsntNull( temp ) );

	PGPFreeData( temp );
}


	static void
sTestMemoryMgr( PGPMemoryMgrRef mgr )
{
	void *			temp	= NULL;
	
	/* test normal allocation */
	temp	= PGPNewData( mgr, 1000,
				kPGPMemoryMgrFlags_Clear );
	if ( IsntNull( temp ) )
	{
		sTestUsingPtr( mgr, temp );
	}
	sTestMultiAllocations( mgr );

	/* test secure allocation */
	temp	= PGPNewSecureData( mgr, 1000,
				kPGPMemoryMgrFlags_Clear );
	if ( IsntNull( temp ) )
	{
		sTestUsingPtr( mgr, temp );
	}

	sTestMultiAllocations( mgr );
	
#if MEMORY_MGR_TRACK_LEAKS
	pgpAssert( mgr->numAllocations == 0 );
#endif

}

#endif


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

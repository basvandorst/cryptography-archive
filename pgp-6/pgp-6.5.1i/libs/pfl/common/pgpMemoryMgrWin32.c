/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	Win32-specific functions to plug into the PGPMemoryMgr:
		pgpCreateStandardMemoryMgr
		pgpDisposeStandardMemoryMgrUserValue
	
	$Id: pgpMemoryMgrWin32.c,v 1.13 1999/03/10 02:53:26 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpMemoryMgrPriv.h"



#define INTEGRITY_CHECKS		PGP_DEBUG

#if PGP_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* header containing device driver definitions */
#include "pgpMemLockWin32.h"



/* names of device drivers used for memory locking */
static const char	kPGPMemLockDriver95Name[] = "\\\\.\\PGPMemLock.vxd";
static const char	kPGPMemLockDriverNTName[] = "\\\\.\\PGPMEMLOCK";

#else

typedef void *	HANDLE;

#endif


/*____________________________________________________________________________
	[Secure memory only]
	
	kPageSize		page size suitable for VM allocation and locking
	kMinHeapSize	minimum size of heap we'll create
	kChunkSize		granularity of allocation and alignment value
					nothing smaller can be allocated and therefore,
					every allocation is aligned to this value.
	kMinLeftoverBytes	dont create a free block with fewer bytes than this
____________________________________________________________________________*/

#define kPageSize			( (PGPSize)4096 )
#define kMinHeapSizeBytes	( kPageSize * 2 )
#define kChunkSize			( 32 )
#define kMinLeftoverBytes	pgpMax( kChunkSize, sizeof( ChunkHeader ) )


/*____________________________________________________________________________
	[Secure memory only]
	
	Free space is linked together in a list in each heap.
	'numChunks' contains the total size of the block, in chunks, which
	includes the ChunkHeader itself.
____________________________________________________________________________*/
typedef struct ChunkHeader
{
	struct ChunkHeader *	next;
	struct ChunkHeader *	prev;
	PGPUInt32				numChunks;
} ChunkHeader;



/*____________________________________________________________________________
	[Secure memory only]
	
	A PageHeap keeps a memory heap on a contiguous range of pages.
	Free space is maintained in a free list; there is no overhead to
	allocated items other than our chunk size requirement.
____________________________________________________________________________*/
typedef struct PageHeap
{
	struct PageHeap *	next;
	struct PageHeap *	prev;
	
	HANDLE			hDriver;
	
	void *			pages;
	PGPUInt32		numPages;
	PGPBoolean		isNonPageable;
	
	PGPUInt32		numFreeChunks;
	
	ChunkHeader *	freeList;
} PageHeap;

/*____________________________________________________________________________
	[Secure memory only]
	
	A pointer to this struct is stored as the customValue of the MemoryMgr.
____________________________________________________________________________*/
typedef struct MyData
{
	/* handle of loaded driver; NULL=>no driver loaded */
	HANDLE		hDriver;
	
	PageHeap *	heapList;
	PageHeap *	lastHeapUsed;
} MyData;




static PageHeap *	sCreateHeap( HANDLE hDriver, PGPSize heapSize );
static void			sDisposeHeap( HANDLE hDriver, PageHeap *heap );

static PageHeap *	sCreateAndAddHeap( MyData * myData, PGPSize heapSize );
static void			sRemoveAndDisposeHeap( MyData *myData, PageHeap *heap );
static PGPBoolean	sHeapOwnsAllocation( PageHeap const *heap,
						void const *allocation );
static PGPBoolean	sHeapIsEmpty( PageHeap const *heap );

static void *	sAllocateBytes( MyData *myData,
					PGPSize requestSize, PGPBoolean *isNonPageable );
static void 	sFreeBytes( MyData *myData,
					void *allocation, PGPSize allocationSize );

static void *	sHeapAllocateChunks( PageHeap *heap,
					PGPUInt32 allocChunks, PGPBoolean *isNonPageable );
static void		sHeapFreeChunks( PageHeap *heap,
					void *ptr, PGPUInt32 numChunks );

#if INTEGRITY_CHECKS	/* [ */
static void		sHeapValidate( PageHeap const *heap );
#else
#define sHeapValidate( heap )		/* nothing */
#endif

static PGPError	sInitMyData( MyData *myData );
static PGPError	sCleanupMyData( MyData *myData );

static void		sLockMemory( HANDLE hDriver, void *mem, PGPSize numBytes,
					PGPBoolean *isNonPageable );
static void		sUnlockMemory( HANDLE hDriver, void *mem, PGPSize numBytes );



	static void *
sInternalAlloc( PGPSize numBytes )
{
	void *	result	= malloc( numBytes );
	
	if ( IsntNull( result ) )
	{
		pgpClearMemory( result, numBytes );
	}
	
	return( result );
}

	static void
sInternalFree( void *ptr )
{
	if ( IsntNull( ptr ) )
		free( ptr );
}


/*____________________________________________________________________________
	For non-secure memory allocation, we just use standard malloc/realloc/free
____________________________________________________________________________*/
	static void *
sWin32MemoryAllocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	PGPSize 			allocationSize,
	PGPMemoryMgrFlags	flags )
{
	(void) mgr;
	(void) flags;
	(void) customValue;
	return( malloc( allocationSize ) );
}


	static PGPError
sWin32MemoryDeallocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	void *				allocation,
	PGPSize				allocationSize )
{
	(void) mgr;
	(void) allocationSize;
	(void) customValue;
	free( allocation );
	return( kPGPError_NoErr );
}


	static PGPError
sWin32MemoryReallocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	void **				allocation,
	PGPSize 			newAllocationSize,
	PGPMemoryMgrFlags	flags,
	PGPSize				existingSize )
{
	PGPError	err;
	void *		oldPtr	= *allocation;
	void *		newPtr	= NULL;
	
	(void) mgr;
	(void) flags;
	(void) existingSize;
	(void) customValue;

	newPtr = realloc( oldPtr, newAllocationSize );
	if ( IsNull( newPtr ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	else
	{
		err	= kPGPError_NoErr;
		*allocation	= newPtr;
	}
	
	return( err );
}




/*____________________________________________________________________________
	Win32 version

	Under Win32 we can lock individual pages or ranges of pages by using
	APIs which are available only to device drivers.  In order to avoid any 
	potential problems derived from locking pages in the process heaps, 
	we restrict ourselves to locking pages which are directly assigned to
	us from the OS using the Win32 API function VirtualAlloc.  Using 
	VirtualAlloc we can obtain virtual pages of memory from our process's 
	private address space and which are outside the process heaps.  Also,
	VirtualAlloc gives us committed and page-aligned memory, two other 
	requirements for pagelocking.

	Memory is allocated with a granularity of 4K bytes (the page size
	in Win32).
____________________________________________________________________________*/
	static void *
sWin32SecureMemoryAllocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue 		customValue,
	PGPSize 			allocationSize,
	PGPMemoryMgrFlags	flags,
	PGPBoolean *		isNonPageable )
{
	void *		result;
	MyData *	myData	= (MyData *)customValue;
	
	(void) mgr;
	(void) flags;

	*isNonPageable	= FALSE;
	
	result	= sAllocateBytes( myData, allocationSize, isNonPageable );
	
	return( result );
}


/*____________________________________________________________________________
	Win32 secure memory deallocator.
____________________________________________________________________________*/
	static PGPError
sWin32SecureMemoryDeallocationProc(
	PGPMemoryMgrRef	mgr,
	PGPUserValue 	customValue,
	void *			allocation,
	PGPSize			allocationSize,
	PGPBoolean		nonPageable)
{
	MyData *	myData	= (MyData *)customValue;

	(void) mgr;
	(void) allocationSize;
	(void) nonPageable;

	sFreeBytes( myData, allocation, allocationSize );
	
	return( kPGPError_NoErr );
}






	static HANDLE
sGetMyDriver( void )
{

	HANDLE			hDriver	= NULL;
#if PGP_WIN32
	OSVERSIONINFO	osid;

	osid.dwOSVersionInfoSize = sizeof ( osid );
	GetVersionEx ( &osid );

	/*	load correct memory-locking driver on basis of which 
		OS we're running under	*/
	switch ( osid.dwPlatformId ) {

		/* Windows 95 */
		case VER_PLATFORM_WIN32_WINDOWS :
			hDriver = 
				CreateFile (	kPGPMemLockDriver95Name, 
								GENERIC_READ | GENERIC_WRITE, 
								0, 
								NULL, 
								OPEN_EXISTING, 
								FILE_FLAG_DELETE_ON_CLOSE, 
								NULL );
//			pgpFixBeforeShip ("Need to do something if driver load fails");
///				pgpAssertMsg ( hDriver != INVALID_HANDLE_VALUE,
///							  "Could not load page-locking driver" ); 
			if ( hDriver == INVALID_HANDLE_VALUE ) 
			{
				hDriver = NULL;
			}
			break;

		/* Windows NT */
		case VER_PLATFORM_WIN32_NT :
			hDriver = 
				CreateFile (	kPGPMemLockDriverNTName,
								GENERIC_READ | GENERIC_WRITE,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL );
//			pgpFixBeforeShip ("Need to do something if driver load fails");
///				pgpAssertMsg ( hDriver != INVALID_HANDLE_VALUE,
///							  "Could not load page-locking driver" ); 
			if ( hDriver == INVALID_HANDLE_VALUE ) 
			{
				hDriver = NULL;
			}
			break;

		/* all other OSes */
		default :
			pgpDebugMsg( "unknown platform" );
			break;	
	}
#endif

	return( hDriver );
}


/*____________________________________________________________________________
	Initialize MyData data structure.  It has already been allocated.
____________________________________________________________________________*/
	static PGPError
sInitMyData( MyData *myData )
{
	PGPError		err		= kPGPError_NoErr;

	PGPValidatePtr( myData );
	
	myData->hDriver	= sGetMyDriver();
	myData->heapList		= NULL;
	myData->lastHeapUsed	= NULL;

	return err;
}


/*____________________________________________________________________________
	Dispose MyData data structure contents and free it.
____________________________________________________________________________*/
	static PGPError
sCleanupMyData( MyData *myData )
{
	PageHeap *	heap	= NULL;
	
	pgpAssertMsg( IsNull( myData->heapList ),
		"sCleanupMyData(): WARNING: secure memory is being leaked" );

	while ( IsntNull( myData->heapList ) )
	{
		sRemoveAndDisposeHeap( myData, myData->heapList );
	}
	
#if PGP_WIN32
	if ( myData->hDriver != NULL ) 
	{
		CloseHandle ( myData->hDriver );
		myData->hDriver = NULL;
	}
#endif

	sInternalFree( myData );

	return kPGPError_NoErr;
}




	PGPError
pgpCreateStandardMemoryMgr( PGPMemoryMgrRef *newMemoryMgr )
{
	PGPError				err	= kPGPError_NoErr;
	MyData *				pmyData	= NULL;
	
	PGPValidatePtr( newMemoryMgr );
	*newMemoryMgr	= NULL;
	
	/* allocate private data which we will store as the customValue */
	pmyData	= (MyData *)sInternalAlloc( sizeof( MyData ) );
	if ( IsntNull( pmyData ) )
	{
		PGPNewMemoryMgrStruct	custom;
		
		pgpClearMemory( pmyData, sizeof( MyData ) );
		err	= sInitMyData( pmyData );
		if ( IsntPGPError( err ) )
		{
			pgpClearMemory( &custom, sizeof( custom ) );
		
			custom.customValue	= (PGPUserValue)pmyData;
			
			custom.sizeofStruct		= sizeof( custom );
			custom.allocProc		= sWin32MemoryAllocationProc;
			custom.reallocProc		= sWin32MemoryReallocationProc;
			custom.deallocProc		= sWin32MemoryDeallocationProc;
			
			custom.secureAllocProc		= sWin32SecureMemoryAllocationProc;
			custom.secureDeallocProc	= sWin32SecureMemoryDeallocationProc;
			
			err	= PGPNewMemoryMgrCustom( &custom, newMemoryMgr );
			if ( IsPGPError( err ) )
			{
				sCleanupMyData( pmyData );
			}
		}
		
		if ( IsPGPError( err ) )
		{
			/* creation failed...dispose our private data */
			sInternalFree( pmyData );
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}



	PGPError
pgpDisposeStandardMemoryMgrUserValue( PGPUserValue customValue )
{
	MyData *	pmyData	= (MyData *)customValue;
	
	sCleanupMyData( pmyData );
	
	return( kPGPError_NoErr );
}



/*____________________________________________________________________________
	Allocate a page of memory from the VM system and make it non-pageable
	if possible.
____________________________________________________________________________*/
	static void *
sAllocatePages(
	HANDLE			hDriver,	/* may be NULL */
	PGPUInt32		numPages,
	PGPBoolean *	isNonPageable )
{
	void *	result	= NULL;
	
	pgpAssert( IsntNull( isNonPageable ) );
	
	*isNonPageable	= FALSE;
	
#if PGP_WIN32
	/* since the first parameter to VirtualAlloc is NULL, it automatically 
	   rounds allocation size up to next page boundary */
	result = VirtualAlloc ( NULL,
				numPages * kPageSize, MEM_COMMIT, PAGE_READWRITE );
	
	if ( IsntNull( result ) )
	{
		sLockMemory( hDriver, result, numPages * kPageSize, isNonPageable );
	}
#else
	(void)hDriver;
	result	= sInternalAlloc( numPages * kPageSize );
#endif
	
	return( result );
}


/*____________________________________________________________________________
	Free pages allocated with sAllocatePages()
____________________________________________________________________________*/
	static void
sFreePages(
	HANDLE		hDriver,	/* may be NULL */
	void *		pages,
	PGPUInt32	numPages,
	PGPBoolean	nonPageable )
{
#if PGP_WIN32
	/* we can assume it has already been safely cleared  */
	if ( nonPageable )
	{
		sUnlockMemory( hDriver, pages, numPages * kPageSize );
	}
	
	VirtualFree( pages, 0, MEM_RELEASE );
#else
	(void)hDriver;
	(void)numPages;
	(void)nonPageable;
	sInternalFree( (Ptr)pages );
#endif
}


/*____________________________________________________________________________
	Create a heap of the specified size.
____________________________________________________________________________*/
	static PageHeap *
sCreateHeap(
	HANDLE		hDriver,
	PGPSize		heapSize)
{
	PGPError	err	= kPGPError_NoErr;
	PageHeap *	heap	= NULL;
	PGPUInt32	numPages;
	PGPUInt32	numChunks;
	
	numPages	= heapSize / kPageSize;
	if ( ( heapSize % kPageSize ) != 0 )
		numPages	+= 1;
	
	numChunks	= ( numPages * kPageSize ) / kChunkSize;
	
	heap	= (PageHeap *)sInternalAlloc( sizeof( *heap ) );
	if ( IsntNull( heap ) )
	{
		heap->hDriver		= hDriver;
		heap->numPages		= numPages;
		heap->numFreeChunks	= numChunks;
		
		heap->pages	= sAllocatePages( hDriver,
						numPages, &heap->isNonPageable );
		if ( IsntNull( heap->pages ) )
		{
			ChunkHeader *	chunkHeader	= (ChunkHeader *)heap->pages;
			
			chunkHeader->next		= NULL;
			chunkHeader->prev		= NULL;
			chunkHeader->numChunks	= numChunks;
			heap->freeList	= chunkHeader;
		}
		else
		{
			sInternalFree( heap );
			heap	= NULL;
			err	= kPGPError_OutOfMemory;
		}
	}
	
	return( heap );
}


/*____________________________________________________________________________
	Dispose the heap, releasing all the memory it references.
____________________________________________________________________________*/
	static void
sDisposeHeap(
	HANDLE		hDriver,
	PageHeap *	heap )
{
	/* free chunks better be all of them... */
	pgpAssert( IsntNull( heap ) &&
		( heap->numPages * kPageSize ) / kChunkSize );
	
	sFreePages( hDriver, heap->pages, heap->numPages, heap->isNonPageable );
	
	sInternalFree( heap );
}


	static PageHeap *
sCreateAndAddHeap(
	MyData *	myData,
	PGPSize		heapSize )
{
	PGPError	err	= kPGPError_NoErr;
	PageHeap *	heap	= NULL;
	
	pgpAssert( ( heapSize % kPageSize ) == 0 );
		
	heap	= sCreateHeap( myData->hDriver, heapSize);
	if ( IsntNull( heap ) )
	{
		heap->prev	= NULL;
		heap->next	= NULL;
			
		if ( IsNull( myData->heapList ) )
		{
			myData->heapList	= heap;
		}
		else
		{
			heap->next	= myData->heapList;
			myData->heapList->prev	= heap;
			myData->heapList		= heap;
		}
		sHeapValidate( heap );
	}
	
	return( heap );
}


	static void
sRemoveAndDisposeHeap(
	MyData *	myData,
	PageHeap *	heap )
{
	PageHeap *	next;
	
	if ( heap == myData->lastHeapUsed )
		myData->lastHeapUsed	= NULL;
		
	sHeapValidate( heap );
	
	next	= heap->next;
	
	if ( heap == myData->heapList )
	{
		myData->heapList	= next;
	}

	if ( IsntNull( heap->prev ) )
	{
		heap->prev->next	= heap->next;
	}
	if ( IsntNull( heap->next ) )
	{
		heap->next->prev	= heap->prev;
	}
	
	sDisposeHeap( myData->hDriver, heap );
}


	static void *
sAllocateChunksFromExistingHeaps(
	MyData *		myData,
	PGPSize			allocChunks,
	PGPBoolean *	isNonPageable )
{
	void *		alloc	= NULL;
	PageHeap *	heap	= NULL;
	
	heap	= myData->lastHeapUsed;
	if ( IsntNull( heap ) )
	{
		alloc	= sHeapAllocateChunks( heap, allocChunks, isNonPageable);
	}
	
	if ( IsNull( alloc ) )
	{
		heap	= myData->heapList;
		while ( IsntNull( heap ) )
		{
			if ( heap->numFreeChunks >= allocChunks )
			{
				alloc	= sHeapAllocateChunks( heap,
							allocChunks,isNonPageable );
				if ( IsntNull( alloc ) )
				{
					break;
				}
			}
			heap	= heap->next;
		}
	}
	
	myData->lastHeapUsed	= heap;
	
	return( alloc );
}

	static PGPSize
sSizeToChunks( PGPSize allocationSize )
{
	PGPSize	allocChunks;
	
	allocChunks	= allocationSize / kChunkSize;
	if ( (allocationSize % kChunkSize) != 0 )
		++allocChunks;
	return( allocChunks );
}

	static void *
sAllocateBytes(
	MyData *		myData,
	PGPSize			requestSize,
	PGPBoolean *	isNonPageable )
{
	void *		alloc	= NULL;
	PGPUInt32	allocChunks;
	
	pgpAssert( IsntNull( isNonPageable ) );
	
	*isNonPageable	= FALSE;
	
	allocChunks	= sSizeToChunks( requestSize );
	
	alloc = sAllocateChunksFromExistingHeaps( myData, allocChunks,
				isNonPageable );
	if ( IsNull( alloc ) )
	{
		PGPSize		heapSize;
		PageHeap *	heap	= NULL;
		
		/* make heap size meet minimum & be multiple of page size */
		heapSize	= allocChunks * kChunkSize;
		if ( heapSize < kMinHeapSizeBytes )
			heapSize	= kMinHeapSizeBytes;
		if ( ( heapSize % kPageSize ) != 0 )
			heapSize	+= kPageSize - (heapSize % kPageSize);

		heap	= sCreateAndAddHeap( myData, heapSize );
		if ( IsntNull( heap ) )
		{
			alloc	= sHeapAllocateChunks( heap, allocChunks, isNonPageable);
		}
	}
	
	return( alloc );
}
					
	
	static PGPBoolean
sHeapOwnsAllocation(
	PageHeap const *	heap,
	void const *		allocation )
{
	PGPBoolean	ownsIt	= FALSE;
	
	if ( allocation >= heap->pages )
	{
		void *last;
	
		last	= (char *)heap->pages + ( (heap->numPages * kPageSize) -1 );
		if ( allocation <= last )
			ownsIt	= TRUE;
	}
	
	return( ownsIt );
}

	
	static PGPBoolean
sHeapIsEmpty( PageHeap const *	heap)
{
	PGPSize	numChunks	= (heap->numPages * kPageSize ) / kChunkSize;
	
	return( heap->numFreeChunks == numChunks );
}


	static void
sFreeBytes(
	MyData *		myData,
	void *			allocation,
	PGPSize			allocationSize )
{
	PageHeap *	heap;
	PGPSize		allocChunks;
	
	allocChunks	= sSizeToChunks( allocationSize );
	
	heap	= myData->heapList;
	while ( IsntNull( heap ) )
	{
		sHeapValidate( heap );
		
		if ( sHeapOwnsAllocation( heap, allocation ) )
		{
			sHeapFreeChunks( heap, allocation, allocChunks );
			
			if ( sHeapIsEmpty( heap ) )
			{
				sRemoveAndDisposeHeap( myData, heap );
			}
			break;
		}
		heap	= heap->next;
	}
}


	static void *
sHeapAllocateChunks(
	PageHeap *		heap,
	PGPUInt32 		allocChunks,
	PGPBoolean *	isNonPageable )
{
	void *			result	= NULL;
	ChunkHeader *	chunk	= NULL;
	
	pgpAssert( IsntNull( isNonPageable ) );
	*isNonPageable	= FALSE;
	
	sHeapValidate( heap );
	
	chunk	= heap->freeList;
	while ( IsntNull( chunk ) )
	{
		if ( chunk->numChunks >= allocChunks )
		{
			ChunkHeader *	remainder;
			PGPUInt32		leftoverChunks;
			PGPUInt32		actualChunksUsed	= 0;
			
			result	= (void *)chunk;
			
			remainder	= (ChunkHeader *)
							((char *)chunk + (allocChunks * kChunkSize ));
			leftoverChunks	= chunk->numChunks - allocChunks;
			
			if ( leftoverChunks != 0 &&
					(leftoverChunks * kChunkSize >= kMinLeftoverBytes) )
			{
				actualChunksUsed	= allocChunks;
				
				remainder->numChunks	= leftoverChunks;
				remainder->next			= chunk->next;
				remainder->prev			= chunk->prev;
				
				/* need to make a new chunk out of leftover */
				if ( chunk == heap->freeList )
				{
					heap->freeList	= remainder;
				}
				else
				{
					chunk->prev->next	= chunk->next;
				}
				if ( IsntNull( remainder->next ) )
					remainder->next->prev	= remainder;
				if ( IsntNull( remainder->prev ) )
					remainder->prev->next	= remainder;
			}
			else
			{
				actualChunksUsed	= allocChunks + leftoverChunks;
				
				/* remove the chunk from the list */
				if ( chunk == heap->freeList )
				{
					heap->freeList	= chunk->next;
				}
				else
				{
					chunk->prev->next	= chunk->next;
				}
				if ( IsntNull( chunk->next ) )
					chunk->next->prev	= chunk->prev;
			}
			
			heap->numFreeChunks	-= actualChunksUsed;
			
			*isNonPageable	= heap->isNonPageable;
			break;
		}
		chunk	= chunk->next;
	}
	
	sHeapValidate( heap );
	
	return( result );
}


	static PGPBoolean
sChunksAreAdjacent( ChunkHeader const * chunkHeader )
{
	ChunkHeader const *	next	= NULL;
	PGPSize		chunkBytes;
	
	pgpAssert( IsntNull( chunkHeader ) );
	next	= chunkHeader->next;
	if ( IsNull( next ) )
		return( FALSE );
	
	chunkBytes	= chunkHeader->numChunks * kChunkSize;
	
	return( (char *)chunkHeader + chunkBytes == (char *)next );
}

	static void
sMergeChunks( ChunkHeader *chunkHeader )
{
	ChunkHeader *	next	= chunkHeader->next;
	
	pgpAssert( sChunksAreAdjacent( chunkHeader ) );
	
	chunkHeader->numChunks	+= next->numChunks;
	if ( IsntNull( next->next ) )
		next->next->prev	= chunkHeader;
	chunkHeader->next	= next->next;
}

	static void
sMergeFree( ChunkHeader *chunkHeader )
{
	ChunkHeader *	prev	= NULL;
	
	pgpAssert( IsntNull( chunkHeader ) );
	
	if ( sChunksAreAdjacent( chunkHeader ) )
	{
		sMergeChunks( chunkHeader );
	}
	prev	= chunkHeader->prev;
	if ( IsntNull( prev ) && sChunksAreAdjacent( prev ) )
	{
		sMergeChunks( prev );
		chunkHeader	= NULL;
	}
}

/*____________________________________________________________________________
	Free an allocation within a heap of specified size.
____________________________________________________________________________*/
	static void
sHeapFreeChunks(
	PageHeap *	heap,
	void *		allocation,
	PGPUInt32 	allocChunks )
{
	ChunkHeader *	chunkHeader;
	ChunkHeader *	cur;
	ChunkHeader *	preceeding	= NULL;
	
	pgpAssert( sHeapOwnsAllocation( heap, allocation ) );
	
	sHeapValidate( heap );
	
	heap->numFreeChunks	+= allocChunks;
	
	chunkHeader	= (ChunkHeader *)allocation;
	chunkHeader->numChunks	= allocChunks;
	
	/* find the first chunk which precedes this one */
	cur	= heap->freeList;
	while ( IsntNull( cur ) )
	{
		if ( chunkHeader < cur )
		{
			preceeding	= cur->prev;
			break;
		}
		cur	= cur->next;
	}
	
	if ( IsNull( preceeding ) )
	{
		chunkHeader->next	= heap->freeList;
		chunkHeader->prev	= NULL;
		if ( IsntNull( chunkHeader->next ) )
			chunkHeader->next->prev	= chunkHeader;
		heap->freeList		= chunkHeader;
	}
	else
	{
		chunkHeader->prev	= preceeding;
		chunkHeader->next	= preceeding->next;
		if ( IsntNull( preceeding->next ) )
			preceeding->next->prev	= chunkHeader;
		preceeding->next	= chunkHeader;
	}
	
	sMergeFree( chunkHeader );
	
	sHeapValidate( heap );
}


#if INTEGRITY_CHECKS	/* [ */
	static void
sHeapValidate( PageHeap const *	heap )
{
	ChunkHeader *	cur;
	PGPSize			freeChunks	= 0;
	
	/* verify that free chunks match count in heap */
	cur	= heap->freeList;
	while ( IsntNull( cur ) )
	{
		pgpAssert( cur->numChunks != 0 );
		freeChunks	+= cur->numChunks;
		
		/* verify next item points back at cur */
		if ( IsntNull( cur->next ) )
		{
			pgpAssert( cur->next->prev == cur );
		}
		cur	= cur->next;
	}
	pgpAssert( heap->numFreeChunks == freeChunks );
}
#endif	/* ] */


#if PGP_WIN32	/* [ */
					
/*____________________________________________________________________________
	Win32 version of sLockMemory()

	Win32 supports locking of pages into memory only by using
	special purpose drivers (VxD under Win95, kernel mode driver
	under WinNT).  Note, the current driver implementations take
	the virtual (linear) address to lock and the number of bytes.
	This memory must have been allocated on a page-boundary and 
	committed using the Win32 API function VirtualAlloc (see routine
	sNewSecureDataHeader).

	Here we attempt to use such drivers if they have been successfully
	loaded (if the handle is non-NULL).  In this case, the return
	codes from the DeviceIoControl call and the driver are used to 
	ascertain if the lock was successful.

	If the driver has not been loaded, we revert back to the "old" 
	method of calling the Win32 APIs "VirtualLock" and "VirtualUnlock".
	Under Windows95 these functions do absolutely nothing.  
	Under WindowsNT they prevent the specified memory range from being 
	paged to disk *while this process is executing*.  When all threads 
	of this process become pre-empted, the OS is free to page whatever 
	it wants (including "VirtualLock-ed" memory) to disk.

	Assign a value of "TRUE" to 'isNonPageable' only if we have
	a valid driver loaded and the driver successfully locks the page
	in memory. 
____________________________________________________________________________*/

	static void
sLockMemory(
	HANDLE			hDriver,
	void *			mem,
	PGPSize			numBytes,
	PGPBoolean *	isNonPageable )
{
	PGPMEMLOCKSTRUCT	mls;
	DWORD				dw;
	BOOL				bDIOreturn;

	pgpAssert( IsntNull( isNonPageable ) );
	*isNonPageable	= FALSE;
 
	if ( IsntNull( hDriver ) ) 
	{
		mls.pMem = mem;
		mls.ulNumBytes = numBytes;
		bDIOreturn = DeviceIoControl(	hDriver, 
										IOCTL_PGPMEMLOCK_LOCK_MEMORY, 
										&mls, 
										sizeof( mls ), 
										&mls, 
										sizeof( mls ), 
										&dw, 
										NULL );
		pgpAssertMsg ( bDIOreturn, "DIOC error from page-locking driver" );
		if ( bDIOreturn )
		{
			pgpAssertMsg ( mls.ulError == 0, 
						  "Internal error in page-locking driver" );

			if ( mls.ulError == 0 ) 
			{
				*isNonPageable	= TRUE;
			}		
		}
	}
	else 
	{
	 	VirtualLock ( mem, numBytes );	/* call for good measure*/
	}
}


	static void
sUnlockMemory(
	HANDLE		hDriver,
	void *		mem,
	PGPSize		numBytes )
{
	PGPMEMLOCKSTRUCT	mls;
	DWORD				dw;
	BOOL				bDIOreturn;

	if ( IsntNull( hDriver ) )
	{
		mls.pMem = mem;
		mls.ulNumBytes = numBytes;
		bDIOreturn = DeviceIoControl(	hDriver, 
										IOCTL_PGPMEMLOCK_UNLOCK_MEMORY, 
										&mls, 
										sizeof( mls ), 
										&mls, 
										sizeof( mls ), 
										&dw, 
										NULL );
		pgpAssertMsg ( bDIOreturn, "DIOC error from page-locking driver" );
		if ( bDIOreturn )
		{
			pgpAssertMsg ( mls.ulError == 0, 
						  "Internal error in page-locking driver" );
		}
	}
	else 
	{
	 	VirtualUnlock ( mem, numBytes );	/* call for good measure*/
	}
}

#endif	/* ] PGP_WIN32 */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

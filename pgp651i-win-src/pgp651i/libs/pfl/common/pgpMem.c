/*____________________________________________________________________________
	pgpMem.c
	
	Copyright (C) 1996,1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	pgpMemAlloc and friends with debugging support

	$Id: pgpMem.c,v 1.28 1999/03/10 02:52:57 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpPFLErrors.h"
#include "pgpDebug.h"
#include "pgpLeaks.h"


#ifdef DEBUG_FIND_LEAKS
#error use PGP_DEBUG_FIND_LEAKS instead of DEBUG_FIND_LEAKS
#endif

#ifndef PGP_DEBUG_FIND_LEAKS
#error you must #define PGP_DEBUG_FIND_LEAKS as 0 or 1
#endif



/* Fills allocated/deallocated memory with 0xDD's */
#ifndef DEBUG_FILL_MEM
#define DEBUG_FILL_MEM			PGP_DEBUG
#endif

/* Puts a ulong-sized magic number before the start of each block */
#ifndef DEBUG_MEM_HEAD_MAGIC
#define DEBUG_MEM_HEAD_MAGIC	PGP_DEBUG
#endif

/* Puts magic bytes after the end of each block */
#ifndef DEBUG_MEM_TAIL_MAGIC
#if PGP_DEBUG
#define DEBUG_MEM_TAIL_MAGIC	4
#else
#define DEBUG_MEM_TAIL_MAGIC	0
#endif
#endif

/* Makes pgpMemRealloc always move a growing block */
#ifndef DEBUG_ALWAYS_MOVE
#define DEBUG_ALWAYS_MOVE		PGP_DEBUG
#endif

#if DEBUG_FILL_MEM
#define MaybeFillMem(p, size)	pgpFillMemory((void *)(p), (size), 0xDD)
#else
#define MaybeFillMem(p, size)
#endif

/* This is a useful macro to find the minimum of two values */
#ifndef min
#define min(x,y) (((x)<(y)) ? (x) : (y))
#endif

#define kMemHeaderMagic		0xD837C51E

typedef struct MemHeader
{
#if PGP_DEBUG_FIND_LEAKS
	LeakItem	leakItem;	/* Information for the leaks package */
#endif
	size_t		size;		/* Size of the user area of the block */
#if DEBUG_MEM_HEAD_MAGIC
	PGPUInt32	magic;		/* Trailing magic number */
#endif
} MemHeader;

#define UserPtrToMemHeader(userPtr)		\
		((MemHeader *)((char *)(userPtr) - sizeof(MemHeader)))
#define MemHeaderToUserPtr(hdrPtr)		\
		((void *)((char *)(hdrPtr) + sizeof(MemHeader)))
#define FullBlockSize(userSize)			\
		(sizeof(MemHeader) + (userSize) + DEBUG_MEM_TAIL_MAGIC)

/*
 * Defines the sequence of tail magic bytes.  We want every byte to be
 * odd to catch memory references, but also each should be distinct.
 */
#define TailMagicByte(i)				\
		((char)(34 * (i) + 0x3D))

#if PGP_DEBUG_FIND_LEAKS
static DECLARE_LEAKDEALLOCTYPE(kPGPFree, "pgpFree");
static DECLARE_LEAKALLOCTYPE(kPGPAlloc, "pgpAlloc", kPGPFree);
#endif

/*_____________________________________________________________________________
  Platform specific routines
_____________________________________________________________________________*/

/*
 * These platform-specific routines can be very simple.  They needn't
 * deal with NULL pointers or perform any assertion checking.  They will
 * never be passed a zero size, because a MemHeader will always be included.
 * pgpPlatformRealloc should never move the block itself; instead it should
 * just return kPGPError_OutOfMemory, causing the platform-independent routine
 * to allocate a new block and move the memory itself.
 */
static void *pgpPlatformAlloc(size_t size);
static PGPError pgpPlatformRealloc(void **ptrRef, size_t oldSize,
									size_t newSize);
static void pgpPlatformFree(void *ptr);

#if PGP_MACINTOSH		/* [ */

#include <Memory.h>

/*
 * Macintosh specific routines
 */
 
	void
pgpCopyMemory(
	void const *	src,
	void *			dest,
	size_t			size)
{
	pgpa((
		pgpaAddrValid(src, VoidAlign),
		pgpaAddrValid(dest, VoidAlign)));
	
	BlockMoveData(src, dest, (Size)size);
}

	static void *
pgpPlatformAlloc(
	size_t		size)
{
#undef NewPtr	/* We don't want DebugTraps_NewPtr tracking this */
	return (void *)NewPtr((Size)size);
}

	static PGPError
pgpPlatformRealloc(
	void **		ptrRef,
	size_t		oldSize,
	size_t		newSize)
{
	Ptr			ptr = (Ptr)*ptrRef;
	size_t		actualSize;
	
	(void)oldSize;		/* Avoid warning */
	SetPtrSize(ptr, (Size)newSize);
	actualSize = GetPtrSize(ptr);
	if (actualSize != newSize)
		return kPGPError_OutOfMemory;
	else
		return kPGPError_NoErr;
}

	static void
pgpPlatformFree(
	void *		ptr)
{
#undef DisposePtr	/* We don't want DebugTraps_DisposePtr tracking this */
	DisposePtr((char *)ptr);
}

#else	/* ] PGP_MACINTOSH [ */

/*
 * ANSI-specific routines (for non-Macs)
 */

	void
pgpCopyMemory(
	void const *	src,
	void *			dest,
	size_t			size)
{
	pgpa((
		pgpaAddrValid(src, VoidAlign),
		pgpaAddrValid(dest, VoidAlign)));
	
	memmove(dest, src, size);
}

	static void *
pgpPlatformAlloc(
	size_t		size)
{
	return (void *)malloc(size);
}

	static PGPError
pgpPlatformRealloc(
	void **		pp,
	size_t		oldSize,
	size_t		newSize)
{
	void *		oldP = *pp;
	void *		newP;
	
	if (newSize > oldSize)
		return kPGPError_OutOfMemory;
	newP = realloc(oldP, newSize);
	if (newP == NULL)
		return kPGPError_OutOfMemory;
	
	*pp = newP;
	return kPGPError_NoErr;
}

	static void
pgpPlatformFree(
	void *		p)
{
	free(p);
}

#endif	/* ] */

/*_____________________________________________________________________________
  Platform independent routines
_____________________________________________________________________________*/

#if PGP_DEBUG	/* [ */

	PGPBoolean
pgpaInternalMemBlockValid(
	pgpaCallPrefixDef,
	const void *			userPtr)
{
	MemHeader *		header = UserPtrToMemHeader(userPtr);
	
	pgpaAssert(userPtr != NULL);
	pgpaAddrValid(header, MemHeader);

#if PGP_DEBUG_FIND_LEAKS
	pgpaLeakItemValid(&header->leakItem);
#endif

#if DEBUG_MEM_HEAD_MAGIC
	pgpaAssert(header->magic == kMemHeaderMagic);
#endif

#if DEBUG_MEM_TAIL_MAGIC
	{
		char *		tailMagic = (char *)userPtr + header->size;
		int			i;
		
		for (i = 0; !pgpaFailed && i < DEBUG_MEM_TAIL_MAGIC; i++)
			pgpaAssert(tailMagic[i] == TailMagicByte(i));
	}
#endif

	return pgpaFailed;
}

	static void
pgpMemSetTailMagic(
	MemHeader *	header)
{
#if DEBUG_MEM_TAIL_MAGIC
	char *		tailMagic = (char *)header + sizeof(MemHeader) + header->size;
	int			i;
	
	for (i = 0; i < DEBUG_MEM_TAIL_MAGIC; i++)
		tailMagic[i] = TailMagicByte(i);
#endif
}

#endif	/* ] PGP_DEBUG */

	void *
PGP_INTERNAL_ALLOC(
	size_t			size
	PGPALLOC_CONTEXT_PARAMS_DEF)	/* fileName and lineNumber */
{
	MemHeader *		header;
	void *			userPtr;
	

	header = (MemHeader *)pgpPlatformAlloc(FullBlockSize(size));
	if (header == NULL)
		return NULL;
	
	MaybeFillMem(header, FullBlockSize(size));
	userPtr = MemHeaderToUserPtr(header);
	
#if PGP_DEBUG_FIND_LEAKS
	pgpLeaksRememberItem(&header->leakItem, kPGPAlloc,
						userPtr, size, fileName, lineNumber);
#endif

	header->size = size;
	
#if DEBUG_MEM_HEAD_MAGIC
	header->magic = kMemHeaderMagic;
#endif

#if DEBUG_MEM_TAIL_MAGIC
	pgpMemSetTailMagic(header);
#endif
	return userPtr;
}

	PGPError
pgpRealloc(
	void **			userPtrRef,
	size_t			newSize)
{
	size_t			oldSize;
	void *			oldUserPtr = *userPtrRef;
	MemHeader *		oldHeader = UserPtrToMemHeader(oldUserPtr);
	void *			newUserPtr;
	MemHeader *		newHeader;
	PGPError		result;
	
	pgpa(pgpaMemBlockValid(oldUserPtr));
	oldSize = oldHeader->size;
	if (newSize == oldSize)
		return kPGPError_NoErr;
	
#if DEBUG_FILL_MEM
	/* If we're shrinking the block, wipe the tail end before resizing */
	if (newSize < oldSize)
		MaybeFillMem((char *)oldUserPtr + newSize, oldSize - newSize);
#endif
	
	newHeader = oldHeader;
	/* Always move growing blocks if DEBUG_ALWAYS_MOVE is set */
	if (DEBUG_ALWAYS_MOVE && newSize > oldSize)
		result = kPGPError_OutOfMemory;
	else
		result = pgpPlatformRealloc((void **)&newHeader,
						FullBlockSize(oldSize), FullBlockSize(newSize));
	
	if (result == kPGPError_OutOfMemory)
	{
		/* pgpPlatformRealloc failed, try allocating a new block and moving */
		pgpa((
			pgpaAssert(newSize > oldSize),
			pgpaMsg("pgpPlatformRealloc failed to shrink block")));
		newHeader = (MemHeader *)pgpPlatformAlloc(FullBlockSize(newSize));
		if (newHeader == NULL)
			return kPGPError_OutOfMemory;
		pgpCopyMemory((void *)oldHeader, (void *)newHeader,
					sizeof(MemHeader) + oldSize);
		MaybeFillMem(oldHeader, FullBlockSize(oldSize));
		pgpPlatformFree(oldHeader);
	}
	else if (result != kPGPError_NoErr)
		return result;
	
	newUserPtr = MemHeaderToUserPtr(newHeader);
	
#if DEBUG_FILL_MEM
	/* If we're growing the block, wipe the newly-allocated portion */
	if (newSize > oldSize)
		MaybeFillMem((char *)newUserPtr + oldSize, newSize - oldSize);
#endif
	
	if (newHeader != oldHeader)
	{
#if PGP_DEBUG_FIND_LEAKS
		pgpLeaksNotifyItemMoved(&oldHeader->leakItem, &newHeader->leakItem);
#endif
		*userPtrRef = newUserPtr;
	}
	newHeader->size = newSize;
	
#if DEBUG_MEM_TAIL_MAGIC
	pgpMemSetTailMagic(newHeader);
#endif
	return kPGPError_NoErr;
}

	void
pgpFree(
	void *			userPtr)
{
	MemHeader *		header = UserPtrToMemHeader(userPtr);
	
	pgpa(pgpaMemBlockValid(userPtr));
#if PGP_DEBUG_FIND_LEAKS
	pgpLeaksForgetItem(&header->leakItem, kPGPFree);
#endif
	MaybeFillMem(header, FullBlockSize(header->size));
	pgpPlatformFree(header);
}

/*_____________________________________________________________________________
  Backward-compatible wrapper routines,
  which treat NULL like a 0-sized block.
_____________________________________________________________________________*/

	void *
PGP_INTERNAL_MEMALLOC(
	size_t			size
	PGPALLOC_CONTEXT_PARAMS_DEF)	/* fileName and lineNumber */
{
	if (size == 0)
		return NULL;
	else
		return PGP_INTERNAL_ALLOC(size  PGPALLOC_CONTEXT_PASS_PARAMS);
}

	void *
PGP_INTERNAL_MEMREALLOC(
	void *			userPtr,
	size_t			newSize
	PGPALLOC_CONTEXT_PARAMS_DEF)	/* fileName and lineNumber */
{
	if (userPtr == NULL)
		return PGP_INTERNAL_MEMALLOC(newSize  PGPALLOC_CONTEXT_PASS_PARAMS);
	else if (newSize == 0)
	{
		if ( IsntNull( userPtr ) )
			pgpFree(userPtr);
		return NULL;
	}
	else if (pgpRealloc(&userPtr, newSize) != kPGPError_NoErr)
		return NULL;
	else
		return userPtr;
}

#if 0
	void
pgpMemFree(
	void *			userPtr)
{
	if (userPtr)
		pgpFree(userPtr);
}
#endif

	void
pgpCopyPattern(
	const void *	pattern,
	size_t			patternLength,
	void *			buffer,
	size_t			bufferLength)
{
	size_t	count;
	size_t	remaining	= bufferLength;
	size_t	doneSoFar	= 0;
	
	pgpa((
		pgpaAddrValid(pattern, VoidAlign),
		pgpaAssert(patternLength > 0),
		pgpaAddrValid(buffer, VoidAlign)));
	
	/* copy the pattern once first */
	count = min(patternLength, bufferLength);
	pgpCopyMemory(pattern, buffer, count);
	remaining	-= count;
	doneSoFar	+= count;
	
	while ( remaining != 0 )
	{
		/*
		** replicate the pattern we have produced so far
		** because the original pattern might be small (say 1 PGPByte)
		*/
		
		count = min(doneSoFar, remaining);
		pgpCopyMemory(buffer, ((uchar *)buffer) + doneSoFar, count);
		
		remaining	-= count;
		doneSoFar	+= count;
	}
}

#if PGP_MACINTOSH	/* [ */

/*____________________________________________________________________________
	Fill memory with a PGPByte pattern.
	Obviously, this routine can be optimized.
  ____________________________________________________________________________*/
	
	static inline void
FillBytes(
	void *	buffer,
	size_t	length,
	uchar	fillChar)
{
	char *	cur = (char *)buffer;
	UInt32	remaining = length;
	
	while (remaining-- > 0)
		*cur++ = fillChar;
}

/*____________________________________________________________________________
	Fast fill routine.  Highly efficient for fills of as little as 16 bytes.
	
	Testing on PPC 604 shows that filling more than 4 bytes (one long) per loop
	iteration does not produce any measurable speed improvement.  That fact,
	coupled with the fact that using PGPByte fills, even on small chunks, is
	slow, means that unrolling it substantially is not a win.
  ____________________________________________________________________________*/
	void
pgpFillMemory(
	void *			buffer,
	const size_t	length,
	const uchar		fillChar)
{
	const UInt32	kAlignSize			= sizeof(UInt32);
	const UInt32	kUnrollBytesPerIter	= 8;	/* must be power of 2 */
	uchar *			cur					= (uchar *)buffer;
	UInt32			remaining			= length;
	
	pgpa((
		pgpaAddrValid(buffer, VoidAlign),
		pgpaAssert( length < 128 *1024UL * 1024UL )));
	
	if ( length >= kUnrollBytesPerIter )
	{
		UInt32			fillPattern;
		UInt32			oddballCount;
		UInt32 *		curLong;
		UInt32			numChunks;
		
		pgpAssert( length >= kAlignSize );
		
		/* create the fill pattern */
		pgpAssert( sizeof(fillPattern) == 4 * sizeof(fillChar) );
		fillPattern = fillChar;
		fillPattern |= (fillPattern << 8);
		fillPattern |= (fillPattern << 16);
		
		/* align address to kAlignSize PGPByte boundary */
		oddballCount	= kAlignSize - ( ((UInt32)cur) % kAlignSize );
		pgpAssert( oddballCount <= remaining );
		FillBytes( cur, oddballCount, fillChar );
		remaining	-= oddballCount;
		cur			+= oddballCount;
		pgpAssert( remaining == 0 || (((UInt32)cur) % kAlignSize) == 0 );
		
		/* fill all the chunks */
		curLong		= (UInt32 *)cur;
		numChunks	= remaining / kUnrollBytesPerIter;
		while ( numChunks-- != 0 )
		{
			/* CAUTION:
				this loop must move a total of 'kUnrollBytesPerIter' bytes */
			*curLong++	= fillPattern;
			*curLong++	= fillPattern;
		}
		
		/* any remaining bytes get filled below... */
		cur			= (Byte *)curLong;
		remaining	&= ( kUnrollBytesPerIter - 1 );
	}
		
	FillBytes( cur, remaining, fillChar );
	
	/* verify that at least first and last bytes have been set properly */
	pgpAssert( length == 0 ||
		( ((Byte *)buffer)[ 0 ] == fillChar &&
		((Byte *)buffer)[ length - 1 ] == fillChar) );
}

/*____________________________________________________________________________
	Return true if memory blocks are equal, false otherwise.
	
	Note: not yet optimized; this version is slow.
____________________________________________________________________________*/
	PGPBoolean
pgpMemoryEqual(
	const void *	b1,
	const void *	b2,
	size_t			length)
{
	size_t			remaining	= length;
	const uchar *	cur1	= (uchar *)b1;
	const uchar *	cur2	= (uchar *)b2;
	
	pgpa((
		pgpaAddrValid(b1, VoidAlign),
		pgpaAddrValid(b2, VoidAlign)));
	
	while (remaining-- > 0)
		if (*cur1++ != *cur2++)
			return FALSE;
	return TRUE;
}

#endif	/* ] PGP_MACINTOSH */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
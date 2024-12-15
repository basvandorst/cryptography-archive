/*
 * pgpMem.c -- pgpMemAlloc and friends with debugging support
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpMem.c,v 1.9.2.3.2.1 1997/06/27 15:36:19 cbertsch Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpDebug.h"
#include "pgpLeaks.h"

/* Fills allocated/deallocated memory with 0xDD's */
#ifndef DEBUG_FILL_MEM
#define DEBUG_FILL_MEM			DEBUG
#endif

/* Puts a ulong-sized magic number before the start of each block */
#ifndef DEBUG_MEM_HEAD_MAGIC
#define DEBUG_MEM_HEAD_MAGIC	DEBUG
#endif

/* Puts magic bytes after the end of each block */
#ifndef DEBUG_MEM_TAIL_MAGIC
#if DEBUG
#define DEBUG_MEM_TAIL_MAGIC	4
#else
#define DEBUG_MEM_TAIL_MAGIC	0
#endif
#endif

/* Makes pgpMemRealloc always move a growing block */
#ifndef DEBUG_ALWAYS_MOVE
#define DEBUG_ALWAYS_MOVE		 DEBUG
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

typedef struct MemHeader MemHeader;
struct MemHeader
{
#if DEBUG_FIND_LEAKS
	LeakItem	leakItem;	/* Information for the leaks package */
#endif
	size_t		size;		 /* Size of the user area of the block */
#if DEBUG_MEM_HEAD_MAGIC
	ulong	 	magic;		 /* Trailing magic number */
#endif
};

#define UserPtrToMemHeader(userPtr)		\
		((MemHeader *)((char *)(userPtr) - sizeof(MemHeader)))
#define MemHeaderToUserPtr(hdrPtr)		\
 	((void *)((char *)(hdrPtr) + sizeof(MemHeader)))
#define FullBlockSize(userSize)			\
		(sizeof(MemHeader) + (userSize) + DEBUG_MEM_TAIL_MAGIC)

/*
* Defines the sequence of tail magic bytes. We want every byte to be
* odd to catch memory references, but also each should be distinct.
*/
#define TailMagicByte(i)				\
		((char)(34 * (i) + 0x3D))

#if DEBUG_FIND_LEAKS
static DECLARE_LEAKDEALLOCTYPE(kPGPFree, "pgpFree");
static DECLARE_LEAKALLOCTYPE(kPGPAlloc, "pgpAlloc", kPGPFree);
#endif

/*_____________________________________________________________________________
Platform specific routines
_____________________________________________________________________________*/

/*
* These platform-specific routines can be very simple. They needn't
* deal with NULL pointers or perform any assertion checking. They will
* never be passed a zero size, because a MemHeader will always be included.
* pgpPlatformRealloc should never move the block itself; instead it should
* just return PGPERR_NOMEM, causing the platform-independent routine to
* allocate a new block and move the memory itself.
*/
static void *pgpPlatformAlloc(size_t size);
static PGPError pgpPlatformRealloc(void **ptrRef, size_t oldSize,
								size_t newSize);
static void pgpPlatformFree(void *ptr);

#if MACINTOSH		 /* [ */

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
		pgpaAddrValid(dest, VoidAlign),
		pgpaAssert(size >= 0)));

BlockMoveData(src, dest, (Size)size);
}

	static void *
pgpPlatformAlloc(
	size_t		size)
{
	pgpa((
 	pgpaAssert((Size)size >= 0),
		pgpaMsg((pgpaFmtPrefix,
				"pgpPlatformAlloc: size negative or too large"))));

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
	
			/*
	* We need to perform this additional assertion, because the Mac uses a
	* signed long instead of an unsigned long (size_t) for memory allocation
			*/
	pgpa((
		pgpaAssert((Size)newSize >= 0),
		pgpaMsg((pgpaFmtPrefix,
			"pgpPlatformRealloc: size negative or too large"))));
	
	(void)oldSize;		/* Avoid warning */
	SetPtrSize(ptr, (Size)newSize);
	actualSize = GetPtrSize(ptr);
	if (actualSize != newSize)
		return PGPERR_NOMEM;
	else
		return PGPERR_OK;
}

	static void
pgpPlatformFree(
	void *		ptr)
{
#undef DisposePtr	/* We don't want DebugTraps_DisposePtr tracking this */
	DisposePtr((char *)ptr);
}

#else	/* ] MACINTOSH [ */

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
		pgpaAddrValid(dest, VoidAlign),
		pgpaAssert(size >= 0)));
	
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
		return PGPERR_NOMEM;
	newP = realloc(oldP, newSize);
	if (newP == NULL)
		return PGPERR_NOMEM;
	
	*pp = newP;
	return PGPERR_OK;
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

#if DEBUG	/* [ */

	Boolean
pgpaInternalMemBlockValid(
	pgpaCallPrefixDef,
	void *			userPtr)
{
	MemHeader *		header = UserPtrToMemHeader(userPtr);
	
	pgpaAssert(userPtr != NULL);
	pgpaAddrValid(header, MemHeader);

#if DEBUG_FIND_LEAKS
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

#endif	/* ] DEBUG */

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
	
#if DEBUG_FIND_LEAKS
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
	void ** 		userPtrRef,
	size_t		 newSize)
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
		return PGPERR_OK;
	
#if DEBUG_FILL_MEM
	/* If we're shrinking the block, wipe the tail end before resizing */
	if (newSize < oldSize)
		MaybeFillMem((char *)oldUserPtr + newSize, oldSize - newSize);
#endif
	
	newHeader = oldHeader;
	/* Always move growing blocks if DEBUG_ALWAYS_MOVE is set */
	if (DEBUG_ALWAYS_MOVE && newSize > oldSize)
		result = PGPERR_NOMEM;
	else
		result = pgpPlatformRealloc((void **)&newHeader,
						FullBlockSize(oldSize), FullBlockSize(newSize));
	
	if (result == PGPERR_NOMEM)
	{
		/* pgpPlatformRealloc failed, try allocating a new block and moving */
		pgpa((
			pgpaAssert(newSize > oldSize),
			pgpaMsg("pgpPlatformRealloc failed to shrink block")));
		newHeader = (MemHeader *)pgpPlatformAlloc(FullBlockSize(newSize));
		if (newHeader == NULL)
			return PGPERR_NOMEM;
		pgpCopyMemory((void *)oldHeader, (void *)newHeader,
					sizeof(MemHeader) + oldSize);
		MaybeFillMem(oldHeader, FullBlockSize(oldSize));
		pgpPlatformFree(oldHeader);
	}
	else if (result != PGPERR_OK)
		return result;
	
	newUserPtr = MemHeaderToUserPtr(newHeader);

#if DEBUG_FILL_MEM
	/* If we're growing the block, wipe the newly-allocated portion */
	if (newSize > oldSize)
		MaybeFillMem((char *)newUserPtr + oldSize, newSize - oldSize);
#endif
	
if (newHeader != oldHeader)
	{
#if DEBUG_FIND_LEAKS
		pgpLeaksNotifyItemMoved(&oldHeader->leakItem, &newHeader->leakItem);
#endif
		*userPtrRef = newUserPtr;
	}
	newHeader->size = newSize;
	
#if DEBUG_MEM_TAIL_MAGIC
	pgpMemSetTailMagic(newHeader);
#endif
	return PGPERR_OK;
}

	void
pgpFree(
	void *			userPtr)
{
	MemHeader *		header = UserPtrToMemHeader(userPtr);
	
	pgpa(pgpaMemBlockValid(userPtr));
#if DEBUG_FIND_LEAKS
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
		return PGP_INTERNAL_ALLOC(size PGPALLOC_CONTEXT_PASS_PARAMS);
}

 void *
PGP_INTERNAL_MEMREALLOC(
	void *			userPtr,
	size_t			newSize
	PGPALLOC_CONTEXT_PARAMS_DEF)	/* fileName and lineNumber */
{
	if (userPtr == NULL)
		return PGP_INTERNAL_MEMALLOC(newSize PGPALLOC_CONTEXT_PASS_PARAMS);
	else if (newSize == 0)
	{
		pgpMemFree(userPtr);
		return NULL;
	}
	else if (pgpRealloc(&userPtr, newSize) != PGPERR_OK)
		return NULL;
	else
		return userPtr;
}

	void
pgpMemFree(
	void *			userPtr)
{
	if (userPtr)
		pgpFree(userPtr);
}

	void
pgpCopyPattern(
	const void *	pattern,
 size_t	 		 patternLength,
	void *			buffer,
	size_t			bufferLength)
{
	size_t	count;
	size_t	remaining	 = bufferLength;
	size_t	doneSoFar	 = 0;
	
	pgpa((
		pgpaAddrValid(pattern, VoidAlign),
		pgpaAssert(patternLength > 0),
		pgpaAddrValid(buffer, VoidAlign),
		pgpaAssert(bufferLength >= 0)));
	
	/* copy the pattern once first */
	count = min(patternLength, bufferLength);
	pgpCopyMemory(pattern, buffer, count);
	remaining	 -= count;
	doneSoFar	 += count;
	
	while ( remaining != 0 )
	{
		/* replicate the pattern we have produced so far */
		/* because the original pattern might be small (say 1 byte) */
		count = min(doneSoFar, remaining);
		pgpCopyMemory(buffer, ((uchar *)buffer) + doneSoFar, count);
		
		remaining	-= count;
		doneSoFar	+= count;
	}
}

#if MACINTOSH	/* [ */

/*____________________________________________________________________________
Fill memory with a byte pattern. Obviously, this routine can be optimized.
____________________________________________________________________________*/
	
	static inline void
FillBytes(
	void *	buffer,
	size_t	length,
	uchar	 fillChar)
{
	char *	cur = (char *)buffer;
	UInt32	remaining = length;
	
	while (remaining-- > 0)
		*cur++ = fillChar;
}

/*____________________________________________________________________________
	Fast fill routine. Highly efficient for fills of as little as 16 bytes.
	
	Testing on PPC 604 shows that filling more than 4 bytes (one long) per loop
	iteration does not produce any measurable speed improvement. That fact,
	coupled with the fact that using byte fills, even on small chunks, is
	slow, means that unrolling it substantially is not a win.
____________________________________________________________________________*/
	void
pgpFillMemory(
	void *			buffer,
	const size_t	length,
	const uchar		fillChar)
{
	const UInt32	kAlignSize			= sizeof(UInt32);
	const UInt32	kUnrollBytesPerIter	= 8;	// must be power of 2
	uchar *			cur					= (uchar *)buffer;
	UInt32			remaining	 		 = length;
	
 pgpa((
		pgpaAddrValid(buffer, VoidAlign),
		pgpaAssert(length >= 0)));
	
	if ( length >= kUnrollBytesPerIter )
	{
		UInt32			fillPattern;
		UInt32			oddballCount;
		UInt32 *		curLong;
		UInt32			numChunks;
		
		pgpAssert( length >= kAlignSize );
		
		// create the fill pattern
		pgpAssert( sizeof(fillPattern) == 4 * sizeof(fillChar) );
		fillPattern = fillChar;
		fillPattern |= (fillPattern << 8);
		fillPattern |= (fillPattern << 16);
		
		// align address to kAlignSize byte boundary
		oddballCount	= kAlignSize - ( ((UInt32)cur) % kAlignSize );
		pgpAssert( oddballCount <= remaining );
		FillBytes( cur, oddballCount, fillChar );
		remaining	-= oddballCount;
		cur			+= oddballCount;
		pgpAssert( remaining == 0 || (((UInt32)cur) % kAlignSize) == 0 );
 	
		// fill all the chunks
		curLong		= (UInt32 *)cur;
		numChunks	= remaining / kUnrollBytesPerIter;
		while ( numChunks-- != 0 )
		{
			// CAUTION: this loop must move a total of
			// 'kUnrollBytesPerIter' bytes
			*curLong++	= fillPattern;
			*curLong++	= fillPattern;
		}
		
		// any remaining bytes get filled below...
		cur			= (Byte *)curLong;
 	remaining	&= ( kUnrollBytesPerIter - 1 );
	}
		
	FillBytes( cur, remaining, fillChar );
	
	// verify that at least first and last bytes have been set properly
 pgpAssert( length == 0 ||
		( ((Byte *)buffer)[ 0 ] ==
				fillChar && ((Byte *)buffer)[ length - 1 ] == fillChar) );
}

/*____________________________________________________________________________
	Return true if memory blocks are equal, false otherwise.
	
	Note: not yet optimized; this version is slow.
____________________________________________________________________________*/
	Boolean
pgpMemoryEqual(
	const void *	b1,
	const void *	b2,
	size_t			length)
{
	size_t			remaining	 = length;
	const uchar *	 cur1	= (uchar *)b1;
	const uchar *	 cur2	= (uchar *)b2;

	pgpa((
		pgpaAddrValid(b1, VoidAlign),
		pgpaAddrValid(b2, VoidAlign),
		pgpaAssert(length >= 0)));
	
	while (remaining-- > 0)
		if (*cur1++ != *cur2++)
			return FALSE;
	return TRUE;
}

#endif	/* ] MACINTOSH */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/

/*____________________________________________________________________________
	pgpMem.c
	
	Copyright (C) 1996,1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	pgpMemAlloc and friends with debugging support

	$Id: pgpMem.c,v 1.4 1999/03/10 02:52:57 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpPFLErrors.h"
#include "pgpDebug.h"

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

/*
 * ANSI-specific routines (for non-Macs)
 */


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

	static void
FillBytes(
	void *	buffer,
	size_t	length,
	uchar	fillChar)
{
	char *	cur = (char *)buffer;
	PGPUInt32	remaining = length;
	
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
	const PGPUInt32	kAlignSize			= sizeof(PGPUInt32);
	const PGPUInt32	kUnrollBytesPerIter	= 8;	/* must be power of 2 */
	uchar *			cur					= (uchar *)buffer;
	PGPUInt32		remaining			= length;
	
	pgpa((
		pgpaAddrValid(buffer, VoidAlign),
		pgpaAssert(length >= 0),
		pgpaAssert( length < 128 *1024UL * 1024UL )));
	
	if ( length >= kUnrollBytesPerIter )
	{
		PGPUInt32			fillPattern;
		PGPUInt32			oddballCount;
		PGPUInt32 *			curLong;
		PGPUInt32			numChunks;
		
		pgpAssert( length >= kAlignSize );
		
		/* create the fill pattern */
		pgpAssert( sizeof(fillPattern) == 4 * sizeof(fillChar) );
		fillPattern = fillChar;
		fillPattern |= (fillPattern << 8);
		fillPattern |= (fillPattern << 16);
		
		/* align address to kAlignSize PGPByte boundary */
		oddballCount	= kAlignSize - ( ((PGPUInt32)cur) % kAlignSize );
		pgpAssert( oddballCount <= remaining );
		FillBytes( cur, oddballCount, fillChar );
		remaining	-= oddballCount;
		cur			+= oddballCount;
		pgpAssert( remaining == 0 || (((PGPUInt32)cur) % kAlignSize) == 0 );
		
		/* fill all the chunks */
		curLong		= (PGPUInt32 *)cur;
		numChunks	= remaining / kUnrollBytesPerIter;
		while ( numChunks-- != 0 )
		{
			/* CAUTION:
				this loop must move a total of 'kUnrollBytesPerIter' bytes */
			*curLong++	= fillPattern;
			*curLong++	= fillPattern;
		}
		
		/* any remaining bytes get filled below... */
		cur			= (PGPByte *)curLong;
		remaining	&= ( kUnrollBytesPerIter - 1 );
	}
		
	FillBytes( cur, remaining, fillChar );
	
	/* verify that at least first and last bytes have been set properly */
	pgpAssert( length == 0 ||
		( ((PGPByte *)buffer)[ 0 ] == fillChar &&
		((PGPByte *)buffer)[ length - 1 ] == fillChar) );
}

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
		pgpaAddrValid(b2, VoidAlign),
		pgpaAssert(length >= 0)));
	
	while (remaining-- > 0)
		if (*cur1++ != *cur2++)
			return FALSE;
	return TRUE;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

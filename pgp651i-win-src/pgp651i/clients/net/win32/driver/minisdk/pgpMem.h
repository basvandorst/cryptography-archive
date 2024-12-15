/*____________________________________________________________________________
	pgpMem.h
	
	Copyright (C) 1996,1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Memory allocation routines with debugging support

	$Id: pgpMem.h,v 1.3 1999/03/10 02:52:59 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpMem_h	/* [ */
#define Included_pgpMem_h

#include "pgpTypes.h"
#include "pgpPFLErrors.h"
#include "pgpDebug.h"

#ifdef USERLAND_TEST
#if USERLAND_TEST
#include "testNDIS.h"
#else
#include <ndis.h>
#endif
#else
#include <ndis.h>
#endif

#ifndef IsNull
#define IsntNull( p )	( (PGPBoolean) ( (p) != NULL ) )
#define IsNull( p )		( (PGPBoolean) ( (p) == NULL ) )
#endif


PGP_BEGIN_C_DECLARATIONS
/*____________________________________________________________________________
 * There are five macros which control the debugging features of pgpmem.
 * Unless explicitly set, they default to the value of the PGP_DEBUG macro.
 *
 * NOTE: There's no need to set these explicitly for most purposes.
 *
 * DEBUG_FILL_MEM		If set, all allocated memory will be set to 0xDD
 *						before use, and all deallocated memory will be set
 *						to 0xDD before being freed.  This is also done when
 *						resizing blocks.
 * DEBUG_MEM_HEAD_MAGIC	If set, a ulong-byte magic number is placed immediately
 *						before each block, to detect buffer overruns.
 * DEBUG_MEM_TAIL_MAGIC	If set, magic numbers are placed after the end of each
 *						block to detect buffer overruns.  The value of the
 *						macro determines the number of bytes added.
 * PGP_DEBUG_FIND_LEAKS		If set, a list is kept of all allocated blocks 
 *						which is used to detect memory leaks on program exit.
 * DEBUG_ALWAYS_MOVE	If set, memory blocks are always moved when increasing
 *						the size of a block.
____________________________________________________________________________*/

#ifndef PGP_DEBUG_FIND_LEAKS
#define PGP_DEBUG_FIND_LEAKS	USE_PGP_LEAKS	/* Find memory leaks */
#endif

#define				pgpaMemBlockValid(userPtr)								\
					pgpaCall(pgpaInternalMemBlockValid,						\
								(pgpaCallPrefix, userPtr))
#define 			pgpAssertMemBlockValid( ptr ) 	\
						pgpa( pgpaMemBlockValid( ptr ) )


#define pgpCopyMemory(src, dest, size)	\
			NdisMoveMemory(dest, (IN PVOID) (src), size)

PGPBoolean 	pgpMemoryEqual(const void *b1, const void *b2,
										size_t length);

void 		pgpFillMemory(void *buffer, const size_t length,
										const uchar fillChar);

#define 			pgpClearMemory(buffer, byteCount)						\
					pgpFillMemory((buffer), (byteCount), '\0')

void 	pgpCopyPattern(const void *pattern, size_t patternLength,
									void *buffer, size_t bufferLength);

/* Lookie here!  An ANSI-compliant alignment finder! */
#ifndef 			alignof
#define 			alignof(type) (sizeof(struct{type _x; char _y;})		\
									- sizeof(type))
#endif

/*
 * WARNING: These should only be used by the above macros
 */

#if PGP_DEBUG
PGPBoolean 	pgpaInternalMemBlockValid(pgpaCallPrefixDef,
												const void *userPtr);
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpMem_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*
* pgpMem.h -- Memory allocation routines with debugging support
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpMem.h,v 1.9.2.3 1997/06/07 09:50:10 mhw Exp $
*/

#ifndef PGPMEM_H
#define PGPMEM_H

#include <stdlib.h>

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#include "pgpTypes.h"
#include "pgpErr.h"
#include "pgpDebug.h"
#include "pgpLeaks.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
* There are five macros which control the debugging features of pgpmem.
* Unless explicitly set, they default to the value of the DEBUG macro.
*
* NOTE: There's no need to set these explicitly for most purposes.
*
* DEBUG_FILL_MEM		If set, all allocated memory will be set to 0xDD
*		 		 		 before use, and all deallocated memory will be set
*		 		 		 to 0xDD before being freed. This is also done when
*		 		 		 resizing blocks.
* DEBUG_MEM_HEAD_MAGIC	If set, a ulong-byte magic number is placed immediately
*		 		 		 before each block, to detect buffer overruns.
* DEBUG_MEM_TAIL_MAGIC	If set, magic numbers are placed after the end of each
*		 		 		 block to detect buffer overruns. The value of the
*		 		 		 macro determines the number of bytes added.
* DEBUG_FIND_LEAKS		 If set, a list is kept of all allocated blocks which is
*		 		 		 used to detect memory leaks on program exit.
* DEBUG_ALWAYS_MOVE	 If set, memory blocks are always moved when increasing
*		 		 		 the size of a block.
*/

#ifndef DEBUG_FIND_LEAKS
#define DEBUG_FIND_LEAKS	USE_PGP_LEAKS	 /* Find memory leaks */
#endif

#if DEBUG_FIND_LEAKS

#define PGPALLOC_CONTEXT_PARAMS			, __FILE__, __LINE__
#define PGPALLOC_CONTEXT_PARAMS_DEF		, char const *fileName, long lineNumber
#define PGPALLOC_CONTEXT_PASS_PARAMS	, fileName, lineNumber
#define PGP_INTERNAL_ALLOC				pgpInternalContextAlloc
#define PGP_INTERNAL_MEMALLOC		 	pgpInternalContextMemAlloc
#define PGP_INTERNAL_MEMREALLOC			pgpInternalContextMemRealloc

#else

#define PGPALLOC_CONTEXT_PARAMS
#define PGPALLOC_CONTEXT_PARAMS_DEF
#define PGPALLOC_CONTEXT_PASS_PARAMS
#define PGP_INTERNAL_ALLOC				pgpInternalAlloc
#define PGP_INTERNAL_MEMALLOC		 	pgpInternalMemAlloc
#define PGP_INTERNAL_MEMREALLOC			pgpInternalMemRealloc

#endif

/*
* These functions won't return NULL for 0-sized blocks,
* and will fail assertions if NULL is passed in.
* pgpRealloc also has a different, cleaner calling convention.
*/
#define 			pgpAlloc(size)											\
					PGP_INTERNAL_ALLOC(size PGPALLOC_CONTEXT_PARAMS)
#define 			pgpNew(type)											\
					((type *)pgpAlloc(sizeof(type)))
PGPError PGPExport	pgpRealloc(void **userPtrRef, size_t newSize);
void PGPExport		pgpFree(void *userPtr);

/*
* These functions WILL return NULL for 0-sized blocks, and
* deal with NULL passed in as if it was a 0-sized block,
* for backward compatibility. They have the same semantics
* as malloc/realloc/free.
*/
#define 			pgpMemAlloc(size) 										\
					PGP_INTERNAL_MEMALLOC(size PGPALLOC_CONTEXT_PARAMS)
#define 			pgpMemNew(type)											\
					((type *)pgpMemAlloc(sizeof(type)))
#define 			pgpMemRealloc(userPtr, newSize) 						\
					PGP_INTERNAL_MEMREALLOC(userPtr, newSize				\
											PGPALLOC_CONTEXT_PARAMS)
void PGPExport		pgpMemFree(void *userPtr);

#define				pgpaMemBlockValid(userPtr)								\
					pgpaCall(pgpaInternalMemBlockValid,						\
								(pgpaCallPrefix, userPtr))

/* Memory copy routine optimized for large blocks, overlapping okay */
void PGPExport		pgpCopyMemory(void const *src, void *dest, size_t size);

#if MACINTOSH	/* [ */

Boolean PGPExport	pgpMemoryEqual(const void *b1, const void *b2,
						size_t length);
void PGPExport		pgpFillMemory(void *buffer, size_t length, uchar fillChar);

#else	/* ] MACINTOSH [ */

#define				pgpMemoryEqual(b1, b2, length)							\
					(memcmp((void *)(b1), (void *)(b2), (length)) == 0)
#define				pgpFillMemory(buffer, length, fillChar)					\
					((void)memset((void *)(buffer), (fillChar), (length)))

#endif	/* ] MACINTOSH */

#define 			pgpClearMemory(buffer, byteCount)	 		 		 	\
					pgpFillMemory((buffer), (byteCount), '\0')

void PGPExport		pgpCopyPattern(const void *pattern, size_t patternLength,
									void *buffer, size_t bufferLength);


/* Lookie here! An ANSI-compliant alignment finder! */
#ifndef 			alignof
#define 			alignof(type) (sizeof(struct{type _x; char _y;})		\
									- sizeof(type))
#endif

/*
* WARNING: These should only be used by the above macros
*/
void PGPExport *	PGP_INTERNAL_ALLOC(size_t size
										PGPALLOC_CONTEXT_PARAMS_DEF);
void PGPExport *	PGP_INTERNAL_MEMALLOC(size_t size
											PGPALLOC_CONTEXT_PARAMS_DEF);
void PGPExport *	PGP_INTERNAL_MEMREALLOC(void *userPtr, size_t newSize
											PGPALLOC_CONTEXT_PARAMS_DEF);

#if DEBUG
Boolean PGPExport	pgpaInternalMemBlockValid(pgpaCallPrefixDef,
												void *userPtr);
#endif

#ifdef __cplusplus
}
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

#endif /* PGPMEM_H */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/

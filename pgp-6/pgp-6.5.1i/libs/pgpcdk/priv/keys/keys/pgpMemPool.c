/*
 * pgpMemPool.c - Pooled memory allocation, similar to GNU obstacks.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpMemPool.c,v 1.9 1997/07/29 20:01:40 hal Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpMemPool.h"
#include "pgpMem.h"
#include "pgpContext.h"

#ifndef NULL
#define NULL 0
#endif

/* Define this symbol to cause secure wiping */
#ifndef SECURE
#define SECURE 1
#endif

/*
 * The memory pool allocation functions
 *
 * These are based on a linked list of memory blocks, usually of uniform
 * size.  New memory is allocated from the tail of the current block,
 * until that is inadequate, then a new block is allocated.
 * The entire pool can be freed at once by calling memPoolFree().
 */

typedef struct PoolBuf {
	struct PoolBuf *next;
	unsigned size;
	/* Data follows */
} PoolBuf;

/* Page size to use */
#ifdef MSDOS
#define ALLOC_UNIT 1024
#else
#define ALLOC_UNIT 4096
#endif

static MemPool const EmptyPool =
	{ NULL, NULL, NULL, 0, ALLOC_UNIT, 0, (int (*)(void *))NULL, NULL};


/*
 * Initialize the pool for first use
 */
void
memPoolInit(
	PGPContextRef	context,
	MemPool *		pool)
{
	*pool = EmptyPool;
	
	pool->context	= context;
}

/* Set the pool's purge function */
void
memPoolSetPurge(MemPool *pool, int (*purge)(void *), void *arg)
{
	pool->purge = purge;
	pool->purgearg = arg;
}

/*
 * Free all the memory in the pool
 */
void
memPoolEmpty(MemPool *pool)
{
	PoolBuf *buf;

	while ((buf = pool->head) != NULL) {
		pool->head = buf->next;
#if SECURE
		pgpClearMemory( buf,  buf->size);
#endif
		pgpContextMemFree( pool->context, buf);
	}
	pool->freespace = 0;
	pool->totalsize = 0;
}


/*
 * Restore a pool to a marked position, freeing subsequently allocated
 * memory.
 */
void
memPoolCutBack(MemPool *pool, MemPool const *cutback)
{
	PoolBuf *buf;

	pgpAssert(pool);
	pgpAssert(cutback);

	pgpAssert(pool->totalsize >= cutback->totalsize);

	while((buf = pool->head) != cutback->head) {
		pool->head = buf->next;
#if SECURE
		pgpClearMemory( buf,  buf->size);
#endif
		pgpContextMemFree( pool->context, buf );
	}
	*pool = *cutback;
}


/*
 * Structure and string allocations.
 * To make alingment work, strings and other objects
 * are allocated from different pools.
 */

/*
 * Allocate a chunk of memory for a structure, with the given alignment.
 * This is more space-efficient than malloc() for small objects
 * with loose alignment restrictions - like short strings.
 * Note that alignment is relative to the beginning of a chunk returned from
 * malloc(), which is guaranteed by ANSI to be as aligned as can possibly
 * matter.
 */
void *
memPoolAlloc(MemPool *pool, unsigned len, unsigned alignment)
{
	char *p;
	unsigned t;

	/* Where to allocate next object */
	p = pool->freeptr;
	/* How far it is from the beginning of the chunk. */
	t = p - (char *)pool->head;
	/*
	 * How much padding to add to freeptr to make alignment.  The first
	 * case (not a power of 2) will never happen, but we do it right
	 * for bragging rights in the ANSI C compliance department.
	 */
	if (alignment & (alignment-1))
		t = alignment-1 - (t-1)%alignment;	/* Never called */
	else
		t = -(int)t & (alignment-1);	/* This one always */

	/* Okay, does it fit? */
	if (pool->freespace >= len+t) {
		pool->freespace -= len+t;
		p += t;
		pool->freeptr = p + len;
		return p;
	}

	/* It does not fit in the current chunk.  Allocate a new chunk. */

	/* First, figure out how padding is needed after the header. */
	if (alignment & (alignment-1))
		alignment -= 1 + (sizeof(PoolBuf)-1) % alignment;
	else
		alignment = (alignment-1) & -(int)sizeof(PoolBuf);
	alignment += sizeof(PoolBuf);
	/* Then, figure out a chunk size that will fit */
	t = pool->chunksize;
	pgpAssert(t);
	while (len + alignment > t)
		t *= 2;
	while ((p = (char *)
		pgpContextMemAlloc( pool->context, t, 0)) == NULL)
	{
		/* If that didn't work, try purging or smaller allocations */
		if (!pool->purge || !pool->purge(pool->purgearg)) {
			if (len + alignment == t)
				return NULL;
			t /= 2;
			if (t < len+alignment)
				t = len+alignment;
		}
	}

	/* Update the various pointers. */
	pool->totalsize += t;
	((PoolBuf *)p)->next = pool->head;
	((PoolBuf *)p)->size = t;
	pool->head = (PoolBuf *)p;
	pool->freespace = t - len - alignment;
	p += alignment;
	pool->freeptr = p + len;

	return p;
}

#ifdef DEADCODE
/*
 * Store a string in the table, returning a pointer to the string.
 */
char const *
memPoolStore(MemPool *pool, char const *str)
{
	unsigned len = strlen(str) + 1;
	char *p;

	p = memPoolAlloc(pool, len, 1);
	if (p)
		memcpy(p, str, len);

	return p;
}
#endif

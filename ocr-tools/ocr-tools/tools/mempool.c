/*
 * mempool.c - Pooled memory allocation, similar to GNU obstacks.
 *
 * $Id: mempool.c,v 1.5 1997/11/13 23:53:08 colin Exp $
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>	/* For malloc() & free() */

#include "mempool.h"

/*
 * The memory pool allocation functions
 *
 * These are based on a linked list of memory blocks, usually of uniform
 * size.  New memory is allocated from the tail of the current block,
 * until that is inadequate, then a new block is allocated.
 * The entire pool can be freed at once by calling memPoolFree().
 */
struct PoolBuf {
	struct PoolBuf *next;
	unsigned size;
	/* Data follows */
};

/* The prototype empty pool, including the default allocation size. */
static struct MemPool EmptyPool = { 0, 0, 0, 4096, 0 , 0, 0};

/* Initialize the pool for first use */
void
memPoolInit(struct MemPool *pool)
{
	*pool = EmptyPool;
}

/* Set the pool's purge function */
void
memPoolSetPurge(struct MemPool *pool, int (*purge)(void *), void *arg)
{
	pool->purge = purge;
	pool->purgearg = arg;
}

/* Free all the memory in the pool */
void
memPoolEmpty(struct MemPool *pool)
{
	struct PoolBuf *buf;

	while ((buf = pool->head) != 0) {
		pool->head = buf->next;
		free(buf);
	}
	pool->freespace = 0;
	pool->totalsize = 0;
}


/*
 * Restore a pool to a marked position, freeing subsequently allocated
 * memory.
 */
void
memPoolCutBack(struct MemPool *pool, struct MemPool const *cutback)
{
	struct PoolBuf *buf;

	assert(pool);
	assert(cutback);
	assert(pool->totalsize >= cutback->totalsize);

	while((buf = pool->head) != cutback->head) {
		pool->head = buf->next;
		free(buf);
	}
	*pool = *cutback;
}

/*
 * Allocate a chunk of memory for a structure.  Alignment is assumed to be
 * a power of 2.  It could be generalized, if that ever becomes relevant.
 * Note that alignment is from the beginning of an allocated chunk, which
 * is guaranteed by ANSI to be as aligned as can possibly matter.
 */
void *
memPoolAlloc(struct MemPool *pool, unsigned len, unsigned alignment)
{
	char *p;
	unsigned t;

	/* Where to allocate next object */
	p = pool->freeptr;
	/* How far it is from the beginning of the chunk. */
	t = p - (char *)pool->head;
	/* How much to round up freeptr to make alignment */
	t = -t & --alignment;

	/* Okay, does it fit? */
	if (pool->freespace >= len+t) {
		pool->freespace -= len+t;
		p += t;
		pool->freeptr = p + len;
		return p;
	}

	/* It does not fit in the current chunk.  Go for a bigger chunk. */

	/* First, figure out how much to skip at the beginning of the chunk */
	alignment &= -(unsigned)sizeof(struct PoolBuf);
	alignment += sizeof(struct PoolBuf);
	/* Then, figure out a chunk size that will fit */
	t = pool->chunksize;
	assert(t);
	while (len + alignment > t)
		t *= 2;
	while ((p = malloc(t)) == 0) {
		/* If that didn't work, try purging or smaller allocations */
		if (!pool->purge || !pool->purge(pool->purgearg)) {
			t /= 2;
			if (len + alignment > t)
				fputs("Out of memory!\n", stderr);
				exit (1);	/* Failed */
		}
	}

	/* Update the various pointers. */
	pool->totalsize += t;
	((struct PoolBuf *)p)->next = pool->head;
	((struct PoolBuf *)p)->size = t;
	pool->head = (struct PoolBuf *)p;
	pool->freespace = t - len - alignment;
	p += alignment;
	pool->freeptr = p + len;

	return p;
}

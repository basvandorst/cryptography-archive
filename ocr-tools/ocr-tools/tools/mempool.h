/* $Id: mempool.h,v 1.2 1997/11/13 23:53:09 colin Exp $ */

#ifndef MEMPOOL_H
#define MEMPOOL_H

typedef struct MemPool {
	struct PoolBuf *head;
	char *freeptr;
	unsigned freespace;
	unsigned chunksize;	/* Default starting point */
	unsigned long totalsize;
	int (*purge)(void *);	/* Return non-zero to retry alloc */
	void *purgearg;
} MemPool;

/* A global pool for miscellaneous stuff. */
extern struct MemPool MiscPool;

/*
 * Nice clean interfaces
 */
void memPoolInit(struct MemPool *pool);
void memPoolSetPurge(struct MemPool *pool, int (*purge)(void *), void *arg);
void memPoolEmpty(struct MemPool *pool);
void memPoolCutBack(struct MemPool *dest, struct MemPool const *cutback);
void *memPoolAlloc(struct MemPool *pool, unsigned len, unsigned alignment);
#ifdef DEADCODE
char const *memPoolStore(struct MemPool *pool, char const *str);
#endif

/* Lookie here!  An ASNI-compliant alignment finder! */
#define alignof(type) (sizeof(struct{type _x; char _y;}) - sizeof(type))

#define memPoolNew(pool, type) memPoolAlloc(pool, sizeof(type), alignof(type))

#endif /* MEMPOOL_H */

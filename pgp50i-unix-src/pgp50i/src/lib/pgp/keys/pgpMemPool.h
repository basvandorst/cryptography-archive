/*
 * pgpMemPool.h
 *
 * Copyright (C) 1993-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpMemPool.h,v 1.3.2.1 1997/06/07 09:50:34 mhw Exp $
 */

#ifndef PGPMEMPOOL_H
#define PGPMEMPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

struct MemPool {
		struct PoolBuf *head;
		char *freeptr;
		unsigned freespace;
		unsigned chunksize;		/* Default starting point */
		unsigned long totalsize;
		int (*purge)(void *);		/*	Return non-zero to retry alloc */
		void *purgearg;
};
#ifndef TYPE_MEMPOOL
#define TYPE_MEMPOOL 1
typedef struct MemPool MemPool;
#endif

/*
 * Nice clean interfaces
 */
#define memPoolIsEmpty(pool) ((pool)->head == 0)

void PGPExport
memPoolInit(struct MemPool *pool);

void PGPExport
memPoolSetPurge(struct MemPool *pool, int (*purge)(void *), void *arg);

void PGPExport
memPoolEmpty(struct MemPool *pool);

void PGPExport
memPoolCutBack(struct MemPool *dest, struct MemPool const *cutback);

void PGPExport *
memPoolAlloc(struct MemPool *pool, unsigned len, unsigned alignment);

#ifdef DEADCODE
char const PGPExport *
memPoolStore(struct MemPool *pool, char const *str);
#endif

/*	Lookie here! An ASNI-compliant alignment finder! */
#define alignof(type) (sizeof(struct{type _x; char _y;}) - sizeof(type))

#define memPoolNew(pool, type) memPoolAlloc(pool, sizeof(type), alignof(type))

#ifdef __cplusplus
}
#endif

#endif /* PGPMEMPOOL_H */

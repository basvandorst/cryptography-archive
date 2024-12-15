/*
 * $Id: pgpMemPool.h,v 1.8 1997/06/21 00:25:35 lloyd Exp $
 */

#ifndef Included_pgpMemPool_h
#define Included_pgpMemPool_h


#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

struct MemPool
{
	PGPContextRef	context;
	
	struct PoolBuf *head;
	char *freeptr;
	unsigned freespace;
	unsigned chunksize;	/* Default starting point */
	unsigned long totalsize;
	int (*purge)(void *);	/* Return non-zero to retry alloc */
	void *purgearg;
};


/*
 * Nice clean interfaces
 */
#define memPoolIsEmpty(pool) ((pool)->head == 0)

void 
memPoolInit(PGPContextRef context, MemPool *pool);

void 
memPoolSetPurge(MemPool *pool, int (*purge)(void *), void *arg);

void 
memPoolEmpty(MemPool *pool);

void 
memPoolCutBack(MemPool *dest, MemPool const *cutback);

void  *
memPoolAlloc(MemPool *pool, unsigned len, unsigned alignment);

#ifdef DEADCODE
char const  *
memPoolStore(MemPool *pool, char const *str);
#endif

/* Lookie here!  An ASNI-compliant alignment finder! */
#define alignof(type) (sizeof(struct{type _x; char _y;}) - sizeof(type))

#define memPoolNew(pool, type) memPoolAlloc(pool, sizeof(type), alignof(type))

PGP_END_C_DECLARATIONS

#endif /* Included_pgpMemPool_h */

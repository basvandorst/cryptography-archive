/*
 * Operations on the usual buffers of bytes
 *
 * $Id: bnimem.h,v 1.3 1997/12/11 23:12:09 lloyd Exp $
 */
#ifndef BNSECURE
#define BNSECURE 1
#endif

#include "pgpMemoryMgr.h"

PGP_BEGIN_C_DECLARATIONS

/*
 * These operations act on buffers of memory, just like malloc & free.
 * One exception: it is not legal to pass a NULL pointer to bniMemFree.
 */

#ifndef bniMemAlloc

void *	bniMemAlloc( PGPMemoryMgrRef mgr,
			PGPBoolean secure, PGPUInt32 bytes);
#endif

#ifndef bniMemFree
void bniMemFree( void *ptr, PGPUInt32 bytes);
#endif

/* This wipes out a buffer of bytes if necessary needed. */

#ifndef bniMemWipe
#if BNSECURE
void bniMemWipe(void *ptr, unsigned bytes);
#else
#define bniMemWipe(ptr, bytes) (void)(ptr,bytes)
#endif
#endif /* !bniMemWipe */

/*
 * bniRealloc is NOT like realloc(); it's endian-sensitive!
 * If bniMemRealloc is #defined, bniRealloc will be defined in terms of it.
 * It is legal to pass a NULL pointer to bniRealloc, although oldbytes
 * will always be sero.
 */
#ifndef bniRealloc
void *bniRealloc(PGPMemoryMgrRef mgr, PGPBoolean secure,
		void *ptr, unsigned oldbytes, unsigned newbytes);
#endif

PGP_END_C_DECLARATIONS

/*
 * These macros are the ones actually used most often in the math library.
 * They take and return pointers to the *end* of the given buffer, and
 * take sizes in terms of words, not bytes.
 *
 * Note that BNIALLOC takes the pointer as an argument instead of returning
 * the value.
 *
 * Note also that these macros are only useable if you have included
 * bni.h (for the BIG and BIGLITTLE macros), which this file does NOT include.
 */

#define BNIALLOC( mgr, sc, p,type,words) BIGLITTLE( \
	if ( ((p) = (type *)bniMemAlloc( mgr, sc, (words)*sizeof*(p))) != 0) \
		(p) += (words), \
	(p) = (type *)bniMemAlloc( mgr, sc, (words) * sizeof*(p)) \
	)
#define BNIFREE(p,words) bniMemFree((p) BIG(-(words)), (words) * sizeof*(p))
#define BNIREALLOC( mgr, sc, p,old,newP) \
	bniRealloc( mgr, sc, p, (old) * sizeof*(p), (newP) * sizeof*(p))
#define BNIWIPE(p,words) bniMemWipe((p) BIG(-(words)), (words) * sizeof*(p))


/*
* bnimem.h - Operations on the usual buffers of bytes
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: bnimem.h,v 1.4.2.1 1997/06/07 09:49:36 mhw Exp $
*/
#ifndef BNSECURE
#define BNSECURE 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
* These operations act on buffers of memory, just like malloc & free.
* One exception: it is not legal to pass a NULL pointer to bniMemFree.
*/

#ifndef bniMemAlloc
void *bniMemAlloc(unsigned bytes);
#endif

#ifndef bniMemFree
void bniMemFree(void *ptr, unsigned bytes);
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
void *bniRealloc(void *ptr, unsigned oldbytes, unsigned newbytes);
#endif

#ifdef __cplusplus
}
#endif

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

#define BNIALLOC(p,type,words) BIGLITTLE( \
	if ( ((p) = (type *)bniMemAlloc((words)*sizeof*(p))) != 0) \
	(p) += (words), \
	(p) = (type *)bniMemAlloc((words) * sizeof*(p)) \
	)
#define BNIFREE(p,words) bniMemFree((p) BIG(-(words)), (words) * sizeof*(p))
#define BNIREALLOC(p,old,new) \
	bniRealloc(p, (old) * sizeof*(p), (new) * sizeof*(p))
#define BNIWIPE(p,words) bniMemWipe((p) BIG(-(words)), (words) * sizeof*(p))

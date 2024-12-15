/*
 * lbnmem.c - low-level bignum memory handling.
 *
 * Copyright (c) 1995  Colin Plumb.  All rights reserved.
 * For licensing and other legal details, see the file legal.c.
 *
 * Note that in all cases, the pointers passed around
 * are pointers to the *least* significant end of the word.
 * On big-endian machines, these are pointers to the *end*
 * of the allocated range.
 *
 * BNSECURE is a simple level of security; for more security
 * change these function to use locked unswappable memory.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>	/* For malloc() & co. */
#else
void *malloc();
void free();
#endif

#if HAVE_STRING_H
#include <string.h>	/* For memset */
#elif HAVE_STRINGS_H
#include <strings.h>
#endif
#if NEED_MEMORY_H
#include <memory.h>
#endif

#if DBMALLOC
/* Development debugging */
#include "../dbmalloc/malloc.h"
#endif

#include "lbn.h"
#include "lbnmem.h"

#include "kludge.h"

#ifndef lbnMemWipe
void
lbnMemWipe(void *ptr, unsigned bytes)
{
	memset(ptr, 0, bytes);
}
#define lbnMemWipe(ptr, bytes) memset(ptr, 0, bytes)
#endif

#ifndef lbnMemAlloc
void *
lbnMemAlloc(unsigned bytes)
{
	return malloc(bytes);
}
#define lbnMemAlloc(bytes) malloc(bytes)
#endif

#ifndef lbnMemFree
void
lbnMemFree(void *ptr, unsigned bytes)
{
	lbnMemWipe(ptr, bytes);
	free(ptr);
}
#endif

#ifndef lbnRealloc
#if defined(lbnMemRealloc) || !BNSECURE
void *
lbnRealloc(void *ptr, unsigned oldbytes, unsigned newbytes)
{
	if (ptr) {
		BIG(ptr = (char *)ptr - oldbytes;)
		if (newbytes < oldbytes)
			memmove(ptr, (char *)ptr + oldbytes-newbytes, oldbytes);
	}
#ifdef lbnMemRealloc
	ptr = lbnMemRealloc(ptr, oldbytes, newbytes);
#else
	ptr = realloc(ptr, newbytes);
#endif
	if (ptr) {
		if (newbytes > oldbytes)
			memmove((char *)ptr + newbytes-oldbytes, ptr, oldbytes);
		BIG(ptr = (char *)ptr + newbytes;)
	}

	return ptr;
}

#else /* BNSECURE */

void *
lbnRealloc(void *ptr, unsigned oldbytes, unsigned newbytes)
{
	void *new = lbnMemAlloc(newbytes);

	if (!new)
		return new;
	if (!ptr)
		return BIGLITTLE((char *)new+newbytes, new);

	/*
	 * The following copies are a bit non-obvious in the big-endian case
	 * because one of the pointers points to the *end* of allocated memory.
	 */
	if (newbytes > oldbytes) {	/* Copy all of old into part of new */
		BIG(new = (char *)new + newbytes;)
		BIG(ptr = (char *)ptr - oldbytes;)
		memcpy(BIGLITTLE((char *)new-oldbytes, new), ptr, oldbytes);
	} else {	/* Copy part of old into all of new */
		memcpy(new, BIGLITTLE((char *)ptr-newbytes, ptr), newbytes);
		BIG(new = (char *)new + newbytes;)
		BIG(ptr = (char *)ptr - oldbytes;)
	}

	lbnMemFree(ptr, oldbytes);
	return new;
}
#endif /* BNSECURE */
#endif /* !lbnRealloc */

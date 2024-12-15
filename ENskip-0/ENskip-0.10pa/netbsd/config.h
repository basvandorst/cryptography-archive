/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
/*
 * All system depending ressources are defined as a macro in this file
 */

#include <sys/types.h>

/*
 * some functions to convert values from network byte order and inverse
 */
typedef int int32;
typedef u_int u_int32;
typedef u_short u_int16;
typedef u_char int8;

#include <sys/param.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define __LITTLE_ENDIAN__
#else
#define __BIG_ENDIAN__
#endif
#define SKIP_HTONL(x)		htonl((x))
#define SKIP_NTOHL(x)		ntohl((x))
#define SKIP_HTONS(x)		htons((x))
#define SKIP_NTOHS(x)		ntohs((x))

#ifdef KERNEL
#include <sys/malloc.h>

#define KALLOC(size)		(void *)malloc((size), M_TEMP, M_NOWAIT)
#define KFREE(ptr, size)	FREE((ptr), M_TEMP); (ptr) = (void *)0

#include <sys/systm.h>

/* MEMCMP should return 0, if areas match */
#define MEMCMP(a, b, size)	bcmp((a), (b), (size))
#define MEMCPY(to, from, size)	bcopy((from), (to), (size))
#define MEMZERO(a, size)	bzero((a), (size))

#include <sys/kernel.h>

#define TIMEOUT(func, arg, sec)	timeout((void *)(func), (arg), (hz * (sec)))
#define UNTIMEOUT(func, arg)	untimeout((void *)(func), (arg))

#define SEMTYPE		        int
#define SEMALLOC(s)    
#define SEMINIT(s)		
#define SEMLOCK(s)		(s) = splsched()
#define SEMUNLOCK(s)		splx((s))
/*
 * SEMLOCKTRY should try to lock the semaphore
 * if the semaphore is lock 0 is returned.
 */
#define SEMLOCKTRY(s)		(SEMLOCK(s), 0)
#define SEMFREE(s)		

#include <sys/vmmeter.h>
#define SEEDTYPE                struct vmmeter
#define SEEDINIT(s)             MEMCPY((s), &cnt, sizeof(*(s)))
#endif /* KERNEL */

/*
 * type definition for queues in cache
 */
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>

typedef struct ifqueue QUEUE;

extern int queue_free(void *x);
extern int queue_feed(void *x);
#define SKIP_FREEQ(x)		queue_free((x))
#define SKIP_FEEDQ(x)		queue_feed((x))

#ifndef NULL
#define NULL	(void *)0
#endif /* NULL */

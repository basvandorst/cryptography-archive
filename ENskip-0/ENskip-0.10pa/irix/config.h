/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
/*
 * All system depending ressources are defined as a macro in this file
 */

#ifdef KERNEL
#define _KERNEL 1
#endif /* KERNEL */

#include <sys/types.h>

/*
 * some functions to convert values from network byte order and inverse
 */
#include <sys/param.h>

typedef int int32;
typedef u_int u_int32;
typedef u_short u_int16;
typedef u_char u_int8;

#include <sys/endian.h>

#define SKIP_HTONL(x)		htonl(x)
#define SKIP_NTOHL(x)		SKIP_HTONL((x))
#define SKIP_HTONS(x)		htons(x)
#define SKIP_NTOHS(x)		SKIP_HTONS((x))


#ifdef KERNEL

#include <sys/kmem.h>

#define KALLOC(size)		kmem_alloc((size), KM_SLEEP)
#define KFREE(ptr, size)	kmem_free((ptr), (size)); (ptr) = (void *)0

#include <sys/systm.h>

/* MEMCMP should return 0, if areas match */
#define MEMCMP(a, b, size)	bcmp((caddr_t)(a), (caddr_t)(b), (size))
#define MEMCPY(to, from, size)	bcopy((caddr_t)(from), (caddr_t)(to), (size))
#define MEMZERO(a, size)	bzero((caddr_t)(a), (size))

#include <sys/ddi.h>
static int timeout_id;
#define TIMEOUT(func, arg, sec)	timeout_id = timeout((void *)(func), (arg), drv_usectohz(1000000 * (sec)))
#define UNTIMEOUT(func, arg)	untimeout(timeout_id)


#include <sys/ksynch.h>

#define SEMTYPE			sleep_t *
#define SEMALLOC(s)		(s) = SLEEP_ALLOC(0, (void *)-1, KM_SLEEP)
#define SEMINIT(s)		
#define SEMLOCK(s)		SLEEP_LOCK(s, PZERO)
#define SEMUNLOCK(s)		SLEEP_UNLOCK(s)
/*
 * SEMLOCKTRY should try to lock the semaphore
 * if the semaphore is locked 0 is returned.
 */
#define SEMLOCKTRY(s)		!SLEEP_TRYLOCK(s)
#define SEMFREE(s)		SEMLOCK(s); SEMUNLOCK(s); SLEEP_DEALLOC(s)

#include <sys/sysinfo.h>
#include <sys/ksa.h>
extern struct ksa ksa;
#define SEEDTYPE		struct ksa
#define SEEDINIT(s)		MEMCPY((s), &ksa, sizeof(*(s)))
#endif /* KERNEL */

/*
 * type definition for queues in cache
 */
#include <sys/stream.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>

typedef struct ifqueue QUEUE;

extern int queue_free(void *x);
extern int queue_feed(void *x);
#define SKIP_FREEQ(x)		  queue_free((x))
#define SKIP_FEEDQ(x)		  queue_feed((x))

#ifndef NULL
#define NULL	(void *)0
#endif /* NULL */

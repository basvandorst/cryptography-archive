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

/* Get __LITTLE_ENDIAN_ or __BIG_ENDIAN__ */
#include <sys/byteorder.h>

#ifdef _LITTLE_ENDIAN
#define SKIP_HTONL(x)		((((x) >> 24) & 0xFF) | \
                                 (((x) >> 8) & 0xFF00) | \
                                 (((x) << 8) & 0xFF0000) | \
				 (((x) << 24)))
#define SKIP_NTOHL(x)		SKIP_HTONL((x))
#define SKIP_HTONS(x)		((((x) >> 8) & 0xFF) | (((x) << 8)))
#define SKIP_NTOHS(x)		SKIP_HTONS((x))
#else
#define SKIP_HTONL(x) (x)
#define SKIP_NTOHL(x) (x)
#define SKIP_HTONS(x) (x)
#define SKIP_NTOHS(x) (x)
#endif /* _LITTLE_ENDIAN */


#ifdef KERNEL

#include <sys/kmem.h>
#include <sys/kmem_impl.h>

#define KALLOC(size)		kmem_alloc((size), KM_SLEEP)
#define KFREE(ptr, size)	kmem_free((ptr), (size)); (ptr) = (void *)0

#include <sys/systm.h>

/* MEMCMP should return 0, if areas match */
#define MEMCMP(a, b, size)	bcmp((caddr_t)(a), (caddr_t)(b), (size))
#define MEMCPY(to, from, size)	bcopy((caddr_t)(from), (caddr_t)(to), (size))
#define MEMZERO(a, size)	bzero((caddr_t)(a), (size))

#include <sys/ddi.h>
int timeout_id;
#define TIMEOUT(func, arg, sec)	timeout_id = timeout((void *)(func), (arg), drv_usectohz(1000000 * (sec)))
#define UNTIMEOUT(func, arg)	untimeout(timeout_id)


#include <sys/semaphore.h>

#define SEMTYPE			ksema_t *
#define SEMALLOC(s)		(s) = (SEMTYPE)KALLOC(sizeof(*s))
#define SEMINIT(s)		sema_init((s), 1, "skip", SEMA_DRIVER, NULL)
#define SEMLOCK(s)		sema_p(s)
#define SEMUNLOCK(s)		sema_v(s)
/*
 * SEMLOCKTRY should try to lock the semaphore
 * if the semaphore is lock 0 is returned.
 */
#define SEMLOCKTRY(s)		!sema_tryp((s))
#define SEMFREE(s)		sema_p(s); sema_destroy(s); KFREE((s), sizeof(*s));

#include <sys/cpuvar.h>
#undef u  /* Remove annoying define */
#define SEEDTYPE		cpu_t
#define SEEDINIT(s)		MEMCPY(s, cpu_list, sizeof(*s))
#endif /* KERNEL */

/*
 * type definition for queues in cache
 */
#include <sys/stream.h>

typedef struct skip_ifq
{
  mblk_t *ifq_head, *ifq_tail;
  int ifq_len;
  int ifq_maxlen;
} QUEUE;

extern int queue_free(void *c);
extern int queue_feed(void *c);

#define SKIP_FREEQ(x)		  queue_free((x))
#define SKIP_FEEDQ(x)		  queue_feed((x))

#ifndef NULL
#define NULL	(void *)0
#endif /* NULL */

/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
/*
 * All system depending ressources are defined as a macro in this file
 */

#ifdef NeXT

#include <sys/types.h>
#include <netinet/in.h>
/*
 * some functions to convert values from network byte order and inverse
 */

typedef int int32;
typedef u_int u_int32;
typedef u_short u_int16;
typedef u_char u_int8;

/* __BIG_ENDIAN__ __LITTLE_ENDIAN__ are predefined by NeXT cc */
#define SKIP_HTONL(x)		htonl((x))
#define SKIP_NTOHL(x)		ntohl((x))
#define SKIP_HTONS(x)		htons((x))
#define SKIP_NTOHS(x)		ntohs((x))

#ifdef KERNEL
#include <kernserv/kalloc.h>

#define KALLOC(size)		kalloc((size))
#define KFREE(ptr, size)	kfree((ptr), (size))

#include <kernserv/prototypes.h>

/* prototype */
extern int bcmp(void *a, void *b, unsigned int len);

/* MEMCMP should return 0, if areas match */
#define MEMCMP(a, b, size)	bcmp((a), (b), (size))
#define MEMCPY(to, from, size)	bcopy((from), (to), (size))
#define MEMZERO(a, size)	bzero((a), (size))

#include <sys/callout.h>
#include <kernserv/ns_timer.h>

#define TIMEOUT(func, arg, sec)	ns_timeout((func), (arg), (1000000000ULL * (sec)), CALLOUT_PRI_SOFTINT0)
#define UNTIMEOUT(func, arg)	ns_untimeout((func), (arg))

#ifdef WHITE_NEXT  /* Not defined... */
/*
 * these semaphores do not work black hardware
 */
#include <kernserv/lock.h>

#define SEMTYPE			lock_t
#define SEMALLOC(s)		(s) = lock_alloc()
#define SEMINIT(s)		lock_init((s), FALSE)
#define SEMLOCK(s)		lock_write((s))
#define SEMUNLOCK(s)		lock_done((s))
/*
 * SEMLOCKTRY should try to lock the semaphore
 * if the semaphore is lock 0 is returned.
 */
#define SEMLOCKTRY(s)		!lock_try_write((s))
#define SEMFREE(s)		lock_free((s))
#else
#include <kernserv/machine/spl.h>

#define SEMTYPE                 int
#define SEMALLOC(s)    
#define SEMINIT(s)              
#define SEMLOCK(s)              s = spl6()
#define SEMUNLOCK(s)            splx((s))
/*
 * SEMLOCKTRY should try to lock the semaphore
 * if the semaphore is lock 0 is returned.
 */
#define SEMLOCKTRY(s)           (SEMLOCK(s), 0)
#define SEMFREE(s)              SEMUNLOCK(s)
#endif
/*
 * type definition for queues in cache
 */
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
typedef struct ifqueue QUEUE;

#define SKIP_MFREE(m)	if (((m)->m_off == MMINOFF) || \
                            ((m)->m_cltype != MCL_LOANED)) \
			  nb_free((netbuf_t)(m)); \
			else \
			  nb_free_wrapper((netbuf_t)(m));
			

extern int queue_free(void *c);
extern int queue_feed(void *c);

#define SKIP_FREEQ(x)		queue_free((x))
#define SKIP_FEEDQ(x)		queue_feed((x))

#include <sys/vmmeter.h>
#define SEEDTYPE                struct vmmeter
#define SEEDINIT(s)             MEMCPY((s), &cnt, sizeof(*(s)))

#endif KERNEL

#ifndef NULL
#define NULL	(void *)0
#endif /* NULL */

#endif /* NeXT */

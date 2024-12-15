/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
/*
 * All system depending ressources are defined as a macro in this file
 */

#include <sys/types.h>
#include <stdio.h>

/* XXX: System-dependent */
typedef int int32;
typedef u_int u_int32;
typedef u_short u_int16;
typedef u_char u_int8;

#include <stdlib.h>

#define KALLOC(size)		malloc((size))
#define KFREE(ptr, size)	free((ptr))

/* MEMCMP should return 0, if areas match */
#include <string.h>
#define MEMCMP(a, b, size)	memcmp((caddr_t)(a), (caddr_t)(b), (size))
#define MEMCPY(to, from, size)	memcpy((caddr_t)(to), (caddr_t)(from), (size))
#define MEMZERO(a, size)	memset((caddr_t)(a), '\0', (size))

#include <signal.h>
#ifdef NeXT
#include <libc.h>
#else
#include <unistd.h>
#endif

static int (*to_func)(void *);
static void *to_arg;

static void sig_hdl(int sig)
{
  signal(sig, sig_hdl);
  if (to_func)
    to_func(to_arg);
}

#define TIMEOUT(func, arg, sec)	to_func = (func), to_arg = (arg), signal(SIGALRM, sig_hdl), alarm((sec))
#define UNTIMEOUT(func, arg)	to_func = NULL, alarm(0)

/* no semaphores at user level */
#define SEMTYPE			int
#define SEMALLOC(s)		
#define SEMINIT(s)		s = 0;  /* Avoid compiler warnings */
#define SEMLOCK(s)		
#define SEMUNLOCK(s)		
/*
 * SEMLOCKTRY should try to lock the semaphore
 * if the semaphore is lock 0 is returned.
 */
#define SEMLOCKTRY(s)		0
#define SEMFREE(s)		

#include <sys/time.h>
#define SEEDTYPE		struct timeval
#define SEEDINIT(s)		gettimeofday((s), NULL)

#define SKIP_FREEQ(x) 
#define SKIP_FEEDQ(x)

#ifndef NULL
#define NULL	(void *)0
#endif /* NULL */

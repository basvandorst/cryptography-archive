/*
 * pgpThreads.c
 *
 * $Id: pgpThreads.c,v 1.3.28.1 1999/06/04 01:23:40 heller Exp $
 */

#include <stdio.h>
#include "pgpThreads.h"

#if PGP_UNIX_SOLARIS

int
PGPThreadAttrCreate( PGPThreadAttr_t *attr )
{
	*attr = 0;
	return( 0 );
}

int
PGPThreadAttrDestroy( PGPThreadAttr_t *attr )
{
	*attr = 0;
	return( 0 );
}

int
PGPThreadAttrGetDetachState( PGPThreadAttr_t *attr, int *detachstate )
{
	*detachstate = *attr;
	return( 0 );
}

int
PGPThreadAttrSetDetachState( PGPThreadAttr_t *attr, int detachstate )
{
	*attr = detachstate;
	return( 0 );
}

/* ARGSUSED */
int
PGPThreadCreate(
	PGPThread_t		*tid,
	PGPThreadAttr_t	*attr,
	VFP			func,
	void		*arg
	)
{
	return( thr_create( NULL, 0, func, arg, *attr, tid ) );
}

void
PGPThreadYield()
{
	thr_yield();
}

void
PGPThreadExit(PGPThread_t tid)
{
	(void) tid;
	thr_exit( NULL );
}

void
PGPThreadJoin( PGPThread_t tid, int *status )
{
	thr_join( tid, NULL, (void **) status );
}

void
PGPThreadKill( PGPThread_t tid, int sig )
{
	thr_kill( tid, sig );
}

int
PGPThreadKeyCreate(PGPThreadKey_t *key, void (*destructor)(void* value))
{
	return thr_keycreate(key, destructor);
}

int
PGPThreadGetSpecific(PGPThreadKey_t key, void **value)
{
	return thr_getspecific(key, value);
}

int
PGPThreadSetSpecific(PGPThreadKey_t key, void *value)
{
	return thr_setspecific(key, value);
}

int
PGPThreadKeyDelete(PGPThreadKey_t key)
{
	(void) key;
	return 0;
}

#endif /* end PGP_UNIX_SOLARIS */

#if HAVE_PTHREAD_CREATE

int
PGPThreadAttrCreate( PGPThreadAttr_t *attr )
{
	return pthread_attr_init(attr);
}

int
PGPThreadAttrDestroy( PGPThreadAttr_t *attr )
{
	return pthread_attr_destroy(attr);
}

int
PGPThreadAttrGetDetachState( PGPThreadAttr_t *attr, int *detachstate )
{
	return pthread_attr_getdetachstate(attr, detachstate);
}

int
PGPThreadAttrSetDetachState( PGPThreadAttr_t *attr, int detachstate )
{
	return pthread_attr_setdetachstate(attr, detachstate);
}

/* ARGSUSED */
int
PGPThreadCreate(
	PGPThread_t		*tid,
	PGPThreadAttr_t	*attr,
	VFP			func,
	void		*arg
	)
{
	return( pthread_create( tid, attr, func, arg) );
}

void
PGPThreadYield()
{
	return;
}

void
PGPThreadExit(PGPThread_t tid)
{
	(void) tid;
	pthread_exit( NULL );
}

void
PGPThreadJoin( PGPThread_t tid, int *status )
{
	pthread_join( tid, (void **) status );
}

void
PGPThreadKill( PGPThread_t tid, int sig )
{
	pthread_kill( tid, sig );
}

int
PGPThreadKeyCreate(PGPThreadKey_t *key, void (*destructor)(void* value))
{
	return pthread_key_create(key, destructor);
}

int
PGPThreadGetSpecific(PGPThreadKey_t key, void **value)
{
	*value = pthread_getspecific(key);
	return (int)*value;
}

int
PGPThreadSetSpecific(PGPThreadKey_t key, void *value)
{
	return pthread_setspecific(key, value);
}

int
PGPThreadKeyDelete(PGPThreadKey_t key)
{
	return pthread_key_delete(key);
}
#endif /* HAVE_PTREAD_CREATE */

#if HAVE_THREAD_ATTR_CREATE

/***********************************************************************
 *																	 *
 * pthreads package with DCE - no mapping to do (except to create a	*
 * pthread_kill() routine)											 *
 *																	 *
 ***********************************************************************/

/* ARGSUSED */
void
PGPThreadKill( pthread_t tid, int sig )
{
	kill( getpid(), sig );
}

#endif /* HAVE_THREAD_ATTR_CREATE (DCE THREADS) */

#if PGP_WIN32

#include <process.h>
#include <windows.h>

int
PGPThreadAttrCreate( PGPThreadAttr_t *attr )
{
	*attr = 0;
	return( 0 );
}

int
PGPThreadAttrDestroy( PGPThreadAttr_t *attr )
{
	return( 0 );
}

int
PGPThreadAttrGetDetachState( PGPThreadAttr_t *attr, int *detachstate )
{
	*detachstate = *attr;
	return( 0 );
}

int
PGPThreadAttrSetDetachState( PGPThreadAttr_t *attr, int detachstate )
{
	*attr = detachstate;
	return( 0 );
}

/* ARGSUSED */
int
PGPThreadCreate(
	PGPThread_t		*tid,
	PGPThreadAttr_t	*attr,
	VFP			func,
	void		*arg
	)
{
	unsigned hack;

	*tid = (HANDLE) _beginthreadex(NULL, 0, func, arg, 0, &hack);
/*	tid = CreateThread(NULL, 0, func, arg, 0, &threadID); */
	
	return !(*tid);
}

void
PGPThreadYield()
{
	return;
}

void
PGPThreadExit(PGPThread_t tid)
{
	_endthreadex(0);
	CloseHandle(tid);
}

void
PGPThreadJoin( PGPThread_t tid, int *status )
{
	int rc;
	
	rc = WaitForSingleObject((HANDLE) tid, INFINITE);
	
	return;
}

/* ARGSUSED */
void
PGPTKill( PGPThread_t tid, int sig )
{
	/* !!! Don't know how to kill a thread created with beginthread. */
	/* Seems to only be done when the server is coming down due to error. */
	return;
}

int
PGPThreadKeyCreate(PGPThreadKey_t *key, void (*destructor)(void* value))
{
	*key = TlsAlloc();
	if (*key == 0xFFFFFFFF)
		return -1;
	else
		return 0;
}

int
PGPThreadGetSpecific(PGPThreadKey_t key, void **value)
{
	*value = TlsGetValue(key);
	if (*value)
		return 0;
	else
		return -1;
}

int
PGPThreadSetSpecific(PGPThreadKey_t key, void *value)
{
	return !(TlsSetValue(key, value));
}

int
PGPThreadKeyDelete(PGPThreadKey_t key)
{
	return !(TlsFree(key));
}

#endif /* PGP_WIN32 */

#ifndef _PGP_THREAD

/* ARGSUSED */
int
PGPThreadAttrCreate( PGPThreadAttr_t *attr )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPThreadAttrDestroy( PGPThreadAttr_t *attr )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPThreadAttrGetDetachState( PGPThreadAttr_t *attr, int *detachstate )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPThreadAttrSetDetachState( PGPThreadAttr_t *attr, int detachstate )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPThreadCreate(
	PGPThread_t		*tid,
	PGPThreadAttr_t	*attr,
	VFP			func,
	void		*arg
	)
{
	(*func)( arg );
	
	return( 0 );
}

void
PGPThreadYield()
{
	return;
}

void
PGPThreadExit(PGPThread_t tid)
{
	return;
}

/* ARGSUSED */
void
PGPThreadKill( PGPThread_t tid, int sig )
{
	return;
}

void
PGPThreadJoin( PGPThread_t tid, int *status )
{
	return;
}
int
PGPThreadKeyCreate(PGPThreadKey_t *key, void (*destructor)(void* value))
{
	return 0;
}

int
PGPThreadGetSpecific(PGPThreadKey_t key, void **value)
{
	return 0;
}

int
PGPThreadSetSpecific(PGPThreadKey_t key, void *value)
{
	return 0;
}

int
PGPThreadKeyDelete(PGPThreadKey_t key)
{
	return 0;
}

#endif /* no threads package */
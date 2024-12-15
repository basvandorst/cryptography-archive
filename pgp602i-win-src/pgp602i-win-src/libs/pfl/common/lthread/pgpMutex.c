/*
 * pgpMutex.c
 *
 * $Id: pgpMutex.c,v 1.2 1998/03/18 05:08:16 elowe Exp $
 */

#include <stdio.h>
#include "pgpThreads.h"

#if PGP_UNIX_SOLARIS

/* ARGSUSED */
int
PGPMutexCreate( PGPMutex_t *mp, PGPMutexAttr_t *attr )
{
	return( mutex_init( mp, attr ? *attr : USYNC_THREAD, NULL ) );
}

int
PGPMutexDestroy( PGPMutex_t *mp )
{
	return( mutex_destroy( mp ) );
}

int
PGPMutexLock( PGPMutex_t *mp )
{
	return( mutex_lock( mp ) );
}

int
PGPMutexUnlock( PGPMutex_t *mp )
{
	return( mutex_unlock( mp ) );
}

int
PGPMutexTryLock( PGPMutex_t *mp )
{
	return( mutex_trylock( mp ) );
}
#endif /* PGP_UNIX_SOLARIS */

#if HAVE_PTHREAD_CREATE

/* ARGSUSED */
int
PGPMutexCreate( PGPMutex_t *mp, PGPMutexAttr_t *attr )
{
	return( pthread_mutex_init(mp, attr) );
}

int
PGPMutexDestroy( PGPMutex_t *mp )
{
	return( pthread_mutex_destroy( mp ) );
}

int
PGPMutexLock( PGPMutex_t *mp )
{
	return( pthread_mutex_lock( mp ) );
}

int
PGPMutexUnlock( PGPMutex_t *mp )
{
	return( pthread_mutex_unlock( mp ) );
}

int
PGPMutexTryLock( PGPMutex_t *mp )
{
	return( pthread_mutex_trylock( mp ) );
}

#endif /* HAVE_PTREAD_CREATE */

#if PGP_WIN32

#include <process.h>
#include <windows.h>

/* ARGSUSED */
int
PGPMutexCreate( PGPMutex_t *mp, PGPMutexAttr_t *attr )
{

	*mp = CreateMutex(NULL, FALSE, NULL);
	
	return( *mp == NULL );
}

int
PGPMutexDestroy( PGPMutex_t *mp )
{
	int	lRC = CloseHandle(*mp);
	return(lRC == 0);
}

int
PGPMutexLock( PGPMutex_t *mp )
{
	int	lRC = WaitForSingleObject(*mp, INFINITE);
	
	return(lRC == WAIT_FAILED);
}

int
PGPMutexUnlock( PGPMutex_t *mp )
{
	int	lRC = ReleaseMutex(*mp);
	return(lRC == 0);
}

int
PGPMutexTryLock( PGPMutex_t *mp )
{
	int	lRC = WaitForSingleObject(*mp, 0);
	
	return(lRC == WAIT_FAILED);
}

#endif /* PGP_WIN32 */

#ifndef _PGP_THREAD

/* ARGSUSED */
int
PGPMutexCreate( PGPMutex_t *mp, PGPMutexAttr_t *attr )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPMutexDestroy( PGPMutex_t *mp )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPMutexLock( PGPMutex_t *mp )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPMutexUnlock( PGPMutex_t *mp )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPMutexTryLock( PGPMutex_t *mp )
{
	return( 0 );
}

#endif /* no thread package */

/*
 * pgpCond.c
 *
 * $Id: pgpCond.c,v 1.3 1998/07/31 18:14:34 marcd Exp $
 *
 * Implements conditional variable stuff
 *
 * copyright (C) 1998 Network Associates Inc.
 */

#include <stdio.h>
#include "pgpThreads.h"

#if PGP_UNIX_SOLARIS
/*
 * Oh goodie, we have solaris, which is chalk full of useful routines.
 * Simply call various system functions.
 */
int
PGPCondCreate( PGPCond_t *cv, PGPCondAttr_t *attr )
{
	return( cond_init( cv, attr ? *attr : USYNC_THREAD, NULL ) );
}

int
PGPCondDestroy( PGPCond_t *cv )
{
	return( cond_destroy( cv ) );
}

int
PGPCondWait( PGPCond_t *cv, PGPMutex_t *mp )
{
	return( cond_wait( cv, mp ) );
}

int
PGPCondSignal( PGPCond_t *cv )
{
	return( cond_signal( cv ) );
}

int
PGPCondBroadcast( PGPCond_t *cv )
{
	return( cond_broadcast( cv ) );
}
#endif /* PGP_UNIX_SOLARIS */

#if HAVE_PTHREAD_CREATE
/*
 * We have pthreads support, so call the pthreads functions
 */

int
PGPCondCreate( PGPCond_t *cv, PGPCondAttr_t *attr )
{
	return( pthread_cond_init( cv, attr ) );
}

int
PGPCondDestroy( PGPCond_t *cv )
{
	return( pthread_cond_destroy( cv ) );
}

int
PGPCondWait( PGPCond_t *cv, PGPMutex_t *mp )
{
	return( pthread_cond_wait( cv, mp ) );
}

int
PGPCondSignal( PGPCond_t *cv )
{
	return( pthread_cond_signal( cv ) );
}

int
PGPCondBroadcast( PGPCond_t *cv )
{
	return( pthread_cond_broadcast( cv ) );
}

#endif /* HAVE_PTREAD_CREATE */

#if PGP_WIN32
/*
 * Windows 32 stuff. Conditionals are called Events. See comment below
 * about broadcast problems.
 */
#include <process.h>
#include <windows.h>

int
PGPCondCreate( PGPCond_t *cv, PGPCondAttr_t *attr )
{

	*cv = CreateEvent(NULL, 0, 0, NULL);

	return(*cv == NULL);
}

/* init2 creates an event object that is manually reset (which means that */
/*   broadcast signals can be done). */
int
PGPCondCreate2( PGPCond_t *cv, PGPCondAttr_t *attr )
{

	*cv = CreateEvent(NULL, 1, 0, NULL);

	return(*cv == NULL);
}

int
PGPCondDestroy( PGPCond_t *cv )
{
	int	lRC;

	lRC = CloseHandle(*cv);
	return(lRC == 0);
}

int
PGPCondWait( PGPCond_t *cv, PGPMutex_t *mp )
{
	int	lRC;
	int	lRC2;

	lRC = ReleaseMutex(*mp);
	if (lRC == 0)
		return 1;

	lRC2 = WaitForSingleObject(*cv, INFINITE);

	lRC = WaitForSingleObject(*mp, INFINITE);

	return(lRC == WAIT_FAILED || lRC2 == WAIT_FAILED);
}

int
PGPCondSignal( PGPCond_t *cv )
{
	int	lRC;

	lRC = SetEvent(*cv);
	return(lRC == 0);
}

int
PGPCondBroadcast( PGPCond_t *cv )
{
	int	lRC;

	/* In order for broadcast to work under Win32, you need to call
	 * cond_init2 when creating the cond variable.  Note that
	 * regular signaling won't work if this is done. WARNING!!!!
	 * This does not do a broadcast.  A single event semaphore does
	 * not seem to be able to do both a broadcast and a
	 * PGPCondSignal.  May need to create 2 events: one manual reset
	 * one auto reset and have the cond_wait wait on both then pulse
	 * the manual reset event for a broadcast.
	 */
	lRC = PulseEvent(*cv);
	return(lRC == 0);
}
#endif /* PGP_WIN32 */

#ifndef _PGP_THREAD
/*
 * Hummm, no thread support. This probably isn't a good thing
 */

/* ARGSUSED */
int
PGPCondCreate( PGPCond_t *cv, PGPCondAttr_t *attr )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPCondDestroy( PGPCond_t *cv )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPCondWait( PGPCond_t *cv, PGPMutex_t *mp )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPCondSignal( PGPCond_t *cv )
{
	return( 0 );
}

/* ARGSUSED */
int
PGPCondBroadcast( PGPCond_t *cv )
{
	return( 0 );
}

#endif /* no threads package */

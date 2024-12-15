/*____________________________________________________________________________
Copyright (C) 1997 Network Associates Inc. and affiliated companies.
All rights reserved.
	
	
	
$Id: pgpRMWOLock.c,v 1.2 1999/03/10 02:54:42 heller Exp $
____________________________________________________________________________*/

#include "pgpBase.h"

#include "pgpRMWOLock.h"

#if PGP_UNIX_SOLARIS
/*
 * Use the Solaris system read/write locks. This is slightly different
 * from the default implementation: In determining who can proceed
 * from blocking, Solaris is random with a bias toward writers. The
 * default implementation has readers and writers proceeding in
 * alternating phases.
 */
void
InitializePGPRMWOLock(PGPRMWOLock * inLock)
{
	(void) rwlock_init(inLock, USYNC_THREAD, NULL);
}

void
DeletePGPRMWOLock(PGPRMWOLock * inLock)
{
	(void) rwlock_destroy(inLock);
}

void
PGPRMWOLockStartReading(PGPRMWOLock * inLock)
{
	(void) rw_rdlock(inLock);
}

void
PGPRMWOLockStopReading(PGPRMWOLock * inLock)
{
	(void) rw_unlock(inLock);
}

void
PGPRMWOLockStartWriting(PGPRMWOLock * inLock)
{
	(void) rw_wrlock(inLock);
}

void
PGPRMWOLockStopWriting(PGPRMWOLock * inLock)
{
	(void) rw_unlock(inLock);
}

#else /* everything else */

void
InitializePGPRMWOLock(PGPRMWOLock *	inLock)
{
	(void) PGPMutexCreate(&(inLock->mutex), PGPMutexAttr_default);
	(void) PGPSemCreate(&(inLock->blockedReaders), PGPSemAttr_default);
	(void) PGPSemCreate(&(inLock->blockedWriters), PGPSemAttr_default);
	
	inLock->activeReaders = 0;
	inLock->waitingReader = 0;
	inLock->activeWriters = 0;
	inLock->waitingWriter = 0;
}

void
DeletePGPRMWOLock(PGPRMWOLock *	inLock)
{	
	(void) PGPMutexDestroy(&(inLock->mutex));
	(void) PGPSemDestroy(&(inLock->blockedReaders));
	(void) PGPSemDestroy(&(inLock->blockedWriters));
}

void
PGPRMWOLockStartReading(PGPRMWOLock *	inLock)
{
	PGPMutexLock(&(inLock->mutex));
	
	if ((inLock->activeWriters > 0) || (inLock->waitingWriter > 0)) {
		inLock->waitingReader++;
		PGPMutexUnlock(&(inLock->mutex));
		/*
		 * we need to wait for any active people to finish before
		 * we can read, so we wait for blockedReaders to fire
		 */
		PGPSemWait(&(inLock->blockedReaders));
	} else {
		inLock->activeReaders++;
		PGPMutexUnlock(&(inLock->mutex));
	}
}

void
PGPRMWOLockStopReading(PGPRMWOLock *	inLock)
{
	PGPMutexLock(&(inLock->mutex));
	inLock->activeReaders--;
	
	if ((inLock->activeReaders == 0) && (inLock->waitingWriter > 0)) {
		inLock->activeWriters = 1;
		inLock->waitingWriter--;
		/*
		 * I've stopped reading, there aren't anymore readers, but there
		 * are some writers, so I need to fire blockedWriters to let
		 * those writers do their thing.
		 */
		PGPSemPost(&(inLock->blockedWriters));
	}
	PGPMutexUnlock(&(inLock->mutex));
}

void
PGPRMWOLockStartWriting(PGPRMWOLock *	inLock)
{
	PGPMutexLock(&(inLock->mutex));
	
	if ((inLock->activeReaders == 0) && (inLock->activeWriters == 0)) {
		inLock->activeWriters = 1;
		PGPMutexUnlock(&(inLock->mutex));
	} else {
		inLock->waitingWriter++;
		PGPMutexUnlock(&(inLock->mutex));
		/*
		 * we need to wait for any active people to finish before
		 * we can write, so we wait for blockedWriters to fire
		 */
		PGPSemWait(&(inLock->blockedWriters));
	}
}

void
PGPRMWOLockStopWriting(PGPRMWOLock *	inLock)
{
	PGPMutexLock(&(inLock->mutex));
	inLock->activeWriters = 0;

	/* waiting readers go before waiting writers */
	if (inLock->waitingReader > 0) {
		while (inLock->waitingReader > 0) {
			inLock->waitingReader--;
			inLock->activeReaders++;
			/* there are some readers, so let 'em go */
			PGPSemPost(&(inLock->blockedReaders));
		}
	} else if (inLock->waitingWriter > 0) {
		inLock->waitingWriter--;
		inLock->activeWriters = 1;
		/* there are some writers, let 'em go */
		PGPSemPost(&(inLock->blockedWriters));
	}
	PGPMutexUnlock(&(inLock->mutex));
}

#endif 
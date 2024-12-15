/*
 * pgpSemaphore.c
 *
 * $Id: pgpSemaphore.c,v 1.2 1998/10/23 18:01:12 elowe Exp $
 */

#include <stdio.h>
#include "pgpThreads.h"

#if PGP_UNIX_SOLARIS

PGPSemAttr_t PGPSemAttr_def = {0,USYNC_THREAD,0};

int
PGPSemCreate(PGPSem_t *sem, PGPSemAttr_t *attr)
{
	return sema_init(sem, attr->count, attr->type, attr->arg);
}

int
PGPSemWait(PGPSem_t *sem)
{
	return sema_wait(sem);
}

int PGPSemTryWait(PGPSem_t *sem)
{
	return sema_trywait(sem);
}

int PGPSemPost(PGPSem_t *sem)
{
	return sema_post(sem);
}

int PGPSemDestroy(PGPSem_t *sem)
{
	return sema_destroy(sem);
}

#endif /* end PGP_UNIX_SOLARIS */

/* Linux has sem_init function but no headers ?? */
#if HAVE_SEM_INIT && !PGP_UNIX_LINUX

PGPSemAttr_t PGPSemAttr_def = {0, 0};

int
PGPSemCreate(PGPSem_t *sem, PGPSemAttr_t *attr)
{
	return sem_init(sem, attr->shared, attr->value);
}

int
PGPSemWait(PGPSem_t *sem)
{
	return sem_wait(sem);
}

int PGPSemTryWait(PGPSem_t *sem)
{
	return sem_trywait(sem);
}

int PGPSemPost(PGPSem_t *sem)
{
	return sem_post(sem);
}

int PGPSemDestroy(PGPSem_t *sem)
{
	return sem_destroy(sem);
}

#endif /* HAVE_SEM_INIT */

#if HAVE_SEMGET

PGPSemAttr_t PGPSemAttr_def = {1, 0};

int
PGPSemCreate(PGPSem_t *sem, PGPSemAttr_t *attr)
{
	*sem = semget(IPC_PRIVATE, attr->nsems, attr->type);
	return *sem;
}

int
PGPSemWait(PGPSem_t *sem)
{
	struct sembuf sops;

	sops.sem_num = 0;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	
	return semop(*sem, &sops, 1);
}

int PGPSemTryWait(PGPSem_t *sem)
{
	struct sembuf sops;

	sops.sem_num = 0;
	sops.sem_op = -1;
	sops.sem_flg = IPC_NOWAIT;
	
	return semop(*sem, &sops, 1);
}

int PGPSemPost(PGPSem_t *sem)
{
	struct sembuf sops;

	sops.sem_num = 0;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	
	return semop(*sem, &sops, 1);
}

int PGPSemDestroy(PGPSem_t *sem)
{
	return semctl(*sem, 0, IPC_RMID, 0);
}

#endif /* HAVE_SEMGET */

#if PGP_WIN32

#if USERLAND_TEST

#include <process.h>
#include <windows.h>

PGPSemAttr_t PGPSemAttr_def = {0, 32, 0};

int
PGPSemCreate(PGPSem_t *sem, PGPSemAttr_t *attr)
{
	*sem = CreateSemaphore(NULL, attr->icount, attr->max, attr->name);
	return (int) *sem;
}

int
PGPSemWait(PGPSem_t *sem)
{
	int lRC = WaitForSingleObject(*sem, INFINITE);
	
	return (lRC == WAIT_FAILED);
}

int PGPSemTryWait(PGPSem_t *sem)
{
	int lRC = WaitForSingleObject(*sem, 0);
	
	return (lRC == WAIT_FAILED);
}

int PGPSemPost(PGPSem_t *sem)
{
	return ReleaseSemaphore(*sem, 1, NULL);
}

int PGPSemDestroy(PGPSem_t *sem)
{
	return CloseHandle(*sem);
}

#else /* USERLAND_TEST */

#include <ndis.h>

int
PGPSemCreate(PGPSem_t *sem, PGPSemAttr_t *attr)
{
	NdisAllocateSpinLock(sem);
	
	return 0;
}

int
PGPSemWait(PGPSem_t *sem)
{
	NdisAcquireSpinLock(sem);
	
	return 0;
}

int
PGPSemPost(PGPSem_t *sem)
{
	NdisReleaseSpinLock(sem);
	
	return 0;
}

int
PGPSemDestroy(PGPSem_t *sem)
{
	NdisFreeSpinLock(sem);
	
	return 0;
}

#endif /* USERLAND_TEST */

#endif /* PGP_WIN32 */

#ifndef _PGP_SEMAPHORE

int
PGPSemCreate(PGPSem_t *sem, PGPSemAttr_t *attr)
{
	return 0;
}

int
PGPSemWait(PGPSem_t *sem)
{
	return 0;
}

int PGPSemTryWait(PGPSem_t *sem)
{
	return 0;
}

int PGPSemPost(PGPSem_t *sem)
{
	return 0;
}

int PGPSemDestroy(PGPSem_t *sem)
{
	return 0;
}

#endif /* no semaphore package */

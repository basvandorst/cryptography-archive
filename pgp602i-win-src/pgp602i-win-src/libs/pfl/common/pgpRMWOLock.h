/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: pgpRMWOLock.h,v 1.8.14.1 1998/11/12 03:18:02 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpRMWOLock_h	/* [ */
#define Included_pgpRMWOLock_h

#if PGP_WIN32
# include <windows.h>
#endif

#include "pgpTypes.h"
#include "pgpThreads.h"

PGP_BEGIN_C_DECLARATIONS

/* These files implement a Read Many/Write Once lock. To use, simply
initialize a lock for the resource and surround your reading and writing of it
with the appropriate calls. */

#if PGP_UNIX_SOLARIS	/* solaris has a system impl. of this */
 typedef rwlock_t PGPRMWOLock;
#elif PGP_MACINTOSH
	typedef char PGPRMWOLock;
#else
 typedef struct PGPRMWOLock {
	PGPMutex_t	mutex;
	PGPSem_t	blockedReaders;
	PGPSem_t	blockedWriters;
	PGPUInt16	activeReaders;
	PGPUInt16	waitingReader;
	PGPUInt16	activeWriters;
	PGPUInt16	waitingWriter;
 } PGPRMWOLock;
#endif

#if PGP_MACINTOSH

# define InitializePGPRMWOLock(x)
# define DeletePGPRMWOLock(x)
# define PGPRMWOLockStartReading(x)
# define PGPRMWOLockStopReading(x)
# define PGPRMWOLockStartWriting(x)
# define PGPRMWOLockStopWriting(x)

#else

void	InitializePGPRMWOLock(PGPRMWOLock * inLock);
void	DeletePGPRMWOLock(PGPRMWOLock * inLock);
void	PGPRMWOLockStartReading(PGPRMWOLock * inLock);
void	PGPRMWOLockStopReading(PGPRMWOLock * inLock);
void	PGPRMWOLockStartWriting(PGPRMWOLock * inLock);
void	PGPRMWOLockStopWriting(PGPRMWOLock * inLock);

#endif /* PGP_MACINTOSH */

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpRMWOLock_h */

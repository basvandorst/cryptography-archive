/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Defines CPGPlockable, a simple C++ wrapper class around PGPMutex

	$Id: pgpLockable.h,v 1.5 1998/10/01 00:50:21 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpLockable_h
#define _Included_pgpLockable_h
	
#include "pgpTypes.h"
#include "pgpThreads.h"

class CPGPlockable
{
private:
	PGPMutex_t m_mutex;
	
	//private and undefined copy and assignment
	CPGPlockable(const CPGPlockable&);
	CPGPlockable & operator=(const CPGPlockable&);

public:
	CPGPlockable()	{ (void) PGPMutexCreate(&m_mutex, PGPMutexAttr_default); }
	~CPGPlockable()	{ (void) PGPMutexDestroy(&m_mutex); }
	
	void lock()			{ (void) PGPMutexLock(&m_mutex); }
	void unlock()		{ (void) PGPMutexUnlock(&m_mutex); }
	PGPBoolean test()	{ return PGPMutexTryLock(&m_mutex); }
};

#endif	// _Included_pgpLockable_h



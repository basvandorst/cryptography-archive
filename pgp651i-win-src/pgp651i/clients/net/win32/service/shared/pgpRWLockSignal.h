/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Defines CPGPrwLockSignal, a simple C++ wrapper class around
	PGPRMWOLock, and PGPCond

	$Id: pgpRWLockSignal.h,v 1.4 1999/04/02 03:47:42 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpRWLockSignal_h
#define _Included_pgpRWLockSignal_h
	
#include "pgpTypes.h"
#include "pgpThreads.h"
#include "pgpRMWOLock.h"

#ifdef _DEBUG
#define CPGPrwLockSignal CRW // 255 char limit problem with debugger
#endif

template <class T>
class CPGPrwLockSignal
{
private:
	PGPRMWOLock	m_rwlock;
	PGPCond_t 	m_event;
	T			m_data;
	
	//private and undefined copy and assignment
	CPGPrwLockSignal(const CPGPrwLockSignal &);
	CPGPrwLockSignal & operator=(const CPGPrwLockSignal &);

public:
	CPGPrwLockSignal(PGPBoolean manual=TRUE) {
		InitializePGPRMWOLock(&m_rwlock);
		if (manual)
			PGPCondCreate2(&m_event, 0);
		else
			PGPCondCreate(&m_event, 0);
	}
	
	~CPGPrwLockSignal()	{
		DeletePGPRMWOLock(&m_rwlock);
		PGPCondDestroy(&m_event);
	}
	
	void startReading()	{ PGPRMWOLockStartReading(&m_rwlock); }
	void stopReading()	{ PGPRMWOLockStopReading(&m_rwlock); }
	void startWriting() { PGPRMWOLockStartWriting(&m_rwlock); }
	void stopWriting()	{ PGPRMWOLockStopWriting(&m_rwlock); }

	void broadcast()	{ PGPCondBroadcast(&m_event); }
	void signal() 		{ PGPCondSignal(&m_event); }
	void setEvent()		{ SetEvent(m_event); }
	void resetEvent()	{ ResetEvent(m_event); }

	// access
	const PGPCond_t event() { return m_event; };
	T & data() { return m_data; };
};

#endif	// _Included_pgpRWLockSignal_h



/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetWorker.h,v 1.9 1999/04/13 19:26:55 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetWorker_h
#define _Included_pgpNetWorker_h

#include "pgpTypes.h"
#include "pgpThreads.h"

class CPGPnetWorker {
private:
	PGPThread_t m_threadID;

	// private and undefined copy and assignment
	CPGPnetWorker(const CPGPnetWorker &);
	CPGPnetWorker & operator=(const CPGPnetWorker &);

protected:
	//PGPBoolean	isWAN();

	virtual unsigned int Run();
	static unsigned int __stdcall StartProc(void* pv);
	
public:
	CPGPnetWorker();
	virtual ~CPGPnetWorker();

	void Go();

	// access functions
	PGPThread_t threadID()	{ return m_threadID; };
};

#endif //  _Included_pgpNetWorker_h

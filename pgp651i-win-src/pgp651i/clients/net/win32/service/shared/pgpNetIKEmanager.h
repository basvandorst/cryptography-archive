/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.
	
	$Id: pgpNetIKEmanager.h,v 1.11.6.1 1999/06/14 03:22:38 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetIKEmanager_h
#define _Included_pgpNetIKEmanager_h

#include "PGPtypes.h"

// forward declarations
class CPGPnetWorker;

//
// notes: only ever need one GUI worker, but might need multiple
// IKE and kernel workers. Definately need more than one kernel worker
//
class CPGPnetIKEmanager {
private:
	CPGPnetWorker	**m_workers;
	PGPInt32		m_numWorkers;
	PGPInt32		m_numKernelWorkers;
	SOCKET			m_socket;

	void		initializeWorkers(const char *displayName, HINSTANCE hInstance);
	PGPInt32	initializeWSA();
	void		reportOutOfMemory(PGPInt32 nMadeIt);

	// private and undefined copy and assignment
	CPGPnetIKEmanager(const CPGPnetIKEmanager&);
	CPGPnetIKEmanager & operator=(const CPGPnetIKEmanager &);
	
public:
	CPGPnetIKEmanager(const char *displayName, HINSTANCE hInstance);
	CPGPnetIKEmanager(PGPInt32 nKernelWorkers, const char *displayName, HINSTANCE hInstance);
	~CPGPnetIKEmanager();
	
	void Go();
	void GoResume();
	void WaitForWorkerFinish();
	void WaitForSuspendFinish();
};

#endif //_Included_pgpNetIKEmanager_h

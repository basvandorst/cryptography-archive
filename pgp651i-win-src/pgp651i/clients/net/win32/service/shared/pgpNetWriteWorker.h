/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetWriteWorker.h,v 1.3 1998/10/07 02:10:20 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetWriteWorker_h
#define _Included_pgpNetWriteWorker_h

#include "pgpNetWorker.h"

class CPGPnetWriteWorker : public CPGPnetWorker {
private:
	SOCKET m_socket;
	WSAEVENT m_writeEvent;

	// private and undefined copy and assignment
	CPGPnetWriteWorker(const CPGPnetWriteWorker &);
	CPGPnetWriteWorker & operator=(const CPGPnetWriteWorker &);	

	void processQueue();

protected:
	virtual unsigned int Run();
	
public:
	CPGPnetWriteWorker(SOCKET s);
	virtual ~CPGPnetWriteWorker();
};

#endif // _Included_pgpNetWriteWorker_h

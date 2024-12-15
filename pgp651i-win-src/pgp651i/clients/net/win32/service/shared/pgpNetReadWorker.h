/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetReadWorker.h,v 1.5 1999/01/28 23:36:25 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetReadWorker_h
#define _Included_pgpNetReadWorker_h

#include "pgpNetWorker.h"

class CPGPnetReadWorker : public CPGPnetWorker {
private:
	SOCKET		m_socket;
	PGPByte		*m_pBuffer;
	PGPUInt32	m_size;
	HWND		m_hWnd;

	// private and undefined copy and assignment
	CPGPnetReadWorker(const CPGPnetReadWorker &);
	CPGPnetReadWorker & operator=(const CPGPnetReadWorker &);

	PGPInt32	registerClass(HINSTANCE hinstance);
	PGPBoolean	createMyWindow(HINSTANCE hinstance);
	void		readUDP();

protected:
	virtual unsigned int Run();
	
public:
	CPGPnetReadWorker(SOCKET s);
	virtual ~CPGPnetReadWorker();

	static LRESULT CALLBACK winProc(HWND, UINT, WPARAM, LPARAM);
};

#endif // _Included_pgpNetReadWorker_h

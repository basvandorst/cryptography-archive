/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetGUIWorker.h,v 1.17 1998/11/25 19:09:57 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetGUIWorker_h
#define _Included_pgpNetGUIWorker_h

#include <string>

#include "pgpNetWorker.h"
#include "pgpNetDSAarray.h"


class CPGPnetGUIWorker : public CPGPnetWorker {
private:
	HWND			m_hWnd;
	std::string		m_winName;
	PGPBoolean		m_bEnableStatus;

	// private functions
	LRESULT		processGUIEvent(WPARAM wParam, LPARAM lParam);
	LRESULT		processCopyData(WPARAM wParam, LPARAM lParam);
	void		processQueue();
	PGPInt32	registerClass(HINSTANCE hinstance);
	PGPBoolean	createMyWindow(HINSTANCE hinstance);
	void		sendFullStatus();
	void		sendToApp(WPARAM wParam, LPARAM lParam = 0);
	void		sendToApp(PGPUInt32 dwData, 
		PGPUInt32 cbData, 
		const void *lpData);

	// private and undefined copy and assignment
	CPGPnetGUIWorker(const CPGPnetGUIWorker &);
	CPGPnetGUIWorker & operator=(const CPGPnetGUIWorker &);
	
protected:
	virtual unsigned int Run();
	
public:
	CPGPnetGUIWorker();
	virtual ~CPGPnetGUIWorker();

	// static window proc function
	static LRESULT CALLBACK winProc(HWND, UINT, WPARAM, LPARAM);
};

#endif // _Included_pgpNetGUIWorker_h

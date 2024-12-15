/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Definition of the CPGPnetMainWorker class

	$Id: pgpNetMainWorker.h,v 1.3 1999/05/05 19:19:49 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetMainWorker_h
#define _Included_pgpNetMainWorker_h

#include <string>

#include "pgpNetWorker.h"

class CPGPnetMainWorker : public CPGPnetWorker
{
private:
	HWND 				m_hWnd;
	HINSTANCE 			m_hInstance;
	std::string			m_shell;
	std::string			m_currentUser;
	std::string			m_displayName;
	HKEY				m_hKey;
	PGPCond_t			m_logonEvent;
	PGPBoolean			m_bInteractiveUser;
	UINT				m_uReloadKeyringMsg;

	// private functions
	PGPInt32 	registerClass(HINSTANCE hinstance);
	PGPBoolean 	createMyWindow(HINSTANCE hinstance);
	PGPBoolean 	isInteractiveShellRunning();
	void		processRegistryChange(HKEY hKey, HANDLE hEvent);

	// private and undefined copy and assignment
	CPGPnetMainWorker(const CPGPnetMainWorker &);
	CPGPnetMainWorker & operator=(const CPGPnetMainWorker &);

protected:
	virtual unsigned int Run();

public:
	CPGPnetMainWorker(const char*, HINSTANCE);
	virtual ~CPGPnetMainWorker();
	
	// access functions
	HWND hWnd()								{ return m_hWnd; };
	const char * const currentUser()		{ return m_currentUser.c_str(); };
	void interactiveUser(PGPBoolean b)		{ m_bInteractiveUser = b; };
	UINT uReloadKeyringMsg()				{ return m_uReloadKeyringMsg; };

	// static window proc function
	static LRESULT CALLBACK winProc(HWND, UINT, WPARAM, LPARAM);
};

#endif // _Included_pgpNetMainWorker_h

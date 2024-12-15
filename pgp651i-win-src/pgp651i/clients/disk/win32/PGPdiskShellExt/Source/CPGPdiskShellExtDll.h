//////////////////////////////////////////////////////////////////////////////
// CPGPdiskShellExtDll.h
//
// Declarations for CPGPdiskShellExtDll.cpp
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskShellExtDll.h,v 1.3 1998/12/14 19:00:42 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskShellExtDll_h // [
#define Included_CPGPdiskShellExtDll_h

#include "DualErr.h"


////////////////////////////
// Class CPGPdiskShellExtDll
////////////////////////////

class CPGPdiskShellExtDll : public CWinApp
{
public:
	PGPUInt32 mGCRefThisDll;

	CPGPdiskShellExtDll();
	~CPGPdiskShellExtDll();

	//{{AFX_VIRTUAL(CPGPdiskShellExtApp)
	public:
	virtual int ExitInstance();
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPGPdiskShellExtApp)
	//}}AFX_MSG

private:
	DualErr CheckWindowsVersion();

	DECLARE_MESSAGE_MAP()
};


/////////////////////
// Exported functions
/////////////////////

STDAPI	DllCanUnloadNow();
STDAPI	DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut);

#endif	// ] Included_CPGPdiskShellExtDll_h

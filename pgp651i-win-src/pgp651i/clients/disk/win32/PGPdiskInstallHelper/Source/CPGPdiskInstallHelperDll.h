//////////////////////////////////////////////////////////////////////////////
// CPGPdiskInstallHelperDll.h
//
// Contains declarations for CPGPdiskInstallHelperDll.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskInstallHelperDll.h,v 1.3 1998/12/14 19:00:33 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskInstallHelper_h	// [
#define Included_CPGPdiskInstallHelper_h


/////////////////////////////////
// Class CPGPdiskInstallHelperDll
/////////////////////////////////

class CPGPdiskInstallHelperDll : public CWinApp
{
public:
	CPGPdiskInstallHelperDll();
	~CPGPdiskInstallHelperDll();

	//{{AFX_VIRTUAL(CPGPdiskInstallHelperApp)
	public:
	virtual int ExitInstance();
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPGPdiskInstallHelperApp)
	//}}AFX_MSG

private:
	DualErr CheckWindowsVersion();

	DECLARE_MESSAGE_MAP()
};


/////////////////////
// Exported functions
/////////////////////

extern "C" void __stdcall		KillPGPdiskResident();
extern "C" void __stdcall		PGPdiskCleanup();
extern "C" BOOLEAN __stdcall	AreAnyPGPdisksMounted();
extern "C" BOOLEAN __stdcall	IsVolumeAPGPdisk(int drive);
extern "C" BOOLEAN __stdcall	UnmountAllPGPdisks();

//{{AFX_INSERT_LOCATION}}

#endif // Included_CPGPdiskInstallHelper_h

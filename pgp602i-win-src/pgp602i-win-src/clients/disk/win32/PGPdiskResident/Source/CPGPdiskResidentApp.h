//////////////////////////////////////////////////////////////////////////////
// CPGPdiskResidentApp.h
//
// Declaration of class CPGPdiskResidentApp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskResidentApp.h,v 1.2.2.3 1998/07/06 08:58:36 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskResidentApp_h // [
#define Included_CPGPdiskResidentApp_h

#include "DualErr.h"
#include "SharedMemory.h"
#include "Win32Utils.h"

#include "CHiddenWindow.h"


////////////////////////////
// Class CPGPdiskResidentApp
////////////////////////////

class CPGPdiskResidentApp : public CWinApp
{
public:
	CHiddenWindow	*mHiddenWindow;
	CString			mHiddenWindowClass;
	PGPdiskResInfo	*mPGPdiskResInfo;

			CPGPdiskResidentApp();

	void	AbortPGPdiskResApp();

	//{{AFX_VIRTUAL(CPGPdiskResidentApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPGPdiskResidentApp)
	//}}AFX_MSG

private:
	SharedMemory mResInfoSharedMem;

	void	CheckWindowsVersion();
	void	EnforceUniquePGPdiskResApp();

	DualErr	CreateHiddenWindow();
	void	DeleteHiddenWindow();

	DualErr	CreatePGPdiskResInfo();
	DualErr	DeletePGPdiskResInfo();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // Included_CPGPdiskResidentApp_h

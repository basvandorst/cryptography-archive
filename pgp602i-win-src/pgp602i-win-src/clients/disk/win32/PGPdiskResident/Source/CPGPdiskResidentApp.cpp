//////////////////////////////////////////////////////////////////////////////
// CPGPdiskResidentApp.cpp
//
// Implementation of class CPGPdiskResidentApp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskResidentApp.cpp,v 1.2.2.7 1998/07/30 00:50:27 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "DriverComm.h"
#include "DualErr.h"
#include "LinkResolution.h"
#include "PGPdiskResidentDefines.h"
#include "StringAssociation.h"
#include "WindowsVersion.h"

#include "CPGPdiskResidentApp.h"


///////////////////////////
// Global variables for MFC
///////////////////////////

CPGPdiskResidentApp theApp;		// the one and only application


///////////////////////////////////////
// Global variables for the application
///////////////////////////////////////

CPGPdiskResidentApp *App;			// pointer to application object


///////////////////////////
// MFC specific definitions
///////////////////////////

// MFC message map

BEGIN_MESSAGE_MAP(CPGPdiskResidentApp, CWinApp)
	//{{AFX_MSG_MAP(CPGPdiskResidentApp)
	//}}AFX_MSG
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CPGPdiskResidentApp public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

// The CPGPdiskResidentApp constructor.

CPGPdiskResidentApp::CPGPdiskResidentApp()
{
	App = this;

	mHiddenWindow = NULL;
	mPGPdiskResInfo = NULL;
}


//////////////////////////////////////////////
// CPGPdiskResidentApp public default handlers
//////////////////////////////////////////////

// AbortPGPdiskResApp is the correct way to abort.

void 
CPGPdiskResidentApp::AbortPGPdiskResApp()
{
	exit(-1);
}

// ExitInstance is called when the app is exiting.

int 
CPGPdiskResidentApp::ExitInstance() 
{
	DualErr derr;

	// Destroy our shared memory.
	derr = DeletePGPdiskResInfo();
	pgpAssert(derr.IsntError());

	// Delete the hidden window.
	DeleteHiddenWindow();

	// Close the driver.
	derr = ClosePGPdiskDriver();
	pgpAssert(derr.IsntError());

	// Cleanup explicit linkage.
	CleanupExplicitLinkage();

	return CWinApp::ExitInstance();
}


// InitInstance is called to initialize the app.

BOOL 
CPGPdiskResidentApp::InitInstance()
{
	DualErr derr;

	// Check Windows version.
	CheckWindowsVersion();

	// Resolve APIs unique to a platform.
	derr = ExplicitlyLinkAPIsForPlatform();

	// There can be only one!
	if (derr.IsntError())
	{
		EnforceUniquePGPdiskResApp();

		// Create our hidden window.
		derr = CreateHiddenWindow();
	}

	// Open the driver.
	if (derr.IsntError())
	{
		derr = OpenPGPdiskDriver();
	}

	// Create our shared info structure.
	if (derr.IsntError())
	{
		derr = CreatePGPdiskResInfo();
	}

	if (derr.IsntError())
	{
		return TRUE;
	}
	else
	{
		ReportError(kPGDMajorError_ResidentAppInitFailed, derr);
		return FALSE;
	}
}


////////////////////////////////////////////////////////////////////////
// CPGPdiskResidentApp private custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

// CheckWindowsVersion exits if we are running an incompatible version of
// Windows.

void 
CPGPdiskResidentApp::CheckWindowsVersion()
{
	if (!IsWin95CompatibleMachine() && !IsWinNT4CompatibleMachine())
	{
		ReportError(kPGDMajorError_NotCompatibleWindows);
		AbortPGPdiskResApp();
	}
}

// EnforceUniquePGPdiskResApp croaks the app if an instance is already
// running.

void 
CPGPdiskResidentApp::EnforceUniquePGPdiskResApp()
{
	DualErr derr;

	derr = mResInfoSharedMem.Attach(kPGPdiskResSharedMemName);

	if (derr.IsntError())
		AbortPGPdiskResApp();
}

// CreateHiddenWindow creates the hidden window.

DualErr 
CPGPdiskResidentApp::CreateHiddenWindow()
{
	DualErr		derr;
	PGPBoolean	createdHiddenWindow	= FALSE;

	try
	{
		// Register our class name.
		mHiddenWindowClass = AfxRegisterWndClass(WS_OVERLAPPED);

		// Allocate the window.
		mHiddenWindow = new CHiddenWindow;
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	createdHiddenWindow = derr.IsntError();

	// Create the actual window.
	if (derr.IsntError())
	{
		if (!mHiddenWindow->CreateEx(NULL, mHiddenWindowClass, 
			kPGPdiskResWindowTitle, WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL, 
			NULL))
		{
			derr = DualErr(kPGDMinorError_WindowCreationFailed);
		}
	}

	if (derr.IsntError())
	{
		m_pMainWnd = mHiddenWindow;
	}

	if (derr.IsError())
	{
		if (createdHiddenWindow)
		{
			delete mHiddenWindow;
			mHiddenWindow = NULL;
		}
	}

	return derr;
}

// DeleteHiddenWindow destroys the hidden window.

void 
CPGPdiskResidentApp::DeleteHiddenWindow()
{
	if (IsntNull(mHiddenWindow))
	{
		delete mHiddenWindow;
		mHiddenWindow = NULL;
	}
}

// CreatePGPdiskResInfo allocates our shared application info structure.

DualErr 
CPGPdiskResidentApp::CreatePGPdiskResInfo()
{
	DualErr derr;

	pgpAssertAddrValid(mHiddenWindow, CHiddenWindow);

	derr = mResInfoSharedMem.CreateAndAttach(kPGPdiskResSharedMemName, 
		sizeof(PGPdiskResInfo));

	if (derr.IsntError())
	{
		// Get shared memory pointer.
		mPGPdiskResInfo = (PGPdiskResInfo *) 
			mResInfoSharedMem.GetMemPointer();

		// Initialize the shared memory object.
		mPGPdiskResInfo->hiddenWindowHwnd = mHiddenWindow->m_hWnd;
	}

	return derr;
}

// DeletePGPdiskResInfo deletes our shared application info structure.

DualErr 
CPGPdiskResidentApp::DeletePGPdiskResInfo()
{
	DualErr derr;

	if (IsntNull(mPGPdiskResInfo))
	{
		pgpAssertAddrValid(mPGPdiskResInfo, PGPdiskResInfo);

		derr = mResInfoSharedMem.Detach();
		mPGPdiskResInfo = NULL;
	}

	return derr;
}

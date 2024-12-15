//////////////////////////////////////////////////////////////////////////////
// CPGPdiskShellExtDll.cpp
//
// Definition of class CPGPdiskShellExtDll and DLL exported functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskShellExtDll.cpp,v 1.4 1999/02/26 04:09:59 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "DriverComm.h"
#include "GlobalPGPContext.h"
#include "LinkResolution.h"
#include "WindowsVersion.h"

#include "CPGPdiskShellExtDll.h"
#include "CShellExtClassFactory.h"


////////////
// Constants
////////////

// Initialize GUIDs (should be done only and at-least once per DLL/EXE).

#pragma data_seg(".text")

#define INITGUID
#include <initguid.h>
#include <shlguid.h>

DEFINE_GUID(CLSID_ShellExtension, 
			0xf8b14440L, 
			0x3785, 0x11d1, 
			0xb3, 0x63, 0x5c, 0x6f, 0x08, 0xc1, 0x00, 0x00);

#pragma data_seg()


///////////////////////////
// Global variables for MFC
///////////////////////////

CPGPdiskShellExtDll theApp;


///////////////////////////////
// Global variables for the DLL
///////////////////////////////

CPGPdiskShellExtDll *DLL;		// pointer to DLL object


///////////////////////////
// MFC specific definitions
///////////////////////////

// MFC message map

BEGIN_MESSAGE_MAP(CPGPdiskShellExtDll, CWinApp)
	//{{AFX_MSG_MAP(CPGPdiskShellExtDll)
	//}}AFX_MSG
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CPGPdiskShellExtDll public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

// The CPGPdiskShellExtDll constructor.

CPGPdiskShellExtDll::CPGPdiskShellExtDll()
{
	DLL = this;

	mGCRefThisDll = 0;
}

// The CPGPdiskShellExtDll destructor.

CPGPdiskShellExtDll::~CPGPdiskShellExtDll()
{
}

// ExitInstance is called when the DLL is unloading.

int 
CPGPdiskShellExtDll::ExitInstance()
{
	DualErr derr;

	// Destroy the PGPcontext.
	FreeGlobalPGPContext();

	if (IsDriverOpen())
		ClosePGPdiskDriver();

	CleanupExplicitLinkage();

	return CWinApp::ExitInstance();
}


// InitInstance is called during DLL initialization.

BOOL 
CPGPdiskShellExtDll::InitInstance()
{
	DualErr derr;

	derr = CheckWindowsVersion();

	if (derr.IsntError())
	{
		derr = ExplicitlyLinkAPIsForPlatform();
	}

	if (derr.IsntError())
	{
		derr = CreateGlobalPGPContext();
	}
	
	if (derr.IsntError())
	{
		derr = OpenPGPdiskDriver();
	}

	if (derr.IsntError())
	{
		derr = CheckDriverVersion();
	}

	// DON'T bug user on error, just fail to load.
	return derr.IsntError();
}


////////////////////////////////////////////////////////////////////////
// CPGPdiskShellExtDll private custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

// CheckWindowsVersion checks if we are running an incompatible version of
// Windows.

DualErr 
CPGPdiskShellExtDll::CheckWindowsVersion()
{
	DualErr derr;

	if (!IsWin95CompatibleMachine() && !IsWinNT4CompatibleMachine())
	{
		derr = DualErr(kPGDMinorError_NotCompatibleWindows);
	}

	return derr;
}


/////////////////////
// Exported functions
/////////////////////

// DllCanUnloadNow is called to determine whether it is OK to unload this
// DLL.

STDAPI 
DllCanUnloadNow()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (DLL->mGCRefThisDll == 0)
		return AfxDllCanUnloadNow();
	else
		return S_FALSE;
}

// DllGetClassObject returns the class object for the shell extension.

STDAPI 
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
	CShellExtClassFactory	*pcf;
	PGPBoolean				madeInterface, makeInterface;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Fail if factory or extension still active
	if (DLL->mGCRefThisDll > 0)
		return CLASS_E_CLASSNOTAVAILABLE;

	(* ppvOut) = NULL;

	// Do we make an interface?
	makeInterface = IsEqualIID(rclsid, CLSID_ShellExtension);

	if (makeInterface)
	{
		try
		{
			pcf = new CShellExtClassFactory;	
			madeInterface = TRUE;
		}
		catch (CMemoryException *ex)
		{
			madeInterface = FALSE;
			ex->Delete();
		}
	}

	if (makeInterface && madeInterface)
		return pcf->QueryInterface(riid, ppvOut);
	else
		return CLASS_E_CLASSNOTAVAILABLE;
}

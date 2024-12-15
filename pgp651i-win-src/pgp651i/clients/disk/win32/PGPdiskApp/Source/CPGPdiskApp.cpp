//////////////////////////////////////////////////////////////////////////////
// CPGPdiskApp.cpp
//
// Implementation of class CPGPdiskApp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskApp.cpp,v 1.26 1999/02/26 04:09:57 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "DriverComm.h"
#include "GlobalPGPContext.h"
#include "PGPdiskPrefs.h"
#include "PGPdiskResidentDefines.h"
#include "UtilityFunctions.h"

#include "CMainDialog.h"
#include "CNagBuyDialog.h"
#include "CPGPdiskApp.h"
#include "CPGPdiskCmdLine.h"
#include "CPreferencesSheet.h"

#undef IDB_CREDITS1
#undef IDB_CREDITS4
#undef IDB_CREDITS8
#undef IDC_CREDITS

#include "PGPclx.h"


////////////
// Constants
////////////

const PGPUInt32 kSplashTimeMs = 2500;

static LPCSTR kPGPdiskMainWndTitle	= kPGPdiskAppName;
static LPCSTR kSharedMemNamePrefix	= "PGPdiskSharedMem";


///////////////////////////
// Global variables for MFC
///////////////////////////

CPGPdiskApp theApp;


///////////////////////////////////////
// Global variables for the application
///////////////////////////////////////

CPGPdiskApp *App;		// pointer to application object


///////////////////////////
// MFC specific definitions
///////////////////////////

// MFC message map

BEGIN_MESSAGE_MAP(CPGPdiskApp, CWinApp)
	//{{AFX_MSG_MAP(CPGPdiskApp)
	ON_COMMAND(ID_CONTEXT_HELP, OnContextHelp) 
	ON_COMMAND(ID_HELP, OnHelp)
	//}}AFX_MSG
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CPGPdiskApp public custom functions and non-default message handlers
///////////////////////////////////////////////////////////////////////

// The CPGPdiskApp constructor.

CPGPdiskApp::CPGPdiskApp()
{
	mPAII		= NULL;
	mMainDialog	= NULL;

	mCommandLineMode = FALSE;

	mAutoUnmount			= FALSE;
	mHotKeyEnabled			= FALSE;
	mHotKeyCode				= 0;
	mUnmountOnSleep			= FALSE;
	mNoSleepIfUnmountFail	= kDefaultNoSleepIfUnmountFail;
	mUnmountTimeout			= kDefaultUnmountTimeout;

	if (IsWin95CompatibleMachine())
	{
		mPlatformMaxKbSize = kPGPdiskMaxKbSizeWin95;
		mPlatformMinKbSize = kPGPdiskMinKbSizeWin95;
	}
	else
	{
		mPlatformMaxKbSize = kPGPdiskMaxKbSizeWinNT;
		mPlatformMinKbSize = kPGPdiskMinKbSizeWinNT;
	}
}

// AbortPGPdiskApp is the correct way to abort.

void 
CPGPdiskApp::AbortPGPdiskApp()
{
	exit(-1);
}

// CallShowAboutBox displays the PGPdisk about box.

void 
CPGPdiskApp::CallShowAboutBox()
{	
	AppCommandInfo ACI;

	ACI.op		= kAppOp_ShowAboutBox;
	ACI.flags	= NULL;
	ACI.drive	= kInvalidDrive;
	ACI.path[0]	= '\0';

	DispatchAppCommandInfo(&ACI);
}

// CallShowHelp displays the PGPdisk help file.

void 
CPGPdiskApp::CallShowHelp()
{	
	AppCommandInfo ACI;

	ACI.op		= kAppOp_ShowHelp;
	ACI.flags	= NULL;
	ACI.drive	= kInvalidDrive;
	ACI.path[0]	= '\0';

	DispatchAppCommandInfo(&ACI);
}

// CallShowPrefs displays the PGPdisk preferences.

void 
CPGPdiskApp::CallShowPrefs()
{	
	AppCommandInfo ACI;

	ACI.op		= kAppOp_ShowPrefs;
	ACI.flags	= NULL;
	ACI.drive	= kInvalidDrive;
	ACI.path[0]	= '\0';

	DispatchAppCommandInfo(&ACI);
}

// SetDriverPrefsFromApp informs the driver of the state of the application's
// preferences.

DualErr 
CPGPdiskApp::SetDriverPrefsFromApp()
{
	return SetDriverPrefs(mAutoUnmount, mUnmountTimeout);
}

// CreatePGPdiskResidentApp starts a normal PGPdisk resident application.

DualErr 
CPGPdiskApp::CreatePGPdiskResidentApp()
{
	DualErr				derr;
	PROCESS_INFORMATION	PI;
	STARTUPINFO			SI;

	pgpClearMemory(&SI, sizeof(SI));
	SI.cb = sizeof(SI);

	if (!CreateProcess(NULL, (LPSTR) kPGPdiskResAppName, NULL, NULL, FALSE, 
		0, NULL, NULL, &SI, &PI))
	{
		derr = DualErr(kPGDMinorError_CreateProcessFailed, GetLastError());
	}

	return derr;
}

// UpdatePGPdiskResident starts and/or updates the PGPdiskResident
// application.

PGPBoolean 
CPGPdiskApp::UpdatePGPdiskResident()
{
	DualErr			derr;
	PGPBoolean		succeeded	= FALSE;
	PGPdiskResInfo	*pgpDiskResInfo;
	SharedMemory	pgpDiskResInfoMem;

	// Try to find an existing PGPdiskResident application.
	derr = pgpDiskResInfoMem.Attach(kPGPdiskResSharedMemName);

	// If found, update it. If not, spawn another one.
	if (derr.IsntError())
	{
		pgpDiskResInfo = (PGPdiskResInfo *) 
			pgpDiskResInfoMem.GetMemPointer();

		PostMessage(pgpDiskResInfo->hiddenWindowHwnd, WM_PGPDISKRES_NEWPREFS, 
			kPGPdiskMessageMagic, NULL);

		succeeded = TRUE;
	}
	else
	{
		succeeded = CreatePGPdiskResidentApp().IsntError();
	}

	if (pgpDiskResInfoMem.IsMemAttached())
		pgpDiskResInfoMem.Detach();

	return succeeded;
}

// FindAndShowNonCommandLinePGPdiskApp looks for and shows a pre-existing
// non-command-line PGPdisk application. TRUE is returned if a match is
// found.

PGPBoolean 
CPGPdiskApp::FindAndShowNonCommandLinePGPdiskApp()
{
	PGPBoolean foundApp = FALSE;;

	try
	{
		CString				memName;
		DualErr				derr;
		PAppInstanceInfo	pAII;
		PGPUInt32			i;
		SharedMemory		sharedMem;

		// Step through all running apps.
		for (i = 0; i < kMaxPGPdiskApps; i++)
		{
			memName.Format("%s%d", kSharedMemNamePrefix, i);

			if (mSharedAppInfoMem.IsMemAttached() && 
				(memName == mSharedAppInfoMem.GetMemName()))
			{
				continue;
			}

			// Try to attach the shared memory.
			derr = sharedMem.Attach(memName);

			// If succeeded, examine the memory.
			if (derr.IsntError())
			{
				pAII = (PAppInstanceInfo) sharedMem.GetMemPointer();

				// If it's a non-command-line app, show it.
				if (!pAII->commandLineMode)
				{
					foundApp = TRUE;
					BringThisPGPdiskAppToFront(pAII->mainDialogHwnd);

					break;
				}
			}

			if (sharedMem.IsMemAttached())
				sharedMem.Detach();
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return foundApp;
}

// FindAndShowSimilarPGPdiskApp looks for a pre-existing PGPdisk
// application that is currently performing the same function that this
// application is performing now. If one is found, its frontmost window is
// shown and TRUE is returned.

PGPBoolean 
CPGPdiskApp::FindAndShowSimilarPGPdiskApp(PAppCommandInfo pACI)
{
	PGPBoolean foundApp = FALSE;

	try
	{
		CString				memName;
		DualErr				derr;
		PAppInstanceInfo	pAII;
		PGPUInt32			i;
		SharedMemory		sharedMem;

		pgpAssertAddrValid(pACI, AppCommandInfo);

		// Compare current command to commands of all running apps.
		for (i = 0; i < kMaxPGPdiskApps; i++)
		{
			memName.Format("%s%d", kSharedMemNamePrefix, i);

			// Don't consider ourselves.
			if (mSharedAppInfoMem.IsMemAttached() && 
				(memName == mSharedAppInfoMem.GetMemName()))
			{
				continue;
			}

			// Try to attach the shared memory.
			derr = sharedMem.Attach(memName);

			// If succeeded, examine the memory.
			if (derr.IsntError())
			{
				pAII = (PAppInstanceInfo) sharedMem.GetMemPointer();

				// Compare commands.
				if (AreAppCommandsSimilar(pACI, &pAII->ACI))
				{
					foundApp = TRUE;
					BringThisPGPdiskAppToFront(pAII->mainDialogHwnd);

					break;
				}
			}

			if (sharedMem.IsMemAttached())
				sharedMem.Detach();
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return foundApp;
}

// SetSharedMemoryCommandInfo sets the ACI structure.

void 
CPGPdiskApp::SetSharedMemoryCommandInfo(AppCommandInfo *pACI)
{
	pgpAssertAddrValid(pACI, AppCommandInfo);

	mPAII->ACI = (* pACI);
}

// ClearSharedMemoryCommandInfo sets the ACI structure in the app's shared
// memory object to a NULL op.

void 
CPGPdiskApp::ClearSharedMemoryCommandInfo()
{
	pgpAssertAddrValid(mPAII, AppInstanceInfo);

	mPAII->ACI.op		= kAppOp_InvalidRequest;
	mPAII->ACI.flags	= NULL;
	mPAII->ACI.drive	= kInvalidDrive;
	mPAII->ACI.path[0]	= NULL;
}

// ExitPGPdiskApp is the correct way to exit normally.

void 
CPGPdiskApp::ExitPGPdiskApp()
{
	// This works even when called from other threads.
	::PostThreadMessage(App->m_nThreadID, WM_QUIT, NULL, NULL);
}


//////////////////////////////////////////////
// CPGPdiskApp public default message handlers
//////////////////////////////////////////////

// ExitInstance is called right before the application dies.

int 
CPGPdiskApp::ExitInstance()
{
	DualErr derr;

	// Close our handle to the driver.
	derr = ClosePGPdiskDriver();
	pgpAssert(derr.IsntError());

	// Release our shared memory.
	derr = DeleteAppInfoStruct();
	pgpAssert(derr.IsntError());

	// Delete the main dialog.
	DeleteMainDialog();

	// Destroy the PGPcontext.
	FreeGlobalPGPContext();

	// Unintialize OLE.
	OleUninitialize();

	// Clear the PGPdisk container.
	mPGPdisks.ClearContainerWithDelete();

	// Cleanup explicit linkage.
	CleanupExplicitLinkage();

	return CWinApp::ExitInstance();
}

// InitInstance is called during application initialization.

BOOL 
CPGPdiskApp::InitInstance()
{
	DualErr derr;

	App = this;

	// If there is a command line, we are in command-line mode.
	mCommandLineMode = (m_lpCmdLine[0] != NULL);

	// Quit if we're not running correct Windows version.
	CheckWindowsVersion();

	// Switch to another non-command-line PGPdisk if one exists.
	EnforceUniquePGPdiskGUI();

#if PGPDISK_BETAVERSION

	// Set beta timeout (once).
	SetPGPdiskBetaTimeout();

	// Check if the beta has timed out.
	CheckBetaTimeout();

#elif PGPDISK_DEMOVERSION

	// Check if the demo has timed out.
	CheckDemoTimeout();

#endif // PGPDISK_BETAVERSION

	// Resolve APIs unique to a platform.
	derr = ExplicitlyLinkAPIsForPlatform();

	// If there is a command line, we will not show the main window and we
	// will be sure to exit after all commands are processed.

	if (derr.IsntError())
	{
#ifdef _AFXDLL
		// Call when using MFC in shared DLL.
		Enable3dControls();
#else
		// Call when linking to MFC statically.
		Enable3dControlsStatic();
#endif

		// Intialize OLE.
		if (OleInitialize(NULL) != S_OK)
			derr = DualErr(kPGDMinorError_OleInitFailed);
	}

	// Create a PGPcontext.
	if (derr.IsntError())
	{
		derr = CreateGlobalPGPContext();
	}

	// Open a handle to the driver.
	if (derr.IsntError())
	{
		derr = OpenPGPdiskDriver();
	}

	// Prepare PGPdisk registry settings.
	if (derr.IsntError())
	{
		derr = InitRegistryKeys();
	}

	// Get personalization info from registry.
	if (derr.IsntError())
	{
		derr = GetPersonalizationInfo();
	}

	// Start and/or update the PGPdiskResident application.
	if (derr.IsntError())
	{
		if (!UpdatePGPdiskResident())
			ReportError(kPGDMajorError_CouldntFindResidentApp);
	}

	if (derr.IsntError())
	{
		// Create and hide main dialog.
		derr = CreateMainDialog();
	}

	// Allocate our shared memory.
	if (derr.IsntError())
	{
		derr = CreateAppInfoStruct();
	}

	// Check version numbers of the driver.
	if (derr.IsntError())
	{
		derr = CheckDriverVersion();
	}

	// Notify the driver of our preferences.
	if (derr.IsntError())
	{
		derr = SetDriverPrefsFromApp();
	}

	// Get PGPdisk info.
	if (derr.IsntError())
	{
		derr = UpdateAppPGPdiskInfo();
	}

	// If there is a command line, process it now, else show the main window.
	if (derr.IsntError())
	{
		if (mCommandLineMode)
		{
			CPGPdiskCmdLine cmdInfo;

			cmdInfo.ParseCommandLine();
			derr = DualErr(kPGDMinorError_FailSilently);	// will quit app
		}
		else
		{
			mMainDialog->ShowWindow(SW_SHOWNORMAL);
			mMainDialog->SetForegroundWindow();	
		}
	}

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_AppInitFailed, derr);
		ExitPGPdiskApp();
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// CPGPdiskApp private custom functions and non-default message handlers
////////////////////////////////////////////////////////////////////////

// EnforceUniquePGPdiskGUI ensures there is only one non-command-line
// instance of PGPdisk.

void 
CPGPdiskApp::EnforceUniquePGPdiskGUI()
{
	if (!mCommandLineMode && FindAndShowNonCommandLinePGPdiskApp())
	{
		AbortPGPdiskApp();
	}
}

// CheckWindowsVersion exits if we are running an incompatible version of
// Windows.

void 
CPGPdiskApp::CheckWindowsVersion()
{
	if (!IsWin95CompatibleMachine() && !IsWinNT4CompatibleMachine())
	{
		ReportError(kPGDMajorError_NotCompatibleWindows);
		AbortPGPdiskApp();
	}
}

#if PGPDISK_BETAVERSION

// CheckBetaTimeout checks if we are running an expired beta and warns or
// exits as appropriate.

void 
CPGPdiskApp::CheckBetaTimeout()
{
	if (!mCommandLineMode && HasBetaTimedOut())
	{
		DisplayMessage(kPGPdiskBetaTimeoutWarning);
	}
}

#elif PGPDISK_DEMOVERSION

// CheckDemoTimeout checks if we are running an expired demo and warns or
// exits as appropriate.

void 
CPGPdiskApp::CheckDemoTimeout()
{
	if (!mCommandLineMode && HasDemoTimedOut())
	{
		CNagBuyDialog nagBuyDialog;

		nagBuyDialog.DisplayDialog(kPGPdiskDemoTimeoutWarning);
	}
}

#endif // PGPDISK_BETAVERSION

// BringThisPGPdiskAppToFront displays the frontmost window of the PGPdisk
// app specified by the given main dialog handle.

void 
CPGPdiskApp::BringThisPGPdiskAppToFront(HWND mainDialogHwnd)
{
	HWND frontmostWindow;

	pgpAssert(IsntNull(mainDialogHwnd));

	frontmostWindow = GetLastActivePopup(mainDialogHwnd);

	ShowWindow(frontmostWindow, SW_RESTORE);
	SetForegroundWindow(frontmostWindow);
}

// InitRegistryKeys prepares the registry with necessary PGPdisk
// initialization values, and acquires other values from the registry as
// needed.

DualErr 
CPGPdiskApp::InitRegistryKeys()
{
	DualErr	derr;

	// Update PGPdiskApp pathnames in the registry.
	SetRegistryPaths();

	// Now get the application preferences from the registry.
	if (derr.IsntError())
	{
		derr = GetRegistryPrefs();
	}

	return derr;
}

// CreateMainDialog creates the main dialog and display it.

DualErr 
CPGPdiskApp::CreateMainDialog()
{
	DualErr		derr;
	PGPBoolean	createdMainDialog	= FALSE;

	// Allocate the dialog.
	try
	{
		mMainDialog = new CMainDialog;
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	createdMainDialog = derr.IsntError();

	// Create the actual dialog.
	if (derr.IsntError())
	{
		if (!mMainDialog->Create(IDD_MAIN_DLG))
			derr = DualErr(kPGDMinorError_DialogDisplayFailed);
	}

	if (derr.IsntError())
	{
		m_pMainWnd = mMainDialog;
		RegisterPGPdiskMsgBoxParent(mMainDialog);

		// Set its title.
		mMainDialog->SetWindowText(kPGPdiskMainWndTitle);
	}

	if (derr.IsError())
	{
		if (createdMainDialog)
		{
			delete mMainDialog;
			mMainDialog = NULL;
		}
	}

	return derr;
}

// DeleteMainDialog destroys the main dialog.

void 
CPGPdiskApp::DeleteMainDialog()
{
	if (IsntNull(mMainDialog))
	{
		delete mMainDialog;
		mMainDialog = NULL;
	}
}

// CreateAppInfoStruct allocates our shared application info structure.

DualErr 
CPGPdiskApp::CreateAppInfoStruct()
{
	DualErr		derr, memError;
	PGPBoolean	foundNameForMem	= FALSE;

	pgpAssertAddrValid(mMainDialog, CMainDialog);

	try
	{
		CString		memName;
		PGPUInt32	i;

		// Find an untaken name for our shared memory.
		for (i = 0; i < kMaxPGPdiskApps; i++)
		{
			memName.Format("%s%d", kSharedMemNamePrefix, i);

			memError = mSharedAppInfoMem.CreateAndAttach(memName, 
				sizeof(AppInstanceInfo));

			if (memError.IsntError())
				break;
		}

		if (memError.IsError())
			derr = DualErr(kPGDMinorError_TooManyAppsRegistered);

		if (derr.IsntError())
		{
			// Get shared memory pointer.
			mPAII = (PAppInstanceInfo) mSharedAppInfoMem.GetMemPointer();

			// Initialize the shared memory object.
			mPAII->commandLineMode = mCommandLineMode;
			mPAII->mainDialogHwnd = mMainDialog->m_hWnd;

			ClearSharedMemoryCommandInfo();
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// DeleteAppInfoStruct deletes our shared application info structure.

DualErr 
CPGPdiskApp::DeleteAppInfoStruct()
{
	DualErr derr;

	if (IsntNull(mPAII))
	{
		pgpAssertAddrValid(mPAII, AppInstanceInfo);

		derr = mSharedAppInfoMem.Detach();
		mPAII = NULL;
	}

	return derr;
}

// ShowPGPdiskAboutBox displays the PGPdisk about box.

DualErr 
CPGPdiskApp::ShowPGPdiskAboutBox()
{
	DualErr	derr;

	PGPclHelpAbout(GetGlobalPGPContext(), App->mMainDialog->GetSafeHwnd(), 
		NULL, NULL, NULL);

	return derr;
}

// ShowPGPdiskHelp displays the PGPdisk help file.

void
CPGPdiskApp::ShowPGPdiskHelp()
{
	::WinHelp(mMainDialog->GetSafeHwnd(), m_pszHelpFilePath, HELP_FINDER, 
		NULL);
}

// ShowPGPdiskPrefs displays the PGPdisk preferences.

DualErr 
CPGPdiskApp::ShowPGPdiskPrefs()
{
	CPreferencesSheet	prefsSheet;
	DualErr				derr;

	derr = prefsSheet.mInitErr;

	if (derr.IsntError())
	{
		derr = prefsSheet.DisplayPreferences();
	}

	return derr;
}

// ShowSplashScreen displays the splash screen once a day.

void 
CPGPdiskApp::ShowSplashScreen()
{
	PGPclSplash(GetGlobalPGPContext(), GetDesktopWindow(), kSplashTimeMs);
}

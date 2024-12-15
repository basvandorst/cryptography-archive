//////////////////////////////////////////////////////////////////////////////
// CDriverCommThread.cpp
//
// Implementation of class CDriverCommThread
//////////////////////////////////////////////////////////////////////////////

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "PGPdiskPfl.h"
#include "PGPdiskVersion.h"
#include "UtilityFunctions.h"

#include "CDriverCommThread.h"
#include "CMainDialog.h"
#include "CPGPdiskAppRegistry.h"
#include "CPGPdiskCmdLine.h"
#include "CSuperHotKeyControl.h"
#include "Globals.h"
#include "PGPdisk.h"


////////////
// Constants
////////////

const PGPUInt32 kCommThreadTimeout = 5000;	// 5 seconds

// Ring-0 shift-state bits, from 'VKD.h'.

const PGPUInt16	SS_Shift		= 0x0001;	// left or right
const PGPUInt16	SS_LShift		= 0x0002;
const PGPUInt16	SS_RShift		= 0x0200;
const PGPUInt16	SS_Ctrl			= 0x0080;	// left or right
const PGPUInt16	SS_LCtrl		= 0x0004;
const PGPUInt16	SS_RCtrl		= 0x0400;
const PGPUInt16	SS_Alt			= 0x0100;	// left or right
const PGPUInt16	SS_LAlt			= 0x0008;
const PGPUInt16	SS_RAlt			= 0x0800;
const PGPUInt16	SS_CapLock		= 0x0040;
const PGPUInt16	SS_NumLock		= 0x0020;
const PGPUInt16	SS_ScrlLock		= 0x0010;
const PGPUInt16	SS_CapLock_Dn	= 0x4000;	// SS_CapLock shl 8
const PGPUInt16	SS_NumLock_Dn	= 0x2000;	// SS_NumLock shl 8
const PGPUInt16	SS_ScrlLock_Dn	= 0x1000;	// SS_ScrlLock shl 8
const PGPUInt16	SS_Unused		= 0x8000;


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CDriverCommThread, CWinThread)
	//{{AFX_MSG_MAP(CDriverCommThread)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDriverCommThread public custom functions and non-default message handlers
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDriverCommThread, CWinThread)

// The CDriverCommThread constructor.

CDriverCommThread::CDriverCommThread()
{
	mDeregSem = NULL;

	mDriverHandle		= INVALID_HANDLE_VALUE;
	mPGPDriverHandShook	= FALSE;
	mDriverVersion		= 0;

	// Create the start semaphore that creation will wait on.
	mInitErr = PGPdiskCreateSemaphore(0, 1, NULL, &mStartSem);

	// Create the de-registration semaphore that the thread will wait on.
	if (mInitErr.IsntError())
	{
		mInitErr = PGPdiskCreateSemaphore(0, 1, NULL, &mDeregSem);
	}
}

// The CDriverCommThread destructor.

CDriverCommThread::~CDriverCommThread()
{
	// Destroy the semaphores.
	::CloseHandle(mDeregSem);
	::CloseHandle(mStartSem);
}

// IsDriverOpen returns TRUE if the PGPdisk driver is open, FALSE otherwise.

PGPBoolean 
CDriverCommThread::IsDriverOpen()
{
	return (mDriverHandle != INVALID_HANDLE_VALUE);
}

// Send a packet to the driver using DeviceIoControl. 'mPGPdiskDrvHandle' is
// the handle to the driver opened previously.

DualErr 
CDriverCommThread::SendPacket(PPacket pPacket)
{
	DualErr			derr;
	unsigned long	nBytesReturned;			// can't use PGPUInt32

	pgpAssert(IsDriverOpen());

	if (!(::DeviceIoControl(mDriverHandle, kCTL_SendPacket, pPacket, 
		sizeof(PPacket), NULL, NULL, &nBytesReturned, NULL)))
	{
		derr = DualErr(kPGDMinorError_DriverCommFailure, ::GetLastError());
	}

	return derr;
}

// SetDriverPrefs informs the driver of the state of the application's
// preferences.

void 
CDriverCommThread::SetDriverPrefs()
{
	AD_ChangePrefs	CP;
	DualErr			derr, prefsDerr;
	PGPUInt8		modState	= GetHighByte(App->mHotKeyCode);

	CP.code					= kAD_ChangePrefs;
	CP.autoUnmount			= App->mAutoUnmount;
	CP.hotKeyEnabled		= App->mHotKeyEnabled;
	CP.unmountOnSleep		= App->mUnmountOnSleep;
	CP.noSleepIfUnmountFail	= App->mNoSleepIfUnmountFail;
	CP.unmountTimeout		= App->mUnmountTimeout;
	CP.useAsyncIO			= App->mUseAsyncIO;
	CP.pDerr				= &prefsDerr;

	// Translate hotkey virtual key-code to scan code.
	CP.hotKeyInfo.scanCode = ::MapVirtualKey(GetLowByte(App->mHotKeyCode), 0);
	CP.hotKeyInfo.shiftState = NULL;

	// Translate hotkey shift state to Ring-0 shift state.
	if (modState & kSHK_Control)
		CP.hotKeyInfo.shiftState |= SS_Ctrl;

	if (modState & kSHK_Alt)
		CP.hotKeyInfo.shiftState |= SS_Alt;

	if (modState & kSHK_Shift)
		CP.hotKeyInfo.shiftState |= SS_Shift;

	// Extended key?
	CP.hotKeyInfo.isExtended = (modState & kSHK_Extended ? TRUE : FALSE);

	// Send the packet.
	derr = Comm->SendPacket((PPacket) &CP);

	if (derr.IsError())
	{
		App->ReportError(kPGDMajorError_DriverCommFailed, derr);
	}
	else if (prefsDerr.IsError())
	{
		App->ReportError(kPGDMajorError_DriverUpdatePrefsFailed, prefsDerr);
	}
}

// StartThread starts the thread.

DualErr 
CDriverCommThread::StartThread()
{
	DualErr	derr;

	if (!CreateThread())
		derr = DualErr(kPGDMinorError_ThreadCreationFailed);

	// Wait for initialization to complete.
	if (derr.IsntError())
	{
		if (::WaitForSingleObject(mStartSem, kCommThreadTimeout) != 
			WAIT_OBJECT_0)
		{
			derr = DualErr(kPGDMinorError_ThreadCreationFailed);
		}
	}

	if (derr.IsntError())
	{
		derr = mInitErr;
	}

	return derr;
}

// EndThread ends the thread.

void 
CDriverCommThread::EndThread()
{
	// Tell the thread to buy it.
	::ReleaseSemaphore(mDeregSem, 1, NULL);

	// We must wait until the thread exits before continuing.
	::WaitForSingleObject(m_hThread, kCommThreadTimeout);
}


////////////////////////////////////////////////////
// CDriverCommThread public default message handlers
////////////////////////////////////////////////////

// The CDriverCommThread cleanup function.

int 
CDriverCommThread::ExitInstance()
{
	AD_DeRegister DR;

	// Deregister and close handles to the driver.
	if (IsDriverOpen())
	{
		DR.code	= kAD_DeRegister;
		SendPacket((PPacket) &DR);

		ClosePGPdiskDriver();
	}

	return CWinThread::ExitInstance();
}

// The CDriverCommThread initializer. We create our semaphore that will be
// released by the application to tell us to exit the thread, and register
// with the driver.

BOOL 
CDriverCommThread::InitInstance()
{
	DualErr		regDerr;
	PGPBoolean	openedPGPdiskDriver, registeredApp;

	openedPGPdiskDriver = registeredApp = FALSE;

	// Start the driver.
	mInitErr = OpenPGPdiskDriver();

	// Fill in and send the registration packet.
	if (mInitErr.IsntError())
	{
		AD_Register REG;

		openedPGPdiskDriver = TRUE;

		REG.code		= kAD_Register;
		REG.appVersion	= kPGPdiskAppVersion;
		REG.apcFunc		= ReceivePacketsFunc;

		REG.pDerr			= &regDerr;
		REG.pDriverVersion	= &mDriverVersion;

		mInitErr = SendPacket((PPacket) &REG);
	}

	// Did registration succeed?
	if (mInitErr.IsntError())
	{
		mInitErr = regDerr;
	}

	// Check for version incompatiblity.
	if (mInitErr.IsntError())
	{
		registeredApp = TRUE;

		if (mDriverVersion != kCompatDriverVersion)
			mInitErr = DualErr(kPGDMinorError_DriverIsIncompatVersion);
	}

	// Notify the driver of our preferences.
	if (mInitErr.IsntError())
	{
		SetDriverPrefs();
	}

	// Tell the app it can continue initializing.
	::ReleaseSemaphore(mStartSem, 1, NULL);

	if (mInitErr.IsError())
	{
		// Must call this ourselves on error.
		ExitInstance();
	}

	return mInitErr.IsntError();
}

// Run is the main message loop of the thread. It is overridden so PGPdisk can
// set up the thread to recieve communication from the driver.

int 
CDriverCommThread::Run() 
{
	DualErr derr;
	
	pgpAssert(IsntNull(mDeregSem));

	// While we are waiting for application exist, we also will receive APC
	// callbacks from the driver.

	while (::WaitForSingleObjectEx(mDeregSem, INFINITE, TRUE) == 
		WAIT_IO_COMPLETION)
	{ }

	// Must call this ourselves.
	return ExitInstance();
}


////////////////////////////////////////////////////////////////////////
// CDriverCommThread protected custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

// OpenPGPdiskDriver opens a handle to the PGPdisk driver.

DualErr 
CDriverCommThread::OpenPGPdiskDriver()
{
	DualErr derr;

	pgpAssert(!IsDriverOpen());

	mDriverHandle = ::CreateFile(kPGPdiskDriverName, 
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (mDriverHandle == INVALID_HANDLE_VALUE)
		derr = DualErr(kPGDMinorError_DriverNotInstalled);

	return derr;
}

// ClosePGPdiskDriver closes a handle to the PGPdisk driver.

DualErr 
CDriverCommThread::ClosePGPdiskDriver()
{
	DualErr derr;

	pgpAssert(IsDriverOpen());

	if (!::CloseHandle(mDriverHandle))
		derr = DualErr(kPGDMinorError_FileCloseFailed);

	return derr;
}

// FreeDriverPacket asks the driver to free a packet it allocated and sent to
// us.

DualErr 
CDriverCommThread::FreeDriverPacket(PPacket pDApacket)
{
	AD_FreePacket	FP;
	DualErr			derr;

	pgpAssertAddrValid(pDApacket, VoidAlign);

	FP.code			= kAD_FreePacket;
	FP.pDApacket	= pDApacket;

	derr = SendPacket((PPacket) &FP);

	return derr;
}

// ProcessReportError processes an error code from the driver.

void 
CDriverCommThread::ProcessReportError(PDA_ReportError pRE)
{
	pgpAssertAddrValid(pRE, DA_ReportError);

	// IO errors are a special case.
	if ((pRE->perr == kPGDMajorError_PGPdiskReadErrorSerious) ||
		(pRE->perr == kPGDMajorError_PGPdiskWriteErrorSerious))
	{
		App->HandleIOError(pRE->perr, pRE->derr, pRE->drive);
	}
	else
	{
		App->ReportError(pRE->perr, pRE->derr, pRE->drive);
	}

	// If we are in 'error mode', quit after seeing one error.
	if (App->mErrorMode)
		App->ExitApp();
}

// ProcessUpdateApp parses the list of mounted PGPdisk info structures sent to
// us by the driver. We scrap our current list of PGPdisks and reconstruct
// it.

void 
CDriverCommThread::ProcessUpdateApp(PDA_UpdateApp pUA)
{
	DualErr			derr;
	PGPdiskInfo		*pPDI	= NULL;

	pgpAssertAddrValid(pUA, DA_UpdateApp);
	pPDI = pUA->pPDI;

	// First empty the container.
	App->mPGPdisks.ClearContainerWithDelete();

	// Each PGPdiskInfo structure contains info about a mounted PGPdisk. We
	// create a corresponding PGPdisk object and store it in the global list.

	while (pPDI && derr.IsntError())
	{
		PGPBoolean	createdPGPdiskObject	= FALSE;
		PGPdisk		*pPGD;

		pgpAssertStrValid(pPDI->path);
		pgpAssert(IsLegalDriveNumber(pPDI->drive));
		pgpAssert(pPDI->sessionId != kInvalidSessionId);

		try
		{
			pPGD = new PGPdisk(pPDI->path, pPDI->drive, pPDI->sessionId);
		}
		catch (CMemoryException *ex)
		{
			derr = DualErr(kPGDMinorError_OutOfMemory);
			ex->Delete();
		}
		
		if (derr.IsntError())			// check if created successfully
		{
			createdPGPdiskObject = TRUE;
			derr = pPGD->mInitErr;
		}

		if (derr.IsntError())			// put in list and iterate
		{
			App->mPGPdisks.AddPGPdisk(pPGD);
			pPDI = pPDI->next;
		}

		if (derr.IsError())
		{
			if (createdPGPdiskObject)
				delete pPGD;
		}
	}

	// If this is the first time updating the application, we can process the
	// command line. Exit the app after processing any commands on the
	// command line, or else show the main window.

	if (derr.IsntError())
	{
		if (!mPGPDriverHandShook)
		{
			mPGPDriverHandShook = TRUE;

			if (App->mCommandLineMode)		// don't show the main dialog
			{
				CPGPdiskCmdLine	cmdInfo;

				derr = cmdInfo.ParseCommandLine();
				App->ExitApp();
			}
			else if (App->mMainDialog)		// show the main dialog
			{
				App->mMainDialog->ShowWindow(SW_SHOWNORMAL);
				App->mMainDialog->SetForegroundWindow();	
			}
		}
	}

	if (derr.IsError())
	{
		App->ReportError(kPGDMajorError_DriverCommFailed, derr);
	}
}

// ReceivePacketsFunc is the function which the driver calls in order to send
// a packet to the application. It is called using the Asynchronous Procedure
// Call (APC) interface. The app must have previously registered this function
// with the driver, which we do during app initialization.

DWORD 
WINAPI 
CDriverCommThread::ReceivePacketsFunc(PPacket DApacket)
{
	DualErr		derr;
	PGPUInt16	DAcode;
	
	pgpAssertAddrValid(DApacket, VoidAlign);
	DAcode = GetDACode(DApacket);

	switch (DAcode)
	{

	case kDA_ReportError:		// driver tells us of an error
		Comm->ProcessReportError((PDA_ReportError) DApacket);
		derr = Comm->FreeDriverPacket(DApacket);
		break;

	case kDA_UpdateApp:			// driver tells us of mounted PGPdisks
		Comm->ProcessUpdateApp((PDA_UpdateApp) DApacket);
		derr = Comm->FreeDriverPacket(DApacket);
		break;

	default:
		pgpAssert(FALSE);
		break;
	}

	if (derr.IsError())
	{
		App->ReportError(kPGDMajorError_DriverCommFailed, derr);
	}

	return 0;
}

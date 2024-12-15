//////////////////////////////////////////////////////////////////////////////
// CCreatePGPdiskThread.cpp
//
// Implementation of class CCreatePGPdiskThread.
//////////////////////////////////////////////////////////////////////////////

// $Id: CCreatePGPdiskThread.cpp,v 1.7 1999/02/26 04:09:56 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "UtilityFunctions.h"

#include "CCreatePGPdiskThread.h"
#include "Globals.h"


////////////
// Constants
////////////

// Custom windows messages

const PGPUInt16 WM_CREATE_PGPDISK = WM_USER + 196;


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CCreatePGPdiskThread, CWinThread)
	//{{AFX_MSG_MAP(CCreatePGPdiskThread)
	ON_THREAD_MESSAGE(WM_CREATE_PGPDISK, OnCreatePGPdisk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////
// CCreatePGPdiskThread public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

// The CCreatePGPdiskThread default constructor.

CCreatePGPdiskThread::CCreatePGPdiskThread()
{
	mIsCreationOngoing	= FALSE;
	mUserCancelFlag		= FALSE;

	// The thread will not delete itself.
	m_bAutoDelete = FALSE;
}

// The CCreatePGPdiskThread default destructor.

CCreatePGPdiskThread::~CCreatePGPdiskThread()
{
}

// CallPGPdiskCreate sends a message to the thread asking it to create a new
// PGPdisk using the specified parameters. When finished, a message is
// sent to the calling window.

DualErr 
CCreatePGPdiskThread::CallPGPdiskCreate(CreatePGPdiskInfo *pCPI)
{
	DualErr derr;

	pgpAssert(!mIsCreationOngoing);

	pgpAssertAddrValid(pCPI, CreatePGPdiskInfo);
	mCPI = (* pCPI);

	pgpAssertStrValid(mCPI.path);
	pgpAssert(mCPI.blocksDisk > 0);
	pgpAssertAddrValid(mCPI.passphrase, SecureString);
	pgpAssert(IsLegalDriveNumber(mCPI.drive));
	pgpAssert(IsntNull(mCPI.callerHwnd));

	// Create the thread
	if (!CreateThread())
	{
		derr = DualErr(kPGDMinorError_ThreadCreationFailed);
	}

	mIsCreationOngoing = derr.IsntError();

	// Send the create command to the thread.
	if (derr.IsntError())
	{
		if (!PostThreadMessage(WM_CREATE_PGPDISK, kPGPdiskMessageMagic, 
			NULL))
		{
			mIsCreationOngoing = FALSE;
			derr = DualErr(kPGDMinorError_PostThreadMessageFailed);
		}
	}

	return derr;
}

// CancelPGPdiskCreate can be called to cancel PGPdisk creation.

void 
CCreatePGPdiskThread::CancelPGPdiskCreate()
{
	pgpAssert(mIsCreationOngoing);
	mUserCancelFlag = TRUE;
}


///////////////////////////////////////////////////////
// CCreatePGPdiskThread public default message handlers
///////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CCreatePGPdiskThread, CWinThread)

// InitInstance is called to initialize the thread.

BOOL 
CCreatePGPdiskThread::InitInstance()
{
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// CCreatePGPdiskThread protected custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////////

// OnCreatePGPdisk is called when the thread recieves a WM_CREATE_PGPDISK
// message. The function creates the PGPdisk using data from the thread's
// initialized parameters.

void 
CCreatePGPdiskThread::OnCreatePGPdisk(WPARAM wParam, LPARAM lParam)
{
	PGPdisk *pPGD = NULL;

	if (wParam != kPGPdiskMessageMagic)		// prevent collisions
		return;

	pgpAssert(mIsCreationOngoing);

	pgpAssertStrValid(mCPI.path);
	pgpAssert(mCPI.blocksDisk > 0);
	pgpAssertAddrValid(mCPI.passphrase, SecureString);
	pgpAssert(IsLegalDriveNumber(mCPI.drive));
	pgpAssert(IsntNull(mCPI.callerHwnd));

	// Create a new PGPdisk object.
	try
	{
		pPGD = new PGPdisk();
	}
	catch (CMemoryException *ex)
	{
		mDerr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (mDerr.IsntError())
	{
		mDerr = pPGD->mInitErr;
	}

	// Set progress window information.
	if (mCPI.progressHwnd)
	{
		pPGD->SetProgressBarInfo(mCPI.progressHwnd, &mUserCancelFlag);
	}

	// Create the PGPdisk.
	if (mDerr.IsntError())
	{
		mDerr = pPGD->CreatePGPdisk(mCPI.path, mCPI.blocksDisk, 
			kDefaultEncryptionAlgorithm, mCPI.passphrase, mCPI.drive);
	}

	// Delete the temporary PGPdisk object.
	if (pPGD)
		delete pPGD;

	mIsCreationOngoing = FALSE;

	// Notify caller window we're done.
	PostMessage(mCPI.callerHwnd, WM_FINISHED_CREATE, kPGPdiskMessageMagic, 
		(LPARAM) this);

	// Quit the thread.
	PostQuitMessage(0);
}

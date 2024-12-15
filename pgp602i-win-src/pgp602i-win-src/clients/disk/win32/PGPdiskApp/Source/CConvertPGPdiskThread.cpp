//////////////////////////////////////////////////////////////////////////////
// CConvertPGPdiskThread.cpp
//
// Implementation of class CConvertPGPdiskThread.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertPGPdiskThread.cpp,v 1.1.2.3 1998/10/22 22:27:23 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "UtilityFunctions.h"

#include "CConvertPGPdiskThread.h"
#include "Globals.h"


////////////
// Constants
////////////

// Custom windows messages

const PGPUInt16 WM_CONVERT_PGPDISK = WM_USER + 243;


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CConvertPGPdiskThread, CWinThread)
	//{{AFX_MSG_MAP(CConvertPGPdiskThread)
	ON_THREAD_MESSAGE(WM_CONVERT_PGPDISK, OnConvertPGPdisk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////
// CConvertPGPdiskThread public custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////

// The CConvertPGPdiskThread default constructor.

CConvertPGPdiskThread::CConvertPGPdiskThread()
{
	mIsConversionOngoing	= FALSE;
	mUserCancelFlag			= FALSE;

	// The thread will not delete itself.
	m_bAutoDelete = FALSE;
}

// The CConvertPGPdiskThread default destructor.

CConvertPGPdiskThread::~CConvertPGPdiskThread()
{
}

// CallPGPdiskConvert sends a message to the thread asking it to convert a
// new PGPdisk using the specified parameters. When finished, a message is
// sent to the calling window.

DualErr 
CConvertPGPdiskThread::CallPGPdiskConvert(ConvertPGPdiskInfo *pCPI)
{
	DualErr derr;

	pgpAssert(!mIsConversionOngoing);

	pgpAssertAddrValid(pCPI, ConvertPGPdiskInfo);
	mCPI = (* pCPI);

	pgpAssertStrValid(mCPI.path);
	pgpAssertAddrValid(mCPI.passphrase, SecureString);
	pgpAssert(IsntNull(mCPI.callerHwnd));

	// Create the thread
	if (!CreateThread())
	{
		derr = DualErr(kPGDMinorError_ThreadCreationFailed);
	}

	mIsConversionOngoing = derr.IsntError();

	// Send the convert command to the thread.
	if (derr.IsntError())
	{
		if (!PostThreadMessage(WM_CONVERT_PGPDISK, kPGPdiskMessageMagic, 
			NULL))
		{
			mIsConversionOngoing = FALSE;
			derr = DualErr(kPGDMinorError_PostThreadMessageFailed);
		}
	}

	return derr;
}

// CancelPGPdiskConvert can be called to cancel PGPdisk conversion.

void 
CConvertPGPdiskThread::CancelPGPdiskConvert()
{
	pgpAssert(mIsConversionOngoing);
	mUserCancelFlag = TRUE;
}


////////////////////////////////////////////////////////
// CConvertPGPdiskThread public default message handlers
////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CConvertPGPdiskThread, CWinThread)

// InitInstance is called to initialize the thread.

BOOL 
CConvertPGPdiskThread::InitInstance()
{
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
// CConvertPGPdiskThread protected custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////////

// OnConvertPGPdisk is called when the thread recieves a WM_CONVERT_PGPDISK
// message. The function converts the PGPdisk using data from the thread's
// initialized parameters.

void 
CConvertPGPdiskThread::OnConvertPGPdisk(WPARAM wParam, LPARAM lParam)
{
	PGPdisk *pPGD = NULL;

	if (wParam != kPGPdiskMessageMagic)		// prevent collisions
		return;

	pgpAssert(mIsConversionOngoing);

	pgpAssertStrValid(mCPI.path);
	pgpAssertAddrValid(mCPI.passphrase, SecureString);

	// Convert a new PGPdisk object.
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

	// Convert the PGPdisk.
	if (mDerr.IsntError())
	{
		mDerr = pPGD->ConvertPGPdisk(mCPI.path, mCPI.passphrase, 
			mCPI.randomPool);
	}

	// Delete the temporary PGPdisk object.
	if (pPGD)
		delete pPGD;

	mIsConversionOngoing = FALSE;

	// Notify caller window we're done.
	PostMessage(mCPI.callerHwnd, WM_FINISHED_CONVERT, kPGPdiskMessageMagic, 
		(LPARAM) this);

	// Quit the thread.
	PostQuitMessage(0);
}

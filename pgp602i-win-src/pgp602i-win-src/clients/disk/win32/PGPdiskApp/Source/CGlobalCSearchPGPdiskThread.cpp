//////////////////////////////////////////////////////////////////////////////
// CGlobalCSearchPGPdiskThread.cpp
//
// Implementation of class CGlobalCSearchPGPdiskThread.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCSearchPGPdiskThread.cpp,v 1.1.2.3 1998/10/23 23:37:24 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskHighLevelUtils.h"
#include "UtilityFunctions.h"

#include "CGlobalCSearchPGPdiskThread.h"
#include "Globals.h"


////////////
// Constants
////////////

// Custom windows messages

const PGPUInt16 WM_SEARCH_PGPDISK = WM_USER + 245;


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CGlobalCSearchPGPdiskThread, CWinThread)
	//{{AFX_MSG_MAP(CGlobalCSearchPGPdiskThread)
	ON_THREAD_MESSAGE(WM_SEARCH_PGPDISK, OnSearchPGPdisk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////
// CGlobalCSearchPGPdiskThread public custom functions and handlers
///////////////////////////////////////////////////////////////////

// The CGlobalCSearchPGPdiskThread default constructor.

CGlobalCSearchPGPdiskThread::CGlobalCSearchPGPdiskThread()
{
	mIsSearchingOngoing	= FALSE;
	mUserCancelFlag			= FALSE;

	// The thread will not delete itself.
	m_bAutoDelete = FALSE;
}

// The CGlobalCSearchPGPdiskThread default destructor.

CGlobalCSearchPGPdiskThread::~CGlobalCSearchPGPdiskThread()
{
}

// CallPGPdiskSearch sends a message to the thread asking it to search for
// unconverted PGPdisks using the specified parameters. When finished, a
// message is sent to the calling window.

DualErr 
CGlobalCSearchPGPdiskThread::CallPGPdiskSearch(SearchPGPdiskInfo *pSPI)
{
	DualErr derr;

	pgpAssert(!mIsSearchingOngoing);

	pgpAssertAddrValid(pSPI, SearchPGPdiskInfo);
	mSPI = (* pSPI);

	pgpAssert(IsntNull(mSPI.callerHwnd));

	// Create the thread
	if (!CreateThread())
	{
		derr = DualErr(kPGDMinorError_ThreadCreationFailed);
	}

	mIsSearchingOngoing = derr.IsntError();

	// Send the convert command to the thread.
	if (derr.IsntError())
	{
		if (!PostThreadMessage(WM_SEARCH_PGPDISK, kPGPdiskMessageMagic, NULL))
		{
			mIsSearchingOngoing = FALSE;
			derr = DualErr(kPGDMinorError_PostThreadMessageFailed);
		}
	}

	return derr;
}

// CancelPGPdiskSearch can be called to cancel PGPdisk searching.

void 
CGlobalCSearchPGPdiskThread::CancelPGPdiskSearch()
{
	pgpAssert(mIsSearchingOngoing);
	mUserCancelFlag = TRUE;
}


//////////////////////////////////////////////////////////////
// CGlobalCSearchPGPdiskThread public default message handlers
//////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CGlobalCSearchPGPdiskThread, CWinThread)

// InitInstance is called to initialize the thread.

BOOL 
CGlobalCSearchPGPdiskThread::InitInstance()
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CGlobalCSearchPGPdiskThread protected custom functions and handlers
//////////////////////////////////////////////////////////////////////

// OnSearchPGPdisk is called when the thread recieves a WM_SEARCH_PGPDISK
// message. The function searches for unconverted PGPdisks using data from
// the thread's initialized parameters.

void 
CGlobalCSearchPGPdiskThread::OnSearchPGPdisk(WPARAM wParam, LPARAM lParam)
{
	if (wParam != kPGPdiskMessageMagic)		// prevent collisions
		return;

	pgpAssert(mIsSearchingOngoing);

	try
	{
		DualErr		derr;
		PGPUInt8	drive;
		PGPUInt32	logicalDrives;
		
		logicalDrives = GetLogicalDrives();

		for (drive = 2; drive < kMaxDrives; drive++)
		{
			// For every drive letter...
			if (logicalDrives & (1 << drive))
			{
				CString root;

				derr = MakeRoot(drive, &root);

				// If it's a local drive, search it for bad PGPdisks.
				if (derr.IsntError())
				{
					if ((GetDriveType(root) == DRIVE_REMOVABLE) || 
						(GetDriveType(root) == DRIVE_FIXED))
					{
						SearchForPGPdisksInDirectory(root);
					}
				}
			}
		}

		mDerr = derr;
	}
	catch (CMemoryException *ex)
	{
		mDerr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	mIsSearchingOngoing = FALSE;

	// Notify caller window we're done.
	PostMessage(mSPI.callerHwnd, WM_FINISHED_SEARCH, kPGPdiskMessageMagic, 
		(LPARAM) this);

	// Quit the thread.
	PostQuitMessage(0);
}

// SearchForPGPdisksInDirectory searches for non-converted PGPdisks in the
// specified directory, and in all sub-directories.

void 
CGlobalCSearchPGPdiskThread::SearchForPGPdisksInDirectory(LPCSTR dir)
{
	CFileFind	dirWalker;
	CString		wildDirPath;

	if (mUserCancelFlag)
		return;

	wildDirPath = dir;

	::SetWindowText(mSPI.dirStaticHwnd, dir);

	if (wildDirPath[wildDirPath.GetLength() - 1] == '\\')
		wildDirPath += "*.*";
	else
		wildDirPath += "\\*.*";

	if (dirWalker.FindFile(wildDirPath))
	{
		PGPBoolean moreFiles = TRUE;

		// For every file or directory in the directory...
		while (moreFiles)
		{
			CString path;

			moreFiles = dirWalker.FindNextFile();
			path = dirWalker.GetFilePath();

			if (dirWalker.IsDirectory())
			{
				// If it's a directory, recurse.
				if (!dirWalker.IsDots())
					SearchForPGPdisksInDirectory(path);
			}
			else
			{
				PGPBoolean hasExtension, isPGPdisk;

				hasExtension = (path.Right(4).CompareNoCase(
					kPGPdiskFileExtension) == 0);
			
				isPGPdisk = hasExtension && ValidatePGPdisk(path).IsntError();

				// If it's a file, check if it's a bad PGPdisk.
				if (isPGPdisk && DoesPGPdiskHaveBadCAST(path))
				{
					SendMessage(mSPI.listBoxHwnd, LB_ADDSTRING, 0, 
						(LPARAM) (LPCSTR) path);
				}
			}
		}
	}
}

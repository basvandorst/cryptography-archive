//////////////////////////////////////////////////////////////////////////////
// CGlobalCSearchPGPdiskThread.h
//
// Declaration of class CGlobalCSearchPGPdiskThread.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCSearchPGPdiskThread.h,v 1.2 1998/12/14 18:57:00 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CGlobalCSearchPGPdiskThread_h	// [
#define Included_CGlobalCSearchPGPdiskThread_h

#include "DualErr.h"


////////////
// Constants
////////////

const PGPUInt16 WM_FINISHED_SEARCH = WM_USER + 244;


////////
// Types
////////

// A SearchPGPdiskInfo structure contains information for searching for
// old, bad CAST PGPdisks.

typedef struct SearchPGPdiskInfo
{
	HWND	callerHwnd;		// HWND of caller window
	HWND	listBoxHwnd;	// HWND of list box
	HWND	dirStaticHwnd;	// HWND of directory static

} SearchPGPdiskInfo;


////////////////////////////////////
// Class CGlobalCSearchPGPdiskThread
////////////////////////////////////

class CGlobalCSearchPGPdiskThread : public CWinThread
{
	DECLARE_DYNCREATE(CGlobalCSearchPGPdiskThread)

public:
	DualErr	mDerr;				// holds error

			CGlobalCSearchPGPdiskThread();
	virtual	~CGlobalCSearchPGPdiskThread();

	DualErr	CallPGPdiskSearch(SearchPGPdiskInfo *pSPI); 
	void	CancelPGPdiskSearch();

	//{{AFX_VIRTUAL(CGlobalCSearchPGPdiskThread)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

protected:
	SearchPGPdiskInfo	mSPI;					// info for the conversion
	PGPBoolean			mIsSearchingOngoing;	// searching happening now?
	PGPBoolean			mUserCancelFlag;		// TRUE to cancel

	void SearchForPGPdisksInDirectory(LPCSTR dir);

	//{{AFX_MSG(CGlobalCSearchPGPdiskThread)
	afx_msg void OnSearchPGPdisk(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CGlobalCSearchPGPdiskThread_h

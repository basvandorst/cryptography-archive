//////////////////////////////////////////////////////////////////////////////
// CCreatePGPdiskThread.h
//
// Declaration of class CCreatePGPdiskThread.
//////////////////////////////////////////////////////////////////////////////

// $Id: CCreatePGPdiskThread.h,v 1.5 1999/02/26 04:09:56 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CCreatePGPdiskThread_h	// [
#define Included_CCreatePGPdiskThread_h

#include "DualErr.h"

#include "PGPdisk.h"


////////////
// Constants
////////////

const PGPUInt16 WM_FINISHED_CREATE = WM_USER + 197;


////////
// Types
////////

// A CreatePGPdiskInfo structure contains information for a PGPdisk create.

typedef struct CreatePGPdiskInfo
{
	LPCSTR			path;			// path to PGPdisk
	PGPUInt64		blocksDisk;		// size in blocks
	SecureString	*passphrase;	// pointer to passphrase
	PGPUInt8		drive;			// preferred drive number
	HWND			callerHwnd;		// HWND of caller window
	HWND			progressHwnd;	// HWND of progress bar

} CreatePGPdiskInfo;


/////////////////////////////
// Class CCreatePGPdiskThread
/////////////////////////////

// A CCreatePGPdiskThread will simply call PGPdisk::Create in a separate
// thread context, using the specified PGPdisk object pointer and the
// specified parameters.

class CCreatePGPdiskThread : public CWinThread
{
	DECLARE_DYNCREATE(CCreatePGPdiskThread)

public:
	DualErr	mDerr;				// holds error

			CCreatePGPdiskThread();
	virtual	~CCreatePGPdiskThread();

	DualErr	CallPGPdiskCreate(CreatePGPdiskInfo *pCPI); 
	void	CancelPGPdiskCreate();

	//{{AFX_VIRTUAL(CCreatePGPdiskThread)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

protected:
	CreatePGPdiskInfo	mCPI;					// info for the create
	PGPBoolean			mIsCreationOngoing;		// creation happening now?
	PGPBoolean			mUserCancelFlag;		// TRUE to cancel

	//{{AFX_MSG(CCreatePGPdiskThread)
	afx_msg void OnCreatePGPdisk(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CCreatePGPdiskThread_h

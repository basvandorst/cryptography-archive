//////////////////////////////////////////////////////////////////////////////
// CConvertPGPdiskThread.h
//
// Declaration of class CConvertPGPdiskThread.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertPGPdiskThread.h,v 1.3 1999/02/26 04:09:56 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CConvertPGPdiskThread_h	// [
#define Included_CConvertPGPdiskThread_h

#include "DualErr.h"
#include "PGPdisk.h"


////////////
// Constants
////////////

const PGPUInt16 WM_FINISHED_CONVERT = WM_USER + 243;


////////
// Types
////////

// A ConvertPGPdiskInfo structure contains information for a PGPdisk
// conversion.

typedef struct ConvertPGPdiskInfo
{
	LPCSTR			path;			// path to PGPdisk
	SecureString	*passphrase;	// pointer to passphrase
	HWND			callerHwnd;		// HWND of caller window
	HWND			progressHwnd;	// HWND of progress bar

} ConvertPGPdiskInfo;


//////////////////////////////
// Class CConvertPGPdiskThread
//////////////////////////////

class CConvertPGPdiskThread : public CWinThread
{
	DECLARE_DYNCREATE(CConvertPGPdiskThread)

public:
	DualErr	mDerr;				// holds error

			CConvertPGPdiskThread();
	virtual	~CConvertPGPdiskThread();

	DualErr	CallPGPdiskConvert(ConvertPGPdiskInfo *pCPI); 
	void	CancelPGPdiskConvert();

	//{{AFX_VIRTUAL(CConvertPGPdiskThread)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

protected:
	ConvertPGPdiskInfo	mCPI;					// info for the conversion
	PGPBoolean			mIsConversionOngoing;	// conversion happening now?
	PGPBoolean			mUserCancelFlag;		// TRUE to cancel

	//{{AFX_MSG(CConvertPGPdiskThread)
	afx_msg void OnConvertPGPdisk(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CConvertPGPdiskThread_h

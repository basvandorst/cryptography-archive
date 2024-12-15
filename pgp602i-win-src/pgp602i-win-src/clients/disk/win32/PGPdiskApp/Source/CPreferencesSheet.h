//////////////////////////////////////////////////////////////////////////////
// CPreferencesSheet.h
//
// Declaration of class CPreferencesSheet.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPreferencesSheet.h,v 1.1.2.5 1998/07/06 08:57:14 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPreferencesSheet_h	// [
#define Included_CPreferencesSheet_h

#include "DualErr.h"

#include "CPrefsAutoUnmountPage.h"
#include "CPrefsUnmountHotKeyPage.h"


//////////////////////////
// Class CPreferencesSheet
//////////////////////////

class CPreferencesSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CPreferencesSheet)

public:
	DualErr mInitErr;

			CPreferencesSheet();
	virtual	~CPreferencesSheet();

	DualErr	DisplayPreferences();

	//{{AFX_VIRTUAL(CPreferencesSheet)
	public:
	virtual BOOL DestroyWindow();
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

protected:
	static PGPUInt8			mLastPageViewed;		// last page viewed

	CPrefsAutoUnmountPage	mAutoUnmountPage;		// the Auto-Unmount page
	CPrefsUnmountHotKeyPage	mHotKeyPage;			// the HotKey page
	
	//{{AFX_MSG(CPreferencesSheet)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // ] Included_CPreferencesSheet_h

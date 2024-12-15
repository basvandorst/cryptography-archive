//////////////////////////////////////////////////////////////////////////////
// CPrefsUnmountHotKeyPage.h
//
// Declaration of class CPrefsUnmountHotKeyPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPrefsUnmountHotKeyPage.h,v 1.1.2.4 1998/07/06 08:57:16 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPrefsUnmountHotKeyPage_h	// ]
#define Included_CPrefsUnmountHotKeyPage_h

#include "CSuperHotKeyControl.h"
#include "Resource.h"


////////////////////////////////
// Class CPrefsUnmountHotKeyPage
////////////////////////////////

class CPrefsUnmountHotKeyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsUnmountHotKeyPage)

public:
	CPrefsUnmountHotKeyPage();
	~CPrefsUnmountHotKeyPage();

	//{{AFX_DATA(CPrefsUnmountHotKeyPage)
	enum { IDD = IDD_PROP_HOTKEY };
	CSuperHotKeyControl	mHotKeyBox;
	BOOL	mHotKeyEnabledValue;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CPrefsUnmountHotKeyPage)
	public:
	virtual void OnOK();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CPrefsUnmountHotKeyPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnHotKeyChange();
	afx_msg void OnHotKeyCheck();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CPrefsUnmountHotKeyPage_h

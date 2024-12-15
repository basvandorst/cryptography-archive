//////////////////////////////////////////////////////////////////////////////
// CPrefsCompatibilityPage.h
//
// Declaration of class CPrefsCompatibilityPage
//////////////////////////////////////////////////////////////////////////////

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPrefsCompatibilityPage_h	// [
#define Included_CPrefsCompatibilityPage_h

#include "PGPdiskPfl.h"
#include "Resource.h"


////////////////////////////////
// Class CPrefsCompatibilityPage
////////////////////////////////

class CPrefsCompatibilityPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsCompatibilityPage)

public:
	CPrefsCompatibilityPage();
	~CPrefsCompatibilityPage();

	//{{AFX_DATA(CPrefsCompatibilityPage)
	enum { IDD = IDD_PROP_COMPAT };
	CButton	mCheckAllowFat32;
	BOOL	mAllowFat32Value;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CPrefsCompatibilityPage)
	public:
	virtual void OnOK();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CPrefsCompatibilityPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckFat32();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CPrefsCompatibilityPage_h

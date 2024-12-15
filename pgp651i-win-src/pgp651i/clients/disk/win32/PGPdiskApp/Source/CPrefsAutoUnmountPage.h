//////////////////////////////////////////////////////////////////////////////
// CPrefsAutoUnmountPage.h
//
// Declaration of class CPrefsAutoUnmountPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPrefsAutoUnmountPage.h,v 1.4 1998/12/14 18:58:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPrefsAutoUnmountPage_h	// [
#define Included_CPrefsAutoUnmountPage_h

#include "Resource.h"


//////////////////////////////
// Class CPrefsAutoUnmountPage
//////////////////////////////

class CPrefsAutoUnmountPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsAutoUnmountPage)

public:
	CPrefsAutoUnmountPage();
	~CPrefsAutoUnmountPage();

	//{{AFX_DATA(CPrefsAutoUnmountPage)
	enum { IDD = IDD_PROP_AUTO };
	CButton	mAutoUnmountButton;
	CButton	mCheckSleepButton;
	CButton	mCheckPreventButton;
	CSpinButtonCtrl	mMinutesSpin;
	CStatic	mMinuteText;
	CEdit	mTimeoutBox;
	UINT	mTimeoutValue;
	BOOL	mSleepValue;
	BOOL	mAutoUnmountValue;
	BOOL	mPreventSleepValue;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CPrefsAutoUnmountPage)
	public:
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual void OnOK();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	PGPBoolean ValidateTimeoutValue();

	//{{AFX_MSG(CPrefsAutoUnmountPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckSleep();
	afx_msg void OnCheckAutoUnmount();
	afx_msg void OnChangeTimeout();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCheckPrevent();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CPrefsAutoUnmountPage_h

//////////////////////////////////////////////////////////////////////////////
// CConvertWizConvertPage.h
//
// Declaration of class CConvertWizConvertPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizConvertPage.h,v 1.1.2.1 1998/10/21 01:35:05 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CConvertWizConvertPage_h	// [
#define Included_CConvertWizConvertPage_h

#include "DualErr.h"

#include "CConvertPGPdiskThread.h"
#include "Resource.h"


////////
// Types
////////

class CConvertWizardSheet;


///////////////////////////////
// Class CConvertWizConvertPage
///////////////////////////////

class CConvertWizConvertPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CConvertWizConvertPage)

public:
	CConvertWizardSheet	*mParentConvertWiz;	// pointer to parent sheet
	DualErr				mInitErr;
	PGPBoolean			mIsThreadActive;	// is the thread active?

	CConvertWizConvertPage();
	~CConvertWizConvertPage();

	//{{AFX_DATA(CConvertWizConvertPage)
	enum { IDD = IDD_CVRTWIZ_CONVERT };
	CStatic	mProgressStatic;
	CStatic	mSidebarGraphic;
	CProgressCtrl	mProgressBar;
	CString	mProgressText;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CConvertWizConvertPage)
	public:
	virtual BOOL OnSetActive();
	virtual void OnCancel();
	virtual BOOL OnQueryCancel();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	CConvertPGPdiskThread	*mConversionThread;	// thread to convert PGPdisk
	PGPBoolean				mIsCTAutoDelete;	// will it delete itself?
	PGPBoolean				mSkipConfirmCancel;	// to skip confirm cancel

	void SetProgressString(LPCSTR message);

	//{{AFX_MSG(CConvertWizConvertPage)
	afx_msg void OnFinishedConvert(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CConvertWizConvertPage_h

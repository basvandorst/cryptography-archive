//////////////////////////////////////////////////////////////////////////////
// CDiskWizCreationPage.h
//
// Declaration of class CDiskWizCreationPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizCreationPage.h,v 1.7 1998/12/14 18:56:25 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizCreationPage_h	// [
#define Included_CDiskWizCreationPage_h

#include "CCreatePGPdiskThread.h"
#include "Resource.h"


////////
// Types
////////

class CDiskWizardSheet;


/////////////////////////////
// Class CDiskWizCreationPage
/////////////////////////////

class CDiskWizCreationPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDiskWizCreationPage)

public:
	CDiskWizardSheet	*mParentDiskWiz;	// pointer to parent sheet
	DualErr				mInitErr;
	PGPBoolean			mIsThreadActive;	// is the thread active?

	CDiskWizCreationPage();
	~CDiskWizCreationPage();

	//{{AFX_DATA(CDiskWizCreationPage)
	enum { IDD = IDD_DISKWIZ_CREATION };
	CStatic	mProgressStatic;
	CStatic	mSidebarGraphic;
	CProgressCtrl	mProgressBar;
	CString	mProgressText;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CDiskWizCreationPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual void OnCancel();
	virtual BOOL OnQueryCancel();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	CCreatePGPdiskThread	*mCreationThread;	// thread to create PGPdisk
	PGPBoolean				mIsCTAutoDelete;	// will it delete itself?
	PGPBoolean				mSkipConfirmCancel;	// to skip confirm cancel

	void SetProgressString(LPCSTR message);

	//{{AFX_MSG(CDiskWizCreationPage)
	afx_msg void OnFinishedCreate(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDiskWizCreationPage_h

//////////////////////////////////////////////////////////////////////////////
// CGlobalCWizSearchPage.h
//
// Declaration of class CGlobalCWizSearchPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCWizSearchPage.h,v 1.2 1998/12/14 18:57:14 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CGlobalCWizSearchPage_h	// [
#define Included_CGlobalCWizSearchPage_h

#include "DualErr.h"

#include "CGlobalCSearchPGPdiskThread.h"
#include "Resource.h"


////////
// Types
////////

class CGlobalCWizardSheet;


///////////////////////////////
// Class CGlobalCWizSearchPage
///////////////////////////////

class CGlobalCWizSearchPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalCWizSearchPage)

public:
	CGlobalCWizardSheet *mParentGlobalCWiz;		// pointer to parent sheet
	DualErr				mInitErr;

	CGlobalCWizSearchPage();
	~CGlobalCWizSearchPage();

	//{{AFX_DATA(CGlobalCWizSearchPage)
	enum { IDD = IDD_GLOBWIZ_SEARCH };
	CStatic	mSidebarGraphic;
	CStatic	mDirNameStatic;
	CListBox	mPGPdisksList;
	CString	mDirNameText;
	CString	mSearchStatusText;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CGlobalCWizSearchPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual void OnCancel();
	virtual BOOL OnQueryCancel();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	CGlobalCSearchPGPdiskThread	*mSearchThread;		// PGPdisk search thread
	PGPBoolean					mIsCTAutoDelete;	// will it delete itself?
	PGPBoolean					mSkipConfirmCancel;	// to skip confirm cancel

	PGPBoolean					mIsThreadActive;	// is the thread active?
	PGPBoolean					mFinishedSearch;	// done with search?

	void ConvertSelectedPGPdisks();

	//{{AFX_MSG(CGlobalCWizSearchPage)
	afx_msg void OnFinishedSearch(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CGlobalCWizSearchPage_h

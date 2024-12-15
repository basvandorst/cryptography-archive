//////////////////////////////////////////////////////////////////////////////
// CGlobalCWizardSheet.h
//
// Declaration of class CGlobalCWizardSheet.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCWizardSheet.h,v 1.1.2.1 1998/10/22 22:27:42 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CGlobalCWizardSheet_h	// [
#define Included_CGlobalCWizardSheet_h

#include "DualErr.h"

#include "CGlobalCWizDonePage.h"
#include "CGlobalCWizIntroPage.h"
#include "CGlobalCWizSearchPage.h"


////////////////////////////
// Class CGlobalCWizardSheet
////////////////////////////

class CGlobalCWizardSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CGlobalCWizardSheet)

	friend class CGlobalCWizDonePage;
	friend class CGlobalCWizIntroPage;
	friend class CGlobalCWizSearchPage;

public:
	DualErr	mInitErr;

			CGlobalCWizardSheet();
	virtual	~CGlobalCWizardSheet();

	DualErr	ExecuteWizard();

	//{{AFX_VIRTUAL(CGlobalCWizardSheet)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

protected:
	CGlobalCWizDonePage			mDonePage;			// convert done page
	CGlobalCWizIntroPage		mIntroPage;			// convert intro page
	CGlobalCWizSearchPage		mSearchPage;		// search page

	DualErr			mDerr;				// pages report	errors here

	CBitmap			mSidebarBitmap;		// sidebar graphic
	CPalette		mSidebarPalette;	// sidebar palette

	//{{AFX_MSG(CGlobalCWizardSheet)
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // ] Included_CGlobalCWizardSheet_h

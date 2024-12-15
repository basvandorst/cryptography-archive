//////////////////////////////////////////////////////////////////////////////
// CConvertWizardSheet.h
//
// Declaration of class CConvertWizardSheet.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizardSheet.h,v 1.2 1998/12/14 18:56:15 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CConvertWizardSheet_h	// [
#define Included_CConvertWizardSheet_h

#include "DualErr.h"
#include "SecureMemory.h"

#include "CConvertWizConvertPage.h"
#include "CConvertWizDonePage.h"
#include "CConvertWizIntroPage.h"
#include "CConvertWizRandomDataPage.h"


////////////////////////////
// Class CConvertWizardSheet
////////////////////////////

class CConvertWizardSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CConvertWizardSheet)

	friend class CConvertWizConvertPage;
	friend class CConvertWizDonePage;
	friend class CConvertWizIntroPage;
	friend class CConvertWizRandomDataPage;

public:
	DualErr	mInitErr;

			CConvertWizardSheet();
	virtual	~CConvertWizardSheet();

	DualErr	ExecuteWizard(LPCSTR path, PGPBoolean restartingConversion);

	//{{AFX_VIRTUAL(CConvertWizardSheet)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	CConvertWizConvertPage		mConvertPage;		// disk conversion page
	CConvertWizDonePage			mDonePage;			// convert done page
	CConvertWizIntroPage		mIntroPage;			// convert intro page
	CConvertWizRandomDataPage	mRandomDataPage;	// random data page

	DualErr			mDerr;				// pages report	errors here

	LPCSTR			mPath;
	PGPBoolean		mRestartingConversion;

	SecureMemory	mLockedMem;
	SecureString	*mPassphrase;

	CBitmap			mSidebarBitmap;		// sidebar graphic
	CPalette		mSidebarPalette;	// sidebar palette

	//{{AFX_MSG(CConvertWizardSheet)
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // ] Included_CConvertWizardSheet_h

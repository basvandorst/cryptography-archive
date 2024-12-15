//////////////////////////////////////////////////////////////////////////////
// CDiskWizardSheet.h
//
// Declaration of class CDiskWizardSheet.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizardSheet.h,v 1.2.2.11.2.1 1998/08/23 03:01:48 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizardSheet_h	// [
#define Included_CDiskWizardSheet_h

#include "DualErr.h"

#include "CDiskWizCreationPage.h"
#include "CDiskWizDonePage.h"
#include "CDiskWizIntroPage.h"
#include "CDiskWizPassphrasePage.h"
#include "CDiskWizADKPage.h"
#include "CDiskWizRandomDataPage.h"
#include "CDiskWizVolumeInfoPage.h"


/////////////////////////
// Class CDiskWizardSheet
/////////////////////////

class CDiskWizardSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CDiskWizardSheet)

	friend class CDiskWizCreationPage;
	friend class CDiskWizDonePage;
	friend class CDiskWizIntroPage;
	friend class CDiskWizPassphrasePage;
	friend class CDiskWizADKPage;
	friend class CDiskWizRandomDataPage;
	friend class CDiskWizVolumeInfoPage;

public:
	DualErr	mInitErr;

			CDiskWizardSheet();
	virtual	~CDiskWizardSheet();

	DualErr	ExecuteWizard(LPCSTR path = kEmptyString);

	//{{AFX_VIRTUAL(CDiskWizardSheet)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	CDiskWizCreationPage	mCreationPage;		// disk creation wizard page
	CDiskWizDonePage		mDonePage;			// done message page
	CDiskWizIntroPage		mIntroPage;			// intro message page
	CDiskWizPassphrasePage	mPassphrasePage;	// passphrase page
	CDiskWizADKPage			mADKPage;			// ADK page
	CDiskWizRandomDataPage	mRandomDataPage;	// random data page
	CDiskWizVolumeInfoPage	mVolumeInfoPage;	// volume information page

	DualErr					mDerr;				// pages report	errors here

	CString					mPath;				// path to new PGPdisk
	File					mStolenFile;		// hold file handle
	PGPBoolean				mAreWeReplacing;	// replacing existing file?

	CBitmap					mSidebarBitmap;		// sidebar graphic
	CPalette				mSidebarPalette;	// sidebar palette

	void UnmountAndRemountPGPdisk(PGPUInt8 drive);

	//{{AFX_MSG(CDiskWizardSheet)
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // ] Included_CDiskWizardSheet_h

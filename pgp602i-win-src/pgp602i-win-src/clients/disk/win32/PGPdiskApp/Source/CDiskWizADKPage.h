//////////////////////////////////////////////////////////////////////////////
// CDiskWizADKPage.h
//
// Declaration of class CDiskWizADKPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizADKPage.h,v 1.1.2.2 1998/07/22 18:28:48 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizADKPage_h	// [
#define Included_CDiskWizADKPage_h

#include "DualErr.h"
#include "Resource.h"


////////
// Types
////////

class CDiskWizardSheet;


////////////////////////
// Class CDiskWizADKPage
////////////////////////

class CDiskWizADKPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDiskWizADKPage)

public:
	CDiskWizardSheet	*mParentDiskWiz;	// pointer to parent sheet
	PGPBoolean			mSkipConfirmCancel;	// to skip confirm cancel

	PGPBoolean			mUseADK;			// use ADK key?

	CDiskWizADKPage();
	~CDiskWizADKPage();

	//{{AFX_DATA(CDiskWizADKPage)
	enum { IDD = IDD_DISKWIZ_ADK };
	CStatic	mSidebarGraphic;
	CString	mADKString;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CDiskWizADKPage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnQueryCancel();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDiskWizADKPage)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDiskWizADKPage_h

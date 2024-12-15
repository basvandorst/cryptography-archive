//////////////////////////////////////////////////////////////////////////////
// CDiskWizDonePage.h
//
// Declaration of class CDiskWizDonePage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizDonePage.h,v 1.5 1998/12/14 18:56:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizDonePage_h	// [
#define Included_CDiskWizDonePage_h

#include "Resource.h"


////////
// Types
////////

class CDiskWizardSheet;


/////////////////////////
// Class CDiskWizDonePage
/////////////////////////

class CDiskWizDonePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDiskWizDonePage)

public:
	CDiskWizardSheet *mParentDiskWiz;	// pointer to parent sheet

	CDiskWizDonePage();
	~CDiskWizDonePage();

	//{{AFX_DATA(CDiskWizDonePage)
	enum { IDD = IDD_DISKWIZ_DONE };
	CStatic	mSidebarGraphic;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CDiskWizDonePage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDiskWizDonePage)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDiskWizDonePage_h

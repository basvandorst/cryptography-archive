//////////////////////////////////////////////////////////////////////////////
// CDiskWizIntroPage.h
//
// Declaration of class CDiskWizIntroPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizIntroPage.h,v 1.5 1998/12/14 18:56:31 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizIntroPage_h	// [
#define Included_CDiskWizIntroPage_h

#include "DualErr.h"
#include "Resource.h"


////////
// Types
////////

class CDiskWizardSheet;


//////////////////////////
// Class CDiskWizIntroPage
//////////////////////////

class CDiskWizIntroPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDiskWizIntroPage)

public:
	CDiskWizardSheet *mParentDiskWiz;	// pointer to parent sheet

	CDiskWizIntroPage();
	~CDiskWizIntroPage();

	//{{AFX_DATA(CDiskWizIntroPage)
	enum { IDD = IDD_DISKWIZ_INTRO };
	CStatic	mSidebarGraphic;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CDiskWizIntroPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDiskWizIntroPage)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DualErr AskForPath();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDiskWizIntroPage_h

//////////////////////////////////////////////////////////////////////////////
// CGlobalCWizIntroPage.h
//
// Declaration of class CGlobalCWizIntroPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCWizIntroPage.h,v 1.2 1998/12/14 18:57:10 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CGlobalCWizIntroPage_h	// [
#define Included_CGlobalCWizIntroPage_h

#include "DualErr.h"
#include "Resource.h"


////////
// Types
////////

class CGlobalCWizardSheet;


/////////////////////////////
// Class CGlobalCWizIntroPage
/////////////////////////////

class CGlobalCWizIntroPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalCWizIntroPage)

public:
	CGlobalCWizardSheet *mParentGlobalCWiz;		// pointer to parent sheet

	CGlobalCWizIntroPage();
	~CGlobalCWizIntroPage();

	//{{AFX_DATA(CGlobalCWizIntroPage)
	enum { IDD = IDD_GLOBWIZ_INTRO };
	CStatic	mSidebarGraphic;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CGlobalCWizIntroPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CGlobalCWizIntroPage)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CGlobalCWizIntroPage_h

//////////////////////////////////////////////////////////////////////////////
// CGlobalCWizDonePage.h
//
// Declaration of class CGlobalCWizDonePage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCWizDonePage.h,v 1.1.2.1 1998/10/22 22:27:34 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CGlobalCWizDonePage_h	// [
#define Included_CGlobalCWizDonePage_h

#include "Resource.h"


////////
// Types
////////

class CGlobalCWizardSheet;


////////////////////////////
// Class CGlobalCWizDonePage
////////////////////////////

class CGlobalCWizDonePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalCWizDonePage)

public:
	CGlobalCWizardSheet *mParentGlobalCWiz;		// pointer to parent sheet

	CGlobalCWizDonePage();
	~CGlobalCWizDonePage();

	//{{AFX_DATA(CGlobalCWizDonePage)
	enum { IDD = IDD_GLOBWIZ_DONE };
	CStatic	mSidebarGraphic;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CGlobalCWizDonePage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CGlobalCWizDonePage)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CGlobalCWizDonePage_h

//////////////////////////////////////////////////////////////////////////////
// CConvertWizDonePage.h
//
// Declaration of class CConvertWizDonePage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizDonePage.h,v 1.2 1998/12/14 18:56:04 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CConvertWizDonePage_h	// [
#define Included_CConvertWizDonePage_h

#include "Resource.h"


////////
// Types
////////

class CConvertWizardSheet;


////////////////////////////
// Class CConvertWizDonePage
////////////////////////////

class CConvertWizDonePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CConvertWizDonePage)

public:
	CConvertWizardSheet *mParentConvertWiz;		// pointer to parent sheet

	CConvertWizDonePage();
	~CConvertWizDonePage();

	//{{AFX_DATA(CConvertWizDonePage)
	enum { IDD = IDD_CVRTWIZ_DONE };
	CStatic	mSidebarGraphic;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CConvertWizDonePage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CConvertWizDonePage)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CConvertWizDonePage_h

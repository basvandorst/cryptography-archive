//////////////////////////////////////////////////////////////////////////////
// CConvertWizIntroPage.h
//
// Declaration of class CConvertWizIntroPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizIntroPage.h,v 1.2 1998/12/14 18:56:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CConvertWizIntroPage_h	// [
#define Included_CConvertWizIntroPage_h

#include "DualErr.h"
#include "Resource.h"


////////
// Types
////////

class CConvertWizardSheet;


/////////////////////////////
// Class CConvertWizIntroPage
/////////////////////////////

class CConvertWizIntroPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CConvertWizIntroPage)

public:
	CConvertWizardSheet *mParentConvertWiz;		// pointer to parent sheet

	CConvertWizIntroPage();
	~CConvertWizIntroPage();

	//{{AFX_DATA(CConvertWizIntroPage)
	enum { IDD = IDD_CVRTWIZ_INTRO };
	CStatic	mSidebarGraphic;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CConvertWizIntroPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CConvertWizIntroPage)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DualErr AskForMasterPassphrase();
	DualErr AskForPublicKeyPassphrase();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CConvertWizIntroPage_h

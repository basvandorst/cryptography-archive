//////////////////////////////////////////////////////////////////////////////
// CConvertWizRandomDataPage.h
//
// Declaration of class CConvertWizRandomDataPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizRandomDataPage.h,v 1.1.2.1 1998/10/21 01:35:11 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CConvertWizRandomDataPage_h	// [
#define Included_CConvertWizRandomDataPage_h

#include "DualErr.h"
#include "PGPdiskRandomPool.h"

#include "Resource.h"


////////
// Types
////////

class CConvertWizardSheet;


//////////////////////////////////
// Class CConvertWizRandomDataPage
//////////////////////////////////

class CConvertWizRandomDataPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CConvertWizRandomDataPage)

public:
	CConvertWizardSheet	*mParentConvertWiz;	// pointer to parent sheet
	DualErr				mInitErr;
	PGPBoolean			mIsDoneCollecting;	// got all we need?
	PGPdiskRandomPool	mRandomPool;		// our pool for random data

	CConvertWizRandomDataPage();
	~CConvertWizRandomDataPage();

	//{{AFX_DATA(CConvertWizRandomDataPage)
	enum { IDD = IDD_CVRTWIZ_RANDOM };
	CStatic	mSidebarGraphic;
	CProgressCtrl	mRandomDataBar;
	CString	mRandomMessage;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CConvertWizRandomDataPage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	PGPUInt32	mRequiredDataCount;		// entropy still needed
	PGPUInt32	mProcessedDataCount;	// total entropy required

	void ProcessChar(PGPUInt8 nChar);

	//{{AFX_MSG(CConvertWizRandomDataPage)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CConvertWizRandomDataPage_h

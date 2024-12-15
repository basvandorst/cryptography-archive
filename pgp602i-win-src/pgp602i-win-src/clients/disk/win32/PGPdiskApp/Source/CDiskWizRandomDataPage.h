//////////////////////////////////////////////////////////////////////////////
// CDiskWizRandomDataPage.h
//
// Declaration of class CDiskWizRandomDataPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizRandomDataPage.h,v 1.1.2.6 1998/07/06 08:56:51 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizRandomDataPage_h	// [
#define Included_CDiskWizRandomDataPage_h

#include "DualErr.h"
#include "PGPdiskRandomPool.h"

#include "Resource.h"


////////
// Types
////////

class CDiskWizardSheet;


///////////////////////////////
// Class CDiskWizRandomDataPage
///////////////////////////////

class CDiskWizRandomDataPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDiskWizRandomDataPage)

public:
	CDiskWizardSheet	*mParentDiskWiz;	// pointer to parent sheet
	DualErr				mInitErr;
	PGPBoolean			mIsDoneCollecting;	// got all we need?
	PGPdiskRandomPool	mRandomPool;		// our pool for random data

	CDiskWizRandomDataPage();
	~CDiskWizRandomDataPage();

	//{{AFX_DATA(CDiskWizRandomDataPage)
	enum { IDD = IDD_DISKWIZ_RANDOM };
	CStatic	mSidebarGraphic;
	CProgressCtrl	mRandomDataBar;
	CString	mRandomMessage;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CDiskWizRandomDataPage)
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

	//{{AFX_MSG(CDiskWizRandomDataPage)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDiskWizRandomDataPage_h

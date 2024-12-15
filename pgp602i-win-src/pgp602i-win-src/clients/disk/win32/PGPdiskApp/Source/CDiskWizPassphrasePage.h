//////////////////////////////////////////////////////////////////////////////
// CDiskWizPassphrasePage.h
//
// Declaration of class CDiskWizPassphrasePage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizPassphrasePage.h,v 1.1.2.5 1998/07/06 08:56:50 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizPassphrasePage_h	// [
#define Included_CDiskWizPassphrasePage_h

#include "DualErr.h"
#include "CSecureEdit.h"

#include "Resource.h"


////////
// Types
////////

class CDiskWizardSheet;


/////////////////////////////
// Class CDiskWizPassphrasePage
/////////////////////////////

class CDiskWizPassphrasePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDiskWizPassphrasePage)

public:
	CDiskWizardSheet	*mParentDiskWiz;	// pointer to parent sheet
	DualErr				mInitErr;

	CDiskWizPassphrasePage();
	~CDiskWizPassphrasePage();

	//{{AFX_DATA(CDiskWizPassphrasePage)
	enum { IDD = IDD_DISKWIZ_PASSPHRASE };
	CStatic	mSidebarGraphic;
	CProgressCtrl	mQualityBar;
	CSecureEdit	mPassphraseEdit;
	CSecureEdit	mConfirmationEdit;
	CStatic	mCapsLockStatic;
	BOOL	mHideTypeValue;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CDiskWizPassphrasePage)
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	CSecureEdit *mIHadFocus;	// edit that had focus

	void SetCapsLockMessageState();

	//{{AFX_MSG(CDiskWizPassphrasePage)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnChangePassphrase();
	afx_msg void OnHideTypeCheck();
	afx_msg void OnKillFocusConfirmEdit();
	afx_msg void OnKillFocusPassphraseEdit();
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDiskWizPassphrasePage_h

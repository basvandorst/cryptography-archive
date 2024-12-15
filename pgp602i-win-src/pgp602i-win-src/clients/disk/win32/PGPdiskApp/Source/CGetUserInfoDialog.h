//////////////////////////////////////////////////////////////////////////////
// CGetUserInfoDialog.h
//
// Declaration of class CGetUserInfoDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGetUserInfoDialog.h,v 1.1.2.4 1998/07/22 23:37:59 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CGetUserInfoDialog_h	// [
#define Included_CGetUserInfoDialog_h

#include "DualErr.h"

#include "Resource.h"


///////////////////////////
// Class CGetUserInfoDialog
///////////////////////////

class CGetUserInfoDialog : public CDialog
{
public:
			CGetUserInfoDialog(CWnd *pParent = NULL);

	DualErr	AskForUserInfo();

	//{{AFX_DATA(CGetUserInfoDialog)
	enum { IDD = IDD_GETUSERINFO_DLG };
	CEdit	mUserStatic;
	CEdit	mOrgStatic;
	CButton	mCancelButton;
	CButton	mOkButton;
	CString	mUserText;
	CString	mOrgText;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CGetUserInfoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	void UpdateOkButton();

	//{{AFX_MSG(CGetUserInfoDialog)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNameEdit();
	afx_msg void OnChangeOrgEdit();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CGetUserInfoDialog_h

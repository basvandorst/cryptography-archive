//////////////////////////////////////////////////////////////////////////////
// CNagBuyDialog.h
//
// Declaration of class CNagBuyDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CNagBuyDialog.h,v 1.2.2.4.2.1 1998/10/21 01:35:17 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CNagBuyDialog_h	// [
#define Included_CNagBuyDialog_h

#include "DualErr.h"
#include "Resource.h"


//////////////////////
// Class CNagBuyDialog
//////////////////////

class CNagBuyDialog : public CDialog
{
public:
			CNagBuyDialog(CWnd *pParent = NULL);

	DualErr	DisplayDialog(LPCSTR message);

	//{{AFX_DATA(CNagBuyDialog)
	enum { IDD = IDD_NAGBUY_DLG };
	CButton	mOkButton;
	CButton	mCancelButton;
	CButton	mBuyNowButton;
	CString	mNagMessageText;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CNagBuyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	void UpdateOkButton();

	//{{AFX_MSG(CNagBuyDialog)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	virtual BOOL OnInitDialog();
	afx_msg void OnBuyNowButton();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CNagBuyDialog_h

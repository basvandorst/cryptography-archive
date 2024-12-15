//////////////////////////////////////////////////////////////////////////////
// CAboutBoxDialog.h
//
// Declaration of class CAboutBoxDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CAboutBoxDialog.h,v 1.4 1998/12/14 18:55:47 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CAboutBoxDialog_h	// [
#define Included_CAboutBoxDialog_h

#include "Resource.h"


////////////////////////
// Class CAboutBoxDialog
////////////////////////

class CAboutBoxDialog : public CDialog
{
public:
			CAboutBoxDialog(CWnd *pParent = NULL);

	DualErr	DisplayAboutBox();

	//{{AFX_DATA(CAboutBoxDialog)
	enum { IDD = IDD_ABOUTBOX_DLG };
	CButton	mPGPLinkButton;
	CButton	mCreditsButton;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAboutBoxDialog)
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	PGPBoolean	mShowingCredits;	// showing credits or graphics?

	BITMAP		mBitmapInfo;		// info for the bitmap
	CBitmap		mAboutBitmap;		// about graphic
	CBitmap		mCreditsBitmap;		// credits graphic
	CPalette	mAboutPalette;		// about palette
	CPalette	mCreditsPalette;	// credits palette

	COLORREF	mRegTextColor;		// color for registration text
	COLORREF	mVersionTextColor;	// color for version text

	//{{AFX_MSG(CAboutBoxDialog)
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnCreditsButton();
	afx_msg void OnPGPLinkButton();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CAboutBoxDialog_h

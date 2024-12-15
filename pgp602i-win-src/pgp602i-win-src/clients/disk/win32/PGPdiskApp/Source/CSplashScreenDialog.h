//////////////////////////////////////////////////////////////////////////////
// CSplashScreenDialog.h
//
// Declaration of class CSplashScreenDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSplashScreenDialog.h,v 1.1.2.3 1998/07/06 08:57:23 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CSplashScreenDialog_h	// [
#define Included_CSplashScreenDialog_h

#include "DualErr.h"

#include "Resource.h"


////////////////////////////
// Class CSplashScreenDialog
////////////////////////////

class CSplashScreenDialog : public CDialog
{
public:
			CSplashScreenDialog(CWnd *pParent = NULL);

	DualErr	ShowSplashScreen();

	//{{AFX_DATA(CSplashScreenDialog)
	enum { IDD = IDD_SPLASH_DLG };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CSplashScreenDialog)
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	BITMAP		mBitmapInfo;		// info for the bitmap
	CBitmap		mSplashBitmap;		// splash graphic
	CPalette	mSplashPalette;		// splash palette

	COLORREF	mRegTextColor;		// color for registration text
	COLORREF	mVersionTextColor;	// color for version text
	
	PGPUInt32	mTimer;				// timer ID

	//{{AFX_MSG(CSplashScreenDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CSplashScreenDialog_h

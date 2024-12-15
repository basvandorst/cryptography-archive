//////////////////////////////////////////////////////////////////////////////
// CSplashScreenDialog.cpp
//
// Implementation of the CSplashScreenDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSplashScreenDialog.cpp,v 1.1.2.6 1998/07/29 00:45:07 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CPGPdiskApp.h"
#include "CSplashScreenDialog.h"
#include "Globals.h"


////////////
// Constants
////////////

const PGPUInt32 kSplashScreenLifeMs = 2000;		// show for this long

const PGPUInt32 kSplashScreenVersionTextX	= 433;
const PGPUInt32 kSplashScreenVersionTextY	= 208;

const PGPUInt32 kSplashScreenRegTextX	= 296;
const PGPUInt32 kSplashScreenRegTextY	= 242;


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CSplashScreenDialog, CDialog)
	//{{AFX_MSG_MAP(CSplashScreenDialog)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CSplashScreenDialog public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

// The CSplashScreenDialog default constructor.

CSplashScreenDialog::CSplashScreenDialog(CWnd *pParent) 
	: CDialog(CSplashScreenDialog::IDD, pParent)
{
	try
	{
		//{{AFX_DATA_INIT(CSplashScreenDialog)
		//}}AFX_DATA_INIT

		mTimer = NULL;
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// ShowSplashScreen is a wrapper around DoModal. The dialog times out after a
// certain number of seconds.

DualErr 
CSplashScreenDialog::ShowSplashScreen()
{
	DualErr		derr;
	PGPInt32	result;
	
	result = DoModal();

	switch (result)
	{
	case -1:
		derr = DualErr(kPGDMinorError_DialogDisplayFailed);
		break;
	}

	return derr;
}


//////////////////////////////////////////////////////////////////////////
// CSplashScreenDialog protected custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////

// The CSplashScreenDialog default data-exchange function.

void 
CSplashScreenDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CSplashScreenDialog)
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


/////////////////////////////////////////////////////////
// CSplashScreenDialog protected default message handlers
/////////////////////////////////////////////////////////

// OnInitDialog is called when the dialog box is created.

	BOOL 
CSplashScreenDialog::OnInitDialog()
{
	HDC			hDC;
	PGPBoolean	loadedBitmap;
	PGPUInt32	numBits;

	CDialog::OnInitDialog();

	// Get screen depth.
	hDC = ::GetDC(NULL);		// DC for desktop
	numBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	::ReleaseDC(NULL, hDC);

	// Load the bitmap and create a palette for it.
	if (numBits <= 1)
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SPLASHSCREEN1, 
			&mSplashBitmap, &mSplashPalette);

		mRegTextColor		= RGB(255, 255, 255);
		mVersionTextColor	= RGB(255, 255, 255);
	}
	else if (numBits <= 4)
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SPLASHSCREEN4, 
			&mSplashBitmap, &mSplashPalette);

		mRegTextColor		= RGB(255, 255, 255);
		mVersionTextColor	= RGB(0, 0, 0);
	}
	else
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SPLASHSCREEN8, 
			&mSplashBitmap, &mSplashPalette);

		mRegTextColor		= RGB(255, 255, 255);
		mVersionTextColor	= RGB(0, 0, 0);
	}

	// Determine its width and height.
	if (!loadedBitmap || !mSplashBitmap.GetBitmap(&mBitmapInfo))
	{
		EndDialog(IDCANCEL);
	}

	// Make ourselves same size as bitmap and put ourselves on top.
	SetWindowPos(&CWnd::wndTopMost, 0, 0, mBitmapInfo.bmWidth, 
		mBitmapInfo.bmHeight, SWP_NOMOVE);

	// Center on screen.
	CenterWindow(GetDesktopWindow());

	// Put us in front.
	SetForegroundWindow();

	// Show now to thwart evil MFC auto-centering.
	ShowWindow(SW_SHOW);

	// Create our timeout.
	mTimer = SetTimer(1, kSplashScreenLifeMs, NULL);

	if (mTimer == 0)
	{
		EndDialog(IDCANCEL);
	}

	return TRUE;
}

// OnKeyDown is overridden so we can close the dialog on a keystroke.

void 
CSplashScreenDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	KillTimer(mTimer);
	EndDialog(IDOK);
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

// OnLButtonDown is overridden so we can close the dialog on a click.

void 
CSplashScreenDialog::OnLButtonDown(UINT nFlags, CPoint point) 
{
	KillTimer(mTimer);
	EndDialog(IDOK);
	
	CDialog::OnLButtonDown(nFlags, point);
}

// We override OnPaint to paint our splash screen.

void 
CSplashScreenDialog::OnPaint() 
{
	CDC			memDC;
	CPaintDC	DC(this);
	HBITMAP		oldBitmap;

	DC.SelectPalette(&mSplashPalette, FALSE);
	DC.RealizePalette();

	memDC.CreateCompatibleDC(&DC);
	memDC.SelectPalette(&mSplashPalette, FALSE);
	memDC.RealizePalette();

	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so don't
	// fool with it.

	oldBitmap = (HBITMAP) SelectObject(memDC, mSplashBitmap);

	// Draw the user and version strings.
	App->PaintRegistrationInfo(&memDC, kSplashScreenRegTextX, 
		kSplashScreenRegTextY, mRegTextColor, this);
	App->PaintVersionInfo(&memDC, kSplashScreenVersionTextX, 
		kSplashScreenVersionTextY, mVersionTextColor, this);

	DC.BitBlt(0, 0, mBitmapInfo.bmWidth, mBitmapInfo.bmHeight, &memDC, 0, 0, 
		SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}

// OnTimer is overridden so we can kill the dialog when its time runs out.

void 
CSplashScreenDialog::OnTimer(UINT nIDEvent) 
{
	KillTimer(mTimer);
	EndDialog(IDOK);

	CDialog::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////////
// CAboutBoxDialog.cpp
//
// Implementation of the CAboutBoxDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CAboutBoxDialog.cpp,v 1.5 1998/12/14 18:55:46 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"

#include "CAboutBoxDialog.h"
#include "CMainDialog.h"
#include "CPGPdiskApp.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


////////////
// Constants
////////////

const PGPUInt32 kAboutBoxVersionTextX	= 433;
const PGPUInt32 kAboutBoxVersionTextY	= 208;

const PGPUInt32 kAboutBoxRegTextX	= 296;
const PGPUInt32 kAboutBoxRegTextY	= 236;


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDOK,				IDH_PGPDISKAPP_ABOUTBOXOK, 
	IDC_CREDITS,		IDH_PGPDISKAPP_ABOUTBOXCREDITS, 
	IDC_PGP_LINK,		IDH_PGPDISKAPP_ABOUTBOXPGPLINK, 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CAboutBoxDialog, CDialog)
	//{{AFX_MSG_MAP(CAboutBoxDialog)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CREDITS, OnCreditsButton)
	ON_BN_CLICKED(IDC_PGP_LINK, OnPGPLinkButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// CAboutBoxDialog public custom functions and non-default message handlers
///////////////////////////////////////////////////////////////////////////

// The CAboutBoxDialog default constructor.

CAboutBoxDialog::CAboutBoxDialog(CWnd *pParent) 
	: CDialog(CAboutBoxDialog::IDD, pParent)
{
	try
	{
		//{{AFX_DATA_INIT(CAboutBoxDialog)
		//}}AFX_DATA_INIT

		mShowingCredits = FALSE;
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// DisplayAboutBox is a wrapper around DoModal.

DualErr 
CAboutBoxDialog::DisplayAboutBox()
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


//////////////////////////////////////////////////////////////////////////////
// CAboutBoxDialog protected custom functions and non-default message handlers
//////////////////////////////////////////////////////////////////////////////

// The CAboutBoxDialog default data-exchange function.

void 
CAboutBoxDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CAboutBoxDialog)
		DDX_Control(pDX, IDC_PGP_LINK, mPGPLinkButton);
		DDX_Control(pDX, IDC_CREDITS, mCreditsButton);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnCreditsButton is called when the credits button is clicked. Switch
// between the credits and graphics screen.

void 
CAboutBoxDialog::OnCreditsButton() 
{
	if (mShowingCredits)
	{
		mShowingCredits = FALSE;
		mCreditsButton.SetWindowText("&Credits");
	}
	else
	{
		mShowingCredits = TRUE;
		mCreditsButton.SetWindowText("&Info");
	}

	InvalidateRect(NULL);
}

// OnPGPLinkButton is called when the user clicks on the "www.pgpi.com" button.
// We go to PGP's web site.

void 
CAboutBoxDialog::OnPGPLinkButton() 
{
	ShellExecute(App->mMainDialog->GetSafeHwnd(), "open", 
		GetCommonString(kPGPdiskURL), NULL, "C:\\", SW_SHOWNORMAL);

	EndDialog(IDOK);
}


/////////////////////////////////////////////////////
// CAboutBoxDialog protected default message handlers
/////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CAboutBoxDialog::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnInitDialog is called when the dialog box is created.

BOOL 
CAboutBoxDialog::OnInitDialog()
{
	HDC			hDC;
	PGPBoolean	loadedBitmap;
	PGPUInt32	numBits;

	CDialog::OnInitDialog();

	// Get screen depth.
	hDC = ::GetDC(NULL);		// DC for desktop
	numBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	::ReleaseDC(NULL, hDC);

	// Load the bitmaps and create palettes for them.
	if (numBits <= 1)
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SPLASHSCREEN1, 
			&mAboutBitmap, &mAboutPalette);

		if (loadedBitmap)
		{
			loadedBitmap = App->GetBitmapAndPalette(IDB_CREDITS1, 
				&mCreditsBitmap, &mCreditsPalette);
		}

		mRegTextColor		= RGB(255, 255, 255);
		mVersionTextColor	= RGB(255, 255, 255);
	}
	else if (numBits <= 4)
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SPLASHSCREEN4, 
			&mAboutBitmap, &mAboutPalette);

		if (loadedBitmap)
		{
			loadedBitmap = App->GetBitmapAndPalette(IDB_CREDITS4, 
				&mCreditsBitmap, &mCreditsPalette);
		}

		mRegTextColor		= RGB(255, 255, 255);
		mVersionTextColor	= RGB(255, 255, 255);
	}
	else
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SPLASHSCREEN8, 
			&mAboutBitmap, &mAboutPalette);

		if (loadedBitmap)
		{
			loadedBitmap = App->GetBitmapAndPalette(IDB_CREDITS8, 
				&mCreditsBitmap, &mCreditsPalette);
		}

		mRegTextColor		= RGB(255, 255, 255);
		mVersionTextColor	= RGB(255, 255, 255);
	}

	// Determine its width and height.
	if (!loadedBitmap || !mAboutBitmap.GetBitmap(&mBitmapInfo))
	{
		EndDialog(IDCANCEL);
	}
	
	// Center on screen.
	CenterWindow(GetDesktopWindow());

	// Put us in front.
	SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(this);

	// Show now to thwart evil MFC auto-centering.
	ShowWindow(SW_SHOW);

	return TRUE;
}

// We override OnPaint to paint our graphics.

void 
CAboutBoxDialog::OnPaint() 
{
	CDC			memDC;
	CPaintDC	DC(this);
	HBITMAP		oldBitmap;

	if (mShowingCredits)
	{
		DC.SelectPalette(&mCreditsPalette, FALSE);
		DC.RealizePalette();

		memDC.CreateCompatibleDC(&DC);
		memDC.SelectPalette(&mCreditsPalette, FALSE);
		memDC.RealizePalette();

		// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
		// don't fool with it.

		oldBitmap = (HBITMAP) SelectObject(memDC, mCreditsBitmap);
	}
	else
	{
		DC.SelectPalette(&mAboutPalette, FALSE);
		DC.RealizePalette();

		memDC.CreateCompatibleDC(&DC);
		memDC.SelectPalette(&mAboutPalette, FALSE);
		memDC.RealizePalette();

		// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
		// don't fool with it.

		oldBitmap = (HBITMAP) SelectObject(memDC, mAboutBitmap);
	}

	// Draw the user and version strings.
	App->PaintRegistrationInfo(&memDC, kAboutBoxRegTextX, 
		kAboutBoxRegTextY, mRegTextColor, this);

	if (!mShowingCredits)
	{
		App->PaintVersionInfo(&memDC, kAboutBoxVersionTextX, 
			kAboutBoxVersionTextY, mVersionTextColor, this);
	}

	DC.BitBlt(0, 0, mBitmapInfo.bmWidth, mBitmapInfo.bmHeight, &memDC, 0, 0, 
		SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}

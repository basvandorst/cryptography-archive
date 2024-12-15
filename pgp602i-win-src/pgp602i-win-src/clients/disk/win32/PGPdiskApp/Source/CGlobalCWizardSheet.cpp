//////////////////////////////////////////////////////////////////////////////
// CGlobalCWizardSheet.cpp
//
// Implementation of class CGlobalCWizardSheet.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCWizardSheet.cpp,v 1.1.2.1 1998/10/22 22:27:40 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <new.h>

#include "Required.h"
#include "StringAssociation.h"

#include "CGlobalCWizardSheet.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDCANCEL,		IDH_PGPDISKAPP_DISKWIZSHEETCANCEL, 
	ID_WIZBACK,		IDH_PGPDISKAPP_DISKWIZSHEETBACK, 
	ID_WIZNEXT,		IDH_PGPDISKAPP_DISKWIZSHEETNEXT, 
	ID_WIZFINISH,	IDH_PGPDISKAPP_DISKWIZSHEETFINISH, 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CGlobalCWizardSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CGlobalCWizardSheet)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CGlobalCWizardSheet public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CGlobalCWizardSheet, CPropertySheet)

// The CGlobalCWizardSheet constructor adds the wizard pages we implement
// to the wizard sheet.

CGlobalCWizardSheet::CGlobalCWizardSheet() :
		CPropertySheet(GetCommonString(kPGPdiskGlobalCWizSheetTitleString), 
			NULL, 0)
{
	try
	{
		mDonePage.mParentGlobalCWiz		= this;
		mIntroPage.mParentGlobalCWiz	= this;
		mSearchPage.mParentGlobalCWiz	= this;

		AddPage(&mIntroPage);
		AddPage(&mSearchPage);
		AddPage(&mDonePage);

		// Exorcise the big help button.
		m_psh.dwFlags &= ~PSH_HASHELP;
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}
}

// The CGlobalCWizardSheet destructor.

CGlobalCWizardSheet::~CGlobalCWizardSheet()
{
}

// ExecuteWizard displays the wizard and searches for unconverted PGPdisks.

DualErr 
CGlobalCWizardSheet::ExecuteWizard()
{
	DualErr derr;

	try
	{
		PGPInt32 result;

		SetWizardMode();

		// Invoke the wizard.
		result = DoModal();

		switch (result)
		{
		case -1:
			derr = DualErr(kPGDMinorError_DialogDisplayFailed);
			break;

		case IDCANCEL:

			derr = mDerr;

			if (derr.IsntError())
				derr = DualErr(kPGDMinorError_UserAbort);

			break;

		default:
			derr = mDerr;
			break;
		}
		
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		derr = DualErr(kPGDMinorError_OutOfMemory);
	}

	return derr;
}


/////////////////////////////////////////////////
// CGlobalCWizardSheet protected default handlers
/////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CGlobalCWizardSheet::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnInitDialog is called during dialog initialization. We take the
// opportunity to add a context help button to the title bar.

BOOL 
CGlobalCWizardSheet::OnInitDialog() 
{
	CMenu		*pSysMenu;
	HDC			hDC;
	PGPBoolean	loadedBitmap;
	PGPUInt32	numBits;

	CPropertySheet::OnInitDialog();

	// Get screen depth.
	hDC = ::GetDC(NULL);		// DC for desktop
	numBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	::ReleaseDC(NULL, hDC);

	// Load the bitmap and create a palette for it.
	if (numBits <= 1)
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SIDEBAR1, 
			&mSidebarBitmap, &mSidebarPalette);
	}
	else if (numBits <= 4)
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SIDEBAR4, 
			&mSidebarBitmap, &mSidebarPalette);
	}
	else
	{
		loadedBitmap = App->GetBitmapAndPalette(IDB_SIDEBAR8, 
			&mSidebarBitmap, &mSidebarPalette);
	}

	// Need to give the dialog a close box if we want a help box as well.
	ModifyStyle(NULL, WS_SYSMENU, NULL);

	// Show the context help button.
	ModifyStyleEx(NULL, WS_EX_CONTEXTHELP, NULL);

	// Alter the system menu.
	pSysMenu = GetSystemMenu(FALSE);
	pgpAssertAddrValid(pSysMenu, CMenu);

	pSysMenu->DeleteMenu(SC_MINIMIZE, MF_BYCOMMAND);
	pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
	pSysMenu->DeleteMenu(SC_RESTORE, MF_BYCOMMAND);
	pSysMenu->DeleteMenu(SC_SIZE, MF_BYCOMMAND);

	DrawMenuBar();

	// Center on screen.
	CenterWindow(GetDesktopWindow());

	// Put us in front.
	SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(this);

	return TRUE;
}

// OnSysCommand is handled to trap close requests.

void 
CGlobalCWizardSheet::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if (nID == SC_CLOSE)
		mDerr = DualErr(kPGDMinorError_UserAbort);
	
	CPropertySheet::OnSysCommand(nID, lParam);
}

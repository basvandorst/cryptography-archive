//////////////////////////////////////////////////////////////////////////////
// CDiskWizIntroPage.cpp
//
// Implementation of class CDiskWizIntroPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizIntroPage.cpp,v 1.2.2.12 1998/08/10 23:31:51 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CDiskWizIntroPage.h"
#include "CDiskWizardSheet.h"
#include "CSavePGPdiskDialog.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_SIDEBAR,			((PGPUInt32) -1), 
	0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CDiskWizIntroPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDiskWizIntroPage)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////
// CDiskWizIntroPage public custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDiskWizIntroPage, CPropertyPage)

// The CDiskWizIntroPage default constructor.

CDiskWizIntroPage::CDiskWizIntroPage()
	 : CPropertyPage(CDiskWizIntroPage::IDD)
{
	//{{AFX_DATA_INIT(CDiskWizIntroPage)
	//}}AFX_DATA_INIT

	mParentDiskWiz = NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CDiskWizIntroPage destructor.

CDiskWizIntroPage::~CDiskWizIntroPage()
{
}


////////////////////////////////////////////////////
// CDiskWizIntroPage public default message handlers
////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CDiskWizIntroPage::OnSetActive() 
{
	pgpAssertAddrValid(mParentDiskWiz, CDiskWizardSheet);
	mParentDiskWiz->SetWizardButtons(PSWIZB_NEXT);

	// Unsteal the handle if stolen.
	if (mParentDiskWiz->mStolenFile.Opened())
	{
		mParentDiskWiz->mStolenFile.Close();

		if (!mParentDiskWiz->mAreWeReplacing)
			DeleteFile(mParentDiskWiz->mPath);
	}

	return CPropertyPage::OnSetActive();
}

// OnWizardNext is called when the user clicks on the 'Next' button.

LRESULT 
CDiskWizIntroPage::OnWizardNext() 
{
	DualErr derr;

	if (CPropertyPage::OnWizardNext() == -1)
		return -1;

	// Ask for a path and filename.
	derr = AskForPath();

	if (derr.IsError())
	{
		mParentDiskWiz->mDerr = derr;
		mParentDiskWiz->PressButton(PSBTN_CANCEL);	// NOT EndDialog (MFC bug)
	}

	return (derr.IsntError() ? 0 : 1);
}


////////////////////////////////////////////////////////////////////////
// CDiskWizIntroPage protected custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

// The CDiskWizIntroPage data exchange function.

void 
CDiskWizIntroPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CDiskWizIntroPage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// AskForPath allows the user to select a directory and filename for the
// destination PGPdisk.

DualErr 
CDiskWizIntroPage::AskForPath() 
{
	CSavePGPdiskDialog	newPGPdiskDlg(kSDT_Create, mParentDiskWiz);
	DualErr				derr;
	File				existingFile;

	// Get a filename and new location from the user.
	if (derr.IsntError())
	{
		derr = newPGPdiskDlg.AskForPath(&mParentDiskWiz->mPath, 
			mParentDiskWiz->mPath);
	}

	if (derr.IsntError())
	{
		// Replacing an existing file?
		mParentDiskWiz->mAreWeReplacing = 
			App->IsFileValid(mParentDiskWiz->mPath);

		// Steal the handle.
		derr = mParentDiskWiz->mStolenFile.Open(mParentDiskWiz->mPath);
	}

	return derr;
}


///////////////////////////////////////////////////////
// CDiskWizIntroPage protected default message handlers
///////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizIntroPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// We override OnPaint to draw the sidebar graphic at the correct depth.

void 
CDiskWizIntroPage::OnPaint() 
{
	CDC			memDC;
	CPaintDC	DC(this);
	HBITMAP		oldBitmap;
	RECT		sidebarRect;

	DC.SelectPalette(&mParentDiskWiz->mSidebarPalette, FALSE);
	DC.RealizePalette();

	memDC.CreateCompatibleDC(&DC);
	memDC.SelectPalette(&mParentDiskWiz->mSidebarPalette, FALSE);
	memDC.RealizePalette();

	mSidebarGraphic.GetWindowRect(&sidebarRect);
	ScreenToClient(&sidebarRect);
	
	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
	// don't fool with it.

	oldBitmap = (HBITMAP) SelectObject(memDC, mParentDiskWiz->mSidebarBitmap);

	DC.BitBlt(sidebarRect.left, sidebarRect.top, sidebarRect.right, 
		sidebarRect.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}

//////////////////////////////////////////////////////////////////////////////
// CGlobalCWizDonePage.cpp
//
// Implementation of class CGlobalCWizDonePage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCWizDonePage.cpp,v 1.1.2.1 1998/10/22 22:27:32 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CGlobalCWizDonePage.h"
#include "CGlobalCWizardSheet.h"
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

BEGIN_MESSAGE_MAP(CGlobalCWizDonePage, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalCWizDonePage)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CGlobalCWizDonePage public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CGlobalCWizDonePage, CPropertyPage)

// The CGlobalCWizDonePage default constructor.

CGlobalCWizDonePage::CGlobalCWizDonePage()
	 : CPropertyPage(CGlobalCWizDonePage::IDD)
{
	//{{AFX_DATA_INIT(CGlobalCWizDonePage)
	//}}AFX_DATA_INIT

	mParentGlobalCWiz = NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CGlobalCWizDonePage destructor.

CGlobalCWizDonePage::~CGlobalCWizDonePage()
{
}


//////////////////////////////////////////////////////
// CGlobalCWizDonePage public default message handlers
//////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CGlobalCWizDonePage::OnSetActive() 
{
	pgpAssertAddrValid(mParentGlobalCWiz, CGlobalCWizardSheet);

	mParentGlobalCWiz->SetWizardButtons(PSWIZB_FINISH);
	mParentGlobalCWiz->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

	return CPropertyPage::OnSetActive();
}


//////////////////////////////////////////////////////////////////////////
// CGlobalCWizDonePage protected custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////

// The CGlobalCWizDonePage data exchange function.

void 
CGlobalCWizDonePage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CGlobalCWizDonePage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


/////////////////////////////////////////////////////////
// CGlobalCWizDonePage protected default message handlers
/////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CGlobalCWizDonePage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CGlobalCWizDonePage::OnPaint() 
{
	CDC			memDC;
	CPaintDC	DC(this);
	HBITMAP		oldBitmap;
	RECT		sidebarRect;

	DC.SelectPalette(&mParentGlobalCWiz->mSidebarPalette, FALSE);
	DC.RealizePalette();

	memDC.CreateCompatibleDC(&DC);
	memDC.SelectPalette(&mParentGlobalCWiz->mSidebarPalette, FALSE);
	memDC.RealizePalette();

	mSidebarGraphic.GetWindowRect(&sidebarRect);
	ScreenToClient(&sidebarRect);
	
	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
	// don't fool with it.

	oldBitmap = (HBITMAP) SelectObject(memDC, 
		mParentGlobalCWiz->mSidebarBitmap);

	DC.BitBlt(sidebarRect.left, sidebarRect.top, sidebarRect.right, 
		sidebarRect.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}

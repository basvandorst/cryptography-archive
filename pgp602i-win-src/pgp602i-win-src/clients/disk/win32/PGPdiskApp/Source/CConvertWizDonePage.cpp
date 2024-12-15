//////////////////////////////////////////////////////////////////////////////
// CConvertWizDonePage.cpp
//
// Implementation of class CConvertWizDonePage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizDonePage.cpp,v 1.1.2.3 1998/10/22 22:27:24 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CConvertWizDonePage.h"
#include "CConvertWizardSheet.h"
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

BEGIN_MESSAGE_MAP(CConvertWizDonePage, CPropertyPage)
	//{{AFX_MSG_MAP(CConvertWizDonePage)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CConvertWizDonePage public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CConvertWizDonePage, CPropertyPage)

// The CConvertWizDonePage default constructor.

CConvertWizDonePage::CConvertWizDonePage()
	 : CPropertyPage(CConvertWizDonePage::IDD)
{
	//{{AFX_DATA_INIT(CConvertWizDonePage)
	//}}AFX_DATA_INIT

	mParentConvertWiz = NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CConvertWizDonePage destructor.

CConvertWizDonePage::~CConvertWizDonePage()
{
}


//////////////////////////////////////////////////////
// CConvertWizDonePage public default message handlers
//////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CConvertWizDonePage::OnSetActive() 
{
	pgpAssertAddrValid(mParentConvertWiz, CConvertWizardSheet);

	mParentConvertWiz->SetWizardButtons(PSWIZB_FINISH);
	mParentConvertWiz->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

	return CPropertyPage::OnSetActive();
}


//////////////////////////////////////////////////////////////////////////
// CConvertWizDonePage protected custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////

// The CConvertWizDonePage data exchange function.

void 
CConvertWizDonePage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CConvertWizDonePage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


/////////////////////////////////////////////////////////
// CConvertWizDonePage protected default message handlers
/////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CConvertWizDonePage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CConvertWizDonePage::OnPaint() 
{
	CDC			memDC;
	CPaintDC	DC(this);
	HBITMAP		oldBitmap;
	RECT		sidebarRect;

	DC.SelectPalette(&mParentConvertWiz->mSidebarPalette, FALSE);
	DC.RealizePalette();

	memDC.CreateCompatibleDC(&DC);
	memDC.SelectPalette(&mParentConvertWiz->mSidebarPalette, FALSE);
	memDC.RealizePalette();

	mSidebarGraphic.GetWindowRect(&sidebarRect);
	ScreenToClient(&sidebarRect);
	
	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
	// don't fool with it.

	oldBitmap = (HBITMAP) SelectObject(memDC, 
		mParentConvertWiz->mSidebarBitmap);

	DC.BitBlt(sidebarRect.left, sidebarRect.top, sidebarRect.right, 
		sidebarRect.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}

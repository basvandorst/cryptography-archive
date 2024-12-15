//////////////////////////////////////////////////////////////////////////////
// CDiskWizDonePage.cpp
//
// Implementation of class CDiskWizDonePage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizDonePage.cpp,v 1.2.2.5.2.2 1998/10/22 22:27:29 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CDiskWizDonePage.h"
#include "CDiskWizardSheet.h"
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

BEGIN_MESSAGE_MAP(CDiskWizDonePage, CPropertyPage)
	//{{AFX_MSG_MAP(CDiskWizDonePage)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////
// CDiskWizDonePage public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDiskWizDonePage, CPropertyPage)

// The CDiskWizDonePage default constructor.

CDiskWizDonePage::CDiskWizDonePage()
	 : CPropertyPage(CDiskWizDonePage::IDD)
{
	//{{AFX_DATA_INIT(CDiskWizDonePage)
	//}}AFX_DATA_INIT

	mParentDiskWiz = NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CDiskWizDonePage destructor.

CDiskWizDonePage::~CDiskWizDonePage()
{
}


///////////////////////////////////////////////////
// CDiskWizDonePage public default message handlers
///////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CDiskWizDonePage::OnSetActive() 
{
	pgpAssertAddrValid(mParentDiskWiz, CDiskWizardSheet);

	mParentDiskWiz->SetWizardButtons(PSWIZB_FINISH);
	mParentDiskWiz->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

	return CPropertyPage::OnSetActive();
}

// OnWizardFinish is called when the user selects the "Finish" button. We
// display the new mounted PGPdisk in an Explorer window.

BOOL 
CDiskWizDonePage::OnWizardFinish() 
{
	if (!CPropertyPage::OnWizardFinish())
		return FALSE;
	
	return TRUE;
}


///////////////////////////////////////////////////////////////////////
// CDiskWizDonePage protected custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

// The CDiskWizDonePage data exchange function.

void 
CDiskWizDonePage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CDiskWizDonePage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


//////////////////////////////////////////////////////
// CDiskWizDonePage protected default message handlers
//////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizDonePage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CDiskWizDonePage::OnPaint() 
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
	
	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so don't
	// fool with it.

	oldBitmap = (HBITMAP) SelectObject(memDC, mParentDiskWiz->mSidebarBitmap);

	DC.BitBlt(sidebarRect.left, sidebarRect.top, sidebarRect.right, 
		sidebarRect.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}

//////////////////////////////////////////////////////////////////////////////
// CGlobalCWizIntroPage.cpp
//
// Implementation of class CGlobalCWizIntroPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCWizIntroPage.cpp,v 1.2 1998/12/14 18:57:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CGlobalCWizIntroPage.h"
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

BEGIN_MESSAGE_MAP(CGlobalCWizIntroPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalCWizIntroPage)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////
// CGlobalCWizIntroPage public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CGlobalCWizIntroPage, CPropertyPage)

// The CGlobalCWizIntroPage default constructor.

CGlobalCWizIntroPage::CGlobalCWizIntroPage()
	 : CPropertyPage(CGlobalCWizIntroPage::IDD)
{
	//{{AFX_DATA_INIT(CGlobalCWizIntroPage)
	//}}AFX_DATA_INIT

	mParentGlobalCWiz = NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CGlobalCWizIntroPage destructor.

CGlobalCWizIntroPage::~CGlobalCWizIntroPage()
{
}


///////////////////////////////////////////////////////
// CGlobalCWizIntroPage public default message handlers
///////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CGlobalCWizIntroPage::OnSetActive() 
{
	pgpAssertAddrValid(mParentGlobalCWiz, CGlobalCWizardSheet);
	mParentGlobalCWiz->SetWizardButtons(PSWIZB_NEXT);

	return CPropertyPage::OnSetActive();
}


///////////////////////////////////////////////////////////////////////////
// CGlobalCWizIntroPage protected custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////////

// The CGlobalCWizIntroPage data exchange function.

void 
CGlobalCWizIntroPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CGlobalCWizIntroPage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


//////////////////////////////////////////////////////////
// CGlobalCWizIntroPage protected default message handlers
//////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CGlobalCWizIntroPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CGlobalCWizIntroPage::OnPaint() 
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

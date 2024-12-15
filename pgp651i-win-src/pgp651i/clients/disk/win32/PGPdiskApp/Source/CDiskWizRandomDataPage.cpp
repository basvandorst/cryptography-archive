//////////////////////////////////////////////////////////////////////////////
// CDiskWizRandomDataPage.cpp
//
// Implementation of class CDiskWizRandomDataPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizRandomDataPage.cpp,v 1.8 1999/03/17 00:34:07 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CDiskWizRandomDataPage.h"
#include "CDiskWizardSheet.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"

#include "pgpRandomPool.h"


////////////
// Constants
////////////

const PGPUInt32 kPGPdiskEntropyNeeded = 300;		// 300 bits


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_RANDOM_BAR,			IDH_PGPDISKAPP_DISKWIZRANDOMBAR, 
	IDC_SIDEBAR,			((PGPUInt32) -1), 
	0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CDiskWizRandomDataPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDiskWizRandomDataPage)
	ON_WM_HELPINFO()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CDiskWizRandomDataPage public custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDiskWizRandomDataPage, CPropertyPage)

// The CDiskWizRandomDataPage default constructor.

CDiskWizRandomDataPage::CDiskWizRandomDataPage()
	 : CPropertyPage(CDiskWizRandomDataPage::IDD)
{
	//{{AFX_DATA_INIT(CDiskWizRandomDataPage)
	mRandomMessage = _T("");
	//}}AFX_DATA_INIT

	mParentDiskWiz		= NULL;
	mIsDoneCollecting	= FALSE;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CDiskWizRandomDataPage destructor.

CDiskWizRandomDataPage::~CDiskWizRandomDataPage()
{
}

// PreTranslateMessage is overridden so we can process all keystrokes as data.

BOOL 
CDiskWizRandomDataPage::PreTranslateMessage(MSG *pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		ProcessChar(MapVirtualKey(pMsg->wParam, 2));
		return TRUE;
	}

	return CPropertyPage::PreTranslateMessage(pMsg);
}


/////////////////////////////////////////////////////////
// CDiskWizRandomDataPage public default message handlers
/////////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CDiskWizRandomDataPage::OnSetActive() 
{
	pgpAssertAddrValid(mParentDiskWiz, CDiskWizardSheet);

	if (PGPGlobalRandomPoolGetEntropy() >= kPGPdiskEntropyNeeded)
	{
		mRandomDataBar.SetPos(100);
		mIsDoneCollecting = TRUE;
	}

	if (mIsDoneCollecting)
		mParentDiskWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	else
		mParentDiskWiz->SetWizardButtons(PSWIZB_BACK);

	return CPropertyPage::OnSetActive();
}


/////////////////////////////////////////////////////////////////////////////
// CDiskWizRandomDataPage protected custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////////

// ProcessChar is called when the user hits a key. Add it to the pool.

void 
CDiskWizRandomDataPage::ProcessChar(PGPUInt8 nChar) 
{
	PGPGlobalRandomPoolAddKeystroke(nChar);

	if (PGPGlobalRandomPoolGetEntropy() >= kPGPdiskEntropyNeeded)
	{
		mRandomDataBar.SetPos(100);
		mIsDoneCollecting = TRUE;
		mParentDiskWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	}
	else
	{
		mRandomDataBar.SetPos(PGPGlobalRandomPoolGetEntropy()*100 / 
			kPGPdiskEntropyNeeded);
	}
}

// The CDiskWizRandomDataPage data exchange function.

void 
CDiskWizRandomDataPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CDiskWizRandomDataPage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		DDX_Control(pDX, IDC_RANDOM_BAR, mRandomDataBar);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


////////////////////////////////////////////////////////////
// CDiskWizRandomDataPage protected default message handlers
////////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizRandomDataPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnInitDialog is overwritten to perform property page initialization.

BOOL 
CDiskWizRandomDataPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// This must be called first to make MFC happy.
	UpdateData(FALSE);

	// Initialize the entropy bar range and position.
	mRandomDataBar.SetRange(0, 100);
	mRandomDataBar.SetPos(0);

	UpdateData(FALSE);

	return TRUE;
}

// OnLButtonDown is overridden to provide a quick exit in debug mode.

void 
CDiskWizRandomDataPage::OnLButtonDown(UINT nFlags, CPoint point) 
{
#if PGP_DEBUG
	mIsDoneCollecting = TRUE;
	mRandomDataBar.SetPos(100);
	mParentDiskWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
#endif	// PGP_DEBUG

	CPropertyPage::OnLButtonDown(nFlags, point);
}

void 
CDiskWizRandomDataPage::OnMouseMove(UINT nFlags, CPoint point) 
{
	PGPGlobalRandomPoolMouseMoved();

	if (PGPGlobalRandomPoolGetEntropy() >= kPGPdiskEntropyNeeded)
	{
		mRandomDataBar.SetPos(100);
		mIsDoneCollecting = TRUE;
		mParentDiskWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	}
	else
	{
		mRandomDataBar.SetPos(PGPGlobalRandomPoolGetEntropy()*100 / 
			kPGPdiskEntropyNeeded);
	}
}

// We override OnPaint to draw the sidebar graphic at the correct depth.

void 
CDiskWizRandomDataPage::OnPaint() 
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

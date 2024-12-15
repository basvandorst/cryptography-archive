//////////////////////////////////////////////////////////////////////////////
// CConvertWizIntroPage.cpp
//
// Implementation of class CConvertWizIntroPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizIntroPage.cpp,v 1.2 1998/12/14 18:56:06 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskPublicKeyUtils.h"
#include "StringAssociation.h"

#include "CConvertWizIntroPage.h"
#include "CConvertWizardSheet.h"
#include "CSinglePassphraseDialog.h"
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

BEGIN_MESSAGE_MAP(CConvertWizIntroPage, CPropertyPage)
	//{{AFX_MSG_MAP(CConvertWizIntroPage)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////
// CConvertWizIntroPage public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CConvertWizIntroPage, CPropertyPage)

// The CConvertWizIntroPage default constructor.

CConvertWizIntroPage::CConvertWizIntroPage()
	 : CPropertyPage(CConvertWizIntroPage::IDD)
{
	//{{AFX_DATA_INIT(CConvertWizIntroPage)
	//}}AFX_DATA_INIT

	mParentConvertWiz = NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CConvertWizIntroPage destructor.

CConvertWizIntroPage::~CConvertWizIntroPage()
{
}


///////////////////////////////////////////////////////
// CConvertWizIntroPage public default message handlers
///////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CConvertWizIntroPage::OnSetActive() 
{
	pgpAssertAddrValid(mParentConvertWiz, CConvertWizardSheet);
	mParentConvertWiz->SetWizardButtons(PSWIZB_NEXT);

	return CPropertyPage::OnSetActive();
}

// OnWizardNext is called when the user clicks on the 'Next' button.

LRESULT 
CConvertWizIntroPage::OnWizardNext() 
{
	DualErr derr;

	if (CPropertyPage::OnWizardNext() == -1)
		return -1;

	// Ask for the passphrase for the PGPdisk.
	if (ArePGPdiskPassesWiped(mParentConvertWiz->mPath))
	{
		DisplayMessage(kPGPdiskConversionWipedMsgString);
		derr = AskForPublicKeyPassphrase();
	}
	else
	{
		derr = AskForMasterPassphrase();
	}

	if (derr.IsError())
	{
		mParentConvertWiz->mDerr = derr;
		mParentConvertWiz->PressButton(PSBTN_CANCEL);	// NOT EndDialog
	}

	return (derr.IsntError() ? 0 : 1);
}


///////////////////////////////////////////////////////////////////////////
// CConvertWizIntroPage protected custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////////

// The CConvertWizIntroPage data exchange function.

void 
CConvertWizIntroPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CConvertWizIntroPage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// AskForMasterPassphrase asks the user for the master passphrase for the
// specified PGPdisk.

DualErr 
CConvertWizIntroPage::AskForMasterPassphrase()
{
	CSinglePassphraseDialog	masterPassDlg(kSPDT_Master);
	DualErr					derr;

	// Did our dialog initialize correctly?
	derr = masterPassDlg.mInitErr;

	// Display the master passphrase dialog.
	if (derr.IsntError())
	{
		derr = masterPassDlg.AskForPassphrase(mParentConvertWiz->mPath);
	}

	// Save the passphrase
	if (derr.IsntError())
	{
		(* mParentConvertWiz->mPassphrase) = 
			(* masterPassDlg.mPassphraseEdit.mContents);
	}
	
	return derr;
}

// AskForPublicKeyPassphrase asks the user for the master passphrase for the
// specified PGPdisk.

DualErr 
CConvertWizIntroPage::AskForPublicKeyPassphrase()
{
	CSinglePassphraseDialog	publicKeyPassDlg(kSPDT_PublicKey);
	DualErr					derr;

	// Did our dialog initialize correctly?
	derr = publicKeyPassDlg.mInitErr;

	// Display the public key passphrase dialog.
	if (derr.IsntError())
	{
		derr = publicKeyPassDlg.AskForPassphrase(mParentConvertWiz->mPath);
	}

	// Save the passphrase
	if (derr.IsntError())
	{
		(* mParentConvertWiz->mPassphrase) = 
			(* publicKeyPassDlg.mPassphraseEdit.mContents);
	}
	
	return derr;
}


//////////////////////////////////////////////////////////
// CConvertWizIntroPage protected default message handlers
//////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CConvertWizIntroPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CConvertWizIntroPage::OnPaint() 
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

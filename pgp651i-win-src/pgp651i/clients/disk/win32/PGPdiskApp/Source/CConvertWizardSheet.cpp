//////////////////////////////////////////////////////////////////////////////
// CConvertWizardSheet.cpp
//
// Implementation of class CConvertWizardSheet.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizardSheet.cpp,v 1.3 1999/02/26 04:09:56 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <new.h>

#include "Required.h"
#include "PGPdiskPublicKeyUtils.h"
#include "StringAssociation.h"

#include "CConvertWizardSheet.h"
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

BEGIN_MESSAGE_MAP(CConvertWizardSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CConvertWizardSheet)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CConvertWizardSheet public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CConvertWizardSheet, CPropertySheet)

// The CConvertWizardSheet constructor adds the wizard pages we implement
// to the wizard sheet.

CConvertWizardSheet::CConvertWizardSheet() :
		mLockedMem(sizeof(SecureString)), 
		CPropertySheet(GetCommonString(kPGPdiskConvertWizSheetTitleString), 
			NULL, 0)
{
	try
	{
		// Make sure our locked memory initialized correctly.
		mInitErr = mLockedMem.mInitErr;

		// Construct the secure string object.
		if (mInitErr.IsntError())
		{
			mPassphrase = new (mLockedMem.GetPtr()) SecureString;	// <new.h>
			mPassphrase->ClearString();
		}
		
		// Did our pages initialize correctly?
		if (mInitErr.IsntError())
		{
			mInitErr = mConvertPage.mInitErr;
		}

		if (mInitErr.IsntError())
		{
			mInitErr = mRandomDataPage.mInitErr;
		}

		if (mInitErr.IsntError())
		{
			mConvertPage.mParentConvertWiz		= this;
			mDonePage.mParentConvertWiz			= this;
			mIntroPage.mParentConvertWiz		= this;
			mRandomDataPage.mParentConvertWiz	= this;

			AddPage(&mIntroPage);
			AddPage(&mRandomDataPage);
			AddPage(&mConvertPage);
			AddPage(&mDonePage);

			// Exorcise the big help button.
			m_psh.dwFlags &= ~PSH_HASHELP;
		}
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}
}

// The CConvertWizardSheet destructor.

CConvertWizardSheet::~CConvertWizardSheet()
{
	if (mPassphrase)
		mPassphrase->~SecureString();		// we used placement new
}

// ExecuteWizard displays the wizard and converts the PGPdisk after asking
// the user for input.

DualErr 
CConvertWizardSheet::ExecuteWizard(
	LPCSTR		path, 
	PGPBoolean	restartingConversion)
{
	DualErr derr;

	try
	{
		PGPBoolean	adkKeyOnKeyring;
		PGPInt32	result;

		pgpAssertStrValid(path);

		derr = CheckIfLockedKeysOnKeyring(path, &adkKeyOnKeyring);

		if (derr.IsntError())
		{
			if (!adkKeyOnKeyring)
			{
				ReportError(kPGDMajorError_PGPdiskConversionFailed, 
					kPGDMinorError_NoADKWhenConverting);

				ShowReadOnlyListOfPublicKeys(path);
				derr = kPGDMinorError_FailSilently;
			}

		}

		if (derr.IsntError())
		{
			mPath = path;
			mRestartingConversion = restartingConversion;

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
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		derr = DualErr(kPGDMinorError_OutOfMemory);
	}

	return derr;
}

// PreTranslateMessage gets to see all messages before MFC does. We check
// for keydown messages and if the random data page is open and active, pass
// them to that page.

BOOL 
CConvertWizardSheet::PreTranslateMessage(MSG* pMsg) 
{
	PGPBoolean	weProcessed	= FALSE;
	PGPUInt8	vCode;
	PGPUInt32	activeIndex, randomIndex;

	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		vCode = pMsg->wParam & 0x7F;

		activeIndex	= GetActiveIndex();
		randomIndex	= GetPageIndex(&mRandomDataPage);

		if ((activeIndex == randomIndex) && 
			!mRandomDataPage.mIsDoneCollecting)
		{
			// Punt key messages down for random data collection.
			mRandomDataPage.PostMessage(pMsg->message, pMsg->wParam, 
				pMsg->lParam);

			weProcessed = TRUE;
		}
		break;
	}

	return (weProcessed ? TRUE : CPropertySheet::PreTranslateMessage(pMsg));
}


/////////////////////////////////////////////////
// CConvertWizardSheet protected default handlers
/////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CConvertWizardSheet::OnHelpInfo(HELPINFO *pHelpInfo)
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
CConvertWizardSheet::OnInitDialog() 
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
CConvertWizardSheet::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if (nID == SC_CLOSE)
		mDerr = DualErr(kPGDMinorError_UserAbort);
	
	CPropertySheet::OnSysCommand(nID, lParam);
}

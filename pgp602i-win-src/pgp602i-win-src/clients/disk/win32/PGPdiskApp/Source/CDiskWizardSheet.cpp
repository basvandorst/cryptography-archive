//////////////////////////////////////////////////////////////////////////////
// CDiskWizardSheet.cpp
//
// Implementation of class CDiskWizardSheet.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizardSheet.cpp,v 1.2.2.22.2.3 1998/10/21 01:35:16 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "StringAssociation.h"

#include "CMainDialog.h"
#include "CDiskWizardSheet.h"
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

BEGIN_MESSAGE_MAP(CDiskWizardSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CDiskWizardSheet)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////
// CDiskWizardSheet public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDiskWizardSheet, CPropertySheet)

// The CDiskWizardSheet constructor adds the wizard pages we implement
// to the wizard sheet.

CDiskWizardSheet::CDiskWizardSheet()

	: CPropertySheet(GetCommonString(kPGPdiskDiskWizSheetTitleString), NULL, 
		0)
{
	try
	{
		// Did our pages initialize correctly?
		mInitErr = mPassphrasePage.mInitErr;

		if (mInitErr.IsntError())
		{
			mInitErr = mRandomDataPage.mInitErr;
		}

		if (mInitErr.IsntError())
		{
			mInitErr = mCreationPage.mInitErr;
		}

		if (mInitErr.IsntError())
		{
			mCreationPage.mParentDiskWiz	= this;
			mDonePage.mParentDiskWiz		= this;
			mIntroPage.mParentDiskWiz		= this;
			mPassphrasePage.mParentDiskWiz	= this;
			mADKPage.mParentDiskWiz			= this;
			mRandomDataPage.mParentDiskWiz	= this;
			mVolumeInfoPage.mParentDiskWiz	= this;

			AddPage(&mIntroPage);
			AddPage(&mVolumeInfoPage);
			AddPage(&mPassphrasePage);
			AddPage(&mADKPage);
			AddPage(&mRandomDataPage);
			AddPage(&mCreationPage);
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

// The CDiskWizardSheet destructor.

CDiskWizardSheet::~CDiskWizardSheet()
{
}

// ExecuteWizard displays the wizard and creates the PGPdisk after asking the
// user for input.

DualErr 
CDiskWizardSheet::ExecuteWizard(LPCSTR path)
{
	DualErr derr;

	try
	{
		PGPdisk		*pPGD;
		PGPInt32	result;

		pgpAssertStrValid(path);
		mPath = path;

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

		// If the PGPdisk was created and mounted successfully, format it and
		// show it in an Explorer window.

		if (derr.IsntError())
		{
			if (IsntNull(pPGD = App->mPGPdisks.FindPGPdisk(mPath)))
			{
				derr = pPGD->HighLevelFormatPGPdisk();

				if (derr.IsntError())
				{
					// Must unmount and remount on Win95 (Q134560).
					if (IsWin95CompatibleMachine() && 
						!IsWin98CompatibleMachine())
					{
						UnmountAndRemountPGPdisk(pPGD->GetDrive());

						if (IsntNull(pPGD = 
							App->mPGPdisks.FindPGPdisk(mPath)))
						{
							pPGD->BrowseToVolume();
						}
					}
					else
					{
						pPGD->BrowseToVolume();
					}
				}
			}
		}

		// If error, unsteal the handle if stolen.
		if (derr.IsError() && mStolenFile.Opened())
		{
			mStolenFile.Close();

			if (!mAreWeReplacing)
				DeleteFile(mPath);
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
CDiskWizardSheet::PreTranslateMessage(MSG* pMsg) 
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


///////////////////////////////////////////////////////////////////////
// CDiskWizardSheet protected custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

// UnmountAndRemountPGPdisk quickly unmounts and remounts the PGPdisk after
// formatting.

void 
CDiskWizardSheet::UnmountAndRemountPGPdisk(PGPUInt8 drive)
{
	AppCommandInfo ACI;

	pgpAssert(IsLegalDriveNumber(drive));

	// Unmount the sucker.
	ACI.op		= kAppOp_Unmount;
	ACI.flags	= NULL;
	ACI.drive	= drive;
	ACI.path[0]	= '\0';
		
	App->DispatchAppCommandInfo(&ACI);

	// Re-mount that bad boy.
	ACI.op		= kAppOp_Mount;
	ACI.flags	= kACF_MountWithNoDialog;
	ACI.drive	= mVolumeInfoPage.mDriveCombo.mDriveNumber;
	strcpy(ACI.path, mPath);

	ACI.data[0]	= (PGPUInt32) mPassphrasePage.mPassphraseEdit.mContents;

	App->DispatchAppCommandInfo(&ACI);
}


//////////////////////////////////////////////
// CDiskWizardSheet protected default handlers
//////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizardSheet::OnHelpInfo(HELPINFO *pHelpInfo)
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
CDiskWizardSheet::OnInitDialog() 
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
CDiskWizardSheet::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if (nID == SC_CLOSE)
		mDerr = DualErr(kPGDMinorError_UserAbort);
	
	CPropertySheet::OnSysCommand(nID, lParam);
}

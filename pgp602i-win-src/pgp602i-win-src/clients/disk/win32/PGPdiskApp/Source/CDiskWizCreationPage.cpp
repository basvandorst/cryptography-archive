//////////////////////////////////////////////////////////////////////////////
// CDiskWizCreationPage.cpp
//
// Implementation of class CDiskWizCreationPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizCreationPage.cpp,v 1.2.2.22.2.1 1998/10/22 22:27:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "StringAssociation.h"

#if PGPDISK_PUBLIC_KEY

#include "PGPclx.h"
#include "PGPdiskPublicKeyUtils.h"

#undef IDB_CREDITS1
#undef IDB_CREDITS4
#undef IDB_CREDITS8
#undef IDC_CREDITS

#endif	// PGPDISK_PUBLIC_KEY

#include "CDiskWizCreationPage.h"
#include "CDiskWizardSheet.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_PROGRESS_BAR,		IDH_PGPDISKAPP_DISKWIZCREATIONPROGRESS, 
	IDC_DISKNAME_TEXT,		((PGPUInt32) -1), 
    IDC_SIDEBAR,			((PGPUInt32) -1), 
	0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CDiskWizCreationPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDiskWizCreationPage)
	ON_MESSAGE(WM_FINISHED_CREATE, OnFinishedCreate)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////
// CDiskWizCreationPage public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDiskWizCreationPage, CPropertyPage)

// The CDiskWizCreationPage default constructor.

CDiskWizCreationPage::CDiskWizCreationPage()
	 : CPropertyPage(CDiskWizCreationPage::IDD)
{
	//{{AFX_DATA_INIT(CDiskWizCreationPage)
	mProgressText = _T("");
	//}}AFX_DATA_INIT

	mParentDiskWiz = NULL;

	mIsThreadActive		= FALSE;
	mSkipConfirmCancel	= FALSE;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;

	// Create the thread object we will use.
	try
	{
		mCreationThread = new CCreatePGPdiskThread;

		// Don't auto-delete.
		mCreationThread->m_bAutoDelete = FALSE;
		mIsCTAutoDelete = FALSE;
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}
}

// The CDiskWizCreationPage destructor.

CDiskWizCreationPage::~CDiskWizCreationPage()
{
	if (!mIsCTAutoDelete)
	{
		delete mCreationThread;
	}
}


///////////////////////////////////////////////////////
// CDiskWizCreationPage public default message handlers
///////////////////////////////////////////////////////

// OnFinishedCreate is called when the thread has finished creating the
// PGPdisk.

void 
CDiskWizCreationPage::OnFinishedCreate(WPARAM wParam, LPARAM lParam)
{
	DualErr derr;

	pgpAssertAddrValid(mCreationThread, CCreatePGPdiskThread);

	mIsThreadActive = FALSE;
	derr = mCreationThread->mDerr;

#if PGPDISK_PUBLIC_KEY && PGP_BUSINESS_SECURITY

	if (derr.IsntError())
	{
		if (mParentDiskWiz->mADKPage.mUseADK)
		{
			derr = AddADKToPGPdisk(mParentDiskWiz->mPath, 
				mParentDiskWiz->mPassphrasePage.mPassphraseEdit.mContents);
		}
	}

#endif	// PGPDISK_PUBLIC_KEY && PGP_BUSINESS_SECURITY

	if (derr.IsntError())
	{
		// Enable the next button.
		mParentDiskWiz->SetWizardButtons(PSWIZB_NEXT);

		// Set the progress string.
		SetProgressString(GetCommonString(kPGPdiskCreationDoneMsgString));
	}

	if (derr.IsError())
	{
		mParentDiskWiz->mDerr = derr;

		mSkipConfirmCancel = TRUE;
		mParentDiskWiz->PressButton(PSBTN_CANCEL);	// NOT EndDialog (MFC bug)
	}
}

// OnQueryCancel confirms that the user really does want to cancel.

BOOL 
CDiskWizCreationPage::OnQueryCancel() 
{
	// Confirm cancellation of creation.
	if (!mSkipConfirmCancel)
	{
		if (DisplayMessage(kPGPdiskConfirmCancelCreation, kPMBS_YesNo, 
			kPMBF_NoButton) == kUR_No)
		{
			return FALSE;
		}
	}

	return CPropertyPage::OnQueryCancel();
}

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled, and begin PGPdisk creation.

BOOL 
CDiskWizCreationPage::OnSetActive() 
{
	pgpAssertAddrValid(mParentDiskWiz, CDiskWizardSheet);
	mParentDiskWiz->SetWizardButtons(NULL);

	// Ask a separate thread to convert the PGPdisk.
	if (!mIsThreadActive)
	{
		CreatePGPdiskInfo	CPI;
		DualErr				derr;

		pgpAssertAddrValid(mCreationThread, CCreatePGPdiskThread);

		CPI.path		= mParentDiskWiz->mPath;
		CPI.blocksDisk	= mParentDiskWiz->mVolumeInfoPage.mBlocksDisk;
		CPI.randomPool	= &mParentDiskWiz->mRandomDataPage.mRandomPool;

		CPI.passphrase = 
			mParentDiskWiz->mPassphrasePage.mPassphraseEdit.mContents;

		CPI.drive = 
			mParentDiskWiz->mVolumeInfoPage.mDriveCombo.mDriveNumber;

		CPI.callerHwnd		= GetSafeHwnd();
		CPI.progressHwnd	= mProgressBar.GetSafeHwnd();

		// Start the creation thread.
		mIsThreadActive = TRUE;
		derr = mCreationThread->CallPGPdiskCreate(&CPI);

		// Close our 'stolen' handle.
		if (mParentDiskWiz->mStolenFile.Opened())
			mParentDiskWiz->mStolenFile.Close();

		// Set the progress message.
		if (derr.IsntError())
		{
			SetProgressString(
				GetCommonString(kPGPdiskCreationCreateMsgString));

			mParentDiskWiz->SendDlgItemMessage(IDCANCEL, WM_KILLFOCUS, NULL, 
				NULL);
		}

		if (derr.IsError())
		{
			mIsThreadActive = FALSE;

			mParentDiskWiz->mDerr = derr;
			mParentDiskWiz->EndDialog(IDCANCEL);
		}
	}

	return CPropertyPage::OnSetActive();
}

// OnWizardNext is called when the user clicks on the 'Next' button. We mount
// the newly created PGPdisk.

LRESULT 
CDiskWizCreationPage::OnWizardNext() 
{
	AppCommandInfo ACI;

	if (CPropertyPage::OnWizardNext() == -1)
		return -1;

	// Mount the unformatted PGPdisk.
	ACI.op		= kAppOp_Mount;
	ACI.flags	= kACF_MountWithNoDialog;
	ACI.drive	= mParentDiskWiz->mVolumeInfoPage.mDriveCombo.mDriveNumber;
	strcpy(ACI.path, mParentDiskWiz->mPath);

	ACI.data[0]	= (PGPUInt32) 
		mParentDiskWiz->mPassphrasePage.mPassphraseEdit.mContents;

	App->DispatchAppCommandInfo(&ACI);

	return 0;
}


///////////////////////////////////////////////////////////////////////////
// CDiskWizCreationPage protected custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////////

// SetProgressString updates the progress string with the specified message,
// inserting the PGPdisk name where appropriate.

void 
CDiskWizCreationPage::SetProgressString(LPCSTR message)
{
	CString bareName;

	pgpAssertStrValid(message);

	// Prepare the dialog message text.
	GetBareName(mParentDiskWiz->mPath, &bareName);

	// Format and truncate the message if necessary.
	App->FormatFitStringToWindow(message, bareName, &mProgressText, 
		&mProgressStatic);

	UpdateData(FALSE);
}

// The CDiskWizCreationPage data exchange function.

void 
CDiskWizCreationPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CDiskWizCreationPage)
		DDX_Control(pDX, IDC_DISKNAME_TEXT, mProgressStatic);
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		DDX_Control(pDX, IDC_PROGRESS_BAR, mProgressBar);
		DDX_Text(pDX, IDC_DISKNAME_TEXT, mProgressText);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


//////////////////////////////////////////////////////////
// CDiskWizCreationPage protected default message handlers
//////////////////////////////////////////////////////////

// OnCancel is called when the cancel button is pressed.

void 
CDiskWizCreationPage::OnCancel() 
{	
	if (mIsThreadActive)
	{
		// Tell it to delete itself since we may be gone by the time it
		// finishes the cancel.

		pgpAssertAddrValid(mCreationThread, CCreatePGPdiskThread);

		mIsCTAutoDelete = TRUE;
		mCreationThread->m_bAutoDelete = TRUE;

		mCreationThread->CancelPGPdiskCreate();
	}

	DeleteFile(mParentDiskWiz->mPath);
	CPropertyPage::OnCancel();
}

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizCreationPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CDiskWizCreationPage::OnPaint() 
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

//////////////////////////////////////////////////////////////////////////////
// CConvertWizConvertPage.cpp
//
// Implementation of the CConvertWizConvertPage class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConvertWizConvertPage.cpp,v 1.3 1999/02/26 04:09:56 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "StringAssociation.h"

#include "CConvertWizardSheet.h"
#include "CConvertWizConvertPage.h"
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

BEGIN_MESSAGE_MAP(CConvertWizConvertPage, CPropertyPage)
	//{{AFX_MSG_MAP(CConvertWizConvertPage)
	ON_MESSAGE(WM_FINISHED_CONVERT, OnFinishedConvert)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CConvertWizConvertPage public custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CConvertWizConvertPage, CPropertyPage)

// The CConvertWizConvertPage default constructor.

CConvertWizConvertPage::CConvertWizConvertPage()
	 : CPropertyPage(CConvertWizConvertPage::IDD)
{
	//{{AFX_DATA_INIT(CConvertWizConvertPage)
	mProgressText = _T("");
	//}}AFX_DATA_INIT

	mParentConvertWiz = NULL;

	mIsThreadActive		= FALSE;
	mSkipConfirmCancel	= FALSE;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;

	// Create the thread object we will use.
	try
	{
		mConversionThread = new CConvertPGPdiskThread;

		// Don't auto-delete.
		mConversionThread->m_bAutoDelete = FALSE;
		mIsCTAutoDelete = FALSE;
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}
}

// The CConvertWizConvertPage destructor.

CConvertWizConvertPage::~CConvertWizConvertPage()
{
	if (!mIsCTAutoDelete)
	{
		delete mConversionThread;
	}
}


/////////////////////////////////////////////////////////
// CConvertWizConvertPage public default message handlers
/////////////////////////////////////////////////////////

// OnFinishedConvert is called when the thread has finished converting the
// PGPdisk.

void 
CConvertWizConvertPage::OnFinishedConvert(WPARAM wParam, LPARAM lParam)
{
	DualErr derr;

	pgpAssertAddrValid(mConversionThread, CConvertPGPdiskThread);

	mIsThreadActive = FALSE;
	derr = mConversionThread->mDerr;

	if (derr.IsntError())
	{
		// Enable the next button and disable the cancel button.
		mParentConvertWiz->SetWizardButtons(PSWIZB_NEXT);
		mParentConvertWiz->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

		// Set the progress string.
		SetProgressString(GetCommonString(kPGPdiskConversionDoneMsgString));
	}

	if (derr.IsError())
	{
		mParentConvertWiz->mDerr = derr;

		mSkipConfirmCancel = TRUE;
		mParentConvertWiz->PressButton(PSBTN_CANCEL);	// NOT EndDialog
	}
}

// OnQueryCancel confirms that the user really does want to cancel.

BOOL 
CConvertWizConvertPage::OnQueryCancel() 
{
	// Confirm cancellation of creation.
	if (!mSkipConfirmCancel)
	{
		if (DisplayMessage(kPGPdiskConfirmCancelConversion, kPMBS_YesNo, 
			kPMBF_NoButton) == kUR_No)
		{
			return FALSE;
		}
	}

	return CPropertyPage::OnQueryCancel();
}

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled, and begin PGPdisk conversion.

BOOL 
CConvertWizConvertPage::OnSetActive() 
{
	pgpAssertAddrValid(mParentConvertWiz, CConvertWizardSheet);
	mParentConvertWiz->SetWizardButtons(NULL);

	// Ask a separate thread to create the PGPdisk.
	if (!mIsThreadActive)
	{
		ConvertPGPdiskInfo	CPI;
		DualErr				derr;

		pgpAssertAddrValid(mConversionThread, CConvertPGPdiskThread);

		CPI.path		= mParentConvertWiz->mPath;
		CPI.passphrase	= mParentConvertWiz->mPassphrase;

		CPI.callerHwnd		= GetSafeHwnd();
		CPI.progressHwnd	= mProgressBar.GetSafeHwnd();

		// Start the conversion thread.
		mIsThreadActive = TRUE;
		derr = mConversionThread->CallPGPdiskConvert(&CPI);

		// Set the progress message.
		if (derr.IsntError())
		{
			SetProgressString(
				GetCommonString(kPGPdiskConversionConvertMsgString));

			mParentConvertWiz->SendDlgItemMessage(IDCANCEL, WM_KILLFOCUS, 
				NULL, NULL);
		}

		if (derr.IsError())
		{
			mIsThreadActive = FALSE;

			mParentConvertWiz->mDerr = derr;
			mParentConvertWiz->EndDialog(IDCANCEL);
		}
	}

	return CPropertyPage::OnSetActive();
}


/////////////////////////////////////////////////////////////////////////////
// CConvertWizConvertPage protected custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////////

// SetProgressString updates the progress string with the specified message,
// inserting the PGPdisk name where appropriate.

void 
CConvertWizConvertPage::SetProgressString(LPCSTR message)
{
	CString bareName;

	pgpAssertStrValid(message);

	// Prepare the dialog message text.
	GetBareName(mParentConvertWiz->mPath, &bareName);

	// Format and truncate the message if necessary.
	App->FormatFitStringToWindow(message, bareName, &mProgressText, 
		&mProgressStatic);

	UpdateData(FALSE);
}

// The CConvertWizConvertPage data exchange function.

void 
CConvertWizConvertPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CConvertWizConvertPage)
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


////////////////////////////////////////////////////////////
// CConvertWizConvertPage protected default message handlers
////////////////////////////////////////////////////////////

// OnCancel is called when the cancel button is pressed.

void 
CConvertWizConvertPage::OnCancel() 
{	
	if (mIsThreadActive)
	{
		// Tell it to delete itself since we may be gone by the time it
		// finishes the cancel.

		pgpAssertAddrValid(mConversionThread, CConvertPGPdiskThread);

		mIsCTAutoDelete = TRUE;
		mConversionThread->m_bAutoDelete = TRUE;

		mConversionThread->CancelPGPdiskConvert();
	}

	CPropertyPage::OnCancel();
}

// OnHelpInfo handles context-sensitive help.

BOOL 
CConvertWizConvertPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CConvertWizConvertPage::OnPaint() 
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

//////////////////////////////////////////////////////////////////////////////
// CGlobalCWizSearchPage.cpp
//
// Implementation of the CGlobalCWizSearchPage class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGlobalCWizSearchPage.cpp,v 1.2 1998/12/14 18:57:12 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "StringAssociation.h"

#include "CGlobalCWizSearchPage.h"
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

BEGIN_MESSAGE_MAP(CGlobalCWizSearchPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalCWizSearchPage)
	ON_MESSAGE(WM_FINISHED_SEARCH, OnFinishedSearch)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////
// CGlobalCWizSearchPage public custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CGlobalCWizSearchPage, CPropertyPage)

// The CGlobalCWizSearchPage default constructor.

CGlobalCWizSearchPage::CGlobalCWizSearchPage()
	 : CPropertyPage(CGlobalCWizSearchPage::IDD)
{
	//{{AFX_DATA_INIT(CGlobalCWizSearchPage)
	mDirNameText = _T("");
	mSearchStatusText = _T("");
	//}}AFX_DATA_INIT

	mParentGlobalCWiz = NULL;

	mSkipConfirmCancel	= FALSE;
	mFinishedSearch		= FALSE;
	mIsThreadActive		= FALSE;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;

	// Create the thread object we will use.
	try
	{
		mSearchThread = new CGlobalCSearchPGPdiskThread;

		// Don't auto-delete.
		mSearchThread->m_bAutoDelete = FALSE;
		mIsCTAutoDelete = FALSE;
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}
}

// The CGlobalCWizSearchPage destructor.

CGlobalCWizSearchPage::~CGlobalCWizSearchPage()
{
	if (!mIsCTAutoDelete)
	{
		delete mSearchThread;
	}
}


////////////////////////////////////////////////////////
// CGlobalCWizSearchPage public default message handlers
////////////////////////////////////////////////////////

// OnFinishedSearch is called when the thread has finished searching the
// PGPdisk.

void 
CGlobalCWizSearchPage::OnFinishedSearch(WPARAM wParam, LPARAM lParam)
{
	DualErr derr;

	pgpAssertAddrValid(mSearchThread, CGlobalCSearchPGPdiskThread);

	mSearchStatusText = GetCommonString(kPGPdiskSearchingDoneMsgString);
	mDirNameText.Empty();
	UpdateData(FALSE);

	mFinishedSearch = TRUE;
	mIsThreadActive = FALSE;

	derr = mSearchThread->mDerr;

	if (derr.IsntError())
	{
		// Enable the next button.
		mParentGlobalCWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

		// Select all items.
		mPGPdisksList.SelItemRange(TRUE, 0, mPGPdisksList.GetCount());
	}

	if (derr.IsError())
	{
		mParentGlobalCWiz->mDerr = derr;

		mSkipConfirmCancel = TRUE;
		mParentGlobalCWiz->PressButton(PSBTN_CANCEL);	// NOT EndDialog
	}
}

// OnQueryCancel confirms that the user really does want to cancel.

BOOL 
CGlobalCWizSearchPage::OnQueryCancel() 
{
	// Confirm cancellation of creation.
	if (!mSkipConfirmCancel)
	{
		if (DisplayMessage(kPGPdiskConfirmCancelSearching, kPMBS_YesNo, 
			kPMBF_NoButton) == kUR_No)
		{
			return FALSE;
		}
	}

	return CPropertyPage::OnQueryCancel();
}

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled, and begin PGPdisk searching.

BOOL 
CGlobalCWizSearchPage::OnSetActive() 
{
	pgpAssertAddrValid(mParentGlobalCWiz, CGlobalCWizardSheet);

	// Ask a separate thread to search for PGPdisks.
	if (!mFinishedSearch && !mIsThreadActive)
	{
		SearchPGPdiskInfo	SPI;
		DualErr				derr;

		pgpAssertAddrValid(mSearchThread, CGlobalCSearchPGPdiskThread);
		mParentGlobalCWiz->SetWizardButtons(NULL);

		mSearchStatusText = GetCommonString(kPGPdiskSearchingSearchMsgString);
		UpdateData(FALSE);

		SPI.callerHwnd	= GetSafeHwnd();
		SPI.listBoxHwnd	= mPGPdisksList.GetSafeHwnd();
		SPI.dirStaticHwnd = mDirNameStatic.GetSafeHwnd();

		// Start the search thread.
		mIsThreadActive = TRUE;
		derr = mSearchThread->CallPGPdiskSearch(&SPI);

		// Set the progress message.
		if (derr.IsntError())
		{
			mParentGlobalCWiz->SendDlgItemMessage(IDCANCEL, WM_KILLFOCUS, 
				NULL, NULL);
		}

		if (derr.IsError())
		{
			mIsThreadActive = FALSE;

			mParentGlobalCWiz->mDerr = derr;
			mParentGlobalCWiz->EndDialog(IDCANCEL);
		}
	}
	else if (mFinishedSearch)
	{
		mParentGlobalCWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	}

	return CPropertyPage::OnSetActive();
}

// OnWizardNext is called when the user clicks on the 'Next' button.

LRESULT 
CGlobalCWizSearchPage::OnWizardNext() 
{
	DualErr derr;

	if (CPropertyPage::OnWizardNext() == -1)
		return -1;

	// Convert all selected PGPdisks.
	ConvertSelectedPGPdisks();

	return 0;
}


////////////////////////////////////////////////////////////////////////////
// CGlobalCWizSearchPage protected custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////////

// The CGlobalCWizSearchPage data exchange function.

void 
CGlobalCWizSearchPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CGlobalCWizSearchPage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		DDX_Control(pDX, IDC_DIRNAME_TEXT, mDirNameStatic);
		DDX_Control(pDX, IDC_PGPDISKS_LIST, mPGPdisksList);
		DDX_Text(pDX, IDC_DIRNAME_TEXT, mDirNameText);
		DDX_Text(pDX, IDC_SEARCHSTATUS_TEXT, mSearchStatusText);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


////////////////////////////////////////////////////////////
// CGlobalCWizSearchPage protected default message handlers
////////////////////////////////////////////////////////////

// OnCancel is called when the cancel button is pressed.

void 
CGlobalCWizSearchPage::OnCancel() 
{	
	if (mIsThreadActive)
	{
		// Tell it to delete itself since we may be gone by the time it
		// finishes the cancel.

		pgpAssertAddrValid(mSearchThread, CGlobalCSearchPGPdiskThread);

		mIsCTAutoDelete = TRUE;
		mSearchThread->m_bAutoDelete = TRUE;

		mSearchThread->CancelPGPdiskSearch();
	}

	CPropertyPage::OnCancel();
}

// OnHelpInfo handles context-sensitive help.

BOOL 
CGlobalCWizSearchPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CGlobalCWizSearchPage::OnPaint() 
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

// ConvertSelectedPGPdisks converts all selected PGPdisks.

void 
CGlobalCWizSearchPage::ConvertSelectedPGPdisks()
{
	try
	{
		PGPUInt32 numSelected = mPGPdisksList.GetSelCount();

		// If there are any PGPdisk selected...
		if (numSelected > 0)
		{
			int			*indexArray;
			PGPUInt32	i;

			indexArray = new int[numSelected];
			mPGPdisksList.GetSelItems(numSelected, indexArray);

			// ... for each PGPdisk selected..
			for (i = 0; i < numSelected; i++)
			{
				CString		path;
				DualErr		derr;
				PGPBoolean	userSaysCancel	= FALSE;

				mPGPdisksList.GetText(indexArray[i], path);

				// ... convert it.
				derr = App->ConvertPGPdiskFromGlobalWizard(path, 
					&userSaysCancel);

				if (derr.IsntError())
				{
					if (userSaysCancel)
						break;
				}

				if (derr.IsError())
				{
					ReportError(kPGDMajorError_PGPdiskConversionFailed, derr);
				}
			}

			delete[] indexArray;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

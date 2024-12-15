//////////////////////////////////////////////////////////////////////////////
// CDiskWizADKPage.cpp
//
// Implementation of class CDiskWizADKPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizADKPage.cpp,v 1.4 1999/02/26 04:09:56 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"

#include "PGPclx.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpAdminPrefs.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"

#undef IDB_CREDITS1
#undef IDB_CREDITS4
#undef IDB_CREDITS8
#undef IDC_CREDITS

#include "CommonStrings.h"
#include "GlobalPGPContext.h"
#include "PGPdiskPublicKeyUtils.h"

#include "CDiskWizADKPage.h"
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

BEGIN_MESSAGE_MAP(CDiskWizADKPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDiskWizADKPage)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////
// CDiskWizADKPage public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDiskWizADKPage, CPropertyPage)

// The CDiskWizADKPage default constructor.

CDiskWizADKPage::CDiskWizADKPage()
	 : CPropertyPage(CDiskWizADKPage::IDD)
{
	//{{AFX_DATA_INIT(CDiskWizADKPage)
	mADKString = _T("");
	//}}AFX_DATA_INIT

	mParentDiskWiz = NULL;
	mUseADK = FALSE;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CDiskWizADKPage destructor.

CDiskWizADKPage::~CDiskWizADKPage()
{
}


//////////////////////////////////////////////////
// CDiskWizADKPage public default message handlers
//////////////////////////////////////////////////

// OnQueryCancel confirms that the user really does want to cancel.

BOOL 
CDiskWizADKPage::OnQueryCancel() 
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
// correct buttons are enabled/disabled.

BOOL 
CDiskWizADKPage::OnSetActive() 
{
	PGPBoolean skipThisPage = TRUE;

	pgpAssertAddrValid(mParentDiskWiz, CDiskWizardSheet);
	mParentDiskWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

#if PGP_BUSINESS_SECURITY

	{
		char			userID[kMaxStringSize];
		DualErr			fatalDerr, notFatalDerr;
		PGPBoolean		gotAllKeys, isThereADK;
		PGPKeySetRef	allKeys;

		gotAllKeys = isThereADK = FALSE;

		// Is there an ADK?
		notFatalDerr = IsTherePGPdiskADK(&isThereADK);

		// Load default key ring if ADK.
		if (notFatalDerr.IsntError() && isThereADK)
		{
			fatalDerr = PGPOpenDefaultKeyRings(GetGlobalPGPContext(), 0, 
				&allKeys);
			gotAllKeys = fatalDerr.IsntError();
		}

		// Get ADK user ID.
		if (fatalDerr.IsntError() && isThereADK)
		{
			fatalDerr = GetADKUserID(allKeys,userID, kMaxStringSize);
		}

		// Show ADK user ID.
		if (fatalDerr.IsntError() && isThereADK)
		{
			skipThisPage = FALSE;
			mUseADK = TRUE;

			mADKString = userID;

			UpdateData(FALSE);
		}

		if (fatalDerr.IsError())
		{
			mParentDiskWiz->mDerr = DualErr(kPGDMinorError_CantGetADK);

			skipThisPage = TRUE;
			mSkipConfirmCancel = TRUE;

			mParentDiskWiz->PostMessage(WM_CLOSE);
		}

		if (gotAllKeys)
			PGPFreeKeySet(allKeys);
	}

#endif	// PGP_BUSINESS_SECURITY

	if (skipThisPage)
		return FALSE;
	else
		return CPropertyPage::OnSetActive();
}


//////////////////////////////////////////////////////////////////////
// CDiskWizADKPage protected custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////

// The CDiskWizADKPage data exchange function.

void 
CDiskWizADKPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CDiskWizADKPage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
	DDX_Text(pDX, IDC_ADK, mADKString);
	//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


/////////////////////////////////////////////////////
// CDiskWizADKPage protected default message handlers
/////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizADKPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CDiskWizADKPage::OnPaint() 
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
	
	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
	// don't fool with it.

	oldBitmap = (HBITMAP) SelectObject(memDC, mParentDiskWiz->mSidebarBitmap);

	DC.BitBlt(sidebarRect.left, sidebarRect.top, sidebarRect.right, 
		sidebarRect.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}

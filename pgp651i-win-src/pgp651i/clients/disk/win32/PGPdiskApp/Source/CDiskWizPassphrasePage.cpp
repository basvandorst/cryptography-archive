//////////////////////////////////////////////////////////////////////////////
// CDiskWizPassphrasePage.cpp
//
// Implementation of class CDiskWizPassphrasePage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizPassphrasePage.cpp,v 1.8 1999/03/08 23:32:42 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CDiskWizPassphrasePage.h"
#include "CDiskWizardSheet.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


////////////
// Constants
////////////

// Hide typing by default?
const PGPBoolean kDefaultDiskWizPassHideTyping = TRUE;


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_PASSPHRASE_EDIT,	IDH_PGPDISKAPP_DISKWIZPASSEDIT1, 
	IDC_CONFIRM_EDIT,		IDH_PGPDISKAPP_DISKWIZPASSEDIT2, 
	IDC_HIDETYPE_CHECK,		IDH_PGPDISKAPP_DISKWIZPASSHIDECHECK, 
	IDC_QUALITY_BAR,		IDH_PGPDISKAPP_DISKWIZPASSQUALITY, 
    IDC_CAPSLOCK_TEXT,		((PGPUInt32) -1), 
	IDC_SIDEBAR,			((PGPUInt32) -1), 
	0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CDiskWizPassphrasePage, CPropertyPage)
	//{{AFX_MSG_MAP(CDiskWizPassphrasePage)
	ON_WM_HELPINFO()
	ON_EN_CHANGE(IDC_PASSPHRASE_EDIT, OnChangePassphrase)
	ON_BN_CLICKED(IDC_HIDETYPE_CHECK, OnHideTypeCheck)
	ON_EN_KILLFOCUS(IDC_CONFIRM_EDIT, OnKillFocusConfirmEdit)
	ON_EN_KILLFOCUS(IDC_PASSPHRASE_EDIT, OnKillFocusPassphraseEdit)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CDiskWizPassphrasePage public custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDiskWizPassphrasePage, CPropertyPage)

// The CDiskWizPassphrasePage default constructor.

CDiskWizPassphrasePage::CDiskWizPassphrasePage()
	 : CPropertyPage(CDiskWizPassphrasePage::IDD)
{
	//{{AFX_DATA_INIT(CDiskWizPassphrasePage)
	mHideTypeValue = FALSE;
	//}}AFX_DATA_INIT

	mParentDiskWiz	= NULL;
	mIHadFocus		= NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;

	// Did the secure edit controls initialize properly?
	mInitErr = mPassphraseEdit.mInitErr;

	if (mInitErr.IsntError())
	{
		mInitErr = mConfirmationEdit.mInitErr;
	}
}

// The CDiskWizPassphrasePage destructor.

CDiskWizPassphrasePage::~CDiskWizPassphrasePage()
{
}

// PreTranslateMessage sees all messages before MFC filters them. We need to
// check for the caps lock key and the return key.

BOOL 
CDiskWizPassphrasePage::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if ((pMsg->wParam & 0x7F) == VK_RETURN)
		{
			CWnd	*pNextButton	= mParentDiskWiz->GetDlgItem(ID_WIZNEXT);
			pNextButton->PostMessage(WM_LBUTTONDOWN, MK_LBUTTON, 0);
			pNextButton->PostMessage(WM_LBUTTONUP, 0, 0);
			return TRUE;
		}
		break;

	case WM_KEYUP:
		SetCapsLockMessageState();
		break;
	}

	return CPropertyPage::PreTranslateMessage(pMsg);
}


/////////////////////////////////////////////////////////
// CDiskWizPassphrasePage public default message handlers
/////////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CDiskWizPassphrasePage::OnSetActive() 
{
	pgpAssertAddrValid(mParentDiskWiz, CDiskWizardSheet);
	mParentDiskWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	return CPropertyPage::OnSetActive();
}

// OnWizardNext is called when the 'Next' button is pressed. We perform
// data validation.

LRESULT 
CDiskWizPassphrasePage::OnWizardNext() 
{
	PGPBoolean	failThis	= FALSE;
	PGPBoolean	lengthBelowMin;

	if (CPropertyPage::OnWizardNext() == -1)
		return -1;

	// Make sure the passphrase equals the confirmation.
	if ((* mPassphraseEdit.mContents) != (* mConfirmationEdit.mContents))
	{
		failThis = TRUE;
		ReportError(kPGDMajorError_PassphraseNotConfirmed);
	}

	// Check if the passphrase is above the legal length.
	if (!failThis)
	{
		lengthBelowMin = (mPassphraseEdit.mContents->GetLength() < 
			kMinPassphraseLength);

		if (lengthBelowMin)				// below the minimum legal size?
		{
			failThis = TRUE;
			ReportError(kPGDMajorError_PassphraseTooSmall);
		}
	}

	if (failThis)
	{
		mPassphraseEdit.ClearEditContents();
		mConfirmationEdit.ClearEditContents();
		mPassphraseEdit.SetFocus();

		return -1;
	}
	else
	{
		return 0;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDiskWizPassphrasePage protected custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////////

// SetCapsLockMessageState shows or hides the caps lock message as needed.

void 
CDiskWizPassphrasePage::SetCapsLockMessageState()
{
	PGPUInt16 bob = GetKeyState(VK_CAPITAL);

	if (GetKeyState(VK_CAPITAL) & 1)
	{
		mCapsLockStatic.ShowWindow(SW_SHOW);
	}
	else
	{
		mCapsLockStatic.ShowWindow(SW_HIDE);
	}
}

// The CDiskWizPassphrasePage data exchange function.

void 
CDiskWizPassphrasePage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CDiskWizPassphrasePage)
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		DDX_Control(pDX, IDC_QUALITY_BAR, mQualityBar);
		DDX_Control(pDX, IDC_PASSPHRASE_EDIT, mPassphraseEdit);
		DDX_Control(pDX, IDC_CONFIRM_EDIT, mConfirmationEdit);
		DDX_Control(pDX, IDC_CAPSLOCK_TEXT, mCapsLockStatic);
		DDX_Check(pDX, IDC_HIDETYPE_CHECK, mHideTypeValue);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnChangePassphrase is called whenever the user types a letter into the
// passphrase edit box. We adjust the "Password Quality" progress bar.

void 
CDiskWizPassphrasePage::OnChangePassphrase() 
{
	PGPUInt16 quality ;
	
	quality = App->CalcPassphraseQuality(mPassphraseEdit.mContents);

	mQualityBar.SetPos(quality);
}

// We override OnHideTypeCheck to set the 'hide typing' behavior of the edit
// boxes

void 
CDiskWizPassphrasePage::OnHideTypeCheck() 
{
	UpdateData(TRUE);

	mPassphraseEdit.SetHideTypingPref(mHideTypeValue);
	mConfirmationEdit.SetHideTypingPref(mHideTypeValue);

	mIHadFocus->SetFocus();
}

// OnKillFocusConfirmEdit is called when the confirmation box is losing focus.

void 
CDiskWizPassphrasePage::OnKillFocusConfirmEdit() 
{
	mIHadFocus = &mConfirmationEdit;	
}

// OnKillFocusPassphraseEdit is called when the passphrase box is losing
// focus.

void 
CDiskWizPassphrasePage::OnKillFocusPassphraseEdit() 
{
	mIHadFocus = &mPassphraseEdit;
}


////////////////////////////////////////////////////////////
// CDiskWizPassphrasePage protected default message handlers
////////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizPassphrasePage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CDiskWizPassphrasePage::OnInitDialog() 
{
	try
	{
		CString	bareName, temp;

		CPropertyPage::OnInitDialog();

		// This must be called first to make MFC happy.
		UpdateData(FALSE);

		// Prepare the passphrase quality bar.
		#if (_MFC_VER < 0x0600)
			mQualityBar.SetRange(0, 100);
		#else
			mQualityBar.SetRange32(0, 100);
		#endif	// (_MFC_VER < 0x0600)

		mQualityBar.SetPos(0);

		// Prepare the dialog message text.
		temp = mParentDiskWiz->mPath;
		bareName = temp.Right(temp.GetLength() - temp.ReverseFind('\\') - 1);

		// Show the caps lock message if necessary.
		SetCapsLockMessageState();

		// Set the hide-typing checkbox.
		mHideTypeValue = kDefaultDiskWizPassHideTyping;

		// Set the hide-typing pref of the edit boxes.
		mPassphraseEdit.SetHideTypingPref(mHideTypeValue);
		mConfirmationEdit.SetHideTypingPref(mHideTypeValue);

		// Set the maximum text length of the edit boxes.
		mPassphraseEdit.SetMaxSizeContents(kMaxPassphraseLength);
		mConfirmationEdit.SetMaxSizeContents(kMaxPassphraseLength);

		// Set the focus to the topmost box.
		mPassphraseEdit.SetFocus();
		mIHadFocus = &mPassphraseEdit;

		UpdateData(FALSE);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return TRUE;
}

// We override OnPaint to draw the sidebar graphic at the correct depth.

void 
CDiskWizPassphrasePage::OnPaint() 
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

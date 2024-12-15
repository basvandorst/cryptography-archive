//////////////////////////////////////////////////////////////////////////////
// CConfirmPassphraseDialog.cpp
//
// Implementation of the CConfirmPassphraseDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConfirmPassphraseDialog.cpp,v 1.6 1999/02/26 04:09:56 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskLowLevelUtils.h"
#include "StringAssociation.h"

#include "CConfirmPassphraseDialog.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


////////////
// Constants
////////////

// Hide typing by default?
const PGPBoolean kDefaultConfirmPassHideTyping = TRUE;


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDOK,					IDH_PGPDISKAPP_CONFIRMPASSOK, 
	IDCANCEL,				IDH_PGPDISKAPP_CONFIRMPASSCANCEL, 
	IDC_HIDETYPE_CHECK,		IDH_PGPDISKAPP_CONFIRMPASSHIDECHECK, 
	IDC_PASSPHRASE_EDIT,	IDH_PGPDISKAPP_CONFIRMPASSEDIT1, 
	IDC_QUALITY_BAR,		IDH_PGPDISKAPP_CONFIRMPASSQUALITY, 
	IDC_CONFIRM_EDIT,		IDH_PGPDISKAPP_CONFIRMPASSEDIT2, 
	IDC_RO_CHECK,			IDH_PGPDISKAPP_CONFIRMPASSROCHECK, 
	IDC_NEWPASSMSG_TEXT,	((PGPUInt32) -1), 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CConfirmPassphraseDialog, CDialog)
	//{{AFX_MSG_MAP(CConfirmPassphraseDialog)
	ON_BN_CLICKED(IDC_HIDETYPE_CHECK, OnHideTypeCheck)
	ON_EN_CHANGE(IDC_PASSPHRASE_EDIT, OnChangePassphrase)
	ON_EN_KILLFOCUS(IDC_PASSPHRASE_EDIT, OnKillFocusPassphrase)
	ON_EN_KILLFOCUS(IDC_CONFIRM_EDIT, OnKillFocusConfirmation)
	ON_BN_CLICKED(IDC_RO_CHECK, OnROCheck)
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////
// CConfirmPassphraseDialog public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////////

// The CConfirmPassphraseDialog default constructor.

CConfirmPassphraseDialog::CConfirmPassphraseDialog(
	ConfirmPassDialogType dlgType)

	: CDialog(CConfirmPassphraseDialog::IDD, NULL)
{
	try
	{
		//{{AFX_DATA_INIT(CConfirmPassphraseDialog)
		mHideTypeValue = FALSE;
		mMessageValue = _T("");
		//}}AFX_DATA_INIT

		mIHadFocus				= NULL;
		mConfirmPassDialogType	= dlgType;
		mReadOnlyValue			= FALSE;
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	// Did the secure edit controls initialize properly?
	if (mInitErr.IsntError())
	{
		mInitErr = mPassphraseEdit.mInitErr;
	}

	if (mInitErr.IsntError())
	{
		mInitErr = mConfirmationEdit.mInitErr;
	}

	// Adjust for the correct resource template.
	if (mInitErr.IsntError())
	{
		if (mConfirmPassDialogType == kCPDT_Add)
		{
			m_lpszTemplateName = (LPCTSTR) IDD_CONFIRMPASS1_DLG;
		}
		else if (mConfirmPassDialogType == kCPDT_Change)
		{
			m_lpszTemplateName = (LPCTSTR) IDD_CONFIRMPASS2_DLG;
		}

		// Stolen from MFC.
		if (HIWORD(m_lpszTemplateName) == 0)
		{
			m_nIDHelp = LOWORD((DWORD) m_lpszTemplateName);
		}
	}
}

// The CConfirmPassphraseDialog destructor.

CConfirmPassphraseDialog::~CConfirmPassphraseDialog()
{
}

// AskForPassphrase is a wrapper around DoModal. It asks for a passphrase for
// the specified PGPdisk.

DualErr 
CConfirmPassphraseDialog::AskForPassphrase(LPCSTR path)
{
	DualErr		derr;
	PGPInt32	result;
	
	pgpAssertStrValid(path);

	try
	{
		mPath = path;
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsntError())
	{
		derr = ValidatePGPdisk(mPath);
	}

	if (derr.IsntError())
	{
		result = DoModal();

		switch (result)
		{
		case -1:
			derr = DualErr(kPGDMinorError_DialogDisplayFailed);
			break;

		case IDCANCEL:
			derr = DualErr(kPGDMinorError_UserAbort);
			break;
		}
	}

	return derr;
}

// DisableReadOnly can be called to disable the read-only button. This is only
// useful for kSPDT_Add dialogs, because in all other types the button is
// hidden no matter what.

void 
CConfirmPassphraseDialog::DisableReadOnlyButton()
{
	pgpAssert(mConfirmPassDialogType == kCPDT_Add);
	mReadOnlyButton.EnableWindow(FALSE);
}

// SetReadOnlyValue can be called to set the value of the read-only checkbox
// before displaying the dialog. This is only useful for kSPDT_Add dialogs.

void 
CConfirmPassphraseDialog::SetReadOnlyValue(PGPBoolean readOnly)
{
	pgpAssert(mConfirmPassDialogType == kCPDT_Add);

	mReadOnlyButton.SetCheck(readOnly ? 1 : 0);
	mReadOnlyValue = (mReadOnlyButton.GetCheck() == 1 ? TRUE : FALSE);
}

// PreTranslateMessage sees all messages before MFC filters them. We need to
// check for the caps lock key.

BOOL 
CConfirmPassphraseDialog::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYUP:
		SetCapsLockMessageState();
		break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


//////////////////////////////////////////////////////////////////////////////
// CConfirmPassphraseDialog protected custom functions and non-default handlrs
//////////////////////////////////////////////////////////////////////////////

// SetCapsLockMessageState shows or hides the caps lock message as needed.

void 
CConfirmPassphraseDialog::SetCapsLockMessageState()
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

// The CConfirmPassphraseDialog data exchange function.

void 
CConfirmPassphraseDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CConfirmPassphraseDialog)
		DDX_Control(pDX, IDC_NEWPASSMSG_TEXT, mMessageStatic);
		DDX_Control(pDX, IDC_CAPSLOCK_TEXT, mCapsLockStatic);
		DDX_Control(pDX, IDOK, mOKButton);
		DDX_Control(pDX, IDCANCEL, mCancelButton);
		DDX_Control(pDX, IDC_PASSPHRASE_EDIT, mPassphraseEdit);
		DDX_Control(pDX, IDC_CONFIRM_EDIT, mConfirmationEdit);
		DDX_Control(pDX, IDC_QUALITY_BAR, mQualityBar);
		DDX_Check(pDX, IDC_HIDETYPE_CHECK, mHideTypeValue);
		DDX_Text(pDX, IDC_NEWPASSMSG_TEXT, mMessageValue);
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
CConfirmPassphraseDialog::OnChangePassphrase() 
{
	PGPUInt16 quality = App->CalcPassphraseQuality(mPassphraseEdit.mContents);

	mQualityBar.SetPos(quality);
}

// We override OnHideTypeCheck to set the 'hide typing' behavior of the edit
// boxes

void 
CConfirmPassphraseDialog::OnHideTypeCheck() 
{
	UpdateData(TRUE);

	mPassphraseEdit.SetHideTypingPref(mHideTypeValue);
	mConfirmationEdit.SetHideTypingPref(mHideTypeValue);

	mIHadFocus->SetFocus();
}

// OnKillFocusConfirmation is called when the confirmation box is losing
// focus.

void 
CConfirmPassphraseDialog::OnKillFocusConfirmation() 
{
	mIHadFocus = &mConfirmationEdit;
}

// OnKillFocusPassphrase is called when the passphrase box is losing focus.

void 
CConfirmPassphraseDialog::OnKillFocusPassphrase() 
{
	mIHadFocus = &mPassphraseEdit;
}

// OnROCheck is called when the read-only check box is clicked.

void 
CConfirmPassphraseDialog::OnROCheck() 
{
	mReadOnlyValue = (mReadOnlyButton.GetCheck() == 1 ? TRUE : FALSE);
}


//////////////////////////////////////////////////////////////
// CConfirmPassphraseDialog protected default message handlers
//////////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CConfirmPassphraseDialog::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnInitDialog performs dialog box initialization.

BOOL 
CConfirmPassphraseDialog::OnInitDialog() 
{	
	try
	{
		CString	bareName, dialogMsg;
		DualErr	derr;

		pgpAssert(!mPath.IsEmpty());

		CDialog::OnInitDialog();

		// Prepare the passphrase quality bar.
		#if (_MFC_VER < 0x0600)
			mQualityBar.SetRange(0, 100);
		#else
			mQualityBar.SetRange32(0, 100);
		#endif	// (_MFC_VER < 0x0600)

		mQualityBar.SetPos(0);

		// Prepare the dialog message text.
		GetBareName(mPath, &bareName);

		switch (mConfirmPassDialogType)
		{
		case kCPDT_Add:
			mReadOnlyButton.SubclassWindow(
				GetDlgItem(IDC_RO_CHECK)->GetSafeHwnd());
			dialogMsg = GetCommonString(kPGPdiskCPDTAddPassMsg);
			break;

		case kCPDT_Change:
			dialogMsg = GetCommonString(kPGPdiskCPDTChangePassMsg);
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
			
		// Format and truncate the message if necessary.
		App->FormatFitStringToWindow(dialogMsg, bareName, &mMessageValue, 
			&mMessageStatic);

		// Show the caps lock message if necessary.
		SetCapsLockMessageState();

		// Set the hide-typing checkbox.
		mHideTypeValue = kDefaultConfirmPassHideTyping;

		// Set the hide-typing pref of the edit boxes.
		mPassphraseEdit.SetHideTypingPref(mHideTypeValue);
		mConfirmationEdit.SetHideTypingPref(mHideTypeValue);

		// Set the maximum text length of the edit boxes.
		mPassphraseEdit.SetMaxSizeContents(kMaxPassphraseLength);
		mConfirmationEdit.SetMaxSizeContents(kMaxPassphraseLength);
		UpdateData(FALSE);

		// Set the focus to the topmost box.
		mPassphraseEdit.SetFocus();
		mIHadFocus = &mPassphraseEdit;

		// Center on screen.
		CenterWindow(GetDesktopWindow());

		// Put us in front.
		SetForegroundWindow();

		// Put on top if necessary.
		App->TweakOnTopAttribute(this);

		// Show now to thwart evil MFC auto-centering.
		ShowWindow(SW_SHOW);
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return FALSE;	            
}

// OnOK handles the OK button. We verify that the passphrase matches the
// confirmation.

void 
CConfirmPassphraseDialog::OnOK() 
{
	PGPBoolean	failThis	= FALSE;
	PGPBoolean	lengthBelowMin;

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

	// Make sure the new passphrase doesn't already exist.
	if (!failThis)
	{
		CWaitCursor	waitCursor;
		DualErr		derr;

		derr = VerifyNormalPassphrase(mPath, mPassphraseEdit.mContents);

		if (derr.IsntError())
		{
			failThis = TRUE;
			ReportError(kPGDMajorError_PassphraseAlreadyExists);
		}
	}

	if (failThis)
	{
		mPassphraseEdit.ClearEditContents();
		mConfirmationEdit.ClearEditContents();
		mPassphraseEdit.SetFocus();
	}
	else
	{
		CDialog::OnOK();
	}
}

//////////////////////////////////////////////////////////////////////////////
// CSinglePassphraseDialog.cpp
//
// Implementation of the CSinglePassphraseDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSinglePassphraseDialog.cpp,v 1.1.2.17.2.2 1998/10/28 00:13:30 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskLowLevelUtils.h"

#if PGPDISK_PUBLIC_KEY
#include "PGPdiskPublicKeyUtils.h"
#endif // PGPDISK_PUBLIC_KEY

#include "StringAssociation.h"

#include "CSinglePassphraseDialog.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


////////////
// Constants
////////////

// Hide typing by default?

const PGPBoolean kDefaultSinglePassHideTyping = TRUE;


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDOK,					IDH_PGPDISKAPP_SINGLEPASSOK, 
	IDCANCEL,				IDH_PGPDISKAPP_SINGLEPASSCANCEL, 
	IDC_HIDETYPE_CHECK,		IDH_PGPDISKAPP_SINGLEPASSHIDECHECK, 
	IDC_PASSPHRASE_EDIT,	IDH_PGPDISKAPP_SINGLEPASSEDIT, 
	IDC_RO_CHECK,			IDH_PGPDISKAPP_SINGLEPASSROCHECK, 
	IDC_DRIVE_COMBO,		IDH_PGPDISKAPP_SINGLEPASSDRIVECOMBO, 
	IDC_SINGLEPASSMSG_TEXT,	((PGPUInt32) -1), 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CSinglePassphraseDialog, CDialog)
	//{{AFX_MSG_MAP(CSinglePassphraseDialog)
	ON_BN_CLICKED(IDC_HIDETYPE_CHECK, OnHideTypeCheck)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_RO_CHECK, OnROCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// CSinglePassphraseDialog public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////////

// The CSinglePassphraseDialog default constructor. You can pass 'dialogMsg'
// to set the dialog box message.

CSinglePassphraseDialog::CSinglePassphraseDialog(SinglePassDialogType dlgType)

	: CDialog(CSinglePassphraseDialog::IDD, NULL)
{
	try
	{
		//{{AFX_DATA_INIT(CSinglePassphraseDialog)
		mHideTypeValue = FALSE;
		mMessageValue = _T("");
		//}}AFX_DATA_INIT

		mSinglePassDialogType = dlgType;

		// Did the secure edit control initialize properly?
		if (mInitErr.IsntError())
		{
			mInitErr = mPassphraseEdit.mInitErr;
		}

		// Adjust for the correct resource template.
		if (mInitErr.IsntError())
		{
			switch (mSinglePassDialogType)
			{
			case kSPDT_Mount:
				m_lpszTemplateName = (LPCTSTR) IDD_SINGLEPASS1_DLG;
				break;

			case kSPDT_PublicKey:
				m_lpszTemplateName = (LPCTSTR) IDD_SINGLEPASS3_DLG;
				break;

			default:
				m_lpszTemplateName = (LPCTSTR) IDD_SINGLEPASS2_DLG;
				break;
			}

			// Stolen from MFC.
			if (HIWORD(m_lpszTemplateName) == 0)
			{
				m_nIDHelp = LOWORD((DWORD) m_lpszTemplateName);
			}
		}
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}
}

// The CSinglePassphraseDialog destructor.

CSinglePassphraseDialog::~CSinglePassphraseDialog()
{
}

// AskForPassphrase is a wrapper around DoModal. It asks for a passphrase
// from the specified PGPdisk.

DualErr 
CSinglePassphraseDialog::AskForPassphrase(
	LPCSTR		path, 
	PGPBoolean	forceReadOnly, 
	PGPBoolean	needMaster)
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
		mNeedMaster = needMaster;
		mReadOnlyValue = forceReadOnly;

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

// PreTranslateMessage sees all messages before MFC filters them. We need to
// check for the caps lock key.

BOOL 
CSinglePassphraseDialog::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYUP:
		SetCapsLockMessageState();
		break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

// SetReadOnlyValue can be called to set the value of the read-only checkbox
// before displaying the dialog. This is only useful for kSPDT_Mount dialogs.

void 
CSinglePassphraseDialog::SetReadOnlyValue(PGPBoolean readOnly)
{
	pgpAssert(mSinglePassDialogType == kSPDT_Mount);

	mReadOnlyButton.SetCheck(readOnly ? 1 : 0);
	mReadOnlyValue = (mReadOnlyButton.GetCheck() == 1 ? TRUE : FALSE);
}

// DisableReadOnly can be called to disable the read-only button. This is only
// useful for kSPDT_Mount dialogs, because in all other types the button is
// hidden no matter what.

void 
CSinglePassphraseDialog::DisableReadOnlyButton()
{
	pgpAssert(mSinglePassDialogType == kSPDT_Mount);
	mReadOnlyButton.EnableWindow(FALSE);
}


//////////////////////////////////////////////////////////////////////////////
// CSinglePassphraseDialog protected custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////////

// SetCapsLockMessageState shows or hides the caps lock message as needed.

void 
CSinglePassphraseDialog::SetCapsLockMessageState()
{
	PGPUInt16 bob = GetKeyState(VK_CAPITAL);

	if (GetKeyState(VK_CAPITAL) & 1)
		mCapsLockStatic.ShowWindow(SW_SHOW);
	else
		mCapsLockStatic.ShowWindow(SW_HIDE);
}

// The CSinglePassphraseDialog data exchange function.

void 
CSinglePassphraseDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CSinglePassphraseDialog)
		DDX_Control(pDX, IDC_SINGLEPASSMSG_TEXT, mMessageStatic);
		DDX_Control(pDX, IDC_CAPSLOCK_TEXT, mCapsLockStatic);
		DDX_Control(pDX, IDOK, mOKButton);
		DDX_Control(pDX, IDCANCEL, mCancelButton);
		DDX_Control(pDX, IDC_PASSPHRASE_EDIT, mPassphraseEdit);
		DDX_Check(pDX, IDC_HIDETYPE_CHECK, mHideTypeValue);
		DDX_Text(pDX, IDC_SINGLEPASSMSG_TEXT, mMessageValue);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// We override OnHideTypeCheck to set the 'hide typing' behavior of the edit
// box.

void 
CSinglePassphraseDialog::OnHideTypeCheck() 
{
	UpdateData(TRUE);

	mPassphraseEdit.SetHideTypingPref(mHideTypeValue);
	mPassphraseEdit.SetFocus();
}

// OnROCheck is called when the read-only check box is clicked.

void 
CSinglePassphraseDialog::OnROCheck() 
{
	mReadOnlyValue = (mReadOnlyButton.GetCheck() == 1 ? TRUE : FALSE);
}


/////////////////////////////////////////////////////
// CSinglePassphraseDialog protected default handlers
/////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CSinglePassphraseDialog::OnHelpInfo(HELPINFO *pHelpInfo)
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
CSinglePassphraseDialog::OnInitDialog() 
{
	CString	bareName, dialogMsg;
	DualErr	derr;

	pgpAssert(!mPath.IsEmpty());

	CDialog::OnInitDialog();
	
	// Prepare the dialog message text.
	GetBareName(mPath, &bareName);

	switch (mSinglePassDialogType)
	{
	case kSPDT_Change:
		dialogMsg = GetCommonString(kPGPdiskSPDTChangePassMsg);
		break;

	case kSPDT_Master:
		dialogMsg = GetCommonString(kPGPdiskSPDTMasterPassMsg);
		break;

	case kSPDT_Mount:
		if (mNeedMaster)
			dialogMsg = GetCommonString(kPGPdiskSPDTMasterPassMsg);
		else
			dialogMsg = GetCommonString(kPGPdiskSPDTMountPassMsg);
		break;

	case kSPDT_PublicKey:
		dialogMsg = GetCommonString(kPGPdiskSPDTPublicKeyPassMsg);
		break;

	case kSPDT_Remove:
		dialogMsg = GetCommonString(kPGPdiskSPDTRemovePassMsg);
		break;

	default:
		pgpAssert(FALSE);
		break;
	}

	// Format and truncate the message if necessary.
	App->FormatFitStringToWindow(dialogMsg, bareName, &mMessageValue, 
		&mMessageStatic);
	
	// Initialize controls for the mount dialog.
	if (mSinglePassDialogType == kSPDT_Mount)
	{
		PGPBoolean	forceReadOnly;
		PGPUInt8	prefDrive;

		mReadOnlyButton.SubclassWindow(
			GetDlgItem(IDC_RO_CHECK)->GetSafeHwnd());

		mDriveCombo.SubclassWindow(
			GetDlgItem(IDC_DRIVE_COMBO)->GetSafeHwnd());

		// Any drive letters free?
		if (!App->AreAnyDriveLettersFree())
		{
			EndDialog(IDCANCEL);
		}

		// Initialize the combo box.
		mDriveCombo.InitDriveLetters();

		// Get the preferred drive letter from the PGPdisk and set the
		// combo box to it.

		prefDrive = GetDriveLetterPref(mPath);
		mDriveCombo.SetDriveLetter(prefDrive);

		// Check if we must force a read-only mount.
		forceReadOnly = mReadOnlyValue;

		if (!forceReadOnly)
		{
#if PGPDISK_BETAVERSION

			forceReadOnly = App->IsFileInUse(mPath) || 
				App->IsFileReadOnly(mPath) ||
				App->HasBetaTimedOut();

#elif PGPDISK_DEMOVERSION

			forceReadOnly = App->IsFileInUse(mPath) || 
				App->IsFileReadOnly(mPath) ||
				App->HasDemoTimedOut();

#else // !PGPDISK_BETAVERSION && !PGPDISK_DEMOVERSION

			forceReadOnly = App->IsFileInUse(mPath) || 
				App->IsFileReadOnly(mPath);

#endif // PGPDISK_BETAVERSION
		}

		if (!forceReadOnly)
		{
			if (IsPGPdiskNotEncrypted(mPath))
			{
				forceReadOnly = TRUE;
				DisplayMessage(kPGPdiskPGPdiskIsNotEncrypted);
			}
		}

		// If path is UNC and server name is local computer, force
		// read-only at this point (should have been resolved on Admin
		// accounts by the path canonicalizer.)

		if (!forceReadOnly)
		{
			if (IsUNCPath(mPath) && IsWinNT4CompatibleMachine())
			{
				CString localCompName, server;

				GetServer(mPath, &server);
				GetLocalComputerName(&localCompName);

				if (server.CompareNoCase(localCompName) == 0)
				{
					DisplayMessage(kPGPdiskForceROOnLoopbackString);
					forceReadOnly = TRUE;
				}
			}
		}

		if (forceReadOnly)
		{
			SetReadOnlyValue(TRUE);
			DisableReadOnlyButton();
		}
		else
		{
			SetReadOnlyValue(FALSE);
		}
	}

	// Show or hide the caps lock text.
	SetCapsLockMessageState();

	// Set the hide-typing checkbox.
	mHideTypeValue = kDefaultSinglePassHideTyping;

	// Set the hide-typing pref of the edit box.
	mPassphraseEdit.SetHideTypingPref(mHideTypeValue);

	// Set the maximum text length of the edit box.
	mPassphraseEdit.SetMaxSizeContents(kMaxPassphraseLength);
	UpdateData(FALSE);

	// Set the focus to the edit box.
	mPassphraseEdit.SetFocus();

	// Center on screen.
	CenterWindow(GetDesktopWindow());

	// Put us in front.
	SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(this);

	// Show now to thwart evil MFC auto-centering.
	ShowWindow(SW_SHOW);

	return FALSE;
}

// OnOK handles the OK button. We verify that the passphrase entered is valid
// for this PGPdisk.

void 
CSinglePassphraseDialog::OnOK() 
{
	CWaitCursor	waitCursor;
	DualErr		derr, dummyErr;
	PGPBoolean	failThis	= FALSE;
	PGPBoolean	isValidPassphrase;

	switch (mSinglePassDialogType)
	{
	case kSPDT_Master:
		// We MUST have a master passphrase. Fail otherwise.
		derr = VerifyMasterPassphrase(mPath, mPassphraseEdit.mContents);
		isValidPassphrase = derr.IsntError();

		if (!isValidPassphrase)
		{
			failThis = TRUE;
			ReportError(kPGDMajorError_IncorrectMasterPassphrase);
		}
		break;

	case kSPDT_Remove:
		// We CAN'T have a master passphrase. Fail if we see one.
		derr = VerifyMasterPassphrase(mPath, mPassphraseEdit.mContents);
		isValidPassphrase = derr.IsntError();

		if (isValidPassphrase)
		{
			failThis = TRUE;
			ReportError(kPGDMajorError_TriedRemoveMasterPass);
		}
		else
		{
			// OK, it isn't a master, check if it is an alternate.
			derr = VerifyNormalPassphrase(mPath, mPassphraseEdit.mContents);
			isValidPassphrase = derr.IsntError();

			if (!isValidPassphrase)
			{
				failThis = TRUE;
				ReportError(kPGDMajorError_IncorrectAlternatePassphrase);
			}
		}
		break;

	case kSPDT_Change:
		// Passphrase can be either a master or an alternate.

		derr = VerifyNormalPassphrase(mPath, mPassphraseEdit.mContents);
		isValidPassphrase = derr.IsntError();

		if (!isValidPassphrase)
		{
			failThis = TRUE;
			ReportError(kPGDMajorError_IncorrectPassphrase);
		}
		break;

	case kSPDT_Mount:
		// Passphrase can be either a master or an alternate or a public key.
		if (mNeedMaster)
			derr = VerifyMasterPassphrase(mPath, mPassphraseEdit.mContents);
		else
			derr = VerifyNormalPassphrase(mPath, mPassphraseEdit.mContents);

		isValidPassphrase = derr.IsntError();

	#if PGPDISK_PUBLIC_KEY

		if (!isValidPassphrase)
		{
			derr = VerifyPublicKeyPassphrase(mPath, 
				mPassphraseEdit.mContents);

			isValidPassphrase = derr.IsntError();
		}

		if (!isValidPassphrase)
		{
			if (ArePGPdiskPassesWiped(mPath))
				derr = DualErr(kPGDMinorError_PGPdiskPassesWiped);
		}

	#endif	// PGPDISK_PUBLIC_KEY

		if (!isValidPassphrase)
		{
			failThis = TRUE;

			switch (derr.mMinorError)
			{
			case kPGDMinorError_IncPassAndKeyNotInRing:
				ReportError(kPGDMajorError_IncPassAndKeyNotInRing);
				break;

			case kPGDMinorError_PGPdiskPassesWiped:
				ReportError(kPGDMajorError_PGPdiskPassesWiped);

				ShowReadOnlyListOfPublicKeys(mPath);
				break;

			default:
				ReportError(kPGDMajorError_IncorrectPassphrase);
				break;
			}
		}

		break;

	case kSPDT_PublicKey:

	#if PGPDISK_PUBLIC_KEY

		// Passphrase MUST be a public key passphrase.
		derr = VerifyPublicKeyPassphrase(mPath, mPassphraseEdit.mContents);
		isValidPassphrase = derr.IsntError();

		if (!isValidPassphrase)
		{
			failThis = TRUE;
			ReportError(kPGDMajorError_IncorrectPublicKeyPassphrase);
		}

	#else	// !PGPDISK_PUBLIC_KEY

		pgpAssert(FALSE);

	#endif	// PGPDISK_PUBLIC_KEY

		break;

	default:
		pgpAssert(FALSE);
		break;
	}

	if (failThis)
	{
		mPassphraseEdit.ClearEditContents();
		mPassphraseEdit.SetFocus();
	}
	else
	{
		CDialog::OnOK();
	}
}

//////////////////////////////////////////////////////////////////////////////
// CMainDialog.cpp
//
// Implementation of the CMainDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CMainDialog.cpp,v 1.1.2.16.2.2 1998/10/25 23:31:25 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskRegistry.h"

#include "CMainDialog.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


////////////
// Constants
////////////

// Size constants for the main window.
const PGPUInt32	kWidthLargeMainDialog	= 268;
const PGPUInt32	kHeightLargeMainDialog	= 69;
const PGPUInt32	kWidthSmallMainDialog	= 132;
const PGPUInt32	kHeightSmallMainDialog	= 36;

// Horizontal bias constants for the main window buttons.
const PGPUInt32	kBigButtonXBias			= 1;
const PGPUInt32	kBigButtonYBias			= 1;
const PGPUInt32	kSmallButtonXBias		= 1;
const PGPUInt32	kSmallButtonYBias		= 2;

// Size constants for the main window buttons
const PGPUInt32	kSideLengthBigButton	= 66;
const PGPUInt32	kSideLengthSmallButton	= 32;

// Size constants for the main window button icons.
const PGPUInt32	kSideLengthBigIcon		= 60;
const PGPUInt32	kSideLengthSmallIcon	= 25;


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_NEWPGPDISK,			IDH_PGPDISKAPP_MAINNEW, 
	IDC_MOUNTPGPDISK,		IDH_PGPDISKAPP_MAINMOUNT, 
	IDC_UNMOUNTPGPDISK,		IDH_PGPDISKAPP_MAINUNMOUNT, 
	IDC_PREFERENCES,		IDH_PGPDISKAPP_MAINPREFS, 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CMainDialog, CDialog)
	//{{AFX_MSG_MAP(CMainDialog)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(IDM_ABOUT, OnAboutBox)
	ON_COMMAND(IDM_ADDPASS, OnAddPassphrase)
	ON_COMMAND(IDM_CHANGEPASS, OnChangePassphrase)
	ON_WM_DROPFILES()
	ON_COMMAND(IDM_REMOVEALL, OnRemoveAlternates)
	ON_COMMAND(IDM_REMOVEPASS, OnRemovePassphrase)
	ON_COMMAND(IDM_EXIT, OnExit)
	ON_BN_CLICKED(IDC_MOUNTPGPDISK, OnMountPGPdisk)
	ON_COMMAND(IDM_UNMOUNTPGPDISK, OnUnmountPGPdisk)
	ON_BN_CLICKED(IDC_NEWPGPDISK, OnNewPGPdisk)
	ON_BN_CLICKED(IDC_PREFERENCES, OnPreferences)
	ON_COMMAND(IDM_HELPCONTENTS, OnHelpContents)
	ON_WM_CLOSE()
	ON_WM_INITMENU()
	ON_WM_HELPINFO()
	ON_WM_SYSCOMMAND()
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_COMMAND(IDM_MOUNTPGPDISK, OnMountPGPdisk)
	ON_COMMAND(IDM_UNMOUNTPGPDISK, OnUnmountPGPdisk)
	ON_COMMAND(IDM_NEWPGPDISK, OnNewPGPdisk)
	ON_COMMAND(IDM_PREFS, OnPreferences)
	ON_COMMAND(IDM_PUBLICKEYS, OnAddRemovePublicKeys)
	ON_COMMAND(IDM_GLOBALCONVERT, OnGlobalConvert)
	ON_BN_CLICKED(IDC_UNMOUNTPGPDISK, OnUnmountPGPdisk)
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CMainDialog public custom functions and non-default message handlers
///////////////////////////////////////////////////////////////////////

// The CMainDialog constructor.

CMainDialog::CMainDialog(CWnd *pParent) : CDialog(CMainDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDialog)
	//}}AFX_DATA_INIT

	mAccelTable = NULL;
	mHIcon = App->LoadIcon(IDI_MAINFRAME);

	mArePastInitDialog = FALSE;

}

// The CMainDialog destructor.
CMainDialog::~CMainDialog()
{
	if (IsntNull(m_hWnd))
		DestroyWindow();
}

// We override PreTranslateMessage so we can handle accelerators, the escape
// key, and the return key properly.

BOOL 
CMainDialog::PreTranslateMessage(MSG *pMsg) 
{
	PGPBoolean weProcessed = FALSE;

	if (mAccelTable && TranslateAccelerator(m_hWnd, mAccelTable, pMsg))
	{
		weProcessed = TRUE;
	}
	else if (pMsg->message == WM_KEYDOWN)
	{
		PGPUInt8 vCode = pMsg->wParam & 0x7F;

		switch (vCode)
		{
		case VK_ESCAPE:
			// Ignore the escape key.
			weProcessed = TRUE;
			break;

		case VK_RETURN:
			// Letting MFC handle return results in main dialog going away.
			if (HasFocus(&mNewButton))
				PostMessage(WM_COMMAND, IDM_NEWPGPDISK, NULL);
			else if (HasFocus(&mMountButton))
				PostMessage(WM_COMMAND, IDM_MOUNTPGPDISK, NULL);
			else if (HasFocus(&mUnmountButton))
				PostMessage(WM_COMMAND, IDM_UNMOUNTPGPDISK, NULL);
			else if (HasFocus(&mPrefsButton))
				PostMessage(WM_COMMAND, IDM_PREFS, NULL);

			weProcessed = TRUE;
			break;
		}
	}

	if (weProcessed)
		return TRUE;
	else
		return CDialog::PreTranslateMessage(pMsg);
}


//////////////////////////////////////////////////////////////////////////
// CMainDialog protected custom functions and non-default message handlers
//////////////////////////////////////////////////////////////////////////

// ShowMainMenu shows the main menu, loading it if necessary.

DualErr 
CMainDialog::ShowMainMenu()
{
	DualErr derr;

	// Load the main menu if it's not loaded already
	if (IsNull(mMainMenu.GetSafeHmenu()))
	{
		if (!mMainMenu.LoadMenu(IDR_MAIN_MENU))
		{
			mInitErr = kPGDMinorError_DialogDisplayFailed;
		}
	}

	// Show the main menu if it's not shown already.
	if (derr.IsntError())
	{
		if (IsNull(GetMenu()))
		{
			// Display the main menu.
			if (!SetMenu(&mMainMenu))
			{
				derr = kPGDMinorError_DialogDisplayFailed;
			}

		#if !PGPDISK_PUBLIC_KEY
			GetMenu()->DeleteMenu(IDM_PUBLICKEYS, MF_BYCOMMAND);
		#endif	// !PGPDISK_PUBLIC_KEY
		}
	}

	return derr;
}

// HideMainMenu hides the main menu.

void 
CMainDialog::HideMainMenu()
{
	pgpAssert(IsntNull(mMainMenu.GetSafeHmenu()));

	SetMenu(NULL);
}

// ConvertMainDialogToLarge enlarges the dialog and its buttons, and shows the
// menubar.

void 
CMainDialog::ConvertMainDialogToLarge()
{
	PGPInt32	edgeX		= GetSystemMetrics(SM_CXFIXEDFRAME);
	PGPInt32	edgeY		= GetSystemMetrics(SM_CYFIXEDFRAME);
	PGPInt32	captionY	= GetSystemMetrics(SM_CYCAPTION);
	PGPInt32	menuY		= GetSystemMetrics(SM_CYMENU);
	PGPInt32	largeSizeX, largeSizeY;

	mAreWeInLargeMode = TRUE;

	// Show the main menu.
	ShowMainMenu();

	// Resize the dialog.
	largeSizeX	= kWidthLargeMainDialog + edgeX * 2;
	largeSizeY	= kHeightLargeMainDialog + edgeY * 2 + captionY + menuY;

	SetWindowPos(NULL, 0, 0, largeSizeX, largeSizeY, 
		SWP_NOMOVE | SWP_NOZORDER);

	// Move and resize the buttons.
	mNewButton.MoveWindow(kBigButtonXBias, kBigButtonYBias, 
		kSideLengthBigButton, kSideLengthBigButton);
	mMountButton.MoveWindow(kSideLengthBigButton + kBigButtonXBias, 
		kBigButtonYBias, kSideLengthBigButton, kSideLengthBigButton);
	mUnmountButton.MoveWindow(kSideLengthBigButton*2 + kBigButtonXBias, 
		kBigButtonYBias, kSideLengthBigButton, kSideLengthBigButton);
	mPrefsButton.MoveWindow(kSideLengthBigButton*3 + kBigButtonXBias, 
		kBigButtonYBias, kSideLengthBigButton, kSideLengthBigButton);

	// Load the big icons.
	mMountButton.SetIcon(mMountBigIcon);
	mNewButton.SetIcon(mNewBigIcon);
	mPrefsButton.SetIcon(mPrefsBigIcon);
	mUnmountButton.SetIcon(mUnmountBigIcon);
}

// ConvertMainDialogToSmall shrinks the dialog and its buttons, and hides the
// the menubar.

void 
CMainDialog::ConvertMainDialogToSmall()
{
	PGPInt32	edgeX		= GetSystemMetrics(SM_CXFIXEDFRAME);
	PGPInt32	edgeY		= GetSystemMetrics(SM_CYFIXEDFRAME);
	PGPInt32	captionY	= GetSystemMetrics(SM_CYCAPTION);
	PGPInt32	smallSizeX, smallSizeY;

	mAreWeInLargeMode = FALSE;

	// Load the small icons.
	mMountButton.SetIcon(mMountSmallIcon);
	mNewButton.SetIcon(mNewSmallIcon);
	mPrefsButton.SetIcon(mPrefsSmallIcon);
	mUnmountButton.SetIcon(mUnmountSmallIcon);

	// Move and resize the buttons.
	mNewButton.MoveWindow(kSmallButtonXBias, kSmallButtonYBias, 
		kSideLengthSmallButton, kSideLengthSmallButton);
	mMountButton.MoveWindow(kSideLengthSmallButton + kSmallButtonXBias, 
		kSmallButtonYBias, kSideLengthSmallButton, kSideLengthSmallButton);
	mUnmountButton.MoveWindow(kSideLengthSmallButton*2 + kSmallButtonXBias, 
		kSmallButtonYBias, kSideLengthSmallButton, kSideLengthSmallButton);
	mPrefsButton.MoveWindow(kSideLengthSmallButton*3 + kSmallButtonXBias, 
		kSmallButtonYBias, kSideLengthSmallButton, kSideLengthSmallButton);

	// Hide the main menu.
	HideMainMenu();

	// Resize the dialog.
	smallSizeX	= kWidthSmallMainDialog + edgeX * 2;
	smallSizeY	= kHeightSmallMainDialog + edgeY * 2 + captionY;

	SetWindowPos(NULL, 0, 0, smallSizeX, smallSizeY, 
		SWP_NOMOVE | SWP_NOZORDER);
}

// RecallMainWndOnTop recalls the 'window on top' position of the main window
// from the registry and checks the menu item to reflect this.

void 
CMainDialog::RecallMainWndOnTop()
{
	CMenu		*pSysMenu;
	PGPUInt32	checkState;
	
	pSysMenu = GetSystemMenu(FALSE);
	pgpAssertAddrValid(pSysMenu, CMenu);

	checkState = PGPdiskGetPrefDWord(kRegistryMainStayOnTopKey, 
		MF_UNCHECKED);

	pSysMenu->CheckMenuItem(IDM_STAYONTOP, MF_BYCOMMAND | checkState);

	if (checkState == MF_CHECKED)
		SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

// SaveMainWndOnTop saves the 'window on top' position of the main window in
// the registry.

void 
CMainDialog::SaveMainWndOnTop()
{
	CMenu		*pSysMenu;
	PGPUInt32	checkState;

	pSysMenu = GetSystemMenu(FALSE);
	pgpAssertAddrValid(pSysMenu, CMenu);

	checkState = pSysMenu->GetMenuState(IDM_STAYONTOP, MF_BYCOMMAND);

	PGPdiskSetPrefDWord(kRegistryMainStayOnTopKey, checkState);
}

// The CMainDialog data exchange function.

void 
CMainDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CMainDialog)
		DDX_Control(pDX, IDC_UNMOUNTPGPDISK, mUnmountButton);
		DDX_Control(pDX, IDC_MOUNTPGPDISK, mMountButton);
		DDX_Control(pDX, IDC_PREFERENCES, mPrefsButton);
		DDX_Control(pDX, IDC_NEWPGPDISK, mNewButton);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnAboutBox displays the about box.

void 
CMainDialog::OnAboutBox()
{
	App->CallShowAboutBox();
}

// OnAddPassphrase adds a passphrase to a PGPdisk.

void 
CMainDialog::OnAddPassphrase()
{
	App->AskUserAddPassphrase();
}

// OnChangePassphrase changes a passphrase on a PGPdisk.

void 
CMainDialog::OnChangePassphrase()
{
	App->AskUserChangePassphrase();
}

// OnExit is called when the user chooses Exit.

void 
CMainDialog::OnExit() 
{
	PostMessage(WM_CLOSE);
}

// OnHelpContents displays the PGPdisk help file.

void 
CMainDialog::OnHelpContents()
{
	App->CallShowHelp();
}

#if PGPDISK_PUBLIC_KEY

// OnAddRemovePublicKeys is called when the user selects the "Manage Public
// Keys" menu option.

void 
CMainDialog::OnAddRemovePublicKeys()
{
	App->AskUserAddRemovePublicKeys();
}

#endif	// PGPDISK_PUBLIC_KEY

// OnGlobalConvert is called when the user selects the "Convert Old PGPdisks"
// menu item.

void 
CMainDialog::OnGlobalConvert()
{
	App->AskUserGlobalConvertPGPdisks();
}

// OnMountPGPdisk is called when the user presses the "Mount PGPdisk" button.

void 
CMainDialog::OnMountPGPdisk()
{
	App->AskUserMountPGPdisk();
}

// OnNewPGPdisk is called when the user presses the New PGPdisk button. We
// display the "Create New PGPdisk" dialog box.

void 
CMainDialog::OnNewPGPdisk()
{
	App->AskUserNewPGPdisk();
}

// OnPreferences handles the 'Preferences' button.

void 
CMainDialog::OnPreferences() 
{
	App->CallShowPrefs();
}

// OnRemoveAlternates removes all alternate passphrases from a PGPdisk.

void 
CMainDialog::OnRemoveAlternates()
{
	App->AskUserRemovePassphrase(TRUE);
}

// OnRemovePassphrase removes a passphrase from a PGPdisk.

void 
CMainDialog::OnRemovePassphrase()
{
	App->AskUserRemovePassphrase();
}

// OnUnmountPGPdisk is called when the user selects the "Unmount PGPdisk" menu
// item.

void 
CMainDialog::OnUnmountPGPdisk()
{
	App->AskUserUnmountPGPdisk();
}


/////////////////////////////////////////////////
// CMainDialog protected default message handlers
/////////////////////////////////////////////////

// OnClose is called before the window is destroyed.

void 
CMainDialog::OnClose() 
{
	// Save the big/small window pref in the registry.
	PGPdiskSetPrefDWord(kRegistryWasDialogSmallKey, !mAreWeInLargeMode);

	// Save the window coords in the registry.
	SaveWindowPos(this, kRegistryMainCoordsKey);

	DestroyWindow();
}

// OnCommand is overridden so we can make sure that spurious commands aren't
// executed in command-line mode.

BOOL 
CMainDialog::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (App->mCommandLineMode)			// ignore if in command line mode
		return TRUE;
	
	return CDialog::OnCommand(wParam, lParam);
}

// OnDestroy is called when the dialog is going away.

void 
CMainDialog::OnDestroy()
{
	CDialog::OnDestroy();
}

// OnDropFiles is called to handle files dropped on the dialog which the user
// dragged there from the desktop.

void 
CMainDialog::OnDropFiles(HDROP hDrop)
{
	PGPUInt32 numDropped;

	numDropped = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, NULL);

	for (PGPUInt32 i=0; i < numDropped; i++)
	{
		AppCommandInfo	ACI;
		CString			path;
		DualErr			derr;

		try
		{
			DragQueryFile(hDrop, i, path.GetBuffer(kMaxStringSize), 
				kMaxStringSize);

			path.ReleaseBuffer();

			ACI.op		= kAppOp_Mount;
			ACI.flags	= NULL;
			ACI.drive	= kInvalidDrive;
			strcpy(ACI.path, path);

			App->DispatchAppCommandInfo(&ACI);
		}
		catch (CMemoryException *ex)
		{
			ex->Delete();
		}
	}
}

// OnHelpInfo handles context-sensitive help.

BOOL 
CMainDialog::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnInitDialog is called to perform dialog initialization for CMainDialog.

BOOL 
CMainDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Show the main menu.
	mInitErr = ShowMainMenu();

	if (mInitErr.IsntError())
	{
		// Recall where the user last put this window.
		RecallWindowPos(this, kRegistryMainCoordsKey);

		// Load our accelerator table.
		mAccelTable = LoadAccelerators(AfxGetResourceHandle(), 
			MAKEINTRESOURCE(IDR_MAINDLG_ACCEL));

		// Set the icon for this dialog.
		SetIcon(mHIcon, TRUE);			// Set big icon
		SetIcon(mHIcon, FALSE);			// Set small icon

		// Load the big icons for the four main buttons.
		mMountBigIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDI_MOUNT), IMAGE_ICON, kSideLengthBigIcon, 
			kSideLengthBigIcon, NULL);
		mNewBigIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDI_NEW), IMAGE_ICON, kSideLengthBigIcon, 
			kSideLengthBigIcon, NULL);
		mPrefsBigIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDI_PREFS), IMAGE_ICON, kSideLengthBigIcon, 
			kSideLengthBigIcon, NULL);
		mUnmountBigIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDI_UNMOUNT), IMAGE_ICON, kSideLengthBigIcon, 
			kSideLengthBigIcon, NULL);

		// Load the small icons for the four main buttons.
		mMountSmallIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDI_MOUNT), IMAGE_ICON, kSideLengthSmallIcon, 
			kSideLengthSmallIcon, NULL);
		mNewSmallIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDI_NEW), IMAGE_ICON, kSideLengthSmallIcon, 
			kSideLengthSmallIcon, NULL);
		mPrefsSmallIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDI_PREFS), IMAGE_ICON, kSideLengthSmallIcon, 
			kSideLengthSmallIcon, NULL);
		mUnmountSmallIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDI_UNMOUNT), IMAGE_ICON, kSideLengthSmallIcon, 
			kSideLengthSmallIcon, NULL);

		if (!mMountBigIcon || !mNewBigIcon || !mPrefsBigIcon || 
			!mUnmountBigIcon || !mMountSmallIcon || !mNewSmallIcon ||
			!mPrefsSmallIcon || !mUnmountSmallIcon)
		{
			mInitErr = kPGDMinorError_DialogDisplayFailed;
		}
	}

	if (mInitErr.IsntError())
	{
		CMenu *pSysMenu;

		// Were we large or small?
		if (PGPdiskGetPrefDWord(kRegistryWasDialogSmallKey, FALSE))
			ConvertMainDialogToSmall();
		else
			ConvertMainDialogToLarge();

		// Alter the system menu.
		pSysMenu = GetSystemMenu(FALSE);
		pgpAssertAddrValid(pSysMenu, CMenu);

		pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);

		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_STAYONTOP, "Stay On &Top");

		DrawMenuBar();

		// Accept dragged files.
		DragAcceptFiles();

		// Recall the window on top preference.
		RecallMainWndOnTop();

		// Focus on the new PGPdisk button.
		mNewButton.SetFocus();
	}

	mArePastInitDialog = TRUE;

	return FALSE;					
}
	
// OnInitMenu is called when the File menu is displayed.

void 
CMainDialog::OnInitMenu(CMenu *pMenu)
{
	PGPBoolean haveWeTimedOut = FALSE;

	CDialog::OnInitMenu(pMenu);

	// If the shift key is help down, show "Remove Alternate Passphrases" and
	// "Convert Old PGPdisks".

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		pMenu->ModifyMenu(IDM_REMOVEPASS, MF_BYCOMMAND | MF_STRING, 
			IDM_REMOVEALL, "Remove Alternate Passphrases...");
	}
	else
	{
		pMenu->ModifyMenu(IDM_REMOVEALL, MF_BYCOMMAND | MF_STRING, 
			IDM_REMOVEPASS, "Remove Passphrase...");
	}

	// Disable certain menu commands if we've timed out.

#if PGPDISK_BETAVERSION

	haveWeTimedOut = App->HasBetaTimedOut();

#elif PGPDISK_DEMOVERSION

	haveWeTimedOut = App->HasDemoTimedOut();

#endif // PGPDISK_BETAVERSION

	if (haveWeTimedOut)
	{
		pMenu->EnableMenuItem(IDM_NEWPGPDISK, MF_GRAYED);
		pMenu->EnableMenuItem(IDM_ADDPASS, MF_GRAYED);
		pMenu->EnableMenuItem(IDM_CHANGEPASS, MF_GRAYED);
		pMenu->EnableMenuItem(IDM_REMOVEPASS, MF_GRAYED);
		pMenu->EnableMenuItem(IDM_REMOVEALL, MF_GRAYED);
	}
}

// OnPaint is called when part of the dialog must be repainted.

void 
CMainDialog::OnPaint() 
{
	CRect rect;

	if (IsIconic())			// MFC iconic stuff
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int	cxIcon	= GetSystemMetrics(SM_CXICON);
		int	cyIcon	= GetSystemMetrics(SM_CYICON);

		GetClientRect(&rect);
			
		int	x	= (rect.Width() - cxIcon + 1)/2;
		int	y	= (rect.Height() - cyIcon + 1)/2;

		// Draw the icon
		dc.DrawIcon(x, y, mHIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// OnSize is called when the dialog has been resized. We alter the buttons and
// the menu as needed.

void 
CMainDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (mArePastInitDialog)
	{
		if (mAreWeInLargeMode)
			ConvertMainDialogToLarge();
		else
			ConvertMainDialogToSmall();
	}
}

// OnSizing is called when the user is resizing the dialog.

void 
CMainDialog::OnSizing(UINT fwSide, LPRECT pRect) 
{
	if (mArePastInitDialog)
	{
		PGPInt32	edgeX		= GetSystemMetrics(SM_CXFIXEDFRAME);
		PGPInt32	edgeY		= GetSystemMetrics(SM_CYFIXEDFRAME);
		PGPInt32	captionY	= GetSystemMetrics(SM_CYCAPTION);
		PGPInt32	menuY		= GetSystemMetrics(SM_CYMENU);
		PGPInt32	smallSizeX, smallSizeY, largeSizeX, largeSizeY;

		largeSizeX	= kWidthLargeMainDialog + edgeX * 2;
		largeSizeY	= kHeightLargeMainDialog + edgeY * 2 + captionY + menuY;

		smallSizeX	= kWidthSmallMainDialog + edgeX * 2;
		smallSizeY	= kHeightSmallMainDialog + edgeY * 2 + captionY;

		// There are only two allowable positions for the drag rectangle.
		// Enforce this.

		switch (fwSide)
		{

		case WMSZ_BOTTOM:
			if ((pRect->bottom - pRect->top) < largeSizeY - 5)
			{
				pRect->bottom	= pRect->top + smallSizeY;
				pRect->right	= pRect->left + smallSizeX;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->bottom - pRect->top) > smallSizeY + 5)
			{
				pRect->bottom	= pRect->top + largeSizeY;
				pRect->right	= pRect->left + largeSizeX;

				mAreWeInLargeMode = TRUE;
			}
			break;

		case WMSZ_BOTTOMLEFT:
			if ((pRect->bottom - pRect->top) < largeSizeY - 5)
			{
				pRect->bottom	= pRect->top + smallSizeY;
				pRect->left		= pRect->right - smallSizeX;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->bottom - pRect->top) > smallSizeY + 5)
			{
				pRect->bottom	= pRect->top + largeSizeY;
				pRect->left		= pRect->right - largeSizeX;

				mAreWeInLargeMode = TRUE;
			}

			if ((pRect->right - pRect->left) < largeSizeX - 5)
			{
				pRect->left		= pRect->right - smallSizeX;
				pRect->bottom	= pRect->top + smallSizeY;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->right - pRect->left) > smallSizeX + 5)
			{
				pRect->left		= pRect->right - largeSizeX;
				pRect->bottom	= pRect->top + largeSizeY;

				mAreWeInLargeMode = TRUE;
			}
			break;

		case WMSZ_BOTTOMRIGHT:
			if ((pRect->bottom - pRect->top) < largeSizeY - 5)
			{
				pRect->bottom	= pRect->top + smallSizeY;
				pRect->right	= pRect->left + smallSizeX;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->bottom - pRect->top) > smallSizeY + 5)
			{
				pRect->bottom	= pRect->top + largeSizeY;
				pRect->right	= pRect->left + largeSizeX;

				mAreWeInLargeMode = TRUE;
			}

			if ((pRect->right - pRect->left) < largeSizeX - 5)
			{
				pRect->right	= pRect->left + smallSizeX;
				pRect->bottom	= pRect->top + smallSizeY;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->right - pRect->left) > smallSizeX + 5)
			{
				pRect->right	= pRect->left + largeSizeX;
				pRect->bottom	= pRect->top + largeSizeY;

				mAreWeInLargeMode = TRUE;
			}
			break;

		case WMSZ_LEFT:
			if ((pRect->right - pRect->left) < largeSizeX - 5)
			{
				pRect->left		= pRect->right - smallSizeX;
				pRect->bottom	= pRect->top + smallSizeY;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->right - pRect->left) > smallSizeX + 5)
			{
				pRect->left		= pRect->right - largeSizeX;
				pRect->bottom	= pRect->top + largeSizeY;

				mAreWeInLargeMode = TRUE;
			}
			break;

		case WMSZ_RIGHT:
			if ((pRect->right - pRect->left) < largeSizeX - 5)
			{
				pRect->right	= pRect->left + smallSizeX;
				pRect->bottom	= pRect->top + smallSizeY;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->right - pRect->left) > smallSizeX + 5)
			{
				pRect->right	= pRect->left + largeSizeX;
				pRect->bottom	= pRect->top + largeSizeY;

				mAreWeInLargeMode = TRUE;
			}
			break;

		case WMSZ_TOP:
			if ((pRect->bottom - pRect->top) < largeSizeY - 5)
			{
				pRect->top		= pRect->bottom - smallSizeY;
				pRect->right	= pRect->left + smallSizeX;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->bottom - pRect->top) > smallSizeY + 5)
			{
				pRect->top		= pRect->bottom - largeSizeY;
				pRect->right	= pRect->left + largeSizeX;

				mAreWeInLargeMode = TRUE;
			}
			break;

		case WMSZ_TOPLEFT:
			if ((pRect->bottom - pRect->top) < largeSizeY - 5)
			{
				pRect->top	= pRect->bottom - smallSizeY;
				pRect->left	= pRect->right - smallSizeX;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->bottom - pRect->top) > smallSizeY + 5)
			{
				pRect->top	= pRect->bottom - largeSizeY;
				pRect->left	= pRect->right - largeSizeX;

				mAreWeInLargeMode = TRUE;
			}

			if ((pRect->right - pRect->left) < largeSizeX - 5)
			{
				pRect->left		= pRect->right - smallSizeX;
				pRect->bottom	= pRect->top + smallSizeY;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->right - pRect->left) > smallSizeX + 5)
			{
				pRect->left		= pRect->right - largeSizeX;
				pRect->bottom	= pRect->top + largeSizeY;

				mAreWeInLargeMode = TRUE;
			}
			break;

		case WMSZ_TOPRIGHT:
			if ((pRect->bottom - pRect->top) < largeSizeY - 5)
			{
				pRect->top		= pRect->bottom - smallSizeY;
				pRect->right	= pRect->left + smallSizeX;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->bottom - pRect->top) > smallSizeY + 5)
			{
				pRect->top		= pRect->bottom - largeSizeY;
				pRect->right	= pRect->left + largeSizeX;

				mAreWeInLargeMode = TRUE;
			}

			if ((pRect->right - pRect->left) < largeSizeX - 5)
			{
				pRect->right	= pRect->left + smallSizeX;
				pRect->bottom	= pRect->top + smallSizeY;

				mAreWeInLargeMode = FALSE;
			}

			if ((pRect->right - pRect->left) > smallSizeX + 5)
			{
				pRect->right	= pRect->left + largeSizeX;
				pRect->bottom	= pRect->top + largeSizeY;

				mAreWeInLargeMode = TRUE;
			}
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
	}

	CDialog::OnSizing(fwSide, pRect);	
}

// OnSysCommand is called when the user makes a choice from the system menu
// and also in other circumstances.

void 
CMainDialog::OnSysCommand(UINT nId, LONG lParam)
{
	CMenu		*pSysMenu;
	PGPUInt32	checkState;

	// Did the user select the "Stay On Top" menu item?
	switch (nId)
	{
	case IDM_STAYONTOP:
		pSysMenu = GetSystemMenu(FALSE);
		pgpAssertAddrValid(pSysMenu, CMenu);

		checkState = pSysMenu->GetMenuState(IDM_STAYONTOP, MF_BYCOMMAND);

		if (checkState == MF_CHECKED)
		{
			pSysMenu->CheckMenuItem(IDM_STAYONTOP, 
				MF_BYCOMMAND | MF_UNCHECKED);

			SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, 
				SWP_NOMOVE | SWP_NOSIZE);
		}
		else
		{
			pSysMenu->CheckMenuItem(IDM_STAYONTOP, MF_BYCOMMAND | MF_CHECKED);

			SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, 
				SWP_NOMOVE | SWP_NOSIZE);
		}

		// Save the window on top position in the registry.
		SaveMainWndOnTop();
		break;

	default:
		CWnd::OnSysCommand(nId, lParam);
		break;
	}
}

// OnQueryDragIcon is called to obtain the cursor to display when dragging a
// minimized window.

HCURSOR 
CMainDialog::OnQueryDragIcon()
{
	return (HCURSOR) mHIcon;
}

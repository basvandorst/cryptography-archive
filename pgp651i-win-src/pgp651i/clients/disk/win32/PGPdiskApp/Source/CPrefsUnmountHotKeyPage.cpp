//////////////////////////////////////////////////////////////////////////////
// CPrefsUnmountHotKeyPage.cpp
//
// Implementation of class CPrefsUnmountHotKeyPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPrefsUnmountHotKeyPage.cpp,v 1.5 1998/12/14 18:58:10 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "UtilityFunctions.h"

#include "CPGPdiskAppRegistry.h"
#include "CPrefsUnmountHotKeyPage.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_HOTKEY_CHECK,		IDH_PGPDISKAPP_PREFSHKHOTKEYCHECK, 
	IDC_HOTKEY_EDIT,		IDH_PGPDISKAPP_PREFSHKHOTKEYEDIT, 
	0,0 
};


///////////////////////////
// MFC specific definitions
/////////////////////////// 

BEGIN_MESSAGE_MAP(CPrefsUnmountHotKeyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsUnmountHotKeyPage)
	ON_BN_CLICKED(IDC_HOTKEY_CHECK, OnHotKeyCheck)
	ON_EN_CHANGE(IDC_HOTKEY_EDIT, OnHotKeyChange)
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// CPrefsUnmountHotKeyPage public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CPrefsUnmountHotKeyPage, CPropertyPage)

// The CPrefsUnmountHotKeyPage default constructor.

CPrefsUnmountHotKeyPage::CPrefsUnmountHotKeyPage()
	 : CPropertyPage(CPrefsUnmountHotKeyPage::IDD)
{
	//{{AFX_DATA_INIT(CPrefsUnmountHotKeyPage)
	mHotKeyEnabledValue = FALSE;
	//}}AFX_DATA_INIT

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CPrefsUnmountHotKeyPage destructor.

CPrefsUnmountHotKeyPage::~CPrefsUnmountHotKeyPage()
{
}


//////////////////////////////////////////////////////////////////////////////
// CPrefsUnmountHotKeyPage protected custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////////

// The CPrefsUnmountHotKeyPage data exchange function.

void 
CPrefsUnmountHotKeyPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CPrefsUnmountHotKeyPage)
		DDX_Control(pDX, IDC_HOTKEY_CHECK, mHotKeyButton);
		DDX_Control(pDX, IDC_HOTKEY_EDIT, mHotKeyBox);
		DDX_Check(pDX, IDC_HOTKEY_CHECK, mHotKeyEnabledValue);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnHotKeyChange is called when the user changes the hot key.

void 
CPrefsUnmountHotKeyPage::OnHotKeyChange() 
{
	SetModified(TRUE);
}

// OnHotKeyCheck is called when the user hits the hot key enabled check box.
// We disable or enable the hot key control as necessary.

void 
CPrefsUnmountHotKeyPage::OnHotKeyCheck() 
{
	SetModified(TRUE);	
	UpdateData(TRUE);

	if (mHotKeyEnabledValue)
		mHotKeyBox.EnableWindow(TRUE);
	else
		mHotKeyBox.EnableWindow(FALSE);
}


/////////////////////////////////////////////////////////////
// CPrefsUnmountHotKeyPage protected default message handlers
/////////////////////////////////////////////////////////////

// OnApply is overridden to apply the changes the user made to the
// application, but we don't save them to the registry.

BOOL 
CPrefsUnmountHotKeyPage::OnApply() 
{
	UpdateData(TRUE);

	App->mHotKeyCode	= mHotKeyBox.GetHotKey();
	App->mHotKeyEnabled	= mHotKeyEnabledValue;

	SetModified(FALSE);

	// Tell the driver.
	App->SetDriverPrefsFromApp();

	return CPropertyPage::OnApply();
}

// OnHelpInfo handles context-sensitive help.

BOOL 
CPrefsUnmountHotKeyPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CPrefsUnmountHotKeyPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// Initialize the check box and hot key control.
	mHotKeyBox.SetHotKey(App->mHotKeyCode);
	mHotKeyEnabledValue = App->mHotKeyEnabled;

	if (!mHotKeyEnabledValue)
		mHotKeyBox.EnableWindow(FALSE);

	// Disable hooking options on NT5.
	if (IsWinNT5CompatibleMachine())
	{
		mHotKeyEnabledValue = FALSE;
		mHotKeyButton.EnableWindow(FALSE);
		mHotKeyBox.EnableWindow(FALSE);
	}

	UpdateData(FALSE);

	return TRUE;
}

// OnOK is overridden to save the new value to the registry.

void 
CPrefsUnmountHotKeyPage::OnOK() 
{
	DualErr derr;

	UpdateData(TRUE);

	App->mHotKeyCode	= mHotKeyBox.GetHotKey();
	App->mHotKeyEnabled	= mHotKeyEnabledValue;

	derr = App->SetRegistryPrefs();

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_PreferencesAccessFailed, derr);
	}
	else
	{
		App->UpdatePGPdiskResident();
	}

	CPropertyPage::OnOK();
}

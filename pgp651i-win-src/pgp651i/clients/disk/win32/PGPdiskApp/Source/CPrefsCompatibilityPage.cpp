//////////////////////////////////////////////////////////////////////////////
// CPrefsCompatibilityPage.cpp
//
// Implementation of class CPrefsCompatibilityPage
//////////////////////////////////////////////////////////////////////////////

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "PGPdiskPfl.h"
#include "UtilityFunctions.h"

#include "CPGPdiskAppRegistry.h"
#include "CPrefsCompatibilityPage.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_CHECK_FAT32,		IDH_PGPDISKAPP_PREFSCOMPATFAT32CHECK, 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CPrefsCompatibilityPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsCompatibilityPage)
	ON_BN_CLICKED(IDC_CHECK_FAT32, OnCheckFat32)
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// CPrefsCompatibilityPage public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CPrefsCompatibilityPage, CPropertyPage)

// The CPrefsCompatibilityPage default constructor.

CPrefsCompatibilityPage::CPrefsCompatibilityPage()
	 : CPropertyPage(CPrefsCompatibilityPage::IDD)
{
	//{{AFX_DATA_INIT(CPrefsCompatibilityPage)
	mAllowFat32Value = FALSE;
	//}}AFX_DATA_INIT

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CPrefsCompatibilityPage destructor.

CPrefsCompatibilityPage::~CPrefsCompatibilityPage()
{
}


//////////////////////////////////////////////////////////////////////////////
// CPrefsCompatibilityPage protected custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////////

// The CPrefsCompatibilityPage data exchange function.

void 
CPrefsCompatibilityPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CPropertyPage::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CPrefsCompatibilityPage)
		DDX_Control(pDX, IDC_CHECK_FAT32, mCheckAllowFat32);
		DDX_Check(pDX, IDC_CHECK_FAT32, mAllowFat32Value);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnCheckFat32 is handled so we can enable the 'Apply' button.

void 
CPrefsCompatibilityPage::OnCheckFat32() 
{
	SetModified(TRUE);	
}


/////////////////////////////////////////////////////////////
// CPrefsCompatibilityPage protected default message handlers
/////////////////////////////////////////////////////////////

// OnApply is overridden to apply the changes the user made to the
// application, but we don't save them to the registry.

BOOL 
CPrefsCompatibilityPage::OnApply() 
{
	UpdateData(TRUE);

	App->mAllowFat32 = mAllowFat32Value;
	SetModified(FALSE);

	return CPropertyPage::OnApply();
}

// OnHelpInfo handles context-sensitive help.

BOOL 
CPrefsCompatibilityPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CPrefsCompatibilityPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// First initialize the check box.
	mAllowFat32Value = App->mAllowFat32;
	UpdateData(FALSE);

	// Disable the FAT32 button if we aren't using OSR2.
	if (!IsWin95OSR2CompatibleMachine(App->mWindowsVersion))
	{
		pgpAssert(!App->mAllowFat32);
		mCheckAllowFat32.EnableWindow(FALSE);
	}

	return TRUE;
}

// OnOK is overridden to save the new value to the registry.

void 
CPrefsCompatibilityPage::OnOK() 
{
	DualErr derr;

	UpdateData(TRUE);
	App->mAllowFat32 = mAllowFat32Value;

	derr = App->SetRegistryPrefs();

	if (derr.IsError())
	{
		App->ReportError(kPGDMajorError_PreferencesAccessFailed, derr);
	}

	CPropertyPage::OnOK();
}

//////////////////////////////////////////////////////////////////////////////
// CPreferencesSheet.cpp
//
// Implementation of class CPreferencesSheet.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPreferencesSheet.cpp,v 1.1.2.9 1998/07/06 08:57:13 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "StringAssociation.h"

#include "CMainDialog.h"
#include "CPreferencesSheet.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDOK,			IDH_PGPDISKAPP_PREFSHEETOK, 
	IDCANCEL,		IDH_PGPDISKAPP_PREFSHEETCANCEL, 
	ID_APPLY_NOW,	IDH_PGPDISKAPP_PREFSHEETAPPLY, 
    0,0 
};


/////////////////////////
// Class global variables
/////////////////////////

// Keep track of the page the user was last looking at.
PGPUInt8 CPreferencesSheet::mLastPageViewed = 0;


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CPreferencesSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CPreferencesSheet)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////
// CPreferencesSheet public custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CPreferencesSheet, CPropertySheet)

// The CPreferencesSheet constructor adds the two property pages we implement
// to the property sheet.

CPreferencesSheet::CPreferencesSheet()
	: CPropertySheet(GetCommonString(kPGPdiskPrefsSheetTitleString), NULL, 0)
{
	try
	{
		AddPage(&mAutoUnmountPage);
		AddPage(&mHotKeyPage);

		// Exorcise the big help button.
		m_psh.dwFlags &= ~PSH_HASHELP;
	}
	catch (CMemoryException *ex)
	{
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}
}

// The CPreferencesSheet destructor.

CPreferencesSheet::~CPreferencesSheet()
{
}

// DestroyWindow is called when the dialog is about to go away.

BOOL 
CPreferencesSheet::DestroyWindow() 
{
	// Save the index of the last page viewed.
	mLastPageViewed = GetActiveIndex();
	return CPropertySheet::DestroyWindow();
}

// DisplayPreferences is a wrapper around DoModal.

DualErr 
CPreferencesSheet::DisplayPreferences()
{
	DualErr		derr;
	PGPInt32	result;
	
	m_psh.nStartPage = mLastPageViewed;
	result = DoModal();

	switch (result)
	{
	case -1:
		derr = DualErr(kPGDMinorError_DialogDisplayFailed);
		break;
	}

	return derr;
}


///////////////////////////////////////////////
// CPreferencesSheet protected default handlers
///////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CPreferencesSheet::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnInitDialog is called during dialog initialization. We take the
// opportunity to add a context help button to the title bar.

BOOL 
CPreferencesSheet::OnInitDialog() 
{
	CPropertySheet::OnInitDialog();

	// Show the context help button.
	ModifyStyleEx(NULL, WS_EX_CONTEXTHELP, NULL);

	// Center on screen.
	CenterWindow(GetDesktopWindow());

	// Put us in front.
	SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(this);

	return FALSE;
}

// OnSysCommand is overridden so we can handle close operations. Confirm in
// certain cases.

void 
CPreferencesSheet::OnSysCommand(UINT nID, LPARAM lParam) 
{
	PGPBoolean skipCommand = FALSE;

	// If apply button is enabled, there are pending changes.
	if (nID == SC_CLOSE)
	{
		if (GetDlgItem(ID_APPLY_NOW)->IsWindowEnabled())
		{
			switch (DisplayMessage(kPGPdiskAskToSavePrefs, 
				kPMBS_YesNoCancel, kPMBF_YesButton))
			{
			case kUR_Yes:
				PressButton(PSBTN_APPLYNOW);
				break;

			case kUR_No:
				break;

			case kUR_Cancel:
				skipCommand = TRUE;
				break;
			}
		}
	}

	if (!skipCommand)
		CPropertySheet::OnSysCommand(nID, lParam);
}

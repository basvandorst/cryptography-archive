//////////////////////////////////////////////////////////////////////////////
// CGetUserInfoDialog.cpp
//
// Implementation of the CGetUserInfoDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CGetUserInfoDialog.cpp,v 1.1.2.6 1998/07/22 23:37:55 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CGetUserInfoDialog.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDOK,				IDH_PGPDISKAPP_GETUSERINFOOK, 
	IDCANCEL,			IDH_PGPDISKAPP_GETUSERINFOCANCEL, 
	IDC_USER_EDIT,		IDH_PGPDISKAPP_GETUSERUSEREDIT, 
	IDC_ORG_EDIT,		IDH_PGPDISKAPP_GETUSERORGEDIT, 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CGetUserInfoDialog, CDialog)
	//{{AFX_MSG_MAP(CGetUserInfoDialog)
	ON_WM_HELPINFO()
	ON_EN_CHANGE(IDC_USER_EDIT, OnChangeNameEdit)
	ON_EN_CHANGE(IDC_ORG_EDIT, OnChangeOrgEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CGetUserInfoDialog public custom functions and non-default message handlers
//////////////////////////////////////////////////////////////////////////////

// The CGetUserInfoDialog default constructor.

CGetUserInfoDialog::CGetUserInfoDialog(CWnd *pParent) 
	: CDialog(CGetUserInfoDialog::IDD, pParent)
{
	try
	{
		//{{AFX_DATA_INIT(CGetUserInfoDialog)
		mUserText = _T("");
		mOrgText = _T("");
		//}}AFX_DATA_INIT
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// AskForUserInfo is a wrapper around DoModal.

DualErr 
CGetUserInfoDialog::AskForUserInfo()
{
	DualErr		derr;
	PGPInt32	result;
	
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

	return derr;
}


/////////////////////////////////////////////////////////////////////////
// CGetUserInfoDialog protected custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////

// UpdateOkButton enables the OK button if at least one of the edit boxes has
// text it in, otherwise the OK button is disabled.

void 
CGetUserInfoDialog::UpdateOkButton()
{
	UpdateData(TRUE);

	if (mUserText.IsEmpty() && mOrgText.IsEmpty())
		mOkButton.EnableWindow(FALSE);
	else
		mOkButton.EnableWindow(TRUE);
}

// The CGetUserInfoDialog default data-exchange function.

void 
CGetUserInfoDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CGetUserInfoDialog)
		DDX_Control(pDX, IDC_USER_EDIT, mUserStatic);
		DDX_Control(pDX, IDC_ORG_EDIT, mOrgStatic);
		DDX_Control(pDX, IDCANCEL, mCancelButton);
		DDX_Control(pDX, IDOK, mOkButton);
		DDX_Text(pDX, IDC_USER_EDIT, mUserText);
		DDV_MaxChars(pDX, mUserText, 64);
		DDX_Text(pDX, IDC_ORG_EDIT, mOrgText);
		DDV_MaxChars(pDX, mOrgText, 64);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnChangeNameEdit is called when the user types in the user name edit box.
// We enable or disable the OK button as necessary.

void 
CGetUserInfoDialog::OnChangeNameEdit() 
{
	UpdateOkButton();
}

// OnChangeOrgEdit is called when the user types in the organization edit box.
// We enable or disable the OK button as necessary.

void 
CGetUserInfoDialog::OnChangeOrgEdit() 
{
	UpdateOkButton();
}


////////////////////////////////////////////////////////
// CGetUserInfoDialog protected default message handlers
////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CGetUserInfoDialog::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnInitDialog is called when the dialog box is created.

BOOL 
CGetUserInfoDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// This will disable the OK button since no text is present yet.
	UpdateOkButton();

	// Center on screen.
	CenterWindow(GetDesktopWindow());

	// Put us in front.
	SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(this);

	// Set focus to the user edit box.
	mUserStatic.SetFocus();

	// Show now to thwart evil MFC auto-centering.
	ShowWindow(SW_SHOW);

	return FALSE;
}

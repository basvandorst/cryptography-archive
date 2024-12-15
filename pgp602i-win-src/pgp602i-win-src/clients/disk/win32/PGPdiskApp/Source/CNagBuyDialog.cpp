//////////////////////////////////////////////////////////////////////////////
// CNagBuyDialog.cpp
//
// Implementation of the CNagBuyDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CNagBuyDialog.cpp,v 1.1.2.7 1998/07/06 23:26:40 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "StringAssociation.h"

#include "CMainDialog.h"
#include "CNagBuyDialog.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDOK,				IDH_PGPDISKAPP_NAGBUYOK, 
	IDCANCEL,			IDH_PGPDISKAPP_NAGBUYCANCEL, 
	IDC_BUYNOW_BUTTON,	IDH_PGPDISKAPP_NAGBUYBUYNOWBUTTON, 
	IDC_NAGBUYMSG_TEXT,	((PGPUInt32) -1), 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CNagBuyDialog, CDialog)
	//{{AFX_MSG_MAP(CNagBuyDialog)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_BUYNOW_BUTTON, OnBuyNowButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////
// CNagBuyDialog public custom functions and non-default message handlers
/////////////////////////////////////////////////////////////////////////

// The CNagBuyDialog default constructor.

CNagBuyDialog::CNagBuyDialog(CWnd *pParent) 
	: CDialog(CNagBuyDialog::IDD, pParent)
{
	try
	{
		//{{AFX_DATA_INIT(CNagBuyDialog)
		mNagMessageText = _T("");
		//}}AFX_DATA_INIT
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// DisplayDialog is a wrapper around DoModal.

DualErr 
CNagBuyDialog::DisplayDialog(LPCSTR message)
{
	DualErr		derr;
	PGPInt32	result;
	
	// Set the nag message.
	try
	{
		mNagMessageText = message;
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsntError())
	{
		// Show the dialog.
		result = DoModal();

		// Return an appropriate value.
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


////////////////////////////////////////////////////////////////////
// CNagBuyDialog protected custom functions and non-default handlers
////////////////////////////////////////////////////////////////////

// The CNagBuyDialog default data-exchange function.

void 
CNagBuyDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CNagBuyDialog)
		DDX_Control(pDX, IDOK, mOkButton);
		DDX_Control(pDX, IDCANCEL, mCancelButton);
		DDX_Control(pDX, IDC_BUYNOW_BUTTON, mBuyNowButton);
		DDX_Text(pDX, IDC_NAGBUYMSG_TEXT, mNagMessageText);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


///////////////////////////////////////////////////
// CNagBuyDialog protected default message handlers
///////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CNagBuyDialog::OnHelpInfo(HELPINFO *pHelpInfo)
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
CNagBuyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Put us in front.
	SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(this);

	return FALSE;
}

// OnBuyNowButton goes to the "Buy PGPdisk Now" web site.

void 
CNagBuyDialog::OnBuyNowButton() 
{
	ShellExecute(App->mMainDialog->GetSafeHwnd(), "open", 
		GetCommonString(kPGPdiskBuyNowURL), NULL, "C:\\", SW_SHOWNORMAL);

	EndDialog(IDOK);
}

//////////////////////////////////////////////////////////////////////////////
// CUnmountPGPdiskDialog.cpp
//
// Implementation of the CUnmountPGPdiskDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CUnmountPGPdiskDialog.cpp,v 1.1.2.5 1998/07/29 00:45:10 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CUnmountPGPdiskDialog.h"
#include "Globals.h"
#include "PGPdisk.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDCANCEL,				IDH_PGPDISKAPP_UNMOUNTCANCEL, 
	IDC_UNMOUNT,			IDH_PGPDISKAPP_UNMOUNTUNMOUNT, 
	IDC_DRIVE_COMBO,		IDH_PGPDISKAPP_UNMOUNTDRIVE, 
	IDC_STATIC,				-1, 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CUnmountPGPdiskDialog, CDialog)
	//{{AFX_MSG_MAP(CUnmountPGPdiskDialog)
	ON_BN_CLICKED(IDC_UNMOUNT, OnUnmount)
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////
// CUnmountPGPdiskDialog public custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////

// The CUnmountPGPdiskDialog default constructor.

CUnmountPGPdiskDialog::CUnmountPGPdiskDialog(CWnd *pParent)
	: CDialog(CUnmountPGPdiskDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUnmountPGPdiskDialog)
	//}}AFX_DATA_INIT

	mDriveToUnmount = kInvalidDrive;
}

// The CUnmountPGPdiskDialog destructor.

CUnmountPGPdiskDialog::~CUnmountPGPdiskDialog()
{
}

// AskForDriveLetter is a wrapper around DoModal.

DualErr 
CUnmountPGPdiskDialog::AskForDriveLetter()
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


////////////////////////////////////////////////////////////////////////////
// CUnmountPGPdiskDialog protected custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////////

// The CUnmountPGPdiskDialog data exchange function.

void 
CUnmountPGPdiskDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CUnmountPGPdiskDialog)
		DDX_Control(pDX, IDC_DRIVE_COMBO, mDriveComboBox);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnUnmount is called when the user pressed the 'Unmount' button.

void 
CUnmountPGPdiskDialog::OnUnmount() 
{
	PGPInt16 index = mDriveComboBox.GetCurSel();

	// If the user made a selection, then...
	if (index != CB_ERR)
	{
		PGPUInt8 comboIndex, drive;

		comboIndex = 0;

		// ...for each drive letter...
		for (drive = 0; drive < kMaxDrives; drive++)
		{
			PGPdisk *pPGD;

			// ...if it's a mounted PGPdisk...
			pPGD = App->mPGPdisks.FindPGPdisk(drive);

			if (IsntNull(pPGD))
			{
				// ... and it corresponds to the user's selection...
				if (comboIndex == index)
				{
					// ... mark it for unmount.
					mDriveToUnmount = drive;
					break;
				}

				comboIndex++;
			}
		}
	}

	EndDialog(IDOK);
}


///////////////////////////////////////////////////
// CUnmountPGPdiskDialog protected default handlers
///////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CUnmountPGPdiskDialog::OnHelpInfo(HELPINFO *pHelpInfo)
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
CUnmountPGPdiskDialog::OnInitDialog() 
{
	PGPUInt8 drive = 0;

	CDialog::OnInitDialog();

	// For each mounted PGPdisk, insert an entry for it in the combo box.
	for (drive = 2; drive < kMaxDrives; drive++)
	{
		PGPdisk *pPGD;

		pPGD = App->mPGPdisks.FindPGPdisk(drive);

		if (IsntNull(pPGD))
		{
			char	label[kMaxStringSize];
			CString	driveChoice;

			pPGD->GetVolumeLabel(label, kMaxStringSize);

			// Convert all but first letter of volume label to lowercase.
			if (strlen(label) > 1)
				_strlwr(label + 1);

			driveChoice.Format("%s (%c:)", label, DriveNumToLet(drive));
			
			if (mDriveComboBox.AddString(driveChoice) == CB_ERR)
				EndDialog(IDCANCEL);
		}
	}

	if (App->mPGPdisks.GetNumPGPdisks() == 0)
		mDriveComboBox.EnableWindow(FALSE);
	else
		mDriveComboBox.SetCurSel(0);

	// Center on screen.
	CenterWindow(GetDesktopWindow());

	// Put us in front.
	SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(this);

	// Show now to thwart evil MFC auto-centering.
	ShowWindow(SW_SHOW);

	// Set focus to the combo box.
	mDriveComboBox.SetFocus();

	return FALSE;
}

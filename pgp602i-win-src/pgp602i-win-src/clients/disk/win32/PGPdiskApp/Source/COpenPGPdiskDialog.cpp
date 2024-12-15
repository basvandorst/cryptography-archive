//////////////////////////////////////////////////////////////////////////////
// COpenPGPdiskDialog.cpp
//
// Implementation of the COpenPGPdiskDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: COpenPGPdiskDialog.cpp,v 1.1.2.8 1998/07/06 23:26:42 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskRegistry.h"

#include "COpenPGPdiskDialog.h"
#include "Globals.h"
#include "PGPdisk.h"


////////////
// Constants
////////////

static LPCSTR	kODTMountDialogTitle	= "Mount PGPdisk";
static LPCSTR	kODTSelectDialogTitle	= "Select PGPdisk";

static LPCSTR kODTOpenFilter = 
	"PGPdisk Files (*.pgd)|*.pgd|All Files (*.*)|*.*||";


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(COpenPGPdiskDialog, CFileDialog)
	//{{AFX_MSG_MAP(COpenPGPdiskDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// COpenPGPdiskDialog public custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(COpenPGPdiskDialog, CFileDialog)

// The COpenPGPdiskDialog default constructor.

COpenPGPdiskDialog::COpenPGPdiskDialog(OpenDialogType dlgType, CWnd *pParent)

	: CFileDialog(TRUE, &kPGPdiskFileExtension[1], NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, kODTOpenFilter, pParent)
{
	mOpenDialogType	= dlgType;

	m_ofn.Flags	|= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	m_ofn.Flags	&= ~OFN_SHAREAWARE;

	// Initialize the title according to the dialog type.
	switch (mOpenDialogType)
	{
	case kODT_Mount:
		m_ofn.lpstrTitle = kODTMountDialogTitle;
		break;

	case kODT_Select:
		m_ofn.lpstrTitle = kODTSelectDialogTitle;
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}

// AskForPath is a wrapper around DoModal.

DualErr 
COpenPGPdiskDialog::AskForPath(CString *path)
{
	CString		initialDir;
	DualErr		derr;
	PGPInt32	result;
	
	pgpAssertAddrValid(path, CString);

	// Get default directory from registry.
	PGPdiskGetPrefString(kRegistryLastOpenDirKey, kEmptyString, &initialDir);

	// Is it legal?
	if (App->IsDirectoryValid(initialDir))
		m_ofn.lpstrInitialDir = initialDir;
	else
		m_ofn.lpstrInitialDir = NULL;

	// Show the dialog.
	result = DoModal();

	// Return an appropriate value.
	switch (result)
	{
	case IDOK:
		try
		{
			(* path) = GetPathName();
		}
		catch (CMemoryException *ex)
		{
			derr = DualErr(kPGDMinorError_OutOfMemory);
			ex->Delete();
		}
		break;

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
// COpenPGPdiskDialog protected custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////

// UpdateOpenDialogPrefs stores the location of the chosen directory.

void 
COpenPGPdiskDialog::UpdateOpenDialogPrefs()
{
	// Update the last open directory in the registry.
	try
	{
		CString dir, path;

		path = GetPathName();
		
		GetDirectory(path, &dir);
		PGPdiskSetPrefString(kRegistryLastOpenDirKey, dir);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// The COpenPGPdiskDialog default data-exchange function.

void 
COpenPGPdiskDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(COpenPGPdiskDialog)
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnFileNameOK is called when the user hits the OK button. We update registry
// information.

BOOL 
COpenPGPdiskDialog::OnFileNameOK()
{
	if (CFileDialog::OnFileNameOK() == 1)
		return 1;

	UpdateOpenDialogPrefs();
	return 0;
}

// OnInitDone is called when initialization is done. We override it to stop
// MFC from centering the window.

void 
COpenPGPdiskDialog::OnInitDone()
{
	// Center on screen.
	GetParent()->CenterWindow(GetDesktopWindow());

	// Put us in front.
	GetParent()->SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(GetParent());
}

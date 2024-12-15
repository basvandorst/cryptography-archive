//////////////////////////////////////////////////////////////////////////////
// CSavePGPdiskDialog.cpp
//
// Implementation of the CSavePGPdiskDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSavePGPdiskDialog.cpp,v 1.9 1998/12/15 01:25:04 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "FatUtils.h"
#include "PGPdiskFileFormat.h"
#include "PGPdiskPrefs.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"

#include "CSavePGPdiskDialog.h"
#include "File.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CSavePGPdiskDialog, CFileDialog)
	//{{AFX_MSG_MAP(CSavePGPdiskDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// CSavePGPdiskDialog public custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CSavePGPdiskDialog, CFileDialog)

// The CSavePGPdiskDialog default constructor.

CSavePGPdiskDialog::CSavePGPdiskDialog(SaveDialogType dlgType, CWnd *pParent)

	: CFileDialog(FALSE, &kPGPdiskFileExtension[1], NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		GetCommonString(kPGPdiskSDTSaveFilter), pParent)
{
	mSaveDialogType = dlgType;

	m_ofn.Flags |= OFN_PATHMUSTEXIST | OFN_NOTESTFILECREATE;

	// Finally initialize the title according to the dialog type.
	switch (mSaveDialogType)
	{
	case kSDT_Create:
		m_ofn.lpstrTitle = GetCommonString(kPGPdiskSDTCreateDialogTitle);
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}

// AskForPath is a wrapper around DoModal.

DualErr 
CSavePGPdiskDialog::AskForPath(CString *path, LPCSTR defaultPath)
{
	DualErr derr;

	pgpAssertAddrValid(path, CString);

	try
	{
		CString		initialDir, initialName;
		DualErr		dirValidErr;
		PGPBoolean	gotDirFromRegistry	= FALSE;
		PGPUInt32	result;

		// If default path is not provided, get default info from registry.
		if (defaultPath && (strlen(defaultPath) > 0))
		{
			// Extract the initial directory.
			GetDirectory(defaultPath, &initialDir);
			GetBareName(defaultPath, &initialName, kBNE_ShowExt);
		}
		else
		{
			PGPdiskWin32Prefs	prefs;

			gotDirFromRegistry = TRUE;

			if (GetPGPdiskWin32Prefs(prefs).IsntError())
				initialDir = prefs.lastSaveDir;
		}

		// Is the directory OK to use?
		if (!App->IsDirectoryValid(initialDir))
		{
			dirValidErr = DualErr(kPGDMinorError_DirectoryNotFound);
		}
		else if (App->IsDirectoryReadOnly(initialDir))
		{
			dirValidErr = DualErr(kPGDMinorError_DirectoryWriteProtected);
		}
		else
		{
			PGPUInt64 bytesFree;

			// Fail if not enough space for any PGPdisk.
			if (App->HowMuchFreeSpace(initialDir, &bytesFree).IsntError())
			{
				PGPUInt32 kbFree = (PGPUInt32) (bytesFree/kBytesPerKb);

				if (kbFree < App->mPlatformMinKbSize)
					dirValidErr = DualErr(kPGDMinorError_DirectoryTooSmall);
			}
		}

		// If the directory isn't valid and it was user specified, fail.
		// Otherwise let the system choose a directory.
		if (dirValidErr.IsError())
		{
			if (gotDirFromRegistry)
				initialDir = kEmptyString;
			else
				derr = dirValidErr;
		}

		// If OK initialize the default directory and default filename.
		if (derr.IsntError())
		{
			// Don't use system default name.
			if (initialName.IsEmpty() || 
				(initialName.Find("New PGPdisk Volume") != -1))
			{
				GetInitialPGPdiskName(initialDir, &initialName);
			}

			m_ofn.lpstrInitialDir = initialDir;
			m_ofn.lpstrFile = initialName.GetBuffer(kMaxStringSize);

			// Show the dialog.
			result = DoModal();

			// Return an appropriate result.
			switch (result)
			{
			case IDOK:
				(* path) = GetPathName();
				break;

			case -1:
				derr = DualErr(kPGDMinorError_DialogDisplayFailed);
				break;

			case IDCANCEL:
				derr = DualErr(kPGDMinorError_UserAbort);
				break;
			}

			initialName.ReleaseBuffer();
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}


/////////////////////////////////////////////////////////////////////////
// CSavePGPdiskDialog protected custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////

// GetInitialPGPdiskName constructs an appropriate initial PGPdisk name.

void 
CSavePGPdiskDialog::GetInitialPGPdiskName(
	LPCSTR	initialDir, 
	CString	*initialName)
{
	pgpAssertAddrValid(initialName, CString);

	try
	{
		CString		fullPath, csInitialDir, postfixString;
		PGPUInt32	i	= 1;

		initialName->Format(kDefaultNewDiskName, "");
		csInitialDir = initialDir;

		if (csInitialDir.IsEmpty())
			GetCurrentDirectory(&csInitialDir);
		
		if (!csInitialDir.IsEmpty() && 
			(csInitialDir.GetAt(csInitialDir.GetLength() - 1) != '\\'))
		{
			csInitialDir += "\\";
		}

		while (TRUE)
		{
			initialName->Format(kDefaultNewDiskName, postfixString);
			fullPath = csInitialDir + (* initialName);	
	
			if (!App->IsFileValid(fullPath))
				break;

			postfixString.Format("%d", i++);
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// UpdateSaveDialogPrefs stores the location of the chosen directory.

void 
CSavePGPdiskDialog::UpdateSaveDialogPrefs()
{
	// Update the last save directory in the registry.
	try
	{
		CString				dir, path;
		PGPdiskWin32Prefs	prefs;

		path = GetPathName();
		
		GetDirectory(path, &dir);

		if (GetPGPdiskWin32Prefs(prefs).IsntError())
		{
			strcpy(prefs.lastSaveDir, dir);
			SetPGPdiskWin32Prefs(prefs);
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// The CSavePGPdiskDialog default data-exchange function.

void 
CSavePGPdiskDialog::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);

		//{{AFX_DATA_MAP(CSavePGPdiskDialog)
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnFileNameOK is called when the user hits the OK button. We perform sanity
// checking.

BOOL 
CSavePGPdiskDialog::OnFileNameOK()
{
	PGPBoolean failThis = FALSE;

	if (CFileDialog::OnFileNameOK() == 1)
		return 1;

	try
	{
		CString dir, path;

		path = GetPathName();
		 
		GetDirectory(path, &dir);

		// Is dir read-only?
        if (App->IsDirectoryReadOnly(dir))
		{
			failThis = TRUE;
			ReportError(kPGDMajorError_ChoseReadOnlyDirInCreate);
		}

		// If user is replacing a file, is that file in use?
		if (!failThis)
		{
			if (App->IsFileValid(path) && App->IsFileInUse(path))
			{
				failThis = TRUE;
				ReportError(kPGDMajorError_ChoseReplaceFileInUse);
			}
		}

		// Is there enough space?
		if (!failThis)
		{
			DualErr		dummyErr;
			PGPUInt32	kbFree;
			PGPUInt64	bytesFile, bytesFree;

			// Get amount of free space on disk.
			dummyErr = App->HowMuchFreeSpace(dir, &bytesFree);

			if (dummyErr.IsntError())
			{
				kbFree = (PGPUInt32) (bytesFree / kBytesPerKb);

				// If we are overwriting a file, add its size to free space.
				if (App->IsFileValid(path))
				{
					File existingFile;

					dummyErr = existingFile.Open(path, 
						kOF_MustExist | kOF_ReadOnly);

					if (dummyErr.IsntError())
					{
						existingFile.GetLength(&bytesFile);
					}

					if (dummyErr.IsntError())
					{
						kbFree += (PGPUInt32) (bytesFile/kBytesPerKb);
					}

					if (existingFile.Opened())
						existingFile.Close();
				}
			}

			// Finally check if we have enough size.
			if (dummyErr.IsntError())
			{
				if (App->mPlatformMinKbSize > kbFree)
				{
					failThis = TRUE;
					ReportError(kPGDMajorError_ChoseTooSmallDirInCreate);
				}
			}
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	if (failThis)
	{
		return 1;
	}
	else
	{
		UpdateSaveDialogPrefs();
		return 0;
	}
}

// OnInitDone is called when initialization is done. We override it to stop
// MFC from centering the window on the parent.

void 
CSavePGPdiskDialog::OnInitDone()
{
	// Center on screen.
	GetParent()->CenterWindow(GetDesktopWindow());

	// Put us in front.
	GetParent()->SetForegroundWindow();

	// Put on top if necessary.
	App->TweakOnTopAttribute(GetParent());
}

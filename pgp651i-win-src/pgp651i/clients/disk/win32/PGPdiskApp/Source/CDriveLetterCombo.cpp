//////////////////////////////////////////////////////////////////////////////
// CDriveLetterCombo.cpp
//
// Implementation of class CDriveLetterCombo.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDriveLetterCombo.cpp,v 1.4 1998/12/14 18:56:47 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "UtilityFunctions.h"
#include "Win32Utils.h"

#include "CDriveLetterCombo.h"


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CDriveLetterCombo, CComboBox)
	//{{AFX_MSG_MAP(CDriveLetterCombo)
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(CBN_SELENDOK, OnSelEndOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDriveLetterCombo public custom functions and non-default message handlers
/////////////////////////////////////////////////////////////////////////////

// The CDriveLetterCombo default constructor.

CDriveLetterCombo::CDriveLetterCombo()
{
}

// The CDriveLetterCombo default destructor.

CDriveLetterCombo::~CDriveLetterCombo()
{
}

// InitDriveLetters fills the control with a list of free drive letters.

void 
CDriveLetterCombo::InitDriveLetters() 
{	
	try
	{
		CString		driveString;
		PGPUInt8	i;
		PGPUInt32	drives;

		// Fill the combo box with available drive letters.
		drives = GetLogicalDrives();
		
		for (i = 2; i < kMaxDrives; i++)
		{
			if (!(drives & (1 << i)) && !IsDriveNetworkMapped(i))
			{
				driveString.Format("%c:", DriveNumToLet(i));
				AddString(driveString);
			}
		}

		// Initialize us to point to the first drive on the list.
		SetCurSel(0);
		GetLBText(GetCurSel(), driveString);

		mDriveNumber = DriveLetToNum(driveString[0]);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// SetDriveLetter sets the current selection to the drive letter corresponding
// to the given drive letter.

void 
CDriveLetterCombo::SetDriveLetter(PGPUInt8 drive)
{
	try
	{
		CString		driveString;
		PGPInt32	prefDriveIndex;

		pgpAssert(IsLegalDriveNumber(drive));

		driveString.Format("%c:", DriveNumToLet(drive));
		prefDriveIndex = FindString(-1, driveString);

		if (prefDriveIndex == CB_ERR)
			prefDriveIndex = 0;

		SetCurSel(prefDriveIndex);
		GetLBText(prefDriveIndex, driveString);

		mDriveNumber = DriveLetToNum(driveString[0]);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

////////////////////////////////////////////////////
// CDriveLetterCombo public default message handlers
////////////////////////////////////////////////////

// We override OnChar so we can implement type-selection correctly.

void 
CDriveLetterCombo::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	char		driveString[2];
	PGPInt32	newDriveIndex;

	driveString[0]	= toupper(nChar);
	driveString[1]	= '\0';

	newDriveIndex = FindString(-1, driveString);

	if (newDriveIndex != CB_ERR)
	{
		SetCurSel(newDriveIndex);
		mDriveNumber = DriveLetToNum(nChar);
	}
}

// We override OnSelEndOk to update the drive letter variable after the user
// makes a new choice.

void 
CDriveLetterCombo::OnSelEndOk() 
{
	try
	{
		CString driveString;

		GetLBText(GetCurSel(), driveString);
		mDriveNumber = DriveLetToNum(driveString[0]);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

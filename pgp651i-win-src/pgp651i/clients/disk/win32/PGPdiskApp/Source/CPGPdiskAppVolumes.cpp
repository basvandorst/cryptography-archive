//////////////////////////////////////////////////////////////////////////////
// CPGPdiskAppVolumes.cpp
//
// Helper functions for volume management.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskAppVolumes.cpp,v 1.15 1999/02/26 04:09:57 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "DriverComm.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskPrefs.h"
#include "PGPdiskPublicKeyUtils.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"

#include "CConvertWizardSheet.h"
#include "CDiskWizardSheet.h"
#include "CGlobalCWizardSheet.h"
#include "CMainDialog.h"
#include "CNagBuyDialog.h"
#include "COpenPGPdiskDialog.h"
#include "CPGPdiskApp.h"
#include "CPGPdiskAppVolumes.h"
#include "CSavePGPdiskDialog.h"
#include "CSinglePassphraseDialog.h"
#include "CUnmountPGPdiskDialog.h"
#include "PGPdisk.h"


/////////////////////////////////////////
// High-Level Mount and Creation Routines
/////////////////////////////////////////

// AskUserMountPGPdisk asks the user to select a PGPdisk to mount.

void 
CPGPdiskApp::AskUserMountPGPdisk()
{
	AppCommandInfo		ACI;
	COpenPGPdiskDialog	mountPGPdiskDlg(kODT_Mount);
	CString				path;
	DualErr				derr;

	// Any drive letters free?
	if (!AreAnyDriveLettersFree())
	{
		derr = DualErr(kPGDMinorError_NoDriveLettersFree);
	}

	// Show the dialog.
	if (derr.IsntError())
	{
		derr = mountPGPdiskDlg.AskForPath(&path);
	}

	if (derr.IsntError())
	{
		// Now attempt the mount.
		ACI.op		= kAppOp_Mount;
		ACI.flags	= NULL;
		ACI.drive	= kInvalidDrive;
		strcpy(ACI.path, path);

		DispatchAppCommandInfo(&ACI);	
	}

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_PGPdiskMountFailed, derr);
	}
}

// AskUserUnmountPGPdisk asks the user to select a PGPdisk to unmount.

void 
CPGPdiskApp::AskUserUnmountPGPdisk()
{
	CUnmountPGPdiskDialog	unmountPGPdiskDlg;
	DualErr					derr;

	// Get PGPdisk info.
	derr = UpdateAppPGPdiskInfo();

	// Complain if there are no PGPdisks to unmount.
	if (derr.IsntError())
	{
		if (mPGPdisks.GetNumPGPdisks() == 0)
		{
			DisplayMessage(kPGPdiskNoPGPdisksToUnmountString);
			return;
		}
	}

	// Show the dialog.
	if (derr.IsntError())
	{
		derr = unmountPGPdiskDlg.AskForDriveLetter();
	}

	// Attempt the unmount.
	if (derr.IsntError())
	{
		AppCommandInfo ACI;

		ACI.op		= kAppOp_Unmount;
		ACI.flags	= NULL;
		ACI.drive	= unmountPGPdiskDlg.mDriveToUnmount;
		ACI.path[0]	= '\0';

		DispatchAppCommandInfo(&ACI);
	}

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_PGPdiskMountFailed, derr);
	}
}

// AskUserNewPGPdisk calls down to begin the PGPdisk creation process.

void 
CPGPdiskApp::AskUserNewPGPdisk()
{
	AppCommandInfo ACI;

	// We don't ask for the path here.
	ACI.op		= kAppOp_Create;
	ACI.flags	= NULL;
	ACI.drive	= kInvalidDrive;
	ACI.path[0]	= '\0';

	DispatchAppCommandInfo(&ACI);
}

// AskUserGlobalConvertPGPdisks starts the global PGPdisk conversion process.

void 
CPGPdiskApp::AskUserGlobalConvertPGPdisks()
{
	AppCommandInfo ACI;

	ACI.op		= kAppOp_GlobalConvert;
	ACI.flags	= NULL;
	ACI.drive	= kInvalidDrive;
	ACI.path[0]	= '\0';

	DispatchAppCommandInfo(&ACI);
}


/////////////////////////////////////////////////
// Low-Level Mount, Unmount and Creation Routines
/////////////////////////////////////////////////

// UpdateAppPGPdiskInfo asks the driver for info on all mounted PGPdisks and
// makes sure we are current.

DualErr 
CPGPdiskApp::UpdateAppPGPdiskInfo()
{
	DualErr		derr;
	PGPBoolean	createdPDIArray;
	PGPdiskInfo	*pPDIArray;

	pgpAssert(IsDriverOpen());

	// Allocate space for the PGPdiskInfo array.
	try
	{
		pPDIArray = new PGPdiskInfo[kMaxDrives];
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	createdPDIArray = derr.IsntError();

	// Ask the driver to fill in our array.
	if (derr.IsntError())
	{
		derr = GetPGPdiskInfo(pPDIArray, kMaxDrives);
	}

	// Update our list of PGPdisks accordingly.
	if (derr.IsntError())
	{
		PGPdisk		*pPGD;
		PGPdiskInfo	*pPDI;
		PGPUInt32	i;

		// Empty the container.
		mPGPdisks.ClearContainerWithDelete();

		// Parse the array.
		for (i = 0; i < kMaxDrives; i++)
		{
			pPDI = &pPDIArray[i];

			// Legal drive letters represent PGPdisks.
			if (IsLegalDriveNumber(pPDI->drive))
			{
				PGPBoolean createdPGPdiskObject = FALSE;

				// Create a new PGPdisk object.
				try
				{
					pPGD = new PGPdisk(pPDI->path, pPDI->drive, 
						pPDI->sessionId);
				}
				catch (CMemoryException *ex)
				{
					derr = DualErr(kPGDMinorError_OutOfMemory);
					ex->Delete();
				}

				createdPGPdiskObject = derr.IsntError();

				// Check if created successfully.
				if (derr.IsntError())
				{
					derr = pPGD->mInitErr;
				}

				// Put the PGPdisk in the global list.
				if (derr.IsntError())
				{
					mPGPdisks.AddPGPdisk(pPGD);
				}

				if (derr.IsError())
				{
					if (createdPGPdiskObject)
						delete pPGD;
				}
			}

			if (derr.IsError())
			{
				break;
			}		
		}
	}

	if (createdPDIArray)
		delete[] pPDIArray;

	return derr;
}

// GlobalConvertPGPdisks invokes the global PGPdisk conversion wizard.

DualErr 
CPGPdiskApp::GlobalConvertPGPdisks()
{
	DualErr				derr;
	CGlobalCWizardSheet	globalCPGPdiskSheet;

	derr = globalCPGPdiskSheet.mInitErr;

	if (derr.IsntError())
	{
		derr = globalCPGPdiskSheet.ExecuteWizard();
	}

	return derr;
}

// ConvertPGPdiskPriorToMount attempts to convert a PGPdisk with bad CAST to
// one with good CAST.

DualErr 
CPGPdiskApp::ConvertPGPdiskPriorToMount(LPCSTR path, PGPBoolean *pUserSaysNo)
{
	DualErr		derr;
	PGPBoolean	userSaysNo	= FALSE;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(pUserSaysNo, PGPBoolean);

	if (IsFileReadOnly(path))
	{
		DisplayMessage(kPGPdiskBadCASTReadOnly);
		userSaysNo = TRUE;
	}
	else
	{
		PGPBoolean		restartingConversion;
		UserResponse	response;

		// Is this PGPdisk partially converted already?
		restartingConversion = WasPGPdiskConversionInterrupted(path);

		if (restartingConversion)
		{
			response = DisplayMessage(kPGPdiskBadCASTAskContinuePM, 
				kPMBS_YesNo, kPMBF_YesButton);
		}
		else
		{
			response = DisplayMessage(kPGPdiskBadCASTAskConvertPM, 
				kPMBS_YesNoCancel, kPMBF_YesButton);
		}

		// Do what the user wants in regards to conversion.
		switch (response)
		{
			case kUR_Yes:
				{
					CConvertWizardSheet convertPGPdiskSheet;

					derr = convertPGPdiskSheet.mInitErr;

					if (derr.IsntError())
					{
						derr = convertPGPdiskSheet.ExecuteWizard(path, 
							restartingConversion);
					}
				}
				break;

			case kUR_No:
				userSaysNo = TRUE;

				if (restartingConversion)
					derr = kPGDMinorError_UserAbort;
				break;

			case kUR_Cancel:
				derr = kPGDMinorError_UserAbort;
				break;
		}
	}

	(* pUserSaysNo) = userSaysNo;

	return derr;
}

// ConvertPGPdiskFromGlobalWizard attempts to convert a PGPdisk with bad CAST
// to one with good CAST.

DualErr 
CPGPdiskApp::ConvertPGPdiskFromGlobalWizard(
	LPCSTR		path, 
	PGPBoolean	*pUserSaysCancel)
{
	DualErr		derr;
	PGPBoolean	userSaysCancel	= FALSE;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(pUserSaysCancel, PGPBoolean);

	if (IsFileReadOnly(path))
	{
		DisplayMessage(kPGPdiskBadCASTReadOnly);
	}
	else
	{
		PGPBoolean		restartingConversion;
		UserResponse	response;

		// Is this PGPdisk partially converted already?
		restartingConversion = WasPGPdiskConversionInterrupted(path);

		if (restartingConversion)
		{
			response = DisplayMessage(kPGPdiskBadCASTAskContinuePNM, 
				kPMBS_YesNoCancel, kPMBF_YesButton);
		}
		else
		{
			response = DisplayMessage(kPGPdiskBadCASTAskConvertPNM, 
				kPMBS_YesNoCancel, kPMBF_YesButton);
		}

		// Do what the user wants in regards to conversion.
		switch (response)
		{
			case kUR_Yes:
				{
					CConvertWizardSheet convertPGPdiskSheet;

					derr = convertPGPdiskSheet.mInitErr;

					if (derr.IsntError())
					{
						derr = convertPGPdiskSheet.ExecuteWizard(path, 
							restartingConversion);
					}
				}
				break;

			case kUR_No:
				derr = kPGDMinorError_UserAbort;
				break;

			case kUR_Cancel:
				userSaysCancel = TRUE;
				break;
		}
	}

	(* pUserSaysCancel) = userSaysCancel;

	return derr;
}

// ConvertPGPdisk attempts to convert a PGPdisk with bad CAST to one
// with good CAST.

DualErr 
CPGPdiskApp::ConvertPGPdisk(LPCSTR path)
{
	DualErr derr;

	pgpAssertStrValid(path);

	if (IsFileReadOnly(path))
	{
		DisplayMessage(kPGPdiskBadCASTReadOnly);
	}
	else
	{
		PGPBoolean		restartingConversion;
		UserResponse	response;

		// Is this PGPdisk partially converted already?
		restartingConversion = WasPGPdiskConversionInterrupted(path);

		if (restartingConversion)
		{
			response = DisplayMessage(kPGPdiskBadCASTAskContinuePNM, 
				kPMBS_YesNo, kPMBF_YesButton);
		}
		else
		{
			response = DisplayMessage(kPGPdiskBadCASTAskConvertPNM, 
				kPMBS_YesNo, kPMBF_YesButton);
		}

		// Do what the user wants in regards to conversion.
		switch (response)
		{
			case kUR_Yes:
				{
					CConvertWizardSheet convertPGPdiskSheet;

					derr = convertPGPdiskSheet.mInitErr;

					if (derr.IsntError())
					{
						derr = convertPGPdiskSheet.ExecuteWizard(path, 
							restartingConversion);
					}
				}
				break;

			case kUR_No:
				derr = kPGDMinorError_UserAbort;
				break;
		}
	}

	return derr;
}

// CreatePGPdisk asks the user to specify a location for a new PGPdisk and
// then displays the wizard.

DualErr 
CPGPdiskApp::CreatePGPdisk(LPCSTR defaultPath)
{
	CDiskWizardSheet	newPGPdiskWizard;
	DualErr				derr;
	PGPBoolean			wasMainEnabled;

	// Forcibly disable main window so it doesn't flicker between dialogs.
	if (mMainDialog)
		wasMainEnabled = !mMainDialog->EnableWindow(FALSE);

	// Did our wizard initialize correctly?
	derr = newPGPdiskWizard.mInitErr;

	// Any drive letters free?
	if (derr.IsntError())
	{
		if (!AreAnyDriveLettersFree())
		{
			derr = DualErr(kPGDMinorError_NoDriveLettersFree);
		}
	}

	// Fire up the wizard.
	if (derr.IsntError())
	{
		derr = newPGPdiskWizard.ExecuteWizard(defaultPath);
	}

	// Re-enable main dialog if it was disabled.
	if (mMainDialog && wasMainEnabled)
		mMainDialog->EnableWindow(TRUE);

	return derr;
}

// MountPGPdisk first determines if we can mount the PGPdisk specified by
// 'path', and then creates an associated PGPdisk object and asks it to mount
// itself.

DualErr 
CPGPdiskApp::MountPGPdisk(
	LPCSTR			path, 
	PGPBoolean		forceReadOnly, 
	PGPBoolean		useDialog, 
	PGPBoolean		needMaster, 
	PGPUInt8		drive, 
	SecureString	*passphrase)
{
	CSinglePassphraseDialog	mountPassDlg(kSPDT_Mount);
	DualErr					derr;
	PGPBoolean				createdPGPdiskObject	= FALSE;
	PGPdisk					*pPGD;

	if (useDialog)
	{
		// Make sure our passphrase dialog initialized correctly.
		derr = mountPassDlg.mInitErr;
	}
	else
	{
		pgpAssert(IsLegalDriveNumber(drive));
		pgpAssertAddrValid(passphrase, SecureString);
	}

	// If the PGPdisk file is mounted already, show its window.
	if (derr.IsntError())
	{
		pPGD = mPGPdisks.FindPGPdisk(path);

		if (IsntNull(pPGD))
		{
			pPGD->BrowseToVolume();
			derr = DualErr(kPGDMinorError_FailSilently);
		}
	}

	// Validate the PGPdisk.
	if (derr.IsntError())
	{
		derr = ValidatePGPdisk(path);
	}

	// If the PGPdisk is opened by someone else with write access, fail.
	if (derr.IsntError())
	{
		if (IsFileInUseByWriter(path))
			derr = DualErr(kPGDMinorError_PGPdiskAlreadyInUse);
	}

	// Any drive letters free?
	if (derr.IsntError())
	{
		if (!AreAnyDriveLettersFree())
		{
			derr = DualErr(kPGDMinorError_NoDriveLettersFree);
		}
	}

	// Check if there is room to mount another volume.
	if (derr.IsntError())
	{
		if (mPGPdisks.GetNumPGPdisks() >= kMaxMountedPGPdisks)
			derr = DualErr(kPGDMinorError_MaxPGPdisksMounted);
	}

	// Create a new PGPdisk object.
	if (derr.IsntError())
	{
		try
		{
			pPGD = new PGPdisk();
		}
		catch (CMemoryException *ex)
		{
			derr = DualErr(kPGDMinorError_OutOfMemory);
			ex->Delete();
		}

		createdPGPdiskObject = derr.IsntError();
	}

	if (derr.IsntError())
	{
		derr = pPGD->mInitErr;
	}

	// Get the passphrase.
	if (derr.IsntError() && useDialog)
	{
		derr = mountPassDlg.AskForPassphrase(path, forceReadOnly, needMaster);
	}

	if (derr.IsntError())
	{
		// If the PGPdisk is marked as being mounted, warn.
//		if (GetPGPdiskMountedFlag(path))
//			DisplayMessage(kPGPdiskNotifyMountMaybeCorrupt);

		// Attempt the actual mount.
		if (useDialog)
		{
			passphrase = mountPassDlg.mPassphraseEdit.mContents;
			drive = mountPassDlg.mDriveCombo.mDriveNumber;

			if (!forceReadOnly)
				forceReadOnly = mountPassDlg.mReadOnlyValue;
		}

		derr = pPGD->Mount(path, passphrase, drive, forceReadOnly);
	}

#if PGPDISK_DEMOVERSION

	if (derr.IsntError())
	{
		// Set demo timeout after (first) successful mount.
		SetPGPdiskDemoTimeout();
	}

#endif // PGPDISK_DEMOVERSION

	if (derr.IsntError())
	{
		CString				dir;
		PGPdiskWin32Prefs	prefs;

		// Add the PGPdisk to the global list.
		mPGPdisks.AddPGPdisk(pPGD);

		// Update the last used directory registy key.
		GetDirectory(path, &dir);

		if (GetPGPdiskWin32Prefs(prefs).IsntError())
		{
			strcpy(prefs.lastOpenDir, dir);
			SetPGPdiskWin32Prefs(prefs);
		}

		// If not formatted, offer user chance to format.
		if (useDialog && !IsVolumeFormatted(pPGD->GetDrive()))
		{
			UserResponse button;

			button = DisplayMessage(kPGPdiskConfirmFormat, kPMBS_YesNo, 
				kPMBF_YesButton);

			if (button == kUR_Yes)
				pPGD->HighLevelFormatPGPdisk();
		}
	}

	// Cleanup if error.
	if (derr.IsError())
	{
		if (createdPGPdiskObject)
		{
			delete pPGD;
		}
	}
	
	return derr;
}

// UnmountPGPdisk unmounts the specified PGPdisk volume.

DualErr 
CPGPdiskApp::UnmountPGPdisk(PGPUInt8 drive, PGPBoolean isThisEmergency)
{
	DualErr		afterFact, derr;
	PGPBoolean	removedPGPdisk, unmountedChildren;
	PGPUInt32	i	= 0;
	PGPdisk		*pPGD;

	removedPGPdisk = unmountedChildren = FALSE;

	pgpAssert(IsLegalDriveNumber(drive));

	// Find the associated PGPdisk object.
	pPGD = mPGPdisks.FindPGPdisk(drive);

	if (IsNull(pPGD))
		derr = DualErr(kPGDMinorError_PGPdiskNotMounted);

	// If this PGPdisk has other PGPdisks mounted on it, we must perform a
	// depth-first unmount of all those PGPdisks first.

	while (derr.IsntError())
	{
		PGPdisk *pClient;

		pClient = mPGPdisks.EnumPGPdisks(i++);

		if (IsNull(pClient))
			break;

		if (!pClient->IsHostNetworked() && 
			(pClient->GetLocalHostDrive() == drive))
		{
			i = 0;
			derr = UnmountPGPdisk(pClient->GetDrive(), isThisEmergency);

			if (isThisEmergency)
				derr = DualErr::NoError;

			unmountedChildren = derr.IsntError();
		}
	}

	// Check if the PGPdisk has open files.
	if (derr.IsntError())
	{
		// If we unmounted any children, we may have to wait a little while
		// for the system to flush its cache. Try about 10 times.

		if (unmountedChildren)
		{
			i = 0;

			while (pPGD->HasOpenFiles() && (i++ < kMaxCheckOpenFilesAttempts))
				Sleep(500);
		}

		if (pPGD->HasOpenFiles())
			derr = DualErr(kPGDMinorError_FilesOpenOnDrive);

		if (isThisEmergency)
			derr = DualErr::NoError;
	}

	// Now we are safe to unmount. Remove it from the global list and unmount.
	if (derr.IsntError())
	{
		mPGPdisks.RemovePGPdisk(pPGD);
		removedPGPdisk = TRUE;

		derr = pPGD->Unmount(isThisEmergency);

		if (isThisEmergency)
			derr = DualErr::NoError;
	}

	// Delete the PGPdisk object.
	if (derr.IsntError())
	{
		delete pPGD;
	}

	// Cleanup if error.
	if (derr.IsError())
	{
		if (removedPGPdisk && pPGD->Mounted())
			mPGPdisks.AddPGPdisk(pPGD);
	}

	return derr;
}

// UnmountAllPGPdisks unmounts all mounted PGPdisks on the system.

DualErr 
CPGPdiskApp::UnmountAllPGPdisks(PGPBoolean isThisEmergency)
{
	DualErr		derr, storedDerr;
	PGPUInt32	i	= 0;
	PGPdisk		*pPGD;

	// Note how we enumerate on position 0, and increase this only when we
	// find a drive we can't unmount for whatever reason. This is because
	// if a drive can't be unmounted it won't be removed, and thus 'sticks'
	// at position 0, 1, etc.

	pPGD = mPGPdisks.EnumPGPdisks(i);

	while (IsntNull(pPGD))
	{
		PGPUInt8 drive;

		pgpAssert(pPGD->Mounted());
		drive = pPGD->GetDrive();

		// Now attempt the unmount.
		derr = UnmountPGPdisk(drive, isThisEmergency);

		if (isThisEmergency)
			derr = DualErr::NoError;

		// If there is an error, note that we saw an error and continue
		// unmounting.

		if (derr.IsError())
		{
			if (storedDerr.IsntError())
				storedDerr = derr;

			derr = DualErr::NoError;
			i++;
		}

		pPGD = mPGPdisks.EnumPGPdisks(i);
	}

	return storedDerr;
}

//////////////////////////////////////////////////////////////////////////////
// CPGPdiskAppACI.cpp
//
// Functions for processing AppCommandInfo structures.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskAppACI.cpp,v 1.1.2.18.2.6 1998/10/28 00:13:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskHighLevelUtils.h"
#include "UtilityFunctions.h"

#include "CMainDialog.h"
#include "CNagBuyDialog.h"
#include "CPGPdiskApp.h"
#include "CPGPdiskAppACI.h"
#include "PGPdisk.h"


////////////////////////////
// AppCommandInfo Processing
////////////////////////////

// AreAppCommandsSimilar returns TRUE if the two specified ACI's are similar.

PGPBoolean 
CPGPdiskApp::AreAppCommandsSimilar(
	PAppCommandInfo	pACI1, 
	PAppCommandInfo	pACI2)
{
	DualErr		derr;
	PGPBoolean	areCommandsSimilar	= FALSE;

	try
	{
		AppOp		op1, op2;
		CString		path1, path2;
		PGPUInt8	drive1, drive2;

		pgpAssertAddrValid(pACI1, AppCommandInfo);
		pgpAssertAddrValid(pACI2, AppCommandInfo);

		// Canonicalize the commands.
		derr = CanonicalizeAppCommandInfo(pACI1);

		if (derr.IsntError())
		{
			derr = CanonicalizeAppCommandInfo(pACI2);
		}

		// Extract the parameters.
		if (derr.IsntError())
		{
			path1	= pACI1->path;
			drive1	= pACI1->drive;
			op1		= pACI1->op;

			path2	= pACI2->path;
			drive2	= pACI2->drive;
			op2		= pACI2->op;

			// Compare the commands.
			switch (op1)
			{
			case kAppOp_Mount:
			case kAppOp_AddPassphrase:
			case kAppOp_ChangePassphrase:
			case kAppOp_RemovePassphrase:
			case kAppOp_RemoveAlternates:
			case kAppOp_AddRemovePublicKeys:
			case kAppOp_WipePassesThisDisk:

				// 'op2' must be one of the above as well.
				switch (op2)
				{
				case kAppOp_Mount:
				case kAppOp_AddPassphrase:
				case kAppOp_ChangePassphrase:
				case kAppOp_RemovePassphrase:
				case kAppOp_RemoveAlternates:
				case kAppOp_AddRemovePublicKeys:
				case kAppOp_WipePassesThisDisk:

					// Also the paths must match.
					if (path1 == path2)
						areCommandsSimilar = TRUE;
					break;
				}
				break;

			case kAppOp_Unmount:
				// For 'unmount' the drive letters must be equal.
				if (drive1 == drive2)
					areCommandsSimilar = TRUE;
				break;

			default:
				// All others match by default.
				if (op1 == op2)
					areCommandsSimilar = TRUE;
				break;
			}
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsError())
		return FALSE;
	else
		return areCommandsSimilar;
}

#if PGPDISK_BETAVERSION

// CheckACIBetaWarnings checks if the beta has timed out and if so performs
// command filtering.

DualErr 
CPGPdiskApp::CheckACIBetaWarnings(PAppCommandInfo pACI)
{
	DualErr derr;

	pgpAssertAddrValid(pACI, AppCommandInfo);

	if (HasBetaTimedOut())
	{
		switch (pACI->op)
		{
		case kAppOp_Create:
		case kAppOp_AddPassphrase:
		case kAppOp_ChangePassphrase:
		case kAppOp_RemovePassphrase:
		case kAppOp_RemoveAlternates:
		case kAppOp_AddRemovePublicKeys:

			// We can't run these commands after beta timeout.
			ReportError(kPGDMajorError_BadCommandAfterBetaTimeout);
			derr = DualErr(kPGDMinorError_FailSilently);
			break;

		default:

			// Otherwise just warn.
			if (mCommandLineMode)
				ReportError(kPGPdiskBetaTimeoutWarning);
			break;
		}
	}

	return derr;
}

#elif PGPDISK_DEMOVERSION

// CheckACIDemoWarnings checks if the demo has timed out and if so performs
// command filtering.

DualErr 
CPGPdiskApp::CheckACIDemoWarnings(PAppCommandInfo pACI)
{
	DualErr derr;

	pgpAssertAddrValid(pACI, AppCommandInfo);

	if (HasDemoTimedOut())
	{
		CNagBuyDialog nagBuyDialog;

		switch (pACI->op)
		{
		case kAppOp_Create:
		case kAppOp_AddPassphrase:
		case kAppOp_ChangePassphrase:
		case kAppOp_RemovePassphrase:
		case kAppOp_RemoveAlternates:
		case kAppOp_AddRemovePublicKeys:

			// We can't run these commands after demo timeout.
			nagBuyDialog.DisplayDialog(kPGPdiskDemoBadCommandWarning);
			derr = DualErr(kPGDMinorError_FailSilently);
			break;

		default:

			// Otherwise just warn.
			if (mCommandLineMode)
				nagBuyDialog.DisplayDialog(kPGPdiskDemoTimeoutWarning);
			break;
		}
	}

	return derr;
}

#endif // PGPDISK_BETAVERSION

// CanonicalizeAppCommandInfo takes an ACI structure and makes it acceptable
// for processing by lower-level functions.

DualErr 
CPGPdiskApp::CanonicalizeAppCommandInfo(PAppCommandInfo pACI)
{
	DualErr derr;

	try
	{
		AppOp		op;
		CString		path;
		PGPdisk		*pPGD;
		PGPUInt8	drive;

		pgpAssertAddrValid(pACI, AppCommandInfo);

		op		= pACI->op;
		path	= pACI->path;
		drive	= pACI->drive;

		// Only some commands need pre-processing.
		switch (op)
		{
		case kAppOp_Mount:

			// Resolve any shortcut.
			if (IsShortCut(path))
				derr = ResolveShortcut(path, &path);

			if (derr.IsntError())
			{
				// Resolve 'root' forms to the corresponding PGPdisk.
				if (IsPlainLocalRoot(path))
				{
					pPGD = mPGPdisks.FindPGPdisk(DriveFromPath(path));

					if (IsntNull(pPGD))
						path = pPGD->GetPath();
				}

				// Pretty the path.
				derr = VerifyAndCanonicalizePath(path, &path);
			}
			break;

		case kAppOp_AddPassphrase:
		case kAppOp_ChangePassphrase:
		case kAppOp_RemovePassphrase:
		case kAppOp_RemoveAlternates:
		case kAppOp_AddRemovePublicKeys:
		case kAppOp_WipePassesThisDisk:

			// Resolve any shortcut.
			if (IsShortCut(path))
				derr = ResolveShortcut(path, &path);

			if (derr.IsntError())
			{
				// Pretty the path.
				derr = VerifyAndCanonicalizePath(path, &path);
			}
			break;

		case kAppOp_Unmount:

			// If we don't have a drive, check for path.
			if (!IsLegalDriveNumber(drive))
			{
				pgpAssert(!path.IsEmpty());

				// Resolve any shortcut.
				if (IsShortCut(path))
					derr = ResolveShortcut(path, &path);

				// Extract the drive letter.
				if (derr.IsntError())
				{
					if (IsPlainLocalRoot(path))
					{
						// We have a root to a PGPdisk volume.
						drive = DriveLetToNum(path[0]);
					}
					else
					{
						// We have a path to a PGPdisk file.
						derr = VerifyAndCanonicalizePath(path, &path);

						if (derr.IsntError())
						{
							pPGD = mPGPdisks.FindPGPdisk(path);

							if (IsntNull(pPGD))
								drive = pPGD->GetDrive();
						}
					}
				}
			}
			break;
		}

		// Update the ACI with the modified values.
		if (derr.IsntError())
		{
			strcpy(pACI->path, path);
			pACI->drive = drive;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// PerformPGPdiskConversionCheck checks if the current PGPdisk needs a CAST
// conversion, and if so, asks the user.

DualErr 
CPGPdiskApp::PerformPGPdiskConversionCheck(PAppCommandInfo pACI)
{
	DualErr		derr;
	PGPBoolean	userSaidNo;

	switch (pACI->op)
	{
	case kAppOp_Mount:

		if (IsntNull(mPGPdisks.FindPGPdisk(pACI->path)))
			break;

		if (DoesPGPdiskHaveBadCAST(pACI->path))
		{
			derr = ConvertPGPdiskPriorToMount(pACI->path, &userSaidNo);

			if (derr.IsntError())
			{
				if (userSaidNo)
					pACI->flags |= kACF_ForceReadOnly;
				else
					pACI->flags |= kACF_NeedMaster;
			}
		}

		break;

	case kAppOp_AddPassphrase:
	case kAppOp_ChangePassphrase:
	case kAppOp_RemovePassphrase:
	case kAppOp_RemoveAlternates:
	case kAppOp_AddRemovePublicKeys:

		if (DoesPGPdiskHaveBadCAST(pACI->path))
		{
			derr = ConvertPGPdisk(pACI->path);
		}

		break;

	default:
		break;
	}

	return derr;
}

// DispatchAppCommandInfo takes the address of an AppCommandInfo structure
// and executes the command contained within.

void 
CPGPdiskApp::DispatchAppCommandInfo(PAppCommandInfo pACI)
{
	DualErr		derr;
	PGPBoolean	doSilentWiping, forceReadOnly, isThisEmergency, needMaster;
	PGPBoolean	noMountDialog;

	pgpAssertAddrValid(pACI, AppCommandInfo);

	// Get PGPdisk info.
	derr = UpdateAppPGPdiskInfo();

	// Prepare the ACI.
	if (derr.IsntError())
	{
		derr = CanonicalizeAppCommandInfo(pACI);
	}

	// Display timeout warnings as necessary.
	if (derr.IsntError())
	{
	#if PGPDISK_BETAVERSION
		derr = CheckACIBetaWarnings(pACI);
	#elif PGPDISK_DEMOVERSION
		derr = CheckACIDemoWarnings(pACI);
	#endif // PGPDISK_BETAVERSION
	}
	
	// If any other apps are doing the same thing, switch to them.
	if (derr.IsntError())
	{
		if (FindAndShowSimilarPGPdiskApp(pACI))
			derr = DualErr(kPGDMinorError_FailSilently);
	}
	
	if (derr.IsntError())
	{
		// Update shared memory with current command info.
		SetSharedMemoryCommandInfo(pACI);

		// Check if PGPdisk needs conversion.
		derr = PerformPGPdiskConversionCheck(pACI);
	}

	// Resolve flags.
	if (derr.IsntError())
	{
		isThisEmergency	= (pACI->flags & kACF_EmergencyUnmount ? TRUE : 
			FALSE);

		noMountDialog	= (pACI->flags & kACF_MountWithNoDialog ? TRUE : 
			FALSE);

		forceReadOnly	= (pACI->flags & kACF_ForceReadOnly ? TRUE : FALSE);
		doSilentWiping	= (pACI->flags & kACF_SilentWiping ? TRUE : FALSE);
		needMaster		= (pACI->flags & kACF_NeedMaster ? TRUE : FALSE);
	}

	switch (pACI->op)
	{
	case kAppOp_GlobalConvert:			// Invoke global conversion wizard.

		if (derr.IsntError())
		{
			derr = GlobalConvertPGPdisks();
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_PGPdiskGlobalConversionFailed, derr);
		}
		break;
	
	case kAppOp_Create:				// Create a new PGPdisk

		// Show splash screen if command-line mode and create.
		if (derr.IsntError())
		{
			if (mCommandLineMode)
				ShowSplashScreen();
		}

		if (derr.IsntError())
		{
			derr = CreatePGPdisk(pACI->path);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_PGPdiskCreationFailed, derr);
		}
		break;

	case kAppOp_Mount:					// Mount a PGPdisk

		if (derr.IsntError())
		{
			derr = MountPGPdisk(pACI->path, forceReadOnly, !noMountDialog, 
				needMaster, pACI->drive, (SecureString *) pACI->data[0]);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_PGPdiskMountFailed, derr);
		}
		break;

	case kAppOp_Unmount:				// Unmount a PGPdisk

		if (derr.IsntError())
		{
			derr = UnmountPGPdisk(pACI->drive, isThisEmergency);
		}

		if (derr.IsError() && !isThisEmergency)
		{
			ReportError(kPGDMajorError_PGPdiskUnmountFailed, derr, 
				pACI->drive);
		}
		break;

	case kAppOp_UnmountAll:				// Unmount all PGPdisks

		if (derr.IsntError())
		{
			derr = UnmountAllPGPdisks(isThisEmergency);
		}

		if (derr.IsError() && !isThisEmergency)
		{
			ReportError(kPGDMajorError_PGPdiskUnmountAllFailed, derr);
		}
		break;

	case kAppOp_AddPassphrase:			// Add a passphrase to a PGPdisk

		if (derr.IsntError())
		{
			derr = AddPGPdiskPassphrase(pACI->path);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_PGPdiskAddPassFailed, derr);
		}
		break;

	case kAppOp_ChangePassphrase:		// Change a passphrase on a PGPdisk

		if (derr.IsntError())
		{
			derr = ChangePGPdiskPassphrase(pACI->path);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_PGPdiskChangePassFailed, derr);
		}
		break;

	case kAppOp_RemovePassphrase:		// Remove a passphrase from a PGPdisk

		if (derr.IsntError())
		{
			derr = RemovePGPdiskPassphrase(pACI->path, FALSE);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_PGPdiskRemoveOnePassFailed, derr);
		}
		break;

	case kAppOp_RemoveAlternates:		// Remove all alternate passphrases

		if (derr.IsntError())
		{
			derr = RemovePGPdiskPassphrase(pACI->path, TRUE);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_PGPdiskRemoveAllPassesFailed, derr);
		}
		break;

#if PGPDISK_PUBLIC_KEY

	case kAppOp_AddRemovePublicKeys:	// Manage PGPdisk public keys

		if (derr.IsntError())
		{
			derr = AddRemovePGPdiskPublicKeys(pACI->path);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_AddRemovePublicKeysFailed, derr);
		}
		break;

#if PGPDISK_WIPE_FUNCS

	case kAppOp_WipePassesAllMounted:	// Wipe passes on all mounted disks

		if (derr.IsntError())
		{
			derr = WipePassesOnAllPGPdisks(doSilentWiping);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_WipePassesFailed, derr);
		}
		break;

	case kAppOp_WipePassesThisDisk:		// Wipe passes on one disk

		if (derr.IsntError())
		{
			derr = WipePassesOnThisPGPdisk(pACI->path, doSilentWiping);
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_WipePassesFailed, derr);
		}
		break;

#endif // PGPDISK_WIPE_FUNCS

#endif	// PGPDISK_PUBLIC_KEY
	
	case kAppOp_ShowAboutBox:			// Show the about box

		if (derr.IsntError())
		{
			derr = ShowPGPdiskAboutBox();
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_AboutBoxDisplayFailed, derr);
		}
		break;

	case kAppOp_ShowHelp:				// Show PGPdisk help

		if (derr.IsntError())
		{
			ShowPGPdiskHelp();
		}
		break;

	case kAppOp_ShowPrefs:				// Show the PGPdisk preferences

		if (derr.IsntError())
		{
			derr = ShowPGPdiskPrefs();
		}

		if (derr.IsError())
		{
			ReportError(kPGDMajorError_PreferencesAccessFailed, derr);
		}
		break;

	default:
		pgpAssert(FALSE);
		break;
	}

	// Clear shared memory command info.
	ClearSharedMemoryCommandInfo();
}

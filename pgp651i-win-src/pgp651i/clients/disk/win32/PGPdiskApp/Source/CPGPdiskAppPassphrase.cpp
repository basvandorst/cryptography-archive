//////////////////////////////////////////////////////////////////////////////
// CPGPdiskAppPassphrase.cpp
//
// Functions for adding, changing, and removing passphrases.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskAppPassphrase.cpp,v 1.9 1999/02/26 04:09:57 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <math.h>

#include "Required.h"
#include "GlobalPGPContext.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskPublicKeyUtils.h"
#include "StringAssociation.h"

#include "CConfirmPassphraseDialog.h"
#include "CMainDialog.h"
#include "COpenPGPdiskDialog.h"
#include "CPGPdiskApp.h"
#include "CPGPdiskAppPassphrase.h"

#include "pgpSDKPrefs.h"
#include "pgpUtilities.h"

#include "CPublicKeyDialog.h"
#include "CSinglePassphraseDialog.h"
#include "Globals.h"
#include "PGPdisk.h"


////////
// Types
////////

// ScoreStruct/Range is used by the CalcPassphraseQuality routine.

typedef struct ScoreStruct
{
	PGPUInt8	firstChar;
	PGPUInt8	lastChar;

} Range;


///////////////////////////////////////////////////////////////
// High-Level Passphrase Addition, Change, and Removal Routines
///////////////////////////////////////////////////////////////

// AskUserAddPassphrase asks to user to pick the PGPdisk for which he wants
// to add a passphrase.

void 
CPGPdiskApp::AskUserAddPassphrase()
{
	AppCommandInfo		ACI;
	COpenPGPdiskDialog	selectPGPdiskDlg(kODT_Select);
	CString				path;
	DualErr				derr;
	PGPBoolean			wasMainEnabled;

	// Forcibly disable main window so it doesn't flicker between dialogs.
	if (mMainDialog)
		wasMainEnabled = !mMainDialog->EnableWindow(FALSE);
	
	// Show the dialog.
	derr = selectPGPdiskDlg.AskForPath(&path);

	if (derr.IsntError())
	{
		// Now dispatch the command.
		ACI.op		= kAppOp_AddPassphrase;
		ACI.flags	= NULL;
		ACI.drive	= kInvalidDrive;
		strcpy(ACI.path, path);

		DispatchAppCommandInfo(&ACI);
	}

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_PGPdiskAddPassFailed, derr);
	}

	// Re-enable main dialog if it was disabled.
	if (IsntNull(mMainDialog) && wasMainEnabled)
		mMainDialog->EnableWindow(TRUE);
}

// AskUserChangePassphrase asks to user to pick the PGPdisk for which he
// wants to change a passphrase.

void 
CPGPdiskApp::AskUserChangePassphrase()
{
	AppCommandInfo		ACI;
	COpenPGPdiskDialog	selectPGPdiskDlg(kODT_Select);
	CString				path;
	DualErr				derr;
	PGPBoolean			wasMainEnabled;

	// Forcibly disable main window so it doesn't flicker between dialogs.
	if (mMainDialog)
		wasMainEnabled = !mMainDialog->EnableWindow(FALSE);

	// Show the dialog.
	derr = selectPGPdiskDlg.AskForPath(&path);

	if (derr.IsntError())
	{
		// Now dispatch the command.
		ACI.op		= kAppOp_ChangePassphrase;
		ACI.flags	= NULL;
		ACI.drive	= kInvalidDrive;
		strcpy(ACI.path, path);

		DispatchAppCommandInfo(&ACI);
	}

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_PGPdiskChangePassFailed, derr);
	}

	// Re-enable main dialog if it was disabled.
	if (IsntNull(mMainDialog) && wasMainEnabled)
		mMainDialog->EnableWindow(TRUE);
}

// AskUserRemovePassphrase asks to user to pick the PGPdisk for which he wants
// to remove a passphrase.

void 
CPGPdiskApp::AskUserRemovePassphrase(PGPBoolean removeAll)
{
	AppCommandInfo		ACI;
	COpenPGPdiskDialog	selectPGPdiskDlg(kODT_Select);
	CString				path;
	DualErr				derr;
	PGPBoolean			wasMainEnabled;

	// Forcibly disable main window so it doesn't flicker between dialogs.
	if (mMainDialog)
		wasMainEnabled = !mMainDialog->EnableWindow(FALSE);

	// Show the dialog.
	derr = selectPGPdiskDlg.AskForPath(&path);

	if (derr.IsntError())
	{
		// Now dispatch the command.
		ACI.op		= (removeAll ? 
			kAppOp_RemoveAlternates : kAppOp_RemovePassphrase);
		ACI.flags	= NULL;
		ACI.drive	= kInvalidDrive;
		strcpy(ACI.path, path);

		DispatchAppCommandInfo(&ACI);
	}

	if (derr.IsError())
	{
		if (removeAll)
			ReportError(kPGDMajorError_PGPdiskRemoveAllPassesFailed, derr);
		else
			ReportError(kPGDMajorError_PGPdiskRemoveOnePassFailed, derr);
	}

	// Re-enable main dialog if it was disabled.
	if (IsntNull(mMainDialog) && wasMainEnabled)
		mMainDialog->EnableWindow(TRUE);
}

// AskUserAddRemovePublicKeys asks the user for a PGPdisk from which to add
// or remove public keys.

void 
CPGPdiskApp::AskUserAddRemovePublicKeys()
{
	AppCommandInfo		ACI;
	COpenPGPdiskDialog	selectPGPdiskDlg(kODT_Select);
	CString				path;
	DualErr				derr;
	PGPBoolean			wasMainEnabled;

	// Forcibly disable main window so it doesn't flicker between dialogs.
	if (mMainDialog)
		wasMainEnabled = !mMainDialog->EnableWindow(FALSE);

	// Show the dialog.
	derr = selectPGPdiskDlg.AskForPath(&path);

	if (derr.IsntError())
	{
		// Now dispatch the command.
		ACI.op		= kAppOp_AddRemovePublicKeys;
		ACI.flags	= NULL;
		ACI.drive	= kInvalidDrive;
		strcpy(ACI.path, path);

		DispatchAppCommandInfo(&ACI);
	}

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_AddRemovePublicKeysFailed, derr);
	}

	// Re-enable main dialog if it was disabled.
	if (IsntNull(mMainDialog) && wasMainEnabled)
		mMainDialog->EnableWindow(TRUE);
}


//////////////////////////////////////////////////////////////
// Low-Level Passphrase Addition, Change, and Removal Routines
//////////////////////////////////////////////////////////////

// AddPGPdiskPassphrase adds a new passphrase to the specified PGPdisk.

	DualErr 
CPGPdiskApp::AddPGPdiskPassphrase(LPCSTR path)
{
	CConfirmPassphraseDialog	addPassDlg(kCPDT_Add);
	CSinglePassphraseDialog		masterPassDlg(kSPDT_Master);
	DualErr						derr;
	PGPUInt32					numAlts;

	pgpAssertStrValid(path);

	// Did our dialogs initialize correctly?
	derr = addPassDlg.mInitErr;

	if (derr.IsntError())
	{
		derr = masterPassDlg.mInitErr;
	}
	
	// Make sure this PGPdisk isn't mounted already.
	if (derr.IsntError())
	{
		if (mPGPdisks.FindPGPdisk(path))
		{
			ReportError(kPGDMajorError_TriedPassOpOnMountedPGPdisk);
			derr = DualErr(kPGDMinorError_FailSilently);
		}
	}

	// Validate the PGPdisk.
	if (derr.IsntError())
	{
		derr = ValidatePGPdisk(path);
	}
	
	// Can't add passphrases to read-only PGPdisks.
	if (derr.IsntError())
	{
		if (IsFileReadOnly(path))
			derr = DualErr(kPGDMinorError_PGPdiskIsWriteProtected);
	}

	// Display the master passphrase dialog.
	if (derr.IsntError())
	{
		derr = masterPassDlg.AskForPassphrase(path);
	}

	// Verify that there is space for another passphrase.
	if (derr.IsntError())
	{
		derr = HowManyAlternatePassphrases(path, &numAlts);
	}

	if (derr.IsntError())
	{
		if (numAlts == kMaxAlternatePassphrases)
		{
			derr = DualErr(kPGDMinorError_AllAlternatesTaken);
		}
	}

	// Display the new passphrase dialog.
	if (derr.IsntError())
	{
		derr = addPassDlg.AskForPassphrase(path);
	}

	if (derr.IsntError())
	{
		// Add the passphrase.
		derr = AddPassphrase(path, 
			masterPassDlg.mPassphraseEdit.mContents, 
			addPassDlg.mPassphraseEdit.mContents, 
			addPassDlg.mReadOnlyValue);
	}

	return derr;
}

// ChangePGPdiskPassphrase changes a passphrase on the specified PGPdisk.

DualErr 
CPGPdiskApp::ChangePGPdiskPassphrase(LPCSTR path)
{
	CConfirmPassphraseDialog	newPassDlg(kCPDT_Change);
	CSinglePassphraseDialog		changePassDlg(kSPDT_Change);
	DualErr						derr;

	pgpAssertStrValid(path);

	// Make sure our passphrase dialog initialized correctly.
	derr = newPassDlg.mInitErr;

	if (derr.IsntError())
	{
		derr = changePassDlg.mInitErr;
	}

	// Make sure this PGPdisk isn't mounted already.
	if (derr.IsntError())
	{
		if (mPGPdisks.FindPGPdisk(path))
		{
			ReportError(kPGDMajorError_TriedPassOpOnMountedPGPdisk);
			derr = DualErr(kPGDMinorError_FailSilently);
		}
	}

	// Validate the PGPdisk.
	if (derr.IsntError())
	{
		derr = ValidatePGPdisk(path);
	}
	
	// Can't change passphrases on read-only PGPdisks.
	if (derr.IsntError())
	{
		if (IsFileReadOnly(path))
			derr = DualErr(kPGDMinorError_PGPdiskIsWriteProtected);
	}

	// Display the dialogs.
	if (derr.IsntError())
	{
		derr = changePassDlg.AskForPassphrase(path);
	}

	if (derr.IsntError())
	{
		derr = newPassDlg.AskForPassphrase(path);
	}
	
	if (derr.IsntError())
	{
		// Change the passphrase.
		derr = ChangePassphrase(path, 
			changePassDlg.mPassphraseEdit.mContents, 
			newPassDlg.mPassphraseEdit.mContents);
	}

	return derr;
}

// RemovePGPdiskPassphrase removes a passphrase from the specified PGPdisk.

DualErr 
CPGPdiskApp::RemovePGPdiskPassphrase(LPCSTR path, PGPBoolean removeAll)
{
	CSinglePassphraseDialog	removePassDlg(kSPDT_Remove);
	DualErr					derr;

	pgpAssertStrValid(path);

	// Did our dialog initialize correctly?
	derr = removePassDlg.mInitErr;

	// Make sure this PGPdisk isn't mounted already.
	if (derr.IsntError())
	{
		if (mPGPdisks.FindPGPdisk(path))
		{
			ReportError(kPGDMajorError_TriedPassOpOnMountedPGPdisk);
			derr = DualErr(kPGDMinorError_FailSilently);
		}
	}

	// Validate the PGPdisk.
	if (derr.IsntError())
	{
		derr = ValidatePGPdisk(path);
	}

	// Can't remove passphrases from read-only PGPdisks.
	if (derr.IsntError())
	{
		if (IsFileReadOnly(path))
			derr = DualErr(kPGDMinorError_PGPdiskIsWriteProtected);
	}

	// Display the dialog.
	if (derr.IsntError())
	{
		if (removeAll)
		{
			PGPUInt32 numAlts;

			// Verify that there are alternates to remove.
			derr = HowManyAlternatePassphrases(path, &numAlts);

			if (derr.IsntError())
			{
				if (numAlts == 0)
					derr = DualErr(kPGDMinorError_NoAlternatesToRemove);
			}

			// Confirm the removal.
			if (derr.IsntError())
			{
				if (DisplayMessage(kPGPdiskVerifyRemoveAlternates, 
					kPMBS_YesNo, kPMBF_YesButton) == kUR_Yes)
				{
					// Remove all alternate passphrases.
					derr = RemoveAlternatePassphrases(path);

					if (derr.IsntError())
					{
						DisplayMessage(kPGPdiskRemoveAltsSuccessString);
					}
				}
			}
		}
		else
		{
			derr = removePassDlg.AskForPassphrase(path);

			if (derr.IsntError())
			{
				// Remove one passphrase.
				derr = RemovePassphrase(path, 
					removePassDlg.mPassphraseEdit.mContents);
			}
		}
	}

	return derr;
}

// AddRemovePGPdiskPublicKeys displays a dialog allowing users to add or
// remove keys from a PGPdisk.

DualErr 
CPGPdiskApp::AddRemovePGPdiskPublicKeys(LPCSTR path)
{
	CPublicKeyDialog			publicKeyDialog;
	CSinglePassphraseDialog		masterPassDlg(kSPDT_Master);
	DualErr						derr;
	PGPBoolean					gotAllKeys, gotExistingKeys;
	PGPKeySetRef				allKeys;
	PGPRecipientSpec			*existRecipKeys;
	PGPUInt32					numRecipKeys;

	gotAllKeys = gotExistingKeys = FALSE;

	pgpAssertStrValid(path);

	// Did our passphrase dialog initialize correctly?
	derr = masterPassDlg.mInitErr;

	// Make sure this PGPdisk isn't mounted already.
	if (mPGPdisks.FindPGPdisk(path))
	{
		ReportError(kPGDMajorError_TriedPassOpOnMountedPGPdisk);
		derr = DualErr(kPGDMinorError_FailSilently);
	}

	// Validate the PGPdisk.
	if (derr.IsntError())
	{
		derr = ValidatePGPdisk(path);
	}

	// Can't alter passphrases on read-only PGPdisks.
	if (derr.IsntError())
	{
		if (IsFileReadOnly(path))
			derr = DualErr(kPGDMinorError_PGPdiskIsWriteProtected);
	}

	// Display the master passphrase dialog.
	if (derr.IsntError())
	{
		derr = masterPassDlg.AskForPassphrase(path);
	}

	// Load SDK prefs.
	if (derr.IsntError())
	{
		derr = PGPsdkLoadDefaultPrefs(GetGlobalPGPContext());
	}

	// Open default key rings.
	if (derr.IsntError())
	{
		derr = PGPOpenDefaultKeyRings(GetGlobalPGPContext(), 0, &allKeys);
		gotAllKeys = derr.IsntError();
	}

	// Retrieve already existing keys from the PGPdisk.
	if (derr.IsntError())
	{
		derr = GetPGPdiskRecipKeys(path, allKeys, &existRecipKeys, 
			&numRecipKeys);

		gotExistingKeys = derr.IsntError();
	}

	// Display the dialog.
	if (derr.IsntError())
	{
		derr = publicKeyDialog.AskForKeys(allKeys, existRecipKeys, 
			numRecipKeys);
	}

	// Alter keys on the PGPdisk.
	if (derr.IsntError())
	{
		derr = UpdatePGPdiskPublicKeys(path, 
			masterPassDlg.mPassphraseEdit.mContents, allKeys, 
			publicKeyDialog.mNewRecipKeys, publicKeyDialog.mNumNewKeys, 
			publicKeyDialog.mNewKeysReadOnly);
	}

	if (gotExistingKeys)
		FreeRecipientSpecList(existRecipKeys, numRecipKeys);

	if (gotAllKeys)
		PGPFreeKeySet(allKeys);

	publicKeyDialog.CleanUp();

	return derr;
}

#if PGPDISK_WIPE_FUNCS

// WipePassesOnAllPGPdisks wipes the master and alternate passphrases on all
// mounted PGPdisks (unmounting them in the process).

DualErr 
CPGPdiskApp::WipePassesOnAllPGPdisks(PGPBoolean doSilentWiping)
{
	DualErr derr, storedDerr;

	try
	{
		CString		path;
		PGPdisk		*pPGD;
		PGPUInt32	i	= 0;

		// Confirm wipe.
		if (!doSilentWiping && 
			(DisplayMessage(kPGPdiskConfirmWipeAllPasses, kPMBS_YesNo, 
				kPMBF_NoButton) == kUR_No))
		{
			derr = DualErr(kPGDMinorError_UserAbort);
		}

		// For each mounted PGPdisk...
		while (derr.IsntError() && 
			IsntNull(pPGD = mPGPdisks.EnumPGPdisks(i)))
		{
			PGPBoolean unmountedDisk = FALSE;

			path = pPGD->GetPath();

			// Emergency unmount it.
			if (pPGD->Mounted())
				derr = UnmountPGPdisk(pPGD->GetDrive(), TRUE);

			unmountedDisk = derr.IsntError();

			// If it has a public key, wipe the master and alternate passes.
			if (derr.IsntError())
			{
				if (DoesPGPdiskHavePublicKey(path))
					derr = WipeMasterAndAlternatePasses(path);
			}

			if (derr.IsError())
			{
				// Store the first error we see.
				if (storedDerr.IsntError())
					storedDerr = derr;

				// Clear error so we can keep going.
				derr = DualErr::NoError;

				// Increment enumerator on unmount failure.
				if (!unmountedDisk)
					i++;
			}
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// WipePassesOnThisPGPdisk wipes the master and alternate passphrases on a
// single PGPdisk (unmounting it if mounted).

DualErr 
CPGPdiskApp::WipePassesOnThisPGPdisk(LPCSTR path, PGPBoolean doSilentWiping)
{
	DualErr derr;

	pgpAssertStrValid(path);

	try
	{
		CString csPath = path;

		// Is this an actual PGPdisk file?
		derr = ValidatePGPdisk(csPath);

		// Confirm wipe.
		if (derr.IsntError())
		{
			if (!doSilentWiping && 
				(DisplayMessage(kPGPdiskConfirmWipeIndPasses, kPMBS_YesNo, 
					kPMBF_NoButton) == kUR_No))
			{
				derr = DualErr(kPGDMinorError_UserAbort);
			}
		}

		if (derr.IsntError())
		{
			PGPdisk	*pPGD;

			pPGD = mPGPdisks.FindPGPdisk(csPath);

			// If mounted, emergency unmount it.
			if (IsntNull(pPGD) && pPGD->Mounted())
				derr = UnmountPGPdisk(pPGD->GetDrive(), TRUE);

			// If it has a public key, wipe the master and alternate passes.
			if (derr.IsntError())
			{
				if (DoesPGPdiskHavePublicKey(path))
					derr = WipeMasterAndAlternatePasses(path);
			}
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

#endif // PGPDISK_WIPE_FUNCS


/////////////////////////////////////
// Miscellaneous passphrase functions
/////////////////////////////////////

// CalcPassphraseQuality computes the "quality" of a passphrase in the range
// 0 - 100. Based on the function from the PGP 5.5 source code, rewritten to
// work with SecureStrings.

PGPUInt16 
CPGPdiskApp::CalcPassphraseQuality(SecureString* passphrase)
{	
	PGPUInt32		quality;
	PGPUInt16		charIndex, rangeIndex, span;

	static Range	sRanges[]	=
					{
						{ 0,		' ' - 1},
						{ ' ',		'A' - 1},
						{ 'A',		'Z'},
						{ 'Z' + 1,	'a' - 1},
						{ 'a',		'z'},
						{ '0',		'9'},
						{ 'z' + 1,	127},
						{ 128,		255},
					};

	const PGPUInt16	kNumRanges	= sizeof(sRanges)/sizeof(sRanges[0]);
	PGPBoolean		haveRange[kNumRanges];
	
	pgpAssertAddrValid(passphrase, SecureString);
	
	pgpClearMemory(haveRange, sizeof(haveRange));
	for (charIndex = 1; charIndex < passphrase->GetLength(); ++charIndex)
	{
		for (rangeIndex = 0; rangeIndex < kNumRanges; ++rangeIndex)
		{
			PGPUInt8	theChar;
			Range*		theRange;
			
			theChar = passphrase->GetAt(charIndex);
			theRange = &sRanges[rangeIndex];
			
			if (theChar >= theRange->firstChar && 
				theChar <= theRange->lastChar) 
			{
				haveRange[rangeIndex] = TRUE;
				break;
			}
		}
	}
	
	// Determine the total span.
	span = 0;
	for (rangeIndex = 0; rangeIndex < kNumRanges; ++rangeIndex)
	{
		if (haveRange[rangeIndex])
		{
			span += 1 + (sRanges[rangeIndex].lastChar - 
				sRanges[rangeIndex].firstChar);
		}
	}
	
	if (span != 0)
	{
		double		bitsPerChar;
		PGPUInt32	totalBits;
			
		bitsPerChar = log (span) / log (2.0);  // log2 of span
		totalBits = (PGPUInt32) (bitsPerChar * passphrase->GetLength());
		
		// Assume a maximum quality of 128 bits.
		quality = (totalBits * 100) / 128;
		if (quality > 100) quality = 100;
	}
	else
	{
		quality	= 0;
	}

	return quality;
}

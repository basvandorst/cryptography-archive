//////////////////////////////////////////////////////////////////////////////
// CommonStrings.cpp
//
// All PGPdisk project strings are defined here.
//////////////////////////////////////////////////////////////////////////////

// $Id: CommonStrings.cpp,v 1.1.2.12.2.5 1998/10/30 22:49:18 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "CommonStrings.h"
#include "Errors.h"


///////////////////////////
// String association array
///////////////////////////

LPCSTR kPGPdiskCommonStrings[kPGPdiskMaxCommonStrings] =
{
	// kPGPdiskBadCmdLineString
	{
		"Unrecognized command line option or command."
	}, 

	// kPGPdiskCmdLineHelpString
	{
		"Commands: create, mount, open unmount, unmountall, addpass, " 
		"changepass, removepass, removealts, addrempubkeys, about, help, "
		"prefs.\n\nOptions: -s: silent mode"
	},

	// kPGPdiskBecauseString
	{
		"because "
	}, 

	// kPGPdiskNoMemForErrorString
	{
		"PGPdisk is running out of memory - please quit now."
	}, 

	// kPGPdiskRemoveAltsSuccessString
	{
		"All alternate passphrases were successfully removed."
	}, 

	// kPGPdiskNoPGPdisksToUnmountString
	{
		"There are no PGPdisks currently mounted."
	}, 

	// kPGPdiskLicenseToText
	{
		"Licensed To:"
	}, 

	// kPGPdiskDemoTimeoutWarning
	{
		"This demo version of PGPdisk has expired. From now on, you will "
		"only be able to mount your PGPdisks in read-only mode. For "
		"instructions on how to purchase a fully-enabled copy of PGPdisk, "
		"click on the link below."
	}, 

	// kPGPdiskDemoBadCommandWarning
	{
		"This command is unavailable becaue your demo copy of PGPdisk has " 
		"expired. For instructions on how to purchase a fully-enabled copy "
		"of PGPdisk, click on the button below."
	}, 

	// kPGPdiskURL
	{
		"http://www.pgpi.com/"
	}, 

	// kPGPdiskBuyNowURL
	{
		"http://www.pgpi.com/"
	}, 

	// kPGPdiskCPDTAddPassMsg
	{
		"Please enter an additional passphrase for \"%s\"."
	}, 

	// kPGPdiskCPDTChangePassMsg
	{
		"Please enter a new passphrase for \"%s\"."
	}, 

	// kPGPdiskSDTCreateDialogTitle
	{
		"New PGPdisk"
	}, 

	// kPGPdiskSDTSaveFilter
	{
		"PGPdisk Files (*.pgd)||"
	}, 

	// kPGPdiskSPDTChangePassMsg
	{
		"Please enter the passphrase to change for \"%s\"."
	}, 

	// kPGPdiskSPDTMasterPassMsg
	{
		"Please enter the master passphrase for \"%s\"."
	}, 

	// kPGPdiskSPDTMountPassMsg
	{
		"Please enter a passphrase for \"%s\"."
	}, 

	// kPGPdiskSPDTPublicKeyPassMsg
	{
		"Please enter a public key passphrase for \"%s\"."
	}, 

	// kPGPdiskSPDTRemovePassMsg
	{
		"Please enter the passphrase to remove from \"%s\"."
	}, 

	// kPGPdiskPrefsSheetTitleString
	{
		"Preferences"
	}, 

	// kPGPdiskDiskWizSheetTitleString
	{
		"New PGPdisk Wizard"
	}, 

	// kPGPdiskCreationCreateMsgString
	{
		"Creating: \"%s\""
	}, 

	// kPGPdiskCreationDoneMsgString
	{
		"Finished: \"%s\""
	}, 


	// kPGPdiskFreeSize1MsgString
	{
		"You have %s of free space available. Note that the actual amount "
		"of free space on the PGPdisk volume you create will be somewhat "
		"smaller than the figure you choose below."
	}, 

	// kPGPdiskFreeSize2MsgString
	{
		"You have %s of free space available. Note that the actual amount "
		"of free space on the PGPdisk volume you create will be somewhat "
		"smaller than the figure you choose below."
	}, 

	// kPGPdiskFreeSize3MsgString
	{
		"PGPdisk was unable to determine the amount of available free "
		"space. Note that the actual amount of free space on the PGPdisk "
		"volume you create will be somewhat smaller than the figure you "
		"choose below."
	}, 

	// kPGPdiskOpenCmdString
	{
		"open"
	}, 

	// kPGPdiskGlobalConvertCmdString
	{
		"globalconvert"
	}, 

	// kPGPdiskCreateCmdString
	{
		"create"
	}, 

	// kPGPdiskMountCmdString
	{
		"mount"
	}, 

	// kPGPdiskUnmountCmdString
	{
		"unmount"
	}, 

	// kPGPdiskUnmountAllCmdString
	{
		"unmountall"
	}, 

	// kPGPdiskAddPassCmdString
	{
		"addpass"
	}, 

	// kPGPdiskChangePassCmdString
	{
		"changepass"
	}, 

	// kPGPdiskRemovePassCmdString
	{
		"removepass"
	}, 

	// kPGPdiskRemoveAltsCmdString
	{
		"removealts"
	}, 

	// kPGPdiskPubKeyCmdString
	{
		"addrempubkeys"
	}, 

	// kPGPdiskWipeAllCmdString
	{
		"disableallmounted"
	},

	// kPGPdiskWipeIndCmdString
	{
		"disablethisdisk"
	},

	// kPGPdiskAboutCmdString
	{
		"about"
	}, 

	// kPGPdiskHelpCmdString
	{
		"help"
	}, 

	// kPGPdiskPrefsCmdString
	{
		"prefs"
	}, 

	// kPGPdiskShellExtOpenMenuString
	{
		"&Open PGPdisks..."
	}, 
	
	// kPGPdiskShellExtMountMenuString
	{
		"&Mount PGPdisk..."
	}, 

	// kPGPdiskShellExtUnmountMenuString
	{
		"&Unmount PGPdisk"
	}, 

	// kPGPdiskShellExtAddPassMenuString
	{
		"&Add Passphrase..."
	}, 

	// kPGPdiskShellExtChangePassMenuString
	{
		"&Change Passphrase..."
	}, 

	// kPGPdiskShellExtRemovePassMenuString
	{
		"&Remove Passphrase..."
	}, 

	// kPGPdiskShellExtRemoveAltsMenuString
	{
		"&Remove Alternate Passphrases..."
	}, 

	// kPGPdiskShellExtPubKeysMenuString
	{
		"Add/Remove Public &Keys..."
	}, 

	// kPGPdiskShellExtHelpMenuString
	{
		"&Help..."
	}, 

	// kPGPdiskShellExtDescribeOpenString, 
	{
		"Mounts or explores the selected PGPdisks"
	}, 

	// kPGPdiskShellExtDescribeMountString, 
	{
		"Mounts an unmounted PGPdisk"
	}, 

	// kPGPdiskShellExtDescribeUnmountString, 
	{
		"Unmounts a mounted PGPdisk"
	}, 

	// kPGPdiskShellExtDescribeAddPassString, 
	{
		"Adds a passphrase to an unmounted PGPdisk"
	}, 

	// kPGPdiskShellExtDescribeChangePassString, 
	{
		"Changes a passphrase on an unmounted PGPdisk"
	}, 

	// kPGPdiskShellExtDescribeRemovePassString, 
	{
		"Remove all alternate passphrases from an unmounted PGPdisk"
	}, 

	// kPGPdiskShellExtDescribeRemoveAltsString, 
	{
		"Removes a passphrase from an unmounted PGPdisk"
	}, 

	// kPGPdiskShellExtDescribePubKeysString, 
	{
		"Adds and removes public keys to or from an unmounted PGPdisk"
	}, 

	// kPGPdiskShellExtDescribeHelpString
	{
		"Displays the PGPdisk help file"
	}, 

	// kPGPdiskBetaTimeOutWarning
	{
		"This beta version of PGPdisk has expired. From now on, you will "
		"only be able to mount your PGPdisks in read-only mode."
	}, 

	// kPGPdiskVerifyRemoveAlternates
	{
		"Are you sure you want to remove all passphrases from this PGPdisk? "
		"If you choose 'Yes', only the master passphrase will be able to "
		"mount this PGPdisk."
	}, 

	// kPGPdiskNotifyMountMaybeCorrupt
	{
		"This PGPdisk was either unmounted improperly or was created by "
		"copying an already mounted PGPdisk file, and may be damaged and/or "
		"out-of-date. You should check it with a disk repair utility at the "
		"first opportunity."
	}, 

	// kPGPdiskConfirmFormat
	{
		"The PGPdisk does not appear to be formatted. Do you want to format "
		"it now?"
	}, 

	// kPGPdiskAskToSavePrefs
	{
		"You have made changes that have not been applied. Do you want to "
		"apply them now?"
	}, 

	// kPGPdiskPGPdiskIsNotEncrypted
	{
		"This PGPdisk was created using a demo version of PGPdisk and is "
		"not encrypted. For security reasons, you will only be able to "
		"mount it in read-only mode."
	}, 

	// kPGPdiskConfirmCancelFormat
	{
		"Your new PGPdisk volume still hasn't been formatted. Do you want "
		"to attempt formatting again?"
	}, 

	// kPGPdiskResConfirmPGPdiskResQuit
	{
		"Are you sure you want to stop the PGPdiskResident application? If "
		"you do so, important PGPdisk features such as the unmount hotkey "
		"will not work."
	}, 

	// kPGPdiskConfirmCancelCreation
	{
		"Are you sure you want to cancel creating your new PGPdisk?"
	}, 

	// kPGPdiskForceROOnLoopbackString
	{
		"You are attempting to mount a PGPdisk via a network path that "
		"points to a directory on the local machine. Since PGPdisk cannot "
		"fully resolve this path on non-Administrator accounts, you will be "
		"forced to mount this PGPdisk in read-only mode."
	}, 

	// kPGPdiskErrorWasString
	{
		"The error was \"%s.\""
	}, 

	// kPGPdiskConfirmWipeAllPasses
	{
		"Are you sure you want to wipe the master and alternate passphrases "
		"of all mounted PGPdisks? Only PGPdisks with public key passphrases "
		"will be affected, and those public key passphrases will be left "
		"alone."
	}, 

	// kPGPdiskConfirmWipeIndPasses
	{
		"Are you sure you want to wipe the master and alternate passphrases "
		"of the specified PGPdisk? Only PGPdisks with public key passphrases "
		"will be affected, and those public key passphrases will be left "
		"alone."
	}, 

	// kPGPdiskBadCASTAskConvertPNM
	{
		"This PGPdisk was created with an earlier version of PGPdisk and "
		"may not be secure. Do you wish to convert the PGPdisk to the newer "
		"format? This may take several minutes, and if you choose \"No\", "
		"you will not be allowed to modify files contained in the PGPdisk."
	}, 

	// kPGPdiskBadCASTAskContinuePNM
	{
		"This PGPdisk was being converted to the new, more secure format "
		"when the process was interrupted. Do you wish to continue the "
		"conversion? You must complete the conversion in order to access "
		"the PGPdisk."
	}, 

	// kPGPdiskBadCASTAskConvertPM
	{
		"This PGPdisk was created with an earlier version of PGPdisk and "
		"may not be secure. Do you wish to convert the PGPdisk to the newer "
		"format? This may take several minutes, and if you choose \"No\", "
		"you will not be allowed to modify files contained in the PGPdisk."
	}, 

	// kPGPdiskBadCASTAskContinuePM
	{
		"This PGPdisk was being converted to the new, more secure format "
		"when the process was interrupted. Do you wish to continue the "
		"conversion? You must complete the conversion in order to access "
		"the PGPdisk."
	}, 

	// kPGPdiskBadCASTReadOnly
	{
		"This PGPdisk was created with an earlier version of PGPdisk and "
		"may not be secure. Since the specified PGPdisk file is read-only, "
		"it cannot be converted to the newer format at the current time. "
		"Please mark it read/write or copy its contents to a new PGPdisk as "
		"soon as possible."
	}, 

	// kPGPdiskConvertWizSheetTitleString
	{
		"PGPdisk Conversion Wizard"
	}, 

	// kPGPdiskConversionConvertMsgString
	{
		"Converting: \"%s\""
	}, 

	// kPGPdiskConversionDoneMsgString
	{
		"Finished: \"%s\""
	}, 

	// kPGPdiskConfirmCancelConversion
	{
		"Are you sure you want to cancel the PGPdisk conversion process? "
		"You won't be able to access the PGPdisk again until you allow "
		"PGPdisk to finish the conversion."
	}, 

	// kPGPdiskGlobalCWizSheetTitleString
	{
		"PGPdisk Global Conversion Wizard"
	}, 

	// kPGPdiskConfirmCancelSearching
	{
		"Are you sure you want to cancel the PGPdisk global search and "
		"conversion process?"
	}, 

	// kPGPdiskSearchingSearchMsgString
	{
		"Searching:"
	}, 

	// kPGPdiskSearchingDoneMsgString
	{
		"Done searching."
	}, 

	// kPGPdiskConversionWipedMsgString
	{
		"This PGPdisk has previously had its master and alternate "
		"passphrases wiped. In order to proceed with conversion, you must "
		"enter the passphrase for one of the public keys left on the "
		"PGPdisk. The conversion process will restore the PGPdisk to "
		"ordinary functionality, with a new master passphrase identical to "
		"the passphrase you enter for the public key. All public keys on "
		"the PGPdisk (except for ADKs) will be removed."
	}

};

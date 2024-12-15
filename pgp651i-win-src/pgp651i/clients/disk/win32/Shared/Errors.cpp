//////////////////////////////////////////////////////////////////////////////
// Errors.cpp
//
// Defines error strings.
//////////////////////////////////////////////////////////////////////////////

// $Id: Errors.cpp,v 1.6 1999/03/31 23:51:09 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>

#elif defined(PGPDISK_NTDRIVER)

#define	__w64
#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "Errors.h"


////////////////////////////
// String association arrays
////////////////////////////

LPCSTR kPGPdiskMajorErrorStrings[kPGPdiskMaxMajorErrorStrings] =
{
	// kPGDMajorError_NoErr
	{
		"No error occured."
	}, 

	// kPGDMajorError_AppInitFailed
	{
		"PGPdisk was unable to start %s."
	}, 

	// kPGDMajorError_PGPdiskCreationFailed
	{
		"PGPdisk was unable to create a new PGPdisk %s."
	}, 

	// kPGDMajorError_PGPdiskMountFailed
	{
		"This PGPdisk could not be mounted %s."
	}, 

	// kPGDMajorError_PGPdiskTooBig
	{
		"PGPdisks cannot be larger than 2048 MB (Win95) or 1024 GB (NT) in "
		"size."
	}, 

	// kPGDMajorError_PGPdiskTooSmall
	{
		"PGPdisks cannot be smaller than 100k in size."
	}, 

	// kPGDMajorError_PGPdiskUnmountFailed
	{
		"The PGPdisk %c:\\ could not be unmounted %s."
	}, 

	// kPGDMajorError_PGPdiskReadError
	{
		"A read operation on the PGPdisk %c:\\ has failed %s. Do you wish "
		"to attempt an emergency unmount of the PGPdisk?"
	}, 

	// kPGDMajorError_PGPdiskWriteError
	{
		"A write operation on the PGPdisk %c:\\ has failed %s. Do you wish "
		"to attempt an emergency unmount of the PGPdisk?"
	}, 

	// kPGDMajorError_AboutBoxDisplayFailed
	{
		"PGPdisk was unable to display the about box %s."
	}, 

	// kPGDMajorError_PreferencesAccessFailed
	{
		"An attempt to access the PGPDisk preferences failed %s."
	}, 

	// kPGDMajorError_TimeoutValueTooSmall
	{
		"The inactivity timeout value must be greater than 1 minute."
	}, 

	// kPGDMajorError_TimeoutValueTooBig
	{
		"The inactivity timeout value must be less than 999 minutes."
	}, 

	// kPGDMajorError_NotEnoughDiskSpace
	{
		"There is not enough free disk space to create a PGPdisk of the "
		"size you specified. Please enter a smaller size."
	}, 

	// kPGDMajorError_PassphraseTooSmall
	{
		"Your passphrase must be at least 8 characters in length. Please "
		"choose a longer passphrase and try again."
	}, 

	// kPGDMajorError_PassphraseNotConfirmed
	{
		"The passphrase you entered does not match the phrase you typed "
		"into the confirmation field. Please correct this and try again."
	}, 

	// kPGDMajorError_PGPdiskAddPassFailed
	{
		"PGPdisk was unable to add another passphrase %s."
	}, 

	// kPGDMajorError_PGPdiskChangePassFailed
	{
		"PGPdisk was unable to change your passphrase %s."
	}, 

	// kPGDMajorError_PGPdiskRemoveOnePassFailed
	{
		"PGPdisk was unable to remove your passphrase %s."
	}, 

	// kPGDMajorError_IncorrectPassphrase
	{
		"The passphrase you entered is not a valid master or alternate "
		"passphrase for this PGPdisk. Please try again."
	}, 

	// kPGDMajorError_IncorrectMasterPassphrase
	{
		"The passphrase you entered is not a valid master passphrase for "
		"this PGPdisk. Please try again."
	}, 

	// kPGDMajorError_TriedRemoveMasterPass
	{
		"You cannot remove the master passphrase from a PGPdisk. Please "
		"enter an alternate passphrase instead."
	}, 

	// kPGDMajorError_IncorrectAlternatePassphrase
	{
		"The passphrase you entered is not a valid alternate passphrase for "
		"this PGPdisk. Please try again."
	}, 

	// kPGDMajorError_PassphraseAlreadyExists
	{
		"That passphrase already exists. Please try again."
	}, 

	// kPGDMajorError_NotCompatibleWindows
	{
		"PGPdisk could not be started because it is only compatible with "
		"Windows 95/98 and Windows NT 4.0 or later."
	}, 

	// kPGDMajorError_TriedPassOpOnMountedPGPdisk
	{
		"You must first unmount the selected PGPdisk before adding, "
		"changing or removing a passphrase or public key."
	}, 

	// kPGDMajorError_InvalidCipherContext
	{
		"The PGPdisk mounted as drive %c:\\ has been emergency unmounted "
		"%s. It may corrupted - please backup all files on this PGPdisk as "
		"soon as possible."
	}, 

	// kPGDMajorError_DriverUpdatePrefsFailed
	{
		"The PGPdisk application was unable to communicate with the PGPdisk "
		"driver %s. Please restart your computer and reinstall PGPdisk."
	}, 

	// kPGDMajorError_DialogDisplayFailed
	{
		"PGPdisk was unable to display a dialog box %s."
	}, 

	// kPGDMajorError_ChoseReadOnlyDirInCreate
	{
		"The directory you have chosen is write-protected. Please choose "
		"another."
	}, 

	// kPGDMajorError_ChoseTooSmallDirInCreate
	{
		"The directory you have chosen does not have the minimum required "
		"amount of free space (about 100k). Please choose another."
	}, 

	// kPGDMajorError_ChoseInvalidDirInCreate
	{
		"The directory you have chosen does not exist. Please choose "
		"another."
	}, 

	// kPGDMajorError_NoSleepOnUnmountFailure
	{
		"Computer sleep has been prevented because PGPdisk could not "
		"unmount one or more of its volumes. You can disable this check by "
		"deselecting the appropriate option from within the PGPdisk "
		"preferences."
	}, 

	// kPGDMajorError_BadCommandAfterBetaTimeout
	{
		"This command is unavailable because your beta copy of PGPdisk has "
		"expired."
	}, 

	// kPGDMajorError_ChoseReplaceFileInUse
	{
		"This file is in use by another program and cannot be replaced."
	}, 

	// kPGDMajorError_PGPdiskRemoveAllPassesFailed
	{
		"PGPdisk was unable to remove all of your passphrases %s."
	}, 

	// kPGDMajorError_PGPdiskUnmountAllFailed
	{
		"Some PGPdisks could not be unmounted %s."
	}, 

	// kPGDMajorError_InvalidVolumeName
	{
		"A PGPdisk volume name cannot contain any of the following "
		"characters: \"&*+./:;<=>?[\\]^|"
	}, 

	// kPGDMajorError_WindowsFormatFailed
	{
		"PGPdisk encountered an error when trying to display the standard "
		"Windows formatting dialog. PGPdisk will now attempt to format your "
		"new PGPdisk volume using its own algorithm; it is suggested that "
		"you reformat from the Windows Explorer as soon as possible."
	}, 

	// kPGDMajorError_ResidentAppInitFailed
	{
		"The background application \"PGPdiskResident.exe\" could not be "
		"started %s. Important PGPdisk functions may not work as a result."
	}, 

	// kPGDMajorError_PGPdiskResHotKeyOpFailed
	{
		"PGPdisk couldn't enable or disable the PGPdisk unmount hotkey. "
		"The key combination you have chosen may not be compatible with "
		"your system, or may be conflicting with another application. "
		"You should choose another hotkey, quit all other applications, or "
		"reboot your system."
	}, 

	// kPGDMajorError_PGPdiskIrpCleanupError
	{
		"PGPdisk couldn't disable IO to the PGPdisk drive %c:\\ %s"
	}, 

	// kPGDMajorError_CouldntFindAppString
	{
		"Your request could not be completed because the PGPdisk "
		"application could not be found. Please run PGPdisk once to correct "
		"this."
	}, 

	// kPGDMajorError_KModeExceptionOccured
	{
		"An internal error was encountered within the PGPdisk driver %s."
	}, 

	// kPGDMajorError_CouldntFindResidentApp
	{
		"The background application \"PGPdiskResident.exe\" could not be "
		"found. This application must always be in the same directory as "
		"\"PGPdisk.exe\". Important PGPdisk functions may not work as a "
		"result."
	}, 

	// kPGDMajorError_AddRemovePublicKeysFailed
	{
		"PGPdisk was unable to read and/or modify the public keys "
		"associated with the specified PGPdisk %s."
	}, 

	// kPGDMajorError_IncPassAndKeyNotInRing
	{
		"The passphrase you entered is not a valid master or alternate "
		"passphrase for this PGPdisk. (Note that there were public key "
		"passphrases on this PGPdisk that could not be checked, either "
		"because they aren't on the local keyring or because the local "
		"keyring could not be opened.) Please try again."
	}, 

	// kPGDMajorError_PGPdiskPassesWiped
	{
		"This PGPdisk has had its master and alternate passphrases wiped. "
		"It can now only be mounted by the holder of the private key "
		"associated with one or more of the public keys on the PGPdisk. You "
		"will now be shown a dialog containing a read-only list of these "
		"public keys (bottom pane)."
	}, 

	// kPGDMajorError_WipePassesFailed
	{
		"PGPdisk was unable to wipe the master and alternate passphrases on "
		"one or more PGPdisks %s."
	}, 

	// kPGDMajorError_PGPdiskConversionFailed
	{
		"PGPdisk was unable to convert a PGPdisk to the new format %s."
	}, 

	// kPGDMajorError_PGPdiskGlobalConversionFailed
	{
		"PGPdisk was unable to invoke the global PGPdisk conversion "
		"wizard %s."
	}, 

	// kPGDMajorError_IncorrectPublicKeyPassphrase
	{
		"The passphrase you entered is not a valid public key passphrase "
		"for this PGPdisk. Please try again."
	}

};

LPCSTR kPGPdiskMinorErrorStrings[kPGPdiskMaxMinorErrorStrings] =
{
	// kPGDMinorError_NoErr
	{
		"there was no error"
	}, 

	// kPGDMinorError_DialogDisplayFailed
	{
		"PGPdisk was unable to display a dialog box"
	}, 
	
	// kPGDMinorError_DriverCommFailure
	{
		"data could not be sent to or received from the driver"
	}, 
	
	// kPGDMinorError_TooManyAppsRegistered
	{
		"there are too many PGPdisk apps currently open"
	}, 
	
	// kPGDMinorError_FilesOpenOnDrive
	{
		"there are open files on or open views of a PGPdisk. Please close "
		"all open files and all Explorer windows and then try again."
	}, 
	
	// kPGDMinorError_MaxPGPdisksMounted
	{
		"the maximum number of PGPdisks are already mounted"
	}, 
	
	// kPGDMinorError_NoPGPdisksToUnmount
	{
		"there are no PGPdisks currently mounted"
	}, 
	
	// kPGDMinorError_OutOfMemory
	{
		"of an out of memory error"
	}, 
	
	// kPGDMinorError_PGPdiskAlreadyMounted
	{
		"the seleced PGPdisk is already mounted"
	}, 
	
	// kPGDMinorError_SemaphoreCreationFailed
	{
		"Windows is running critically low on resources"
	}, 
	
	// kPGDMinorError_CloseHandleFailed
	{
		"PGPdisk was unable to close a handle"
	}, 
	
	// kPGDMinorError_DriverNotInstalled
	{
		"the PGPdisk driver is not installed. Please re-install PGPdisk and "
		"then try again"
	}, 
	
	// kPGDMinorError_ThreadCreationFailed
	{
		"Windows is running critically low on resources"
	}, 
	
	// kPGDMinorError_FileOpenFailed
	{
		"PGPdisk was unable to open a file"
	}, 
	
	// kPGDMinorError_SetFilePosFailed
	{
		"PGPdisk was unable to set a file position"
	}, 
	
	// kPGDMinorError_SetEOFFailed
	{
		"PGPdisk was unable to set the end of file"
	}, 
	
	// kPGDMinorError_FileReadFailed
	{
		"PGPdisk was unable to read from a file"
	}, 
	
	// kPGDMinorError_FileWriteFailed
	{
		"PGPdisk was unable to write to a file"
	}, 
	
	// kPGDMinorError_MutexCreationFailed
	{
		"Windows is running critically low on resources"
	}, 
	
	// kPGDMinorError_GetFreeSpaceFailed
	{
		"PGPdisk was unable to determine the amount of free disk space"
	}, 
	
	// kPGDMinorError_GetVersionFailed
	{
		"PGPdisk was unable to determine the version of Windows installed "
		"on this system"
	}, 
	
	// kPGDMinorError_IOFlagViolation
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IspGetDcbFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IspInsertCalldownFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_FindAppPathFailed
	{
		"PGPdisk was unable to find the pathname of the PGPdisk application"
	}, 
	
	// kPGDMinorError_GetAppModuleFailed
	{
		"Windows is running critically low on resources"
	}, 
	
	// kPGDMinorError_LockVolumeFailed
	{
		"a volume could not be locked for use by PGPdisk. It may be in use "
		"by another disk utility"
	}, 
	
	// kPGDMinorError_UnlockVolumeFailed
	{
		"a volume could not be unlocked for use by PGPdisk. It may be in "
		"use by another disk utility"
	}, 
	
	// kPGDMinorError_EnumOpenFilesFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_InvalidVRP
	{
		"the PGPdisk's host volume appears to have gone away"
	}, 
	
	// kPGDMinorError_IorAllocationFailed
	{
		"Windows is running critically low on resources"
	}, 
	
	// kPGDMinorError_IspDriveLetterPickFailed
	{
		"there are no free drive letters available"
	}, 
	
	// kPGDMinorError_IspAssociateDcbFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IspDisassocDcbFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_R0OpenFailed
	{
		"the driver was unable to open a file"
	}, 
	
	// kPGDMinorError_R0CloseFailed
	{
		"the driver was unable to close a file"
	}, 
	
	// kPGDMinorError_R0ReadFailed
	{
		"the driver was unable to read from a file"
	}, 
	
	// kPGDMinorError_R0WriteFailed
	{
		"the driver was unable to write to a file"
	}, 
	
	// kPGDMinorError_R0SetFilePosFailed
	{
		"the driver was unable to set a file position"
	}, 

	// kPGDMinorError_IorReadFailure
	{
		"the driver was unable to read from a volume"
	}, 
	
	// kPGDMinorError_IorWriteFailure
	{
		"the driver was unable to write to a volume"
	}, 
	
	// kPGDMinorError_IosRegistrationFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_LoadLibraryFailed
	{
		"PGPdisk was unable to load a needed Windows .dll"
	}, 
	
	// kPGDMinorError_GetProcAddrFailed
	{
		"of an internal error within the PGPdisk application"
	}, 
	
	// kPGDMinorError_R0GetFileSize
	{
		"the driver was unable to determine the size of a file"
	}, 
	
	// kPGDMinorError_OOBFileRequest
	{
		"an invalid read or write request was received"
	}, 
	
	// kPGDMinorError_CorruptFat
	{
		"the FAT table of the host volume appears to be corrupted"
	}, 
	
	// kPGDMinorError_GetFirstClustFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_VWIN32NotFound
	{
		"of an internal error within the PGPdisk application"
	}, 
	
	// kPGDMinorError_DcbClaimFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_FileNotFound
	{
		"this file could not be found"
	}, 
	
	// kPGDMinorError_RegistryWriteFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_FailSilently
	{
		"the user canceled the operation"
	}, 
	
	// kPGDMinorError_DeviceHookFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_AppIsIncompatVersion
	{
		"this system is running different versions of the PGPdisk driver "
		"and the PGPdisk application. Please re-install PGPdisk to correct "
		"this"
	}, 
	
	// kPGDMinorError_DriverIsIncompatVersion
	{
		"this system is running different versions of the PGPdisk driver "
		"and the PGPdisk application. Please re-install PGPdisk to correct "
		"this"
	}, 
	
	// kPGDMinorError_PostThreadMessageFailed
	{
		"of an internal error within the PGPdisk application"
	}, 
	
	// kPGDMinorError_BadAppOp
	{
		"of an internal error within the PGPdisk application"
	}, 
	
	// kPGDMinorError_SetVolumeLabelFailed
	{
		"PGPdisk was unable to set the name of a volume"
	}, 
	
	// kPGDMinorError_SetTimerFailed
	{
		"Windows is running critically low on resources"
	}, 
	
	// kPGDMinorError_HookSysBroadcastFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_BitmapLoadFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_LinPageLockFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_LinPageUnlockFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_VirtualFreeFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_RegCreateKeyExFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_RegSetValueExFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_RegCloseKeyFailed
	{
		"of a problem accessing the system registry"
	}, 

	// kPGDMinorError_BadHeaderMagic
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_BadHeaderCRC
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_InvalidHeaderType
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_NewerHeaderVersion
	{
		"this PGPdisk was created by a newer version of the PGPdisk "
		"application."
	}, 
	
	// kPGDMinorError_BadHeaderSize
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_InvalidAlgorithm
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_InvalidTotalFileSize
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_InvalidFileDataSize
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_InvalidHeaderBlocks
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_IncorrectPassphrase
	{
		"the passphrase you entered is not a valid master or alternate "
		"passphrase for this PGPdisk"
	}, 
	
	// kPGDMinorError_PGPdiskIsWriteProtected
	{
		"this PGPdisk file is write-protected"
	}, 
	
	// kPGDMinorError_AllAlternatesTaken
	{
		"all seven alternate passphrases are already in use"
	}, 
	
	// kPGDMinorError_UnsupportedFilesystem
	{
		"this file does not appear to be a valid PGPdisk"
	}, 
	
	// kPGDMinorError_ContextNotInitialized
	{
		"of an internal error within the PGPdisk application"
	}, 
	
	// kPGDMinorError_InvalidSBoxes
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_GetFileAttribsFailed
	{
		"PGPdisk was unable to access a file"
	}, 
	
	// kPGDMinorError_GetVolumeInfoFailed
	{
		"PGPdisk was unable to get information about a volume"
	}, 
	
	// kPGDMinorError_HotKeyHookFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_RegOpenKeyExFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_RegQueryValueExFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_IopRequestOverflow
	{
		"an internal buffer overflow error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_MiniRequestOverflow
	{
		"an internal buffer overflow error within the PGPdisk drive"
	}, 
	
	// kPGDMinorError_RegisteredAppNotFound
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_PGPdiskNotFound
	{
		"this PGPdisk does not exist or could not be found"
	}, 
	
	// kPGDMinorError_DirectoryWriteProtected
	{
		"the specified directory is write-protected"
	}, 
	
	// kPGDMinorError_DirectoryNotFound
	{
		"the specified directory does not exist"
	}, 
	
	// kPGDMinorError_DirectoryTooSmall
	{
		"the specified directory does not have the minimum required amount "
		"of free space (about 75k)"
	}, 
	
	// kPGDMinorError_NoDriveLettersFree
	{
		"there are no free drive letters available"
	}, 
	
	// kPGDMinorError_UserAbort
	{
		"the user canceled the operation"
	}, 
	
	// kPGDMinorError_GetFileSizeFailed
	{
		"PGPdisk could not determine the size of a file"
	}, 
	
	// kPGDMinorError_NoAlternatesToRemove
	{
		"this PGPdisk has no alternate passphrases to remove"
	}, 
	
	// kPGDMinorError_RegEnumKeyExFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_RegDeleteKeyFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_PGPdiskNotMounted
	{
		"the specified file is not a mounted PGPdisk"
	}, 
	
	// kPGDMinorError_InvalidPathNameFound
	{
		"the specified path or filename is invalid"
	}, 
	
	// kPGDMinorError_FindFirstFileFailed
	{
		"an error occured while finding a directory or a file"
	}, 
	
	// kPGDMinorError_CoCreateInstanceFailed
	{
		"an error occurred while initializing an OLE object"
	}, 
	
	// kPGDMinorError_QueryInterfaceFailed
	{
		"an error occurred while querying an OLE object"
	}, 
	
	// kPGDMinorError_OLELoadCommandFailed
	{
		"an error occurred whlie loading an OLE object"
	}, 
	
	// kPGDMinorError_ResolveShortcutFailed
	{
		"the specified shortcut could not be resolved"
	}, 
	
	// kPGDMinorError_OLEGetPathFailed
	{
		"the specified shortcut could not be resolved"
	}, 
	
	// kPGDMinorError_OLEGetDescriptionFailed
	{
		"an error occurred while getting a description of the file"
	}, 
	
	// kPGDMinorError_CoInitializeExFailed
	{
		"the COM library could not be initialized"
	}, 
	
	// kPGDMinorError_InstallFSAPIHookFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_PGPdiskAlreadyInUse
	{
		"this PGPdisk may already be opened with read/write access by "
		"another user or application"
	}, 
	
	// kPGDMinorError_RegDeleteValueFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_MapViewOfFileFailed
	{
		"Windows is running critically low on resources"
	}, 
	
	// kPGDMinorError_OleInitFailed
	{
		"OLE could not be initialized"
	}, 
	
	// kPGDMinorError_DeviceConstructFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_RegEnumValueFailed
	{
		"of a problem accessing the system registry"
	}, 
	
	// kPGDMinorError_PGPdiskIsBusy
	{
		"this PGPdisk is busy processing an I/O request"
	}, 
	
	// kPGDMinorError_SHFormatDriveFailed
	{
		"PGPdisk was unable to display the Windows formatting dialog"
	}, 
	
	// kPGDMinorError_DiskTooBigToFormat
	{
		"the specified PGPdisk volume is too large to format automatically. "
		"Instead, browse to the volume in the Windows Explorer, "
		"right-click on the drive letter, and choose 'Format' from the "
		"context menu."
	}, 
	
	// kPGDMinorError_ZwQueryInfoFileFailed
	{
		"PGPdisk was unable to read the attributes of a file"
	}, 
	
	// kPGDMinorError_ZwCreateFileFailed
	{
		"PGPdisk was unable to open or create a file"
	}, 
	
	// kPGDMinorError_ZwCloseFailed
	{
		"PGPdisk was unable to close a file"
	}, 
	
	// kPGDMinorError_ZwReadFileFailed
	{
		"PGPdisk was unable to read from a file"
	}, 
	
	// kPGDMinorError_ZwWriteFileFailed
	{
		"PGPdisk was unable to write to a file"
	}, 
	
	// kPGDMinorError_RtlAnsiToUniFailed
	{
		"PGPdisk was unable to convert a string from ANSI to Unicode"
	}, 
	
	// kPGDMinorError_RtlUniToAnsiFailed
	{
		"PGPdisk was unable to convert a string from Unicode to ANSI"
	}, 
	
	// kPGDMinorError_IoGetDeviceObjFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IoCreateDeviceFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IoCreateSymbolicLinkFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IoDeleteSymbolicLinkFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_ObRefObjByHandleFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IoBuildIOCTLRequestFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IoCallDriverFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_IoBuildSynchFsdRequestFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 

	// kPGDMinorError_UniStringOpFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_NotCompatibleWindows
	{
		"PGPdisk is not compatible with your version of Windows"
	}, 
	
	// kPGDMinorError_RegisterClassExFailed
	{
		"a window class could not be registered"
	}, 
	
	// kPGDMinorError_CreateWindowFailed
	{
		"a window could not be created"
	}, 

	// kPGDMinorError_AttachMenuFailed
	{
		"Windows is running critically low on resources"
	}, 

	// kPGDMinorError_CreatePopupMenuFailed
	{
		"a menu could not be created"
	}, 

	// kPGDMinorError_InsertMenuFailed
	{
		"a menu item could not be inserted into a menu"
	}, 

	// kPGDMinorError_CreateFileMappingFailed
	{
		"Windows is running critically low on resources"
	}, 

	// kPGDMinorError_OpenFileMappingFailed
	{
		"Windows is running critically low on resources"
	}, 

	// kPGDMinorError_UnmapViewOfFileFailed, 
	{
		"of an internal error within the PGPdisk application"
	}, 

	// kPGDMinorError_WindowCreationFailed
	{
		"a window couldn't be created"
	}, 

	// kPGDMinorError_InvalidParameter
	{
		"an invalid parameter was received"
	}, 

	// kPGDMinorError_IoAllocateMdlFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 

	// kPGDMinorError_CreateSystemThreadFailed
	{
		"Windows is running critically low on resources"
	}, 

	// kPGDMinorError_InvalidDeviceReceived
	{
		"the system attempted to perform I/O to a non-existant PGPdisk"
	}, 

	// kPGDMinorError_BroadcastSystemMessageFailed
	{
		"PGPdisk was unable to broadcast a windows message to the system"
	}, 

	// kPGDMinorError_IoInitializeTimerFailed
	{
		"Windows is running critically low on resources"
	}, 

	// kPGDMinorError_KModeExceptionOccured
	{
		"of an internal error within the PGPdisk driver"
	}, 

	// kPGDMinorError_RegisterHotKeyFailed
	{
		"PGPdisk was unable to register a hotkey"
	}, 

	// kPGDMinorError_UnregisterHotKeyFailed
	{
		"PGPdisk was unable to unregister a hotkey"
	}, 

	// kPGDMinorError_CantFormatDrive
	{
		"PGPdisk doesn't support formatting drives of this type. Use "
		"Windows to format the drive instead"
	}, 

	// kPGDMinorError_CreateProcessFailed
	{
		"of an internal error within the PGPdisk application"
	}, 

	// kPGDMinorError_AccessDeniedToVolume
	{
		"access is denied. The current user doesn't have the privileges "
		"necessary to access the specified volume"
	}, 

	// kPGDMinorError_CouldntFindAppString
	{
		"the PGPdisk application could not be found. Please run PGPdisk "
		"once to correct this"
	}, 

	// kPGDMinorError_BlockSizeNotSupported
	{
		"PGPdisk does not support volumes whose block size is greater than "
		"the page size of the current system (usually 4k)"
	}, 

	// kPGDMinorError_WNetGetUniNameFailed
	{
		"PGPdisk was unable to determine the location of a file on a "
		"networked drive"
	}, 

	// kPGDMinorError_NetShareGetInfoFailed
	{
		"PGPdisk was unable to determine the location of a file on a "
		"networked drive"
	}, 

	// kPGDMinorError_TriedNTFSOnWin95
	{
		"it was formatted using the NTFS filesystem under Windows NT, which "
		"is not compatible with either Windows 95 or 98"
	}, 

	// kPGDMinorError_TriedReadOnlyNTFS
	{
		"it was formatted using the NTFS filesystem, which cannot be used "
		"in read-only mode. If you are attempting to mount this disk with "
		"an ADK and are seeing this message, then either uncheck the "
		"read-only option in the passphrase dialog or mark the PGPdisk "
		"file as non-read-only."
	}, 

	// kPGDMinorError_ZwOpenProcessFailed, 
	{
		"of an internal error within the PGPdisk driver"
	}, 
	
	// kPGDMinorError_ZwOpenProcessTokenFailed, 
	{
		"of an internal error within the PGPdisk driver"
	}, 

	// kPGDMinorError_ZwDuplicateTokenFailed, 
	{
		"of an internal error within the PGPdisk driver"
	}, 

	// kPGDMinorError_ZwSetThreadInfoFailed
	{
		"of an internal error within the PGPdisk driver"
	}, 

	// kPGDMinorError_NetWkstaGetInfoFailed
	{
		"PGPdisk was unable to determine the name of a computer"
	}, 

	// kPGDMinorError_SDKFuncFailed
	{
		"an error occurred while processing a public or private PGP key"
	}, 

	// kPGDMinorError_SDKNotAvailable
	{
		"the PGPsdk dll file could not be found"
	}, 

	// kPGDMinorError_ZwSetInfoFileFailed
	{
		"PGPdisk was unable to set the size of a file"
	}, 

	// kPGDMinorError_IncPassAndKeyNotInRing
	{
		"the passphrase you entered is not a valid master or alternate "
		"passphrase for this PGPdisk. (Please note that there were "
		"public key passphrases on this PGPdisk that could not be checked, "
		"either because they aren't on the local keyring or because the "
		"local keyring could not be opened.)"
	}, 

	// kPGDMinorError_CantGetADK
	{
		"the Additional Decryption Key associated with this installation of "
		"PGPdisk could not be accessed"
	}, 

	// kPGDMinorError_PGPdiskPassesWiped
	{
		"this PGPdisk has had its master and alternate passphrases wiped. "
		"It can now only be mounted by the holder of the private key "
		"associated with one or more of the public keys on the PGPdisk"
	}, 

	// kPGDMinorError_NoADKWhenConverting
	{
		"this PGPdisk contains an ADK that is not present on the current "
		"public key ring. Please acquire a copy of this public key before "
		"attempting conversion. A list of the public keys on this PGPdisk "
		"will now be displayed."
	}

};

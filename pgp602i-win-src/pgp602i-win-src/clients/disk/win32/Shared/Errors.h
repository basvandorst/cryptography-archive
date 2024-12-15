//////////////////////////////////////////////////////////////////////////////
// Errors.h
//
// Contains declarations for errors for the project.
//////////////////////////////////////////////////////////////////////////////

// $Id: Errors.h,v 1.3.2.52.2.5 1998/10/25 23:31:37 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Errors_h	// [
#define Included_Errors_h


////////////
// Constants
////////////

const PGPUInt32 kPGPdiskMaxMajorErrorStrings = 1024;
const PGPUInt32 kPGPdiskMaxMinorErrorStrings = 1024;


//////////
// Externs
//////////

extern LPCSTR	kPGPdiskMajorErrorStrings[kPGPdiskMaxMajorErrorStrings];
extern LPCSTR	kPGPdiskMinorErrorStrings[kPGPdiskMaxMinorErrorStrings];


////////
// Types
////////

// Assume no external error is larger than 32 bits.

typedef	PGPUInt32 ExternalError;
const ExternalError kExternalError_NoErr = 0;

// A PGDMajorError explains what operation failed.

static LPCSTR kPGPdiskUnknownMajorErrorString = 
	"An unknown error occurred.";

enum PGDMajorError
{
	kPGDMajorError_NoErr, 
	kPGDMajorError_AppInitFailed, 
	kPGDMajorError_PGPdiskCreationFailed, 
	kPGDMajorError_PGPdiskMountFailed, 
	kPGDMajorError_PGPdiskTooBig, 
	kPGDMajorError_PGPdiskTooSmall, 
	kPGDMajorError_PGPdiskUnmountFailed, 
	kPGDMajorError_PGPdiskReadError, 
	kPGDMajorError_PGPdiskWriteError, 
	kPGDMajorError_AboutBoxDisplayFailed, 
	kPGDMajorError_PreferencesAccessFailed, 
	kPGDMajorError_TimeoutValueTooSmall, 
	kPGDMajorError_TimeoutValueTooBig, 
	kPGDMajorError_NotEnoughDiskSpace, 
	kPGDMajorError_PassphraseTooSmall, 
	kPGDMajorError_PassphraseNotConfirmed, 
	kPGDMajorError_PGPdiskAddPassFailed, 
	kPGDMajorError_PGPdiskChangePassFailed, 
	kPGDMajorError_PGPdiskRemoveOnePassFailed, 
	kPGDMajorError_IncorrectPassphrase, 
	kPGDMajorError_IncorrectMasterPassphrase, 
	kPGDMajorError_TriedRemoveMasterPass, 
	kPGDMajorError_IncorrectAlternatePassphrase, 
	kPGDMajorError_PassphraseAlreadyExists, 
	kPGDMajorError_NotCompatibleWindows, 
	kPGDMajorError_TriedPassOpOnMountedPGPdisk, 
	kPGDMajorError_InvalidCipherContext, 
	kPGDMajorError_DriverUpdatePrefsFailed, 
	kPGDMajorError_DialogDisplayFailed, 
	kPGDMajorError_ChoseReadOnlyDirInCreate, 
	kPGDMajorError_ChoseTooSmallDirInCreate, 
	kPGDMajorError_ChoseInvalidDirInCreate, 
	kPGDMajorError_NoSleepOnUnmountFailure, 
	kPGDMajorError_BadCommandAfterBetaTimeout, 
	kPGDMajorError_ChoseReplaceFileInUse, 
	kPGDMajorError_PGPdiskRemoveAllPassesFailed, 
	kPGDMajorError_PGPdiskUnmountAllFailed, 
	kPGDMajorError_InvalidVolumeName, 
	kPGDMajorError_WindowsFormatFailed, 
	kPGDMajorError_ResidentAppInitFailed, 
	kPGDMajorError_PGPdiskResHotKeyOpFailed, 
	kPGDMajorError_PGPdiskIrpCleanupError, 
	kPGDMajorError_CouldntFindAppString, 
	kPGDMajorError_KModeExceptionOccured, 
	kPGDMajorError_CouldntFindResidentApp, 
	kPGDMajorError_AddRemovePublicKeysFailed, 
	kPGDMajorError_IncPassAndKeyNotInRing, 
	kPGDMajorError_PGPdiskPassesWiped, 
	kPGDMajorError_WipePassesFailed, 
	kPGDMajorError_PGPdiskConversionFailed, 
	kPGDMajorError_PGPdiskGlobalConversionFailed, 
	kPGDMajorError_IncorrectPublicKeyPassphrase

};

// A PGDMinorError explains why the operation failed.

static LPCSTR kPGPdiskDefaultMinorErrorString = 
	"an error occurred.";

static LPCSTR kPGPdiskUnknownMinorErrorString = 
	"an unknown error occurred.";

enum PGDMinorError
{
	kPGDMinorError_NoErr, 
	kPGDMinorError_DialogDisplayFailed, 
	kPGDMinorError_DriverCommFailure, 
	kPGDMinorError_TooManyAppsRegistered, 
	kPGDMinorError_FilesOpenOnDrive, 
	kPGDMinorError_MaxPGPdisksMounted, 
	kPGDMinorError_NoPGPdisksToUnmount, 
	kPGDMinorError_OutOfMemory, 
	kPGDMinorError_PGPdiskAlreadyMounted, 
	kPGDMinorError_SemaphoreCreationFailed, 
	kPGDMinorError_CloseHandleFailed, 
	kPGDMinorError_DriverNotInstalled, 
	kPGDMinorError_ThreadCreationFailed, 
	kPGDMinorError_FileOpenFailed, 
	kPGDMinorError_SetFilePosFailed, 
	kPGDMinorError_SetEOFFailed, 
	kPGDMinorError_FileReadFailed, 
	kPGDMinorError_FileWriteFailed, 
	kPGDMinorError_MutexCreationFailed, 
	kPGDMinorError_GetFreeSpaceFailed, 
	kPGDMinorError_GetVersionFailed, 
	kPGDMinorError_IOFlagViolation, 
	kPGDMinorError_IspGetDcbFailed, 
	kPGDMinorError_IspInsertCalldownFailed, 
	kPGDMinorError_FindAppPathFailed, 
	kPGDMinorError_GetAppModuleFailed, 
	kPGDMinorError_LockVolumeFailed, 
	kPGDMinorError_UnlockVolumeFailed, 
	kPGDMinorError_EnumOpenFilesFailed, 
	kPGDMinorError_InvalidVRP, 
	kPGDMinorError_IorAllocationFailed, 
	kPGDMinorError_IspDriveLetterPickFailed, 
	kPGDMinorError_IspAssociateDcbFailed, 
	kPGDMinorError_IspDisassocDcbFailed, 
	kPGDMinorError_R0OpenFailed, 
	kPGDMinorError_R0CloseFailed, 
	kPGDMinorError_R0ReadFailed, 
	kPGDMinorError_R0WriteFailed, 
	kPGDMinorError_R0SetFilePosFailed, 
	kPGDMinorError_IorReadFailure, 
	kPGDMinorError_IorWriteFailure, 
	kPGDMinorError_IosRegistrationFailed, 
	kPGDMinorError_LoadLibraryFailed, 
	kPGDMinorError_GetProcAddrFailed, 
	kPGDMinorError_R0GetFileSize, 
	kPGDMinorError_OOBFileRequest, 
	kPGDMinorError_CorruptFat, 
	kPGDMinorError_GetFirstClustFailed, 
	kPGDMinorError_VWIN32NotFound, 
	kPGDMinorError_DcbClaimFailed, 
	kPGDMinorError_FileNotFound, 
	kPGDMinorError_RegistryWriteFailed, 
	kPGDMinorError_FailSilently, 
	kPGDMinorError_DeviceHookFailed, 
	kPGDMinorError_AppIsIncompatVersion, 
	kPGDMinorError_DriverIsIncompatVersion, 
	kPGDMinorError_PostThreadMessageFailed, 
	kPGDMinorError_BadAppOp, 
	kPGDMinorError_SetVolumeLabelFailed, 
	kPGDMinorError_SetTimerFailed, 
	kPGDMinorError_HookSysBroadcastFailed, 
	kPGDMinorError_BitmapLoadFailed, 
	kPGDMinorError_LinPageLockFailed, 
	kPGDMinorError_LinPageUnlockFailed, 
	kPGDMinorError_VirtualFreeFailed, 
	kPGDMinorError_RegCreateKeyExFailed, 
	kPGDMinorError_RegSetValueExFailed, 
	kPGDMinorError_RegCloseKeyFailed, 
	kPGDMinorError_BadHeaderMagic, 
	kPGDMinorError_BadHeaderCRC, 
	kPGDMinorError_InvalidHeaderType, 
	kPGDMinorError_NewerHeaderVersion, 
	kPGDMinorError_BadHeaderSize, 
	kPGDMinorError_InvalidAlgorithm, 
	kPGDMinorError_InvalidTotalFileSize, 
	kPGDMinorError_InvalidFileDataSize, 
	kPGDMinorError_InvalidHeaderBlocks, 
	kPGDMinorError_IncorrectPassphrase, 
	kPGDMinorError_PGPdiskIsWriteProtected, 
	kPGDMinorError_AllAlternatesTaken, 
	kPGDMinorError_UnsupportedFilesystem, 
	kPGDMinorError_ContextNotInitialized, 
	kPGDMinorError_InvalidSBoxes, 
	kPGDMinorError_GetFileAttribsFailed, 
	kPGDMinorError_GetVolumeInfoFailed, 
	kPGDMinorError_HotKeyHookFailed, 
	kPGDMinorError_RegOpenKeyExFailed, 
	kPGDMinorError_RegQueryValueExFailed, 
	kPGDMinorError_IopRequestOverflow, 
	kPGDMinorError_MiniRequestOverflow, 
	kPGDMinorError_RegisteredAppNotFound, 
	kPGDMinorError_PGPdiskNotFound, 
	kPGDMinorError_DirectoryWriteProtected, 
	kPGDMinorError_DirectoryNotFound, 
	kPGDMinorError_DirectoryTooSmall, 
	kPGDMinorError_NoDriveLettersFree, 
	kPGDMinorError_UserAbort, 
	kPGDMinorError_GetFileSizeFailed, 
	kPGDMinorError_NoAlternatesToRemove, 
	kPGDMinorError_RegEnumKeyExFailed, 
	kPGDMinorError_RegDeleteKeyFailed, 
	kPGDMinorError_PGPdiskNotMounted, 
	kPGDMinorError_InvalidPathNameFound, 
	kPGDMinorError_FindFirstFileFailed, 
	kPGDMinorError_CoCreateInstanceFailed, 
	kPGDMinorError_QueryInterfaceFailed, 
	kPGDMinorError_OLELoadCommandFailed, 
	kPGDMinorError_ResolveShortcutFailed, 
	kPGDMinorError_OLEGetPathFailed, 
	kPGDMinorError_OLEGetDescriptionFailed, 
	kPGDMinorError_CoInitializeExFailed, 
	kPGDMinorError_InstallFSAPIHookFailed, 
	kPGDMinorError_PGPdiskAlreadyInUse, 
	kPGDMinorError_RegDeleteValueFailed, 
	kPGDMinorError_MapViewOfFileFailed, 
	kPGDMinorError_OleInitFailed, 
	kPGDMinorError_DeviceConstructFailed, 
	kPGDMinorError_RegEnumValueFailed, 
	kPGDMinorError_PGPdiskIsBusy, 
	kPGDMinorError_SHFormatDriveFailed, 
	kPGDMinorError_DiskTooBigToFormat, 
	kPGDMinorError_ZwQueryInfoFileFailed, 
	kPGDMinorError_ZwCreateFileFailed, 
	kPGDMinorError_ZwCloseFailed, 
	kPGDMinorError_ZwReadFileFailed, 
	kPGDMinorError_ZwWriteFileFailed, 
	kPGDMinorError_RtlAnsiToUniFailed, 
	kPGDMinorError_RtlUniToAnsiFailed, 
	kPGDMinorError_IoGetDeviceObjFailed, 
	kPGDMinorError_IoCreateDeviceFailed, 
	kPGDMinorError_IoCreateSymbolicLinkFailed, 
	kPGDMinorError_IoDeleteSymbolicLinkFailed, 
	kPGDMinorError_ObRefObjByHandleFailed, 
	kPGDMinorError_IoBuildIOCTLRequestFailed, 
	kPGDMinorError_IoCallDriverFailed, 
	kPGDMinorError_IoBuildSynchFsdRequestFailed, 
	kPGDMinorError_UniStringOpFailed, 
	kPGDMinorError_NotCompatibleWindows, 
	kPGDMinorError_RegisterClassExFailed, 
	kPGDMinorError_CreateWindowFailed, 
	kPGDMinorError_AttachMenuFailed, 
	kPGDMinorError_CreatePopupMenuFailed, 
	kPGDMinorError_InsertMenuFailed, 
	kPGDMinorError_CreateFileMappingFailed, 
	kPGDMinorError_OpenFileMappingFailed, 
	kPGDMinorError_UnmapViewOfFileFailed, 
	kPGDMinorError_WindowCreationFailed, 
	kPGDMinorError_InvalidParameter, 
	kPGDMinorError_IoAllocateMdlFailed, 
	kPGDMinorError_CreateSystemThreadFailed, 
	kPGDMinorError_InvalidDeviceReceived, 
	kPGDMinorError_BroadcastSystemMessageFailed, 
	kPGDMinorError_IoInitializeTimerFailed, 
	kPGDMinorError_KModeExceptionOccured, 
	kPGDMinorError_RegisterHotKeyFailed, 
	kPGDMinorError_UnregisterHotKeyFailed, 
	kPGDMinorError_CantFormatDrive, 
	kPGDMinorError_CreateProcessFailed, 
	kPGDMinorError_AccessDeniedToVolume, 
	kPGDMinorError_CouldntFindAppString, 
	kPGDMinorError_BlockSizeNotSupported, 
	kPGDMinorError_WNetGetUniNameFailed, 
	kPGDMinorError_NetShareGetInfoFailed, 
	kPGDMinorError_TriedNTFSOnWin95, 
	kPGDMinorError_TriedReadOnlyNTFS, 
	kPGDMinorError_ZwOpenProcessFailed, 
	kPGDMinorError_ZwOpenProcessTokenFailed, 
	kPGDMinorError_ZwDuplicateTokenFailed, 
	kPGDMinorError_ZwSetThreadInfoFailed, 
	kPGDMinorError_NetWkstaGetInfoFailed, 
	kPGDMinorError_SDKFuncFailed, 
	kPGDMinorError_SDKNotAvailable, 
	kPGDMinorError_ZwSetInfoFileFailed, 
	kPGDMinorError_IncPassAndKeyNotInRing, 
	kPGDMinorError_CantGetADK, 
	kPGDMinorError_PGPdiskPassesWiped, 
	kPGDMinorError_NoADKWhenConverting

};


////////////////////
// Utility functions
////////////////////

// DoesMajorErrTakeDrive returns TRUE if the major error string takes a drive
// number parameter.

inline 
PGPBoolean
DoesMajorErrTakeDrive(PGDMajorError perr)
{
	switch (perr)
	{
	case kPGDMajorError_PGPdiskUnmountFailed:
	case kPGDMajorError_InvalidCipherContext:
	case kPGDMajorError_PGPdiskIrpCleanupError:
		return TRUE;

	default:
		return FALSE;
	}
}

// IsExternalError returns TRUE if the passed external error is an error.

inline 
PGPBoolean
IsExternalError(ExternalError err)
{
	return (err != kExternalError_NoErr);
}

// IsntExternalError returns TRUE if the passed external error isn't an
// error.

inline 
PGPBoolean
IsntExternalError(ExternalError err)
{
	return (err == kExternalError_NoErr);
}

// IsPGDMajorError returns TRUE if the passed PGDMajorError variable is an
// error.

inline 
PGPBoolean
IsPGDMajorError(PGDMajorError perr)
{
	return (perr != kPGDMajorError_NoErr);
}

// IsntPGDMajorError returns TRUE if the passed PGDMajorError variable is not
// an error.

inline 
PGPBoolean
IsntPGDMajorError(PGDMajorError perr)
{
	return (perr == kPGDMajorError_NoErr);
}

// IsPGDMinorError returns TRUE if the passed PGDMinorError variable is an
// error.

inline 
PGPBoolean
IsPGDMinorError(PGDMinorError perr)
{
	return (perr != kPGDMinorError_NoErr);
}

// IsntPGDMinorError returns TRUE if the passed PGDMinorError variable is not
// an error.

inline 
PGPBoolean
IsntPGDMinorError(PGDMinorError perr)
{
	return (perr == kPGDMinorError_NoErr);
}

#endif	// ] Included_Errors_h

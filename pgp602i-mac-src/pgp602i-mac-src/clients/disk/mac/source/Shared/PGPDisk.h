/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: PGPDisk.h,v 1.7.8.1 1998/11/12 03:06:06 heller Exp $____________________________________________________________________________*/#pragma once#include <Power.h>#include <Disks.h>#include "PGPMountUtils.h"#define	kPGPDiskFileCreator 	'pgpD'#define kPGPDiskFileType		'Dsk1'#define	kMaxPassPhraseLength	130#define	kMinPassPhraseLength	8#include "DriverAPI.h"// PGPDisk error numbers// CAUTION: if you add an error, you probably want to add an error string// to the list in PGPDiskResources.henum{	kPGPDiskInvalidFileHeaderError 	= 1000,	kPGPDiskUnknownFileVersionError,	kPGPDiskNewerFileVersionError,	kPGPDiskUnknownFileKindError,			// Invalid type and/or creator	kPGPDiskUnknownAlgorithmError,		kPGPDiskRemoteVolumeTypeError,	kPGPDiskForeignVolumeTypeError,		kPGPDiskIncorrectPassphraseError,	kPGPDiskMasterPassphraseError,	#if PGPDISK_PUBLIC_KEY	kPGPDiskIncorrectPassphraseNoKeyringsError,	kPGPDiskIncorrectPassphraseKeyNotFoundError,	kPGPdiskADKNotFoundError,#endif		kPGPDiskFileInUseByDriverError,	kPGPDiskFileAlreadyOpenError,	kPGPDiskFileLockedError,	kPGPDiskFileOnLockedVolumeError,		kCastSBoxesInvalidError,	kCipherProcDamagedError,	kPGPDiskInvalidGlobalsMagicError,	kNoMorePassphrasesAvailableError,	kEndOfKeyIterationError};
//////////////////////////////////////////////////////////////////////////////
// PGPdiskHighLevelUtils.cpp
//
// High-level utility functions for working with PGPdisks.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskHighLevelUtils.cpp,v 1.14 1999/03/31 23:51:09 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"
#include <new.h>

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include "PlacementNew.h"
#elif defined(PGPDISK_NTDRIVER)

#define	__w64
#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"

#if !defined(PGPDISK_NOSDK)
#include "PGPclx.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpAdminPrefs.h"
#include "pgpErrors.h"
#include "pgpPublicKey.h"
#include "pgpUtilities.h"
#include "GlobalPGPContext.h"
#include "PGPdiskPublicKeyUtils.h"
#endif	// !PGPDISK_NOSDK

#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskLowLevelUtils.h"
#include "SecureMemory.h"
#include "UtilityFunctions.h"

#include "File.h"


//////////////////////////////////////
// High-level header utility functions
//////////////////////////////////////

// ReadPGPdiskFileMainHeader(File *diskFile... ) reads in the main header
// from the opened PGPdisk.

DualErr
ReadPGPdiskFileMainHeader(File *diskFile, PGPdiskFileHeader **fileHeader)
{
	DualErr	derr;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader *);

	// Assert rigid file format assumptions.
	pgpAssert(sizeof(PGPdiskFileHeader) == 380);
	pgpAssert(sizeof(PGPdiskFileHeader)%4 == 0);
	pgpAssert(sizeof(PGPdiskEncryptionAlgorithm) == 4);
	pgpAssertMsg(kDefaultBlockSize == 512, "kDefaultBlockSize must be 512");
	pgpAssertMsg(kPGPdiskAlternateHeaderSize == 512, "Cannot change the " 
		"alternate header size and expect to read old files correctly");
	pgpAssert(sizeof(PassphraseSalt) == 8);
	pgpAssert(sizeof(EncryptedCASTKey) == 16);

	derr = ReadPGPdiskFilePrimaryHeader(diskFile, fileHeader);

	if (derr.IsError())
	{
		DualErr savedDerr = derr;

		derr = ReadPGPdiskFileAlternateHeader(diskFile, fileHeader);

		// Success on alternate wipes out first error, else report the
		// first error and not the second error.

		if (derr.IsntError())
		{
			derr = DualErr::NoError;
		}
		else
		{
			derr = savedDerr;
		}
	}

	return derr;
}

// ReadPGPdiskFileMainHeader(LPCSTR path... ) is a wrapper function.

DualErr
ReadPGPdiskFileMainHeader(LPCSTR path, PGPdiskFileHeader **fileHeader)
{
	DualErr	derr;
	File	diskFile;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader *);

	derr = diskFile.Open(path, kOF_MustExist | kOF_ReadOnly);

	if (derr.IsntError())
	{
		derr = ReadPGPdiskFileMainHeader(&diskFile, fileHeader);
	}

	if (diskFile.Opened())
		diskFile.Close();

	return derr;
}

// WritePGPdiskFileMainHeader(File *diskFile... ) writes the main PGPdisk
// file header.

DualErr
WritePGPdiskFileMainHeader(
	File					*diskFile, 
	const PGPdiskFileHeader	*fileHeader)
{
	DualErr derr;
	
	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);

	derr = WritePGPdiskFilePrimaryHeader(diskFile, fileHeader);

	if (derr.IsntError())
	{
		derr = WritePGPdiskFileAlternateHeader(diskFile, fileHeader);
	}
		
	return derr;
}

// WritePGPdiskFileMainHeader(LPCSTR path... ) is a wrapper function.

DualErr
WritePGPdiskFileMainHeader(LPCSTR path, const PGPdiskFileHeader *fileHeader)
{
	DualErr	derr;
	File	diskFile;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);

	derr = diskFile.Open(path, kOF_MustExist);

	if (derr.IsntError())
	{
		derr = WritePGPdiskFileMainHeader(&diskFile, fileHeader);
	}

	if (diskFile.Opened())
		diskFile.Close();

	return derr;
}

// ValidatePGPdisk checks if the file specified by 'path' is a valid PGPdisk.

DualErr 
ValidatePGPdisk(LPCSTR path)
{
	DualErr				derr;
	File				diskFile;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;
	PGPUInt64			bytesFile;
	
	pgpAssertStrValid(path);
	
	// Open the file.
	derr = diskFile.Open(path, kOF_MustExist | kOF_ReadOnly);

	// Get the file's length.
	if (derr.IsntError())
	{
		derr = diskFile.GetLength(&bytesFile);
	}

	// Close the file.
	if (diskFile.Opened())
	{
		diskFile.Close();
	}

	// Read in the header.
	if (derr.IsntError())
	{
		derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
		readHeader = derr.IsntError();
	}

	// Verify the header.
	if (derr.IsntError())
	{
		derr = VerifyPGPdiskFileMainHeader(fileHeader, bytesFile);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}


//////////////////////////////////////////
// High-level passphrase utility functions
//////////////////////////////////////////

// HowManyAlternatePassphrases returns the number of alternate passphrases in
// use on the specified PGPdisk.

DualErr 
HowManyAlternatePassphrases(LPCSTR path, PGPUInt32 *numAlts)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;
	
	pgpAssertStrValid(path);
	pgpAssertAddrValid(numAlts, PGPUInt32);

	// Read in the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Calculate number of alternate passphrases in use.
	if (derr.IsntError())
	{
		(* numAlts) = 0;

		for (PGPUInt32 i=0; i < kMaxAlternatePassphrases; i++)
		{
			if (fileHeader->otherPassphrases[i].inUse)
				(* numAlts)++;
		}
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// VerifyNormalPassphrase verifies a non-public-key passphrase.

DualErr
VerifyNormalPassphrase(LPCSTR path, SecureString *passphrase)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(passphrase, SecureString);

	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Check normal keys.
	if (derr.IsntError())
	{
		PGPUInt16 index;

		derr = FindMatchingPassphrase(fileHeader, passphrase, &index);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

#if !defined(PGPDISK_NOSDK)

// VerifyPublicKeyPassphrase verifies that a passphrase is a public key
// passphrase for a public key on the PGPdisk.

DualErr
VerifyPublicKeyPassphrase(LPCSTR path, SecureString *passphrase)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(passphrase, SecureString);

	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Check public keys.
	if (derr.IsntError())
	{
		PGPdiskPublicKeyHeader *pubKeyHdr;

		derr = FindPublicPGPdiskKeyHeader(path, passphrase, &pubKeyHdr);

		if (derr.IsntError())
			FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) pubKeyHdr);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

#endif	// !PGPDISK_NOSDK

// VerifyPassphrase verifies that a passphrase is a master or an alternate
// passphrase for the specified PGPdisk.

DualErr
VerifyPassphrase(LPCSTR path, SecureString *passphrase)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(passphrase, SecureString);

	// Check normal keys.
	if (derr.IsntError())
	{
		derr = VerifyNormalPassphrase(path, passphrase);
	}

#if !defined(PGPDISK_NOSDK)
	// Check public keys.
	if (derr.IsError())
	{
		derr = VerifyPublicKeyPassphrase(path, passphrase);
	}
#endif	// !PGPDISK_NOSDK

	return derr;
}

// VerifyMasterPassphrase verifies that a passphrase is the master passphrase
// for the specified PGPdisk.

DualErr
VerifyMasterPassphrase(LPCSTR path, SecureString *passphrase)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(passphrase, SecureString);
	
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	if (derr.IsntError())
	{
		derr = VerifyFileHeaderPassphrase(fileHeader, passphrase, 
			kMasterPassphraseIndex);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// InternalAddPassphrase is a helper function for AddPassphrase.

DualErr
InternalAddPassphrase(
	PGPdiskFileHeader	*fileHeader, 
	SecureString		*masterPassphrase, 
	SecureString		*newPassphrase, 
	PassphraseKey		*userInfo, 
	PGPBoolean			readOnly)
{
	CASTKey		*decryptedKey;
	DualErr		derr;

	SecureMemory	smDecryptedKey(sizeof(CASTKey));

	// Did we get our locked memory?
	derr = smDecryptedKey.mInitErr;

	if (derr.IsntError())
	{
		// Set the locked memory pointer.
		decryptedKey = (CASTKey *) smDecryptedKey.GetPtr();

		userInfo->inUse = TRUE;
		userInfo->readOnly = readOnly;

		// Decrypt the master key
		derr = DecryptPassphraseKey(&fileHeader->masterPassphrase, 
			&fileHeader->salt, masterPassphrase, decryptedKey);
	}

	if (derr.IsntError())
	{
		// Encrypt the decrypted key again using the user's passphrase.
		derr = EncryptPassphraseKey(decryptedKey, &fileHeader->salt, 
			newPassphrase, userInfo);
	}

	return derr;
}

// AddPassphrase adds a new alternate passphrase to the specified PGPdisk.

DualErr
AddPassphrase(
	LPCSTR			path, 
	SecureString	*masterPassphrase, 
	SecureString	*newPassphrase, 
	PGPBoolean		readOnly)
{
	DualErr				derr;
	PGPBoolean			didAdd, readHeader;
	PGPdiskFileHeader	*fileHeader;
	PGPUInt16			index;

	didAdd = readHeader = FALSE;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(masterPassphrase, SecureString);
	pgpAssertAddrValid(newPassphrase, SecureString);

	pgpAssert(VerifyMasterPassphrase(path, masterPassphrase).IsntError());
	pgpAssert(VerifyNormalPassphrase(path, newPassphrase).IsError());

	// Read in file header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Find a place to put the new passphrase and put it there.
	if (derr.IsntError())
	{
		for (index = 1; index <= kMaxAlternatePassphrases; index++)
		{
			PassphraseKey *userInfo;

			userInfo = (PassphraseKey *) GetPassphraseKeyPtr(fileHeader, 
				index);

			// We found a spot
			if (!userInfo->inUse)
			{
				derr = InternalAddPassphrase(fileHeader, masterPassphrase, 
					newPassphrase, userInfo, readOnly);

				didAdd = derr.IsntError();
				break;
			}
		}
	}

	if (derr.IsntError())
	{
		if (!didAdd)
			derr = DualErr(kPGDMinorError_AllAlternatesTaken);
	}

	// Write out the new header
	if (derr.IsntError())
	{
		derr = WritePGPdiskFileMainHeader(path, fileHeader);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// InternalChangePassphrase is a helper function for ChangePassphrase.

DualErr
InternalChangePassphrase(
	PassphraseKey			*userInfo, 
	const PassphraseSalt	*salt, 
	SecureString			*oldPassphrase, 
	SecureString			*newPassphrase)
{
	CASTKey	*decryptedKey;
	DualErr	derr;

	SecureMemory	smDecryptedKey(sizeof(CASTKey));

	pgpAssertAddrValid(userInfo, PassphraseKey);
	pgpAssertAddrValid(salt, PassphraseSalt);
	pgpAssertAddrValid(oldPassphrase, SecureString);
	pgpAssertAddrValid(newPassphrase, SecureString);

	pgpAssert(VerifyPassphraseKey(userInfo, salt, oldPassphrase).IsntError());

	// Did we get our locked memory?
	derr = smDecryptedKey.mInitErr;

	if (derr.IsntError())
	{
		// Set the locked memory pointer.
		decryptedKey = (CASTKey *) smDecryptedKey.GetPtr();

		// Decrypt the key
		derr = DecryptPassphraseKey(userInfo, salt, oldPassphrase, 
			decryptedKey);
	}

	if (derr.IsntError())
	{
		// Encrypt the decrypted key again using the user's passphrase.
		derr = EncryptPassphraseKey(decryptedKey, salt, newPassphrase, 
			userInfo);
	}

	return derr;
}

// ChangePassphrase changes an existing passphrase on the specified PGPdisk.

DualErr
ChangePassphrase(
	LPCSTR			path, 
	SecureString	*oldPassphrase, 
	SecureString	*newPassphrase)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;
	PGPUInt16			index;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(oldPassphrase, SecureString);
	pgpAssertAddrValid(newPassphrase, SecureString);

	// Read in file header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Find the index of this passphrase
	if (derr.IsntError())
	{
		derr = FindMatchingPassphrase(fileHeader, oldPassphrase, &index);
	}

	// Change the passphrase
	if (derr.IsntError())
	{
		PassphraseKey *userInfo;

		userInfo = (PassphraseKey *) GetPassphraseKeyPtr(fileHeader, index);

		derr = InternalChangePassphrase(userInfo, &fileHeader->salt, 
			oldPassphrase, newPassphrase);
	}

	// Set the modified header
	if (derr.IsntError())
	{
		derr = WritePGPdiskFileMainHeader(path, fileHeader);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// RemovePassphrase removes an alternate passphrase from the specified
// PGPdisk.

DualErr
RemovePassphrase(LPCSTR path, SecureString *passphrase)
{
	DualErr				derr;
	PGPBoolean			didRemove, readHeader;
	PGPdiskFileHeader	*fileHeader;

	didRemove = readHeader = FALSE;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(passphrase, SecureString);

	// Read in the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Remove the passphrase if we can find it.
	if (derr.IsntError())
	{
		for (PGPUInt16 index = 1; index <= kMaxAlternatePassphrases; ++index)
		{
			PassphraseKey *userInfo;

			userInfo = (PassphraseKey *) 
				GetPassphraseKeyPtr(fileHeader, index);

			// Is this passphrase in use?
			if (userInfo->inUse)
			{
				DualErr passMatches;

				passMatches = VerifyPassphraseKey(userInfo, 
					&fileHeader->salt, passphrase);

				// Does it match the passphrase we wish to remove?
				if (passMatches.IsntError())
				{
					// Update the fileheader.
					if (derr.IsntError())
					{
						pgpClearMemory(userInfo, sizeof(PassphraseKey));
						userInfo->inUse	= FALSE;

						derr = WritePGPdiskFileMainHeader(path, fileHeader);
						didRemove = derr.IsntError();
						break;
					}
				}
			}
		}
	}

	if (derr.IsntError())
	{
		if (!didRemove)
			derr = DualErr(kPGDMinorError_IncorrectPassphrase);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// RemoveAlternatePassphrases removes all alternate passphrases from the 
// specified PGPdisk.

DualErr
RemoveAlternatePassphrases(LPCSTR path)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);

	// Read in the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Remove all alternate passphrases.
	if (derr.IsntError())
	{
		for (PGPUInt16 index = 1; index <= kMaxAlternatePassphrases; ++index)
		{
			PassphraseKey *userInfo;

			userInfo = 
				(PassphraseKey *) GetPassphraseKeyPtr(fileHeader, index);

			// This clears the inUse flag as well
			pgpClearMemory(userInfo, sizeof(PassphraseKey));
			userInfo->inUse	= FALSE;
		}

		derr = WritePGPdiskFileMainHeader(path, fileHeader);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}


//////////////////////////////////
// Miscellaneous utility functions
//////////////////////////////////

// IsPGPdiskNotEncrypted returns TRUE if the specified PGPdisk was created
// without encryption, FALSE otherwise.

PGPBoolean 
IsPGPdiskNotEncrypted(LPCSTR path)
{
	DualErr				derr;
	PGPBoolean			isNotEncrypted, readHeader;
	PGPdiskFileHeader	*fileHeader;

	isNotEncrypted = readHeader = FALSE;

	pgpAssertStrValid(path);

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Determine if encryption was used.
	if (derr.IsntError())
	{
		isNotEncrypted = 
			(fileHeader->algorithm == kCopyDataEncryptionAlgorithm);
	}
	else
	{
		isNotEncrypted = FALSE;
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return isNotEncrypted;
}

// GetDriveLetterPref returns the preferred drive letter setting that is
// stored in the PGPdisk's header.

PGPUInt8 
GetDriveLetterPref(LPCSTR path)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPUInt8			drive;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Retrieve the drive letter field.
	if (derr.IsntError())
	{
		drive = fileHeader->drive;
	}
	else
	{
		drive = kInvalidDrive;
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return drive;
}

// SetDriveLetterPref alters the PGPdisk's header so it remembers the given
// drive letter as its preferred drive letter from now on.

DualErr 
SetDriveLetterPref(LPCSTR path, PGPUInt8 drive)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Alter the drive letter field and write out the header.
	if (derr.IsntError())
	{
		fileHeader->drive = drive;

		derr = WritePGPdiskFileMainHeader(path, fileHeader);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// GetPGPdiskMountedFlag returns the value of the 'mountedFlag' header
// variable.

PGPBoolean 
GetPGPdiskMountedFlag(LPCSTR path)
{
	DualErr				derr;
	PGPBoolean			isMounted, readHeader;
	PGPdiskFileHeader	*fileHeader;

	isMounted = readHeader = FALSE;

	pgpAssertStrValid(path);

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	if (derr.IsError())
	{
		isMounted = FALSE;
	}
	else
	{
		isMounted = fileHeader->mountedFlag;
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return isMounted;
}

// SetPGPdiskMountedFlag(File *diskFile... ) sets the value of the
// 'mountedFlag' header variable on the opened PGPdisk.

DualErr 
SetPGPdiskMountedFlag(File *diskFile, PGPBoolean mountedFlag)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(diskFile, &fileHeader);
	readHeader = derr.IsntError();

	// Set the value of the 'mountedFlag' variable.
	if (derr.IsntError())
	{
		fileHeader->mountedFlag = mountedFlag;
		derr = WritePGPdiskFileMainHeader(diskFile, fileHeader);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// SetPGPdiskMountedFlag(LPCSTR path... ) is a wrapper function.

DualErr 
SetPGPdiskMountedFlag(LPCSTR path, PGPBoolean mountedFlag)
{
	DualErr	derr;
	File	diskFile;

	pgpAssertStrValid(path);

	derr = diskFile.Open(path, kOF_MustExist);

	if (derr.IsntError())
	{
		derr = SetPGPdiskMountedFlag(&diskFile, mountedFlag);
	}

	if (diskFile.Opened())
		diskFile.Close();

	return derr;
}

// GetPGPdiskUniqueSessionId returns the value of the 'uniqueSessionId' header
// variable.

PGPUInt64 
GetPGPdiskUniqueSessionId(LPCSTR path)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;
	PGPUInt64			sessionId;

	pgpAssertStrValid(path);

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	if (derr.IsError())
	{
		sessionId = kInvalidSessionId;
	}
	else
	{
		sessionId = fileHeader->uniqueSessionId;
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return sessionId;
}

// MakePGPdiskUniqueSessionId sets the value of the 'uniqueSessionId' header
// variable.

DualErr 
SetPGPdiskUniqueSessionId(LPCSTR path, PGPUInt64 uniqueSessionId)
{
	DualErr				derr;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Set the value of the 'uniqueSessionId'.
	if (derr.IsntError())
	{
		fileHeader->uniqueSessionId = uniqueSessionId;
		derr = WritePGPdiskFileMainHeader(path, fileHeader);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// DoesPGPdiskHaveBadCAST returns TRUE if the specified PGPdisk needs to be
// converted to a newer version of CAST.

PGPBoolean 
DoesPGPdiskHaveBadCAST(LPCSTR path)
{
	DualErr				derr;
	PGPBoolean			hasBadCAST, readHeader;
	PGPdiskFileHeader	*fileHeader;

	hasBadCAST = readHeader = FALSE;

	pgpAssertStrValid(path);

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	if (derr.IsError())
	{
		hasBadCAST = FALSE;
	}
	else
	{
		hasBadCAST = (fileHeader->majorVersion <= 
			kPGPdiskConvertInProgressMajorVersion);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return hasBadCAST;
}

// WasPGPdiskConversionInterrupted returns TRUE if the specified PGPdisk
// is only partially converted to the newer version of CAST.

PGPBoolean 
WasPGPdiskConversionInterrupted(LPCSTR path)
{
	DualErr				derr;
	PGPBoolean			wasInterrupted, readHeader;
	PGPdiskFileHeader	*fileHeader;

	wasInterrupted = readHeader = FALSE;

	pgpAssertStrValid(path);

	// Read the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	if (derr.IsError())
	{
		wasInterrupted = FALSE;
	}
	else
	{
		wasInterrupted = (fileHeader->majorVersion == 
			kPGPdiskConvertInProgressMajorVersion);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return wasInterrupted;
}

#if !defined(PGPDISK_NOSDK)

// NukeAllNonADKHeaders deletes all non-ADK headers on the PGPdisk. If there
// are any ADK headers, it re-encrypts them.

DualErr 
NukeAllNonADKHeaders(File *diskFile, const CASTKey *newSessionKey)
{
	DualErr					derr;
	PGPBoolean				gotItemList, gotNewItemList;
	PGPdiskFileHeaderItem	*itemList, *newList;

	gotItemList = gotNewItemList = FALSE;

	PGPBoolean				foundADKToAdd, gotAllKeys;
	PGPByte					*exportedKeyID;
	PGPdiskFileHeaderItem	*curItem;
	PGPKeyID				keyID;
	PGPKeyRef				pubKey;
	PGPKeySetRef			allKeys;

	foundADKToAdd = gotAllKeys = FALSE;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(newSessionKey, CASTKey);

	// Get list of headers.
	derr = GetHeaderItemList(diskFile, &itemList);
	gotItemList = derr.IsntError();

	// Deal with ADKs.
	if (derr.IsntError())
	{
		curItem = itemList->next;

		// Open default key rings.
		derr = PGPOpenDefaultKeyRings(GetGlobalPGPContext(), 0, &allKeys);
			gotAllKeys = derr.IsntError();
		
		while (derr.IsntError() && IsntNull(curItem))
		{
			if (IsPublicKeyHeader(curItem->hdr))
			{
				PGPdiskPublicKeyHeader *pubKeyHdr;

				pubKeyHdr = (PGPdiskPublicKeyHeader *) curItem->hdr;

				// A locked key means ADK.
				if (pubKeyHdr->locked)
				{
					// Import key ID.
					exportedKeyID = (PGPByte *) pubKeyHdr + 
						pubKeyHdr->keyIDOffset;

					derr = PGPImportKeyID(exportedKeyID, &keyID);

					// Get actual key.
					if (derr.IsntError())
					{
						derr = PGPGetKeyByKeyID(allKeys, &keyID, 
							pubKeyHdr->algorithm, &pubKey);
					}

					if (derr.IsntError())
					{
						foundADKToAdd = TRUE;
					}

					break;
				}
			}

			curItem = curItem->next;
		}
	}

	// Construct new list.
	if (derr.IsntError())
	{
		derr = MakeHeaderItem(&newList);
		gotNewItemList = derr.IsntError();

		if (derr.IsntError())
		{
			derr = CopyHeader(itemList->hdr, &newList->hdr);
		}

		if (derr.IsntError())
		{
			newList->prev = newList->next = NULL;
		}
	}

	// Attach ADK.
	if (derr.IsntError() && foundADKToAdd)
	{
		PGPdiskPublicKeyHeader *newADKHeader;

		derr = CreatePublicKeyHeader(pubKey, newSessionKey, TRUE, TRUE, 
			&newADKHeader);

		if (derr.IsntError())
		{
			derr = InsertPGPdiskHeaderInList(newList, 
				(PGPdiskFileHeaderInfo *) newADKHeader);
		}
	}

	if (derr.IsntError())
	{
		UpdateHeaderItemList(diskFile, newList);
	}

	if (gotAllKeys)
		PGPFreeKeySet(allKeys);

	if (gotNewItemList)
		FreeHeaderItemList(newList);

	if (gotItemList)
		FreeHeaderItemList(itemList);

	return derr;
}

#endif	// !PGPDISK_NOSDK
//////////////////////////////////////////////////////////////////////////////
// PGPdisk.cpp
//
// Class PGPdisk definition.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdisk.cpp,v 1.9 1999/02/26 04:09:57 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <new.h>

#include "Required.h"

#include "pgpUtilities.h"

#include "CommonStrings.h"
#include "GlobalPGPContext.h"
#include "PGPdiskFileFormat.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskLowLevelUtils.h"
#include "PGPdiskPublicKeyUtils.h"
#include "SecureMemory.h"
#include "UtilityFunctions.h"
#include "Win32Utils.h"

#include "CipherContext.h"
#include "SHA.h"

#include "Globals.h"
#include "PGPdisk.h"


////////////
// Constants
////////////

const PGPUInt32 kNumBlocksToConvertAtATime = 128;

const PGPUInt32 kOffsetToCCIStruct = kDefaultBlockSize - 
	sizeof(PGPdiskCASTConvertInfo);


////////////////////////////////////////
// Class PGPdisk public member functions
////////////////////////////////////////

// The (LPCSTR, PGPUInt8) overloaded constructor attaches a drive and a
// pathname for an already mounted PGPdisk.

PGPdisk::PGPdisk(LPCSTR path, PGPUInt8 drive, PGPUInt64 sessionId)
	 : VolFile(path, drive)
{
	mProgressBarHwnd	= NULL;
	mPCancelFlag		= NULL;

	mInitErr = VolFile::mInitErr;

	// Read the session ID.
	if (mInitErr.IsntError())
	{
		if (IsntNull(path))
		{
			pgpAssert(sessionId != kInvalidSessionId);
			mUniqueSessionId = sessionId;
		}
	}
}

// BestGuessComparePaths uses a number of methods to determine if the
// specified file is the same as the current mounted PGPdisk.

PGPBoolean 
PGPdisk::BestGuessComparePaths(LPCSTR path)
{
	File		testFile;
	CString		csPath, PGPdiskBareName, testBareName;
	DualErr		derr;
	PGPBoolean	failedTest	= FALSE;
	PGPUInt32	i, indexLastPGPdiskPathSlash, indexLastTestFilePathSlash;
	PGPUInt32	numPGPdiskPathElems, numTestFilePathElems;
	CString		diskPath;
	CString		checkPath;

	pgpAssert(Mounted());

	// First try the easy case.
	if (ComparePaths(path))
		return TRUE;

	// Convert paths to canonical form.
	if (VerifyAndCanonicalizePath(path, &csPath).IsError())
		csPath = path;

	// See if the number of path elements in each path is the same.
	numPGPdiskPathElems = 0;

	for (i=0; i<strlen(diskPath); i++)
	{
		if ((diskPath.GetAt(i) == '\\') ||
			(diskPath.GetAt(i) == '/'))
		{
			indexLastPGPdiskPathSlash = i;
			numPGPdiskPathElems++;
		}
	}

	numTestFilePathElems = 0;

	for (i=0; i<strlen(checkPath); i++)
	{
		if ((checkPath.GetAt(i) == '\\') ||
			(checkPath.GetAt(i) == '/'))
		{
			indexLastTestFilePathSlash = i;
			numTestFilePathElems++;
		}
	}

	if (numPGPdiskPathElems != numTestFilePathElems)
		failedTest = TRUE;

	// Compare the file names.
	if (_stricmp(diskPath.Mid(indexLastPGPdiskPathSlash + 1), 
		checkPath.Mid(indexLastTestFilePathSlash + 1)) != 0)
	{
		failedTest = TRUE;
	}

	// Check if the test file is in use (which must be the case with any
	// mounted PGPdisk.

	if (derr.IsntError() && !failedTest)
	{
		if (!App->IsFileInUse(checkPath))
			failedTest = TRUE;
	}

	// Finally return false if there was an error.
	if (derr.IsError())
	{
		failedTest = TRUE;
	}

	return !failedTest;
}

// GetUniqueSessionId returns the unique session ID for the PGPdisk.

PGPUInt64 
PGPdisk::GetUniqueSessionId()
{
	pgpAssert(Mounted());

	return mUniqueSessionId;
}

// ReadInUniqueSessionId reads in the unique session ID from the PGPdisk.

void 
PGPdisk::ReadInUniqueSessionId(LPCSTR path)
{
	mUniqueSessionId = GetPGPdiskUniqueSessionId(path);
}

// MakeUniqueSessionId makes a unique session ID for the PGPdisk.

DualErr 
PGPdisk::MakeUniqueSessionId(LPCSTR path)
{
	FILETIME	fileTime;
	PGPUInt64	tenthMsSince1601;

	GetSystemTimeAsFileTime(&fileTime);

	tenthMsSince1601 = MakeQWord(fileTime.dwLowDateTime, 
		fileTime.dwHighDateTime);

	return SetPGPdiskUniqueSessionId(path, tenthMsSince1601);
}

// Mount asks the driver to mount the PGPdisk specified by 'path' with the
// specified password.

DualErr 
PGPdisk::Mount(
	LPCSTR			path, 
	SecureString	*passphrase, 
	PGPUInt8		drive, 
	PGPBoolean		mountReadOnly)
{
	AD_Mount			MNT;
	CASTKey				*decryptedKey;
	CipherContext		*pContext			= NULL;
	DualErr				derr;
	const PassphraseKey *userInfo;
	PGPBoolean			readOnlyDueToADK	= FALSE;
	PGPBoolean			readHeader			= FALSE;
	PGPdiskFileHeader	*fileHeader;
	PGPUInt16			index;

	SecureMemory	smDecryptedKey(sizeof(CASTKey));
	SecureMemory	smContext(sizeof(CipherContext));

	pgpAssertStrValid(path);
	pgpAssertAddrValid(passphrase, SecureString);

	pgpAssert(Unmounted());
	pgpAssert(!Opened());
	
	// Did we get our locked memory?
	derr = smDecryptedKey.mInitErr;

	if (derr.IsntError())
	{
		derr = smContext.mInitErr;
	}

	if (derr.IsntError())
	{
		// Initialize the pointers to our locked memory.
		decryptedKey	= (CASTKey *) smDecryptedKey.GetPtr();
		pContext		= new (smContext.GetPtr()) CipherContext;	// <new.h>

		// Set and get the session ID and the drive letter preference.
		if (!mountReadOnly)
		{
			MakeUniqueSessionId(path);
			SetDriveLetterPref(path, drive);
		}

		ReadInUniqueSessionId(path);
	}

	// Read the header.
	if (derr.IsntError())
	{
		derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
		readHeader = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Look for corresponding normal passphrase key.
		derr = FindMatchingPassphrase(fileHeader, passphrase, &index);

		if (derr.IsntError())
		{
			// Get a pointer to the passphrase info.
			userInfo = GetPassphraseKeyPtr(fileHeader, index);

			// Force read-only if the key is read-only.
			if (userInfo->readOnly)
				mountReadOnly = TRUE;

			// Decrypt the encrypted key.
			derr = DecryptPassphraseKey(userInfo, &fileHeader->salt, 
				passphrase, decryptedKey);
		}
		else
		{
			PGPBoolean				gotPubKeyHdr	= FALSE;
			PGPdiskPublicKeyHeader	*pubKeyHdr;

			// Look for corresponding public key header.
			derr = FindPublicPGPdiskKeyHeader(path, passphrase, &pubKeyHdr);
			gotPubKeyHdr = derr.IsntError();

			if (derr.IsntError())
			{
				// Force read-only if the key is read-only.
				if (pubKeyHdr->readOnly)
				{
					if (!mountReadOnly)
						readOnlyDueToADK = TRUE;

					mountReadOnly = TRUE;
				}

				// Get decrypted key.
				derr = GetDecryptedKeyUsingPublicKey(passphrase, pubKeyHdr, 
					decryptedKey);
			}

			if (gotPubKeyHdr)
				FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) pubKeyHdr);
		}
	}

	if (derr.IsntError())
	{
		// Initialize the cipher context.
		pContext->InitContext(fileHeader->algorithm, decryptedKey->keyBytes, 
			fileHeader->salt.saltBytes);

		// If NTFS perform checks.
		if (IsPGPdiskNTFS(path, pContext))
		{
			if (IsWin95CompatibleMachine())
			{
				derr = DualErr(kPGDMinorError_TriedNTFSOnWin95);
			}
			else if (mountReadOnly)
			{
				if (readOnlyDueToADK)
					mountReadOnly = FALSE;
				else
					derr = DualErr(kPGDMinorError_TriedReadOnlyNTFS);
			}
		}
	}

	if (derr.IsntError())
	{
		// Prepare the mount request packet; base classes will do the rest.
		MNT.pContext = pContext;

		// Call the request down.
		derr = VolFile::Mount(path, drive, mountReadOnly, &MNT);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	if (IsntNull(pContext))
		pContext->~CipherContext();

	return derr;
}

// Unmount asks the driver to unmount us.

DualErr 
PGPdisk::Unmount(PGPBoolean isThisEmergency)
{
	DualErr derr;

	pgpAssert(Mounted());

	// Call the request down.
	derr = VolFile::Unmount(isThisEmergency);

	return derr;
}

// CreatePGPdisk creates a new PGPdisk at the location specified by 'path'
// and with the desired free space specified in 'blocksData'.

DualErr 
PGPdisk::CreatePGPdisk(
	LPCSTR						path, 
	PGPUInt64					blocksDisk, 
	PGPdiskEncryptionAlgorithm	algorithm, 
	SecureString				*passphrase, 
	PGPUInt8					drive)
{
	DualErr				derr;
	PGPdiskFileHeader	*fileHeader;
	SecureMemory		smFileHeader(sizeof(PGPdiskFileHeader));

	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertStrValid(path);

	pgpAssert(algorithm != kInvalidEncryptionAlgorithm);
	pgpAssert(blocksDisk > 0);

	// Did we get our locked memory?
	derr = smFileHeader.mInitErr;

	if (derr.IsntError())
	{
		// Get a pointer to the locked memory.
		fileHeader = (PGPdiskFileHeader *) smFileHeader.GetPtr();

		// Create the file.
		DeleteFile(path);
		derr = Open(path, kOF_Trunc);
	}

	// Set its length.
	if (derr.IsntError())
	{
		PGPUInt64 blocksLength;

		blocksLength = kPGPdiskReservedHeaderBlocks + blocksDisk + 
			kPGPdiskAlternateHeaderBlocks;

		derr = SetLength(blocksLength*kDefaultBlockSize);
	}

	if (derr.IsntError())
	{
		// Initialize the PGPdisk file header.
		InitPGPdiskFileHeader(fileHeader, blocksDisk, algorithm, passphrase, 
			drive);
	}

	// Encrypt the disk.
	if (derr.IsntError())
	{
		derr = FirstTimeEncryptDiskFile(fileHeader, passphrase);
	}

	// Write the header out to the PGPdisk file. Need to do this
	if (derr.IsntError())
	{
		derr = FirstTimeWriteHeaderDiskFile(fileHeader);
	}

	// Close the file.
	if (Opened())
		Close();

	// Cleanup in case of error.
	if (derr.IsError())
	{
		DeleteFile(path);
	}

	MessageBeep(MB_OK);

	return derr;
}

// ConvertPGPdisk attempts to re-encrypt a broken CAST PGPdisk using the
// correct implementation of CAST.

DualErr 
PGPdisk::ConvertPGPdisk(LPCSTR path, SecureString *passphrase)
{
	DualErr					derr;
	CASTKey					*newSessionKey;
	PGPBoolean				conversionRestarting, readHeader;
	PGPdiskCASTConvertInfo	CCI;
	PGPdiskFileHeader		*fileHeader;

	SecureMemory	smFileHeader(sizeof(PGPdiskFileHeader));
	SecureMemory	smNewSessionKey(sizeof(CASTKey));

	conversionRestarting = readHeader = FALSE;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(passphrase, SecureString);

	// Did we get our locked memory?
	derr = smFileHeader.mInitErr;

	if (derr.IsntError())
	{
		derr = smNewSessionKey.mInitErr;
	}

	if (derr.IsntError())
	{
		// Set up secure pointers.
		fileHeader		= (PGPdiskFileHeader *) smFileHeader.GetPtr();
		newSessionKey	= (CASTKey *) smNewSessionKey.GetPtr();

		// Open the PGPdisk.
		derr = Open(path, kOF_MustExist);
	}

	// Read in main header.
	if (derr.IsntError())
	{
		derr = ReadPGPdiskFileMainHeader(this, &fileHeader);
		readHeader = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Are we restarting an already begun conversion?
		conversionRestarting = (fileHeader->majorVersion == 
			kPGPdiskConvertInProgressMajorVersion);

		if (conversionRestarting)
		{
			// Read CCI struct.
			derr = File::Read((PGPUInt8 *) &CCI, kOffsetToCCIStruct, 
				sizeof(CCI));

			// Verify the stored conversion information.
			if (derr.IsntError())
			{
				if (strncmp(CCI.convertInfoMagic, kPGPdiskBadCASTInfoMagic, 
					4) != 0)
				{
					derr = DualErr(kPGDMinorError_BadHeaderMagic);
				}
			}

			// Retrieve the new session key.
			if (derr.IsntError())
			{
				derr = DecryptPassphraseKey(GetPassphraseKeyPtr(fileHeader, 
					kMasterPassphraseIndex), &fileHeader->salt, passphrase, 
					newSessionKey);
			}
		}
		else
		{
			// Prepare the main header for conversion.
			fileHeader->majorVersion = 
				kPGPdiskConvertInProgressMajorVersion;

			// Clear alternate passphrases.
			pgpClearMemory(fileHeader->otherPassphrases, 
				sizeof(fileHeader->otherPassphrases));

			// Prepare conversion info structure.
			strncpy(CCI.convertInfoMagic, kPGPdiskBadCASTInfoMagic, 4);

			CCI.lastConvertedBlock = fileHeader->numHeaderBlocks - 1;
			CCI.oldSalt = fileHeader->salt;

			// Retrieve and save the old session key.
			derr = GetOldSessionKey(path, fileHeader, passphrase, 
				&CCI.oldSessionKey);

			// Put a new session key in the header.
			if (derr.IsntError())
			{	
				derr = InitHeaderWithNewSessionKey(fileHeader, 
					kCASTEncryptionAlgorithm, passphrase);
			}

			// Retrieve the new session key from the header.
			if (derr.IsntError())
			{
				derr = DecryptPassphraseKey(GetPassphraseKeyPtr(fileHeader, 
					kMasterPassphraseIndex), &fileHeader->salt, passphrase, 
					newSessionKey);
			}

			// Update main header.
			if (derr.IsntError())
			{
				derr = WritePGPdiskFileMainHeader(this, fileHeader);
			}

			if (derr.IsntError())
			{
				// Remove other headers (non-fatal).
				NukeAllNonADKHeaders(this, newSessionKey);

				// Write out CCI.
				derr = File::Write((PGPUInt8 *) &CCI, kOffsetToCCIStruct, 
					sizeof(CCI));
			}

			// Read in main header again.
			if (derr.IsntError())
			{
				FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

				derr = ReadPGPdiskFileMainHeader(this, &fileHeader);
				readHeader = derr.IsntError();
			}
		}
	}

	// Convert all non-converted blocks.
	if (derr.IsntError())
	{		
		PGPUInt32 numBlocks;

		numBlocks = fileHeader->numDataBlocks - 
			(CCI.lastConvertedBlock - fileHeader->numHeaderBlocks + 1);

		derr = ConvertBlocks(fileHeader, &CCI, &CCI.oldSessionKey, 
			&CCI.oldSalt, newSessionKey, &fileHeader->salt, 
			CCI.lastConvertedBlock + 1, numBlocks);
	}

	// Mark the PGPdisk as finished with the conversion process.
	if (derr.IsntError())
	{
		fileHeader->majorVersion = kPGPdiskPrimaryHeaderMajorVersion;
		fileHeader->minorVersion = kPGPdiskPrimaryHeaderMinorVersion;

		fileHeader->algorithm = kCASTEncryptionAlgorithm;

		derr = WritePGPdiskFileMainHeader(this, fileHeader);
	}

	if (derr.IsntError())
	{
		MessageBeep(MB_OK);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	if (Opened())
		Close();

	return derr;
}

// HighLevelFormatPGPdisk displays the format dialog. Failing that, it
// formats using our own algorithm.

DualErr 
PGPdisk::HighLevelFormatPGPdisk()
{
	DualErr derr;

	pgpAssert(Mounted());

	// Display the format dialog.
	derr = App->ShowWindowsFormatDialog(GetDrive());

	// If failed, warn user and format using our own algorithm if we can.
	if (derr.IsError())
	{
		ReportError(kPGDMajorError_WindowsFormatFailed, derr);
		derr = Format();

		// Set the volume label.
		if (derr.IsntError())
		{
			SetVolumeLabel(kPGPdiskDefaultVolLabel);
		}
	}

	return derr;
}

// SetProgressBarInfo is called to specify the window handle of a progress
// bar slider control to be used during disk creation, and also to specify
// a pointer to a shared boolean that will be polled to determine if the user
// has canceled the operation.

void 
PGPdisk::SetProgressBarInfo(HWND hWnd, PGPBoolean *pCancelFlag)
{
	pgpAssertAddrValid(pCancelFlag, PGPBoolean);

	mProgressBarHwnd = hWnd;
	mPCancelFlag = pCancelFlag;

	SetProgressBarRange(0, 100);
	SetProgressBarPos(0);
}


/////////////////////////////////////////
// Class PGPdisk private member functions
/////////////////////////////////////////

// HasUserCanceled checks the shared cancel variable to determine if the user
// has canceled the request.

PGPBoolean 
PGPdisk::HasUserCanceled()
{
	if (mProgressBarHwnd && mPCancelFlag)
	{
		pgpAssertAddrValid(mPCancelFlag, PGPBoolean);
		
		return (* mPCancelFlag);
	}
	else
	{
		return FALSE;
	}
}

// SetProgressBarPos sets the position of the disk creation progress bar.

void 
PGPdisk::SetProgressBarPos(PGPUInt32 nPos)
{
	if (mProgressBarHwnd)
		PostMessage(mProgressBarHwnd, PBM_SETPOS, nPos, 0);
}

// SetProgressBarRange sets the range of the disk creation progress bar.

void 
PGPdisk::SetProgressBarRange(PGPUInt32 nMin, PGPUInt32 nMax)
{
	if (mProgressBarHwnd)
	{
		PostMessage(mProgressBarHwnd, PBM_SETRANGE, 0, 
			MAKELPARAM(nMin, nMax));
	}
}

// IsNTFSPGPdisk checks if the PGPdisk is NTFS formatted.

PGPBoolean  
PGPdisk::IsPGPdiskNTFS(LPCSTR path, CipherContext *pContext)
{
	DualErr		derr;
	PGPBoolean	isNTFS = FALSE;
	PGPUInt8	*bootBlock;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(pContext, CipherContext);

	try
	{
		bootBlock = new PGPUInt8[kDefaultBlockSize];

		// Open the PGPdisk.
		derr = Open(path, kOF_MustExist | kOF_ReadOnly);

		// Read on the boot block.
		if (derr.IsntError())
		{
			derr = File::Read(bootBlock, 
				kPGPdiskReservedHeaderBlocks*kDefaultBlockSize, 
				kDefaultBlockSize);
		}

		if (derr.IsntError())
		{
			// Decrypt it.
			pContext->CipherBlocks(0, 1, bootBlock, bootBlock, 
				kCipherOp_Decrypt);

			// Is it NTFS?
			isNTFS = (strncmp((LPCSTR) &bootBlock[3], "NTFS", 4) == 0);
		}

		// Close the PGPdisk.
		if (Opened())
			Close();

		delete[] bootBlock;
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();

		if (Opened())
			Close();
	}

	return isNTFS;
}

// InitHeaderWithNewSessionKey initializes the header with a new session
// key generated from the specified master passphrase and random data.

DualErr 
PGPdisk::InitHeaderWithNewSessionKey(
	PGPdiskFileHeader			*fileHeader, 
	PGPdiskEncryptionAlgorithm	algorithm, 
	SecureString				*passphrase)
{
	DualErr				derr;
	EncryptedCASTKey	*key;
	ExpandedCASTKey		*expandedKey;
	PGPUInt8			*secureBuf;
	PGPUInt32			i, index;

	SecureMemory	smKey(sizeof(EncryptedCASTKey));
	SecureMemory	smExpandedKey(sizeof(ExpandedCASTKey));
	SecureMemory	smSecureBuf(kMaxPassphraseLength);

	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
	pgpAssertAddrValid(passphrase, SecureString);

	// Make sure we got our locked memory.
	derr = smKey.mInitErr;

	if (derr.IsntError())
	{
		derr = smExpandedKey.mInitErr;
	}

	if (derr.IsntError())
	{
		derr = smSecureBuf.mInitErr;
	}

	if (derr.IsntError())
	{
		// Initialize the pointers to our locked memory.
		key			= (EncryptedCASTKey *) smKey.GetPtr();
		expandedKey	= (ExpandedCASTKey *) smExpandedKey.GetPtr();
		secureBuf	= (PGPUInt8 *) smSecureBuf.GetPtr();

		// Initialize the salt with random data.
		PGPContextGetRandomBytes(GetGlobalPGPContext(), 
			fileHeader->salt.saltBytes, sizeof(fileHeader->salt));
		
		// Initialize the master passphrase with random data.
		PGPContextGetRandomBytes(GetGlobalPGPContext(), 
			fileHeader->masterPassphrase.encryptedKey.keyBytes, 
			sizeof(fileHeader->masterPassphrase.encryptedKey));
		
		// Hash, salt, and schedule the passphrase. Ask for the unexpanded
		// key since we will need it later.

		derr = HashSaltAndSchedule(passphrase, expandedKey, 
			&fileHeader->salt, &fileHeader->masterPassphrase.hashReps, key);
	}

	if (derr.IsntError())
	{
		// Encrypt the random key twice
		for (i = 0; i < 2; i++)
		{
			CAST5encrypt(
				&fileHeader->masterPassphrase.encryptedKey.keyBytes[0], 
				&fileHeader->masterPassphrase.encryptedKey.keyBytes[0], 
				expandedKey->keyDWords);

			CAST5encrypt(
				&fileHeader->masterPassphrase.encryptedKey.keyBytes[8],
				&fileHeader->masterPassphrase.encryptedKey.keyBytes[8],
				expandedKey->keyDWords);
		}

		// Encrypt the key itself for checkbytes purposes.
		CAST5encrypt(key->keyBytes, key->keyBytes, expandedKey->keyDWords);

		for (index = 0; 
			index < sizeof(fileHeader->masterPassphrase.checkBytes); 
			index++)
		{
			fileHeader->masterPassphrase.checkBytes.theBytes[index] = 
				key->keyBytes[index];
		}

		// Must mark as in use.
		fileHeader->masterPassphrase.inUse = TRUE;
	}

	return derr;
}

// InitPGPdiskFileHeader initializes a brand new PGPdisk file header.

DualErr 
PGPdisk::InitPGPdiskFileHeader(
	PGPdiskFileHeader			*fileHeader, 
	PGPUInt64					blocksDisk, 
	PGPdiskEncryptionAlgorithm	algorithm, 
	SecureString				*passphrase, 
	PGPUInt8					drive)
{
	DualErr derr;

	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
	pgpAssertAddrValid(passphrase, SecureString);

	pgpAssert(algorithm != kInvalidEncryptionAlgorithm);

	pgpAssert(blocksDisk > 0);
	pgpAssert(blocksDisk < 0xFFFFFFFF);	// header doesn't do 64-bit yet

	// Initialize the default fields of the header.
	pgpClearMemory(fileHeader, sizeof(PGPdiskFileHeader));	

	strncpy(fileHeader->headerInfo.headerMagic, kPGPdiskHeaderMagic, 4);
	strncpy(fileHeader->headerInfo.headerType, kPGPdiskPrimaryHeaderType, 4);
	fileHeader->headerInfo.headerSize = sizeof(PGPdiskFileHeader);

	fileHeader->majorVersion	= kPGPdiskPrimaryHeaderMajorVersion;
	fileHeader->minorVersion	= kPGPdiskPrimaryHeaderMinorVersion;

	fileHeader->algorithm = algorithm;

	fileHeader->numHeaderBlocks	= kPGPdiskReservedHeaderBlocks;
	fileHeader->numDataBlocks	= (PGPUInt32) blocksDisk;

	fileHeader->numFileBlocks = kPGPdiskReservedHeaderBlocks + 
		fileHeader->numDataBlocks + kPGPdiskAlternateHeaderBlocks;

	fileHeader->drive			= drive;
	fileHeader->mountedFlag		= FALSE;
	fileHeader->uniqueSessionId	= kInvalidSessionId;

	// Initialize the header with a new session key.
	derr = InitHeaderWithNewSessionKey(fileHeader, algorithm, passphrase);

	if (derr.IsntError())
	{
		// Update the CRC.
		UpdatePGPdiskFileHeaderCRC((PGPdiskFileHeaderInfo *) fileHeader);
	}

	return derr;
}

// FirstTimeEncryptDiskFile wipes the non-FAT blocks and encrypts all of the
// non-header blocks in a new, open PGPdisk.

DualErr 
PGPdisk::FirstTimeEncryptDiskFile(
	PGPdiskFileHeader	*fileHeader, 
	SecureString		*passphrase)
{
	CASTKey			*decryptedKey;
	CipherContext	*pContext		= NULL;
	DualErr			derr;
	PGPBoolean		createdWriteBuffer	= FALSE;
	PGPUInt8		*dataBuf;
	PGPUInt32		blocksBuf;

	SecureMemory	smDecryptedKey(sizeof(CASTKey));
	SecureMemory	smContext(sizeof(CipherContext));

	pgpAssert(Opened());
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
	pgpAssertAddrValid(passphrase, SecureString);

	// Make sure we got our locked memory.
	derr = smDecryptedKey.mInitErr;

	if (derr.IsntError())
	{
		derr = smContext.mInitErr;
	}

	if (derr.IsntError())
	{
		// Initialize the pointers to our locked memory.
		decryptedKey	= (CASTKey *) smDecryptedKey.GetPtr();
		pContext		= new (smContext.GetPtr()) CipherContext;	// <new.h>
		
		// Decrypt the encrypted key.
		derr = DecryptPassphraseKey(&fileHeader->masterPassphrase, 
			&fileHeader->salt, passphrase, decryptedKey);
	}

	if (derr.IsntError())
	{
		// Initialize the cipher context.
		pContext->InitContext(fileHeader->algorithm, decryptedKey->keyBytes, 
			fileHeader->salt.saltBytes);

		// Get the memory for our data buffer.
		derr = CreateReasonableWriteBuffer(fileHeader->numDataBlocks, 
			&dataBuf, &blocksBuf);

		createdWriteBuffer = derr.IsntError();
	}

	// Format the PGPdisk.
	if (derr.IsntError() && !HasUserCanceled())
	{
		PGPUInt32	blocksThisTime, endBlock, startBlock;
		PGPUInt32	totalWrites, writesDone, writesPerUpdate;

		// Calculate the starting and ending blocks.
		startBlock	= fileHeader->numHeaderBlocks;
		endBlock	= startBlock + fileHeader->numDataBlocks;

		// Calculate information used for progress bar updates.
		totalWrites = (PGPUInt32) 
			CeilDiv(endBlock - startBlock, blocksBuf);

		writesDone = 0;
		writesPerUpdate = (PGPUInt32) CeilDiv(totalWrites, 100);

		// Encrypt the PGPdisk.
		for (PGPUInt32 i = startBlock; i < endBlock; i += blocksBuf)
		{
			blocksThisTime = min(blocksBuf, endBlock - i);

			// Encrypt the blocks and output them.
			if (derr.IsntError())
			{
				pgpClearMemory(dataBuf, blocksThisTime*kDefaultBlockSize);

				pContext->CipherBlocks(i - fileHeader->numHeaderBlocks, 
					blocksThisTime, dataBuf, dataBuf, kCipherOp_Encrypt);

				derr = File::Write(dataBuf, i*kDefaultBlockSize, 
					blocksThisTime*kDefaultBlockSize);
			}

			// Update the progress dialog.
			if (derr.IsntError())
			{
				if (writesDone++ % writesPerUpdate == 0)
					SetProgressBarPos(writesDone*100/totalWrites);
			}

			if (derr.IsError() || HasUserCanceled())
			{
				break;
			}
		}
	}

	// If the user cancelled the creation, fail but don't be noisy.
	if (HasUserCanceled())
		derr = DualErr(kPGDMinorError_FailSilently);

	if (createdWriteBuffer)
		FreeByteBuffer(dataBuf);

	if (IsntNull(pContext))
		pContext->~CipherContext();

	return derr;
}

// FirstTimeWriteHeaderDiskFile clears the reserved headers blocks and writes
// out the header to the specified PGPdisk.

DualErr 
PGPdisk::FirstTimeWriteHeaderDiskFile(PGPdiskFileHeader* fileHeader)
{
	DualErr		derr;
	PGPUInt8	blanks[kDefaultBlockSize];
	PGPUInt32	i;

	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
	pgpAssert(Opened());

	pgpClearMemory(blanks, kDefaultBlockSize);

	// Clear the main header blocks.
	for (i = 0; i < fileHeader->numHeaderBlocks; i++)
	{
		derr = File::Write(blanks, i*kDefaultBlockSize, 
			kDefaultBlockSize);

		if (derr.IsError())
			break;
	}

	// Clear the alternate header blocks.
	if (derr.IsntError())
	{
		PGPUInt32 altStart;

		altStart = fileHeader->numHeaderBlocks + fileHeader->numDataBlocks;

		for (i=0; i<kPGPdiskAlternateHeaderBlocks; i++)
		{
			derr = File::Write(blanks, (altStart+i)*kDefaultBlockSize, 
				kDefaultBlockSize);

			if (derr.IsError())
				break;
		}
	}

	// Write out the header.
	if (derr.IsntError())
	{
		derr = WritePGPdiskFileMainHeader(this, fileHeader);
	}

	return derr;
}

// GetOldSessionKey extracts the old session key from a PGPdisk.

DualErr 
PGPdisk::GetOldSessionKey(
	LPCSTR				path, 
	PGPdiskFileHeader	*fileHeader, 
	SecureString		*passphrase, 
	CASTKey				*oldSessionKey)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(oldSessionKey, CASTKey);
	pgpAssert(Opened());

	// Must close file first.
	derr = Close();

	if (derr.IsntError())
	{
		if (ArePGPdiskPassesWiped(path))
		{
			PGPBoolean				gotPubKeyHdr	= FALSE;
			PGPdiskPublicKeyHeader	*pubKeyHdr;

			// Look for corresponding public key header.
			derr = FindPublicPGPdiskKeyHeader(path, passphrase, &pubKeyHdr);
			gotPubKeyHdr = derr.IsntError();

			if (derr.IsntError())
			{
				// Get decrypted key.
				derr = GetDecryptedKeyUsingPublicKey(passphrase, pubKeyHdr, 
					oldSessionKey);
			}

			if (gotPubKeyHdr)
				FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) pubKeyHdr);
		}
		else
		{
			// Get old session key from master passphrase.
			derr = DecryptPassphraseKey(GetPassphraseKeyPtr(fileHeader, 
				kMasterPassphraseIndex), &fileHeader->salt, passphrase, 
				oldSessionKey);
		}
	}

	if (derr.IsntError())
	{
		derr = Open(path, kOF_MustExist);
	}

	return derr;
}

// ConvertBlocks converts the specified blocks in the PGPdisk to the new
// session key.

DualErr 
PGPdisk::ConvertBlocks(
	PGPdiskFileHeader		*fileHeader, 
	PGPdiskCASTConvertInfo	*pCCI, 
	const CASTKey			*oldSessionKey, 
	const PassphraseSalt	*oldSalt, 
	const CASTKey			*newSessionKey, 
	const PassphraseSalt	*newSalt, 
	PGPUInt32				startingBlock, 
	PGPUInt32				numBlocks)
{
	CipherContext	*pNewContext, *pOldContext;
	DualErr			derr;
	PGPBoolean		createdWriteBuffer	= FALSE;
	PGPUInt8		*dataBuf;

	SecureMemory	smNewContext(sizeof(CipherContext));
	SecureMemory	smOldContext(sizeof(CipherContext));

	pNewContext = pOldContext = NULL;

	pgpAssert(Opened());
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
	pgpAssertAddrValid(pCCI, PGPdiskCASTConvertInfo);
	pgpAssertAddrValid(oldSessionKey, CASTKey);
	pgpAssertAddrValid(newSessionKey, CASTKey);

	// Make sure we got our locked memory.
	derr = smNewContext.mInitErr;

	if (derr.IsntError())
	{
		derr = smOldContext.mInitErr;
	}

	if (derr.IsntError())
	{
		// Initialize the pointers to our locked memory.
		pNewContext		= new (smNewContext.GetPtr()) CipherContext;
		pOldContext		= new (smOldContext.GetPtr()) CipherContext;

		// Initialize the cipher contexts.
		pNewContext->InitContext(kCASTEncryptionAlgorithm, 
			newSessionKey->keyBytes, newSalt->saltBytes);

		pOldContext->InitContext(kBrokenCASTEncryptionAlgorithm, 
			oldSessionKey->keyBytes, oldSalt->saltBytes);

		// Get the memory for our data buffer.
		derr = GetByteBuffer(kNumBlocksToConvertAtATime * kDefaultBlockSize, 
			&dataBuf);

		createdWriteBuffer = derr.IsntError();
	}

	// Convert the blocks.
	if (derr.IsntError() && !HasUserCanceled())
	{
		PGPUInt32	blocksBuf, blocksThisTime, endBlock, startBlock;
		PGPUInt32	totalWrites, writesDone, writesPerUpdate;

		blocksBuf = kNumBlocksToConvertAtATime;

		// Calculate the starting and ending blocks.
		startBlock	= startingBlock;
		endBlock	= startBlock + numBlocks;

		// Calculate information used for progress bar updates.
		totalWrites = (PGPUInt32) 
			CeilDiv(endBlock - startBlock, blocksBuf);

		writesDone = 0;
		writesPerUpdate = (PGPUInt32) CeilDiv(totalWrites, 100);

		// Encrypt the PGPdisk.
		for (PGPUInt32 i = startBlock; i < endBlock; i += blocksBuf)
		{
			blocksThisTime = min(blocksBuf, endBlock - i);

			// Read the blocks.
			derr = File::Read(dataBuf, i*kDefaultBlockSize, 
				blocksThisTime*kDefaultBlockSize);

			if (derr.IsntError())
			{
				// Decrypt the blocks.
				pOldContext->CipherBlocks(i - fileHeader->numHeaderBlocks, 
					blocksThisTime, dataBuf, dataBuf, kCipherOp_Decrypt);

				// Encrypt the blocks.
				pNewContext->CipherBlocks(i - fileHeader->numHeaderBlocks, 
					blocksThisTime, dataBuf, dataBuf, kCipherOp_Encrypt);

				// Write the blocks.
				derr = File::Write(dataBuf, i*kDefaultBlockSize, 
					blocksThisTime*kDefaultBlockSize);
			}

			// Write the updated block counter.
			if (derr.IsntError())
			{
				pCCI->lastConvertedBlock = i + blocksThisTime - 1;

				derr = File::Write((PGPUInt8 *) pCCI, kOffsetToCCIStruct, 
					sizeof(PGPdiskCASTConvertInfo));
			}

			// Update the progress dialog.
			if (derr.IsntError())
			{
				if (writesDone++ % writesPerUpdate == 0)
					SetProgressBarPos(writesDone*100/totalWrites);
			}

			if (derr.IsError() || HasUserCanceled())
			{
				break;
			}
		}
	}

	// If the user cancelled the creation, fail but don't be noisy.
	if (HasUserCanceled())
		derr = DualErr(kPGDMinorError_FailSilently);

	if (createdWriteBuffer)
		FreeByteBuffer(dataBuf);

	if (IsntNull(pOldContext))
		pOldContext->~CipherContext();

	if (IsntNull(pNewContext))
		pNewContext->~CipherContext();

	return derr;
}

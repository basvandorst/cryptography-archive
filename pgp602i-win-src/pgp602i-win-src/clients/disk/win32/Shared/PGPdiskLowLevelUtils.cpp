//////////////////////////////////////////////////////////////////////////////
// PGPdiskLowLevelUtils.cpp
//
// Low-level utility functions for working with PGPdisks.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskLowLevelUtils.cpp,v 1.1.2.27.2.1 1998/10/21 01:35:32 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"
#include <new.h>

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include "PlacementNew.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT
#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskLowLevelUtils.h"
#include "SecureMemory.h"
#include "UtilityFunctions.h"

#include "File.h"


/////////////////////////////////////
// Low-level header utility functions
/////////////////////////////////////

// MakeEmptyHeader creates a PGPdisk header of the specified size.

DualErr 
MakeEmptyHeader(PGPUInt32 headerSize, PGPdiskFileHeaderInfo **hdr)
{
	DualErr derr;

	pgpAssertAddrValid(hdr, PGPdiskFileHeaderInfo *);

#if defined(PGPDISK_MFC)

	try
	{
		(* hdr) = (PGPdiskFileHeaderInfo *) new PGPUInt8[headerSize];
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

#else	// !PGPDISK_MFC

	(* hdr) = (PGPdiskFileHeaderInfo *) new PGPUInt8[headerSize];

	if (IsNull((* hdr)))
		derr = DualErr(kPGDMinorError_OutOfMemory);

#endif	// PGPDISK_MFC

	return derr;
}

// CopyHeader copies a PGPdisk header.

DualErr 
CopyHeader(PGPdiskFileHeaderInfo *inHdr, PGPdiskFileHeaderInfo **outHdr)
{
	DualErr derr;

	pgpAssertAddrValid(inHdr, PGPdiskFileHeaderInfo);
	pgpAssertAddrValid(outHdr, PGPdiskFileHeaderInfo *);

	derr = MakeEmptyHeader(inHdr->headerSize, outHdr);

	if (derr.IsntError())
	{
		pgpCopyMemory(inHdr, (* outHdr), inHdr->headerSize);
	}

	return derr;
}

// FreeHeader frees a PGPdisk headder allocated with MakeEmptyHeader.

void 
FreeHeader(PGPdiskFileHeaderInfo *hdr)
{
	pgpAssertAddrValid(hdr, PGPdiskFileHeaderInfo);

	delete[] (PGPUInt8 *) hdr;
}

// ReadPGPdiskFileHeaderAtOffset reads in the header at the given offset.

DualErr 
ReadPGPdiskFileHeaderAtOffset(
	File					*diskFile, 
	PGPUInt64				pos, 
	PGPdiskFileHeaderInfo	**hdr)
{
	DualErr					derr;
	PGPBoolean				allocedFileHeader	= FALSE;
	PGPdiskFileHeaderInfo	hdrInfo;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(hdr, PGPdiskFileHeaderInfo *);

	// Read in next header.
	derr = diskFile->Read((PGPUInt8 *) &hdrInfo, pos, 
		sizeof(PGPdiskFileHeaderInfo));

	if (derr.IsntError())
	{
		if (strncmp(hdrInfo.headerMagic, kPGPdiskHeaderMagic, 4) != 0)
			derr = DualErr(kPGDMinorError_BadHeaderMagic);
	}

	// Allocate space for header.
	if (derr.IsntError())
	{
		derr = MakeEmptyHeader(hdrInfo.headerSize, hdr);
		allocedFileHeader = derr.IsntError();
	}

	// Read it in.
	if (derr.IsntError())
	{
		derr = diskFile->Read((PGPUInt8 *) (* hdr), pos, hdrInfo.headerSize);
	}

	// Cleanup on error.
	if (derr.IsError())
	{
		if (allocedFileHeader)
			FreeHeader((* hdr));
	}

	return derr;
}

// WritePGPdiskFileHeaderAtOffset writes a header at the given offset.

DualErr 
WritePGPdiskFileHeaderAtOffset(
	File					*diskFile, 
	PGPUInt64				pos, 
	PGPdiskFileHeaderInfo	*hdr)
{
	DualErr		derr;
	PGPUInt8	blanks[kDefaultBlockSize];

	pgpClearMemory(blanks, kDefaultBlockSize);

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(hdr, PGPdiskFileHeaderInfo);

	// Update header CRC.
	UpdatePGPdiskFileHeaderCRC(hdr);

	// Write out the header.
	derr = diskFile->Write((PGPUInt8 *) hdr, pos, hdr->headerSize);

	// Clear space up to block boundary.
	if (hdr->headerSize % kDefaultBlockSize > 0)
	{
		derr = diskFile->Write(blanks, pos + hdr->headerSize, 
			kDefaultBlockSize - hdr->headerSize%kDefaultBlockSize);
	}

	return derr;
}

// FreePGPdiskFileHeader frees a header allocated with
// ReadPGPdiskFileHeaderAtOffset.

void 
FreePGPdiskFileHeader(PGPdiskFileHeaderInfo *hdr)
{
	pgpAssertAddrValid(hdr, PGPdiskFileHeaderInfo);

	FreeHeader(hdr);
}

// MakeHeaderItem simply allocates a new header item.

DualErr 
MakeHeaderItem(PGPdiskFileHeaderItem **item)
{
	DualErr derr;

	pgpAssertAddrValid(item, PGPdiskFileHeaderItem *);

#if defined(PGPDISK_MFC)

	try
	{
		(* item) = new PGPdiskFileHeaderItem;
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

#else	// !PGPDISK_MFC

	(* item) = new PGPdiskFileHeaderItem;

	if (IsNull((* item)))
		derr = DualErr(kPGDMinorError_OutOfMemory);

#endif	// PGPDISK_MFC

	return derr;
}

// FreeHeaderItem frees an item allocated with MakeHeaderItem.

void 
FreeHeaderItem(PGPdiskFileHeaderItem *item)
{
	pgpAssertAddrValid(item, PGPdiskFileHeaderItem *);

	delete item;
}

// InsertPGPdiskHeaderInList inserts a header item in a header item list.

DualErr 
InsertPGPdiskHeaderInList(
	 PGPdiskFileHeaderItem	*curItem, 
	 PGPdiskFileHeaderInfo	*newHdr)
{
	DualErr					derr;
	PGPdiskFileHeaderItem	*newItem;

	pgpAssertAddrValid(curItem, PGPdiskFileHeaderItem *);
	pgpAssertAddrValid(newHdr, PGPdiskFileHeader *);

	// Get a new item.
	derr = MakeHeaderItem(&newItem);

	// Adjust the list.
	if (derr.IsntError())
	{
		newItem->prev = curItem;
		newItem->next = curItem->next;

		newItem->hdr = newHdr;

		if (IsntNull(newItem->next))
			newItem->next->prev = newItem;

		curItem->next = newItem;
	}

	return derr;
}

// RemovePGPdiskHeaderFromList removes a header item in a header item list.

void 
RemovePGPdiskHeaderFromList(PGPdiskFileHeaderItem **itemToRemove)
{
	PGPdiskFileHeaderItem *prevItem, *nextItem;

	pgpAssertAddrValid(itemToRemove, PGPdiskFileHeaderItem *);

	prevItem = (* itemToRemove)->prev;
	nextItem = (* itemToRemove)->next;

	// Remove the item from the linked list.
	if (IsntNull(prevItem))
		prevItem->next = nextItem;

	if (IsntNull(nextItem))
		nextItem->prev = prevItem;

	// Free the item.
	FreeHeaderItem((* itemToRemove));

	(* itemToRemove) = (IsntNull(prevItem) ? prevItem : nextItem);
}

// GetHeaderItemList constructs an item list of the headers in the PGPdisk.

DualErr
GetHeaderItemList(File *diskFile, PGPdiskFileHeaderItem **itemList)
{
	DualErr					derr;
	PGPdiskFileHeaderInfo	*hdr;
	PGPdiskFileHeaderItem	*curItem, *firstItem, *nextItem;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(itemList, PGPdiskFileHeaderItem *);

	// Read in main header.
	derr = ReadPGPdiskFileMainHeader(diskFile, (PGPdiskFileHeader **) &hdr);

	// Set it as first item.
	if (derr.IsntError())
	{
		derr = MakeHeaderItem(&firstItem);
	}

	if (derr.IsntError())
	{
		firstItem->next = firstItem->prev = NULL;
		firstItem->hdr = hdr;
	}

	// Read in remaining headers.
	if (derr.IsntError())
	{
		PGPBoolean headersLeft = TRUE;

		curItem = firstItem;

		do
		{
			PGPUInt64 pos;

			// Get position of next header.
			pos = curItem->hdr->nextHeaderOffset;

			if (pos == 0)
			{
				// There is no next header.
				headersLeft = FALSE;
			}
			else
			{
				PGPBoolean readHeader = FALSE;

				// Read in next header.
				derr = ReadPGPdiskFileHeaderAtOffset(diskFile, pos, &hdr);
				readHeader = derr.IsntError();

				// Insert it into the list.
				if (derr.IsntError())
				{
					derr = InsertPGPdiskHeaderInList(curItem, hdr);
				}

				// Iterate to next item.
				if (derr.IsntError())
				{
					curItem = curItem->next;
				}

				// Cleanup on error.
				if (derr.IsError())
				{
					if (readHeader)
						FreePGPdiskFileHeader(hdr);
				}
			}
		}
		while (derr.IsntError() && headersLeft);

		if (derr.IsntError())
		{
			curItem->next = NULL;
			(* itemList) = firstItem;
		}
	}

	// Cleanup on error.
	if (derr.IsError())
	{
		curItem = firstItem;

		while (IsntNull(curItem))
		{
			nextItem = curItem->next;

			FreePGPdiskFileHeader(curItem->hdr);
			FreeHeaderItem(curItem);

			curItem = nextItem;
		}
	}

	return derr;
}

// FreeHeaderItemList frees a list created with GetHeaderItemList.

void 
FreeHeaderItemList(PGPdiskFileHeaderItem *itemList)
{
	PGPdiskFileHeaderItem *curItem, *nextItem;

	curItem = itemList;

	while (IsntNull(curItem))
	{
		pgpAssertAddrValid(curItem, PGPdiskFileHeaderItem);
		pgpAssertAddrValid(curItem->hdr, PGPdiskFileHeaderInfo);

		nextItem = curItem->next;

		FreePGPdiskFileHeader(curItem->hdr);
		FreeHeaderItem(curItem);

		curItem = nextItem;
	}
}

// UpdateHeaderItemList writes out a header list to a PGPdisk.

DualErr
UpdateHeaderItemList(File *diskFile, PGPdiskFileHeaderItem *itemList)
{
	DualErr					derr;
	PGPdiskFileHeader		*mainHdr;
	PGPdiskFileHeaderItem	*curItem, *nextItem;
	PGPUInt32				blockAfterData, i;
	PGPUInt64				length;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(itemList, PGPdiskFileHeaderItem *);
	pgpAssertAddrValid(itemList->hdr, PGPdiskFileHeaderInfo);

	mainHdr = (PGPdiskFileHeader *) itemList->hdr;
	blockAfterData = mainHdr->numHeaderBlocks + mainHdr->numDataBlocks;

	// Set file size to minimum.
	derr = diskFile->SetLength((PGPUInt64) 
		blockAfterData * kDefaultBlockSize);

	// Wipe reserved header blocks.
	if (derr.IsntError())
	{
		PGPUInt8 blanks[kDefaultBlockSize];

		pgpClearMemory(blanks, kDefaultBlockSize);

		for (i = 0; i < mainHdr->numHeaderBlocks; i++)
		{
			derr = diskFile->Write(blanks, i*kDefaultBlockSize, 
				kDefaultBlockSize);

			if (derr.IsError())
				break;
		}
	}

	// Assume always enough room to store main header in beginning.
	if (derr.IsntError())
	{
		PGPBoolean	usedUpReservedBlocks	= FALSE;
		PGPUInt32	blocksTaken, nextBlock, remainingBlocks;

		curItem = itemList;

		nextBlock = 0;
		remainingBlocks = mainHdr->numHeaderBlocks;

		// Calculate new offsets for each header.
		while (IsntNull(curItem))
		{
			nextItem = curItem->next;

			if (IsNull(nextItem))
			{
				curItem->hdr->nextHeaderOffset = 0;
			}
			else
			{

				pgpAssertAddrValid(nextItem, PGPdiskFileHeaderItem);
				pgpAssertAddrValid(nextItem->hdr, PGPdiskFileHeaderInfo);

				blocksTaken = (PGPUInt32) CeilDiv(curItem->hdr->headerSize, 
					kDefaultBlockSize);

				// Any space left at beginning of file?
				if (!usedUpReservedBlocks)
				{
					if (blocksTaken <= remainingBlocks)
					{
						remainingBlocks -= blocksTaken;
						nextBlock += blocksTaken;

						// OK to put at current offset.
						curItem->hdr->nextHeaderOffset = nextBlock * 
							kDefaultBlockSize;
					}
					else
					{
						nextBlock = blockAfterData + blocksTaken;
						usedUpReservedBlocks = TRUE;

						// Exhausted bytes at beginning of file.
						if (IsntNull(curItem->prev))
						{
							curItem->prev->hdr->nextHeaderOffset = 
								blockAfterData * kDefaultBlockSize;
						}

						curItem->hdr->nextHeaderOffset = nextBlock * 
							kDefaultBlockSize;
					}
				}
				else
				{
					nextBlock += blocksTaken;

					// Already after end of file.
					curItem->hdr->nextHeaderOffset = 
						nextBlock * kDefaultBlockSize;
				}
			}

			curItem = nextItem;
		}
	}

	// Write out all the headers.
	if (derr.IsntError())
	{
		PGPUInt64 curOffset;

		curItem = itemList;
		curOffset = 0;

		while (derr.IsntError() && IsntNull(curItem))
		{
			derr = WritePGPdiskFileHeaderAtOffset(diskFile, curOffset, 
				curItem->hdr);

			curOffset = curItem->hdr->nextHeaderOffset;
			curItem = curItem->next;
		}
	}

	// Restore main and alternate header.
	if (derr.IsntError())
	{
		derr = diskFile->GetLength(&length);
	}

	if (derr.IsntError())
	{
		mainHdr->numFileBlocks = (PGPUInt32) (length/kDefaultBlockSize + 
			kPGPdiskAlternateHeaderBlocks);

		derr = diskFile->SetLength(length + kPGPdiskAlternateHeaderBlocks *
			kDefaultBlockSize);
	}

	if (derr.IsntError())
	{
		derr = WritePGPdiskFileMainHeader(diskFile, mainHdr);
	}

	return derr;
}

// ReadPGPdiskFilePrimaryHeader reads in and verifies the primary PGPdisk
// header.

DualErr
ReadPGPdiskFilePrimaryHeader(File *diskFile, PGPdiskFileHeader **fileHeader)
{
	DualErr		derr;
	PGPBoolean	readHeader	= FALSE;
	PGPUInt64	bytesFile;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader *);

	derr = ReadPGPdiskFileHeaderAtOffset(diskFile, 0, 
		(PGPdiskFileHeaderInfo **) fileHeader);

	readHeader = derr.IsntError();

	if (derr.IsntError())
	{
		derr = diskFile->GetLength(&bytesFile);
	}

	if (derr.IsntError())
	{
		derr = VerifyPGPdiskFileMainHeader((* fileHeader), bytesFile);
	}

	if (derr.IsError())
	{
		if (readHeader)
			FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) (* fileHeader));
	}

	return derr;
}

// WritePGPdiskFilePrimaryHeader verifies and writes out the primary PGPdisk
// file header to a PGPdisk file.

DualErr
WritePGPdiskFilePrimaryHeader(
	File					*diskFile, 
	const PGPdiskFileHeader	*fileHeader)
{
	DualErr		derr;
	PGPUInt64	bytesFile;
	
	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);

	derr = diskFile->GetLength(&bytesFile);

	if (derr.IsntError())
	{
		derr = WritePGPdiskFileHeaderAtOffset(diskFile, 0, 
			(PGPdiskFileHeaderInfo *) fileHeader);
	}

	return derr;
}

// ReadPGPdiskFileAlternateHeader reads in and verifies the alternate PGPdisk
// header.

DualErr
ReadPGPdiskFileAlternateHeader(File *diskFile, PGPdiskFileHeader **fileHeader)
{
	DualErr		derr;
	PGPBoolean	readHeader	= FALSE;
	PGPUInt64	bytesFile;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader *);

	derr = diskFile->GetLength(&bytesFile);

	if (derr.IsntError())
	{
		derr = ReadPGPdiskFileHeaderAtOffset(diskFile, 
			bytesFile - kPGPdiskAlternateHeaderSize, 
			(PGPdiskFileHeaderInfo **) fileHeader);

		readHeader = derr.IsntError();
	}

	if (derr.IsError())
	{
		if (readHeader)
			FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) (* fileHeader));
	}

	return derr;
}

// WritePGPdiskFileAlternateHeader verifies and writes out the alternate
// PGPdisk file header to a PGPdisk file.

DualErr
WritePGPdiskFileAlternateHeader(
	File					*diskFile, 
	const PGPdiskFileHeader	*fileHeader)
{
	DualErr		derr;
	PGPUInt64	bytesFile;
	
	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);

	derr = diskFile->GetLength(&bytesFile);

	if (derr.IsntError())
	{
		// Alternate header is ALWAYS 'kPGPdiskAlternateHeaderSize' bytes
		// from the end of the file.

		derr = WritePGPdiskFileHeaderAtOffset(diskFile, 
			bytesFile - kPGPdiskAlternateHeaderSize, 
			(PGPdiskFileHeaderInfo *) fileHeader);
	}
	
	return derr;
}

// VerifyPGPdiskFileMainHeader verifies certain fields of the given PGPdisk
// file header for a file of the given size.

DualErr 
VerifyPGPdiskFileMainHeader(
	const PGPdiskFileHeader	*fileHeader,
	PGPUInt64				bytesFile)
{
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
		
	if (strncmp(fileHeader->headerInfo.headerMagic, kPGPdiskHeaderMagic, 4) 
		!= 0)
	{
		return DualErr(kPGDMinorError_BadHeaderMagic);
	}

	if (fileHeader->headerInfo.headerSize > sizeof(PGPdiskFileHeader))
	{
		return DualErr(kPGDMinorError_BadHeaderSize);
	}

	if (fileHeader->headerInfo.headerCRC != 
		ComputePGPdiskFileHeaderCRC((PGPdiskFileHeaderInfo *) fileHeader))
	{
		return DualErr(kPGDMinorError_BadHeaderCRC);
	}

	if (strncmp(fileHeader->headerInfo.headerType, 
		kPGPdiskPrimaryHeaderType, 4) != 0)
	{
		return DualErr(kPGDMinorError_InvalidHeaderType);
	}

	if (fileHeader->majorVersion > kPGPdiskPrimaryHeaderMajorVersion)
	{
		return DualErr(kPGDMinorError_NewerHeaderVersion);
	}

	if ((fileHeader->algorithm != kBrokenCASTEncryptionAlgorithm) && 
		(fileHeader->algorithm != kCASTEncryptionAlgorithm) && 
		(fileHeader->algorithm != kCopyDataEncryptionAlgorithm))
	{
		return DualErr(kPGDMinorError_InvalidAlgorithm);
	}

	if ((PGPUInt64) fileHeader->numFileBlocks*kDefaultBlockSize != bytesFile)
	{
		return DualErr(kPGDMinorError_InvalidTotalFileSize);
	}

	if (fileHeader->numDataBlocks > fileHeader->numFileBlocks)
	{
		return DualErr(kPGDMinorError_InvalidFileDataSize);
	}

	if (fileHeader->numHeaderBlocks + fileHeader->numDataBlocks > 
		fileHeader->numFileBlocks)
	{
		return DualErr(kPGDMinorError_InvalidHeaderBlocks);
	}

	return DualErr::NoError;
}

// ComputePGPdiskFileHeaderCRC computes and returns a checksum over the given
// PGPdisk file header.

CRC32 
ComputePGPdiskFileHeaderCRC(PGPdiskFileHeaderInfo *fileHeader)
{
	CRC32	saveCRCValue;
	CRC32	crc;
	
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeaderInfo);

	// Always set CRC value to zero before computing.
	saveCRCValue = fileHeader->headerCRC;
	
	fileHeader->headerCRC = 0;
	
	crc = ComputeCRC32((const PGPUInt32 *) fileHeader, 
		fileHeader->headerSize / sizeof(PGPUInt32));
	
	fileHeader->headerCRC = saveCRCValue;
	
	return crc;
}

// UpdatePGPdiskFileHeaderCRC stores the checksum of the PGPdisk file header
// in its 'headerCRC' member.

void 
UpdatePGPdiskFileHeaderCRC(PGPdiskFileHeaderInfo *fileHeader)
{
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeaderInfo);

	fileHeader->headerCRC = ComputePGPdiskFileHeaderCRC(fileHeader);
}


/////////////////////////////////////////
// Low-level passphrase utility functions
/////////////////////////////////////////

// FindMatchingPassphrase determines if the specified passphrase is
// legitimate for any of the keys.

DualErr 
FindMatchingPassphrase(
	const PGPdiskFileHeader	*fileHeader, 
	SecureString			*passphrase, 
	PGPUInt16				*index)
{
	DualErr		derr;
	PGPBoolean	foundKey	= FALSE;
	
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(index, PGPUInt16);

	pgpAssert(kMasterPassphraseIndex == 0);

	for (PGPUInt16 keyIndex = 0; keyIndex <= kMaxAlternatePassphrases; 
		++keyIndex)
	{
		const PassphraseKey *userInfo = NULL;

		userInfo = GetPassphraseKeyPtr(fileHeader, keyIndex);

		if (userInfo->inUse)
		{
			derr = VerifyPassphraseKey(userInfo, &fileHeader->salt, 
				passphrase);

			foundKey = derr.IsntError();

			if (foundKey)
			{
				(* index) = keyIndex;
				break;
			}
		}
	}

	if (!foundKey)
		derr = DualErr(kPGDMinorError_IncorrectPassphrase);

	return derr;
}

// GetPassphraseKeyPtr returns a pointer to the PassphraseKey corresponding to
// the index.

const PassphraseKey * 
GetPassphraseKeyPtr(
	const PGPdiskFileHeader	*fileHeader, 
	PGPUInt16				whichPassphrase)
{
	const PassphraseKey *userInfo = NULL;
	
	// 0 being master, 1...kMaxAlternatePassphrases alternates.
	pgpAssert(whichPassphrase <= kMaxAlternatePassphrases);
		
	if (whichPassphrase == kMasterPassphraseIndex)
		userInfo = &fileHeader->masterPassphrase;
	else
		userInfo = &fileHeader->otherPassphrases[whichPassphrase-1];
			
	return userInfo;
}

// VerifyPassphraseKey verifies that the specified passphrase unlocks the
// specified key.

DualErr 
VerifyPassphraseKey(
	const PassphraseKey		*userInfo, 
	const PassphraseSalt	*salt, 
	SecureString			*passphrase)
{
	DualErr	derr;
	CASTKey	*decryptedKey;

	SecureMemory smDecryptedKey(sizeof(CASTKey));

	pgpAssertAddrValid(userInfo, PassphraseKey);
	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(salt, PassphraseSalt);
	pgpAssert(userInfo->inUse);

	// Did we get our locked memory?
	derr = smDecryptedKey.mInitErr;

	if (derr.IsntError())
	{
		// Set the locked memory pointer.
		decryptedKey = (CASTKey *) smDecryptedKey.GetPtr();

		// Attempt to decypt the key
		derr = DecryptPassphraseKey(userInfo, salt, passphrase,
			decryptedKey);
	}

	return derr;
}

// VerifyFileHeaderPassphrase verifies that a passphrase is valid for the
// specified passphrase index in the specified header.

DualErr
VerifyFileHeaderPassphrase(
	const PGPdiskFileHeader	*fileHeader,
	SecureString			*passphrase,
	PGPUInt16				whichKey)
{
	DualErr				derr;
	const PassphraseKey	*userInfo;
	
	pgpAssertAddrValid(fileHeader, PGPdiskFileHeader);
	pgpAssertAddrValid(passphrase, SecureString);
	
	userInfo = GetPassphraseKeyPtr(fileHeader, whichKey);

	if (!userInfo->inUse)
		derr = DualErr(kPGDMinorError_IncorrectPassphrase);

	if (derr.IsntError())
	{
		derr = VerifyPassphraseKey(userInfo, &fileHeader->salt, passphrase);
	}

	return derr;
}

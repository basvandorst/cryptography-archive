//////////////////////////////////////////////////////////////////////////////
// PGPdiskLowLevelUtils.h
//
// Declarations for PGPdiskLowLevelUtils.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskLowLevelUtils.h,v 1.6 1998/12/14 19:01:06 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef	Included_PGPdiskLowLevelUtils_h	// [
#define	Included_PGPdiskLowLevelUtils_h

#include "FatUtils.h"
#include "PGPdiskFileFormat.h"

#include "Cast5.h"
#include "CipherUtils.h"


////////
// Types
////////

class File;

typedef struct PGPdiskFileHeaderItem
{
	PGPdiskFileHeaderItem	*next;
	PGPdiskFileHeaderItem	*prev;

	PGPdiskFileHeaderInfo	*hdr;

} PGPdiskFileHeaderItem;


/////////////////////
// Exported functions
/////////////////////

DualErr					MakeEmptyHeader(PGPUInt32 headerSize, 
							PGPdiskFileHeaderInfo **hdr);

DualErr					CopyHeader(PGPdiskFileHeaderInfo *inHdr, 
							PGPdiskFileHeaderInfo **outHdr);

void					FreeHeader(PGPdiskFileHeaderInfo *hdr);

DualErr					ReadPGPdiskFileHeaderAtOffset(File *diskFile, 
							PGPUInt64 pos, PGPdiskFileHeaderInfo **hdr);

DualErr					WritePGPdiskFileHeaderAtOffset(File *diskFile, 
							PGPUInt64 pos, PGPdiskFileHeaderInfo *hdr);

void					FreePGPdiskFileHeader(PGPdiskFileHeaderInfo *hdr);

DualErr					MakeHeaderItem(PGPdiskFileHeaderItem **item);
void					FreeHeaderItem(PGPdiskFileHeaderItem *item);

DualErr					InsertPGPdiskHeaderInList(
							PGPdiskFileHeaderItem *curItem, 
							PGPdiskFileHeaderInfo *newHdr);

void					RemovePGPdiskHeaderFromList(
							PGPdiskFileHeaderItem **itemToRemove);

DualErr					GetHeaderItemList(File *diskFile, 
							PGPdiskFileHeaderItem **itemList);

void					FreeHeaderItemList(PGPdiskFileHeaderItem *itemList);

DualErr					UpdateHeaderItemList(File *diskFile, 
							PGPdiskFileHeaderItem *itemList);

DualErr					ReadPGPdiskFilePrimaryHeader(File *diskFile, 
							PGPdiskFileHeader **fileHeader);

DualErr					WritePGPdiskFilePrimaryHeader(File *diskFile, 
							const PGPdiskFileHeader *fileHeader);

DualErr					ReadPGPdiskFileAlternateHeader(File *diskFile, 
							PGPdiskFileHeader **fileHeader);

DualErr					WritePGPdiskFileAlternateHeader(File *diskFile, 
							const PGPdiskFileHeader *fileHeader);

DualErr					VerifyPGPdiskFileMainHeader(
							const PGPdiskFileHeader *fileHeader, 
							PGPUInt64 bytesFile);

CRC32					ComputePGPdiskFileHeaderCRC(
							PGPdiskFileHeaderInfo *fileHeader);

void					UpdatePGPdiskFileHeaderCRC(
							PGPdiskFileHeaderInfo *fileHeader);

DualErr					FindMatchingPassphrase(
							const PGPdiskFileHeader *fileHeader, 
							SecureString *passphrase, PGPUInt16 *index);

const PassphraseKey *	GetPassphraseKeyPtr(
							const PGPdiskFileHeader *fileHeader, 
							PGPUInt16 whichPassphrase);

DualErr					VerifyPassphraseKey(const PassphraseKey *userInfo, 
							const PassphraseSalt *salt, 
							SecureString *passphrase);

DualErr					VerifyFileHeaderPassphrase(
							const PGPdiskFileHeader *fileHeader,
							SecureString *passphrase, PGPUInt16 whichKey);

#endif	// ] Included_PGPdiskLowLevelUtils_h

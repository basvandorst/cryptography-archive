//////////////////////////////////////////////////////////////////////////////
// PGPdiskFileFormat.h
//
// The file format of PGPdisks.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskFileFormat.h,v 1.9 1999/02/27 06:15:54 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef	Included_PGPdiskFileFormat_h	// [
#define	Included_PGPdiskFileFormat_h

#if !defined(PGPDISK_NOSDK)
#include "pgpPubTypes.h"
#endif	// !PGPDISK_NOSDK

#include "FatUtils.h"

#include "Cast5.h"
#include "CipherUtils.h"

// Align to 1.
#pragma pack(push, 1)

////////////
// Constants
////////////

static LPCSTR	kPGPdiskHeaderMagic			= "PGPd";
static LPCSTR	kPGPdiskPrimaryHeaderType	= "MAIN";

static LPCSTR	kPGPdiskBadCASTInfoMagic	= "BADC";

const PGPUInt16	kPGPdiskPrimaryHeaderMajorVersion 	= 4;
const PGPUInt16	kPGPdiskPrimaryHeaderMinorVersion 	= 0;

const PGPUInt16 kPGPdiskBadCASTMaxHeaderMajorVersion = 2;
const PGPUInt16 kPGPdiskConvertInProgressMajorVersion = 3;

const PGPUInt32	kPGPdiskReservedHeaderBlocks	= 12;
const PGPUInt32	kPGPdiskAlternateHeaderBlocks	= 1;
const PGPUInt32	kPGPdiskAlternateHeaderSize		=  
	kPGPdiskAlternateHeaderBlocks * kDefaultBlockSize;

const PGPUInt16	kMasterPassphraseIndex		= 0;
const PGPUInt16	kMaxAlternatePassphrases	= 7;

const PGPUInt64 kInvalidSessionId = 0xFFFFFFFFFFFFFFFF;


////////
// Types
////////

// PGPdiskEncryptionAlgorithm describes valid and invalid encryption
// algorithms for PGPdisks.

enum PGPdiskEncryptionAlgorithm
{
	kInvalidEncryptionAlgorithm		= 0, 
	kCopyDataEncryptionAlgorithm	= 1, 
	kBrokenCASTEncryptionAlgorithm	= 2, 
	kCASTEncryptionAlgorithm		= 3, 

	kDefaultEncryptionAlgorithm		= kCASTEncryptionAlgorithm, 
	kDummyEncryptionAlgorithm 		= ULONG_MAX		// force size to 4 bytes

};

// PGPdiskFileHe         aderInfo stores info about the structure of a PGPdisk's
// header.

typedef struct PGPdiskFileHeaderInfo
{
	char		headerMagic[4];		// Magic field. Always kPGPdiskHeaderMagic
	char		headerType[4];		// kPGPdiskPrimaryHeaderType + ???
	PGPUInt32	headerSize;			// Total size of this header, in bytes
	CRC32		headerCRC;			// CRC of this header
	PGPUInt64	nextHeaderOffset;	// Offset to next header from file start
									// 0 = no additional headers
	PGPUInt32	reserved[2];

} PGPdiskFileHeaderInfo;

// CheckBytes contains data that lets us verify a passphrase is valid.

typedef struct CheckBytes
{
	PGPUInt8 theBytes[8];

} CheckBytes;

// PassphraseKey contains the encrypted key for the PGPdisk.

typedef struct PassphraseKey
{
	EncryptedCASTKey	encryptedKey;	// session encrypted with salted pass
	CheckBytes			checkBytes;		// first 8b of enc-hash-salt user key
	PGPUInt16			hashReps;		// # of hashes on passphrase
	PGPUInt16			inUse		: 1;
	PGPUInt16			readOnly	: 1;
	PGPUInt16			unused		: 14;
	PGPUInt32			unusedLong;

} PassphraseKey;

// A PGPdiskFileHeader describes the attributes of a PGPdisk.

typedef struct PGPdiskFileHeader
{
	PGPdiskFileHeaderInfo		headerInfo;

	PGPUInt8	majorVersion;
	PGPUInt8	minorVersion;
	PGPUInt16	reserved;

	PGPUInt32	numFileBlocks;			// Size of the file, in blocks
	PGPUInt32	numHeaderBlocks;		// Size of header data
	PGPUInt32	numDataBlocks;			// Size of the encrypted data

	PGPdiskEncryptionAlgorithm algorithm;	// encryption method (4 bytes)

	PGPUInt8	drive;					// preferred drive letter
	PGPBoolean	mountedFlag;			// set while disk is mounted
	PGPUInt64	uniqueSessionId;		// random session ID
	PGPUInt8	reservedBytes[54];		// reserved for future use

	PassphraseSalt	salt;				// salt for passphrase and IVs
	PassphraseKey	masterPassphrase;
	PassphraseKey	otherPassphrases[kMaxAlternatePassphrases];

} PGPdiskFileHeader;

#if !defined(PGPDISK_NOSDK)

static LPCSTR	kPGPdiskPublicKeyHeaderType			= "PKEY";
const PGPUInt16	kPGPdiskPublicKeyHeaderMajorVersion	= 1;
const PGPUInt16	kPGPdiskPublicKeyHeaderMinorVersion	= 0;

// A PGPdiskPublicKeyHeader describes a public-key-encrypted session key.

typedef struct PGPdiskPublicKeyHeader
{
	PGPdiskFileHeaderInfo	headerInfo;
	
	PGPUInt8				majorVersion;
	PGPUInt8				minorVersion;
	PGPUInt16				reserved;

	PGPUInt32				readOnly 	: 1;
	PGPUInt32				locked 		: 1;
	PGPUInt32				unused		: 30;
	
	CheckBytes				checkBytes;
	
	PGPUInt32				keyIDOffset;
	PGPUInt32				keyIDSize;
	
	PGPUInt32				encryptedKeyOffset;
	PGPUInt32				encryptedKeySize;
	
	PGPPublicKeyAlgorithm	algorithm;
	
	PGPUInt32				reserved2[10];

	// Exported key ID here
	// Encrypted key here.
	
} PGPdiskPublicKeyHeader;

#endif	// !PGPDISK_NOSDK

// PGPdiskCASTConvertInfo is placed at the end of the first sector and keeps
// track of the conversion of a PGPdisk with bad CAST.

typedef struct PGPdiskCASTConvertInfo
{
	char			convertInfoMagic[4];	// kPGPdiskBadCASTInfoMagic

	PGPUInt32		lastConvertedBlock;		// index of last converted block
	CASTKey			oldSessionKey;			// the old session key
	PassphraseSalt	oldSalt;				// the old salt

} PGPdiskCASTConvertInfo;

// Restore alignment.
#pragma pack(pop)


////////////////////
// Utility functions
////////////////////

#if !defined(PGPDISK_NOSDK)

// IsPublicKeyHeader returns TRUE if the given header is a public key header, 
// FALSE otherwise.

inline 
PGPBoolean 
IsPublicKeyHeader(PGPdiskFileHeaderInfo *hdrInfo)
{
	pgpAssertAddrValid(hdrInfo, PGPdiskFileHeaderInfo);

	return (strncmp(hdrInfo->headerType, kPGPdiskPublicKeyHeaderType, 4) == 
		0);
}

#endif	// !PGPDISK_NOSDK

#endif	// ] Included_PGPdiskFileFormat_h

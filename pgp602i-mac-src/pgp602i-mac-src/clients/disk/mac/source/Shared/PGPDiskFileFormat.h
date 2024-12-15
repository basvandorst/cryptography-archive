/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: PGPDiskFileFormat.h,v 1.6.8.1 1998/11/12 03:06:08 heller Exp $____________________________________________________________________________*/#pragma once#include <limits.h>#include "MacFiles.h"#include "MacNumerics.h"#if PGPDISK_PUBLIC_KEY#include "pgpPubTypes.h"#endif#include "CipherContext.h"#pragma options align=mac68k	// [const OSType	kPGPDiskHeaderMagic					= 'PGPD';const OSType	kPGPDiskPrimaryHeaderType			= 'MAIN';const UInt16	kPGPDiskPrimaryHeaderMajorVersion 	= 2;const UInt16	kPGPDiskPrimaryHeaderMinorVersion 	= 1;const UInt32	kPGPDiskReservedHeaderBlocks	= 12;const UInt32	kPGPDiskAlternateHeaderBlocks	= 1;const UInt32	kPGPDiskAlternateHeaderSize		=						kPGPDiskAlternateHeaderBlocks * kDiskBlockSize;typedef enum{	kInvalidEncryptionAlgorithm		= 0,	kCopyDataEncryptionAlgorithm	= 1,	kCASTEncryptionAlgorithm		= 2,	kDefaultEncryptionAlgorithm		= kCASTEncryptionAlgorithm,	// force size of enum to be 4 bytes	kDummyEncryptionAlgorithm 		= ULONG_MAX} PGPDiskEncryptionAlgorithm;typedef struct PGPDiskFileHeaderInfo{	OSType			headerMagic;		// Always kPGPDiskHeaderMagic	OSType			headerType;			// kPGPDiskPrimaryHeaderType + ???	UInt32			headerSize;			// Total size of this header, in bytes.	CRC32			headerCRC;			// CRC of this header	UnsignedWide	nextHeaderOffset;	// Offset to next header from										// beginning of file, in bytes										// 0 = no additional headers	ulong			reserved[ 2 ];	} PGPDiskFileHeaderInfo;typedef struct CheckBytes{	uchar	theBytes[8];} CheckBytes;typedef struct PassphraseKey{	// session key encrypted with salted passphrase	EncryptedCASTKey	encryptedKey;			// first 8 bytes of encrypted hashed salted user key	CheckBytes			checkBytes;		// introduced in v1.1, # of hashes on passphrase	short				hashReps;		UInt16				inUse 		: 1;	UInt16				readOnly	: 1;	UInt16				unused : 14;	UInt32				unusedLong;} PassphraseKey;const ushort	kNumPassphrases			= 8; // master + 1 thru 7const ushort	kMasterPassphraseIndex	= 0;typedef struct PGPDiskFileHeader{	PGPDiskFileHeaderInfo	headerInfo;	UInt8			majorVersion;	UInt8			minorVersion;	UInt16			reserved;	// Total size of the file, in blocks (eof == numFileBlocks * 512)	UInt32			numFileBlocks;			// Size of the header data at the beginning of the file, in blocks	UInt32			numHeaderBlocks;		// Total size of the encrypted data, in blocks	UInt32			numDataBlocks;			 	// encryption method (4 bytes)	PGPDiskEncryptionAlgorithm 	algorithm;			UInt32			numPassphrases;	ulong			reservedLongs[ 15 ];		// random data used to salt passphrase and IVs	PassphraseSalt	salt;									// All of the individual passphrase info for one user	PassphraseKey	passphrases[kNumPassphrases];	} PGPDiskFileHeader;#if PGPDISK_PUBLIC_KEY	// [const OSType	kPGPDiskPublicKeyHeaderType			= 'PKEY';const OSType	kPGPDiskPublicKeyHeaderMajorVersion	= 1;const OSType	kPGPDiskPublicKeyHeaderMinorVersion	= 0;typedef struct PGPDiskPublicKeyHeader{	PGPDiskFileHeaderInfo	headerInfo;		UInt8					majorVersion;	UInt8					minorVersion;	UInt16					reserved;	UInt32					readOnly 	: 1;	UInt32					locked 		: 1;	UInt32					unused		: 30;		CheckBytes				checkBytes;		UInt32					keyIDOffset;	UInt32					keyIDSize;		UInt32					encryptedKeyOffset;	UInt32					encryptedKeySize;		PGPPublicKeyAlgorithm	algorithm;		UInt32					reserved2[10];	// Exported key ID here	// Encrypted key here.	} PGPDiskPublicKeyHeader;#endif	// ] PGPDISK_PUBLIC_KEY#pragma options align=reset	// ]
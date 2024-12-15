//////////////////////////////////////////////////////////////////////////////
// SystemConstants.h
//
// General constants.
//////////////////////////////////////////////////////////////////////////////

// $Id: SystemConstants.h,v 1.7 1998/12/15 01:25:05 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_SystemConstants_h	// [
#define Included_SystemConstants_h


////////
// Types
////////

#if !defined(LPSTR)
typedef char *LPSTR;
#endif // LPSTR

#if !defined(LPCSTR)
typedef const char *LPCSTR;
#endif // LPCSTR

#if !defined(LPCWSTR)
typedef const WCHAR *LPCWSTR;
#endif // LPCWSTR


////////////
// Constants
////////////

// Memory constants.

static PGPUInt32 kWin32Power2PageSize = 12;

// Character constants.
const char kNullChar = '\0';

// String constants.

static LPCSTR kEmptyString = "";

static LPCWSTR	kNTDevicePathPrefix		= L"\\Device\\";
static LPCWSTR	kNTLinkPathPrefix		= L"\\??\\";
static LPCWSTR	kNTUNCLinkPathPrefix	= L"\\Device\\mup\\";

// Kernel-mode constants.

const PGPUInt32 kPGPdiskBugCheckCode = 0x20000042;

// Drive constants

static LPCSTR kInvalidVolumeNameChars = "\"&*+./:;<=>?[\\]^|";

const PGPUInt16	kMaxSpts	= 64;
const PGPUInt8	kMaxSpt		= 63;
const PGPUInt16	kMaxHeads	= 256;
const PGPUInt8	kMaxHead	= 255;
const PGPUInt16	kMaxCyls	= 256;
const PGPUInt8	kMaxCyl		= 255;

const PGPUInt32	kSizeSpt	= 1;
const PGPUInt32	kSizeHead	= kSizeSpt*kMaxSpt;
const PGPUInt32	kSizeCyl	= kSizeHead*kMaxHeads;

const PGPUInt32	kMaxDrives				= 26;		
const PGPUInt32	kMaxMountedPGPdisks		= kMaxDrives;
const PGPUInt32	kMaxVolumeLabelLength	= 11;
const PGPUInt8	kInvalidDrive			= 0xFF;		// invalid drive #
const PGPUInt32	kMaxStringSize			= 300;		// > MAX_PATH

#if defined(MAX_PATH)
const PGPUInt32	kMaxPathSize	= MAX_PATH;
#else	// !MAX_PATH
const PGPUInt32	kMaxPathSize	= 260;
#endif	// MAX_PATH

// Conversion constants

const PGPUInt8	kBitsPerByte	= 8;
const PGPUInt8	kBitsPerWord	= 16;
const PGPUInt8	kBitsPerDWord	= 32;

const PGPUInt16	kBytesPerKb		= 1024;
const PGPUInt32	kBytesPerMeg	= 1024*1024;
const PGPUInt16	kKbPerMeg		= 1024;
const PGPUInt32	kKbPerGig		= 1024*kKbPerMeg;
const PGPUInt32	kMbPerGig		= 1024;

// Time constants

const PGPUInt32	k100nsPerSecond	= 10000000;
const PGPUInt64	kSecondsInADay	= 60*60*24;

const PGPUInt64 kSecsBetween1601And1970 = 0x00000002b6109100;

#endif	// ] Included_SystemConstants_h

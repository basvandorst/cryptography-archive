//////////////////////////////////////////////////////////////////////////////
// PGPdiskDefaults.h
//
// Default values for new PGPdisks.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskDefaults.h,v 1.13 1998/12/14 19:01:02 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskDefaults_h	// [
#define Included_PGPdiskDefaults_h


////////////////////////////
// PGPdisk default constants
////////////////////////////

#if PGPDISK_BETAVERSION

// Beta timeout constants

const PGPUInt16	kPGPdiskBetaTimeoutYear		= 1998;
const PGPUInt16	kPGPdiskBetaTimeoutMonth	= 9;		// June
const PGPUInt16	kPGPdiskBetaTimeoutDay		= 15;

#elif PGPDISK_DEMOVERSION

// Demo timeout constants.
const PGPUInt16 kDemoTimeoutDays = 60;

#endif // PGPDISK_BETAVERSION

// Max number of apps.
const PGPUInt32 kMaxPGPdiskApps = 32;

// Make PGPdisk custom Windows messages unique.
const PGPUInt16 kPGPdiskMessageMagic = 0xAB42;

// Names.

static LPCSTR	kPGPdiskAppName		= "PGPdisk";
static LPCSTR	kPGPdiskDriverName	= "\\\\.\\PGPdisk";
static LPCSTR	kPGPdiskServiceName	= "PGPdisk";

// File and volume defaults for new PGPdisks.

static LPCSTR	kPGPdiskFileExtension	= ".pgd";
static LPCSTR	kDefaultNewDiskName		= "New PGPdisk%s.pgd";
static LPCSTR	kPGPdiskDefaultVolLabel	= "NEW PGPDISK";

const PGPUInt32	kPGPdiskMinKbSizeWin95	= 100;
const PGPUInt32	kPGPdiskMaxKbSizeWin95	= 2047*kKbPerMeg;
const PGPUInt32	kPGPdiskMinKbSizeWinNT	= 100;
const PGPUInt32	kPGPdiskMaxKbSizeWinNT	= 1024*kKbPerGig;
const PGPUInt32	kDefaultPGPdiskMbSize	= 100;

// Default FAT values for new PGPdisks

static LPCSTR	kDefaultOEMName		= "PGPDISK ";
static LPCSTR	kDefaultVolLabel	= "UNNAMED    ";

const PGPUInt16	kDefaultBlockSize		= 512;
const PGPUInt8	kDefaultActiveFat		= 1;
const PGPUInt8	kDefaultFatCount		= 2;
const PGPUInt32	kDefaultFat12Reserved	= 1;
const PGPUInt32	kDefaultFat16Reserved	= 1;
const PGPUInt32	kDefaultFat32Reserved	= 32;
const PGPUInt16	kDefaultRootDirEnts		= 512;
const PGPUInt32	kDefaultRootDirStart	= 2;
const PGPUInt32	kDefaultBigFatStart		= 1;

// Passphrase specific constants.

const PGPUInt32	kMaxPassphraseLength	= 130;
const PGPUInt32	kMinPassphraseLength	= 8;

// Default values for preferences.

const PGPBoolean	kDefaultAutoUnmount				= FALSE;
const PGPBoolean	kDefaultHotKeyEnabled			= FALSE;
const PGPBoolean	kDefaultUseAsyncIO				= TRUE;
const PGPUInt32		kDefaultHotKeyCode				= 0;
const PGPBoolean	kDefaultUnmountOnSleep			= FALSE;
const PGPBoolean	kDefaultNoSleepIfUnmountFail	= FALSE;
const PGPUInt32		kDefaultUnmountTimeout			= 15;
const PGPUInt32		kMaxUnmountTimeout				= 999;

// Miscellaneous.

const PGPUInt32 kMaxCheckOpenFilesAttempts = 10;

#endif	// ] Included_PGPdiskDefaults_h

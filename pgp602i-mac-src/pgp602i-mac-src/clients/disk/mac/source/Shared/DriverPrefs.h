/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: DriverPrefs.h,v 1.3.8.1 1998/11/12 03:06:05 heller Exp $____________________________________________________________________________*/#pragma once#pragma options align=mac68k	// [typedef struct PGPDiskDriverPrefs{	enum { kDefaultIdleSeconds = 15 * 60UL };	// 15 minutes	Boolean		unmountDrivesOnPBSleep;		Boolean		unmountAfterIdleTime;	ulong		unmountIdleTimeSeconds;		ulong		reserved[ 2 ];} PGPDiskDriverPrefs;const PGPDiskDriverPrefs kDefaultPGPDiskDriverPrefs =	{	TRUE,					// unmountDrivesOnPBSleep	TRUE,					// unmountAfterIdleTime	PGPDiskDriverPrefs::kDefaultIdleSeconds,	// unmountIdleTimeSeconds	{0,}	};#pragma options align=reset	// ]
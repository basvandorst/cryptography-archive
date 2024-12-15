//////////////////////////////////////////////////////////////////////////////
// WindowsVersion.cpp
//
// Functions for determining the currently running version of Windows.
//////////////////////////////////////////////////////////////////////////////

// $Id: WindowsVersion.cpp,v 1.1.2.5.2.1 1998/08/21 05:57:01 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#else
#error Define PGPDISK_MFC.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "UtilityFunctions.h"
#include "WindowsVersion.h"


////////
// Types
////////

// Enumerates the currently running version of Windows.

enum WindowsVersion {kWV_Win95OSR1, kWV_Win95OSR2, kWV_Win98OrFuture, 
	kWV_WinNT3, kWV_WinNT4NoSp, kWV_WinNT4Sp1, kWV_WinNT4Sp2, kWV_WinNT4Sp3, 
	kWV_WinNT4PostSp3, kWV_WinNTPost4, kWV_WinUnknown, kWV_Uninitialized};


////////////
// Functions
////////////

// GetWindowsVersion returns the currently running version of Windows.

WindowsVersion 
GetWindowsVersion()
{
	OSVERSIONINFO verInfo;

	verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!::GetVersionEx(&verInfo))
		return kWV_WinUnknown;

	switch (verInfo.dwPlatformId)
	{

	case VER_PLATFORM_WIN32_WINDOWS:
		if (verInfo.dwMinorVersion == 0)
		{
			if (GetLowWord(verInfo.dwBuildNumber) > 1080)
			{
				return kWV_Win95OSR2;
			}
			else
			{
				return kWV_Win95OSR1;
			}
		}
		else
		{
			return kWV_Win98OrFuture;
		}

	case VER_PLATFORM_WIN32_NT:
		if (verInfo.dwMajorVersion == 3)
		{
			return kWV_WinNT3;
		}
		else if (verInfo.dwMajorVersion == 4)
		{
			if (strlen(verInfo.szCSDVersion) == 0)
			{
				return kWV_WinNT4NoSp;
			}
			else if (strcmp(verInfo.szCSDVersion, "Service Pack 1") == 0)
			{
				return kWV_WinNT4Sp1;
			}
			else if (strcmp(verInfo.szCSDVersion, "Service Pack 2") == 0)
			{
				return kWV_WinNT4Sp2;
			}
			else if (strcmp(verInfo.szCSDVersion, "Service Pack 3") == 0)
			{
				return kWV_WinNT4Sp3;
			}
			else
			{
				return kWV_WinNT4PostSp3;
			}
		}
		else
		{
			return kWV_WinNTPost4;
		}

	default:
		return kWV_WinUnknown;
	}
}

// IsWin95CompatibleMachine returns TRUE if the user is running a Win95
// compatible machine.

PGPBoolean 
IsWin95CompatibleMachine()
{
	switch (GetWindowsVersion())
	{
		case kWV_Win95OSR1:
		case kWV_Win95OSR2:
		case kWV_Win98OrFuture:
			return TRUE;

		default:
			return FALSE;
	}
}

// IsWin95OSR2CompatibleMachine returns TRUE if the user is running a Win95
// OSR2 compatible machine.

PGPBoolean 
IsWin95OSR2CompatibleMachine()
{
	switch (GetWindowsVersion())
	{
		case kWV_Win95OSR2:
		case kWV_Win98OrFuture:
			return TRUE;

		default:
			return FALSE;
	}
}

// IsWin98CompatibleMachine returns TRUE if the user is running a Win98
// compatible machine.

PGPBoolean 
IsWin98CompatibleMachine()
{
	switch (GetWindowsVersion())
	{
		case kWV_Win98OrFuture:
			return TRUE;

		default:
			return FALSE;
	}
}

// IsWinNT4CompatibleMachine returns TRUE if the user is running a version of
// Windows NT greater than version 3.

PGPBoolean 
IsWinNT4CompatibleMachine()
{
	switch (GetWindowsVersion())
	{
		case kWV_WinNT4NoSp:
		case kWV_WinNT4Sp1:
		case kWV_WinNT4Sp2:
		case kWV_WinNT4Sp3:
		case kWV_WinNT4PostSp3:
		case kWV_WinNTPost4:
			return TRUE;

		default:
			return FALSE;
	}
}

// IsWinNT5CompatibleMachine returns TRUE if the user is running a version of
// Windows NT greater than version 4.

PGPBoolean 
IsWinNT5CompatibleMachine()
{
	switch (GetWindowsVersion())
	{
		case kWV_WinNTPost4:
			return TRUE;

		default:
			return FALSE;
	}
}

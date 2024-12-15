//____________________________________________________________________________
//	Copyright (C) 1998 Network Associates Inc. and affiliated companies.
//	All rights reserved.
//	
//	oscheck.c -- code used for OS version checking
//
//  Author: Philip Nathan
//
//Id: oscheck.c,v 1.1 1999/02/10 00:06:08 philipn Exp $_______________________

#include <windows.h>
#include "prototype.h"


GetWindowsVersion()
{
	OSVERSIONINFO	verInfo;

	verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&verInfo))
		return WinUnknown;

	switch (verInfo.dwPlatformId)
	{

	case VER_PLATFORM_WIN32_WINDOWS:
		if (verInfo.dwMinorVersion == 0)
		{
			if (GetLowWord(verInfo.dwBuildNumber) > 1080)
			{
				return Win95OSR2;
			}
			else
			{
				return Win95OSR1;
			}
		}
		else
		{
			return Win98OrFuture;
		}

	case VER_PLATFORM_WIN32_NT:
		if (verInfo.dwMajorVersion == 3)
		{
			return WinNT3;
		}
		else if (verInfo.dwMajorVersion == 4)
		{
			if (strlen(verInfo.szCSDVersion) == 0)
			{
				return WinNT4NoSp;
			}
			else if (strcmp(verInfo.szCSDVersion, "Service Pack 1") == 0)
			{
				return WinNT4Sp1;
			}
			else if (strcmp(verInfo.szCSDVersion, "Service Pack 2") == 0)
			{
				return WinNT4Sp2;
			}
			else if (strcmp(verInfo.szCSDVersion, "Service Pack 3") == 0)
			{
				return WinNT4Sp3;
			}
			else
			{
				return WinNT4PostSp3;
			}
		}
		else if (verInfo.dwMajorVersion == 5)
		{
			return Win2000;
		}

	default:
		return WinUnknown;
	}
}


BOOL IsWin95OSR2Compatible()
{
	switch (GetWindowsVersion())
	{
		case Win95OSR2:
		case Win98OrFuture:
			return TRUE;

		default:
			return FALSE;
	}
}

BOOL IsWin2000Compatible()
{
	switch (GetWindowsVersion())
	{
		case Win2000:
			return TRUE;

		default:
			return FALSE;
	}
}
//////////////////////////////////////////////////////////////////////////////
// LinkResolution.cpp
//
// Contains code for explicitly loading certain Windows API functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: LinkResolution.cpp,v 1.1.2.9.2.2 1998/08/22 01:55:41 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.


#if defined(PGPDISK_MFC)

#define NOSERVICE
#include "StdAfx.h"

#include "Required.h"

#include "WindowsVersion.h"
#include "LinkResolution.h"

#else
#error Define PGPDISK_MFC.
#endif	// PGPDISK_MFC


/////////
// Macros
/////////

#define CLEANUP_WINAPI_STACK(bytes) __asm sub esp, bytes


////////
// Types
////////

typedef long (* BroadcastSystemMessageType)(DWORD, LPDWORD, UINT, WPARAM, 
	LPARAM);

typedef BOOL (* GetDiskFreeSpaceExAType)(LPCTSTR, PULARGE_INTEGER, 
	PULARGE_INTEGER, PULARGE_INTEGER);

typedef BOOL (* GetDiskFreeSpaceExWType)(LPCWSTR, PULARGE_INTEGER, 
	PULARGE_INTEGER, PULARGE_INTEGER);

typedef SC_HANDLE (* OpenSCManagerAType)(LPCSTR, LPCSTR, DWORD);
typedef SC_HANDLE (* OpenSCManagerWType)(LPCWSTR, LPCWSTR, DWORD);

typedef SC_HANDLE (* OpenServiceAType)(SC_HANDLE, LPCSTR, DWORD);
typedef SC_HANDLE (* OpenServiceWType)(SC_HANDLE, LPCWSTR, DWORD);

typedef BOOL (* StartServiceAType)(SC_HANDLE, DWORD, LPCSTR *);
typedef BOOL (* StartServiceWType)(SC_HANDLE, DWORD, LPCWSTR *);

typedef BOOL (* ControlServiceType)(SC_HANDLE, DWORD, LPSERVICE_STATUS);
typedef BOOL (* CloseServiceHandleType)(SC_HANDLE);

typedef DWORD (* WinNT_NetShareGetInfoType)(LPWSTR, LPWSTR, DWORD, LPBYTE *);

typedef DWORD (* Win95_NetShareGetInfoType)(const char FAR *, 
	const char FAR *, short, char FAR *, unsigned short, 
	unsigned short FAR *);

typedef DWORD (* WinNT_NetWkstaGetInfoType)(LPWSTR servername, DWORD level, 
	LPBYTE *bufptr);

typedef DWORD (* WinNT_NetApiBufferFreeType)(LPVOID);


//////////
// Globals
//////////

// Dll handles.
static HINSTANCE	Advapi32Handle;
static HINSTANCE	Kernel32Handle;
static HINSTANCE	NetApi32Handle;
static HINSTANCE	SvrApiHandle;
static HINSTANCE	User32Handle;

// Function addresses.
static BroadcastSystemMessageType	BroadcastSystemMessageAddress;
static GetDiskFreeSpaceExAType		GetDiskFreeSpaceExAAddress;
static GetDiskFreeSpaceExWType		GetDiskFreeSpaceExWAddress;

static OpenSCManagerAType		OpenSCManagerAAddress;
static OpenSCManagerWType		OpenSCManagerWAddress;
static OpenServiceAType			OpenServiceAAddress;
static OpenServiceWType			OpenServiceWAddress;
static StartServiceAType		StartServiceAAddress;
static StartServiceWType		StartServiceWAddress;
static ControlServiceType		ControlServiceAddress;
static CloseServiceHandleType	CloseServiceHandleAddress;

static WinNT_NetShareGetInfoType	WinNT_NetShareGetInfoAddress;
static Win95_NetShareGetInfoType	Win95_NetShareGetInfoAddress;
static WinNT_NetWkstaGetInfoType	WinNT_NetWkstaGetInfoAddress;
static WinNT_NetApiBufferFreeType	WinNT_NetApiBufferFreeAddress;


//////////////////////////////////////
// Initialization and cleanup routines
//////////////////////////////////////

// LinkStandardWin32API links in 'standard' Win32 functions.

DualErr 
LinkStandardWin32API()
{
	DualErr derr;

	Kernel32Handle = LoadLibrary("kernel32.dll");

	if (IsNull(Kernel32Handle))
		derr = DualErr(kPGDMinorError_LoadLibraryFailed);

	if (derr.IsntError())
	{
		User32Handle = LoadLibrary("user32.dll");

		if (IsNull(User32Handle))
			derr = DualErr(kPGDMinorError_LoadLibraryFailed);
	}

	if (derr.IsntError())
	{
		BroadcastSystemMessageAddress = (BroadcastSystemMessageType)
			GetProcAddress(User32Handle, "BroadcastSystemMessage");
		
		if (IsNull(BroadcastSystemMessageAddress))
		{
			derr = DualErr(kPGDMinorError_GetProcAddrFailed, GetLastError());
		}
	}

	if (derr.IsntError())
	{
		if (IsWin95OSR2CompatibleMachine() || 
			IsWinNT4CompatibleMachine())
		{
			GetDiskFreeSpaceExAAddress = (GetDiskFreeSpaceExAType) 
				GetProcAddress(Kernel32Handle, "GetDiskFreeSpaceExA");

			GetDiskFreeSpaceExWAddress = (GetDiskFreeSpaceExWType) 
				GetProcAddress(Kernel32Handle, "GetDiskFreeSpaceExW");

			if (IsNull(BroadcastSystemMessageAddress) || 
				IsNull(GetDiskFreeSpaceExAAddress) || 
				IsNull(GetDiskFreeSpaceExWAddress))
			{
				derr = DualErr(kPGDMinorError_GetProcAddrFailed, 
					GetLastError());
			}
		}
	}

	return derr;
}

// LinkNetworkingFunctions links in networking functions.

DualErr 
LinkNetworkingFunctions()
{
	DualErr derr;

	if (IsWin95CompatibleMachine())
	{
		SvrApiHandle = LoadLibrary("SvrApi.dll");

		if (IsNull(SvrApiHandle))
			derr = DualErr(kPGDMinorError_LoadLibraryFailed);

		if (derr.IsntError())
		{
			Win95_NetShareGetInfoAddress = (Win95_NetShareGetInfoType) 
				GetProcAddress(SvrApiHandle, "NetShareGetInfo");
			
			if (IsNull(Win95_NetShareGetInfoAddress))
			{
				derr = DualErr(kPGDMinorError_GetProcAddrFailed, 
					GetLastError());
			}
		}
	}
	else if (IsWinNT4CompatibleMachine())
	{
		NetApi32Handle = LoadLibrary("NetApi32.dll");

		if (IsNull(NetApi32Handle))
			derr = DualErr(kPGDMinorError_LoadLibraryFailed);

		if (derr.IsntError())
		{
			WinNT_NetShareGetInfoAddress = (WinNT_NetShareGetInfoType) 
				GetProcAddress(NetApi32Handle, "NetShareGetInfo");

			WinNT_NetWkstaGetInfoAddress = (WinNT_NetWkstaGetInfoType)
				GetProcAddress(NetApi32Handle, "NetWkstaGetInfo");

			WinNT_NetApiBufferFreeAddress = (WinNT_NetApiBufferFreeType) 
				GetProcAddress(NetApi32Handle, "NetApiBufferFree");

			if (IsNull(WinNT_NetShareGetInfoAddress) || 
				IsNull(WinNT_NetWkstaGetInfoAddress) || 
				IsNull(WinNT_NetApiBufferFreeAddress))
			{
				derr = DualErr(kPGDMinorError_GetProcAddrFailed, 
					GetLastError());
			}
		}
	}

	return derr;
}

// LinkServiceFunctions links in NT service functions.

DualErr 
LinkServiceFunctions()
{
	DualErr derr;

	if (IsWinNT4CompatibleMachine())
	{
		Advapi32Handle = LoadLibrary("Advapi32.dll");

		if (IsNull(Advapi32Handle))
			derr = DualErr(kPGDMinorError_LoadLibraryFailed);

		if (derr.IsntError())
		{
			OpenSCManagerAAddress = (OpenSCManagerAType) 
				GetProcAddress(Advapi32Handle, "OpenSCManagerA");

			OpenSCManagerWAddress = (OpenSCManagerWType) 
				GetProcAddress(Advapi32Handle, "OpenSCManagerW");

			OpenServiceAAddress = (OpenServiceAType) 
				GetProcAddress(Advapi32Handle, "OpenServiceA");

			OpenServiceWAddress = (OpenServiceWType) 
				GetProcAddress(Advapi32Handle, "OpenServiceW");

			StartServiceAAddress = (StartServiceAType) 
				GetProcAddress(Advapi32Handle, "StartServiceA");

			StartServiceWAddress = (StartServiceWType) 
				GetProcAddress(Advapi32Handle, "StartServiceW");

			ControlServiceAddress = (ControlServiceType) 
				GetProcAddress(Advapi32Handle, "ControlService");

			CloseServiceHandleAddress = (CloseServiceHandleType) 
				GetProcAddress(Advapi32Handle, "CloseServiceHandle");

			if (IsNull(OpenSCManagerAAddress) || 
				IsNull(OpenSCManagerWAddress) || 
				IsNull(OpenServiceAAddress) || 
				IsNull(OpenServiceWAddress) || 
				IsNull(StartServiceAAddress) || 
				IsNull(StartServiceWAddress) || 
				IsNull(ControlServiceAddress) || 
				IsNull(CloseServiceHandleAddress))
			{
				derr = DualErr(kPGDMinorError_GetProcAddrFailed, 
					GetLastError());
			}
		}
	}

	return derr;
}

// ExplicitlyLinkAPIsForPlatform loads and resolves needed API function
// addresses for the version of Windows that we are using.

DualErr 
ExplicitlyLinkAPIsForPlatform()
{
	DualErr derr;

	pgpAssert(IsWin95CompatibleMachine() || IsWinNT4CompatibleMachine());

	derr = LinkStandardWin32API();

	if (derr.IsntError())
	{
		derr = LinkServiceFunctions();
	}

	if (derr.IsntError())
	{
		derr = LinkNetworkingFunctions();
	}

	if (derr.IsError())
	{
		CleanupExplicitLinkage();
	}

	return derr;
}

// CleanupExplicitLinkage unloads all Dlls that have handles open.

void 
CleanupExplicitLinkage()
{
	if (IsntNull(SvrApiHandle))
		FreeLibrary(SvrApiHandle);

	if (IsntNull(NetApi32Handle))
		FreeLibrary(NetApi32Handle);

	if (IsntNull(Advapi32Handle))
		FreeLibrary(Advapi32Handle);

	if (IsntNull(Kernel32Handle))
		FreeLibrary(Kernel32Handle);

	if (IsntNull(User32Handle))
		FreeLibrary(User32Handle);
}


//////////////////////////////////////
// Initialization and cleanup routines
//////////////////////////////////////

// Wrapper around BroadcastSystemMessage.

long 
LinkSafe_BroadcastSystemMessage(
	DWORD	dwFlags, 
	LPDWORD	lpdwRecipients, 
	UINT	uiMessage, 
	WPARAM	wParam, 
	LPARAM	lParam)
{
	long result;

	pgpAssert(IsntNull(BroadcastSystemMessageAddress));

	result = BroadcastSystemMessageAddress(dwFlags, lpdwRecipients, 
		uiMessage, wParam, lParam);

	CLEANUP_WINAPI_STACK(20);

	return result;
}


// Wrapper around GetDiskFreeSpaceExA for OSR2/NT4.

BOOL 
LinkSafe_GetDiskFreeSpaceExA(
	LPCSTR			lpDirectoryName, 
	PULARGE_INTEGER	lpFreeBytesAvailableToCaller, 
	PULARGE_INTEGER	lpTotalNumberOfBytes, 
	PULARGE_INTEGER	lpTotalNumberOfFreeBytes)
{
	BOOL result;

	pgpAssert(IsntNull(GetDiskFreeSpaceExAAddress));

	result = GetDiskFreeSpaceExAAddress(lpDirectoryName, 
		lpFreeBytesAvailableToCaller, lpTotalNumberOfBytes, 
		lpTotalNumberOfFreeBytes);

	CLEANUP_WINAPI_STACK(16);

	return result;
}

// Wrapper around GetDiskFreeSpaceExW for OSR2/NT4.

BOOL 
LinkSafe_GetDiskFreeSpaceExW(
	LPCWSTR			lpDirectoryName, 
	PULARGE_INTEGER	lpFreeBytesAvailableToCaller, 
	PULARGE_INTEGER	lpTotalNumberOfBytes, 
	PULARGE_INTEGER	lpTotalNumberOfFreeBytes)
{
	BOOL result;

	pgpAssert(IsntNull(GetDiskFreeSpaceExWAddress));

	result = GetDiskFreeSpaceExWAddress(lpDirectoryName, 
		lpFreeBytesAvailableToCaller, lpTotalNumberOfBytes, 
		lpTotalNumberOfFreeBytes);

	CLEANUP_WINAPI_STACK(16);

	return result;
}

// Wrapper around OpenSCManagerA for NT.

SC_HANDLE 
LinkSafe_OpenSCManagerA(
	LPCSTR	lpMachineName, 
	LPCSTR	lpDatabaseName, 
	DWORD	dwDesiredAccess)
{
	SC_HANDLE result;

	pgpAssert(IsntNull(OpenSCManagerAAddress));

	result = OpenSCManagerAAddress(lpMachineName, lpDatabaseName, 
		dwDesiredAccess);

	CLEANUP_WINAPI_STACK(12);

	return result;
}

// Wrapper around OpenSCManagerW for NT.

SC_HANDLE 
LinkSafe_OpenSCManagerW(
	LPCWSTR	lpMachineName, 
	LPCWSTR	lpDatabaseName, 
	DWORD	dwDesiredAccess)
{
	SC_HANDLE result;

	pgpAssert(IsntNull(OpenSCManagerWAddress));

	result = OpenSCManagerWAddress(lpMachineName, lpDatabaseName, 
		dwDesiredAccess);

	CLEANUP_WINAPI_STACK(12);

	return result;
}

// Wrapper around OpenServiceA for NT.

SC_HANDLE 
LinkSafe_OpenServiceA(
	SC_HANDLE	hSCManager, 
	LPCSTR		lpServiceName, 
	DWORD		dwDesiredAccess)
{
	SC_HANDLE result;

	pgpAssert(IsntNull(OpenServiceAAddress));

	result = OpenServiceAAddress(hSCManager, lpServiceName, dwDesiredAccess);

	CLEANUP_WINAPI_STACK(12);

	return result;
}

// Wrapper around OpenServiceW for NT.

SC_HANDLE 
LinkSafe_OpenServiceW(
	SC_HANDLE	hSCManager, 
	LPCWSTR		lpServiceName, 
	DWORD		dwDesiredAccess)
{
	SC_HANDLE result;

	pgpAssert(IsntNull(OpenServiceWAddress));

	result = OpenServiceWAddress(hSCManager, lpServiceName, dwDesiredAccess);

	CLEANUP_WINAPI_STACK(12);

	return result;
}

// Wrapper around StartServiceA for NT.

BOOL 
LinkSafe_StartServiceA(
	SC_HANDLE	hService, 
	DWORD		dwNumServiceArgs, 
	LPCSTR		*lpServiceArgVectors)
{
	BOOL result;

	pgpAssert(IsntNull(StartServiceAAddress));

	result = StartServiceAAddress(hService, dwNumServiceArgs, 
		lpServiceArgVectors);

	CLEANUP_WINAPI_STACK(12);

	return result;
}

// Wrapper around StartServiceW for NT.

BOOL 
LinkSafe_StartServiceW(
	SC_HANDLE	hService, 
	DWORD		dwNumServiceArgs, 
	LPCWSTR		*lpServiceArgVectors)
{
	BOOL result;

	pgpAssert(IsntNull(StartServiceWAddress));

	result = StartServiceWAddress(hService, dwNumServiceArgs, 
		lpServiceArgVectors);

	CLEANUP_WINAPI_STACK(12);

	return result;
}

// Wrapper around ControlService for NT.

BOOL 
LinkSafe_ControlService(
	SC_HANDLE			hService, 
	DWORD				dwControl, 
	LPSERVICE_STATUS	lpServiceStatus)
{
	BOOL result;

	pgpAssert(IsntNull(ControlServiceAddress));

	result = ControlServiceAddress(hService, dwControl, lpServiceStatus);

	CLEANUP_WINAPI_STACK(12);

	return result;
}

// Wrapper around CloseServiceHandle for NT.

BOOL 
LinkSafe_CloseServiceHandle(SC_HANDLE hSCObject)
{
	BOOL result;

	pgpAssert(IsntNull(CloseServiceHandleAddress));

	result = CloseServiceHandleAddress(hSCObject);

	CLEANUP_WINAPI_STACK(4);

	return result;
}

// Wrapper around NetShareGetInfo for NT4.

DWORD 
LinkSafe_WinNT_NetShareGetInfo(
	LPWSTR	servername, 
	LPWSTR	netname, 
	DWORD	level, 
	LPBYTE	*bufptr)
{
	DWORD result;

	pgpAssert(IsntNull(WinNT_NetShareGetInfoAddress));

	result = WinNT_NetShareGetInfoAddress(servername, netname, level, bufptr);

	CLEANUP_WINAPI_STACK(16);

	return result;
}

// Wrapper around NetShareGetInfo for Win95.

DWORD 
LinkSafe_Win95_NetShareGetInfo(
	const char FAR		*pszServer, 
	const char FAR		*pszNetName, 
	short				sLevel, 
	char FAR			*pbBuffer, 
	unsigned short		cbBuffer, 
	unsigned short FAR	*pcbTotalAvail)
{
	DWORD result;

	pgpAssert(IsntNull(Win95_NetShareGetInfoAddress));

	result = Win95_NetShareGetInfoAddress(pszServer, pszNetName, sLevel, 
		pbBuffer, cbBuffer, pcbTotalAvail);

	CLEANUP_WINAPI_STACK(24);

	return result;
}

// Wrapper around NetWkstaGetInfo for WinNT

DWORD 
LinkSafe_WinNT_NetWkstaGetInfo(LPWSTR servername, DWORD level, LPBYTE *bufptr)
{
	DWORD result;

	pgpAssert(IsntNull(WinNT_NetWkstaGetInfoAddress));

	result = WinNT_NetWkstaGetInfoAddress(servername, level, bufptr);

	CLEANUP_WINAPI_STACK(12);

	return result;
}

// Wrapper around NetApiBufferFree for NT4.

DWORD 
LinkSafe_WinNT_NetApiBufferFree(LPVOID Buffer)
{
	DWORD result;

	pgpAssert(IsntNull(WinNT_NetApiBufferFreeAddress));

	result = WinNT_NetApiBufferFreeAddress(Buffer);

	CLEANUP_WINAPI_STACK(4);

	return result;
}

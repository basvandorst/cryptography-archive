//////////////////////////////////////////////////////////////////////////////
// LinkResolution.h
//
// Contains declarations for Windows API functions we explicitly load.
//////////////////////////////////////////////////////////////////////////////

// $Id: LinkResolution.h,v 1.1.2.8 1998/08/04 02:10:14 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_LinkResolution_h	// [
#define Included_LinkResolution_h

#include "DualErr.h"


////////
// Types
////////

#if defined(NOSERVICE)

// From Winsvc.h.

typedef HANDLE SC_HANDLE;

typedef struct _SERVICE_STATUS {
	DWORD	dwServiceType;
	DWORD	dwCurrentState;
	DWORD	dwControlsAccepted;
	DWORD	dwWin32ExitCode;
	DWORD	dwServiceSpecificExitCode;
	DWORD	dwCheckPoint;
	DWORD	dwWaitHint;

} SERVICE_STATUS, *LPSERVICE_STATUS;

#endif	// NOSERVICE


////////////////////////
// Function declarations
////////////////////////

DualErr		ExplicitlyLinkAPIsForPlatform();
void		CleanupExplicitLinkage();

long		LinkSafe_BroadcastSystemMessage(DWORD dwFlags, 
				LPDWORD lpdwRecipients, UINT uiMessage, WPARAM wParam, 
				LPARAM lParam);

BOOL		LinkSafe_GetDiskFreeSpaceExA(LPCSTR lpDirectoryName, 
				PULARGE_INTEGER lpFreeBytesAvailableToCaller, 
				PULARGE_INTEGER lpTotalNumberOfBytes, 
				PULARGE_INTEGER lpTotalNumberOfFreeBytes);

BOOL		LinkSafe_GetDiskFreeSpaceExW(LPCWSTR lpDirectoryName, 
				PULARGE_INTEGER lpFreeBytesAvailableToCaller, 
				PULARGE_INTEGER lpTotalNumberOfBytes, 
				PULARGE_INTEGER lpTotalNumberOfFreeBytes);

SC_HANDLE	LinkSafe_OpenSCManagerA(LPCSTR lpMachineName, 
				LPCSTR lpDatabaseName, DWORD dwDesiredAccess);

SC_HANDLE	LinkSafe_OpenSCManagerW(LPCWSTR lpMachineName, 
				LPCWSTR lpDatabaseName, DWORD dwDesiredAccess);

SC_HANDLE	LinkSafe_OpenServiceA(SC_HANDLE hSCManager, 
				LPCSTR lpServiceName, DWORD dwDesiredAccess);

SC_HANDLE	LinkSafe_OpenServiceW(SC_HANDLE hSCManager, 
				LPCWSTR lpServiceName, DWORD dwDesiredAccess);

BOOL		LinkSafe_StartServiceA(SC_HANDLE hService, 
				DWORD dwNumServiceArgs, LPCSTR *lpServiceArgVectors);

BOOL		LinkSafe_StartServiceW(SC_HANDLE hService, 
				DWORD dwNumServiceArgs, LPCWSTR *lpServiceArgVectors);

BOOL		LinkSafe_ControlService(SC_HANDLE hService, DWORD dwControl, 
				LPSERVICE_STATUS lpServiceStatus);

BOOL		LinkSafe_CloseServiceHandle(SC_HANDLE hSCObject);

DWORD		LinkSafe_WinNT_NetShareGetInfo(LPWSTR servername, LPWSTR netname, 
				DWORD level, LPBYTE *bufptr);

DWORD		LinkSafe_Win95_NetShareGetInfo(const char FAR *pszServer, 
				const char FAR *pszNetName, short sLevel, char FAR *pbBuffer, 
				unsigned short cbBuffer, unsigned short FAR * pcbTotalAvail);

DWORD		LinkSafe_WinNT_NetWkstaGetInfo(LPWSTR servername, DWORD level, 
				LPBYTE *bufptr);

DWORD		LinkSafe_WinNT_NetApiBufferFree(LPVOID Buffer);


///////////////////
// Function defines
///////////////////

#undef BroadcastSystemMessage
#undef GetDiskFreeSpaceEx
#undef OpenSCManager
#undef OpenService
#undef StartService

#if defined(UNICODE)

#define BroadcastSystemMessage	LinkSafe_BroadcastSystemMessage
#define	GetDiskFreeSpaceEx		LinkSafe_GetDiskFreeSpaceExW
#define	OpenSCManager			LinkSafe_OpenSCManagerW
#define	OpenService				LinkSafe_OpenServiceW
#define	StartService			LinkSafe_StartServiceW

#else

#define BroadcastSystemMessage	LinkSafe_BroadcastSystemMessage
#define GetDiskFreeSpaceEx		LinkSafe_GetDiskFreeSpaceExA
#define	OpenSCManager			LinkSafe_OpenSCManagerA
#define	OpenService				LinkSafe_OpenServiceA
#define	StartService			LinkSafe_StartServiceA

#endif // UNICODE

#define WinNT_ControlService		LinkSafe_ControlService
#define WinNT_CloseServiceHandle	LinkSafe_CloseServiceHandle

#define	WinNT_NetShareGetInfo	LinkSafe_WinNT_NetShareGetInfo
#define	Win95_NetShareGetInfo	LinkSafe_Win95_NetShareGetInfo
#define	WinNT_NetWkstaGetInfo	LinkSafe_WinNT_NetWkstaGetInfo
#define	WinNT_NetApiBufferFree	LinkSafe_WinNT_NetApiBufferFree

#endif	// ] Included_LinkResolution_h

/*
 # $Id: sysobj.cpp,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
 # Copyright (C) 1997-2000 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#include <sys/types.h>
#include <string.h>
#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
// Windows NT:
#include "psapi.h"
// Windows 95:
#include <tlhelp32.h>

#include "misc.h"
#include "log.h"
#include "fortify.h"
#include "sysobj.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

sysObj::sysObj()
{
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	osInfoOK = GetVersionEx((LPOSVERSIONINFO) &osInfo);

	typedef struct _OSVERSIONINFO{ 
    DWORD dwMajorVersion; 
    DWORD dwMinorVersion; 
    DWORD dwBuildNumber; 
    DWORD dwPlatformId; 
    TCHAR szCSDVersion[128]; 
	} OSVERSIONINFO; 

	if (!osInfoOK) {
		Log(1, "t1>> Platform info is not available!\n");
		return;
	}

	Log(4, "t4>> Platform info:\n");
	Log(4, "t4>>    major=%d minor=%d build=%d [0x%x]\n",
		osInfo.dwMajorVersion, osInfo.dwMinorVersion, 
		osInfo.dwBuildNumber, osInfo.dwBuildNumber);
	Log(4, "t4>>    on %s [%d]\n",
		IDToStr(osInfo.dwPlatformId), osInfo.dwPlatformId);
	Log(4, "t4>>    \"%s\"\n", osInfo.szCSDVersion);
}

sysObj::~sysObj()
{
}

char *
sysObj::IDToStr(DWORD id)
{
	if (id == VER_PLATFORM_WIN32_NT)
		return "WinNT";
	if (id == VER_PLATFORM_WIN32_WINDOWS)
		return "Win95";
	if (id == VER_PLATFORM_WIN32s)
		return "Win32s";
	return "???";
}

int
sysObj::procIsRunning(char *procName)
{
	if (!osInfoOK)
		return 0;
	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		return checkWinNTProc(procName);
	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		return checkWin95Proc(procName);
	return 0;
}

int
sysObj::checkWinNTProc(char *procName)
{
	int found = 0;

    HANDLE psapi; 
    psapi = LoadLibrary("PSAPI.DLL");

	if (psapi == NULL)
        return found;
	
	typedef BOOL (WINAPI *ENUMPROCESSES)(
		DWORD * lpidProcess,  // array to receive the process identifiers
		DWORD cb,             // size of the array
		DWORD * cbNeeded      // receives the number of bytes returned
	);

	typedef BOOL (WINAPI *ENUMPROCESSMODULES)(
		HANDLE hProcess,      // handle to the process
		HMODULE * lphModule,  // array to receive the module handles
		DWORD cb,             // size of the array
		LPDWORD lpcbNeeded    // receives the number of bytes returned
	);

	typedef DWORD (WINAPI *GETMODULEFILENAME)( 
		HANDLE hProcess,		// handle to the process
		HMODULE hModule,		// handle to the module
		LPTSTR lpstrFileName,	// array to receive filename
		DWORD nSize			// size of filename array.
	);

	ENUMPROCESSES EnumProcesses;
    ENUMPROCESSMODULES EnumProcessModules;
	GETMODULEFILENAME GetModuleFileName;

	EnumProcesses =(ENUMPROCESSES) GetProcAddress(
			(HINSTANCE)psapi, "EnumProcesses");
	EnumProcessModules = (ENUMPROCESSMODULES) GetProcAddress(
			(HINSTANCE)psapi, "EnumProcessModules");
	GetModuleFileName = (GETMODULEFILENAME) GetProcAddress(
			(HINSTANCE)psapi, "GetModuleFileNameExA");

	if (EnumProcesses == NULL || EnumProcessModules == NULL || GetModuleFileName == NULL)
        return found;

    DWORD procIDs[1024];
    DWORD nProcs;

    if (!EnumProcesses(procIDs, sizeof(procIDs), &nProcs))
        return found;

	nProcs /= sizeof(procIDs[0]);

    for (unsigned int i = 0; !found && i < nProcs; i++) {

        HANDLE process = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
            FALSE, 
            procIDs[i]);

        HMODULE modules[1024];
        DWORD nmodules;
        char pathName[MAX_PATH];

        EnumProcessModules(process, modules, sizeof(modules), &nmodules);
        nmodules /= sizeof(modules[0]);

        if (GetModuleFileName(process, modules[0], pathName, sizeof(pathName))) {
			if (checkProc(pathName, (int) procIDs[i], procName)) {
				found = 1;
			}			
        }
        CloseHandle(process);
    }
	FreeLibrary((HINSTANCE)psapi);
	return found;
}


int
sysObj::checkWin95Proc(char *procName)
{
	HANDLE kernel = GetModuleHandle("KERNEL32.DLL");
	if (kernel == NULL)
        return 0;

	typedef HANDLE (WINAPI *CREATESNAPSHOT)(
	   DWORD dwFlags,
	   DWORD th32ProcessID
	);

	typedef BOOL (WINAPI *PROCESSWALK)(
	   HANDLE hSnapshot,    
	   LPPROCESSENTRY32 lppe
	);

	CREATESNAPSHOT CreateToolhelp32Snapshot;
	PROCESSWALK Process32First;
	PROCESSWALK Process32Next;

	CreateToolhelp32Snapshot = (CREATESNAPSHOT) GetProcAddress((HINSTANCE) kernel, 
			"CreateToolhelp32Snapshot");
	Process32First = (PROCESSWALK) GetProcAddress((HINSTANCE) kernel, 
			"Process32First");
	Process32Next = (PROCESSWALK) GetProcAddress((HINSTANCE) kernel, 
			"Process32Next");

	if (CreateToolhelp32Snapshot == NULL)
		return 0;
	if (Process32First == NULL || Process32Next == NULL)
		return 0;

    PROCESSENTRY32 proc;
    proc.dwSize = sizeof(proc);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	Process32First(snapshot, &proc);
    do {
		if (checkProc(proc.szExeFile, (int) proc.th32ProcessID, procName)) {
			CloseHandle(snapshot);
			return 1;
		}
	} while (Process32Next(snapshot, &proc));

    CloseHandle(snapshot);
    return 0;
}


int
sysObj::checkProc(char *pathName, int pid, char *procName)
{
	int cmp;
	int procNameLen = strlen(procName);
	int pathNameLen = strlen(pathName);

	if (pathNameLen >= procNameLen) {
		cmp = f_stricmp(pathName + pathNameLen - procNameLen, procName);
		if (cmp == 0)
			Log(9, "t9>> proc: \"%s\" [%#10x]\n", pathName, pid);
		return (cmp == 0);
	}
	return 0;
}

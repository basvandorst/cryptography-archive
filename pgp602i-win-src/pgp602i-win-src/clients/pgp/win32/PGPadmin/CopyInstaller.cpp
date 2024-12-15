/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: CopyInstaller.cpp,v 1.4.8.1 1998/11/12 03:13:02 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include <commctrl.h>
#include "resource.h"


static
DWORD FindInstallerFile(HWND hwnd,
						char *szFindFile, 
						HANDLE hFind,
						WIN32_FIND_DATA *findData,
						DWORD *pdwFile,
						DWORD *pdwTotalFiles,
						char *szPGPPath, 
						char *szInstallDir,
						BOOL bCopy);

static
void ReportCopyProgress(HWND hwnd, 
						char *szFileName, 
						DWORD dwFile, 
						DWORD dwTotalFiles);

BOOL CALLBACK CopyProgressProc(HWND hwndDlg, 
							   UINT uMsg, 
							   WPARAM wParam, 
							   LPARAM lParam);


DWORD CopyInstaller(HWND hwnd, char *szClientInstaller, char *szInstallDir)
{
	char			szFindFile[MAX_PATH];
	char			szTitle[255];
	WIN32_FIND_DATA findData;
	HANDLE			hFind;
	DWORD			dwFile;
	DWORD			dwTotalFiles;
	DWORD			dwLastError;
	char			szError[255];
	HWND			hwndDlg;

	strcpy(szFindFile, szClientInstaller);
	strcat(szFindFile, "*.*");

	SetLastError(0);
	hFind = FindFirstFile(szFindFile, &findData);
	dwLastError = GetLastError();

	if (dwLastError)
	{
		FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_ARGUMENT_ARRAY,
						NULL, dwLastError, 0, szError, 254, NULL);
	
		LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
		MessageBox(hwnd, szError, szTitle, MB_ICONEXCLAMATION);

		return dwLastError;
	}

	// First, just find files and count up the total

	dwFile = 0;
	dwTotalFiles = 0;
	dwLastError = 0;
	
	while (!dwLastError)
	{
		dwLastError = FindInstallerFile(hwnd, szFindFile, hFind, 
						&findData, &dwFile, &dwTotalFiles, 
						szClientInstaller, szInstallDir, FALSE);
	}
	
	FindClose(hFind);
	hwndDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_COPYPROGRESS), 
				hwnd, (DLGPROC) CopyProgressProc);
	ShowWindow(hwndDlg, SW_SHOW);

	// Now, do the actual copying of files

	hFind = FindFirstFile(szFindFile, &findData);
	dwFile = 0;
	dwLastError = 0;
	
	while (!dwLastError)
	{
		dwLastError = FindInstallerFile(hwndDlg, szFindFile, hFind, 
						&findData, &dwFile, &dwTotalFiles, 
						szClientInstaller, szInstallDir, TRUE);
	}
	
	FindClose(hFind);
	DestroyWindow(hwndDlg);
	return 0;
}


DWORD FindInstallerFile(HWND hwnd,
						char *szFindFile, 
						HANDLE hFind,
						WIN32_FIND_DATA *findData,
						DWORD *pdwFile,
						DWORD *pdwTotalFiles,
						char *szPGPPath, 
						char *szInstallDir,
						BOOL bCopy)
{
	char szOrigFile[MAX_PATH];
	char szNewFile[MAX_PATH];
	char szFindSubFile[MAX_PATH];
	char szError[255];
	char szTitle[255];
	HANDLE hFindSub;
	DWORD dwLastError;

	// Ignore "." and ".."

	if (!strcmp(findData->cFileName, ".") || 
		!strcmp(findData->cFileName, ".."))
	{
		SetLastError(0);
		FindNextFile(hFind, findData);
		dwLastError = GetLastError();
		return dwLastError;
	}

	strcpy(szOrigFile, szPGPPath);
	strcpy(szNewFile, szInstallDir);
	strcat(szOrigFile, findData->cFileName);
	strcat(szNewFile, findData->cFileName);

	// If it's a directory, create a copy, and start finding
	// files within that directory

	if (findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		CreateDirectory(szNewFile, NULL);
		strcat(szOrigFile, "\\");
		strcat(szNewFile, "\\");
		strcpy(szFindSubFile, szOrigFile);
		strcat(szFindSubFile, "*.*");
		SetLastError(0);
		hFindSub = FindFirstFile(szFindSubFile, findData);
		dwLastError = GetLastError();
		
		if (dwLastError)
		{
			FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_ARGUMENT_ARRAY,
				NULL, dwLastError, 0, szError, 254, NULL);
			
			LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
			MessageBox(hwnd, szError, szTitle, MB_ICONEXCLAMATION);
			
			return dwLastError;
		}

		while (!dwLastError)
		{
			dwLastError = FindInstallerFile(hwnd, szFindSubFile, hFindSub, 
							findData, pdwFile, pdwTotalFiles, 
							szOrigFile, szNewFile, bCopy);
		}

		FindClose(hFindSub);
	}

	// If it's a file, copy it

	else
	{
		if (bCopy)
		{
			ReportCopyProgress(hwnd, szNewFile, *pdwFile, *pdwTotalFiles);
			CopyFile(szOrigFile, szNewFile, FALSE);
		}
		else
			(*pdwTotalFiles)++;

		(*pdwFile)++;
	}

	SetLastError(0);
	FindNextFile(hFind, findData);
	dwLastError = GetLastError();
	return dwLastError;
}


void ReportCopyProgress(HWND hwnd, 
						char *szFileName, 
						DWORD dwFile, 
						DWORD dwTotalFiles)
{
	DWORD dwPos;

	dwPos = 100 * dwFile / dwTotalFiles;
	SendMessage(GetDlgItem(hwnd, IDC_PROGRESS), PBM_SETPOS, dwPos, 0);
	SetWindowText(GetDlgItem(hwnd, IDC_FILENAME), szFileName);
	UpdateWindow(hwnd);

	return;
}


BOOL CALLBACK CopyProgressProc(HWND hwndDlg, 
							   UINT uMsg, 
							   WPARAM wParam, 
							   LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT rc;

			// center dialog on screen
			GetWindowRect(GetParent(hwndDlg), &rc);
			SetWindowPos(GetParent(hwndDlg), NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);

			break;
		}
	}

	return 0;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPsda.c - Code for decoding SDA's
	

	$Id: seautil.c,v 1.10 1999/05/13 22:44:09 wjb Exp $
____________________________________________________________________________*/

#include "DecodeStub.h"

UINT FileListFromFile(FILELIST **filelist,char *filename,BOOL *UserCancel)
{
	FILELIST *Current;

	if(UserCancel!=NULL)
	{
		if(*UserCancel)
		{
			return FALSE;
		}
	}

	Current=(FILELIST *)malloc(sizeof(FILELIST));
    memset(Current,0x00,sizeof(FILELIST));

	strcpy(Current->name,filename);
	Current->next=*filelist;
	*filelist=Current;

	return TRUE;
}

void SEAReboot(void)
{
	BOOL IsNT;
	OSVERSIONINFO osid;

	memset(&osid,0x00,sizeof(OSVERSIONINFO));

	osid.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	GetVersionEx (&osid);   
	IsNT=(osid.dwPlatformId == VER_PLATFORM_WIN32_NT);

	if(IsNT)
	{
		TOKEN_PRIVILEGES TpNew;
		HANDLE           hToken;
		LUID             ShutDownValue;

		OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES |
			TOKEN_QUERY, &hToken);
		LookupPrivilegeValue( (LPSTR)NULL, SE_SHUTDOWN_NAME, &ShutDownValue);

		TpNew.PrivilegeCount = 1;
		TpNew.Privileges[0].Luid = ShutDownValue;
		TpNew.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		AdjustTokenPrivileges( hToken, FALSE, &TpNew, sizeof(TpNew),
			(TOKEN_PRIVILEGES *)NULL, (DWORD *)NULL);
	}

	ExitWindowsEx(EWX_REBOOT,0);
}

#define RUNREGPATH "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"

BOOL WriteRunRegKey(char *filename)
{
	HKEY hKey;
	DWORD dw;

	if(RegCreateKeyEx (	HKEY_LOCAL_MACHINE, 
							RUNREGPATH, 
							0, 
							NULL,
							REG_OPTION_NON_VOLATILE, 
							KEY_ALL_ACCESS, 
							NULL, 
							&hKey, 
							&dw)==ERROR_SUCCESS)
	{
		RegSetValueEx (	hKey, 
						"Run PGP SEA Installer on Reboot", 
						0, 
						REG_SZ, 
						(LPBYTE)filename, 
						strlen(filename)+1);

		RegCloseKey (hKey);

		return TRUE;
	}

	return FALSE;
}

BOOL DeleteRunRegKey(void)
{
	HKEY hKey;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						RUNREGPATH, 
						0, 
						KEY_ALL_ACCESS, 
						&hKey) == ERROR_SUCCESS) 
	{
		RegDeleteValue( hKey, 
			"Run PGP SEA Installer on Reboot");

		RegCloseKey (hKey);

		return TRUE;
	}

	return FALSE;
}

#define SEAREGPATH "Software\\Network Associates\\PGP"

BOOL WriteRebootRegKey(DWORD dwReboot)
{
	HKEY hKey;
//	DWORD dw;

//	if(RegCreateKeyEx (	HKEY_LOCAL_MACHINE, 
//							SEAREGPATH, 
//							0, 
//							NULL,
//							REG_OPTION_NON_VOLATILE, 
//							KEY_ALL_ACCESS, 
//							NULL, 
//							&hKey, 
//							&dw)==ERROR_SUCCESS)
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						SEAREGPATH, 
						0, 
						KEY_ALL_ACCESS, 
						&hKey) == ERROR_SUCCESS) 
	{
		RegSetValueEx (	hKey, 
						"Reboot", 
						0, 
						REG_DWORD, 
						(LPBYTE)&dwReboot, 
						sizeof(dwReboot));
		RegCloseKey (hKey);

		return TRUE;
	}

	return FALSE;
}

#define DELPGPREGPATH "Software\\Network Associates"

BOOL DeletePGPRegKey(void)
{
	HKEY hKey;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						DELPGPREGPATH, 
						0, 
						KEY_ALL_ACCESS, 
						&hKey) == ERROR_SUCCESS) 
	{
		RegDeleteKey( hKey, 
			"PGP");

		RegCloseKey (hKey);

		return TRUE;
	}

	return FALSE;
}

DWORD ReadRebootRegKey(void)
{
	DWORD dwReboot;
	HKEY hKey;

	dwReboot=0;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						SEAREGPATH, 
						0, 
						KEY_ALL_ACCESS, 
						&hKey) == ERROR_SUCCESS) 
	{
		DWORD  size = sizeof(dwReboot);
		DWORD  type = 0;

		RegQueryValueEx(hKey, 
					"Reboot", 
					0, 
					&type, 
					(LPBYTE)&dwReboot, 
					&size);
		RegCloseKey (hKey);
	}

	return dwReboot;
}

#if !IS_SDA
// Fake routine since we aren't using compression in SEA
void Deflate_Decompress(void *gpi)
{
	int c;

	while((c=getc_buffer(gpi))!=EOF)
	{
		putc_buffer(c,gpi);
	}
}
#endif

BOOL ExecProgram(HWND hwnd,
				 char *prefpath,
				 char *filename)
{
	char name[MAX_PATH+1];
	char Message[100];
	int error;
	PROCESS_INFORMATION ProcessInformation;
	STARTUPINFO StartupInfo;

	strcpy(name,prefpath);
	strcat(name,filename);

	memset( &StartupInfo , 0x00, sizeof(STARTUPINFO) );
	memset( &ProcessInformation , 0x00, sizeof(PROCESS_INFORMATION) );

	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_SHOWNORMAL;

	error = CreateProcess(NULL,(LPTSTR)name, NULL,NULL,FALSE,
		NORMAL_PRIORITY_CLASS,NULL,NULL,
		(LPSTARTUPINFO)&StartupInfo,
		(LPPROCESS_INFORMATION)&ProcessInformation);

	if(error==0)
	{ 
		LoadString (g_hinst, IDS_CANTSTARTEXE, Message, sizeof(Message));
		
		MessageBox(hwnd,Message,
			name,
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return FALSE;
	}


	// Wait for process to end
	WaitForSingleObject(ProcessInformation.hProcess,INFINITE);
	CloseHandle(ProcessInformation.hProcess);

	return TRUE;
}

BOOL MakeAdminFile(HWND hwnd,
				  FILE *fin,
				  char *prefpath,
				  FILELIST **p_fl,
				  char *filename,
				  DWORD offset,
				  DWORD size)
{
	char *buf;
	FILE *fout;
	char Message[100];
	char name[MAX_PATH+1];

	// No pref file to create
	if(offset==0)
		return TRUE;

	buf=(char *)malloc(size);

	if(buf==NULL)
		return FALSE;

	memset(buf,0x00,size);

	fseek(fin, offset, SEEK_SET );
	fread(buf,1,size,fin);

	strcpy(name,prefpath);
	strcat(name,filename);

	fout=fopen(name,"wb");

	if(fout==0)
	{
		LoadString (g_hinst, IDS_CANTCREATEADMIN, Message, sizeof(Message));

		MessageBox(hwnd,Message,
			name,
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return FALSE;
	}
	else
	{
		fwrite(buf,1,size,fout);
		fclose(fout);

		// Remember file so we can delete it later
		FileListFromFile(p_fl,name,NULL);
	}

	free(buf);

	return TRUE;
}

PGPError SEAGetTempPath(GETPUTINFO *gpi)
{
	char HexString[100];
	char Message[256];
	char Caption[256];
	int ret_val;

	// Note that GetTempPath returns a path with a backslash
	if(GetTempPath(sizeof(gpi->szPrefixPath),gpi->szPrefixPath ) == 0)
	{
		LoadString (g_hinst, IDS_NOTEMPDIR, Message, sizeof(Message));

		MessageBox(gpi->hwnd,Message,
			Caption,
		MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
	
		return kPGPError_UserAbort;
	}

	srand( (unsigned)time( NULL ) );
	sprintf(HexString,"%x",rand()*rand());
	HexString[5]=0; // Truncate at length 5

	strcat(gpi->szPrefixPath,"PGP");
	strcat(gpi->szPrefixPath,HexString);
	strcat(gpi->szPrefixPath,"\\");

	ret_val=_mkdir(gpi->szPrefixPath);

	if(ret_val!=0)
	{
		LoadString (g_hinst, IDS_NOTEMPDIR, Message, sizeof(Message));

		MessageBox(gpi->hwnd,Message,
			Caption,
		MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return kPGPError_UserAbort;
	}

		// Keep track of it so we can delete it later
	FileListFromFile(&gpi->fl,gpi->szPrefixPath,NULL);
	gpi->fl->IsDirectory=TRUE;

	return kPGPError_NoErr;
}

void SEACreateExecDelete(FILE *fin,GETPUTINFO *gpi)
{
	FILELIST *freeatlast;
	int retval;

	// Clear reboot registry key to zero so installer can set it
	WriteRebootRegKey(0);

	if(!gpi->CancelOperation)
	{
		// If we haven't already cancelled the extraction
		// create preference files for SEA and exec setup
		char PrefPath[MAX_PATH+1];
		BOOL FoundProcess;
		HWND hwndWin;
		DWORD dwProcID;
		HANDLE hProcess;

		ShowWindow(gpi->hwnd,SW_HIDE);

		strcpy(PrefPath,gpi->szPrefixPath);
		strcat(PrefPath,"Disk1\\");

		MakeAdminFile(gpi->hwnd,fin,PrefPath,&gpi->fl,
				"PGPadmin.dat",
				gpi->SDAHeader->AdminOffset,
				gpi->SDAHeader->AdminSize);
		MakeAdminFile(gpi->hwnd,fin,PrefPath,&gpi->fl,
				"PGPclient.dat",
				gpi->SDAHeader->ClientOffset,
				gpi->SDAHeader->ClientSize);
		MakeAdminFile(gpi->hwnd,fin,PrefPath,&gpi->fl,
				"PGPnet.dat",
				gpi->SDAHeader->NetOffset,
				gpi->SDAHeader->NetSize);
		MakeAdminFile(gpi->hwnd,fin,PrefPath,&gpi->fl,
				"setup.ini",
				gpi->SDAHeader->SetupOffset,
				gpi->SDAHeader->SetupSize);

		fclose(fin);

		// Exec setup.exe and wait for it to close
		ExecProgram(gpi->hwnd,
					PrefPath,
					"Setup.exe");

		// Wait for setup.exe spawned processes to close
		do
		{
			FoundProcess=FALSE;

			// Main installshield program
			hwndWin=FindWindow("InstallShield_Win",NULL);
		
			if(hwndWin!=NULL)
			{
				FoundProcess=TRUE;

				GetWindowThreadProcessId(hwndWin,&dwProcID);
				hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwProcID);
				// Wait for process to end
				WaitForSingleObject(hProcess,INFINITE);
				CloseHandle(hProcess);
			}

			// Is delete cleanup program
			hwndWin=FindWindow("isdelete",NULL);
		
			if(hwndWin!=NULL)
			{
				FoundProcess=TRUE;

				GetWindowThreadProcessId(hwndWin,&dwProcID);
				hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwProcID);
				// Wait for process to end
				WaitForSingleObject(hProcess,INFINITE);
				CloseHandle(hProcess);
			}
		} while(FoundProcess);
	}

	// Remove the files the SEA extracted

	while(gpi->fl!=NULL)
	{
		freeatlast=gpi->fl;
		gpi->fl=gpi->fl->next;

		// Erase SEA files
		if(freeatlast->IsDirectory)
		{
			retval=_rmdir(freeatlast->name);
		}
		else
		{
			retval=_unlink(freeatlast->name);
		}
		free(freeatlast);
	}
}

void SEADoInstallerCommand(void)
{
	DWORD dwReboot;
	char filename[MAX_PATH+1];

	// Basically run once. We delete after completion.
	DeleteRunRegKey();

	// See if the installer wants us to reboot or relaunch
	dwReboot=ReadRebootRegKey();

	switch(dwReboot)
	{
		case 1:
		{
			SEAReboot();
			break;
		}

		case 2:
		{
			GetModuleFileName(NULL, filename, MAX_PATH);

			WriteRunRegKey(filename);
			SEAReboot();
			break;
		}

		case 3:
		{
			DeletePGPRegKey();
			SEAReboot();
			break;
		}
	}
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

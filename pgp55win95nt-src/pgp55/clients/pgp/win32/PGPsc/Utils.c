/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Utils.c,v 1.23.2.2 1997/11/20 21:17:49 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

#define PGPPATHREG "Software\\PGP\\PGP55\\PGPkeys"

BOOL WindowExists (LPSTR lpszClassName, LPSTR lpszWindowTitle) 
{
	HWND hWndMe;

   	hWndMe = FindWindow (lpszClassName, lpszWindowTitle);
    if (hWndMe) 
    {
		if (!IsZoomed (hWndMe)) 
			ShowWindow (hWndMe, SW_RESTORE);
       	BringWindowToTop(hWndMe);
		SetForegroundWindow(hWndMe);
        return TRUE;
	}  
	else
		return FALSE;
}

BOOL InitPGPsc(HWND hwnd,void **PGPsc)
{
	PGPError err;
	PGPContextRef context;

	if(PGPcomdlgIsExpired(hwnd))
		return FALSE;

	err=PGPNewContext( kPGPsdkAPIVersion, &context );

	if( IsPGPError(err) )
	{
		if(err == kPGPError_IncompatibleAPI)
		{
			MessageBox(hwnd,"Incompatible SDK API. You might try reinstalling.",
							"PGP Error",MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
		}

		if(err == kPGPError_FeatureNotAvailable)
		{
			MessageBox(hwnd,"The evaluation period for PGP has passed.",
							"PGP Expired",MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
		}

		return FALSE;
	}

	*PGPsc=(void *)context;

	return TRUE;
}

BOOL UninitPGPsc(HWND hwnd,void *PGPsc)
{
	PGPError err;
	PGPContextRef context;
	
	context=(PGPContextRef)PGPsc;

	while (PGPPurgeCachedPhrase() || PGPPurgeSignCachedPhrase())
	{
		Sleep(250);
	}

	PGPcomdlgCloseLibrary ();

	err=PGPFreeContext(context);

	if(IsntPGPError(err))
		return TRUE;

	return FALSE;
}

BOOL OpenRings(HWND hwnd,PGPContextRef context,PGPKeySetRef *pkeyset)
{
	PGPError err;

	PGPsdkLoadDefaultPrefs(context);

	err=PGPOpenDefaultKeyRings( context,
		OPENKEYRINGFLAGS,pkeyset);

	if(IsPGPError(err))
	{
		MessageBox(hwnd,
			"Could not open default key rings. Another\n"
			"application might have them open?",
			"PGPSDK Error",MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
		return FALSE;
	}
	
	return TRUE;
}


void PGPDebugMessage(char *msg)
{
	MessageBox(NULL,msg,"DebugMessage",MB_OK);
}

void PGPpath(char *szPath)
{
	strcpy(szPath,"");

	PGPcomdlgGetPGPPath (szPath,MAX_PATH);
}

char *JustFile(char *filename)
{
	char *p;

	p = strrchr(filename, '\\');

	if(p!=0)
		return (p+1);
	else
		return filename;
}

BOOL GetValidityDrawPrefs(BOOL *pbInputBool)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;
	PGPBoolean  bDisplayMarginal;

	bDisplayMarginal=FALSE;

	err=PGPcomdlgOpenClientPrefs (&PrefRefClient);
	
	if(IsntPGPError(err))
	{
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefDisplayMarginalValidity, &bDisplayMarginal);
		PGPcomdlgCloseClientPrefs (PrefRefClient, FALSE);

		*pbInputBool=FALSE;

		if(bDisplayMarginal)
			*pbInputBool=TRUE;

		return TRUE;
	}

	return FALSE;
}

BOOL GetMarginalInvalidPref(BOOL *pbInputBool)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;
	PGPBoolean  bMarginalInvalid;

	bMarginalInvalid=FALSE;

	err=PGPcomdlgOpenClientPrefs (&PrefRefClient);
	
	if(IsntPGPError(err))
	{
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefMarginalIsInvalid, &bMarginalInvalid);
		PGPcomdlgCloseClientPrefs (PrefRefClient, FALSE);

		*pbInputBool=FALSE;

		if(bMarginalInvalid)
			*pbInputBool=TRUE;
		
		return TRUE;
	}

	return FALSE;
}

UINT FileListFromFile(FILELIST **filelist,char *filename)
{
	FILELIST *Current;

//	vErrorOut(fg_yellow,"%s\n",filename);

	Current=(FILELIST *)malloc(sizeof(FILELIST));
    memset(Current,0x00,sizeof(FILELIST));

	strcpy(Current->name,filename);
	Current->next=*filelist;
	*filelist=Current;

	return TRUE;
}


UINT FileListFromDirectory(FILELIST **filelist,char *directory)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA FindFileData;
	BOOL RetVal=FALSE;
	char search[MAX_PATH];
	char filename[MAX_PATH];

	if(directory[strlen(directory)]!='\\')
		strcat(directory,"\\");

	strcpy(search,directory);

	strcat(search,"*.*");

	hFindFile=FindFirstFile(search,&FindFileData);

	if(hFindFile!=INVALID_HANDLE_VALUE)
	{
		RetVal=TRUE;

		while(RetVal==TRUE)
		{
			strcpy(filename,directory);
			strcat(filename,FindFileData.cFileName);

			if((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				!=FILE_ATTRIBUTE_DIRECTORY)
			{
				FileListFromFile(filelist,filename);
			}
			else
			{
				if((strcmp(FindFileData.cFileName,"."))&&
					(strcmp(FindFileData.cFileName,"..")))
				{
					FileListFromFile(filelist,filename);
					(*filelist)->IsDirectory=TRUE;
					FileListFromDirectory(filelist,filename);
				}
			}
			RetVal=FindNextFile(hFindFile,&FindFileData); 
		}

		RetVal=FindClose(hFindFile);
	}

	return TRUE;
} 

UINT AddToFileList(FILELIST **filelist,char *filename)
{
	DWORD dwFileAttributes;
	UINT RetVal;

	dwFileAttributes=GetFileAttributes(filename);

	if(dwFileAttributes==0xFFFFFFFF)
		return FALSE;

	if((dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		==FILE_ATTRIBUTE_DIRECTORY)
	{
		FileListFromFile(filelist,filename);
		(*filelist)->IsDirectory=TRUE;
		RetVal=FileListFromDirectory(filelist,filename);
	}
	else
	{
		RetVal=FileListFromFile(filelist,filename);
	}

	return RetVal;
}

UINT FreeFileList(FILELIST *FileList)
{
	FILELIST *FreeAtLast;

	while(FileList!=0)
	{
		FreeAtLast=FileList;
		FileList=FileList->next;
		memset(FreeAtLast,0x00,sizeof(FILELIST));
		free(FreeAtLast);
	}

	return TRUE;
}

UINT PGPscQueryCacheSecsRemaining (void)
{
	return PGPQueryCacheSecsRemaining ();
}

UINT PGPscPreferences(HWND hwnd,void *PGPsc)
{
	PGPContextRef context;

	context=(PGPContextRef)PGPsc;

	PGPcomdlgPreferences(context,hwnd, 0);

	return TRUE;
}

UINT PGPscAbout (HWND hwnd)
{
	return PGPcomdlgHelpAbout (hwnd,NULL,NULL,NULL);
}

unsigned long DoLaunchKeys(HWND hwnd)
{  
	char PGPkeys[MAX_PATH];
	int error;
	
	PGPpath(PGPkeys);

// Since Windows can't handle tray launched from CD
	SetCurrentDirectory(PGPkeys);

	strcat(PGPkeys,"PGPkeys.exe /s");
	error=WinExec(PGPkeys, SW_SHOW);
 
	if(error<=31)
	{ 
		MessageBox(NULL,PGPkeys,
			"Could not open PGPkeys.exe",
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return FALSE;
	}

	return TRUE;
}

unsigned long DoLaunchTools(HWND hwnd)
{  
	char PGPtools[MAX_PATH];
	int error;
	
	PGPpath(PGPtools);

// Since Windows can't handle tray launched from CD
	SetCurrentDirectory(PGPtools);

	strcat(PGPtools,"PGPtools.exe");
	error=WinExec(PGPtools, SW_SHOW);
 
	if(error<=31)
	{ 
		MessageBox(NULL,PGPtools,
			"Could not open PGPtools.exe",
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return FALSE;
	}

	return TRUE;
}

void FreewareScreen(HINSTANCE hInstance)
{
/*
#if PGP_FREEWARE
	PGPcomdlgSplash (hInstance, NULL, 10); // 10 is non-zero dummy
#endif // PGP_FREEWARE
*/
}

void FreewareSetParent(HWND hwnd)
{
/*
#if PGP_FREEWARE
	PGPcomdlgSetSplashParent(hwnd);
#endif // PGP_FREEWARE
*/
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

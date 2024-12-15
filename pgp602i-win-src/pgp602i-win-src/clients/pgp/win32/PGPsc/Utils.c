/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Utils.c,v 1.57.2.6.2.1 1998/11/12 03:13:30 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include "PGPhk.h"

#define PGPPATHREG "Software\\Network Associates\\PGP60\\PGPkeys"

DWORD g_uPurgePassphraseCacheMessage;

#pragma data_seg(".SHARDAT")
static UINT g_KeyArray[4]={0,0,0,0};
static UINT * lpJournalKeys = 0;
static UINT g_KeyIndex = 0;
static UINT g_NumKeys = 0;
static HHOOK  g_hJournalHook=NULL ;
#pragma data_seg()

#define LAUNCHKEYS_TIMER		53
#define MILLISECONDS_PER_DAY	86400000

#define SK_KEYDOWN  0x8000
#define SK_SYSKEY   0x4000

BOOL WINAPI SendKeys( UINT *lpKeys, UINT nKeys );

LRESULT 
PGPscMessageBox (
		 HWND	hWnd, 
		 INT	iCaption, 
		 INT	iMessage,
		 ULONG	ulFlags) 
{
	CHAR szCaption [128];
	CHAR szMessage [500];

	LoadString (g_hinst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hinst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}

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

BOOL InitPGPsc(HWND hwnd,void **PGPsc,void **PGPtls)
{
	PGPError err;
	PGPContextRef context;
	PGPtlsContextRef tls;
	DWORD OLERetVal;

	err=PGPNewContext( kPGPsdkAPIVersion, &context );

	if( IsPGPError(err) )
	{
		if(err == kPGPError_IncompatibleAPI)
		{
			PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_WRONGSDK,
					MB_OK|MB_ICONSTOP);
		}
		else
		if(err == kPGPError_FeatureNotAvailable)
		{
			PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_EVALEXPIRED,
					MB_OK|MB_ICONSTOP);
		}
		else
			PGPclErrorBox(hwnd,err);

		return FALSE;
	}

	if(IsPGPError(PGPclIsExpired(hwnd)))
		return FALSE;

	err=PGPNewTLSContext (context, &tls);

	if( IsPGPError(err) )
	{
		PGPclErrorBox(hwnd,err);
		return FALSE;
	}

	err=PGPclInitLibrary(context);

	if(err==kPGPError_UserAbort)
		return FALSE;

	if( IsPGPError(err) )
	{
		PGPclErrorBox(hwnd,err);
		return FALSE;
	}

	OLERetVal=OleInitialize (NULL);

	switch(OLERetVal)
	{
		case S_OK:
			// The COM library and additional functionality were 
			// initialized successfully on this apartment. 
			break;

		case S_FALSE:
		{
			// The COM library is already initialized on this 
			// apartment.
			break;
		}

		case OLE_E_WRONGCOMPOBJ:
		{
			MessageBox(hwnd,
				"The versions of COMPOBJ.DLL and OLE2.DLL on\n"
				"your machine are incompatible with each other.",
				"OLE Error",MB_OK);
			return FALSE;
		}

		default:
		{
			MessageBox(hwnd,
				"Error initializing OLE.",
				"OLE Error",MB_OK);
			return FALSE;
		}
	}

	*PGPsc=(void *)context;
	*PGPtls=(void *)tls;

	g_uPurgePassphraseCacheMessage = 
		RegisterWindowMessage (PURGEPASSPHRASECACEHMSG);

	return TRUE;
}

BOOL UninitPGPsc(HWND hwnd,void *PGPsc,void *PGPtls)
{
	PGPError err;
	PGPContextRef context;
	PGPtlsContextRef tls;

	context=(PGPContextRef)PGPsc;
	tls=(PGPtlsContextRef)PGPtls;

	PGPclCloseLibrary ();

	err=PGPFreeTLSContext(tls);

	err=PGPFreeContext(context);

	OleUninitialize();

	if(IsntPGPError(err))
		return TRUE;

	return FALSE;
}

BOOL CheckForPurge(UINT uMsg,WPARAM wParam)
{
	if (uMsg == g_uPurgePassphraseCacheMessage)
	{
		PGPclPurgeCachedPassphrase (wParam);
			return TRUE;
	}

	return FALSE;
}

OUTBUFFLIST *MakeOutBuffItem(OUTBUFFLIST **obl)
{
	OUTBUFFLIST *nobl,*lastobl,*indexobl;

	nobl=(OUTBUFFLIST *)malloc(sizeof(OUTBUFFLIST));
	memset(nobl,0x00,sizeof(OUTBUFFLIST));

	lastobl=indexobl=*obl;

	while(indexobl!=0)
	{
		lastobl=indexobl;
		indexobl=indexobl->next;
	}

	if(lastobl==0)
		*obl=nobl;
	else
		lastobl->next=nobl;

	return nobl;
}

BOOL ConcatOutBuffList(void *PGPsc,
					   OUTBUFFLIST *obl,
					   char **pBuff,
					   DWORD *dwBuffSize,
					   BOOL *FYEO)
{
	OUTBUFFLIST *indexobl,*prevobl;
	DWORD memindex;
	PGPMemoryMgrRef	memmgr;
	PGPContextRef context;

	context=(PGPContextRef)PGPsc;

	*pBuff=0;
	*dwBuffSize=0;
	*FYEO=FALSE;

	if(obl==NULL)
		return TRUE;

	memmgr = PGPGetContextMemoryMgr (context);

	// Find size and whether complete block is FYEO or not
	indexobl=obl;
	*dwBuffSize=0;
	*FYEO=FALSE;

	while(indexobl!=0)
	{
		if(indexobl->FYEO)
		{
			*FYEO=TRUE;
		}
		*dwBuffSize=*dwBuffSize+indexobl->dwBuffSize;
		indexobl=indexobl->next;
	}

	// Allocate new memory buffer
	if(*FYEO)
	{
		*pBuff=PGPNewSecureData (memmgr, *dwBuffSize+1, 0);
	}
	else
	{
		*pBuff=PGPNewData (memmgr, *dwBuffSize+1, 0);
	}

	if(*pBuff==NULL)
	{
		return FALSE;
	}

	// +1 for terminating null
	memset(*pBuff,0x00,*dwBuffSize+1);

	// Concatenate memory blocks and free originals/linked list
	indexobl=obl;
	memindex=0;

	while(indexobl!=0)
	{
		memcpy(&((*pBuff)[memindex]),indexobl->pBuff,indexobl->dwBuffSize);
		memset(indexobl->pBuff,0x00,indexobl->dwBuffSize);
		PGPFreeData(indexobl->pBuff);
		memindex=memindex+indexobl->dwBuffSize;

		prevobl=indexobl;
		indexobl=indexobl->next;
		free(prevobl);
	}

	return TRUE;
}

BOOL OpenRings(HWND hwnd,PGPContextRef context,PGPKeySetRef *pkeyset)
{
	PGPError err;

	PGPsdkLoadDefaultPrefs(context);

	err=PGPOpenDefaultKeyRings( context,
		OPENKEYRINGFLAGS,pkeyset);

	if(IsPGPError(err))
	{
		PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_CANTOPENRINGS,
				MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	
	return TRUE;
}

void PGPpath(char *szPath)
{
	strcpy(szPath,"");

	PGPclGetPGPPath (szPath,MAX_PATH);
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

BOOL GetValidityDrawPrefs(void *PGPsc,
						  BOOL *pbInputBool)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;
	PGPBoolean  bDisplayMarginal;
	PGPMemoryMgrRef memMgr;
	PGPContextRef context;

	context=(PGPContextRef)PGPsc;

	memMgr=PGPGetContextMemoryMgr(context);

	bDisplayMarginal=FALSE;

	err=PGPclOpenClientPrefs (memMgr,&PrefRefClient);
	
	if(IsntPGPError(err))
	{
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefDisplayMarginalValidity, &bDisplayMarginal);
		PGPclCloseClientPrefs (PrefRefClient, FALSE);

		*pbInputBool=FALSE;

		if(bDisplayMarginal)
			*pbInputBool=TRUE;

		return TRUE;
	}

	return FALSE;
}

BOOL GetMarginalInvalidPref(void *PGPsc,
							BOOL *pbInputBool)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;
	PGPBoolean  bMarginalInvalid;
	PGPMemoryMgrRef memMgr;
	PGPContextRef context;

	context=(PGPContextRef)PGPsc;

	memMgr=PGPGetContextMemoryMgr(context);

	bMarginalInvalid=FALSE;

	err=PGPclOpenClientPrefs (memMgr,&PrefRefClient);
	
	if(IsntPGPError(err))
	{
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefMarginalIsInvalid, &bMarginalInvalid);
		PGPclCloseClientPrefs (PrefRefClient, FALSE);

		*pbInputBool=FALSE;

		if(bMarginalInvalid)
			*pbInputBool=TRUE;
		
		return TRUE;
	}

	return FALSE;
}

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

UINT FileListFromDirectory(FILELIST **filelist,char *directory,BOOL *UserCancel)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA FindFileData;
	BOOL RetVal=FALSE;
	BOOL KeepGoing;
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
			// Our quick fix callback to allow user to back out
			// of operation
			if(UserCancel!=NULL)
			{
				if(*UserCancel)
				{
					FindClose(hFindFile);
					return FALSE;
				}
			}

			strcpy(filename,directory);
			strcat(filename,FindFileData.cFileName);

			if((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				!=FILE_ATTRIBUTE_DIRECTORY)
			{
				FileListFromFile(filelist,filename,UserCancel);
			}
			else
			{
				if((strcmp(FindFileData.cFileName,"."))&&
					(strcmp(FindFileData.cFileName,"..")))
				{
					KeepGoing=FileListFromFile(filelist,filename,UserCancel);
					if(!KeepGoing)
					{
						FindClose(hFindFile);
						return FALSE;
					}

					(*filelist)->IsDirectory=TRUE;
					KeepGoing=FileListFromDirectory(filelist,filename,UserCancel);

					if(!KeepGoing)
					{
						FindClose(hFindFile);
						return FALSE;
					}
				}
			}
			RetVal=FindNextFile(hFindFile,&FindFileData); 
		}

		FindClose(hFindFile);
	}

	return TRUE;
} 

BOOL AddToFileList(FILELIST **filelist,char *filename,BOOL *UserCancel)
{
	DWORD dwFileAttributes;
	UINT RetVal;

	dwFileAttributes=GetFileAttributes(filename);

	if(dwFileAttributes==0xFFFFFFFF)
		return TRUE;

	if((dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		==FILE_ATTRIBUTE_DIRECTORY)
	{
		RetVal=FileListFromFile(filelist,filename,UserCancel);
		if(!RetVal)
			return FALSE;

		(*filelist)->IsDirectory=TRUE;
		RetVal=FileListFromDirectory(filelist,filename,UserCancel);
	}
	else
	{
		RetVal=FileListFromFile(filelist,filename,UserCancel);
	}

	return RetVal;
}

BOOL FreeFileList(FILELIST *FileList)
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

FILELIST *HDropToFileList(HDROP hDrop)
{
	char szFile[MAX_PATH+1];
	int NumFiles,i,ReturnCode;
	FILELIST *ListHead;

	NumFiles=DragQueryFile(hDrop,0xFFFFFFFF,NULL,0);
	ListHead=0;
	ReturnCode=TRUE;

	if(NumFiles==0)
		return 0;

	for(i=0;i<NumFiles && ReturnCode == TRUE;i++)
	{
		DragQueryFile(hDrop,i,szFile,MAX_PATH);
		AddToFileList(&ListHead,szFile,NULL);
	}

	return ListHead;
}

FILELIST *CmdLineToFileList(char *szCmdLine)
{
	char *szNext,*szNow;
	FILELIST *ListHead;

	szNext=szCmdLine;
	ListHead=0;

	while(szNext!=NULL)
	{
		szNow=szNext;
			
		szNext=strstr(szNext," ");
		
		if(szNext!=NULL)
		{
			szNext=strstr(szNext,":\\");

			if(szNext!=NULL)
			{
				szNext=szNext-2;
				if(szNext>szCmdLine)
				{
					*szNext=0;
					szNext++;
				}
				else
				{
					szNext=NULL;
				}
			}
		}
		AddToFileList(&ListHead,szNow,NULL);
	}

	return ListHead;
}

UINT PGPscQueryCacheSecsRemaining (void)
{
	UINT SignCacheSecs;
	UINT DecCacheSecs;

	SignCacheSecs=PGPclQuerySigningCacheSecsRemaining();
	DecCacheSecs=PGPclQueryDecryptionCacheSecsRemaining ();

	if(SignCacheSecs>DecCacheSecs)
		return SignCacheSecs;

	return DecCacheSecs;
}

BOOL PGPscPreferences(HWND hwnd,void *PGPsc,void *PGPtls)
{
	PGPContextRef context;
	PGPtlsContextRef tls;

	context=(PGPContextRef)PGPsc;
	tls=(PGPtlsContextRef)PGPtls;

	PGPclPreferences(context,hwnd, 0, NULL);

	return TRUE;
}

BOOL PGPscAbout (HWND hwnd,void *PGPsc)
{
	PGPContextRef context;

	context=(PGPContextRef)PGPsc;

	PGPclHelpAbout (context,hwnd,NULL,NULL,NULL);

	return TRUE;
}

BOOL DoLaunchKeys(HWND hwnd)
{  
	char PGPkeys[MAX_PATH];
	int error;
	char StrRes[500];
	
	PGPpath(PGPkeys);

// Since Windows can't handle tray launched from CD
	SetCurrentDirectory(PGPkeys);

	strcat(PGPkeys,"PGPkeys.exe /s");
	error=WinExec(PGPkeys, SW_SHOW);
 
	if(error<=31)
	{ 
		LoadString (g_hinst, IDS_CANTOPENPGPKEYS, StrRes, sizeof(StrRes));

		MessageBox(hwnd,PGPkeys,
			StrRes,
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return FALSE;
	}

	return TRUE;
}

BOOL DoLaunchTools(HWND hwnd)
{  
	char PGPtools[MAX_PATH];
	int error;
	char StrRes[500];

	PGPpath(PGPtools);

// Since Windows can't handle tray launched from CD
	SetCurrentDirectory(PGPtools);

	strcat(PGPtools,"PGPtools.exe");
	error=WinExec(PGPtools, SW_SHOW);
 
	if(error<=31)
	{ 
		LoadString (g_hinst, IDS_CANTOPENPGPTOOLS, StrRes, sizeof(StrRes));

		MessageBox(hwnd,PGPtools,
			StrRes,
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return FALSE;
	}

	return TRUE;
}

BOOL DoLaunchDisk(HWND hwnd)
{  
	char PGPtools[MAX_PATH];
	int error;
	char StrRes[500];

	PGPpath(PGPtools);

// Since Windows can't handle tray launched from CD
	SetCurrentDirectory(PGPtools);

	strcat(PGPtools,"PGPdisk.exe");
	error=WinExec(PGPtools, SW_SHOW);
 
	if(error<=31)
	{ 
		LoadString (g_hinst, IDS_CANTOPENPGPDISK, StrRes, sizeof(StrRes));

		MessageBox(hwnd,PGPtools,
			StrRes,
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return FALSE;
	}

	return TRUE;
}

BOOL CheckClipboard()
{
	char *pInput;
	DWORD dwInputSize;
	UINT ClipboardFormat;
	BOOL RetVal;

	RetVal=FALSE;

	pInput=RetrieveClipboardData
			(NULL,&ClipboardFormat,&dwInputSize);

	if((ClipboardFormat == CF_TEXT)&&(pInput!=0))
	{
		if(*pInput!=0)
		{
			RetVal=TRUE;
		}
	}

	if(pInput!=0)
	{
		memset(pInput,0x00,dwInputSize);
		free(pInput);
	}

	return RetVal;
}

void ControlA(void)
{
	g_KeyArray[0]=VK_CONTROL|SK_KEYDOWN;
	g_KeyArray[1]='A'|SK_KEYDOWN;
	g_KeyArray[2]='A';
	g_KeyArray[3]=VK_CONTROL;

	SendKeys( g_KeyArray, 4 );
}

void ControlC(void)
{
	g_KeyArray[0]=VK_CONTROL|SK_KEYDOWN;
	g_KeyArray[1]='C'|SK_KEYDOWN;
	g_KeyArray[2]='C';
	g_KeyArray[3]=VK_CONTROL;

	SendKeys( g_KeyArray, 4 );
}

void ControlV(void)
{
	g_KeyArray[0]=VK_CONTROL|SK_KEYDOWN;
	g_KeyArray[1]='V'|SK_KEYDOWN;
	g_KeyArray[2]='V';
	g_KeyArray[3]=VK_CONTROL;

	SendKeys( g_KeyArray, 4 );
}

void EMSetSel(HWND hwndEdit)
{
	SendMessage(hwndEdit,EM_SETSEL,0,-1);
}

void WMCopy(HWND hwndEdit)
{
	SendMessage(hwndEdit,WM_COPY,0,0);
}

BOOL DoCopy(HWND hwnd,void *PGPsc,BOOL bUseCurrent,HWND *hwndFocus)
{
	BOOL ClipOK;
	HWND hwndEdit;
	BOOL RetVal=TRUE;
	char WinClass[256];
	PGPContextRef context;
	DWORD OtherAppThreadID;
	DWORD OtherAppProcessID;
	HWND hwndOldFocus;
	DWORD TrayThreadID;

	context=(PGPContextRef)PGPsc;

	*hwndFocus=NULL;

	if(bUseCurrent)
	{
		TrayThreadID=GetCurrentThreadId();
		hwndEdit=ReadCurrentFocus();
		*hwndFocus=hwndEdit;

		GetClassName(hwndEdit,WinClass,255);

		ClipboardWipe(NULL,context);

		// We can key off WinClass if something else
		// needs to be done 

		OtherAppThreadID = GetWindowThreadProcessId( hwndEdit, 
			&OtherAppProcessID);

		AttachThreadInput( TrayThreadID, OtherAppThreadID, TRUE );

		hwndOldFocus=SetFocus(hwndEdit);

		WMCopy(hwndEdit);

		ClipOK=CheckClipboard();
		if(!ClipOK)
		{
			// We can key off WinClass if something else
			// needs to be done 
			EMSetSel(hwndEdit);
			WMCopy(hwndEdit);

			ClipOK=CheckClipboard();
			if(!ClipOK)
			{
				ControlC();
			
				ClipOK=CheckClipboard();
				if(!ClipOK)
				{
					ControlA();
					ControlC();

					ClipOK=CheckClipboard();
					if(!ClipOK)
					{
						RetVal=FALSE;
					}
				}
			}
		}

		AttachThreadInput( TrayThreadID, OtherAppThreadID, FALSE );
		SetFocus(hwndOldFocus);
	}

	if(!RetVal)
	{
#if _DEBUG
		MessageBox(hwnd,WinClass,"Couldn't get text. _DEBUG VERSION ONLY_ info",
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
#else
		PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_CANTAUTOCOPY,
			MB_OK|MB_ICONSTOP);
#endif
	}
	return RetVal;
}

void DoPaste(BOOL bUseCurrent,HWND hwndFocus)
{
	HWND hwndEdit;
	char WinClass[256];
	DWORD OtherAppThreadID;
	DWORD OtherAppProcessID;
	DWORD TrayThreadID;

	if(bUseCurrent)
	{
		TrayThreadID=GetCurrentThreadId();
		hwndEdit=hwndFocus; 
		GetClassName(hwndEdit,WinClass,255);

		OtherAppThreadID = GetWindowThreadProcessId( hwndEdit, 
			&OtherAppProcessID);

		AttachThreadInput( TrayThreadID, OtherAppThreadID, TRUE );

		// We can key off WinClass if something else
		// needs to be done 

		SetFocus(hwndEdit);

		ControlV();

		AttachThreadInput( TrayThreadID, OtherAppThreadID, FALSE );
	}
}

void DoFocus(BOOL bUseCurrent,HWND hwndFocus)
{
	HWND hwndEdit;
	char WinClass[256];
	DWORD OtherAppThreadID;
	DWORD OtherAppProcessID;
	DWORD TrayThreadID;

	if(bUseCurrent)
	{
		TrayThreadID=GetCurrentThreadId();
		hwndEdit=hwndFocus; 
		GetClassName(hwndEdit,WinClass,255);

		OtherAppThreadID = GetWindowThreadProcessId( hwndEdit, 
			&OtherAppProcessID);

		AttachThreadInput( TrayThreadID, OtherAppThreadID, TRUE );

		// We can key off WinClass if something else
		// needs to be done 

		SetFocus(hwndEdit);

		AttachThreadInput( TrayThreadID, OtherAppThreadID, FALSE );
	}
}

void StartUpdateTimer(HWND hwnd, UINT *pnLaunchKeysTimer)
{
	if (pnLaunchKeysTimer != NULL)
		*pnLaunchKeysTimer = 0;

	if (hwnd == NULL)
		return;

	UpdateTimerProc(hwnd, WM_TIMER, LAUNCHKEYS_TIMER, 0);

	*pnLaunchKeysTimer = SetTimer(hwnd, LAUNCHKEYS_TIMER, 
									MILLISECONDS_PER_DAY,
									(TIMERPROC) UpdateTimerProc);
	return;
}


VOID CALLBACK UpdateTimerProc(HWND hwnd, 
							  UINT uMsg, 
							  UINT idEvent, 
							  DWORD dwTime)
{
	PGPMemoryMgrRef memoryMgr;
	PGPBoolean		bUpdateAllKeys;
	PGPBoolean		bUpdateTrustedIntroducers;
	PGPError		err = kPGPError_NoErr;

	if (idEvent != LAUNCHKEYS_TIMER)
		return;

	err = PGPNewMemoryMgr(0, &memoryMgr);
	if (IsPGPError(err))
	{
		PGPclErrorBox(hwnd, err);
		return;
	}

	err = PGPclCheckAutoUpdate(memoryMgr, FALSE, 
			&bUpdateAllKeys, &bUpdateTrustedIntroducers);
	if (IsPGPError(err))
	{
		PGPclErrorBox(hwnd, err);
		return;
	}

	if (bUpdateAllKeys || bUpdateTrustedIntroducers)
		DoLaunchKeys(hwnd);

	PGPFreeMemoryMgr(memoryMgr);
	return;
}
 
BOOL FileHasThisExtension(char *filename,char *extension)
{
	char *p;

	if(filename!=NULL)
	{
		if((p = strrchr(filename, '\\')))
			filename=p; // we just want the filename not path

		if((p = strrchr(filename, '.')))
		{
			p++;

			if(!strcmpi(p,extension))
				return TRUE;
		}
	}

	return FALSE;
}
	
LRESULT CALLBACK PlaybackHook( int nCode, WPARAM wParam,
                 LPARAM lParam )
{
   LPEVENTMSG lpEM;

   switch(nCode) {
      case HC_SKIP:
		 g_KeyIndex++;
         if( g_KeyIndex == g_NumKeys ) {  // no more keys?
            // remove hook
            UnhookWindowsHookEx(g_hJournalHook);
            g_hJournalHook = NULL;
         }
         return 0;

      case HC_GETNEXT:
         lpEM = (LPEVENTMSG)lParam;
         if( lpJournalKeys[g_KeyIndex] & SK_KEYDOWN )
            lpEM->message = lpJournalKeys[g_KeyIndex] & SK_SYSKEY
                                        ? WM_SYSKEYDOWN : WM_KEYDOWN;
         else
            lpEM->message = lpJournalKeys[g_KeyIndex] & SK_SYSKEY 
                                        ? WM_SYSKEYUP : WM_KEYUP;
         lpEM->paramL = LOBYTE(lpJournalKeys[g_KeyIndex])
               | (MapVirtualKey(LOBYTE(lpJournalKeys[g_KeyIndex]),0)<<8);
         lpEM->paramH = 1;
         lpEM->time = GetCurrentTime();
         return 0;

      default:
         return CallNextHookEx(g_hJournalHook,nCode,wParam,lParam);
   }
}

#define KeyDown( uKey ) ( GetAsyncKeyState(uKey) & 0x8000 )

BOOL WINAPI SendKeys( UINT *lpKeys, UINT nKeys )
{
   lpJournalKeys = lpKeys;
   g_KeyIndex =   0;
   g_NumKeys =  nKeys;
   if( g_hJournalHook )
      return FALSE;
   while( KeyDown(VK_MENU) || KeyDown(VK_SHIFT)
                                || KeyDown(VK_CONTROL) ) {
      MSG msg;
      if( PeekMessage(&msg,NULL,0,0,PM_REMOVE) ) {
         if( msg.message == WM_QUIT ) {
            PostMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
            return FALSE;
         }
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }
   g_hJournalHook = SetWindowsHookEx(WH_JOURNALPLAYBACK,
      (HOOKPROC)PlaybackHook,g_hinst,0);
   if(!g_hJournalHook) return FALSE;
   // while hook is installed, pump messages
   while(g_hJournalHook) {
      MSG msg;
      if( PeekMessage(&msg,NULL,0,0,PM_REMOVE) ) {
         if( msg.message == WM_QUIT ) {
            if( g_hJournalHook )
               UnhookWindowsHookEx(g_hJournalHook);
            PostMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
            return FALSE;
         }
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }
   return TRUE;
}
		
/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

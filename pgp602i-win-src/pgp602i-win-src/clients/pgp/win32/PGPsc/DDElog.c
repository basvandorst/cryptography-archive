/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: DDElog.c,v 1.14.8.1 1998/11/12 03:13:26 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include <ddeml.h>

HDDEDATA CALLBACK myDdeCallback(UINT type,UINT fmt,HCONV hconv,
                                HSZ hsz1, HSZ hsz2, HDDEDATA hData,
								DWORD dwData1, DWORD dwData2)
{
	return NULL;
}

void InitPGPlogDDE(DWORD *pidInst)
{
	DdeInitialize(pidInst,(PFNCALLBACK) myDdeCallback,
		CBF_SKIP_ALLNOTIFICATIONS,0);
}

void UninitPGPlogDDE(DWORD idInst)
{
	DdeUninitialize(idInst);
}

BOOL PGPLogWindowExists (LPSTR lpszClassName, LPSTR lpszWindowTitle) 
{
	HWND hWndMe;

   	hWndMe = FindWindow (lpszClassName, lpszWindowTitle);
    if (hWndMe) 
    {
		if (!IsZoomed (hWndMe)) 
			ShowWindow (hWndMe, SW_RESTORE);
		SetWindowPos( hWndMe,HWND_TOP,0,0,0,0,
			SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOSIZE);
        return TRUE;
	}  
	else
		return FALSE;
}

BOOL SendPGPlogMsg(HWND hwnd,char* szMsg)
{
	HSZ hszServName;
	HSZ hszSysTopic;
	HCONV hConv;         // conversation handle 
	HDDEDATA hdded;
	HDDEDATA ddemsg;
	DWORD idInst;
	char StrRes[500];
	int error;
  
	if(!PGPLogWindowExists (DDElogServer,DDElogServer))
	{            
		char PGPlog[MAX_PATH];
		
		PGPpath(PGPlog);
		strcat(PGPlog,DDElogServer);
		
		error=WinExec(PGPlog,SW_SHOWNA);

		if(error<=31)
		{
			LoadString (g_hinst, IDS_CANTLOCATELOG, StrRes, sizeof(StrRes));

			MessageBox(hwnd,szMsg,StrRes,
				MB_ICONEXCLAMATION|MB_OK|MB_SETFOREGROUND);
			return FALSE;
		}
	}
	
	// Sigh... just in case log gets stuck in a silly
	// can't find memlock driver message box, we'll wait
	// here until the user lets log through
	while(!PGPLogWindowExists (DDElogServer,DDElogServer))
	{
		Sleep(500);
	}

	idInst=0;

	InitPGPlogDDE(&idInst);

	hszServName = DdeCreateStringHandle(idInst,DDElogServer,CP_WINANSI);
	hszSysTopic = DdeCreateStringHandle(idInst,DDElogTopic,CP_WINANSI);

	ddemsg=DdeCreateDataHandle(idInst,szMsg,strlen(szMsg)+1,0,0L,CF_TEXT,0);

	hConv = DdeConnect(idInst,hszServName,hszSysTopic,(PCONVCONTEXT) NULL);

	if(hConv==NULL)
	{              
		error=DdeGetLastError(idInst);
		UninitPGPlogDDE(idInst);
		LoadString (g_hinst, IDS_CANTCONNECTLOG, StrRes, sizeof(StrRes));

		MessageBox(hwnd,szMsg,StrRes,
			MB_ICONEXCLAMATION|MB_OK|MB_SETFOREGROUND);
		return FALSE;
	}

	hdded = DdeClientTransaction((void *)ddemsg,0xFFFFFFFF,
		hConv,(UINT)0, (UINT)0, XTYP_EXECUTE, TIMEOUT_ASYNC, NULL);

	DdeDisconnect(hConv);

	UninitPGPlogDDE(idInst);

	if(hdded==0)
	{
		LoadString (g_hinst, IDS_CANTTRANSMITLOG, StrRes, sizeof(StrRes));
		MessageBox(hwnd,szMsg,StrRes,
			MB_ICONEXCLAMATION|MB_OK|MB_SETFOREGROUND);
		return FALSE;
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

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: DDElog.c,v 1.9 1997/09/12 18:45:11 wjb Exp $
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

int SendPGPlogMsg(char* szMsg)
{
	HSZ hszServName;
	HSZ hszSysTopic;
	HCONV hConv;         // conversation handle 
	HDDEDATA hdded;
	HDDEDATA ddemsg;
	DWORD idInst;
	int error;
  
	if(!WindowExists (DDElogServer,DDElogServer))
	{            
		char PGPlog[MAX_PATH];
		
		PGPpath(PGPlog);
		strcat(PGPlog,DDElogServer);
		
		error=WinExec(PGPlog,SW_SHOW);

		if(error<=31)
		{
			MessageBox(NULL,szMsg,"Can't locate PGPlog",
				MB_ICONEXCLAMATION|MB_OK|MB_SETFOREGROUND);
			return FALSE;
		}
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
		MessageBox(NULL,szMsg,"Can't connect to PGPlog",
			MB_ICONEXCLAMATION|MB_OK|MB_SETFOREGROUND);
		return FALSE;
	}

	hdded = DdeClientTransaction((void *)ddemsg,0xFFFFFFFF,
		hConv,(UINT)0, (UINT)0, XTYP_EXECUTE, TIMEOUT_ASYNC, NULL);

	DdeDisconnect(hConv);

	UninitPGPlogDDE(idInst);

	if(hdded==0)
	{
		MessageBox(NULL,szMsg,"Can't transmit to PGPlog",
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

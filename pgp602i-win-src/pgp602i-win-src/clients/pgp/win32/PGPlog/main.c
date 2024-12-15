/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: main.c,v 1.20.2.2.2.1 1998/11/12 03:13:21 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

#define LOGREGPATH "Software\\Network Associates\\PGP60\\PGPlog"

LRESULT  CALLBACK WndProc     (HWND, UINT, WPARAM, LPARAM) ;
HDDEDATA CALLBACK DdeCallback (UINT, UINT, HCONV, HSZ, HSZ,
	HDDEDATA, DWORD, DWORD) ;

static float ColRatio[NUMCOLUMNS]={0.20f,0.30f,0.1f,0.39f};
static char *ColText[NUMCOLUMNS]={"Name","Signer","Validity","Signed"};

DWORD idInst ;
HSZ hszService, hszTopic;
HWND hwndList;
DRAWDATA *ddlist;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
		    LPSTR szCmdLine, int iCmdShow)
{
    MSG msg;
    WNDCLASS wndclass;
	HWND hwnd; 
	int error;
	DRAWDATA *FreeAtLast;

	if(WindowExists(DDElogServer,DDElogServer))
	{
		return TRUE;
	}
    
	if(!InitPGPsc(NULL,&PGPsc,&PGPtls))
			return TRUE;

	ddlist=0;

    g_hinst=hInstance;
 
    wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc   = (WNDPROC)WndProc ;
    wndclass.cbClsExtra    = 0 ;
    wndclass.cbWndExtra    = 0 ;
    wndclass.hInstance     = hInstance ;
    wndclass.hIcon         = LoadIcon(hInstance,
								MAKEINTRESOURCE(IDI_LOGICON));
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = DDElogServer;

    error=RegisterClass (&wndclass) ;

    hwnd = CreateWindow (DDElogServer, DDElogServer,
                  WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | 
				  WS_BORDER | WS_SYSMENU | WS_THICKFRAME |
				  WS_MAXIMIZEBOX,
                  CW_USEDEFAULT, CW_USEDEFAULT,
                  590,120,
                  NULL, NULL, hInstance, NULL) ;

    ShowWindow (hwnd, SW_SHOW);
    UpdateWindow (hwnd) ;

    // Initialize for using DDEML

    if (DdeInitialize (&idInst, (PFNCALLBACK) &DdeCallback,
                  CBF_FAIL_POKES |
                  CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS |
			 	  CBF_SKIP_CONNECT_CONFIRMS, 0))
	{ 
		MessageBox (hwnd,"Could not initialize PGPlog's DDEML server!",
                      DDElogServer, MB_ICONEXCLAMATION | MB_OK) ;

		DestroyWindow (hwnd) ;
		return FALSE ;
    }

   
    hszService = DdeCreateStringHandle (idInst, DDElogServer, 0) ;
    hszTopic   = DdeCreateStringHandle (idInst, DDElogTopic,   0) ;

    DdeNameService (idInst, hszService, NULL, DNS_REGISTER) ;

    while (GetMessage (&msg, NULL, 0, 0))
    {
		if( IsDialogMessage( hwnd, &msg ) )     
			//  Process Tab messages and such
			continue;

		TranslateMessage (&msg) ;
		DispatchMessage (&msg) ;
    }

    // Clean up

    DdeUninitialize (idInst) ;

	// Erase the juicy info
	while(ddlist!=0)
	{
		FreeAtLast=ddlist;
		ddlist=ddlist->next;

		memset(FreeAtLast->chunk,0x00,FreeAtLast->chunksize);
		free(FreeAtLast->chunk);

		memset(FreeAtLast,0x00,sizeof(DRAWDATA));
		free(FreeAtLast);
	}

	UninitPGPsc(NULL,PGPsc,PGPtls);

	return msg.wParam ;
}

HDDEDATA CALLBACK 
#ifndef WIN32 
_export 
#endif
DdeCallback (UINT iType, UINT iFmt, HCONV hConv,
                               HSZ hsz1, HSZ hsz2, HDDEDATA hData,
                               DWORD dwData1, DWORD dwData2)
{  

	switch (iType)
    {
		case XTYP_CONNECT :           
		{
			char szBuffer[256];

			DdeQueryString (idInst, hsz2, szBuffer, sizeof (szBuffer), 0) ;

            if (0 != strcmp (szBuffer, DDElogServer))
				return FALSE ;

            DdeQueryString (idInst, hsz1, szBuffer, sizeof (szBuffer), 0) ;

            if (0 != strcmp (szBuffer, DDElogTopic))
				return FALSE ;

            return (HDDEDATA) TRUE ;
        }
		
// From client since server will recieve instead
		case XTYP_EXECUTE :   
		{
			char szItem[1000];
			int index;
			DRAWDATA *dd;
			char *myString;
			char *start,*end;

			DdeGetData(hData,szItem,500,0);

			dd=(DRAWDATA *)malloc(sizeof(DRAWDATA));
			memset(dd,0x00,sizeof(DRAWDATA));

			dd->next=ddlist;  // Add to our linked list for cleanup
			ddlist=dd;

			dd->chunksize=strlen(szItem)+1;

			myString=(char *)malloc(dd->chunksize);
			strcpy(myString,szItem);

//			MessageBox(NULL,myString,"Information from client",MB_OK);

			dd->chunk=myString;
			dd->numcols=0;

			start=myString;

			end=strstr(myString,"\n");
			if(end!=0)
				*end=0;

			if(strstr(start,"Signature"))
			{
				if(!strcmp(start,"Good Signature"))
					dd->icon=IDX_CERT;
				else
					dd->icon=IDX_REVCERT; // Using Rev instead of bad now

				start=end+1;

				do
				{
					end=strstr(start,"\n");
					if(end!=0)
						*end=0;

					dd->type[dd->numcols]=PGP_DDTEXT;
					dd->data1[dd->numcols]=start;

					if(dd->numcols==2)
					{
						dd->type[dd->numcols]=PGP_DDBAR;
						dd->data2[dd->numcols]=(void *)2;
						if(!strcmp(start,"Invalid Key"))
							dd->data1[dd->numcols]=(void *)0;
						else if(!strcmp(start,"Marginal Key"))
							dd->data1[dd->numcols]=(void *)1;
						else if(!strcmp(start,"Valid Key"))
							dd->data1[dd->numcols]=(void *)2;
						else
							dd->data1[dd->numcols]=(void *)3;
					}

					dd->numcols++;
					start=end+1;
				}
				while(end!=0);

				index=AddAnItem(logList.hwndlist,dd);
				SetListCursor(logList.hwndlist,index);
			}

            return (HDDEDATA) DDE_FACK ;
		}
	}
    return NULL ;
}
      
void SetReg(HKEY hKey,char *item,DWORD value)
{
	RegSetValueEx (	hKey, 
		item, 
		0, 
		REG_DWORD, 
		(LPBYTE)&value, 
		sizeof(DWORD));
}

void SavePosition(HWND hwnd)
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dw = 0;
	RECT rect;

	GetWindowRect(hwnd, &rect);
	
	lResult = RegOpenKeyEx(	HKEY_CURRENT_USER,
							LOGREGPATH, 
							0, 
							KEY_ALL_ACCESS, 
							&hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		SetReg(hKey,"xPos",rect.left);
		SetReg(hKey,"yPos",rect.top);
		SetReg(hKey,"xSize",rect.right-rect.left);
		SetReg(hKey,"ySize",rect.bottom-rect.top);

		RegCloseKey (hKey);
	}

}

DWORD GetReg(HKEY hKey,char *item)
{
	DWORD value,type,size;

	value=0;
	type=0;
	size=sizeof(DWORD);

	RegQueryValueEx(hKey, 
		item, 
		0, 
		&type, 
		(LPBYTE)&value, 
		&size);

	return value;
}

void RememberPosition(HWND hwnd,RECT *rc)
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	x,y,dx,dy;

	x=y=dx=dy=0;

	lResult = RegOpenKeyEx(	HKEY_CURRENT_USER,
							LOGREGPATH, 
							0, 
							KEY_ALL_ACCESS, 
							&hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		x=GetReg(hKey,"xPos");
		y=GetReg(hKey,"yPos");

		dx=GetReg(hKey,"xSize");
		dy=GetReg(hKey,"ySize");

		RegCloseKey (hKey);
	}
	else
	{
		DWORD dw;

		lResult = RegCreateKeyEx (	HKEY_CURRENT_USER, 
									LOGREGPATH, 
									0, 
									NULL,
									REG_OPTION_NON_VOLATILE, 
									KEY_ALL_ACCESS, 
									NULL, 
									&hKey, 
									&dw);

	}

	if((dx==0)||(dy==0)||
		(x<0)||(y<0)||
		(x>(DWORD)GetSystemMetrics(SM_CXSCREEN))||
		(y>(DWORD)GetSystemMetrics(SM_CYSCREEN)))
	{
		dx=rc->right-rc->left;
		dy=rc->bottom-rc->top;
		x=(GetSystemMetrics(SM_CXSCREEN)-(rc->right-rc->left))/2;
		y=0;
	}

	SetWindowPos (hwnd,NULL,x,y,dx,dy,SWP_NOZORDER);
}

void QuitLog(void)
{
	DdeNameService (idInst, hszService, NULL, DNS_UNREGISTER) ;
    DdeFreeStringHandle (idInst, hszService) ;
    DdeFreeStringHandle (idInst, hszTopic) ;
	DeleteDrawElements(&logDraw);

    PostQuitMessage (0) ;
}
            
LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, 
						  WPARAM wParam, LPARAM lParam)
{

    switch (iMsg)
    {
		case WM_CREATE:
		{
			RECT rc;
			LOGFONT lf;

			InitCommonControls();
			   
            SystemParametersInfo (SPI_GETICONTITLELOGFONT, 
				sizeof(LOGFONT), &lf, 0);
			hFont = CreateFontIndirect (&lf);

			CreateDrawElements(&logDraw);
	  
		    GetClientRect(hwnd,&rc);

			InitList(hwnd,&logList,ColText,ColRatio);

			GetWindowRect (hwnd, &rc);

			MoveList(&logList,rc.right,rc.bottom);

			RememberPosition(hwnd,&rc);
		
			SetForegroundWindow(hwnd);
			return TRUE;
		}

        case WM_DRAWITEM: 
        {
            Main_OnDrawItem(hwnd,(LPDRAWITEMSTRUCT) lParam);
            return TRUE;
        }

        case WM_MEASUREITEM:
            Main_OnMeasureItem(hwnd,(LPMEASUREITEMSTRUCT) lParam);
            return TRUE;
  
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* lpmmi;

		    lpmmi = (MINMAXINFO*) lParam;
    		lpmmi->ptMinTrackSize.x = 350;
    		lpmmi->ptMinTrackSize.y = 120;
            break;
		}

		case WM_CTLCOLOR:
		{          
 			if((wParam!=0)&&(HIWORD(lParam)!=CTLCOLOR_EDIT))
			{ 
				DWORD color;
				color=GetSysColor(COLOR_BTNFACE);
				
			  	SetBkColor ((HDC)wParam, color);
			  	SetTextColor ((HDC)wParam, 
					GetSysColor(COLOR_WINDOWTEXT));      
			  	return (BOOL)CreateSolidBrush (color);
			} 	 
			break;
		}

		case WM_SIZE:
		{
			unsigned short Width, Height;
			Width = LOWORD(lParam);  // width of client area 
			Height = HIWORD(lParam); // height of client area 

			MoveList(&logList,Width,Height);
			break;
		}

		case WM_CLOSE:
        case WM_DESTROY :
		{
			SavePosition(hwnd);
			QuitLog();
            return 0 ;
        }
	}

	return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

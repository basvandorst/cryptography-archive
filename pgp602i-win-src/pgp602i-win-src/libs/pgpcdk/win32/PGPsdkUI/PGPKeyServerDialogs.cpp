/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: PGPKeyServerDialogs.cpp,v 1.19.10.1 1998/11/12 03:24:16 heller Exp $
____________________________________________________________________________*/

#include "PGPui32.h"
#include "pgpDialogs.h"
#include "pgpKeyServerDialogCommon.h"
#include "pgpKeyServer.h"
#include "pgpImage.h"

// local globals
extern HINSTANCE g_hInst;

typedef struct _KEYGBL 
{
	HWND hwndServerProgress;
	UINT mLastKeyServerState;
	PGPKeyServerRef mLastKeyServer;
	INT iStatusValue;
	INT iStatusDirection;
	PGPContextRef context;
	CPGPKeyServerDialogOptions *options;
	PGPError err;
	PGPBoolean bSearch;
	PGPBoolean bCancel;
	HANDLE hSemaphore;
	DWORD dwThreadID;
	char serverStr[255];
	HIMAGELIST hIml;
	HANDLE hThread;
	PGPBoolean bSecureConnection;
} KEYGBL;


//	____________________________________
//
//  Draw the "LED" progress indicator

#define LEDTIMER		111L
#define LEDTIMERPERIOD	100L
#define NUMLEDS			10
#define LEDSPACING		2

VOID
DrawStatus (
		HWND				hwnd,
		INT					*iStatusValue,
		INT					*iStatusDirection) 
{
	HBRUSH			hBrushLit, hBrushUnlit, hBrushOld;
	HPEN			hPen, hPenOld;
	INT				i;
	INT				itop, ibot, ileft, iright, iwidth;
	PAINTSTRUCT		ps;
	HDC				hdc;
	RECT			rc;

	if (*iStatusValue < -1) return;

	hdc = BeginPaint (hwnd, &ps);

	// draw 3D shadow
	GetClientRect (hwnd, &rc);
	itop = rc.top+1;
	ibot = rc.bottom-2;

	iwidth = (rc.right-rc.left) / NUMLEDS;
	iwidth -= LEDSPACING;

	ileft = rc.left + 4;
	for (i=0; i<NUMLEDS; i++) {
		iright = ileft + iwidth;

		MoveToEx (hdc, ileft, ibot, NULL);
		LineTo (hdc, iright, ibot);
		LineTo (hdc, iright, itop);

		ileft += iwidth + LEDSPACING;
	}

	hPen = CreatePen (PS_SOLID, 0, RGB (128, 128, 128));
	hPenOld = SelectObject (hdc, hPen);
	hBrushLit = CreateSolidBrush (RGB (0, 255, 0));
	hBrushUnlit = CreateSolidBrush (RGB (0, 128, 0));

	ileft = rc.left + 4;

	// draw "Knight Rider" LEDs
	if (*iStatusDirection) {
		hBrushOld = SelectObject (hdc, hBrushUnlit);
		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i == *iStatusValue) {
				SelectObject (hdc, hBrushLit);
				Rectangle (hdc, ileft, itop, iright, ibot);
				SelectObject (hdc, hBrushUnlit);
			}
			else  {
				Rectangle (hdc, ileft, itop, iright, ibot);
			}
	
			ileft += iwidth + LEDSPACING;
		}
	}

	// draw "progress bar" LEDs
	else { 
		if (*iStatusValue >= 0) 
			hBrushOld = SelectObject (hdc, hBrushLit);
		else
			hBrushOld = SelectObject (hdc, hBrushUnlit);

		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i > *iStatusValue) {
				SelectObject (hdc, hBrushUnlit);
			}
			Rectangle (hdc, ileft, itop, iright, ibot);
	
			ileft += iwidth + LEDSPACING;
		}
	}

	SelectObject (hdc, hBrushOld);
	SelectObject (hdc, hPenOld);
	DeleteObject (hPen);
	DeleteObject (hBrushLit);
	DeleteObject (hBrushUnlit);

	EndPaint (hwnd, &ps);
}

void PGPsdkUICreateImageList(HIMAGELIST *hIml)
{
	int iNumBits;
	HDC hDC;
	HBITMAP hBmp;

	// ImageList Init

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		*hIml =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
							NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (*hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		*hIml =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
							NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (*hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
}

	PGPError
KeyServerCallbackProc(
	PGPContextRef 	context,
	struct PGPEvent *event,
	PGPUserValue 	userValue)
{
	KEYGBL		*pkg;
	PGPError	err 		= kPGPError_NoErr;

	pkg=(KEYGBL *)userValue;

	if(pkg->bCancel)
	{
		err = kPGPError_UserAbort;
	}

	switch( event->type )
	{
		case kPGPEvent_KeyServerIdleEvent:
		{
			break;
		}
			
		case kPGPEvent_KeyServerEvent:
		{
			if( PGPKeyServerRefIsValid(
						event->data.keyServerData.keyServerRef ) &&
				event->data.keyServerData.keyServerRef != pkg->mLastKeyServer )
			{
				const char	*hostName 		= NULL;
				PGPBoolean	freeHostName 	= FALSE;
				PGPUInt32	index;
				
				pkg->mLastKeyServer = event->data.keyServerData.keyServerRef;

				// Set the server name:
				
				for( index = 0; index < pkg->options->mServerCount; index++ )
				{
					if( pkg->options->mServerList[index].server == 
							event->data.keyServerData.keyServerRef )
					{
						hostName = pkg->options->mServerList[index].serverName;
						break;
					}
				}
				
				if( IsNull( hostName ) )
				{
					if( IsntPGPError( PGPGetKeyServerHostName(
								pkg->mLastKeyServer, (char **) &hostName ) ) )
					{
						freeHostName = TRUE;
					}
				}
				
				if( IsntNull( hostName ) )
				{
					strcpy( pkg->serverStr, hostName );
					
					if( freeHostName )
						PGPFreeData( (char *) hostName );
				}
			}
		
			if( event->data.keyServerData.state != pkg->mLastKeyServerState )
			{
				DWORD	newStateStrIndex = 0;
				char	szStatusStr[256];
			
				switch( event->data.keyServerData.state )
				{
					case kPGPKeyServerState_Opening:
						newStateStrIndex = IDS_KSCONNECTING;
						SetTimer (pkg->hwndServerProgress, 
							LEDTIMER, LEDTIMERPERIOD, NULL);
						pkg->bSecureConnection=FALSE;
						InvalidateRect(pkg->hwndServerProgress,NULL,TRUE);
						UpdateWindow(pkg->hwndServerProgress);
						break;
						
					case kPGPKeyServerState_Querying:
						newStateStrIndex = IDS_KSSEARCHING;
						break;

					case kPGPKeyServerState_ProcessingResults:
						newStateStrIndex = IDS_KSPROCESSING;
						break;

					case kPGPKeyServerState_Uploading:
						newStateStrIndex = IDS_KSSENDING;
						break;

					case kPGPKeyServerState_Deleting:
						newStateStrIndex = IDS_KSDELETING;
						break;

					case kPGPKeyServerState_Disabling:
						newStateStrIndex = IDS_KSDISABLING;
						break;

					case kPGPKeyServerState_Closing:
						newStateStrIndex = IDS_KSCLOSINGSEND;
						break;
				}

				LoadString(g_hInst,
					newStateStrIndex, szStatusStr, sizeof(szStatusStr));
					
				if((newStateStrIndex == IDS_KSCONNECTING)||
					(newStateStrIndex == IDS_KSSEARCHING)||
					(newStateStrIndex == IDS_KSPROCESSING)||
					(newStateStrIndex == IDS_KSSENDING))
					strcat(szStatusStr,pkg->serverStr);

				SetWindowText(GetDlgItem(pkg->hwndServerProgress,IDC_PROGRESSTEXT),
					szStatusStr);

				pkg->mLastKeyServerState = (PGPKeyServerState)
											event->data.keyServerData.state;
			}
		
			break;
		}

		case kPGPEvent_KeyServerTLSEvent:
		{
			PGPEventKeyServerTLSData data = event->data.keyServerTLSData;
	
			if(data.state==kPGPKeyServerState_TLSConnectionSecured)
			{
				pkg->bSecureConnection=TRUE;

				InvalidateRect(pkg->hwndServerProgress,NULL,TRUE);
				UpdateWindow(pkg->hwndServerProgress);
			}
			
			break;
		}
		
	}
	
	return( err );
}


DWORD WINAPI
KeyServerThread (LPVOID lpvoid) 
{
	KEYGBL		*pkg;
	PGPContextRef context;
	PGPKeySetRef mNewKeys,mFailedKeys;
	PGPKeyServerSpec *serverList;
	PGPUInt32 memamt;


	mNewKeys=mFailedKeys=NULL;

	pkg=(KEYGBL *)lpvoid;
	context=pkg->context;

	if(pkg->bSearch)
	{
		CPGPSearchKeyServerDialogOptions *options;

		options=(CPGPSearchKeyServerDialogOptions *)pkg->options;

		// Make copy of servers in case we're orphaned
		memamt=(options->mServerCount) * sizeof(PGPKeyServerSpec);

		serverList = (PGPKeyServerSpec*)PGPNewData(
			PGPGetContextMemoryMgr (context),
			memamt,kPGPMemoryMgrFlags_Clear);

		memcpy(serverList,options->mServerList,memamt);

		pkg->err = PGPSearchKeyServerDialogCommon( context,
			serverList, options->mServerCount,
			options->mTLSContext, options->mFilter,
			KeyServerCallbackProc, (PGPUserValue) pkg,
			options->mSearchAllServers, &mNewKeys );

		PGPFreeData(serverList);

		if(!pkg->bCancel)
			*(options->mNewKeys)=mNewKeys;
	}
	else
	{
		CPGPSendToKeyServerDialogOptions *options;

		options=(CPGPSendToKeyServerDialogOptions *)pkg->options;

		// Make copy of servers in case we're orphaned
		memamt=(options->mServerCount) * sizeof(PGPKeyServerSpec);

		serverList = (PGPKeyServerSpec*)PGPNewData(
			PGPGetContextMemoryMgr (context),
			memamt,kPGPMemoryMgrFlags_Clear);

		memcpy(serverList,options->mServerList,memamt);

		pkg->err = PGPSendToKeyServerDialogCommon( context,
			serverList, options->mTLSContext,
			options->mKeysToSend,
			KeyServerCallbackProc, (PGPUserValue) pkg,
			&mFailedKeys );

		PGPFreeData(serverList);

		if(!pkg->bCancel)
			*(options->mFailedKeys)=mFailedKeys;
	}

	if(pkg->bCancel)
	{
		// If canceled, we have to free it
		free(pkg);
	}
	else
	{
		// Kill the window and let the main thread free it
		ReleaseSemaphore(pkg->hSemaphore, 1, NULL);
		SendMessage(pkg->hwndServerProgress,WM_DESTROY,0,0);
	}

	return 0;
}

//	______________________________________________________
//
//  Server progress dialog procedure

BOOL CALLBACK
ServerProgressDlgProc (
		 HWND	hDlg, 
		 UINT	uMsg, 
		 WPARAM wParam,
		 LPARAM lParam) 
{
	KEYGBL		*pkg;

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			pkg=(KEYGBL *)lParam;

			PGPsdkUICreateImageList(&(pkg->hIml));
			pkg->hwndServerProgress=hDlg;

			pkg->iStatusValue = 0;
			pkg->iStatusDirection = 1;

			pkg->hThread=CreateThread (NULL, 0, KeyServerThread, 
				(LPVOID)pkg, 0, &(pkg->dwThreadID));

			return TRUE;
		}

		case WM_PAINT :
		{
			PAINTSTRUCT		ps;
			HDC				hdc;
			int				LockIcon;
			int				rst;

			pkg=(KEYGBL *)GetWindowLong (hDlg, GWL_USERDATA);

			// Draw lock for TLS connection
			if(pkg->bSecureConnection)
				LockIcon=IDX_CLOSEDLOCK;
			else
				LockIcon=IDX_OPENLOCK;

			hdc = BeginPaint (hDlg, &ps);

			rst=ImageList_Draw(pkg->hIml,LockIcon,
				hdc,
				8,80,
				ILD_TRANSPARENT);

			EndPaint (hDlg, &ps);

			// Draw star trek progress lights
			DrawStatus (GetDlgItem (hDlg, IDC_PROGRESS), 
				&(pkg->iStatusValue),
				&(pkg->iStatusDirection));
			break;
		}

		case WM_TIMER :
		{
			pkg=(KEYGBL *)GetWindowLong (hDlg, GWL_USERDATA);

			pkg->iStatusValue += pkg->iStatusDirection;
			if (pkg->iStatusValue <= 0) 
			{
				pkg->iStatusValue = 0;
				pkg->iStatusDirection = 1;
			}
			else if (pkg->iStatusValue >= NUMLEDS-1) 
			{
				pkg->iStatusValue = NUMLEDS-1;
				pkg->iStatusDirection = -1;
			}
			InvalidateRect (hDlg, NULL, FALSE);
			break;
		}


		case WM_NOTIFY :
		{
			break;
		}

		case WM_DESTROY:
		case WM_QUIT:
		case WM_CLOSE:
		{
			pkg=(KEYGBL *)GetWindowLong (hDlg, GWL_USERDATA);

			DeleteObject(pkg->hIml);
			EndDialog(hDlg, 0);
			return TRUE;
		}

		case WM_COMMAND :
		{
			switch(LOWORD (wParam)) 
			{
				case IDCANCEL :
				{
					pkg=(KEYGBL *)GetWindowLong (hDlg, GWL_USERDATA);

					// Do the dirty deed
//					pkg->err=kPGPError_UserAbort;
//					TerminateThread(pkg->hThread,0);

					pkg->bCancel=TRUE;

					if (pkg->mLastKeyServer) 
					{
						PGPError err;

						err = PGPCancelKeyServerCall (pkg->mLastKeyServer);
						PGPsdkUIErrorBox (hDlg, err);
					}

					ReleaseSemaphore(pkg->hSemaphore, 1, NULL);
					EndDialog(hDlg, 0);
					break;
				}

			}
			return TRUE;
		}
	}

	return FALSE;
}

	PGPError
pgpSearchKeyServerDialogPlatform(
	PGPContextRef						context,
	CPGPSearchKeyServerDialogOptions 	*options)
{
	PGPError	err;
	DWORD		dialogID;
	KEYGBL		*pkg;
	
	PGPValidatePtr( options );
		
	err=kPGPError_NoErr;

	pkg=(KEYGBL *)malloc(sizeof(KEYGBL));
	if(!pkg)
	{
		err=kPGPError_OutOfMemory;
	}
	else
	{
		memset(pkg,0x00,sizeof(KEYGBL));

		pkg->context=context;
		pkg->options=options;
		pkg->bSearch=TRUE;

		pkg->hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

		if( options->mKeyDescription[0] == 0 )
		{
			dialogID = IDD_SERVERPROGRESS;
		}
		else
		{
			dialogID = IDD_SERVERPROGRESS;
		}
		
		DialogBoxParam (g_hInst, 
			MAKEINTRESOURCE(dialogID),
			options->mHwndParent, 
			(DLGPROC)ServerProgressDlgProc, 
			(LPARAM)pkg);

		WaitForSingleObject (pkg->hSemaphore, INFINITE);
		CloseHandle (pkg->hSemaphore);
		CloseHandle (pkg->hThread);

		if(pkg->bCancel)
		{
			// Let thread free its own memory after its done
			err=kPGPError_UserAbort;
		}
		else
		{
			err=pkg->err;
			free(pkg);
		}
	}

	return( err );
}

	PGPError
pgpSendToKeyServerDialogPlatform(
	PGPContextRef						context,
	CPGPSendToKeyServerDialogOptions 	*options)
{
	PGPError	err;
	DWORD		dialogID;
	KEYGBL		*pkg;

	PGPValidatePtr( options );
		
	err=kPGPError_NoErr;
	dialogID = IDD_SERVERPROGRESS;
	
	pkg=(KEYGBL *)malloc(sizeof(KEYGBL));
	if(!pkg)
	{
		err=kPGPError_OutOfMemory;
	}
	else
	{
		memset(pkg,0x00,sizeof(KEYGBL));

		pkg->context=context;
		pkg->options=options;
		pkg->bSearch=FALSE;

		pkg->hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

		DialogBoxParam (g_hInst, 
			MAKEINTRESOURCE(dialogID),
			options->mHwndParent, 
			(DLGPROC)ServerProgressDlgProc, 
			(LPARAM)pkg);

		WaitForSingleObject (pkg->hSemaphore, INFINITE);
		CloseHandle (pkg->hSemaphore);
		CloseHandle (pkg->hThread);

		if(pkg->bCancel)
		{
			// Let thread free its own memory after its done
			err=kPGPError_UserAbort;
		}
		else
		{
			err=pkg->err;
			free(pkg);
		}
	}
	
	return( err );
}
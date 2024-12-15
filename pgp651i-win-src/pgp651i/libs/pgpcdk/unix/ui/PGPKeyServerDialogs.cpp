/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPKeyServerDialogs.cpp,v 1.2 1999/03/10 02:52:17 heller Exp $
____________________________________________________________________________*/
#include <stdlib.h>
#include <string.h>

#include "pgpDialogs.h"
#include "pgpKeyServerDialogCommon.h"
#include "pgpKeyServer.h"

typedef struct _KEYGBL 
{
	PGPUInt32 mLastKeyServerState;
	PGPKeyServerRef mLastKeyServer;
	PGPInt32 iStatusValue;
	PGPInt32 iStatusDirection;
	PGPContextRef context;
	CPGPKeyServerDialogOptions *options;
	PGPError err;
	PGPBoolean bSearch;
	PGPBoolean bCancel;
	PGPInt32 dwThreadID;
	char serverStr[255];
} KEYGBL;


//	____________________________________
//
//  Draw the "LED" progress indicator

#define LEDTIMER		111L
#define LEDTIMERPERIOD	100L
#define NUMLEDS			10
#define LEDSPACING		2

	PGPError
KeyServerCallbackProc(
	PGPContextRef 	context,
	struct PGPEvent *event,
	PGPUserValue 	userValue)
{
	KEYGBL		*pkg;
	PGPError	err 		= kPGPError_NoErr;

	(void) context;
	(void) event;

	pkg=(KEYGBL *)userValue;

	if(pkg->bCancel)
	{
		err = kPGPError_UserAbort;
	}
	
#if 0
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
				
				if( hostName == NULL )
				{
					if( IsntPGPError( PGPGetKeyServerHostName(
								pkg->mLastKeyServer, (char **) &hostName ) ) )
					{
						freeHostName = TRUE;
					}
				}
				
				if( hostName != NULL )
				{
					strcpy( pkg->serverStr, hostName );
					
					if( freeHostName )
						PGPFreeData( (char *) hostName );
				}
			}
		
			if( event->data.keyServerData.state != pkg->mLastKeyServerState )
			{
				switch( event->data.keyServerData.state )
				{
					case kPGPKeyServerState_Opening:
						break;
						
					case kPGPKeyServerState_Querying:
						break;

					case kPGPKeyServerState_ProcessingResults:
						break;

					case kPGPKeyServerState_Uploading:
						break;

					case kPGPKeyServerState_Deleting:
						break;

					case kPGPKeyServerState_Disabling:
						break;

					case kPGPKeyServerState_Closing:
						break;
				}

				pkg->mLastKeyServerState = (PGPKeyServerState)
											event->data.keyServerData.state;
			}
		
			break;
		}
	}
#endif
	
	return( err );
}

#if 0
PGPInt32 WINAPI
KeyServerThread (LPVOID lpvoid) 
{
	KEYGBL		*pkg;
	PGPContextRef context;

	pkg=(KEYGBL *)lpvoid;
	context=pkg->context;

	if(pkg->bSearch)
	{
		CPGPSearchKeyServerDialogOptions *options;

		options=(CPGPSearchKeyServerDialogOptions *)pkg->options;

		pkg->err = PGPSearchKeyServerDialogCommon( context,
			options->mServerList, options->mServerCount,
			options->mTLSContext, options->mFilter,
			KeyServerCallbackProc, (PGPUserValue) pkg,
			options->mSearchAllServers, options->mNewKeys );
	}
	else
	{
		CPGPSendToKeyServerDialogOptions *options;

		options=(CPGPSendToKeyServerDialogOptions *)pkg->options;

		pkg->err = PGPSendToKeyServerDialogCommon( context,
			options->mServerList, options->mTLSContext,
			options->mKeysToSend,
			KeyServerCallbackProc, (PGPUserValue) pkg,
			options->mFailedKeys );
	}

	SendMessage(pkg->hwndServerProgress,WM_DESTROY,0,0);

	ReleaseSemaphore(pkg->hSemaphore, 1, NULL);

	return 0;
}

//	______________________________________________________
//
//  Server progress dialog procedure

BOOL CALLBACK
ServerProgressDlgProc (
		 HWND	hDlg, 
		 PGPUInt32	uMsg, 
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

			pkg->hwndServerProgress=hDlg;

			pkg->iStatusValue = 0;
			pkg->iStatusDirection = 1;

			CreateThread (NULL, 0, KeyServerThread, 
				(LPVOID)pkg, 0, &(pkg->dwThreadID));

			return TRUE;
		}

		case WM_PAINT :
		{
			pkg=(KEYGBL *)GetWindowLong (hDlg, GWL_USERDATA);

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

		case WM_QUIT:
		case WM_CLOSE:
		case WM_DESTROY:
		{
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

					pkg->bCancel=TRUE;
					if (pkg->mLastKeyServer) 
					{
						PGPError err;

						err = PGPCancelKeyServerCall (pkg->mLastKeyServer);
						PGPsdkUIErrorBox (hDlg, err);
					}

					break;
				}

			}
			return TRUE;
		}
	}

	return FALSE;
}
#endif

	PGPError
pgpSearchKeyServerDialogPlatform(
	PGPContextRef						context,
	CPGPSearchKeyServerDialogOptions 	*options)
{
	PGPError	err;
//	PGPInt32		dialogID;
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

//		pkg->hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

		if( options->mKeyDescription[0] == 0 )
		{
//			dialogID = IDD_SERVERPROGRESS;
		}
		else
		{
//			dialogID = IDD_SERVERPROGRESS;
		}
		
//		DialogBoxParam (g_hInst, 
	//		MAKEINTRESOURCE(dialogID),
	//		options->mHwndParent, 
	//		(DLGPROC)ServerProgressDlgProc, 
	//		(LPARAM)pkg);

	//	WaitForSingleObject (pkg->hSemaphore, INFINITE);
	//	CloseHandle (pkg->hSemaphore);

		err=pkg->err;

		free(pkg);
	}

	return( err );
}

	PGPError
pgpSendToKeyServerDialogPlatform(
	PGPContextRef						context,
	CPGPSendToKeyServerDialogOptions 	*options)
{
	PGPError	err;
//	PGPInt32		dialogID;
	KEYGBL		*pkg;

	PGPValidatePtr( options );
		
	err=kPGPError_NoErr;
//	dialogID = IDD_SERVERPROGRESS;
	
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

//		DialogBoxParam (g_hInst, 
	//		MAKEINTRESOURCE(dialogID),
	//		options->mHwndParent, 
	//		(DLGPROC)ServerProgressDlgProc, 
	//		(LPARAM)pkg);
	
		err=pkg->err;

		free(pkg);
	}
	
	return( err );
}
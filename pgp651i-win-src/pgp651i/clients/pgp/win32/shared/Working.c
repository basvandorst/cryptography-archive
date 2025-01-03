/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Working.c,v 1.6 1999/04/01 01:23:42 dgal Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <process.h>
#include <stdio.h>
#include <commctrl.h>


// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpWerr.h"
#include "pgpUtilities.h"
#include "pgpVersionHeader.h"

// Project Headers
#include "Working.h"
#include "WorkingResource.h"

typedef struct _wdplt
{
	HWND hWorkingDlg;
	HINSTANCE hMod;
	HINSTANCE hInst;
	HWND hParent;
	HANDLE hSemaphore;
	char *Action;
	char *FileName;
	BOOL CancelPending;
	float LastUpdate;
}WDPLT;


#define PGPM_PROGRESS_UPDATE  WM_USER + 100
void WorkingDlgProcLaunchThread(void *Args);


BOOL CALLBACK WorkingDlgProc (HWND hDlg, 
							  UINT uMsg, 
							  WPARAM wParam,
							  LPARAM lParam)
{
	BOOL ReturnCode = FALSE;
	WDPLT *pThreadArgs;

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETRANGE, 0, 
				MAKELPARAM(0, 100));
			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, 0, 0);
			if(lParam)
			{
				char Title[256];

				SetWindowLong(hDlg, GWL_USERDATA, lParam);
				pThreadArgs = (WDPLT *) lParam;
				pThreadArgs->CancelPending = FALSE;
				if(pThreadArgs->Action)
				{
					pThreadArgs->LastUpdate = (float) 0.0;
					if(pThreadArgs->FileName)
						sprintf(Title, 
								"%s %s", 
								pThreadArgs->Action, 
								pThreadArgs->FileName);
					else
						strcpy(Title, pThreadArgs->Action);
								
					SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM) Title);
				}
			}
			SetFocus(hDlg);
			SetActiveWindow(hDlg);
			break;
		}

		case PGPM_PROGRESS_UPDATE:
		{
			char szTransfer[256];
			int PercentComplete = 0;
			float ftot, fsofar;

			if((pThreadArgs = (WDPLT *) GetWindowLong(hDlg, GWL_USERDATA)))
			{
				if(!pThreadArgs->CancelPending)
				{
					if ((wParam != (WPARAM) lParam) &&
						(wParam > 0) &&
						(lParam > 0))
					{
						fsofar = (float) wParam;
						ftot = (float) lParam;
						PercentComplete = (int) ((fsofar / ftot) * 100.0);

						fsofar /= 1024;
						ftot /= 1024;

						if(!pThreadArgs->LastUpdate || 
							pThreadArgs->LastUpdate + 10 < fsofar)
						{
							pThreadArgs->LastUpdate = fsofar;
						}
						SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, 
							PercentComplete, 0);
					}
					else if (wParam == (WPARAM) lParam)
					{
						LoadString(pThreadArgs->hInst, IDS_WORKINGTRANSFER,
							szTransfer, sizeof(szTransfer));
						SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, 
							100, 0);
						if(pThreadArgs && pThreadArgs->FileName)
							SetDlgItemText(hDlg, IDC_PROGRESS_TEXT, 
							szTransfer);
					}
				}
				else
				{
					EndDialog(hDlg, 0);
				}
				ReturnCode = TRUE;
			}
			break;
		}

		case WM_CLOSE:
/*
		case WM_QUIT:
*/
		case WM_DESTROY:
		{
			if((pThreadArgs = (WDPLT *) GetWindowLong(hDlg, GWL_USERDATA)))
			{
				if(pThreadArgs->Action)
					free(pThreadArgs->Action);
				
				if(pThreadArgs->FileName)
					free(pThreadArgs->FileName);

				free(pThreadArgs);
				pThreadArgs = NULL;
			}
				
			EndDialog(hDlg, 0);
			ReturnCode = TRUE;
			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD (wParam)) 
			{
				case IDCANCEL:
				{
					if((pThreadArgs = (WDPLT *) GetWindowLong(hDlg, 
						GWL_USERDATA)))
						pThreadArgs->CancelPending = TRUE;
					else
						EndDialog (hDlg, 0);

					ReturnCode = TRUE;
					break;
				}
			}
			ReturnCode = TRUE;
			break;
		}
	}
	return (ReturnCode);
}


//----------------------------------------------------|
// Callback routine to be called by Encryption/Decryption/Signing 

int WorkingCallback (void* arg, INT SoFar, INT Total) 
{
	int CancelHit = FALSE;
	int ReturnCode = 0;
	WDPLT *pThreadArgs;

	if(arg)
	{
		SendMessage((HWND) arg, PGPM_PROGRESS_UPDATE, SoFar, Total);

		if((pThreadArgs = (WDPLT *) GetWindowLong((HWND) arg, GWL_USERDATA)))
			CancelHit = pThreadArgs->CancelPending;
	}

	if(CancelHit)
	{
		ReturnCode = kPGPError_UserAbort;
	}

	return(ReturnCode);
}

HWND WorkingDlgProcThread(HMODULE hMod, 
						  HINSTANCE hInst,
						  HWND hParent,
						  char *Action,
						  char *FileName)
{
	WDPLT *pThreadArgs;

	pThreadArgs = (WDPLT *) calloc(sizeof(WDPLT), 1);

	pThreadArgs->hWorkingDlg = NULL;

	if((pThreadArgs->hSemaphore = CreateSemaphore(NULL, 0, 1, NULL)))
	{
		pThreadArgs->hMod = hMod;
		pThreadArgs->hInst = hInst;
		pThreadArgs->hParent = hParent;

		if(Action)
		{
			if((pThreadArgs->Action = (char *) malloc(sizeof(char) * 
				(strlen(Action) + 1))))
				strcpy(pThreadArgs->Action, Action);
		}
		else
			pThreadArgs->Action = NULL;

		if(FileName)
		{
			if((pThreadArgs->FileName = (char *) malloc(sizeof(char) * 
				(strlen(FileName) + 1))))
				strcpy(pThreadArgs->FileName, FileName);
		}
		else
		{
			pThreadArgs->FileName = NULL;
		}

/*
		_beginthread(WorkingDlgProcLaunchThread, 0x8000, (void *)&ThreadArgs);

		WaitForSingleObject(pThreadArgs->hSemaphore, INFINITE);
*/
	pThreadArgs->hWorkingDlg = CreateDialogParam(pThreadArgs->hMod, 
								MAKEINTRESOURCE(IDD_WORKING), 
								pThreadArgs->hParent,
								(DLGPROC) WorkingDlgProc,
								(LPARAM) pThreadArgs);
	}

	return(pThreadArgs->hWorkingDlg);
}

void WorkingDlgProcLaunchThread(void *Args)
{
	WDPLT tArgs;
	MSG msg;

	tArgs = *((WDPLT *) Args);

	tArgs.hWorkingDlg = CreateDialogParam(	tArgs.hMod, 
											MAKEINTRESOURCE(IDD_WORKING), 
											tArgs.hParent,
											(DLGPROC) WorkingDlgProc,
											(LPARAM) &tArgs);

	((WDPLT *) Args)->hWorkingDlg = tArgs.hWorkingDlg;

	ReleaseSemaphore(tArgs.hSemaphore, 1, NULL);

	while (GetMessage(&msg, NULL, (UINT)NULL, (UINT)NULL))
    {
		if (msg.hwnd == NULL)	// if thread message
		{
			// send message to main window
			PostMessage(tArgs.hWorkingDlg, msg.message, 
				msg.wParam, msg.lParam);
		}
		else
		{
			if (IsDialogMessage(tArgs.hWorkingDlg, &msg))
			{
				;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
    }

	if(tArgs.Action)
		free(tArgs.Action);

	if(tArgs.FileName)
		free(tArgs.FileName);

	_endthread();
}



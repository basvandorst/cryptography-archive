/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: working.c,v 1.24.2.1.2.1 1998/11/12 03:13:40 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

int WorkingCallback (void* arg, unsigned long SoFar, unsigned long Total) 
{
	WORKSTRUCT *ws;
	HWND hwnd;
	MYSTATE *myState;

	hwnd=(HWND)arg;
   
	myState = (MYSTATE *) GetWindowLong(hwnd, GWL_USERDATA);
	ws = &(myState->ws);

	ws->newsofar=SoFar;
	ws->total=Total;

	SendMessage(hwnd, PGPM_PROGRESS_UPDATE,
		(WPARAM)PGPSC_WORK_PROGRESS,0);

	if(ws->CancelPending)
	{
		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI
DoWorkThread (LPVOID lpvoid) 
{
	MYSTATE *myState;
	PGPError err;
	DWORD nOleErr;

	myState=(MYSTATE *)lpvoid;

	nOleErr = OleInitialize(NULL);

	if ((nOleErr == S_OK) || (nOleErr == S_FALSE))
	{
		switch(myState->Operation)
		{
			case MS_ENCRYPTFILELIST:
				err=EncryptFileListStub (myState);
				break;

			case MS_ENCRYPTCLIPBOARD:
				err=EncryptClipboardStub (myState);
				break;

			case MS_DECRYPTFILELIST:
				err=DecryptFileListStub (myState);
				break;

			case MS_DECRYPTCLIPBOARD:
				err=DecryptClipboardStub (myState);
				break;

			case MS_ADDKEYFILELIST:
				err=AddKeyFileListStub (myState);
				break;

			case MS_ADDKEYCLIPBOARD:
				err=AddKeyClipboardStub (myState);
				break;

			case MS_WIPEFILELIST:
				err=WipeFileListStub (myState);
				break;
		}
		OleUninitialize();
	}

	myState->err=err;

	SendMessage(myState->hwndWorking,WM_DESTROY,0,0);
	ReleaseSemaphore(myState->hSemaphore, 1, NULL);
	return 0;
}


BOOL CALLBACK WorkingDlgProc (HWND hDlg, 
							  UINT uMsg, 
							  WPARAM wParam,
							  LPARAM lParam)
{
	MYSTATE *myState;
	WORKSTRUCT *ws;

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{    
			RECT rc;

			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			myState=(MYSTATE *)lParam;
			myState->hwndWorking=hDlg;

			CreateThread (NULL, 0, DoWorkThread, 
				(LPVOID)myState, 0, &(myState->dwThreadID));

			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETRANGE, 0, 
				MAKELPARAM(0, 100));
			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, 0, 0);

 			GetWindowRect (hDlg, &rc);
			SetWindowPos (hDlg, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
  
			SetForegroundWindow(hDlg);
			return TRUE;
		}

		case WM_CTLCOLOR:
		{          
 			if(wParam!=0)
			{ 
				DWORD color;

				color=GetSysColor(COLOR_BTNFACE);
				
			  	SetBkColor ((HDC)wParam, color);
			  	SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
			  	return (BOOL)CreateSolidBrush (color);
			} 	 
			break;
		}

		case PGPM_PROGRESS_UPDATE:
		{
			int done, total;

			myState = (MYSTATE *) GetWindowLong(hDlg, GWL_USERDATA);
			ws = &(myState->ws);

			if(wParam==PGPSC_WORK_NEWFILE)
			{
				SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)ws->TitleText);
				SetDlgItemText(hDlg, IDC_PROGRESS_TEXT,ws->StartText);

				ws->sofar=0;
				ws->newsofar=0;
				ws->total=0;
				ws->curpos=0;

				SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, ws->curpos, 0);

				ShowWindow(hDlg,SW_SHOW);
				break;
			}

			if(ws->sofar != ws->total)
			{
				if(ws->newsofar!=ws->sofar)
				{
					char completed[500];
					char StrRes[500];

					ws->sofar=ws->newsofar;

					done=ws->sofar;
					total=ws->total;

					ws->curpos = (long) (((float)done / (float)total) *
					(float)100.0);

					done /= 1024;
					total /= 1024;

					if(total==0)
						break; // Don't want 0 of 0K etc.

					LoadString (g_hinst, IDS_WORKPERCENT, StrRes, sizeof(StrRes));

					sprintf(completed, StrRes, done, total);
					SetDlgItemText(hDlg, IDC_PROGRESS_TEXT, completed);

					SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, ws->curpos, 0);
				}
				break;
			}

			SetDlgItemText(hDlg, IDC_PROGRESS_TEXT,ws->EndText);

			ws->curpos=100; 
			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, ws->curpos, 0);
			break;
		}

//		case WM_SETCURSOR:
//		{
//			SetCursor(LoadCursor(NULL,IDC_WAIT));
//			return TRUE;
//		}

		case WM_CLOSE:
		case WM_QUIT:
		case WM_DESTROY:
		{
			EndDialog(hDlg, FALSE);
			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD (wParam)) 
			{
				case IDCANCEL:
				{
					myState = (MYSTATE *) GetWindowLong(hDlg, GWL_USERDATA);
					ws = &(myState->ws);
					ws->CancelPending=TRUE;
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

PGPError DoWork(HWND hwndParent,MYSTATE *myState)
{
	// Just to be sure, a semaphore reinforces the barrier
	// made by the DialogBox.
	myState->hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	DialogBoxParam (g_hinst, 
		MAKEINTRESOURCE(IDD_NEWWORKING),
		hwndParent, 
		(DLGPROC)WorkingDlgProc, 
		(LPARAM)myState);

	WaitForSingleObject (myState->hSemaphore, INFINITE);
	CloseHandle (myState->hSemaphore);

	return myState->err;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

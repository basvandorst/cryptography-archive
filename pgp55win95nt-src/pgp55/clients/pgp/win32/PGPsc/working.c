/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: working.c,v 1.21 1997/11/03 19:39:08 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

extern WNDPROC origProgValProc;

BOOL CALLBACK WorkingDlgProc (HWND hDlg, 
							  UINT uMsg, 
							  WPARAM wParam,
							  LPARAM lParam)
{
	WORKSTRUCT *ws;

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{    
			RECT rc;

			SetWindowLong(hDlg, DWL_USER, lParam);
			ws = (WORKSTRUCT *) lParam;
			ws->maxpos=100;

			origProgValProc=SubclassWindow(GetDlgItem(hDlg,IDC_PROGRESS),
				MyProgressValidityWndProc); 
	
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

			ws = (WORKSTRUCT *) GetWindowLong(hDlg, DWL_USER);

			if(wParam==PGPSC_WORK_NEWFILE)
			{
				SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)ws->TitleText);
				SetDlgItemText(hDlg, IDC_PROGRESS_TEXT,ws->StartText);

				ws->sofar=0;
				ws->newsofar=0;
				ws->total=0;
				ws->curpos=0;

				InvalidateRect(GetDlgItem(hDlg,IDC_PROGRESS),NULL,TRUE);

				ShowWindow(hDlg,SW_SHOW);
				break;
			}

			if(ws->sofar != ws->total)
			{
				if(ws->newsofar!=ws->sofar)
				{
					char completed[500];

					ws->sofar=ws->newsofar;

					done=ws->sofar;
					total=ws->total;

					ws->curpos = (long) (((float)done / (float)total) *
					(float)100.0);

					done /= 1024;
					total /= 1024;

					if(total==0)
						break; // Don't want 0 of 0K etc.

					sprintf(completed, "%dk of %dk", done, total);
					SetDlgItemText(hDlg, IDC_PROGRESS_TEXT, completed);

					UpdateProgValBar(GetDlgItem(hDlg,IDC_PROGRESS));
				}
				break;
			}

			SetDlgItemText(hDlg, IDC_PROGRESS_TEXT,ws->EndText);

			ws->curpos=100; 
			UpdateProgValBar(GetDlgItem(hDlg,IDC_PROGRESS));
			break;
		}

		case WM_SETCURSOR:
		{
			SetCursor(LoadCursor(NULL,IDC_WAIT));
			return TRUE;
		}

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
					ws = (WORKSTRUCT *) GetWindowLong(hDlg, DWL_USER);
					ws->CancelPending=TRUE;
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}


//----------------------------------------------------|
// Callback routine to be called by Encryption/Decryption/Signing 

void ProcessWorkingDlg(HWND hwnd)
{
	MSG msg;

	while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		if (!IsDialogMessage(hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

int WorkingCallback (void* arg, unsigned long SoFar, unsigned long Total) 
{
	WORKSTRUCT *ws;
	HWND hwnd;

	hwnd=(HWND)arg;
   
	ws=(WORKSTRUCT *)GetWindowLong(hwnd, DWL_USER);

	ws->newsofar=SoFar;
	ws->total=Total;

	SendMessage(hwnd, PGPM_PROGRESS_UPDATE,
		(WPARAM)PGPSC_WORK_PROGRESS,0);

	ProcessWorkingDlg(hwnd);

	if(ws->CancelPending)
	{
		return TRUE;
	}

	return FALSE;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

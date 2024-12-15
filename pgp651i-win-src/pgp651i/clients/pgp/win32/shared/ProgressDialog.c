/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: ProgressDialog.c,v 1.7 1999/03/10 06:47:59 wjb Exp $
____________________________________________________________________________*/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include "pgpErrors.h"
#include "ProgressDialog.h"
#include "resource.h"

#ifndef IDC_PRGAVI
#define IDC_PRGAVI 100 // dummy if no resource is spec'ed
#endif

extern HINSTANCE g_hinst;

#define UPDATETIMERID 100
#define UPDATETIME 75 // milliseconds between updates

DWORD WINAPI
PrgDlgThreadRoutine (LPVOID lpvoid) 
{
	HPRGDLG hPrgDlg;

	hPrgDlg=(HPRGDLG)lpvoid;

	hPrgDlg->err=(hPrgDlg->UserFunction)(hPrgDlg,hPrgDlg->pUserValue);

	SendMessage(hPrgDlg->hwnd,WM_DESTROY,0,0);
	ReleaseSemaphore(hPrgDlg->hSemaphore, 1, NULL);
	return 0;
}

BOOL CALLBACK PrgDlgProc (HWND hDlg, 
							  UINT uMsg, 
							  WPARAM wParam,
							  LPARAM lParam)
{
	HPRGDLG hPrgDlg;

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{    
			RECT rc;

			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			hPrgDlg=(HPRGDLG)lParam;
			hPrgDlg->hwnd=hDlg;

			// Spawn work thread
			CreateThread (NULL, 0, PrgDlgThreadRoutine, 
				(LPVOID)hPrgDlg, 0, &(hPrgDlg->dwThreadID));

			// Initialize GUI elements
			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETRANGE, 
				0, MAKELPARAM(0, 100));
			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS,
				(WPARAM)hPrgDlg->dwBar, 0);

			if(hPrgDlg->szTitle)
				SetWindowText(hDlg,hPrgDlg->szTitle);

			if(hPrgDlg->szFile)
				SetWindowText(GetDlgItem(hPrgDlg->hwnd,IDC_PRGFILE),hPrgDlg->szFile);

			if(hPrgDlg->szInfo)
				SetWindowText(GetDlgItem(hPrgDlg->hwnd,IDC_PRGINFO),hPrgDlg->szInfo);

			if(hPrgDlg->dwAVIClip!=0)
			{
				hPrgDlg->bAVIPlaying=TRUE;
				Animate_Open(GetDlgItem(hPrgDlg->hwnd,IDC_PRGAVI),
					MAKEINTRESOURCE(hPrgDlg->dwAVIClip));
			}

			// Create update timer
			SetTimer(hDlg,UPDATETIMERID,UPDATETIME,NULL);

			// Center window and set to foreground
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

		case WM_TIMER:
		{
			BOOL bWindowEnabled;

			hPrgDlg = (HPRGDLG) GetWindowLong(hDlg, GWL_USERDATA);

			if(hPrgDlg->dwAVIClip!=0)
			{
				bWindowEnabled=IsWindowEnabled(hPrgDlg->hwnd);

				if((!bWindowEnabled)&&(hPrgDlg->bAVIPlaying))
				{
					hPrgDlg->bAVIPlaying=FALSE;
					Animate_Stop(GetDlgItem(hPrgDlg->hwnd,IDC_PRGAVI));
				}

				if((bWindowEnabled)&&(!hPrgDlg->bAVIPlaying))
				{
					hPrgDlg->bAVIPlaying=TRUE;
					Animate_Play(GetDlgItem(hPrgDlg->hwnd,IDC_PRGAVI), 0, -1, -1);
				}
			}

			// If there has been a change, process it
			if(hPrgDlg->dwNewValue!=hPrgDlg->dwOldValue)
			{
				hPrgDlg->dwOldValue=hPrgDlg->dwNewValue;

				SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS,
					(WPARAM)hPrgDlg->dwNewValue, 0);
			}

			break;
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
					hPrgDlg = (HPRGDLG) GetWindowLong(hDlg, GWL_USERDATA);
					ShowWindow(hDlg,SW_HIDE);

					hPrgDlg->bCancel=TRUE;
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

PGPError SCProgressDialog(HWND hwndParent,USERFUNCTION UserFunction,
						  void *pUserValue,DWORD dwBar,char *szTitle,
						  char *szFile,char *szInfo,DWORD dwAVIClip)
{
	HPRGDLG hPrgDlg;
	PGPError err;

	hPrgDlg=(HPRGDLG)malloc(sizeof(PRGDLG));

	if(hPrgDlg==NULL)
	{
		return kPGPError_OutOfMemory;
	}

	memset(hPrgDlg,0x00,sizeof(PRGDLG));

	hPrgDlg->hwndParent=hwndParent;
	hPrgDlg->UserFunction=UserFunction;
	hPrgDlg->pUserValue=pUserValue;
	hPrgDlg->dwBar=dwBar;
	hPrgDlg->szTitle=szTitle;
	hPrgDlg->szFile=szFile;
	hPrgDlg->szInfo=szInfo;
//	hPrgDlg->dwAVIClip=0;
	hPrgDlg->dwAVIClip=dwAVIClip;

	// Just to be sure, a semaphore reinforces the barrier
	// made by the DialogBox.
	hPrgDlg->hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	DialogBoxParam (g_hinst, 
		MAKEINTRESOURCE(IDD_PRGDLG),
		hwndParent, 
		(DLGPROC)PrgDlgProc, 
		(LPARAM)hPrgDlg);

	WaitForSingleObject (hPrgDlg->hSemaphore, INFINITE);
	CloseHandle (hPrgDlg->hSemaphore);

	err=hPrgDlg->err;

	free(hPrgDlg);

	return err;
}

PGPError SCSetProgressBar(HPRGDLG hPrgDlg,DWORD dwBarValue,BOOL bForce)
{
	hPrgDlg->dwNewValue=dwBarValue;

	if(bForce)
	{
		hPrgDlg->dwOldValue=hPrgDlg->dwNewValue;

		SendDlgItemMessage (hPrgDlg->hwnd, IDC_PROGRESS, PBM_SETPOS,
				(WPARAM)dwBarValue, 0);
	}

	if(hPrgDlg->bCancel)
		return kPGPError_UserAbort;

	return kPGPError_NoErr;
}

PGPError SCSetProgressTitle(HPRGDLG hPrgDlg,char *szTitle)
{
	SetWindowText(hPrgDlg->hwnd,szTitle);

	return kPGPError_NoErr;
}

PGPError SCSetProgressFile(HPRGDLG hPrgDlg,char *szFile)
{
	SetWindowText(GetDlgItem(hPrgDlg->hwnd,IDC_PRGFILE),szFile);

	return kPGPError_NoErr;
}

PGPError SCSetProgressInfo(HPRGDLG hPrgDlg,char *szInfo)
{
	SetWindowText(GetDlgItem(hPrgDlg->hwnd,IDC_PRGINFO),szInfo);

	return kPGPError_NoErr;
}

PGPError SCSetProgressAVI(HPRGDLG hPrgDlg,DWORD dwAVIClip)
{
	HWND hwndAVI;

	hwndAVI=GetDlgItem(hPrgDlg->hwnd,IDC_PRGAVI);

	hPrgDlg->dwAVIClip=dwAVIClip;

	if(hPrgDlg->dwAVIClip!=0)
	{
		hPrgDlg->bAVIPlaying=TRUE;
		
		Animate_Open(hwndAVI,
			MAKEINTRESOURCE(hPrgDlg->dwAVIClip));
		ShowWindow(hwndAVI,SW_SHOW);
	}
	else
	{
		hPrgDlg->bAVIPlaying=FALSE;

		ShowWindow(hwndAVI,SW_HIDE);
		Animate_Close(hwndAVI);
	}
	
	return kPGPError_NoErr;
}

HWND SCGetProgressHWND(HPRGDLG hPrgDlg)
{
	return hPrgDlg->hwnd;
}

BOOL SCGetProgressCancel(HPRGDLG hPrgDlg)
{
	return hPrgDlg->bCancel;
}

void *SCGetProgressUserValue(HPRGDLG hPrgDlg)
{
	return hPrgDlg->pUserValue;
}

void SCSetProgressNewFilename(HPRGDLG hPrgDlg,char *szInfoFormat,
							  char *infile,BOOL bResetBar)
{
	char *p;
	char *name;
	char *directory;
	char filename[1000];
	char info[1000];

	name=directory=NULL;

	strcpy(filename,infile);

	// Just get the filename
	p=strrchr(filename,'\\');

	if(p!=NULL)
	{
		*p=0;
		p++;
		name=p;

		// Now look for directory
		p=strrchr(filename,'\\');

		if(p!=NULL)
		{
			*p=0;
			p++;
			directory=p;
		}
		else
		{
			directory=filename;
		}
	}
	else
	{
		// oops... must be in the root or something
		name=filename;
	}

	if(directory!=NULL)
	{
		sprintf(info,szInfoFormat,directory,directory);
	}
	else
	{
		strcpy(info,"");
	}

	SCSetProgressFile(hPrgDlg,name);
	SCSetProgressInfo(hPrgDlg,info);

	if(bResetBar)
		SCSetProgressBar(hPrgDlg,0,TRUE);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

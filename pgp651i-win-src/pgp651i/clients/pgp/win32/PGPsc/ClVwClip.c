/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: ClVwClip.c,v 1.16 1999/03/10 02:34:38 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

char* m_pDialogOutput = NULL;
DWORD m_dwDialogOuputSize = 0;

BOOL WINAPI ClipboardViewProc(HWND hdlg, UINT uMsg, 
							  WPARAM wParam, LPARAM lParam);


UINT EnclyptorViewClipboard(HWND hwnd, void* pInput,DWORD dwInSize, 
							void** ppOutput, DWORD* pOutSize)
{
	UINT ReturnValue = FALSE;
	BOOL DialogReturn = FALSE;
	char* szDialogInput = NULL;
	
	assert(pInput);
	assert(ppOutput);

	DialogReturn = DialogBoxParam(g_hinst, 
		MAKEINTRESOURCE(IDD_CLIPBOARDVIEWER), hwnd, 
		ClipboardViewProc, (LPARAM) pInput);

	if(DialogReturn && m_pDialogOutput)
	{
		*ppOutput = m_pDialogOutput;
		*pOutSize = m_dwDialogOuputSize;

		ReturnValue = TRUE;
	}

	return ReturnValue;
}

void SizeEditClip(HWND hdlg,int Width,int Height)
{
	InvalidateRect(GetDlgItem(hdlg, IDC_EDIT1),NULL,TRUE);

	MoveWindow(GetDlgItem(hdlg, IDC_EDIT1),
		10,10,
		Width-20,Height-55,
		TRUE);

	InvalidateRect(GetDlgItem(hdlg, IDCANCEL),NULL,TRUE);

	MoveWindow(GetDlgItem(hdlg, IDCANCEL),
		Width-130,Height-35,
		120,25,
		TRUE);

	InvalidateRect(GetDlgItem(hdlg, IDOK),NULL,TRUE);

	MoveWindow(GetDlgItem(hdlg, IDOK),
		Width-260,Height-35,
		120,25,
		TRUE);
}

BOOL WINAPI ClipboardViewProc(HWND hdlg, UINT uMsg, 
							  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			RECT rc;

			SetWindowText(GetDlgItem(hdlg,IDC_EDIT1), (char*)lParam);
			GetClientRect(hdlg,&rc);
			SizeEditClip(hdlg,rc.right-rc.left,rc.bottom-rc.top);
			SetFocus(GetDlgItem(hdlg,IDC_EDIT1));

            GetWindowRect (hdlg, &rc);
            SetWindowPos (hdlg, NULL,
                (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
                (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

			SetForegroundWindow(hdlg);
			return FALSE;
		}
	
		case WM_QUIT:
		case WM_CLOSE:
		{
			EndDialog(hdlg, FALSE);
			break;
		} 
		
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* lpmmi;

		    lpmmi = (MINMAXINFO*) lParam;
    		lpmmi->ptMinTrackSize.x = 300;
    		lpmmi->ptMinTrackSize.y = 200;
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
					GetSysColor (COLOR_WINDOWTEXT));      
			  	return (BOOL)CreateSolidBrush (color);
			} 	 
			break;
		}

		case WM_SIZE:
		{
			unsigned short Width, Height;
			Width = LOWORD(lParam);  // width of client area 
			Height = HIWORD(lParam); // height of client area 

			SizeEditClip(hdlg,Width,Height);
			break;
		}

		case WM_COMMAND:
		{
			switch(wParam)
			{
				case IDOK:
				{
					DWORD TextLength = 0;

					TextLength=GetWindowTextLength(GetDlgItem(hdlg, IDC_EDIT1));
					TextLength=TextLength+1; // Bug in GetWindowTextLength

					if(TextLength)
					{
						m_pDialogOutput = (char*) malloc( TextLength+1 );
						memset(m_pDialogOutput,0x00,TextLength+1);
						m_dwDialogOuputSize = TextLength;

						if(m_pDialogOutput)
						{
							GetWindowText(GetDlgItem(hdlg, IDC_EDIT1),
								  m_pDialogOutput, TextLength);

							m_pDialogOutput[TextLength]=0; // Trailing NULL
						}
					}
					else
					{
						m_pDialogOutput = (char *) malloc(1);
						m_dwDialogOuputSize=0;
						*m_pDialogOutput = '\0';
					}

					EndDialog(hdlg, TRUE);
					break;
				}

				case IDCANCEL:
				{
					EndDialog(hdlg, FALSE);
					break;
				}

			}
			return TRUE;
		}

	}
	return FALSE;
}

void TextViewer(HWND hwnd,char *szInText,DWORD dwInSize)
{
	DWORD dwOutSize = 0;
	char* szOutText = NULL;
 	BOOL WinNT;

    WinNT=FALSE;
    
	{
#ifdef _WIN32		
		OSVERSIONINFO osid;
		osid.dwOSVersionInfoSize = sizeof (osid);
		GetVersionEx (&osid);   
		WinNT=(osid.dwPlatformId == VER_PLATFORM_WIN32_NT);
#endif // WIN32    		
		if (WinNT || (dwInSize < 65535))
		{
			if(EnclyptorViewClipboard(hwnd,szInText,dwInSize, 
				&szOutText,&dwOutSize) == TRUE)
			{
				StoreClipboardData(hwnd,szOutText,dwOutSize);
				memset(szOutText,0x00,dwOutSize);
				free(szOutText);
			}
		}
		else 
		{
			PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_CLIPBOARDEXCEEDED,
					MB_OK|MB_ICONSTOP);
		}
	}
}

void LaunchInternalViewer(void *PGPsc,HWND hwnd)
{
	void *pClipboardData;
	UINT ClipboardFormat = 0;
	DWORD dwDataSize = 0;
	DWORD dwOutputSize = 0;
	void* pOutputBuffer = NULL;
 	BOOL WinNT;

	pClipboardData = RetrieveClipboardData(hwnd, &ClipboardFormat, 
						&dwDataSize);
  
	if(pClipboardData==0)
	{
		pClipboardData=(void *)malloc(1);
		memset(pClipboardData,0x00,1);
	}

    WinNT=FALSE;
    
	if(pClipboardData)
	{
		if(ClipboardFormat == CF_TEXT)
		{    
//			Always use editor when we edit. -wjb
//
//			if(GetSecureViewerPref(PGPsc))
//			{
//				TempestViewer(PGPsc,hwnd,pClipboardData,dwDataSize,FALSE);
//			}
//			else
//			{
				TextViewer(hwnd,
					pClipboardData,dwDataSize);
//			}
		}
		memset(pClipboardData,0x00,dwDataSize);
		free(pClipboardData);
	}
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


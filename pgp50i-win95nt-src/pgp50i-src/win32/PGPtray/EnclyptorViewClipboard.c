/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <assert.h>

#include "..\include\config.h"
#include "..\include\pgpmem.h"

#include "define.h"
#include "resource.h"

char* m_pDialogOutput = NULL;
DWORD m_dwDialogOuputSize = 0;

extern HINSTANCE g_hinst;

BOOL WINAPI ClipboardViewProc(HWND hdlg, UINT uMsg, WPARAM wParam,
								LPARAM lParam);


UINT EnclyptorViewClipboard(HWND hwnd, void* pInput,DWORD dwInSize,
							void** ppOutput, DWORD* pOutSize)
{
	UINT ReturnValue = STATUS_FAILURE;
	BOOL DialogReturn = FALSE;
	char* szDialogInput = NULL;
	
	assert(pInput);
	assert(ppOutput);

	// No guarantee our data is null terminated so let's do that.

	szDialogInput = (char*) pgpAlloc ( dwInSize + 1);

	if(szDialogInput)
	{
		memcpy(szDialogInput, pInput, dwInSize);
		*(szDialogInput + dwInSize) = 0x00;

		DialogReturn = DialogBoxParam(g_hinst,
								MAKEINTRESOURCE(IDD_CLIPBOARDVIEWER), hwnd,
								ClipboardViewProc, (LPARAM) szDialogInput);

		pgpFree(szDialogInput);
	}

	if(DialogReturn && m_pDialogOutput)
	{
		*ppOutput = m_pDialogOutput;
		*pOutSize = m_dwDialogOuputSize;

		//vErrorOut(fg_white, "%s\n%d\n", m_pDialogOutput, m_dwDialogOuputSize);

		ReturnValue = STATUS_SUCCESS;
	}

	return ReturnValue;
}

BOOL WINAPI ClipboardViewProc(HWND hdlg, UINT uMsg, WPARAM wParam,
								LPARAM lParam)
{
	static HWND hWnd = NULL;

	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			if(!hWnd)
			{
				hWnd = hdlg;
				if(lParam)
				{
					SetWindowText(GetDlgItem(hdlg,IDC_EDIT1), (char*)lParam);
					SetFocus(GetDlgItem(hdlg,IDC_EDIT1));
				}
			}
			else
			{
				char *ExistingBuffer;
				int BufferSize;

				SetActiveWindow(hWnd);
				BufferSize = GetWindowTextLength(GetDlgItem(hWnd, IDC_EDIT1));
				if(BufferSize)
				{
					ExistingBuffer = pgpAlloc(sizeof(char) * (BufferSize + 1));
					if(ExistingBuffer)
					{
						GetWindowText(GetDlgItem(hWnd, IDC_EDIT1),
										ExistingBuffer, BufferSize);
						if(strcmp(ExistingBuffer, (char *) lParam) != 0)
						{
							if(MessageBox(hdlg,
										  "The Clipboard Contents have "
											  "Changed.  Reload?",
										  "PGP",
									      MB_ICONQUESTION | MB_YESNO) == IDYES)
							{
								SetWindowText(GetDlgItem(hWnd, IDC_EDIT1),
												(char *) lParam);
							}
						}
					}
				}
				SetActiveWindow(hWnd);
				EndDialog(hdlg, FALSE);
			}
			return FALSE;
		}

		case WM_CLOSE:
		{
			hWnd = NULL;
			break;
		}

		case WM_SIZING:
		{
			LPRECT pRect;

			pRect = (LPRECT) lParam;
			if(pRect->right - pRect->left < 200)
				pRect->right = pRect->left + 200;

			if(pRect->bottom - pRect->top < 200)
				pRect->bottom = pRect->top + 200;
			break;
		}

		case WM_SIZE:
		{
			unsigned short Width, Height;
			Width = LOWORD(lParam);  // width of client area
			Height = HIWORD(lParam); // height of client area

			SetWindowPos(GetDlgItem(hdlg, IDC_EDIT1),
						 NULL,
						 0,
						 0,
						 Width - 10,
						 Height - 42,
						 SWP_NOMOVE | SWP_NOZORDER);

			SetWindowPos(GetDlgItem(hdlg, IDCANCEL),
						 NULL,
						 Width - 79,
						 Height - 28,
						 0,
						 0,
						 SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(GetDlgItem(hdlg, IDOK),
						 NULL,
						 Width - 166,
						 Height - 28,
						 0,
						 0,
						 SWP_NOZORDER | SWP_NOSIZE);

			InvalidateRect(GetDlgItem(hdlg, IDC_EDIT1),NULL,TRUE);
			InvalidateRect(GetDlgItem(hdlg, IDOK),NULL,TRUE);
			InvalidateRect(GetDlgItem(hdlg, IDCANCEL),NULL,TRUE);

			UpdateWindow(hdlg);
			break;
		}

		case WM_COMMAND:
		{
			switch(wParam)
			{
				case IDOK:
				{
					DWORD TextLength = 0;

					TextLength = GetWindowTextLength(GetDlgItem(hdlg,
																IDC_EDIT1));

					if(TextLength)
					{
						TextLength += 2;

						m_pDialogOutput = (char*) pgpAlloc( TextLength );

						if(m_pDialogOutput)
						{
							m_dwDialogOuputSize = GetWindowText(
											GetDlgItem(hdlg, IDC_EDIT1),
											m_pDialogOutput, TextLength);

							m_dwDialogOuputSize += 1; // make room for NULL;

							//MessageBox(NULL, m_pDialogOutput,
							//			"m_pDialogOutput", MB_OK);
						}
					}
					else
					{
						m_pDialogOutput = (char *) pgpAlloc(1);
						*m_pDialogOutput = '\0';
					}

					hWnd = NULL;
					EndDialog(hdlg, TRUE);
					break;
				}

				case IDCANCEL:
				{
					hWnd = NULL;
					EndDialog(hdlg, FALSE);
					break;
				}

			}
			return TRUE;
		}

	}
	return FALSE;
}

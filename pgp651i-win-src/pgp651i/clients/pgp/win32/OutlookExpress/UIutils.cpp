/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: UIutils.cpp,v 1.6 1999/04/09 15:26:06 dgal Exp $
____________________________________________________________________________*/
#include <windows.h>
#include <commctrl.h>
#include <stdarg.h>
#include <crtdbg.h>

#include "pgpDebug.h"
#include "UIutils.h"
#include "resource.h"
#include "pgpKeys.h"
#include "PGPcl.h"

#define WARN_BIT		0x0001
#define CONTINUE_BIT	0x0002

static HINSTANCE g_hInstance;

typedef struct _HexDumpParam
{
	char *szText;
	int nLength;
} HexDumpParam;


BOOL CALLBACK WarnUserDlgProc(HWND hwndDlg, 
							  UINT uMsg, 
							  WPARAM wParam, 
							  LPARAM lParam);

BOOL CALLBACK HexDumpDlgProc(HWND hwndDlg, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam);

void DumpText(HWND hList, char *szText, int nLength);


HINSTANCE UIGetInstance(void)
{
	return g_hInstance;
}


void UISetInstance(HINSTANCE hInstance)
{
	g_hInstance = hInstance;
}


void UIGetString(char *szBuffer, size_t nLength, UINT nID)
{
	LoadString(g_hInstance, nID, szBuffer, nLength);
	return;
}


void UIFormatString(char *szBuffer, size_t nLength, UINT nID, char *szData)
{
	char szTemp[1024];

    LoadString(g_hInstance, nID, szTemp, 1023);
	wsprintf(szBuffer, szTemp, szData);
	return;
}


void UIDisplayString(HWND hwnd, char *szBuffer)
{
	char szTitle[255];

	LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
	MessageBox(hwnd, szBuffer, szTitle, MB_OK);
	return;
}


void UIDisplayStringID(HWND hwnd, UINT nID)
{
	char szTitle[255];
	char szBuffer[1024];

	LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
	LoadString(g_hInstance, nID, szBuffer, 1023);
	MessageBox(hwnd, szBuffer, szTitle, MB_OK);
	return;
}


BOOL UIAskYesNoStringID(HWND hwnd, UINT nID)
{
	char szTitle[255];
	char szBuffer[1024];

	LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
	LoadString(g_hInstance, nID, szBuffer, 1023);
	
	return MessageBox(hwnd, szBuffer, szTitle, MB_YESNO);
}


void UIDisplayErrorCode(char *szFile, 
						int nLine, 
						char *szModule, 
						int nCode)
{
	char szErrorMsg[255];
	char szTitle[255];

	PGPclEncDecErrorToString(nCode, szErrorMsg, 254);

#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
	_CrtDbgReport(_CRT_ERROR, szFile, nLine, szModule, szErrorMsg);
#endif

	LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
	MessageBox(NULL, szErrorMsg, szTitle, MB_ICONEXCLAMATION);
	return;
}


BOOL UIWarnUser(HWND hwnd, UINT nID, char *szRegValue)
{
	HKEY hkey;
	char szRegKey[255];
	char szMessage[255];
	BOOL bContinue;
	DWORD dwResult;
	DWORD dwDummy;
	DWORD dwValue;
	DWORD dwType;
	DWORD dwSize=4;

	UIGetString(szMessage, 254, nID);
	UIGetString(szRegKey, 254, IDS_WARNING_REGKEY);

	RegCreateKeyEx(HKEY_CURRENT_USER, szRegKey, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDummy);

	if (RegQueryValueEx(hkey, szRegValue, 0, &dwType, (BYTE *) &dwValue, 
		&dwSize) != ERROR_SUCCESS)
	{
		dwValue = TRUE;
		RegSetValueEx(hkey, szRegValue, 0, REG_DWORD, (BYTE *) &dwValue, 
			sizeof(DWORD));
	}
	
	if (dwValue == FALSE)
		return TRUE;

	dwResult = DialogBoxParam(UIGetInstance(), MAKEINTRESOURCE(IDD_WARN), 
					hwnd, (DLGPROC) WarnUserDlgProc, (LPARAM) szMessage);

	dwValue = (dwResult & WARN_BIT) == WARN_BIT;
	bContinue = (dwResult & CONTINUE_BIT) == CONTINUE_BIT;

	RegSetValueEx(hkey, szRegValue, 0, REG_DWORD, (BYTE *) &dwValue, 
		sizeof(DWORD));
	RegCloseKey(hkey);

	return bContinue;
}


void PGPDebugMessage(const char *szMessage)
{
	char szTitle[255];

#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
	_CrtDbgReport(_CRT_ERROR, NULL, NULL, NULL, szMessage);
#endif

	LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
	MessageBox(NULL, szMessage, szTitle, MB_ICONEXCLAMATION);
	return;
}


void UIHexDump(HWND hwnd, char *szMessage, int nLength)
{
	HexDumpParam lParam;

	lParam.szText = szMessage;
	lParam.nLength = nLength;

	DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_HEXDUMP), hwnd, 
		(DLGPROC) HexDumpDlgProc, (LPARAM) &lParam);

	return;
}


BOOL CALLBACK WarnUserDlgProc(HWND hwndDlg, 
							  UINT uMsg, 
							  WPARAM wParam, 
							  LPARAM lParam)
{
	BOOL bReturnCode = FALSE;
	DWORD dwResult;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(GetDlgItem(hwndDlg, IDC_WARNING), (char *) lParam);
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_YESCONTINUE:
			if (IsDlgButtonChecked(hwndDlg, IDC_NOWARN) == BST_CHECKED)
				dwResult = CONTINUE_BIT;
			else
				dwResult = CONTINUE_BIT | WARN_BIT;

			EndDialog(hwndDlg, dwResult);
			bReturnCode = TRUE;
			break;

		case IDC_NOCONTINUE:
			if (IsDlgButtonChecked(hwndDlg, IDC_NOWARN) == BST_CHECKED)
				dwResult = 0;
			else
				dwResult = WARN_BIT;

			EndDialog(hwndDlg, dwResult);
			bReturnCode = TRUE;
			break;
		}
		break;
	}

	return(bReturnCode);
}


BOOL CALLBACK HexDumpDlgProc(HWND hwndDlg, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam)
{
	BOOL bReturnCode = FALSE;
	HWND hList;

	hList = GetDlgItem(hwndDlg, IDC_DUMP);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			LV_COLUMN lvcText;
			HexDumpParam *pParam;

			// Setup the list control columns
			lvcText.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lvcText.fmt = LVCFMT_LEFT;
			lvcText.cx = 500;
			lvcText.pszText = "ASCII";
			lvcText.iSubItem = -1;
			ListView_InsertColumn(hList, 0, &lvcText);
			
			lvcText.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lvcText.fmt = LVCFMT_LEFT;
			lvcText.cx = 1400;
			lvcText.pszText = "Hex";
			lvcText.iSubItem = 1;
			ListView_InsertColumn(hList, 1, &lvcText);

			pParam = (HexDumpParam *) lParam;

			// Fill list control with text
			DumpText(hList, pParam->szText, pParam->nLength);
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, FALSE);
			bReturnCode = TRUE;
			break;
		}
		break;
	}

	return(bReturnCode);
}


void DumpText(HWND hList, char *szText, int nLength)
{
	int nX, nY;
	int nHexIndex, nAsciiIndex;
	int nLimit;
	int nLines;
	char szHex[241];
	char szAscii[81];
	LV_ITEM	lviText;

	nLines = 0;

	for (nX=0; nX<nLength; nX+=80)
	{
		nHexIndex = 0;
		nAsciiIndex = 0;

		if ((nX+80) > nLength)
			nLimit = nLength;
		else
			nLimit = nX+80;

		for (nY=nX; nY<nLimit; nY++)
		{
			wsprintf(&(szHex[nHexIndex]), "%02X ", szText[nY]);
			nHexIndex += 3;

			if ((szText[nY] > 31) && (szText[nY] < 127))
				wsprintf(&(szAscii[nAsciiIndex]), "%c", szText[nY]);
			else if (szText[nY] == '\n')
			{
				nX = nY-79;
				nY = nLimit;
			}
			else
				wsprintf(&(szAscii[nAsciiIndex]), " ");
			nAsciiIndex++;
		}
		
		szHex[nHexIndex] = 0;
		szAscii[nAsciiIndex] = 0;

		lviText.mask = LVIF_TEXT;
		lviText.pszText = szAscii;
		lviText.iItem = nLines;
		lviText.iSubItem = 0;
		ListView_InsertItem(hList, &lviText);

		lviText.mask = LVIF_TEXT;
		lviText.pszText = szHex;
		lviText.iItem = nLines;
		lviText.iSubItem = 1;
		ListView_SetItem(hList, &lviText);

		nLines++;
	}

	return;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

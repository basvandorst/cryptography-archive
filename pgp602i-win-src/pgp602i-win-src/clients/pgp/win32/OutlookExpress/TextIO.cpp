/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: TextIO.cpp,v 1.7.8.1 1998/11/12 03:12:49 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <richedit.h>
#include "resource.h"
#include "PluginInfo.h"
#include "TextIO.h"
#include "UIutils.h"

BOOL CopyRichEditCtrl(HWND hwnd, char **szText);
BOOL GetNextLine(char *szBuffer, UINT *pnIndex, char *szLine, int nMax);


BOOL GetRecipientText(HWND hwnd, char **szRecipients)
{
	HWND hNoteHdr;
	HWND hTo, hCc, hBcc;
	char *szTo = NULL;
	char *szCc = NULL;
	char *szBcc = NULL;
	BOOL bSuccess = FALSE;

	hNoteHdr = FindWindowEx(hwnd, NULL, "ATH_NoteHdr", NULL);
	hTo = FindWindowEx(hNoteHdr, NULL, "RICHEDIT", NULL);
	hCc = FindWindowEx(hNoteHdr, hTo, "RICHEDIT", NULL);
	hBcc = FindWindowEx(hNoteHdr, hCc, "RICHEDIT", NULL);

	if (!CopyRichEditCtrl(hTo, &szTo))
		goto GetRecipientTextError;

	if (!CopyRichEditCtrl(hCc, &szCc))
		goto GetRecipientTextError;

	if (!CopyRichEditCtrl(hBcc, &szBcc))
		goto GetRecipientTextError;

	*szRecipients = (char *) calloc(strlen(szTo) + 
								strlen(szCc) + strlen(szBcc) + 3, 1);
	
	if (szTo[0] != '\r')
		strcat(*szRecipients, szTo);

	if (szCc[0] != '\r')
	{
		if (szTo[0] != '\r')
			strcat(*szRecipients, ",");

		strcat(*szRecipients, szCc);
	}
	
	if (szBcc[0] != '\r')
	{
		if ((szTo[0] != '\r') || (szCc[0] != '\r'))
			strcat(*szRecipients, ",");

		strcat(*szRecipients, szBcc);
	}

	bSuccess = TRUE;

GetRecipientTextError:

	if (szTo != NULL)
		free(szTo);

	if (szCc != NULL)
		free(szCc);

	if (szBcc != NULL)
		free(szBcc);

	return bSuccess;
}


BOOL GetMessageText(HWND hwnd, BOOL bCut, char **szMessage)
{
	HWND hDocHost;
	HWND hMessage;
	HANDLE hText;
	char *szText = NULL;
	char *szOldText = NULL;
	BOOL bSuccess = TRUE;

	OpenClipboard(hwnd);
	hText = GetClipboardData(CF_TEXT);
	szText = (char *) GlobalLock(hText);

	if (szText != NULL)
	{
		szOldText = (char *) calloc(strlen(szText)+1, 1);
		strcpy(szOldText, szText);
	}

	GlobalUnlock(hText);
	CloseClipboard();

	hDocHost = FindWindowEx(hwnd, NULL, "Ath_DocHost", NULL);
	hMessage = FindWindowEx(hDocHost, NULL, "Internet Explorer_Server", NULL);

	SetFocus(hDocHost);
	SetFocus(hMessage);
	SendMessage(hwnd, WM_COMMAND, 
		MAKEWPARAM(IDC_SELECTALL, 0), (LPARAM) hMessage);

	if (bCut)
	{
		SendMessage(hwnd, WM_COMMAND, 
			MAKEWPARAM(IDC_CUT, 0), (LPARAM) hMessage);
	}
	else
	{
		SendMessage(hwnd, WM_COMMAND, 
			MAKEWPARAM(IDC_COPY, 0), (LPARAM) hMessage);
	}
	
	OpenClipboard(hwnd);
	hText = GetClipboardData(CF_TEXT);
	szText = (char *) GlobalLock(hText);
	if (szText == NULL)
	{
		GlobalUnlock(hText);
		bSuccess = FALSE;
		goto GetMessageTextError;
	}

	if (strlen(szText) >= 65535)
	{
		OSVERSIONINFO osvi;

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)
		{
			UIDisplayStringID(hwnd, IDS_E_BUFFERTOOBIG);
			if (bCut)
			{
				SendMessage(hwnd, WM_COMMAND, 
					MAKEWPARAM(IDC_UNDO, 0), (LPARAM) hMessage);
			}
			
			bSuccess = FALSE;
		}
	}

	if (bSuccess == TRUE)
	{
		*szMessage = (char *) calloc(strlen(szText)+1, 1);
		strcpy(*szMessage, szText);
	}

	GlobalUnlock(hText);
	EmptyClipboard();

	if (szOldText != NULL)
	{
		hText = GlobalAlloc(GHND | GMEM_DDESHARE, strlen(szOldText)+1);
		szText = (char *) GlobalLock(hText);
		strcpy(szText, szOldText);
		GlobalUnlock(hText);

		if (!SetClipboardData(CF_TEXT, hText))
		{
			DWORD dwError;
			char szError[255];
			
			dwError = GetLastError();
			wsprintf(szError, "Error %d", dwError);
			MessageBox(hwnd, szError, NULL, MB_ICONEXCLAMATION);
			bSuccess = FALSE;
			
			goto GetMessageTextError;
		}
	}

GetMessageTextError:

	CloseClipboard();

	if (szOldText != NULL)
		free(szOldText);

	return bSuccess;
}


BOOL SetMessageText(HWND hwnd, char *szMessage)
{
	HWND hDocHost;
	HWND hMessage;
	HANDLE hText;
	char *szText = NULL;
	char *szOldText = NULL;
	BOOL bSuccess = TRUE;
	
	if (strlen(szMessage) >= 65535)
	{
		OSVERSIONINFO osvi;

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)
		{
			UIDisplayStringID(hwnd, IDS_E_BUFFERTOOBIG);
			bSuccess = FALSE;
			goto SetMessageTextError;
		}
	}

	hDocHost = FindWindowEx(hwnd, NULL, "Ath_DocHost", NULL);
	hMessage = FindWindowEx(hDocHost, NULL, "Internet Explorer_Server", NULL);
	
	OpenClipboard(hwnd);
	hText = GetClipboardData(CF_TEXT);
	szText = (char *) GlobalLock(hText);

	if (szText != NULL)
	{
		szOldText = (char *) calloc(strlen(szText)+1, 1);
		strcpy(szOldText, szText);
	}

	GlobalUnlock(hText);
	EmptyClipboard();

	hText = GlobalAlloc(GHND | GMEM_DDESHARE, strlen(szMessage)+1);
	szText = (char *) GlobalLock(hText);
	if (szText != NULL)
	{
		strcpy(szText, szMessage);
		GlobalUnlock(hText);
		
		if (!SetClipboardData(CF_TEXT, hText))
		{
			DWORD dwError;
			char szError[255];
			
			dwError = GetLastError();
			wsprintf(szError, "Error %d", dwError);
			MessageBox(hwnd, szError, NULL, MB_ICONEXCLAMATION);
			bSuccess = FALSE;
			
			goto SetMessageTextError;
		}
	}

	CloseClipboard();

	SetFocus(hDocHost);
	SetFocus(hMessage);
	SendMessage(hwnd, WM_COMMAND, 
		MAKEWPARAM(IDC_SELECTALL, 0), (LPARAM) hMessage);
	SendMessage(hwnd, WM_COMMAND, 
		MAKEWPARAM(IDC_PASTE, 0), (LPARAM) hMessage);

	OpenClipboard(hwnd);
	EmptyClipboard();

	if (szOldText != NULL)
	{
		hText = GlobalAlloc(GHND | GMEM_DDESHARE, strlen(szOldText)+1);
		szText = (char *) GlobalLock(hText);
		strcpy(szText, szOldText);
		GlobalUnlock(hText);
		
		if (!SetClipboardData(CF_TEXT, hText))
		{
			DWORD dwError;
			char szError[255];
			
			dwError = GetLastError();
			wsprintf(szError, "Error %d", dwError);
			MessageBox(hwnd, szError, NULL, MB_ICONEXCLAMATION);
			bSuccess = FALSE;
			
			goto SetMessageTextError;
		}
	}

SetMessageTextError:
	CloseClipboard();

	if (szOldText != NULL)
		free(szOldText);

	return bSuccess;
}


BOOL CopyRichEditCtrl(HWND hwnd, char **szText)
{
	CHARRANGE chRange;
	HANDLE hText;
	char *szLocalText = NULL;
	char *szOldText = NULL;
	BOOL bSuccess = FALSE;

	OpenClipboard(hwnd);
	hText = GetClipboardData(CF_TEXT);
	szLocalText = (char *) GlobalLock(hText);

	if (szLocalText != NULL)
	{
		szOldText = (char *) calloc(strlen(szLocalText)+1, 1);
		strcpy(szOldText, szLocalText);
	}

	GlobalUnlock(hText);
	CloseClipboard();

	chRange.cpMin = 0;
	chRange.cpMax = -1;
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &chRange);
	SendMessage(hwnd, WM_COPY, 0, 0);
	
	OpenClipboard(hwnd);
	hText = GetClipboardData(CF_TEXT);
	szLocalText = (char *) GlobalLock(hText);

	if (szLocalText != NULL)
	{
		*szText = (char *) calloc(strlen(szLocalText)+1, 1);
		strcpy(*szText, szLocalText);
	}

	GlobalUnlock(hText);
	EmptyClipboard();

	if (szOldText != NULL)
	{
		hText = GlobalAlloc(GHND | GMEM_DDESHARE, strlen(szOldText)+1);
		szLocalText = (char *) GlobalLock(hText);
		strcpy(szLocalText, szOldText);
		GlobalUnlock(hText);
		
		if (!SetClipboardData(CF_TEXT, hText))
		{
			DWORD dwError;
			char szError[255];
			
			dwError = GetLastError();
			wsprintf(szError, "Error %d", dwError);
			MessageBox(hwnd, szError, NULL, MB_ICONEXCLAMATION);
			
			goto CopyRichEditCtrlError;
		}
	}

	bSuccess = TRUE;

CopyRichEditCtrlError:

	CloseClipboard();

	if (szOldText != NULL)
		free(szOldText);

	return bSuccess;
}


BOOL FindReplyHeader(char *szReplyHeader)
{
	char szLine[1024];
	UINT nIndex;
	int nStop;
	BOOL bEnd = FALSE;

	nIndex = 0;
	do
	{
		bEnd = GetNextLine(szReplyHeader, &nIndex, szLine, 1023);
		if (bEnd)
			break;

		szLine[5] = '\0';
	}
	while (strcmp(szLine, "Date:"));

	if (bEnd)
		return FALSE;

	do
	{
		nStop = nIndex;
		bEnd = GetNextLine(szReplyHeader, &nIndex, szLine, 1023);
		if (bEnd)
			break;
	}
	while (szLine[0] != '\r');

	if (bEnd)
		return FALSE;

	szReplyHeader[nStop] = 0;
	return TRUE;
}


void AddReply(char *szHeader, char *szReply)
{
	char szLine[1024];
	UINT nIndex = 0;

	strcat(szHeader, "\r\n\r\n");

	while (!GetNextLine(szReply, &nIndex, szLine, 1023))
	{
		strcat(szHeader, ">");
		strcat(szHeader, szLine);
	}

	return;
}


BOOL GetNextLine(char *szBuffer, UINT *pnIndex, char *szLine, int nMax)
{
	char cCopy;
	int nLength;

	if (*pnIndex > strlen(szBuffer))
		return TRUE;

	nLength = 0;
	do
	{
		cCopy = szBuffer[(*pnIndex)++];
		szLine[nLength] = cCopy;
	}
	while ((cCopy != '\n') && (cCopy != 0) && (nLength++ < nMax));

	szLine[nLength] = 0;
	return FALSE;
}


void FixBadSpaces(char *szInput)
{
	int nIndex = 0;
	int nLength;

	if (szInput == NULL)
		return;

	nLength = strlen(szInput);
	while (nIndex <= nLength)
	{
		if (szInput[nIndex] == (char) 0xa0)
			szInput[nIndex] = 0x20;

		nIndex++;
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

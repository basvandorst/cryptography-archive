/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: TextIO.cpp,v 1.9 1999/04/09 15:26:06 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <richedit.h>
#include <process.h>
#include "resource.h"
#include "PluginInfo.h"
#include "TextIO.h"
#include "UIutils.h"
#include "PGPsc.h"

BOOL CopyRichEditCtrl(HWND hwnd, char **szText);
BOOL GetNextLine(char *szBuffer, UINT *pnIndex, char *szLine, int nMax);


BOOL GetRecipientText(HWND hwnd, char **szRecipients)
{
	HWND hContainer;
	HWND hTo, hCc, hBcc;
	char *szTo = NULL;
	char *szCc = NULL;
	char *szBcc = NULL;
	BOOL bSuccess = FALSE;
	PluginInfo *plugin;

	plugin = GetPluginInfo(hwnd);

	if (plugin->bOE5)
		hContainer = FindWindowEx(hwnd, NULL, "OE_Envelope", NULL);
	else
		hContainer = FindWindowEx(hwnd, NULL, "ATH_NoteHdr", NULL);

	hTo = FindWindowEx(hContainer, NULL, "RICHEDIT", NULL);
	hCc = FindWindowEx(hContainer, hTo, "RICHEDIT", NULL);
	hBcc = FindWindowEx(hContainer, hCc, "RICHEDIT", NULL);

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


BOOL SaveClipboardText(HWND hwnd)
{
	HANDLE hText;
	char *szText = NULL;
	BOOL bSuccess = TRUE;
	PluginInfo *plugin;

	plugin = GetPluginInfo(hwnd);

	OpenClipboard(hwnd);
	hText = GetClipboardData(CF_TEXT);
	szText = (char *) GlobalLock(hText);

	if (szText != NULL)
	{
		plugin->szOldText = (char *) calloc(strlen(szText)+1, 1);
		strcpy(plugin->szOldText, szText);
	}

	GlobalUnlock(hText);
	CloseClipboard();

	return bSuccess;
}


BOOL RestoreClipboardText(HWND hwnd)
{
	HANDLE hText;
	char *szText = NULL;
	BOOL bSuccess = TRUE;
	PluginInfo *plugin;

	plugin = GetPluginInfo(hwnd);

	OpenClipboard(hwnd);
	EmptyClipboard();

	if (plugin->szOldText != NULL)
	{
		hText = GlobalAlloc(GHND | GMEM_DDESHARE, 
					strlen(plugin->szOldText)+1);

		szText = (char *) GlobalLock(hText);
		strcpy(szText, plugin->szOldText);
		GlobalUnlock(hText);

		if (!SetClipboardData(CF_TEXT, hText))
		{
			DWORD dwError;
			char szError[255];
			
			dwError = GetLastError();
			wsprintf(szError, "Error %d", dwError);
			MessageBox(hwnd, szError, NULL, MB_ICONEXCLAMATION);
			bSuccess = FALSE;
			
			goto RestoreClipboardTextError;
		}
	}

RestoreClipboardTextError:

	CloseClipboard();

	if (plugin->szOldText != NULL)
	{
		free(plugin->szOldText);
		plugin->szOldText = NULL;
	}

	return bSuccess;
}


BOOL GetMessageText(HWND hwnd, BOOL bCut, char **szMessage)
{
	HANDLE hText;
	char *szText = NULL;
	BOOL bSuccess = TRUE;
	PluginInfo *plugin;

	plugin = GetPluginInfo(hwnd);

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
			bSuccess = FALSE;
		}
	}

	if (bSuccess == TRUE)
	{
		*szMessage = (char *) calloc(strlen(szText)+1, 1);
		strcpy(*szMessage, szText);
	}

	GlobalUnlock(hText);

GetMessageTextError:

	CloseClipboard();

	return bSuccess;
}


BOOL SetMessageText(HWND hwnd, char *szMessage)
{
	HANDLE hText;
	char *szText = NULL;
	BOOL bSuccess = TRUE;
	PluginInfo *plugin;

	plugin = GetPluginInfo(hwnd);
	
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

	OpenClipboard(hwnd);
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

SetMessageTextError:

	CloseClipboard();

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
	char szDate[256];
	char szSent[256];

	UIGetString(szDate, 255, IDS_REPLY_DATE);
	UIGetString(szSent, 255, IDS_REPLY_SENT);

	nIndex = 0;
	do
	{
		bEnd = GetNextLine(szReplyHeader, &nIndex, szLine, 1023);
		if (bEnd)
			break;

		szLine[5] = '\0';
	}
	while (strcmp(szLine, szDate) && strcmp(szLine, szSent));

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


void DoCopyThread(void *args)
{
	HWND hwnd;
	HWND hwndFocus;
	PluginInfo *plugin;
	BOOL bSuccess;

	hwnd = (HWND) args;
	plugin = GetPluginInfo(hwnd);

	Sleep(1000);
	bSuccess = DoCopy(hwnd, plugin->pgpContext, TRUE, &hwndFocus);
	PostMessage(hwnd, plugin->nCopyDoneMsg, bSuccess, 0);

	_endthread();
}


void DoPasteThread(void *args)
{
	HWND hwnd;
	PluginInfo *plugin;

	hwnd = (HWND) args;
	plugin = GetPluginInfo(hwnd);

	Sleep(1000);
	DoPaste(TRUE, hwnd);
	PostMessage(hwnd, plugin->nPasteDoneMsg, 0, 0);

	_endthread();
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

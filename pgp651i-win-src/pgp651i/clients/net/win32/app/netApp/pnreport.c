/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PNreport.c - PGPnet log and status file report generation
	

	$Id: pnreport.c,v 1.4 1998/12/09 15:48:32 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "PGPnetApp.h"

extern HINSTANCE	g_hinst;

//	____________________________________
//
//	save listview info to a text file

static PGPError
sSaveListViewToFile (
		HWND	hwndListView,
		LPSTR	pszFile,
		LPSTR	pszHeader)
{
	PGPError	err				= kPGPError_NoErr;

	INT			iNumColumns;
	INT			iNumRows;
	INT			iRow;
	INT			i;
	SYSTEMTIME	st;
	CHAR		szLine[256];
	CHAR		sz[64];
	HANDLE		hfile;
	DWORD		dw;
	LV_COLUMN	lvc;
	LV_ITEM		lvi;
	BOOL		b;

	// open file
	hfile = CreateFile (pszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
		return kPGPError_CantOpenFile;

	// write header line
	wsprintf (szLine, "\r\n%s\r\n", pszHeader);
	WriteFile (hfile, szLine, lstrlen (szLine), &dw, NULL);

	// write date/time line
	GetLocalTime (&st);
	GetTimeFormat (LOCALE_USER_DEFAULT, 0, 
								&st, NULL, sz, sizeof(sz));
	GetDateFormat (LOCALE_USER_DEFAULT, DATE_LONGDATE, 
								&st, NULL, szLine, sizeof(szLine));
	lstrcat (szLine, " ");
	lstrcat (szLine, sz);
	lstrcat (szLine, "\r\n\r\n");
	WriteFile (hfile, szLine, lstrlen (szLine), &dw, NULL);

	// write column headers
	i = 0;
	iNumColumns = 0;
	szLine[0] = '\0';
	lvc.mask = LVCF_TEXT;
	lvc.pszText = sz;
	lvc.cchTextMax = sizeof(sz);
	do 
	{
		lvc.iSubItem = i;
		b = ListView_GetColumn (hwndListView, i, &lvc);
		if (b)
		{
			iNumColumns++;
			lstrcat (szLine, sz);
			lstrcat (szLine, "\t");
		}
		i++;
	} while (b);
	lstrcat (szLine, "\r\n\r\n");
	WriteFile (hfile, szLine, lstrlen (szLine), &dw, NULL);

	// write lines of text
	lvi.mask = LVIF_TEXT;
	lvi.pszText = sz;
	lvi.cchTextMax = sizeof(sz);
	iNumRows = ListView_GetItemCount (hwndListView);
	for (iRow=0; iRow<iNumRows; iRow++)
	{
		lvi.iItem = iRow;
		szLine[0] = '\0';
		for (i=0; i<iNumColumns; i++) 
		{
			lvi.iSubItem = i;
			ListView_GetItem (hwndListView, &lvi);
			lstrcat (szLine, sz);
			lstrcat (szLine, "\t");
		}
		lstrcat (szLine, "\r\n");
		WriteFile (hfile, szLine, lstrlen (szLine), &dw, NULL);
	}

	CloseHandle (hfile);
	return err;
}

//	____________________________________
//
//	get file name from user

static PGPError
sGetSaveFileName (
		HWND	hwndParent,
		LPSTR	pszPrompt,
		LPSTR	pszFile)
{
	PGPError		err		= kPGPError_NoErr;

	OPENFILENAME	ofn;
	CHAR			szFilter[64];
	CHAR*			p;

	LoadString (g_hinst, IDS_TEXTFILEFILTER, szFilter, sizeof(szFilter));
	while (p = strrchr (szFilter, '@')) *p = '\0';

	ofn.lStructSize			= sizeof(OPENFILENAME);
	ofn.hwndOwner			= hwndParent;
	ofn.hInstance			= g_hinst;
	ofn.lpstrFilter			= szFilter;
	ofn.lpstrCustomFilter	= NULL;
	ofn.nMaxCustFilter		= 0;
	ofn.nFilterIndex		= 1;
	ofn.lpstrFile			= pszFile;
	ofn.nMaxFile			= MAX_PATH;
	ofn.lpstrFileTitle		= NULL;
	ofn.nMaxFileTitle		= 0;
	ofn.lpstrInitialDir		= NULL;
	ofn.lpstrTitle			= pszPrompt;
	ofn.Flags				=	OFN_HIDEREADONLY|
								OFN_NOCHANGEDIR|
								OFN_OVERWRITEPROMPT;
	ofn.nFileOffset			= 0;
	ofn.nFileExtension		= 0;
	ofn.lpstrDefExt			= "";
	ofn.lCustData			= 0;

	if (!GetSaveFileName (&ofn))
		err = kPGPError_UserAbort;

	return err;
}

//	____________________________________
//
//	save log info to a text file

PGPError
PNSaveLogToFile (
		HWND	hwnd)
{
	PGPError	err					= kPGPError_NoErr;

	CHAR		sz[256];
	CHAR		szFile[MAX_PATH];

	LoadString (g_hinst, IDS_LOGFILEPROMPT, sz, sizeof(sz));
	LoadString (g_hinst, IDS_DEFAULTLOGFILENAME, szFile, sizeof(szFile));
	err = sGetSaveFileName (hwnd, sz, szFile);

	if (IsntPGPError (err))
	{
		LoadString (g_hinst, IDS_LOGFILEHEADER, sz, sizeof(sz));
		err = sSaveListViewToFile (
				GetDlgItem (hwnd, IDC_LOGLIST), szFile, sz);
	}

	return err;
}

//	____________________________________
//
//	save status info to a text file

PGPError
PNSaveStatusToFile (
		HWND	hwnd)
{
	PGPError	err		= kPGPError_NoErr;

	CHAR		sz[256];
	CHAR		szFile[MAX_PATH];

	LoadString (g_hinst, IDS_STATUSFILEPROMPT, sz, sizeof(sz));
	LoadString (g_hinst, IDS_DEFAULTSTATUSFILENAME, szFile, sizeof(szFile));
	err = sGetSaveFileName (hwnd, sz, szFile);

	if (IsntPGPError (err))
	{
		LoadString (g_hinst, IDS_STATUSFILEHEADER, sz, sizeof(sz));
		err = sSaveListViewToFile (
				GetDlgItem (hwnd, IDC_STATUSLIST), szFile, sz);
	}

	return err;
}



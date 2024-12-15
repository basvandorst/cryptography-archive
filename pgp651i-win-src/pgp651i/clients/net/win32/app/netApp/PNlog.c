/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PNlog.c - PGPnet log window message procedure
	

	$Id: PNlog.c,v 1.53.4.1 1999/06/08 16:32:07 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#include "PGPnetApp.h"
#include "pgpNetIPC.h"
#include "pgpNetPaths.h"
#include "pgpNetLog.h"
#include "PGPnetHelp.h"

#include "pgpFileSpec.h"
#include "pgpEndianConversion.h"

#define TOP_LIST_OFFSET			36
#define HOR_LIST_OFFSET			8
#define HOR_CONTROL_OFFSET		8
#define BOTTOM_LIST_OFFSET		50
#define BOTTOM_BUTTON_OFFSET	10
#define BUTTON_HEIGHT			24
#define BUTTON_WIDTH			64
#define BUTTON_SPACING			8

#define TIMECOLWIDTH			120
#define EVENTCOLWIDTH			50
#define IPCOLWIDTH				90
#define MESSAGECOLWIDTH			600

#define COL_TIME				0
#define COL_EVENT				1
#define COL_IP					2
#define COL_MESSAGE				3

#define FILTER_SERVICE			0x01
#define FILTER_IKE				0x02
#define FILTER_PGP				0x04
#define FILTER_SYSTEM			0x08
#define FILTER_IPSEC			0x10
#define FILTER_ALL				0x1F

#define SORT_ASCENDING			1
#define SORT_DESCENDING			-1

static UINT				g_uBottomListOffset		= BOTTOM_LIST_OFFSET;
static UINT				g_uOnOffBoxHeight		= ONOFFBOX_HEIGHT;
static UINT				g_uOnOffBoxWidth		= ONOFFBOX_WIDTH;
static UINT				g_uButtonHeight			= BUTTON_HEIGHT;
static UINT				g_uButtonWidth			= BUTTON_WIDTH;

extern HINSTANCE		g_hinst;
extern INT				g_iMinWindowWidth;
extern PGPContextRef	g_context;
extern APPOPTIONSSTRUCT	g_AOS;
extern CHAR				g_szHelpFile[];
extern BOOL				g_bReadOnly;	

// prototypes
	PGPError 
PGPGetIPsecErrorString(
	PGPError			theError,
	PGPSize				bufferSize,
	char *				theString );

	PGPError 
PGPGetIkeAlertString(
	PGPikeAlert			theAlert,
	PGPSize				bufferSize,
	char *				theString );

	PGPError 
PGPGetIkeInternalAlertString(
	PGPikeInternalAlert	theAlert,
	PGPSize				bufferSize,
	char *				theString );

	PGPError 
PGPnetGetServiceErrorString(
	PGPnetServiceError	theError,
	PGPSize				bufferSize,
	char *				theString );

static struct {
	INT		iTimeWidth;
	INT		iEventWidth;
	INT		iIPWidth;
	INT		iMessageWidth;
	INT		iSortField;
	INT		iSortDirection;
	UINT	uFilterFlags;
} s_LogPrefs;

static INT s_iCurrentIndex		= 0;

static DWORD aLogIds[] = {			// Help IDs
	IDC_LOGLIST,		IDH_PNLOG_LIST,
	IDC_PGPNETON,		IDH_PNMAIN_PGPNETENABLE,
	IDC_PGPNETOFF,		IDH_PNMAIN_PGPNETDISABLE,
	IDC_SAVE,			IDH_PNLOG_SAVE,
	IDC_REMOVE,			IDH_PNLOG_CLEAR,
	IDC_FILTERSERVICE,	IDH_PNLOG_FILTERSERVICE,
	IDC_FILTERIKE,		IDH_PNLOG_FILTERIKE,
	IDC_FILTERIPSEC,	IDH_PNLOG_FILTERIPSEC,
	IDC_FILTERPGP,		IDH_PNLOG_FILTERPGP,
	IDC_FILTERSYSTEM,	IDH_PNLOG_FILTERSYSTEM,
	0,0
};


//	___________________________________________
//
//	Get widths of columns from prefs file

static VOID 
sGetColumnPreferences (VOID)
{
	PGPError			err;
	PGPPrefRef			prefref;
	PGPSize				size;
	PVOID				pvoid;

	s_LogPrefs.iTimeWidth		= TIMECOLWIDTH;
	s_LogPrefs.iEventWidth		= EVENTCOLWIDTH;
	s_LogPrefs.iIPWidth			= IPCOLWIDTH;
	s_LogPrefs.iMessageWidth	= MESSAGECOLWIDTH;
	s_LogPrefs.iSortField		= COL_TIME;
	s_LogPrefs.iSortDirection	= SORT_ASCENDING;
	s_LogPrefs.uFilterFlags		= FILTER_ALL;

	err = PGPnetOpenPrefs (PGPGetContextMemoryMgr (g_context), &prefref);
	if (IsntPGPError (err))
	{
		err = PGPGetPrefData (prefref, 
					kPGPNetPrefAppWinLogWindowData, &size, &pvoid);

		if (IsntPGPError (err)) 
		{
			if (size == sizeof(s_LogPrefs)) 
			{
				memcpy (&s_LogPrefs, pvoid, sizeof(s_LogPrefs));
			}
			PGPDisposePrefData (prefref, pvoid);
		}
		PGPnetClosePrefs (prefref, FALSE);
	}
}


//	___________________________________________
// 
//	Put window position information in registry

static VOID 
sSetColumnPreferences (
		HWND	hwndList) 
{
	PGPError			err;
	PGPPrefRef			prefref;

	err = PGPnetOpenPrefs (PGPGetContextMemoryMgr (g_context), &prefref);
	if (IsntPGPError (err))
	{
		s_LogPrefs.iTimeWidth = 
					ListView_GetColumnWidth (hwndList, COL_TIME);
		s_LogPrefs.iEventWidth = 
					ListView_GetColumnWidth (hwndList, COL_EVENT);
		s_LogPrefs.iIPWidth = 
					ListView_GetColumnWidth (hwndList, COL_IP);
		s_LogPrefs.iMessageWidth = 
					ListView_GetColumnWidth (hwndList, COL_MESSAGE);

		PGPSetPrefData (prefref, 
					kPGPNetPrefAppWinLogWindowData, 
					sizeof(s_LogPrefs), &s_LogPrefs);

		PGPnetClosePrefs (prefref, TRUE);
	}

}

//	____________________________________
//
//	initialize the list control

static VOID
sInitLogList (
		HWND	hwnd)
{
	HWND			hwndList;
	LV_COLUMN		lvc;
	CHAR			sz[64];

	hwndList = GetDlgItem (hwnd, IDC_LOGLIST);

	sGetColumnPreferences ();

	lvc.mask = LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
	lvc.pszText = sz;

	LoadString (g_hinst, IDS_TIMECOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_LogPrefs.iTimeWidth;
	lvc.iSubItem = COL_TIME;
	ListView_InsertColumn (hwndList, COL_TIME, &lvc);

	LoadString (g_hinst, IDS_EVENTCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_LogPrefs.iEventWidth;
	lvc.iSubItem = COL_EVENT;
	ListView_InsertColumn (hwndList, COL_EVENT, &lvc);

	LoadString (g_hinst, IDS_IPADDRESSFIELD, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_LogPrefs.iIPWidth;
	lvc.iSubItem = COL_IP;
	ListView_InsertColumn (hwndList, COL_IP, &lvc);

	LoadString (g_hinst, IDS_MESSAGECOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_LogPrefs.iMessageWidth;
	lvc.iSubItem = COL_MESSAGE;
	ListView_InsertColumn (hwndList, COL_MESSAGE, &lvc);
}


//	____________________________________
//
//	size the list control and move the buttons

static VOID
sSizeControls (
		HWND	hwnd,
		WORD	wXsize,
		WORD	wYsize)
{
	HDWP	hdwp; 

	// size the tree control accordingly
	hdwp = BeginDeferWindowPos (6); 
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_LOGLIST), NULL, 
		HOR_LIST_OFFSET, TOP_LIST_OFFSET, 
		wXsize - (2*HOR_LIST_OFFSET), 
		wYsize - g_uBottomListOffset - TOP_LIST_OFFSET -1,
		SWP_NOZORDER);  
	
	// move the "On/Off" box
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_ONOFFBOX), NULL, 
		HOR_LIST_OFFSET, 
		wYsize - BOTTOM_ONOFF_OFFSET - g_uOnOffBoxHeight, 
		g_uOnOffBoxWidth, g_uOnOffBoxHeight, 
		SWP_NOZORDER); 

	// move the "On" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_PGPNETON), NULL, 
		HOR_CONTROL_OFFSET + (g_uOnOffBoxWidth/5), 
		wYsize - g_uBottomListOffset + 
					TOP_ONOFFBOX_OFFSET + (g_uOnOffBoxHeight/3),
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "Off" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_PGPNETOFF), NULL, 
		HOR_CONTROL_OFFSET + 3*(g_uOnOffBoxWidth/5), 
		wYsize - g_uBottomListOffset + 
					TOP_ONOFFBOX_OFFSET + (g_uOnOffBoxHeight/3),
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "clear" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_REMOVE), NULL, 
		wXsize - HOR_LIST_OFFSET - g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "save" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_SAVE), NULL, 
		wXsize - HOR_LIST_OFFSET - BUTTON_SPACING - 2*g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE); 

	EndDeferWindowPos(hdwp);
}

//	____________________________________
//
//	construct IP address string

static VOID
sConstructIPAddress (
		PGPnetLogEvent*	pevent,
		LPSTR			szIP,
		UINT			uLen)
{
	struct		in_addr ia;

	szIP[0] = '\0';
	if (pevent->ipaddress != 0)
	{
		ia.S_un.S_addr = pevent->ipaddress;
		lstrcpyn (szIP, inet_ntoa (ia), uLen);
	}
}

//	____________________________________
//
//	construct IPSEC message string

static VOID
sConstructIpsecErrorMessage (
		PGPnetLogEvent*	pevent,
		LPSTR			szMessage,
		UINT			uLen)
{
	CHAR		szFmt [64];
	CHAR		szStr [128];
	PGPError	err;

	// try to get IPsec error string
	err = PGPGetIPsecErrorString (
				pevent->info.pgperror.error, sizeof(szStr), szStr);

	if (IsPGPError (err))
	{
		LoadString (g_hinst, IDS_UNKNOWNIPSECERRORFMT, szFmt, sizeof(szFmt));
		wsprintf (szStr, szFmt, pevent->info.pgperror.error);
	}

	lstrcpyn (szMessage, szStr, uLen);
}

//	____________________________________
//
//	construct PGPError message string

static VOID
sConstructPGPErrorMessage (
		PGPnetLogEvent*	pevent,
		LPSTR			szMessage,
		UINT			uLen)
{
	CHAR		szFmt [64];
	CHAR		szStr [128];
	PGPError	err;
#if PGP_DEBUG
	CHAR		szMsg [256];
#endif

	pevent->info.pgperror.filename[
			sizeof(pevent->info.pgperror.filename)-1] = '\0';

	err = PGPGetErrorString (
				pevent->info.pgperror.error, sizeof(szStr), szStr);

	if (IsPGPError (err))
	{
		LoadString (g_hinst, IDS_UNKNOWNPGPERRORFMT, szFmt, sizeof(szFmt));
		wsprintf (szStr, szFmt, pevent->info.pgperror.error);
	}

#if PGP_DEBUG
	LoadString (g_hinst, IDS_PGPERRORFMT, szFmt, sizeof(szFmt));
	wsprintf (szMsg, szFmt,  
				pevent->info.pgperror.filename,
				pevent->info.pgperror.lineno,
				szStr);
	lstrcpyn (szMessage, szMsg, uLen);
#else
	lstrcpyn (szMessage, szStr, uLen);
#endif

}

//	____________________________________
//
//	construct IKE alert message string

static VOID
sConstructIkeAlertMessage (
		PGPnetLogEvent*	pevent,
		LPSTR			szMessage,
		UINT			uLen)
{
	BOOL		bFormatted		= FALSE;
	CHAR		szFmt [64];
	CHAR		szStr [128];
	CHAR		szMsg [256];
	PGPError	err;

	if (pevent->info.ikealert.alert == kPGPike_AL_None)
		err = PGPGetIkeInternalAlertString (
				(PGPikeInternalAlert)pevent->info.ikealert.value, 
				sizeof(szStr), szStr);
	else
	{
		err = PGPGetIkeAlertString (
				pevent->info.ikealert.alert, sizeof(szStr), szStr);
		bFormatted = TRUE;
	}

	if (IsPGPError (err))
	{
		LoadString (g_hinst, IDS_UNKNOWNIKEALERTFMT, szFmt, sizeof(szFmt));
		wsprintf (szStr, szFmt, pevent->info.ikealert.alert);
		bFormatted = TRUE;
	}

	if (bFormatted) 
	{
		if (pevent->info.ikealert.remoteGenerated)
			LoadString (g_hinst, IDS_IKEALERTREMOTEFMT, szFmt, sizeof(szFmt));
		else
			LoadString (g_hinst, IDS_IKEALERTLOCALFMT, szFmt, sizeof(szFmt));

		wsprintf (szMsg, szFmt, szStr, "");
		lstrcpyn (szMessage, szMsg, uLen);
	}
	else
		lstrcpyn (szMessage, szStr, uLen);
}

//	____________________________________
//
//	construct system error message string

static VOID
sConstructSystemErrorMessage (
		PGPnetLogEvent*	pevent,
		LPSTR			szMessage,
		UINT			uLen)
{
	CHAR		szFmt [64];
	CHAR		szStr [128];
	INT			ireturn;
#if PGP_DEBUG
	CHAR		szMsg [256];
#endif

	pevent->info.syserror.filename[
			sizeof(pevent->info.pgperror.filename)-1] = '\0';

	ireturn = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						pevent->info.syserror.error,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						szStr, sizeof(szStr), NULL);

	if (ireturn == 0)
	{
		LoadString (g_hinst, IDS_UNKNOWNSYSTEMERRORFMT, szFmt, sizeof(szFmt));
		wsprintf (szStr, szFmt, pevent->info.syserror.error);
	}
	else
	{
		// remove newlines
		LPSTR	p;
		while (p = strchr (szStr, 0x0A)) *p = ' ';
		while (p = strchr (szStr, 0x0D)) *p = ' ';
	}

#if PGP_DEBUG
	LoadString (g_hinst, IDS_SYSTEMERRORFMT, szFmt, sizeof(szFmt));
	wsprintf (szMsg, szFmt, 
				pevent->info.syserror.filename,
				pevent->info.syserror.lineno,
				szStr);
	lstrcpyn (szMessage, szMsg, uLen);
#else
	lstrcpyn (szMessage, szStr, uLen);
#endif

}

//	____________________________________
//
//	construct Service error message string

static VOID
sConstructServiceErrorMessage (
		PGPnetLogEvent*	pevent,
		LPSTR			szMessage,
		UINT			uLen)
{
	CHAR		szFmt [64];
	CHAR		szStr [128];
	PGPError	err;
#if PGP_DEBUG
	CHAR		szMsg [256];
#endif

	err = PGPnetGetServiceErrorString (
				pevent->info.svcerror.error, sizeof(szStr), szStr);
	if (IsPGPError (err))
	{
		LoadString (g_hinst, IDS_UNKNOWNSVCERRORFMT, szFmt, sizeof(szFmt));
		wsprintf (szStr, szFmt, pevent->info.svcerror.error);
	}
	else if (pevent->info.svcerror.error == kPGPnetSrvcError_AuthenticatedKey)
	{
		CHAR	szID[kPGPMaxKeyIDStringSize +1];

		PGPGetKeyIDString (&pevent->info.svcerror.keyID, 
				kPGPKeyIDString_Abbreviated, szID);
		lstrcat (szStr, szID);
	}

#if PGP_DEBUG
	pevent->info.svcerror.filename[
			sizeof(pevent->info.svcerror.filename)-1] = '\0';

	LoadString (g_hinst, IDS_SERVICEERRORFMT, szFmt, sizeof(szFmt));
	wsprintf (szMsg, szFmt,  
				pevent->info.svcerror.filename,
				pevent->info.svcerror.lineno,
				szStr);
	lstrcpyn (szMessage, szMsg, uLen);
#else
	lstrcpyn (szMessage, szStr, uLen);
#endif

}

//	____________________________________
//
//	construct unknown event message string

static VOID
sConstructUnknownEventMessage (
		PGPnetLogEvent*	pevent,
		LPSTR			szMessage,
		UINT			uLen)
{
	CHAR	szFmt[64];
	CHAR	szMsg[128];

	LoadString (g_hinst, IDS_UNKNOWNEVENTMESSAGEFMT, szFmt, sizeof(szFmt));
	wsprintf (szMsg, szFmt, pevent->typeOfEvent);

	lstrcpyn (szMessage, szMsg, uLen);
}

//	____________________________________
//
//	log the incoming event to the window

static VOID
sLogEvent (
		HWND				hwndList,
		PGPnetLogEvent*		pevent)
{
	LV_ITEM		lvi;
	CHAR		sz[128];

	switch (pevent->typeOfEvent) {
	case kPGPnetLogEvent_None :
		return;

	case kPGPnetLogEvent_PGPError :
		if (!(s_LogPrefs.uFilterFlags & FILTER_PGP))
			return;
		break;

	case kPGPnetLogEvent_PGPike :
		if (!(s_LogPrefs.uFilterFlags & FILTER_IKE))
			return;
		break;

	case kPGPnetLogEvent_IPSEC :
		if (!(s_LogPrefs.uFilterFlags & FILTER_IPSEC))
			return;
		break;

	case kPGPnetLogEvent_System :
		if (!(s_LogPrefs.uFilterFlags & FILTER_SYSTEM))
			return;
		break;

	case kPGPnetLogEvent_Service :
		if (!(s_LogPrefs.uFilterFlags & FILTER_SERVICE))
			return;
		break;

#if PGP_DEBUG
	case kPGPnetLogEvent_DebugText :
		break;
#endif //PGP_DEBUG

	default :
		return;
	}

	PNConvertTimeToString (pevent->timeOfEvent, sz, sizeof(sz)); 

	lvi.mask = LVIF_TEXT|LVIF_PARAM;
	lvi.iItem = s_iCurrentIndex;
	lvi.iSubItem = COL_TIME;
	lvi.lParam = s_iCurrentIndex;
	lvi.pszText = sz;

	s_iCurrentIndex = ListView_InsertItem (hwndList, &lvi) +1;

	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = COL_IP;
	sConstructIPAddress (pevent, sz, sizeof(sz));
	ListView_SetItem (hwndList, &lvi);

	lvi.iSubItem = COL_EVENT;

	switch (pevent->typeOfEvent) {

	case kPGPnetLogEvent_DebugText :
		LoadString (g_hinst, IDS_DEBUGEVENT, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		lvi.iSubItem = COL_MESSAGE;
		lstrcpyn (sz, pevent->info.debug.sztext, 
								sizeof(pevent->info.debug.sztext));
		ListView_SetItem (hwndList, &lvi);
		break;

	case kPGPnetLogEvent_PGPError :
		LoadString (g_hinst, IDS_PGPERROREVENT, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		lvi.iSubItem = COL_MESSAGE;
		sConstructPGPErrorMessage (pevent, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);
		break;

	case kPGPnetLogEvent_PGPike :
		LoadString (g_hinst, IDS_IKEALERTEVENT, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		lvi.iSubItem = COL_MESSAGE;
		sConstructIkeAlertMessage (pevent, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);
		break;

	case kPGPnetLogEvent_IPSEC :
		LoadString (g_hinst, IDS_IPSECERROR, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		lvi.iSubItem = COL_MESSAGE;
		sConstructIpsecErrorMessage (pevent, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);
		break;

	case kPGPnetLogEvent_System :
		LoadString (g_hinst, IDS_SYSTEMERROREVENT, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		lvi.iSubItem = COL_MESSAGE;
		sConstructSystemErrorMessage (pevent, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);
		break;

	case kPGPnetLogEvent_Service :
		LoadString (g_hinst, IDS_SERVICEERROREVENT, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		lvi.iSubItem = COL_MESSAGE;
		sConstructServiceErrorMessage (pevent, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);
		break;

	default :
		LoadString (g_hinst, IDS_UNKNOWNEVENT, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		lvi.iSubItem = COL_MESSAGE;
		sConstructUnknownEventMessage (pevent, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);
		break;
	}
}

//	____________________________________
//
//	load pre-existing log events into window

static VOID
sLoadLogFile (
		HWND			hwnd)
{
	CHAR				sz[128];
	CHAR				szFile[256];
	LV_ITEM				lvi;
	HWND				hwndList;
	PFLFileSpecRef		filespecLog;
	PGPnetLogRef		logRef;
	PGPUInt32			numberEvents, u;
	PGPnetLogEvent*		pEvents;

	lvi.pszText = sz;
	hwndList = GetDlgItem (hwnd, IDC_LOGLIST);

	PGPnetGetLogFileFullPath (szFile, sizeof(szFile));
	PFLNewFileSpecFromFullPath (
			PGPGetContextMemoryMgr (g_context), szFile, &filespecLog);

	PGPnetOpenLogFile (filespecLog, FALSE, FALSE, &logRef);
	PFLFreeFileSpec (filespecLog);

	PGPnetReadLogData (logRef, &numberEvents, (void**)&pEvents);

	for (u=0; u<numberEvents; u++) 
		sLogEvent (hwndList, &pEvents[u]);

	if (pEvents) {
		EnableWindow (GetDlgItem (hwnd, IDC_REMOVE), !g_bReadOnly);
		EnableWindow (GetDlgItem (hwnd, IDC_SAVE), TRUE);
		PGPnetFreeLogData (pEvents);
	}
	else
	{
		EnableWindow (GetDlgItem (hwnd, IDC_REMOVE), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDC_SAVE), FALSE);
	}


	PGPnetCloseLogFile (logRef);

	InvalidateRect (hwndList, NULL, FALSE);
}

//	____________________________________
//
//	compare entries for sorting purposes

static INT CALLBACK 
sCompareFunction (
			  LPARAM lParam1, 
			  LPARAM lParam2, 	
			  LPARAM lParamSort)
{
	HWND			hwndList	= (HWND)lParamSort;
	INT				iCmp		= 0;
	LV_FINDINFO		lvfi;
	LV_ITEM			lvi;
	INT				index1, index2;
	CHAR			sz1[32], sz2[32];

	// if we're not sorting on time, we need to get the string
	if (s_LogPrefs.iSortField != COL_TIME)
	{
		lvfi.flags = LVFI_PARAM;
		lvfi.lParam = lParam1;
		index1 = ListView_FindItem (hwndList, -1, &lvfi);
		lvfi.lParam = lParam2;
		index2 = ListView_FindItem (hwndList, -1, &lvfi);

		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = s_LogPrefs.iSortField;

		lvi.iItem = index1;
		lvi.pszText = sz1;
		lvi.cchTextMax = sizeof(sz1);
		ListView_GetItem (hwndList, &lvi);

		lvi.iItem = index2;
		lvi.pszText = sz2;
		lvi.cchTextMax = sizeof(sz2);
		ListView_GetItem (hwndList, &lvi);

		// if we're sorting on IP address, convert the string to a number
		if (s_LogPrefs.iSortField == COL_IP)
		{
			UINT	u, uIP1, uIP2;

			u = inet_addr (sz1);
			PGPUInt32ToEndian (u, kPGPBigEndian, (PGPByte*)&uIP1);

			u = inet_addr (sz2);
			PGPUInt32ToEndian (u, kPGPBigEndian, (PGPByte*)&uIP2);

			if (uIP1 < uIP2) 
				iCmp = -1;
			if (uIP1 > uIP2)
				iCmp =  1;
		}

		// otherwise just compare the strings
		else
			iCmp = lstrcmp (sz1, sz2);
	}

	// if we don't have an order yet, sort on the sequence in the list
	// (i.e. the time)
	if (iCmp == 0)
	{
		if (lParam1 < lParam2)
			iCmp = -1;
		else if (lParam1 > lParam2)
			iCmp = 1;
	}

	return (s_LogPrefs.iSortDirection * iCmp);
}

//	____________________________________
//
//	ask user if OK to clear log, then ask service to do it

static VOID
sClearLogFile (
		HWND	hwnd)
{
	if (PNMessageBox (hwnd, IDS_CAPTION, IDS_CLEARLOGPROMPT,
					MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
	{
		if (!PNSendServiceMessage (PGPNET_M_APPMESSAGE,
					(WPARAM)PGPNET_CLEARLOGFILE, (LPARAM)hwnd))
		{
			PNMessageBox (hwnd, IDS_CAPTION, IDS_SERVICENOTAVAIL,
							MB_OK|MB_ICONSTOP);
		}
	}
}


//	____________________________________
//
//	process log message from service

static VOID
sProcessLogCopyDataMessage (
		HWND			hwnd,
		PCOPYDATASTRUCT	pcds)
{
	HWND	hwndList;

	hwndList = GetDlgItem (hwnd, IDC_LOGLIST);

	switch (pcds->dwData) {
	case PGPNET_LOGUPDATE :
		if ((pcds->cbData == sizeof(PGPnetLogEvent)) &&
			(pcds->lpData != NULL))
		{
			sLogEvent (hwndList, (PGPnetLogEvent*)(pcds->lpData));
			ListView_SortItems (hwndList, sCompareFunction, hwndList);
			InvalidateRect (hwndList, NULL, FALSE);

			EnableWindow (GetDlgItem (hwnd, IDC_REMOVE), !g_bReadOnly);
			EnableWindow (GetDlgItem (hwnd, IDC_SAVE), TRUE);
		}
		break;
	}
}


//	____________________________________
//
//	process log message from service

static VOID
sProcessLogMessage (
		HWND			hwnd,
		WPARAM			wParam)
{
	HWND	hwndList;

	hwndList = GetDlgItem (hwnd, IDC_STATUSLIST);

	switch (wParam) {
	case PGPNET_CLEARLOGFILE :
		ListView_DeleteAllItems (GetDlgItem (hwnd, IDC_LOGLIST));
		s_iCurrentIndex = 0;
		EnableWindow (GetDlgItem (hwnd, IDC_REMOVE), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDC_SAVE), FALSE);
		break;
	}
}


//	____________________________________
//
//	set filter flags based on checkboxes

static VOID
sSetFilterFlags (
		HWND			hwnd)
{
	s_LogPrefs.uFilterFlags = 0;

	if (IsDlgButtonChecked (hwnd, IDC_FILTERSERVICE) == BST_CHECKED)
		s_LogPrefs.uFilterFlags |= FILTER_SERVICE;

	if (IsDlgButtonChecked (hwnd, IDC_FILTERIKE) == BST_CHECKED)
		s_LogPrefs.uFilterFlags |= FILTER_IKE;

	if (IsDlgButtonChecked (hwnd, IDC_FILTERPGP) == BST_CHECKED)
		s_LogPrefs.uFilterFlags |= FILTER_PGP;

	if (IsDlgButtonChecked (hwnd, IDC_FILTERSYSTEM) == BST_CHECKED)
		s_LogPrefs.uFilterFlags |= FILTER_SYSTEM;

	if (IsDlgButtonChecked (hwnd, IDC_FILTERIPSEC) == BST_CHECKED)
		s_LogPrefs.uFilterFlags |= FILTER_IPSEC;
}


//	____________________________________
//
//	dialog proc of log dialog

BOOL CALLBACK
PNLogDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		RECT	rc;

		// setup control size values
		GetWindowRect (GetDlgItem (hwnd, IDC_SAVE), &rc);
		g_uButtonWidth = rc.right - rc.left;
		g_uButtonHeight = rc.bottom - rc.top;
		g_uBottomListOffset	= 2 * (rc.bottom - rc.top);

		GetWindowRect (GetDlgItem (hwnd, IDC_ONOFFBOX), &rc);
		g_uOnOffBoxWidth = rc.right - rc.left;
		g_uOnOffBoxHeight = rc.bottom - rc.top;

		GetWindowRect (GetDlgItem (hwnd, IDC_LOGLIST), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		g_iMinWindowWidth = max (g_iMinWindowWidth,
					(rc.right-rc.left) + (6*HOR_TAB_OFFSET));

		sInitLogList (hwnd);
		PNSendServiceMessage (PGPNET_M_APPMESSAGE,
					(WPARAM)PGPNET_ENABLELOGMESSAGES, (LPARAM)hwnd);
		sLoadLogFile (hwnd);
		ListView_SortItems (GetDlgItem (hwnd, IDC_LOGLIST),
						sCompareFunction, GetDlgItem (hwnd, IDC_LOGLIST));

		// initialize filter checkboxes
		if (s_LogPrefs.uFilterFlags & FILTER_SERVICE)
			CheckDlgButton (hwnd, IDC_FILTERSERVICE, BST_CHECKED);

		if (s_LogPrefs.uFilterFlags & FILTER_IKE)
			CheckDlgButton (hwnd, IDC_FILTERIKE, BST_CHECKED);

		if (s_LogPrefs.uFilterFlags & FILTER_IPSEC)
			CheckDlgButton (hwnd, IDC_FILTERIPSEC, BST_CHECKED);

		if (s_LogPrefs.uFilterFlags & FILTER_PGP)
			CheckDlgButton (hwnd, IDC_FILTERPGP, BST_CHECKED);

		if (s_LogPrefs.uFilterFlags & FILTER_SYSTEM)
			CheckDlgButton (hwnd, IDC_FILTERSYSTEM, BST_CHECKED);

		// disable "On/Off" buttons if we don't have privileges
		if (g_bReadOnly)
		{
			EnableWindow (GetDlgItem (hwnd, IDC_PGPNETON), FALSE);
			EnableWindow (GetDlgItem (hwnd, IDC_PGPNETOFF), FALSE);
		}
		return FALSE;
	}

	case PGPNET_M_ACTIVATEPAGE :
		// init on/off buttons
		if (g_AOS.pnconfig.bPGPnetEnabled)
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETON);
		else
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETOFF);
		break;

	case PGPNET_M_SETFOCUS :
		SetFocus (GetDlgItem (hwnd, IDC_LOGLIST));
		break;

	case WM_SIZE :
		sSizeControls (hwnd, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_DESTROY :
		sSetColumnPreferences (GetDlgItem (hwnd, IDC_LOGLIST));
		PNSendServiceMessage (PGPNET_M_APPMESSAGE,
					(WPARAM)PGPNET_DISABLELOGMESSAGES, (LPARAM)hwnd);
		break;

	case PGPNET_M_INTERNALCOPYDATA :
		sProcessLogCopyDataMessage (hwnd, (PCOPYDATASTRUCT)lParam);
		break;

	case PGPNET_M_APPMESSAGE :
		sProcessLogMessage (hwnd, wParam);
		break;

	case WM_NOTIFY :
		if (wParam == IDC_LOGLIST)
		{
			NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)lParam;
			if (pnmlv->hdr.code == LVN_COLUMNCLICK)
			{
				if (s_LogPrefs.iSortField == pnmlv->iSubItem)
				{
					s_LogPrefs.iSortDirection *= -1;
				}
				else 
				{
					s_LogPrefs.iSortField = pnmlv->iSubItem;
					s_LogPrefs.iSortDirection = SORT_ASCENDING;
				}
				ListView_SortItems (GetDlgItem (hwnd, IDC_LOGLIST),
						sCompareFunction, GetDlgItem (hwnd, IDC_LOGLIST));
				InvalidateRect (
						GetDlgItem (hwnd, IDC_LOGLIST), NULL, FALSE);
			}
		}
		break;

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPSTR) aLogIds); 
		return TRUE; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aLogIds); 
		return TRUE; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_SAVE :
			if (IsWindowEnabled (GetDlgItem (hwnd, IDC_SAVE)))
			{
				PNSaveLogToFile (hwnd);
				SetFocus (GetDlgItem (hwnd, IDC_SAVE));
			}
			break;

		case IDC_REMOVE :
			if (IsWindowEnabled (GetDlgItem (hwnd, IDC_REMOVE)))
			{
				sClearLogFile (hwnd);
				SetFocus (GetDlgItem (hwnd, IDC_REMOVE));
			}
			break;

		case IDC_PGPNETOFF :
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETOFF);
			PNSaveOnOffButtons (hwnd, g_context);
			SetFocus (GetDlgItem (hwnd, IDC_PGPNETOFF));
			break;

		case IDC_PGPNETON :
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETON);
			PNSaveOnOffButtons (hwnd, g_context);
			SetFocus (GetDlgItem (hwnd, IDC_PGPNETON));
			break;

		case IDC_TOGGLEFILTERSERVICE :
			if (IsDlgButtonChecked (hwnd, IDC_FILTERSERVICE) == BST_CHECKED)
				CheckDlgButton (hwnd, IDC_FILTERSERVICE, BST_UNCHECKED);
			else
				CheckDlgButton (hwnd, IDC_FILTERSERVICE, BST_CHECKED);
			PostMessage (hwnd, WM_COMMAND, IDC_FILTERSERVICE, 0);
			break;

		case IDC_TOGGLEFILTERIKE :
			if (IsDlgButtonChecked (hwnd, IDC_FILTERIKE) == BST_CHECKED)
				CheckDlgButton (hwnd, IDC_FILTERIKE, BST_UNCHECKED);
			else
				CheckDlgButton (hwnd, IDC_FILTERIKE, BST_CHECKED);
			PostMessage (hwnd, WM_COMMAND, IDC_FILTERIKE, 0);
			break;

		case IDC_TOGGLEFILTERIPSEC :
			if (IsDlgButtonChecked (hwnd, IDC_FILTERIPSEC) == BST_CHECKED)
				CheckDlgButton (hwnd, IDC_FILTERIPSEC, BST_UNCHECKED);
			else
				CheckDlgButton (hwnd, IDC_FILTERIPSEC, BST_CHECKED);
			PostMessage (hwnd, WM_COMMAND, IDC_FILTERIPSEC, 0);
			break;

		case IDC_TOGGLEFILTERPGP :
			if (IsDlgButtonChecked (hwnd, IDC_FILTERPGP) == BST_CHECKED)
				CheckDlgButton (hwnd, IDC_FILTERPGP, BST_UNCHECKED);
			else
				CheckDlgButton (hwnd, IDC_FILTERPGP, BST_CHECKED);
			PostMessage (hwnd, WM_COMMAND, IDC_FILTERPGP, 0);
			break;

		case IDC_TOGGLEFILTERSYSTEM :
			if (IsDlgButtonChecked (hwnd, IDC_FILTERSYSTEM) == BST_CHECKED)
				CheckDlgButton (hwnd, IDC_FILTERSYSTEM, BST_UNCHECKED);
			else
				CheckDlgButton (hwnd, IDC_FILTERSYSTEM, BST_CHECKED);
			PostMessage (hwnd, WM_COMMAND, IDC_FILTERSYSTEM, 0);
			break;

		case IDC_FILTERSERVICE :
		case IDC_FILTERIKE :
		case IDC_FILTERIPSEC :
		case IDC_FILTERPGP :
		case IDC_FILTERSYSTEM :
			sSetFilterFlags (hwnd);
			ListView_DeleteAllItems (GetDlgItem (hwnd, IDC_LOGLIST));
			s_iCurrentIndex = 0;
			sLoadLogFile (hwnd);
			ListView_SortItems (GetDlgItem (hwnd, IDC_LOGLIST),
						sCompareFunction, GetDlgItem (hwnd, IDC_LOGLIST));
			break;
		}
		return 0;
	}

	return FALSE;
}



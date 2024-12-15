/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	DurationControl.c - PGPnet duration custom control
	

	$Id: DurationControl.c,v 1.7 1999/04/01 14:53:54 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "DurationControl.h"

#define MAXTEXTLEN	8

// Duration control window extra data.  Must include window handle field.
typedef struct _DCWndData {
	HWND	hwnd;
	HWND	hwndParent;
	INT		iID;
	DWORD	dwStyle;

	HWND	hwndEdit[NUM_FIELDS];
	WNDPROC	wpOrigEditProc[NUM_FIELDS];

	UINT	uValue[NUM_FIELDS];
	INT		iRangeLo[NUM_FIELDS];
	INT		iRangeHi[NUM_FIELDS];
	CHAR	szText[NUM_FIELDS][MAXTEXTLEN];

	HWND	hwndText[NUM_FIELDS];
	WNDPROC	wpOrigTextProc[NUM_FIELDS];

	HWND	hwndUpDown;

	HFONT	hFont;

	INT		iFocused;
	INT		iPrevFocused;
	BOOL	bFirstKey;
	INT		iHeight;

	ULONG	ulDuration;
	ULONG	ulMinDuration;
	ULONG	ulMaxDuration;
} DCWndData;


#define RETURN_VALUE		0
#define NORMAL_PROCESSING	1
#define MOVE_LEFT			2
#define MOVE_RIGHT			3

#define VERTICAL_OFFSET		2
#define HORIZONTAL_OFFSET	2

#define IDC_EDITDAYS		1000
#define IDC_EDITHOURS		1001
#define IDC_EDITMINS		1002
#define IDC_TEXTDAYS		1010
#define IDC_TEXTHOURS		1011
#define IDC_TEXTMINS		1012

#define Duration_GetPtr(hwnd)  (DCWndData*)GetWindowLong((hwnd), 0)
#define Duration_SetPtr(hwnd, pDC)  \
			(DCWndData*)SetWindowLong((hwnd), 0, (LONG)(pDC))

//	____________________________________
//
//	Private memory allocation routine

static VOID* 
sAlloc (LONG size) 
{
	VOID* p;
	p = malloc (size);
	if (p) 
		memset (p, 0, size);
	
	return p;
}

//	____________________________________
//
//	Private memory deallocation routine

static VOID 
sFree (VOID* p) 
{
	if (p) 
		free (p);
}

//	____________________________________
//
//	size and position the children

static VOID 
sSetControls (
		HWND		hwnd,
		DCWndData*	pDC)
{
	HDC		hdc			= GetDC (hwnd);

	INT		iXpos, iXposLast;
	INT		i, iWidth;
	HDWP	hdwp;
	CHAR	sz[8];
	HFONT	hfontOrig;
	SIZE	size;
	RECT	rc;

	hfontOrig	= SelectObject (hdc, pDC->hFont);
		
	hdwp = BeginDeferWindowPos (7);

	// days
	iXpos = HORIZONTAL_OFFSET;
	i = wsprintf (sz, "%ld", pDC->iRangeHi[DAYS]);
	GetTextExtentPoint32 (hdc, sz, i, &size);
	iWidth = size.cx;
	DeferWindowPos (hdwp, pDC->hwndEdit[DAYS], NULL, 
					iXpos, VERTICAL_OFFSET, iWidth, pDC->iHeight,
					SWP_NOZORDER);

	iXpos += iWidth;
	i = lstrlen (&pDC->szText[DAYS][0]);
	GetTextExtentPoint32 (hdc, &pDC->szText[DAYS][0], i, &size);
	iWidth = size.cx;
	DeferWindowPos (hdwp, pDC->hwndText[DAYS], NULL, 
					iXpos, VERTICAL_OFFSET, iWidth, pDC->iHeight,
					SWP_NOZORDER);
	SetWindowText (pDC->hwndText[DAYS], &pDC->szText[DAYS][0]);

	// hours
	iXpos += iWidth;
	i = wsprintf (sz, "%ld", pDC->iRangeHi[HOURS]);
	GetTextExtentPoint32 (hdc, sz, i, &size);
	iWidth = size.cx;
	DeferWindowPos (hdwp, pDC->hwndEdit[HOURS], NULL, 
					iXpos, VERTICAL_OFFSET, iWidth, pDC->iHeight,
					SWP_NOZORDER);

	iXpos += iWidth;
	i = lstrlen (&pDC->szText[HOURS][0]);
	GetTextExtentPoint32 (hdc, &pDC->szText[HOURS][0], i, &size);
	iWidth = size.cx;
	DeferWindowPos (hdwp, pDC->hwndText[HOURS], NULL, 
					iXpos, VERTICAL_OFFSET, iWidth, pDC->iHeight,
					SWP_NOZORDER);
	SetWindowText (pDC->hwndText[HOURS], &pDC->szText[HOURS][0]);

	// minutes
	iXpos += iWidth;
	i = wsprintf (sz, "%ld", pDC->iRangeHi[MINS]);
	GetTextExtentPoint32 (hdc, sz, i, &size);
	iWidth = size.cx;
	DeferWindowPos (hdwp, pDC->hwndEdit[MINS], NULL, 
					iXpos, VERTICAL_OFFSET, iWidth, pDC->iHeight,
					SWP_NOZORDER);

	iXpos += iWidth;
	i = lstrlen (&pDC->szText[MINS][0]);
	GetTextExtentPoint32 (hdc, &pDC->szText[MINS][0], i, &size);
	iWidth = size.cx;
	DeferWindowPos (hdwp, pDC->hwndText[MINS], NULL, 
					iXpos, VERTICAL_OFFSET, iWidth, pDC->iHeight,
					SWP_NOZORDER);
	SetWindowText (pDC->hwndText[MINS], &pDC->szText[MINS][0]);

	iXposLast = iXpos + iWidth + HORIZONTAL_OFFSET;
	GetClientRect (hwnd, &rc);
	iWidth = GetSystemMetrics (SM_CXVSCROLL);
	iXpos = rc.right - iWidth;
	if (iXposLast > iXpos)
	{
		iXpos = iXposLast;
		iWidth = rc.right - iXpos;
	}
	DeferWindowPos (hdwp, pDC->hwndUpDown, NULL, 
					iXpos, 0, iWidth, rc.bottom,
					SWP_NOZORDER);

	EndDeferWindowPos (hdwp);

	SelectObject (hdc, hfontOrig);
	ReleaseDC (hwnd, hdc);
}

//	____________________________________
//
//	compute duration from individual values

static VOID 
sComputeDuration (
		DCWndData*	pDC)
{
	pDC->ulDuration  = pDC->uValue[DAYS]  * 86400;
	pDC->ulDuration += pDC->uValue[HOURS] * 3600;
	pDC->ulDuration += pDC->uValue[MINS]  * 60;
}

//	____________________________________
//
//	validate and display current duration value

static VOID 
sDisplayDuration (
		DCWndData*	pDC)
{
	CHAR	sz[4];
	ULONG	ul;
	ULONG	ulSec;

	if (pDC->ulDuration < pDC->ulMinDuration)
		pDC->ulDuration = pDC->ulMinDuration;

	if (pDC->ulDuration > pDC->ulMaxDuration)
		pDC->ulDuration = pDC->ulMaxDuration;

	ulSec = pDC->ulDuration;
	ul = ulSec / 86400;
	pDC->uValue[DAYS] = ul;
	wsprintf (sz, "%ld", ul);
	SetWindowText (pDC->hwndEdit[DAYS], sz);

	ulSec -= ul * 86400;
	ul = ulSec / 3600;
	pDC->uValue[HOURS] = ul;
	wsprintf (sz, "%2.2ld", ul);
	SetWindowText (pDC->hwndEdit[HOURS], sz);

	ulSec -= ul * 3600;
	ul = ulSec / 60;
	pDC->uValue[MINS] = ul;
	wsprintf (sz, "%2.2ld", ul);
	SetWindowText (pDC->hwndEdit[MINS], sz);
}

//	____________________________________
//
//	change the duration by delta

static VOID 
sScrollUnit (
		DCWndData*	pDC,
		INT			iDelta)
{
	INT		index;
	INT		i;

	if (pDC->iFocused < 0) 
	{
		index = pDC->iPrevFocused;
		SetFocus (pDC->hwndEdit[index]);
	}
	else
		index = pDC->iFocused;

	i = (INT)(pDC->uValue[index]) + iDelta;

	if (i < pDC->iRangeLo[index]) 
		i = pDC->iRangeHi[index];
	if (i > pDC->iRangeHi[index]) 
		i = pDC->iRangeLo[index];

	pDC->uValue[index] = i;

	sComputeDuration (pDC);
	sDisplayDuration (pDC);

	SendMessage (pDC->hwndEdit[index], EM_SETSEL, 0, (LPARAM)-1);
}

//	____________________________________
//
//	set the unit value to that specified

static VOID 
sSetUnit (
		DCWndData*	pDC,
		INT			i)
{
	INT		index	= pDC->iFocused;

	if (i < pDC->iRangeLo[index]) 
		i = pDC->iRangeHi[index];
	if (i > pDC->iRangeHi[index]) 
		i = pDC->iRangeLo[index];

	pDC->uValue[index] = i;

	sComputeDuration (pDC);
	sDisplayDuration (pDC);

	SendMessage (pDC->hwndEdit[index], EM_SETSEL, 0, (LPARAM)-1);
}

//	____________________________________
//
//	common "edit" edit box subclass procedures

static UINT 
sCommonEditProc (
		HWND		hwnd, 
		DCWndData*	pDC,
		INT			index,
		UINT		uMsg, 
		WPARAM		wParam, 
		LPARAM		lParam,
		LRESULT*	plReturn) 
{
	INT		i;
	UINT	uRetVal;

	*plReturn = 0;
	uRetVal = NORMAL_PROCESSING;

	switch (uMsg) {
	case WM_GETDLGCODE :
		*plReturn = DLGC_WANTARROWS|DLGC_WANTCHARS;
		return RETURN_VALUE;

	case WM_CONTEXTMENU :
	case WM_MOUSEMOVE :
	case WM_LBUTTONDBLCLK :
	case WM_SETCURSOR :
		return RETURN_VALUE;

	case WM_LBUTTONDOWN :
		if (pDC->iFocused == index)
			return RETURN_VALUE;
		break;

	case WM_KILLFOCUS :
		pDC->iPrevFocused = index;
		pDC->iFocused = -1;
		break;

	case WM_SETFOCUS :
		PostMessage (hwnd, EM_SETSEL, 0, (LPARAM)-1);
		pDC->iFocused = index;
		pDC->bFirstKey = TRUE;
		break;

	case WM_KEYDOWN :
		switch (wParam) {
		case VK_HOME :
			sSetUnit (pDC, pDC->iRangeLo[index]);
			uRetVal = RETURN_VALUE;
			break;

		case VK_END :
			sSetUnit (pDC, pDC->iRangeHi[index]);
			uRetVal = RETURN_VALUE;
			break;

		case VK_RIGHT :
			uRetVal = MOVE_RIGHT;
			break;

		case VK_LEFT :
			uRetVal = MOVE_LEFT;
			break;

		case VK_DOWN :
			sScrollUnit (pDC, -1);
			uRetVal = RETURN_VALUE;
			break;

		case VK_UP :
			sScrollUnit (pDC, 1);
			uRetVal = RETURN_VALUE;
			break;

		case VK_BACK :
		case VK_DELETE :
			uRetVal = RETURN_VALUE;
			break;
		}
		break;

	case WM_CHAR :
		switch (wParam) {
		case VK_BACK :
			uRetVal = RETURN_VALUE;
			break;

		case '0' :
		case '1' :
		case '2' :
		case '3' :
		case '4' :
		case '5' :
		case '6' :
		case '7' :
		case '8' :
		case '9' :
			i = wParam - '0';
			if (pDC->bFirstKey)
			{
				pDC->uValue[index] = i;
				pDC->bFirstKey = FALSE;
			}
			else
			{
				i += pDC->uValue[index] * 10;
				if ((i >= pDC->iRangeLo[index]) &&
					(i <= pDC->iRangeHi[index]))
					pDC->uValue[index] = i;
				pDC->bFirstKey = TRUE;
			}
			sComputeDuration (pDC);
			sDisplayDuration (pDC);
			SendMessage (pDC->hwndEdit[index], EM_SETSEL, 0, (LPARAM)-1);
			uRetVal = RETURN_VALUE;
			break;
		}

		break;
	}

	return uRetVal;
}

//	______________________________________________
//
//	"edit" edit box subclass procedures

static LRESULT APIENTRY 
sEditDaysProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	DCWndData*	pDC = Duration_GetPtr(GetParent (hwnd));
	LRESULT		lReturn;
	UINT		u;
	
	u = sCommonEditProc (hwnd, pDC, DAYS, uMsg, wParam, lParam, &lReturn);
	switch (u) {
	case RETURN_VALUE :
		return lReturn;

	case NORMAL_PROCESSING :
		return CallWindowProc (pDC->wpOrigEditProc[DAYS], 
										hwnd, uMsg, wParam, lParam); 
	case MOVE_RIGHT :
		SetFocus (pDC->hwndEdit[HOURS]);
		return 0;

	case MOVE_LEFT :
		SetFocus (pDC->hwndEdit[MINS]);
		return 0;

	default :
		return 0;
	}
} 

static LRESULT APIENTRY 
sEditHoursProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	DCWndData*	pDC = Duration_GetPtr(GetParent (hwnd));
	LRESULT		lReturn;
	UINT		u;
	
	u = sCommonEditProc (hwnd, pDC, HOURS, uMsg, wParam, lParam, &lReturn);

	switch (u) {
	case RETURN_VALUE :
		return lReturn;

	case NORMAL_PROCESSING :
		return CallWindowProc (pDC->wpOrigEditProc[HOURS], 
										hwnd, uMsg, wParam, lParam); 
	case MOVE_RIGHT :
		SetFocus (pDC->hwndEdit[MINS]);
		return 0;

	case MOVE_LEFT :
		SetFocus (pDC->hwndEdit[DAYS]);
		return 0;

	default :
		return 0;
	}
} 

static LRESULT APIENTRY 
sEditMinsProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	DCWndData*	pDC = Duration_GetPtr(GetParent (hwnd));
	LRESULT		lReturn;
	UINT		u;
	
	u = sCommonEditProc (hwnd, pDC, MINS, uMsg, wParam, lParam, &lReturn);

	switch (u) {
	case RETURN_VALUE :
		return lReturn;

	case NORMAL_PROCESSING :
		return CallWindowProc (pDC->wpOrigEditProc[MINS], 
										hwnd, uMsg, wParam, lParam); 
	case MOVE_RIGHT :
		SetFocus (pDC->hwndEdit[DAYS]);
		return 0;

	case MOVE_LEFT :
		SetFocus (pDC->hwndEdit[HOURS]);
		return 0;

	default :
		return 0;
	}
} 

//	______________________________________________
//
//	static text edit box subclass procedures

static LRESULT APIENTRY 
sText0Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	DCWndData* pDC;

	switch (uMsg) {
	case WM_PAINT :
	case WM_ENABLE :
	case EM_SETREADONLY :
	case WM_NCHITTEST :
	case WM_SETTEXT :
	case WM_HELP :
		pDC = Duration_GetPtr(GetParent (hwnd));
		return CallWindowProc(pDC->wpOrigTextProc[DAYS], 
								hwnd, uMsg, wParam, lParam); 

	case WM_MOUSEACTIVATE :
		pDC = Duration_GetPtr(GetParent (hwnd));
		SetFocus (pDC->hwndEdit[DAYS]);
		return 0;

	default : 
		return 0;
	}
} 

static LRESULT APIENTRY 
sText1Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	DCWndData* pDC;

	switch (uMsg) {
	case WM_PAINT :
	case WM_ENABLE :
	case EM_SETREADONLY :
	case WM_NCHITTEST :
	case WM_SETTEXT :
	case WM_HELP :
		pDC = Duration_GetPtr(GetParent (hwnd));
		return CallWindowProc(pDC->wpOrigTextProc[HOURS], 
								hwnd, uMsg, wParam, lParam); 

	case WM_MOUSEACTIVATE :
		pDC = Duration_GetPtr(GetParent (hwnd));
		SetFocus (pDC->hwndEdit[HOURS]);
		return 0;

	default : 
		return 0;
	}
} 

static LRESULT APIENTRY 
sText2Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	DCWndData* pDC;

	switch (uMsg) {
	case WM_PAINT :
	case WM_ENABLE :
	case EM_SETREADONLY :
	case WM_NCHITTEST :
	case WM_SETTEXT :
	case WM_HELP :
		pDC = Duration_GetPtr(GetParent (hwnd));
		return CallWindowProc(pDC->wpOrigTextProc[MINS], 
								hwnd, uMsg, wParam, lParam); 

	case WM_MOUSEACTIVATE :
		pDC = Duration_GetPtr(GetParent (hwnd));
		SetFocus (pDC->hwndEdit[MINS]);
		return 0;

	default : 
		return 0;
	}
} 

//	____________________________________
//
//  Message processing function for Duration class

static LRESULT WINAPI 
sDurationMsgProc (
		HWND	hwnd, 
		UINT	msg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	DCWndData* pDC = Duration_GetPtr(hwnd);

    if (pDC == NULL) {
        if (msg == WM_NCCREATE) 
		{
			pDC = (DCWndData*)sAlloc (sizeof(DCWndData));
			if (pDC == NULL) return 0L;
			Duration_SetPtr (hwnd, pDC);
        }
        else 
        	return DefWindowProc (hwnd, msg, wParam, lParam);
    }
	
    if (msg == WM_NCDESTROY) 
	{
		sFree (pDC);
        pDC = NULL;
        Duration_SetPtr (hwnd, NULL);
    }

	switch (msg) {

	case WM_CREATE :
	{
		LPCREATESTRUCT	lpcs		= (LPCREATESTRUCT)lParam;

		pDC->hwnd		= hwnd;
		pDC->hwndParent = lpcs->hwndParent;
		pDC->iID		= (INT)(lpcs->hMenu); 
		pDC->dwStyle	= lpcs->style & ES_READONLY;
		pDC->hFont		= GetStockObject (DEFAULT_GUI_FONT);

		pDC->iFocused		= -1;
		pDC->iPrevFocused	= 0;
		pDC->bFirstKey		= TRUE;
		pDC->iHeight		= lpcs->cy;

		pDC->iRangeLo[DAYS]		= 0;
		pDC->iRangeLo[HOURS]	= 0;
		pDC->iRangeLo[MINS]		= 0;

		pDC->iRangeHi[DAYS]		= 99;
		pDC->iRangeHi[HOURS]	= 23;
		pDC->iRangeHi[MINS]		= 59;

		pDC->ulDuration			= 0;
		pDC->ulMinDuration		= 0;
		pDC->ulMaxDuration		= 8553600 + 82800 + 3540; // 99d + 23h + 59m

		lstrcpy (&pDC->szText[DAYS][0], "d, ");
		lstrcpy (&pDC->szText[HOURS][0], "h, ");
		lstrcpy (&pDC->szText[MINS][0], "m");

		// create the up/down control
		pDC->hwndUpDown = CreateWindowEx (0L, UPDOWN_CLASS, "",
			WS_CHILD|WS_VISIBLE|pDC->dwStyle,
			0, 0, 0, 0, hwnd, NULL, 0, NULL);

		// create edit controls
		// days
		pDC->hwndEdit[DAYS] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_NUMBER|ES_RIGHT|pDC->dwStyle, 
			0, 0, 0, 0, hwnd, (HMENU)IDC_EDITDAYS, 0, NULL);
		SendMessage (pDC->hwndEdit[DAYS], 
						WM_SETFONT, (WPARAM)pDC->hFont, FALSE);
		SendMessage (pDC->hwndEdit[DAYS], 
						EM_SETLIMITTEXT, 2, 0);
		pDC->wpOrigEditProc[DAYS] = 
						(WNDPROC) SetWindowLong (pDC->hwndEdit[DAYS],
								GWL_WNDPROC, (LONG)sEditDaysProc); 

		pDC->hwndText[DAYS] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_VISIBLE|ES_LEFT|pDC->dwStyle, 
			0, 0, 0, 0, hwnd, (HMENU)IDC_TEXTDAYS, 0, NULL);
		SendMessage (pDC->hwndText[DAYS], 
						WM_SETFONT, (WPARAM)pDC->hFont, FALSE);
		pDC->wpOrigTextProc[DAYS] = 
						(WNDPROC) SetWindowLong (pDC->hwndText[DAYS],
								GWL_WNDPROC, (LONG)sText0Proc); 

		// hours
		pDC->hwndEdit[HOURS] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_NUMBER|ES_RIGHT|pDC->dwStyle, 
			0, 0, 0, 0, hwnd, (HMENU)IDC_EDITHOURS, 0, NULL);
		SendMessage (pDC->hwndEdit[HOURS], 
						WM_SETFONT, (WPARAM)pDC->hFont, FALSE);
		SendMessage (pDC->hwndEdit[HOURS], 
						EM_SETLIMITTEXT, 2, 0);
		pDC->wpOrigEditProc[HOURS] = 
						(WNDPROC) SetWindowLong (pDC->hwndEdit[HOURS],
								GWL_WNDPROC, (LONG)sEditHoursProc); 

		pDC->hwndText[HOURS] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_VISIBLE|ES_LEFT|pDC->dwStyle, 
			0, 0, 0, 0, hwnd, (HMENU)IDC_TEXTHOURS, 0, NULL);
		SendMessage (pDC->hwndText[HOURS], 
						WM_SETFONT, (WPARAM)pDC->hFont, FALSE);
		pDC->wpOrigTextProc[HOURS] = 
						(WNDPROC) SetWindowLong (pDC->hwndText[HOURS],
								GWL_WNDPROC, (LONG)sText1Proc); 

		// minutes
		pDC->hwndEdit[MINS] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_NUMBER|ES_RIGHT|pDC->dwStyle, 
			0, 0, 0, 0, hwnd, (HMENU)IDC_EDITMINS, 0, NULL);
		SendMessage (pDC->hwndEdit[MINS], 
						WM_SETFONT, (WPARAM)pDC->hFont, FALSE);
		SendMessage (pDC->hwndEdit[MINS], 
						EM_SETLIMITTEXT, 2, 0);
		pDC->wpOrigEditProc[MINS] = 
						(WNDPROC) SetWindowLong (pDC->hwndEdit[MINS],
								GWL_WNDPROC, (LONG)sEditMinsProc); 

		pDC->hwndText[MINS] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_VISIBLE|ES_LEFT|pDC->dwStyle, 
			0, 0, 0, 0, hwnd, (HMENU)IDC_TEXTMINS, 0, NULL);
		SendMessage (pDC->hwndText[MINS], 
						WM_SETFONT, (WPARAM)pDC->hFont, FALSE);
		pDC->wpOrigTextProc[MINS] = 
						(WNDPROC) SetWindowLong (pDC->hwndText[MINS],
								GWL_WNDPROC, (LONG)sText2Proc); 

		sSetControls (hwnd, pDC);

		break;
	}

	case WM_GETDLGCODE :
		return DLGC_WANTMESSAGE;
		
	case WM_NOTIFY :
	{
		LPNMHDR	pnmhdr = (LPNMHDR)lParam;

		if (pnmhdr->hwndFrom == pDC->hwndUpDown)
		{
			LPNMUPDOWN pnmud = (LPNMUPDOWN)lParam;
			sScrollUnit (pDC, -1*pnmud->iDelta);
		}
		break;
	}

	case WM_PAINT :
		// paint background, if necessary
		if (pDC->dwStyle & (WS_DISABLED | ES_READONLY))
		{
			PAINTSTRUCT	ps;
			HDC			hdc;
			RECT		rc;

			hdc = BeginPaint (hwnd, &ps);
			GetClientRect (hwnd, &rc);
			FillRect (hdc, &rc, (HBRUSH)(COLOR_3DFACE+1));
			EndPaint (hwnd, &ps);
		}
		break;

	case WM_SETFOCUS :
		SetFocus (pDC->hwndEdit[pDC->iPrevFocused]);
		break;

	case WM_DESTROY :
		SetWindowLong (pDC->hwndText[DAYS], 
							GWL_WNDPROC, (LONG)pDC->wpOrigTextProc[DAYS]); 
		SetWindowLong (pDC->hwndText[HOURS], 
							GWL_WNDPROC, (LONG)pDC->wpOrigTextProc[HOURS]); 
		SetWindowLong (pDC->hwndText[MINS],
							GWL_WNDPROC, (LONG)pDC->wpOrigTextProc[MINS]);
		
		DestroyWindow (pDC->hwndText[DAYS]);
		DestroyWindow (pDC->hwndText[HOURS]);
		DestroyWindow (pDC->hwndText[MINS]);

		SetWindowLong (pDC->hwndEdit[DAYS], 
							GWL_WNDPROC, (LONG)pDC->wpOrigEditProc[DAYS]); 
		SetWindowLong (pDC->hwndEdit[HOURS], 
							GWL_WNDPROC, (LONG)pDC->wpOrigEditProc[HOURS]); 
		SetWindowLong (pDC->hwndEdit[MINS], 
							GWL_WNDPROC, (LONG)pDC->wpOrigEditProc[MINS]); 
		
		DestroyWindow (pDC->hwndEdit[DAYS]);
		DestroyWindow (pDC->hwndEdit[HOURS]);
		DestroyWindow (pDC->hwndEdit[MINS]);

		DestroyWindow (pDC->hwndUpDown);
		break;

	case EM_SETREADONLY :
		if (wParam)
			pDC->dwStyle |= ES_READONLY;
		else
			pDC->dwStyle &= ~ES_READONLY;

		SendMessage (pDC->hwndText[DAYS], msg, wParam, lParam);
		SendMessage (pDC->hwndText[HOURS], msg, wParam, lParam);
		SendMessage (pDC->hwndText[MINS], msg, wParam, lParam);

		SendMessage (pDC->hwndEdit[DAYS], msg, wParam, lParam);
		SendMessage (pDC->hwndEdit[HOURS], msg, wParam, lParam);
		SendMessage (pDC->hwndEdit[MINS], msg, wParam, lParam);

		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_HELP :
	{
		LPHELPINFO lphi = (LPHELPINFO)lParam;

		lphi->iCtrlId = pDC->iID;
		lphi->hItemHandle = pDC->hwnd;
		break;
	}

	case WM_ENABLE :
		if (wParam)
		{
			pDC->dwStyle &= ~WS_DISABLED;
			EnableWindow (pDC->hwndUpDown, TRUE);
		}
		else
		{
			pDC->dwStyle |= WS_DISABLED;
			EnableWindow (pDC->hwndUpDown, FALSE);
		}

		SendMessage (pDC->hwndText[DAYS], msg, wParam, lParam);
		SendMessage (pDC->hwndText[HOURS], msg, wParam, lParam);
		SendMessage (pDC->hwndText[MINS], msg, wParam, lParam);

		SendMessage (pDC->hwndEdit[DAYS], msg, wParam, lParam);
		SendMessage (pDC->hwndEdit[HOURS], msg, wParam, lParam);
		SendMessage (pDC->hwndEdit[MINS], msg, wParam, lParam);

		InvalidateRect (hwnd, NULL, TRUE);
		return 0;

	case PGP_DCM_SETDURATION :
		pDC->ulDuration = lParam;
		sDisplayDuration (pDC);
		break;

	case PGP_DCM_GETDURATION :
		*(PULONG)lParam = pDC->ulDuration;
		break;

	case PGP_DCM_SETMINDURATION :
		pDC->ulMinDuration = (ULONG)lParam;
		break;

	case PGP_DCM_SETMAXDURATION :
		pDC->ulMaxDuration = (ULONG)lParam;
		break;

	case PGP_DCM_SETTEXT :
		lstrcpyn (&pDC->szText[wParam][0], (LPSTR)lParam, MAXTEXTLEN);
		sSetControls (hwnd, pDC);
		InvalidateRect (hwnd, NULL, TRUE);
		break;
	} 
	return (DefWindowProc (hwnd, msg, wParam, lParam));
}

//	____________________________________
//
//  load and initialize control

VOID WINAPI 
InitPGPDurationControl (VOID) 
{
	WNDCLASS  wc;
	
	InitCommonControls ();

	// register new window class
	wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC; // Class style(s).
	wc.lpfnWndProc = (WNDPROC) sDurationMsgProc; 
	wc.cbClsExtra = 0;	                        // No per-class extra data.
	wc.cbWndExtra = sizeof (DCWndData*);		// pointer to extra data 
												//		structure
	wc.hInstance = 0;	
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1); // Background color
	wc.lpszMenuName = NULL;						// No menu
	wc.lpszClassName = WC_PGPDURATION;			// Name used in CreateWindow
	RegisterClass (&wc);

}



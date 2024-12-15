/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	IPAddressControl.c - PGPnet ip address custom control
	

	$Id: IPAddressControl.c,v 1.9 1999/04/01 14:53:55 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "IPAddressControl.h"

///>>> these shouldn't be here for a generalized control,
///>>> but they're needed here for getting error message strings
#include "resource.h"
extern HINSTANCE	g_hinst;
///<<<


// IPAddress window extra data.  Must include window handle field.
typedef struct _IPWndData {
	HWND	hwnd;
	HWND	hwndParent;
	INT		iID;
	DWORD	dwStyle;
	DWORD	dwFlags;

	HWND	hwndEdit[4];
	WNDPROC	wpOrigEditProc[4];
	INT		iRangeLo[4];
	INT		iRangeHi[4];
	BOOL	bValid[4];

	HWND	hwndDot[3];
	WNDPROC	wpOrigDotProc[3];

	HFONT	hFont;
} IPWndData;

#define IPFLAG_DISABLED		0x0001

#define RETURN_VALUE		0
#define NORMAL_PROCESSING	1
#define MOVE_LEFT			2
#define MOVE_RIGHT			3
#define MOVE_RIGHT_SELECT	4

#define VERTICAL_OFFSET		2

#define IDC_EDIT0			1000
#define IDC_EDIT1			1001
#define IDC_EDIT2			1002
#define IDC_EDIT3			1003
#define IDC_DOT0			1004
#define IDC_DOT1			1005
#define IDC_DOT2			1006

#define IPAddress_GetPtr(hwnd)  (IPWndData*)GetWindowLong((hwnd), 0)
#define IPAddress_SetPtr(hwnd, pIP)  \
			(IPWndData*)SetWindowLong((hwnd), 0, (LONG)(pIP))

//	____________________________________
//
// Private memory allocation routine

VOID* 
IPAlloc (LONG size) 
{
	VOID* p;
	p = malloc (size);
	if (p) 
		memset (p, 0, size);
	
	return p;
}

//	____________________________________
//
// Private memory deallocation routine

VOID 
IPFree (VOID* p) 
{
	if (p) 
		free (p);
}


//	______________________________________________
//
//	post error message about number being out of range

static VOID 
sErrorMessage (
		HWND	hwnd,
		INT		iVal,
		INT		iRangeLo,
		INT		iRangeHi)
{
	CHAR	sz1[256];
	CHAR	sz2[256];

	LoadString (g_hinst, IDS_INVALIDIPFIELD, sz1, sizeof(sz1));
	wsprintf (sz2, sz1, iVal, iRangeLo, iRangeHi);

	LoadString (g_hinst, IDS_CAPTION, sz1, sizeof(sz1));

	MessageBox (hwnd, sz2, sz1, MB_OK|MB_ICONEXCLAMATION);
}

//	______________________________________________
//
//	common "edit" edit box subclass procedures

static UINT 
sCommonEditProc (
		HWND		hwnd, 
		IPWndData*	pIP,
		INT			index,
		UINT		uMsg, 
		WPARAM		wParam, 
		LPARAM		lParam,
		LRESULT*	lReturn) 
{
	INT		i, j;
	DWORD	dwStart;
	DWORD	dwEnd;
	POINT	pt;
	CHAR	sz[8];
	UINT	uRetVal;
	NMHDR	nmhdr;

	*lReturn = 0;
	uRetVal = NORMAL_PROCESSING;

	switch (uMsg) {
	case WM_GETDLGCODE :
		*lReturn = DLGC_WANTARROWS|DLGC_WANTCHARS;
		return RETURN_VALUE;

	case WM_KEYDOWN :
		switch (wParam) {
		case VK_DOWN :
			wParam = VK_RIGHT;

		case VK_RIGHT :
			SendMessage (hwnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
			if (dwStart == dwEnd)
			{
				GetCaretPos (&pt);
				i = SendMessage (hwnd, 
						EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));
				if (i == GetWindowTextLength (hwnd)) 
					uRetVal = MOVE_RIGHT;
			}
			break;

		case VK_UP :
			wParam = VK_LEFT;

		case VK_LEFT :
		case VK_BACK :
			SendMessage (hwnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
			if (dwStart == dwEnd)
			{
				GetCaretPos (&pt);
				i = SendMessage (hwnd, 
						EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));
				if (i == 0) 
					uRetVal = MOVE_LEFT;
			}
			break;

		case VK_HOME :
			SetFocus (pIP->hwndEdit[0]);
			SendMessage (pIP->hwndEdit[0], EM_SETSEL, 0, 0);
			uRetVal = RETURN_VALUE;
			break;

		case VK_END :
			SetFocus (pIP->hwndEdit[3]);
			SendMessage (pIP->hwndEdit[3], EM_SETSEL, 3, 3);
			uRetVal = RETURN_VALUE;
			break;
		}
		break;

	case WM_CHAR :
		switch (wParam) {
		case '.' :
			if (GetWindowTextLength (hwnd) > 0)
				uRetVal = MOVE_RIGHT_SELECT;
			else 
				uRetVal = RETURN_VALUE;
			break;

		default :
			CallWindowProc (pIP->wpOrigEditProc[index], 
									hwnd, uMsg, wParam, lParam); 

			GetWindowText (hwnd, sz, sizeof(sz));
			i = atoi (sz);
			if (i > pIP->iRangeHi[index])
			{
				wsprintf (sz, "%i", pIP->iRangeHi[index]);
				SetWindowText (hwnd, sz);
				SendMessage (hwnd, EM_SETSEL, 0, 3);
				sErrorMessage (hwnd, i, 
						pIP->iRangeLo[index], pIP->iRangeHi[index]);
				uRetVal = RETURN_VALUE;
			}
			else if (i < pIP->iRangeLo[index])
			{
				wsprintf (sz, "%i", pIP->iRangeLo[index]);
				SetWindowText (hwnd, sz);
				SendMessage (hwnd, EM_SETSEL, 0, 3);
				sErrorMessage (hwnd, i, 
						pIP->iRangeLo[index], pIP->iRangeHi[index]);
				uRetVal = RETURN_VALUE;
			}
			else 
			{
				if (GetWindowTextLength (hwnd) >= 3)
					uRetVal = MOVE_RIGHT_SELECT;
				else
					uRetVal = RETURN_VALUE;
			}

			if (GetWindowTextLength (hwnd) > 0)
				pIP->bValid[index] = TRUE;
			else
				pIP->bValid[index] = FALSE;

			j = 0;
			for (i=0; i<4; i++)
			{
				if (pIP->bValid[i])
					j++;
			}

			switch (j) {
			case 0 :
				nmhdr.code = PGP_IPN_EMPTYADDRESS;
				break;
			case 1 :
			case 2 :
			case 3 :
				nmhdr.code = PGP_IPN_INVALIDADDRESS;
				break;
			
			case 4 :
				nmhdr.code = PGP_IPN_VALIDADDRESS;
				break;
			}

			nmhdr.hwndFrom = pIP->hwnd;
			nmhdr.idFrom = pIP->iID;
			SendMessage (pIP->hwndParent, 
							WM_NOTIFY, pIP->iID, (LPARAM)&nmhdr);

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
sEdit0Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	IPWndData*	pIP = IPAddress_GetPtr(GetParent (hwnd));
	LRESULT		lReturn;
	UINT		u;
	
	u = sCommonEditProc (hwnd, pIP, 0, uMsg, wParam, lParam, &lReturn);
	switch (u) {
	case RETURN_VALUE :
		return lReturn;

	case NORMAL_PROCESSING :
		return CallWindowProc (pIP->wpOrigEditProc[0], 
										hwnd, uMsg, wParam, lParam); 
	case MOVE_RIGHT :
		SetFocus (pIP->hwndEdit[1]);
		return 0;

	case MOVE_RIGHT_SELECT :
		SetFocus (pIP->hwndEdit[1]);
		SendMessage (pIP->hwndEdit[1], EM_SETSEL, 0, 3);
		return 0;

	case MOVE_LEFT :
		return 0;

	default :
		return 0;
	}
} 

static LRESULT APIENTRY 
sEdit1Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	IPWndData*	pIP = IPAddress_GetPtr(GetParent (hwnd));
	LRESULT		lReturn;
	UINT		u;
	
	u = sCommonEditProc (hwnd, pIP, 1, uMsg, wParam, lParam, &lReturn);

	switch (u) {
	case RETURN_VALUE :
		return lReturn;

	case NORMAL_PROCESSING :
		return CallWindowProc (pIP->wpOrigEditProc[1], 
										hwnd, uMsg, wParam, lParam); 
	case MOVE_RIGHT :
		SetFocus (pIP->hwndEdit[2]);
		return 0;

	case MOVE_RIGHT_SELECT :
		SetFocus (pIP->hwndEdit[2]);
		SendMessage (pIP->hwndEdit[2], EM_SETSEL, 0, 3);
		return 0;

	case MOVE_LEFT :
		SetFocus (pIP->hwndEdit[0]);
		SendMessage (pIP->hwndEdit[0], EM_SETSEL, 3, 3);
		return 0;

	default :
		return 0;
	}
} 

static LRESULT APIENTRY 
sEdit2Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	IPWndData*	pIP = IPAddress_GetPtr(GetParent (hwnd));
	LRESULT		lReturn;
	UINT		u;
	
	u = sCommonEditProc (hwnd, pIP, 2, uMsg, wParam, lParam, &lReturn);

	switch (u) {
	case RETURN_VALUE :
		return lReturn;

	case NORMAL_PROCESSING :
		return CallWindowProc (pIP->wpOrigEditProc[2], 
										hwnd, uMsg, wParam, lParam); 
	case MOVE_RIGHT :
		SetFocus (pIP->hwndEdit[3]);
		return 0;

	case MOVE_RIGHT_SELECT :
		SetFocus (pIP->hwndEdit[3]);
		SendMessage (pIP->hwndEdit[3], EM_SETSEL, 0, 3);
		return 0;

	case MOVE_LEFT :
		SetFocus (pIP->hwndEdit[1]);
		SendMessage (pIP->hwndEdit[1], EM_SETSEL, 3, 3);
		return 0;

	default :
		return 0;
	}
} 

static LRESULT APIENTRY 
sEdit3Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	IPWndData*	pIP = IPAddress_GetPtr(GetParent (hwnd));
	LRESULT		lReturn;
	UINT		u;
	
	u = sCommonEditProc (hwnd, pIP, 3, uMsg, wParam, lParam, &lReturn);

	switch (u) {
	case RETURN_VALUE :
		return lReturn;

	case NORMAL_PROCESSING :
		return CallWindowProc (pIP->wpOrigEditProc[3], 
										hwnd, uMsg, wParam, lParam); 
	case MOVE_RIGHT :
	case MOVE_RIGHT_SELECT :
		return 0;

	case MOVE_LEFT :
		SetFocus (pIP->hwndEdit[2]);
		SendMessage (pIP->hwndEdit[2], EM_SETSEL, 3, 3);
		return 0;

	default :
		return 0;
	}
} 

//	______________________________________________
//
//	"dot" edit box subclass procedures

static LRESULT APIENTRY 
sDot0Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	IPWndData* pIP;

	switch (uMsg) {
	case WM_PAINT :
	case EM_SETREADONLY :
	case WM_ENABLE :
	case WM_HELP :
		pIP = IPAddress_GetPtr(GetParent (hwnd));
		return CallWindowProc(pIP->wpOrigDotProc[0], 
								hwnd, uMsg, wParam, lParam); 

	default : 
		return 0;
	}
} 

static LRESULT APIENTRY 
sDot1Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	IPWndData* pIP;

	switch (uMsg) {
	case WM_PAINT :
	case EM_SETREADONLY :
	case WM_ENABLE :
	case WM_HELP :
		pIP = IPAddress_GetPtr(GetParent (hwnd));
		return CallWindowProc(pIP->wpOrigDotProc[1], 
								hwnd, uMsg, wParam, lParam); 

	default : 
		return 0;
	}
} 

static LRESULT APIENTRY 
sDot2Proc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	IPWndData* pIP;

	switch (uMsg) {
	case WM_PAINT :
	case EM_SETREADONLY :
	case WM_ENABLE :
	case WM_HELP :
		pIP = IPAddress_GetPtr(GetParent (hwnd));
		return CallWindowProc(pIP->wpOrigDotProc[2], 
								hwnd, uMsg, wParam, lParam); 

	default : 
		return 0;
	}
} 

//	____________________________________
//
//  Message processing function for IPAddress class

static LRESULT WINAPI 
sIPAddressMsgProc (
		HWND	hwnd, 
		UINT	msg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	IPWndData* pIP = IPAddress_GetPtr(hwnd);

    if (pIP == NULL) {
        if (msg == WM_NCCREATE) 
		{
			pIP = (IPWndData*)IPAlloc (sizeof(IPWndData));
			if (pIP == NULL) return 0L;
			IPAddress_SetPtr (hwnd, pIP);
        }
        else 
        	return DefWindowProc (hwnd, msg, wParam, lParam);
    }
	
    if (msg == WM_NCDESTROY) 
	{
		IPFree (pIP);
        pIP = NULL;
        IPAddress_SetPtr (hwnd, NULL);
    }

	switch (msg) {

	case WM_CREATE :
	{
		LPCREATESTRUCT	lpcs		= (LPCREATESTRUCT)lParam;
		INT				iDotWidth	= 3;
		INT				iNumWidth	= (lpcs->cx)/4 - iDotWidth;

		INT				iXPos;

		pIP->hwnd		= hwnd;
		pIP->hwndParent = lpcs->hwndParent;
		pIP->iID		= (INT)(lpcs->hMenu); 
		pIP->dwStyle	= lpcs->style & ES_READONLY;
		pIP->dwFlags	= 0;
		pIP->hFont		= GetStockObject (DEFAULT_GUI_FONT);

		pIP->iRangeLo[0] = 0;
		pIP->iRangeLo[1] = 0;
		pIP->iRangeLo[2] = 0;
		pIP->iRangeLo[3] = 0;

		pIP->iRangeHi[0] = 255;
		pIP->iRangeHi[1] = 255;
		pIP->iRangeHi[2] = 255;
		pIP->iRangeHi[3] = 255;

		pIP->bValid[0] = FALSE;
		pIP->bValid[1] = FALSE;
		pIP->bValid[2] = FALSE;
		pIP->bValid[3] = FALSE;

		// create edit controls
		iXPos = 0;
		pIP->hwndEdit[0] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_NUMBER|ES_CENTER|pIP->dwStyle, 
			iXPos, VERTICAL_OFFSET, iNumWidth, lpcs->cy,
			hwnd, (HMENU)IDC_EDIT0, 0, NULL);
		SendMessage (pIP->hwndEdit[0], WM_SETFONT, (WPARAM)pIP->hFont, FALSE);
		SendMessage (pIP->hwndEdit[0], EM_SETLIMITTEXT, 3, 0);
		pIP->wpOrigEditProc[0] = (WNDPROC) SetWindowLong (pIP->hwndEdit[0],
						GWL_WNDPROC, (LONG)sEdit0Proc); 

		iXPos = (lpcs->cx)/4;
		pIP->hwndEdit[1] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_NUMBER|ES_CENTER|pIP->dwStyle, 
			iXPos, VERTICAL_OFFSET, iNumWidth, lpcs->cy,
			hwnd, (HMENU)IDC_EDIT1, 0, NULL);
		SendMessage (pIP->hwndEdit[1], WM_SETFONT, (WPARAM)pIP->hFont, FALSE);
		SendMessage (pIP->hwndEdit[1], EM_SETLIMITTEXT, 3, 0);
		pIP->wpOrigEditProc[1] = (WNDPROC) SetWindowLong (pIP->hwndEdit[1],
						GWL_WNDPROC, (LONG)sEdit1Proc); 

		iXPos = (lpcs->cx)/2;
		pIP->hwndEdit[2] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_NUMBER|ES_CENTER|pIP->dwStyle, 
			iXPos, VERTICAL_OFFSET, iNumWidth, lpcs->cy,
			hwnd, (HMENU)IDC_EDIT2, 0, NULL);
		SendMessage (pIP->hwndEdit[2], WM_SETFONT, (WPARAM)pIP->hFont, FALSE);
		SendMessage (pIP->hwndEdit[2], EM_SETLIMITTEXT, 3, 0);
		pIP->wpOrigEditProc[2] = (WNDPROC) SetWindowLong (pIP->hwndEdit[2],
						GWL_WNDPROC, (LONG)sEdit2Proc); 

		iXPos = 3*(lpcs->cx)/4;
		pIP->hwndEdit[3] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_NUMBER|ES_CENTER|pIP->dwStyle, 
			iXPos, VERTICAL_OFFSET, iNumWidth, lpcs->cy,
			hwnd, (HMENU)IDC_EDIT3, 0, NULL);
		SendMessage (pIP->hwndEdit[3], WM_SETFONT, (WPARAM)pIP->hFont, FALSE);
		SendMessage (pIP->hwndEdit[3], EM_SETLIMITTEXT, 3, 0);
		pIP->wpOrigEditProc[3] = (WNDPROC) SetWindowLong (pIP->hwndEdit[3],
						GWL_WNDPROC, (LONG)sEdit3Proc); 

		// create "dot" controls
		iXPos = (lpcs->cx)/4 - iDotWidth;
		pIP->hwndDot[0] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_VISIBLE|ES_CENTER|pIP->dwStyle, 
			iXPos, VERTICAL_OFFSET, iDotWidth, lpcs->cy,
			hwnd, (HMENU)IDC_DOT0, 0, NULL);
		SendMessage (pIP->hwndDot[0], WM_SETFONT, (WPARAM)pIP->hFont, FALSE);
		SetWindowText (pIP->hwndDot[0], ".");
		pIP->wpOrigDotProc[0] = (WNDPROC) SetWindowLong (pIP->hwndDot[0],
						GWL_WNDPROC, (LONG)sDot0Proc); 

		iXPos = (lpcs->cx)/2 - iDotWidth;
		pIP->hwndDot[1] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_VISIBLE|ES_CENTER|pIP->dwStyle, 
			iXPos, VERTICAL_OFFSET, iDotWidth, lpcs->cy,
			hwnd, (HMENU)IDC_DOT1, 0, NULL);
		SendMessage (pIP->hwndDot[1], WM_SETFONT, (WPARAM)pIP->hFont, FALSE);
		SetWindowText (pIP->hwndDot[1], ".");
		pIP->wpOrigDotProc[1] = (WNDPROC) SetWindowLong (pIP->hwndDot[1],
						GWL_WNDPROC, (LONG)sDot1Proc); 

		iXPos = 3*(lpcs->cx)/4 - iDotWidth;
		pIP->hwndDot[2] = CreateWindowEx (0L, "EDIT", "",
			WS_CHILD|WS_VISIBLE|ES_CENTER|pIP->dwStyle, 
			iXPos, VERTICAL_OFFSET, iDotWidth, lpcs->cy,
			hwnd, (HMENU)IDC_DOT2, 0, NULL);
		SendMessage (pIP->hwndDot[2], WM_SETFONT, (WPARAM)pIP->hFont, FALSE);
		SetWindowText (pIP->hwndDot[2], ".");
		pIP->wpOrigDotProc[2] = (WNDPROC) SetWindowLong (pIP->hwndDot[2],
						GWL_WNDPROC, (LONG)sDot2Proc); 

		break;
	}
	
	case WM_PAINT :
		// paint background, if necessary
		if ((pIP->dwStyle & ES_READONLY) ||
			(pIP->dwFlags & IPFLAG_DISABLED))
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
		SendMessage (pIP->hwndEdit[0], EM_SETSEL, 0, (LPARAM)-1);
		SetFocus (pIP->hwndEdit[0]);
		break;

	case WM_ENABLE :
		if (wParam)
			pIP->dwFlags &= ~IPFLAG_DISABLED;
		else
			pIP->dwFlags |= IPFLAG_DISABLED;

		SendMessage (pIP->hwndDot[0], msg, wParam, lParam);
		SendMessage (pIP->hwndDot[1], msg, wParam, lParam);
		SendMessage (pIP->hwndDot[2], msg, wParam, lParam);

		SendMessage (pIP->hwndEdit[0], msg, wParam, lParam);
		SendMessage (pIP->hwndEdit[1], msg, wParam, lParam);
		SendMessage (pIP->hwndEdit[2], msg, wParam, lParam);
		SendMessage (pIP->hwndEdit[3], msg, wParam, lParam);

		InvalidateRect (hwnd, NULL, TRUE);
		return 0;

	case WM_DESTROY :
		SetWindowLong (pIP->hwndDot[0], 
							GWL_WNDPROC, (LONG)pIP->wpOrigDotProc[0]); 
		SetWindowLong (pIP->hwndDot[1], 
							GWL_WNDPROC, (LONG)pIP->wpOrigDotProc[1]); 
		SetWindowLong (pIP->hwndDot[2], 
							GWL_WNDPROC, (LONG)pIP->wpOrigDotProc[2]);
		
		DestroyWindow (pIP->hwndDot[0]);
		DestroyWindow (pIP->hwndDot[1]);
		DestroyWindow (pIP->hwndDot[2]);

		SetWindowLong (pIP->hwndEdit[0], 
							GWL_WNDPROC, (LONG)pIP->wpOrigEditProc[0]); 
		SetWindowLong (pIP->hwndEdit[1], 
							GWL_WNDPROC, (LONG)pIP->wpOrigEditProc[1]); 
		SetWindowLong (pIP->hwndEdit[2], 
							GWL_WNDPROC, (LONG)pIP->wpOrigEditProc[2]); 
		SetWindowLong (pIP->hwndEdit[3], 
							GWL_WNDPROC, (LONG)pIP->wpOrigEditProc[3]); 
		
		DestroyWindow (pIP->hwndEdit[0]);
		DestroyWindow (pIP->hwndEdit[1]);
		DestroyWindow (pIP->hwndEdit[2]);
		DestroyWindow (pIP->hwndEdit[3]);
		break;

	case WM_HELP :
	{
		LPHELPINFO lphi = (LPHELPINFO)lParam;

		lphi->iCtrlId = pIP->iID;
		lphi->hItemHandle = pIP->hwnd;
		break;
	}

	case EM_SETREADONLY :
		if (wParam)
			pIP->dwStyle |= ES_READONLY;
		else
			pIP->dwStyle &= ~ES_READONLY;

		SendMessage (pIP->hwndDot[0], msg, wParam, lParam);
		SendMessage (pIP->hwndDot[1], msg, wParam, lParam);
		SendMessage (pIP->hwndDot[2], msg, wParam, lParam);

		SendMessage (pIP->hwndEdit[0], msg, wParam, lParam);
		SendMessage (pIP->hwndEdit[1], msg, wParam, lParam);
		SendMessage (pIP->hwndEdit[2], msg, wParam, lParam);
		SendMessage (pIP->hwndEdit[3], msg, wParam, lParam);

		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case PGP_IPM_SETADDRESS :
	{
		CHAR	sz[4];
		INT		i;
		NMHDR	nmhdr;

		i = HIBYTE(HIWORD(lParam));
		wsprintf (sz, "%i", i);
		SetWindowText (pIP->hwndEdit[3], sz);

		i = LOBYTE(HIWORD(lParam));
		wsprintf (sz, "%i", i);
		SetWindowText (pIP->hwndEdit[2], sz);

		i = HIBYTE(LOWORD(lParam));
		wsprintf (sz, "%i", i);
		SetWindowText (pIP->hwndEdit[1], sz);

		i = LOBYTE(LOWORD(lParam));
		wsprintf (sz, "%i", i);
		SetWindowText (pIP->hwndEdit[0], sz);

		for (i=0; i<4; i++) 
			pIP->bValid[i] = TRUE;

		nmhdr.hwndFrom = pIP->hwnd;
		nmhdr.idFrom = pIP->iID;
		nmhdr.code = PGP_IPN_VALIDADDRESS;
		SendMessage (pIP->hwndParent, 
							WM_NOTIFY, pIP->iID, (LPARAM)&nmhdr);
		break;
	}

	case PGP_IPM_CLEARADDRESS :
	{
		NMHDR	nmhdr;
		INT		i;

		SetWindowText (pIP->hwndEdit[3], "");
		SetWindowText (pIP->hwndEdit[2], "");
		SetWindowText (pIP->hwndEdit[1], "");
		SetWindowText (pIP->hwndEdit[0], "");

		for (i=0; i<4; i++) 
			pIP->bValid[i] = FALSE;

		nmhdr.hwndFrom = pIP->hwnd;
		nmhdr.idFrom = pIP->iID;
		nmhdr.code = PGP_IPN_INVALIDADDRESS;
		SendMessage (pIP->hwndParent, 
							WM_NOTIFY, pIP->iID, (LPARAM)&nmhdr);
		break;
	}

	case PGP_IPM_GETADDRESS :
	{
		BOOL	bNotValid = FALSE;
		CHAR	sz[4];
		INT		i;
		DWORD	l;

		if (lParam == 0) 
			break;

		*(PDWORD)lParam = 0;

		for (i=0; i<4; i++) 
			if (!pIP->bValid[i])
				bNotValid = TRUE;

		if (bNotValid)
			break;

		GetWindowText (pIP->hwndEdit[3], sz, 4);
		i = atoi (sz);
		i &= 0xFF;
		l = i;

		GetWindowText (pIP->hwndEdit[2], sz, 4);
		i = atoi (sz);
		i &= 0xFF;
		l <<= 8;
		l |= i;

		GetWindowText (pIP->hwndEdit[1], sz, 4);
		i = atoi (sz);
		i &= 0xFF;
		l <<= 8;
		l |= i;

		GetWindowText (pIP->hwndEdit[0], sz, 4);
		i = atoi (sz);
		i &= 0xFF;
		l <<= 8;
		l |= i;

		*(PDWORD)lParam = l;

		break;
	}

	case PGP_IPM_SETRANGE :
		if (wParam > 3) 
			break;
		pIP->iRangeLo[wParam] = LOWORD(lParam);
		pIP->iRangeHi[wParam] = HIWORD(lParam);
		break;

	} 
	return (DefWindowProc (hwnd, msg, wParam, lParam));
}

//	____________________________________
//
//  load and initialize control

VOID WINAPI 
InitPGPIPAddressControl (VOID) 
{
	WNDCLASS  wc;
	
	InitCommonControls ();

	// register new window class
	wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC; // Class style(s).
	wc.lpfnWndProc = (WNDPROC) sIPAddressMsgProc; 
	wc.cbClsExtra = 0;	                        // No per-class extra data.
	wc.cbWndExtra = sizeof (IPWndData*);		// pointer to extra data 
												//		structure
	wc.hInstance = 0;	
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1); // Background color
	wc.lpszMenuName = NULL;						// No menu
	wc.lpszClassName = WC_PGPIPADDRESS;		// Name used in CreateWindow
	RegisterClass (&wc);

}



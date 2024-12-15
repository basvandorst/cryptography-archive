head	 1.7;
branch   ;
access   ;
symbols  ;
locks    ; strict;
comment  @ * @;


1.7
date	 94.09.18.22.45.21;  author jcooper;  state Exp;
branches ;
next	 1.6;

1.6
date	 94.09.16.00.58.09;  author jcooper;  state Exp;
branches ;
next	 1.5;

1.5
date	 94.08.11.00.09.17;  author jcooper;  state Exp;
branches ;
next	 1.4;

1.4
date	 94.07.25.20.13.40;  author jcooper;  state Exp;
branches ;
next	 1.3;

1.3
date	 94.02.24.21.36.10;  author jcoop;  state Exp;
branches ;
next	 1.2;

1.2
date	 94.01.24.17.40.28;  author jcoop;  state Exp;
branches ;
next	 1.1;

1.1
date	 94.01.16.12.10.59;  author jcoop;  state Exp;
branches ;
next	 ;


desc
@jcoop's coding status window routines
@


1.7
log
@Better handling of status window.  
@
text
@/********************************************************************
 *                                                                  *
 *  MODULE    :  WVCODEWN.C                                         *
 *                                                                  *
 *  PURPOSE   : This file contains the window proc for coding       *
 *		status windows                                      *                                 
 *                                                                  *
 * Author: John S. Cooper (jcooper@@netcom.com)                      *
 *   Date: Sept 30, 1993                                            *
 ********************************************************************/
/* 
 * $Id: wvcodewn.c 1.6 1994/09/16 00:58:09 jcooper Exp $
 * $Log: wvcodewn.c $
 * Revision 1.6  1994/09/16  00:58:09  jcooper
 * always-on-top option, and general cleanup for 92.6
 * 
 * Revision 1.5  1994/08/11  00:09:17  jcooper
 * Enhancements to Mime and article encoding/encoding
 *
 * Revision 1.4  1994/07/25  20:13:40  jcooper
 * execution of decoded files
 *
 * Revision 1.3  1994/02/24  21:36:10  jcoop
 * jcoop changes
 *
 * Revision 1.2  1994/01/24  17:40:28  jcoop
 * 90.2 changes
 *
 * Revision 1.1  1994/01/16  12:10:59  jcoop
 * Initial revision
 *
 */ 
#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------------------------------------------------------------------
 * 	Window Proc for block coding status window
 *	Simple fixed text display 
 *	Associated with TypCoded  (always currentCoded)
 *	Fixed size, only ever one on screen, destroyed on block coding complete
 *	To avoid redrawing the entire status window on each update,
 *	wParam in association with WM_PAINT decides what to draw this time:
 */
long FAR PASCAL
WinVnCodedBlockWndProc (hWnd, message, wParam, lParam)
	HWND hWnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
{
	HDC          hDC;
	HMENU        hMenu;
	PAINTSTRUCT  ps;
	void	DrawStatusFrame (HDC hDC);
	void	DrawStatusName (HDC hDC);
	void	DrawStatusLines (HDC hDC);
	void	DrawStatusBytes (HDC hDC);
	void	DrawStatusSeq (HDC hDC);
	void	DrawStatusActivity (HDC hDC);

	switch (message)
	{
	case WM_CREATE:
		hMenu = GetSystemMenu(hWnd, FALSE);
		AppendMenu(hMenu, MF_SEPARATOR, 0, (LPSTR) NULL);
		AppendMenu(hMenu, MF_STRING, IDM_ALWAYSONTOP, "Always On Top");
		if (BlockCodingStatusAlwaysOnTop) {
		        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			CheckMenuItem(hMenu, IDM_ALWAYSONTOP, MF_BYCOMMAND|MF_CHECKED);
		} else {
		        SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			CheckMenuItem(hMenu, IDM_ALWAYSONTOP, MF_BYCOMMAND|MF_UNCHECKED);
		}
		break;

	case WM_SYSCOMMAND:
		if (wParam == IDM_ALWAYSONTOP) 
		{
			hMenu = GetSystemMenu(hWnd, FALSE);
			BlockCodingStatusAlwaysOnTop = !(GetMenuState(hMenu, IDM_ALWAYSONTOP, 
				MF_BYCOMMAND) & MF_CHECKED);
			if (BlockCodingStatusAlwaysOnTop) {
			        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
				CheckMenuItem(hMenu, IDM_ALWAYSONTOP, MF_BYCOMMAND|MF_CHECKED);
			} else {
			        SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
				CheckMenuItem(hMenu, IDM_ALWAYSONTOP, MF_BYCOMMAND|MF_UNCHECKED);
			}
		}
		else
			return (DefWindowProc (hWnd, message, wParam, lParam));

	case WM_SIZE:
	{
		RECT client;
		if (wParam != SIZE_MINIMIZED)
		{
			GetWindowRect (hWnd, &client);
		        MoveWindow (hWnd, client.left, client.top, STATUSWIDTH,
		               	    STATUSHEIGHT, TRUE);
		}
	      	break;
	}
			
	case WM_PAINT:
		hDC = BeginPaint (hWnd, &ps);
		SetBkColor (hDC, StatusBackgroundColor);
		SetTextColor (hDC, StatusTextColor);
		SelectObject (hDC, hStatusFont);
		if (currentCoded == NULL)
			DrawStatusFrame (hDC);
		else
		{
			DrawStatusFrame (hDC);
			DrawStatusName (hDC);		
			DrawStatusLines (hDC);		
			DrawStatusBytes (hDC);		
			DrawStatusSeq (hDC);		
			DrawStatusActivity (hDC);		
                }
		EndPaint (hWnd, &ps);
		break;

	case WM_CLOSE:
		if (CodingState)
		   MessageBox (hWnd,
		     "Please wait until en/decoding is complete",
		     "Cannot close status window", MB_OK|MB_ICONSTOP);
		else                             
		   DestroyWindow (hWnd);

		break;
	
	default:
		return (DefWindowProc (hWnd, message, wParam, lParam));
	}
	
	return (0);
}

/* ------------------------------------------------------------------------
 *   Routines for drawing coding block status window
 *   2 columns, 3 lines
 *
 *   HTITLE1       HTEXT1        HTITLE2  HTEXT2
 *   !             !             !        !
 *   Retrieving    ___________   Sequence ___________   <- YOFFSET
 *   Lines Read    __________    Activity ___________   <- YOFFSET+YSPACE
 *   Bytes Decoded __________                           <- YOFFSET+2*YSPACE
 */
#define HTITLE1 (3*StatusCharWidth)
#define HTITLE2 (45*StatusCharWidth)
#define HTEXT1	(22*StatusCharWidth)
#define HTEXT2	(57*StatusCharWidth)
#define YOFFSET (StatusLineHeight)
#define YSPACE	(int)(StatusLineHeight*1.5)

void
DrawStatusFrame (HDC hDC)
{
	if (CodingState >= ATTACH_PROCESSING)
	{
	  TextOut (hDC, HTITLE1, YOFFSET, "Reading file", 12);
	  if (CodingState == ATTACH_PROCESSING)
	    TextOut (hDC, HTITLE1, YOFFSET + YSPACE, "Lines encoded", 13);
	  else
	    TextOut (hDC, HTITLE1, YOFFSET + YSPACE, "Lines posted", 12);
	  TextOut (hDC, HTITLE1, YOFFSET + 2*YSPACE, "Bytes read", 10);
	  if (CodingState == ATTACH_POSTING)
		TextOut (hDC, HTITLE2, YOFFSET, "Sequence", 8);
	} 
	else
	{
	  TextOut (hDC, HTITLE1, YOFFSET, "Retrieving", 10);
	  TextOut (hDC, HTITLE1, YOFFSET + YSPACE, "Lines Read", 10);
	  TextOut (hDC, HTITLE1, YOFFSET + 2*YSPACE, "Bytes Decoded", 13);
	  TextOut (hDC, HTITLE2, YOFFSET, "Sequence", 8);
	} 
	TextOut (hDC, HTITLE2, YOFFSET + YSPACE, "Activity", 8);
}

void
DrawStatusName (HDC hDC)
{
	char temp[MAXINTERNALLINE];
	RECT aRect;
		
	if (currentCoded->name[0] == '\0' && currentCoded->ident[0] == '\0')
		strcpy (temp, "Name unknown");
	else
	{
		if (currentCoded->name[0] != '\0')	// len must be <= 20	
		{
			NameWithoutPath (str, currentCoded->name);// cut off any path
			sprintf(temp, "%s", str);
		}
		else
		{
			NameWithoutPath (str, currentCoded->ident);//cut off any path
			sprintf(temp, "%s", str); 
		}
	}

	// be sure to erase old name 
	SetRect (&aRect, HTEXT1, YOFFSET, HTITLE2-1, YOFFSET+YSPACE-1);
	ExtTextOut (hDC, HTEXT1, YOFFSET, ETO_OPAQUE|ETO_CLIPPED, &aRect, 
	            temp, lstrlen(temp), (LPINT)NULL); 
}
void
DrawStatusLines (HDC hDC)
{
	int len;
	RECT aRect;
	
	len = sprintf (str, "%lu", currentCoded->numLines);
	SetRect (&aRect, HTEXT1, YOFFSET+YSPACE, HTITLE2-1, YOFFSET+2*YSPACE-1);
	ExtTextOut (hDC, HTEXT1, YOFFSET+YSPACE, ETO_OPAQUE|ETO_CLIPPED, 
	            &aRect, str, len, (LPINT)NULL); 
}
void
DrawStatusBytes (HDC hDC)
{
	int len;
	RECT aRect;
	
	len = sprintf (str, "%lu", currentCoded->numBytes);
	TextOut (hDC, HTEXT1, YOFFSET + 2*YSPACE, str, len);
	SetRect (&aRect, HTEXT1, YOFFSET+2*YSPACE, HTITLE2-1, YOFFSET+3*YSPACE-1);
	ExtTextOut (hDC, HTEXT1, YOFFSET+2*YSPACE, ETO_OPAQUE|ETO_CLIPPED, 
	            &aRect, str, len, (LPINT)NULL); 
}
void
DrawStatusSeq (HDC hDC)
{
	int len;
	RECT aRect;
	
	if (CodingState == ATTACH_PROCESSING)
		return;
		
	if (currentCoded->sequence == -1)
		len = sprintf (str, "%s", "Unknown");
	else	
		len = sprintf (str, "%d", currentCoded->sequence);
		
	SetRect (&aRect, HTEXT2, YOFFSET, STATUSWIDTH-1, YOFFSET+YSPACE-1);
	ExtTextOut (hDC, HTEXT2, YOFFSET, ETO_OPAQUE|ETO_CLIPPED, 
	            &aRect, str, len, (LPINT)NULL); 
}

void
DrawStatusActivity (HDC hDC)
{
//	static int prevState = -1;
	COLORREF backColor;
	int len;
	SIZE size;
	RECT aRect;
//	if (CodingState == prevState)
//		return;
	
	switch (CodingState)
	{
	case DECODE_SKIPPING:
 		backColor = RGB(128, 0, 0);             // red
		strcpy (str, "Skipping");
		break;
		
	case DECODE_GET_TABLE:
 		backColor = RGB(0, 128, 128);           // purple?
		strcpy (str, "Getting table");
		break;                                                     
		
	case DECODE_PROCESSING:
 		backColor = RGB(0, 128, 0);             // green
		strcpy (str, "Decoding");
		break;

	case ATTACH_PROCESSING:
 		backColor = RGB(0, 128, 0);		// green
		strcpy (str, "Encoding");
		break;

	case ATTACH_POSTING:	
 		backColor = RGB(128, 128, 0);		// yellow
		if (ReviewAttach)
		   strcpy (str, "Creating review");
		else
		   strcpy (str, "Posting");
		break;

	case ATTACH_READFILE:
 		backColor = RGB(0, 128, 0);		// green
		strcpy (str, "Reading file");
		break;
	}

	len = lstrlen(str);
	// Create colored rectangle behind text
	SetRect (&aRect, HTEXT2, YOFFSET+YSPACE, HTEXT2 + 20*StatusCharWidth, 
	         YOFFSET+YSPACE + StatusLineHeight+1);
	
	SetTextColor (hDC, RGB(0,0,0));
	SetBkColor (hDC, backColor);

	// center the text in the rectangle
	GetTextExtentPoint (hDC, str, len, &size);
	ExtTextOut (hDC, HTEXT2 + (20*StatusCharWidth - size.cx)/2,
	            YOFFSET+YSPACE, ETO_OPAQUE|ETO_CLIPPED, &aRect, 
	            str, len, (LPINT)NULL); 

	SetTextColor (hDC, StatusTextColor);
	SetBkColor (hDC, StatusBackgroundColor);

//	prevState = CodingState;
}

/* ------------------------------------------------------------------------
 * 	Window Proc for Coding status window
 *	Simple text display window w/ scroll bars
 *	Associated with TypDecodeThread
 *	One per decode thread, variable size, user must delete it
 *
 *	Mostly copied from Petzold
 */
long FAR PASCAL
WinVnCodingWndProc  (hWnd, message, wParam, lParam)
     HWND hWnd;
     UINT message;
     WPARAM wParam;
     LPARAM lParam;
{
	static int   cxClient, cyClient, nVscrollPos, nVscrollMax,
		     nHscrollPos, nHscrollMax;
	int rangeTemp;
	RECT         client;
	HDC          hDC;
	int          i, x, y, nPaintBeg, nPaintEnd, nVscrollInc, nHscrollInc;
	PAINTSTRUCT  ps;
	TypTextBlock *ThisText;
	int	  ThisTextNum;
	int       CtrlState;
	
	for (ThisTextNum = 0; ThisTextNum < NumStatusTexts; ThisTextNum++)
		if (CodingStatusText[ThisTextNum]->hTextWnd == hWnd)
			break;
	
	if (ThisTextNum == NumStatusTexts)
		ThisTextNum = NumStatusTexts - 1;	// new status window

	ThisText = CodingStatusText[ThisTextNum];

	switch (message)
	{
	case WM_CREATE:
//		nVscrollPos = nHscrollPos = 0;
                break;

	case WM_SIZE:
		GetClientRect (hWnd, &client);
		cxClient = client.right - client.left;
		cyClient = client.bottom - client.top;
		// nVscrollMax is # lines that won't fit in client y area
		rangeTemp = ((int)(ThisText->numLines + 2) * (int)StatusLineHeight - cyClient) / (int)StatusLineHeight;
		nVscrollMax = max (0, rangeTemp);
		nVscrollPos = min (nVscrollPos, nVscrollMax);

		SetScrollRange (hWnd, SB_VERT, 0, nVscrollMax, FALSE);
		SetScrollPos   (hWnd, SB_VERT, nVscrollPos, TRUE);

		// nHscrollMax is # chars that won't fit in client x area
		rangeTemp = ((int)(ThisText->maxLineLen + 2) * (int)StatusCharWidth - cxClient) / (int)StatusCharWidth;
		nHscrollMax = max (0, rangeTemp);
		nHscrollPos = min (nHscrollPos, nHscrollMax);

		SetScrollRange (hWnd, SB_HORZ, 0, nHscrollMax, FALSE);
		SetScrollPos   (hWnd, SB_HORZ, nHscrollPos, TRUE);
		break;
     
	case WM_VSCROLL:
		switch (wParam)
		{
		case SB_TOP:
			nVscrollInc = -nVscrollPos;
			break;
	
		case SB_BOTTOM:
			nVscrollInc = nVscrollMax - nVscrollPos;
			break;

		case SB_LINEUP:
			nVscrollInc = -1;
			break;
	
		case SB_LINEDOWN:
			nVscrollInc = 1;
			break;

		case SB_PAGEUP:
			nVscrollInc = min (-1, -cyClient / (int)StatusLineHeight);
			break;

		case SB_PAGEDOWN:
			nVscrollInc = max (1, cyClient / (int)StatusLineHeight);
			break;

		case SB_THUMBTRACK:
			nVscrollInc = LOWORD (lParam) - nVscrollPos;
			break;

		default:
			nVscrollInc = 0;
		}
		nVscrollInc = max (-nVscrollPos,
			      min (nVscrollInc, nVscrollMax - nVscrollPos));

		if (nVscrollInc != 0)
		{
			nVscrollPos += nVscrollInc;
			ScrollWindow (hWnd, 0, -1 * (int)StatusLineHeight * nVscrollInc, NULL, NULL);
			SetScrollPos (hWnd, SB_VERT, nVscrollPos, TRUE);
		
			UpdateWindow (hWnd);
		}
		break;

	case WM_HSCROLL:
		switch (wParam)
		{
		case SB_LINEUP:
			nHscrollInc = -1;
			break;

		case SB_LINEDOWN:
			nHscrollInc = 1;
			break;

		case SB_PAGEUP:
			nHscrollInc = -8;
			break;			

		case SB_PAGEDOWN:
			nHscrollInc = 8;
			break;

		case SB_THUMBPOSITION:
			nHscrollInc = LOWORD (lParam) - nHscrollPos;
			break;
		default:
			nHscrollInc = 0;
		}
		nHscrollInc = max (-nHscrollPos,
			min (nHscrollInc, nHscrollMax - nHscrollPos));

		if (nHscrollInc != 0)
		{
		     nHscrollPos += nHscrollInc;
		     ScrollWindow (hWnd, -1 * (int)StatusCharWidth * nHscrollInc, 0, NULL, NULL);
		     SetScrollPos (hWnd, SB_HORZ, nHscrollPos, TRUE);

		}
		break;

	      case WM_PAINT:
		if (ThisText->numLines == 0)
			break;
       
		hDC = BeginPaint (hWnd, &ps);
		SetBkColor (hDC, StatusBackgroundColor);
		SetTextColor (hDC, StatusTextColor);
		SelectObject (hDC, hStatusFont);

		nPaintBeg = max (0, (nVscrollPos + ps.rcPaint.top / (int)StatusLineHeight - 1));
		nPaintEnd = min ((int)ThisText->numLines,
			(nVscrollPos + ps.rcPaint.bottom / (int)StatusLineHeight));

		SetTextAlign (hDC, TA_LEFT | TA_TOP);

		for (i = nPaintBeg; i < nPaintEnd; i++)
		{
			x = (int)StatusCharWidth * (1 - nHscrollPos);
			y = (int)StatusLineHeight * (1 - nVscrollPos + i);

			TextOut (hDC, x, y, TextBlockLine (ThisText, i), 
				lstrlen (TextBlockLine (ThisText, i)));
		}

/*		GetClientRect (hWnd, &client);
		cyClient = client.bottom - client.top;
		cxClient = client.right - client.left;
*/
		// nVscrollMax is # lines that won't fit in client y area
		rangeTemp = ((int)(ThisText->numLines + 2) * (int)StatusLineHeight - cyClient) / (int)StatusLineHeight;
		nVscrollMax = max (0, rangeTemp);
		SetScrollRange (hWnd, SB_VERT, 0, nVscrollMax, TRUE);
		// nHscrollMax is # chars that won't fit in client x area
		rangeTemp = ((int)(ThisText->maxLineLen + 2) * (int)StatusCharWidth - cxClient) / (int)StatusCharWidth;
		nHscrollMax = max (0, rangeTemp);
		SetScrollRange (hWnd, SB_HORZ, 0, nHscrollMax, TRUE);

		EndPaint (hWnd, &ps);
		break;

	      case WM_KEYDOWN:
		/* See if this key should be mapped to a scrolling event
		* for which we have programmed the mouse.  If so,
		* construct the appropriate mouse call and call the mouse code.
		*/
	
		CtrlState = GetKeyState (VK_CONTROL) < 0;
		for (i = 0; i < NUMKEYS; i++)
		{
		   if (wParam == key2scroll[i].wVirtKey &&
			CtrlState == key2scroll[i].CtlState)
		   {
			SendMessage (hWnd, key2scroll[i].iMessage,
			key2scroll[i].wRequest, 0L);
		   }
		}
		break;

	case WM_CLOSE:
		if (CodingState && ThisText->IsBusy)
		   MessageBox (hWnd,
		     "Please wait until en/decoding is complete",
		     "Cannot close status window", MB_OK|MB_ICONSTOP);
		else
		   DestroyWindow (hWnd);
		break;
		
	case WM_DESTROY:
		for (i = ThisTextNum; i < NumStatusTexts; i++)
			CodingStatusText[i] = CodingStatusText[i+1];
		FreeTextBlock (ThisText);
		NumStatusTexts--;
		break;
	
	default:
		return (DefWindowProc (hWnd, message, wParam, lParam));
	}
	
	return (0);
}


@


1.6
log
@always-on-top option, and general cleanup for 92.6
@
text
@d12 1
a12 1
 * $Id: wvcodewn.c 1.5 1994/08/11 00:09:17 jcooper Exp $
d14 3
a98 1
#if 0
a109 1
#endif
d350 2
a351 2
	for (ThisTextNum = 0; ThisTextNum < numStatusTexts; ThisTextNum++)
		if (codingStatusText[ThisTextNum]->hTextWnd == hWnd)
d354 2
a355 2
	if (ThisTextNum == numStatusTexts)
		ThisTextNum = numStatusTexts - 1;	// new status window
d357 1
a357 1
	ThisText = codingStatusText[ThisTextNum];
d529 1
a529 1
		if (CodingState)
d538 2
a539 2
		for (i = ThisTextNum; i < numStatusTexts; i++)
			codingStatusText[i] = codingStatusText[i+1];
d541 1
a541 1
		numStatusTexts--;
@


1.5
log
@Enhancements to Mime and article encoding/encoding
@
text
@d12 1
a12 1
 * $Id: wvcodewn.c 1.4 1994/07/25 20:13:40 jcooper Exp $
d14 3
a29 2
#include <stdio.h>
#include <stdlib.h>
a31 1
#include <string.h>
d34 4
d55 1
a56 1
	RECT         client;
d67 10
d79 18
d98 2
d107 2
d138 1
a138 1

@


1.4
log
@execution of decoded files
@
text
@d8 1
a8 1
 * Author: John S. Cooper (jcoop@@apl.com)                           *
d12 1
a12 1
 * $Id: wvcodewn.c 1.3 1994/02/24 21:36:10 jcoop Exp $
d14 3
d66 6
a71 3
		GetWindowRect (hWnd, &client);
	        MoveWindow (hWnd, client.left, client.top, STATUSWIDTH,
	               	    STATUSHEIGHT, TRUE);
@


1.3
log
@jcoop changes
@
text
@d12 1
a12 1
 * $Id: wvcodewn.c 1.2 1994/01/24 17:40:28 jcoop Exp $
d14 3
d305 1
d427 1
a427 1
	 case WM_PAINT:
d465 18
@


1.2
log
@90.2 changes
@
text
@d12 1
a12 1
 * $Id: wvcodewn.c 1.1 1994/01/16 12:10:59 jcoop Exp $
d14 3
d61 2
a62 2
	        MoveWindow (hWnd, client.left, client.top, 85 * StatusCharWidth, 
	        	    9 * StatusLineHeight, TRUE);
d89 1
a89 1
		else
d102 2
a103 2
 * 	Routines for drawing coding block status window
 *	2 columns, 3 lines
d105 5
a109 5
 *	HTITLE1       HTEXT1        HTITLE2  HTEXT2
 *	!             !             !        !
 *	Retrieving    ___________   Sequence ___________
 *	Lines Read    __________    Activity ___________
 *	Bytes Decoded __________    
d112 4
a115 4
#define HTITLE2 (43*StatusCharWidth)
#define HTEXT1	(20*StatusCharWidth)
#define HTEXT2	(56*StatusCharWidth)
#define YOFFSET (int)(StatusLineHeight*1.5)
a144 1
	HBRUSH hBrOld;
d146 1
d149 1
a149 1
		strcpy (str, "Name unknown");
d155 1
a155 1
			sprintf(temp, "%-20.20s", str);
d160 1
a160 1
			sprintf(temp, "%-20.20s", str); 
d164 4
a167 4
	hBrOld = SelectObject(hDC, hStatusBackgroundBrush);
	PatBlt (hDC, HTEXT1, YOFFSET, HTITLE2-HTEXT1-1, YSPACE-1, PATCOPY);
	TextOut (hDC, HTEXT1, YOFFSET, temp, lstrlen(temp)); 
	SelectObject(hDC, hBrOld);
d173 1
d175 4
a178 2
	len = sprintf (str, "%-20lu", currentCoded->numLines);
	TextOut (hDC, HTEXT1, YOFFSET + YSPACE, str, len);
d184 1
d186 1
a186 1
	len = sprintf (str, "%-20lu", currentCoded->numBytes);
d188 3
d196 1
d202 1
a202 1
		len = sprintf (str, "%-20.20s", "Unknown");
d204 5
a208 2
		len = sprintf (str, "%-20d", currentCoded->sequence);
	TextOut (hDC, HTEXT2, YOFFSET, str, len);
a214 1
	HBRUSH hBrOld, hBr;
d216 3
a218 1
	
d225 2
a226 2
 		backColor = RGB(200, 0, 0);             // red
		strcpy (str, "      Skipping");
d230 2
a231 2
 		backColor = RGB(0, 200, 200);           // purple?
		strcpy (str, "   Getting table");
d235 2
a236 2
 		backColor = RGB(0, 200, 0);             // green
		strcpy (str, "      Decoding");
d240 2
a241 2
 		backColor = RGB(0, 200, 0);		// green
		strcpy (str, "      Encoding");
d245 1
a245 1
 		backColor = RGB(210, 210, 0);		// yellow
d247 1
a247 1
		   strcpy (str, "  Creating review");
d249 1
a249 1
		   strcpy (str, "      Posting");
d253 2
a254 2
 		backColor = RGB(0, 200, 0);		// green
		strcpy (str, "    Reading file");
d258 5
a262 4
	hBr = CreateSolidBrush(backColor);
	hBrOld = SelectObject(hDC, hBr);
	PatBlt(hDC, HTEXT2, YOFFSET+YSPACE, 20*StatusCharWidth, StatusLineHeight+1, PATCOPY);

d265 7
a271 2
	TextOut (hDC, HTEXT2, YOFFSET + YSPACE, str, strlen(str));
	DeleteObject (SelectObject(hDC, hBrOld));
@


1.1
log
@Initial revision
@
text
@d12 5
a16 2
 * $Id: $
 * $Log: $
d81 10
d287 1
a287 1
		ThisTextNum = numStatusTexts - 1;
d442 9
@

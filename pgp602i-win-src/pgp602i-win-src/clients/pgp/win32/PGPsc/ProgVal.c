/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: ProgVal.c,v 1.5.18.1 1998/11/12 03:13:27 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

WNDPROC origProgValProc;

void PaintProgValBar(HWND hwnd,HDC hDC)
{
	RECT rcBar;
	HBRUSH hBrush;
	WORKSTRUCT *ws;

	ws=(WORKSTRUCT *)GetWindowLong(GetParent(hwnd), DWL_USER);

	GetClientRect(hwnd,&rcBar);
	hBrush=CreateSolidBrush (GetSysColor(COLOR_BTNFACE));
	SelectObject(hDC,hBrush);			            
	Rectangle(hDC,rcBar.left,rcBar.top,rcBar.right,rcBar.bottom);  
	DeleteObject(hBrush);
			
	if(ws->curpos!=0)
	{ 
		unsigned long cx;
			
		cx=ws->curpos;

		if(cx>ws->maxpos)
			cx=ws->maxpos;

		rcBar.right=rcBar.left+3+
			(int)((float)(rcBar.right-rcBar.left-6)*
			((float)cx/(float)ws->maxpos));    
							
		rcBar.left+=3;
		rcBar.top+=3;
		rcBar.right-=3;
		rcBar.bottom-=3;
			
		hBrush=CreateSolidBrush (GetSysColor(COLOR_HIGHLIGHT));
		FillRect(hDC,&rcBar,hBrush);
		DeleteObject(hBrush); 
	}
}

void UpdateProgValBar(HWND hwnd)
{
	HDC hDC;

	hDC=GetDC(hwnd);
	PaintProgValBar(hwnd,hDC);
	ReleaseDC(hwnd,hDC);
	ValidateRect(hwnd,NULL);
}

LRESULT WINAPI MyProgressValidityWndProc(HWND hwnd, UINT msg,
										 WPARAM wParam, LPARAM lParam)
{
 
	HDC hDC;
	PAINTSTRUCT ps;     

	switch(msg)
	{
		case WM_PAINT:   
		{		
			hDC=BeginPaint(hwnd, &ps);
			PaintProgValBar(hwnd,hDC);
            EndPaint(hwnd, &ps);
 			break;
		}
    }
     
	//  Pass all non-custom messages to old window proc
	return CallWindowProc((FARPROC)origProgValProc, hwnd,
						  msg, wParam, lParam ) ;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

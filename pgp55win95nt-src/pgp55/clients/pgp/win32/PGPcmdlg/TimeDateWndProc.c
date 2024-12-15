/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	TimeDateWndProc.c - implements procs for timedate control
	

	$Id: TimeDateWndProc.c,v 1.15 1997/09/30 19:02:34 elrod Exp $
____________________________________________________________________________*/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <time.h>

#include "pgpKeys.h"
#include "PGPcmdlg.h"
#include "resource.h"


#define MAX_YEAR	2050
#define MIN_YEAR	1970

BOOL 
WrapValue(	HWND hwnd, 
			LONG* value);

void HandleNewValue(HWND hwnd, 
					LONG value,
					BOOL bWrap);

LRESULT 
CALLBACK 
TimeDateEditSubclassWndProc(HWND hwnd, 
					UINT msg, 
					WPARAM wParam, 
					LPARAM lParam);

LRESULT 
CALLBACK 
TimeDateWndProc(HWND hwnd, 
				UINT msg, 
				WPARAM wParam, 
				LPARAM lParam)
{ 
	switch(msg)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
			HDC			hdc;
			TEXTMETRIC  tm;
			HFONT		hfont;
			LONG        cxFont;
			LONG        cyFont;
			HWND		hwndEdit, hwndSpin;
			LONG		offset = 0;
			LONG		type = (LONG)lpcs->lpCreateParams;
			time_t		ltime;
			struct tm*	today;
			char		szNumber[10];
			RECT		rect;
			int			DaysInMonth[] = {	31, 28, 31, 30, 31, 30,
											31, 31, 30, 31, 30, 31};

			InitCommonControls();

			time(&ltime);
			today = localtime(&ltime);

			hfont = GetStockObject(DEFAULT_GUI_FONT);

			SendMessage(hwnd, WM_SETFONT,  (WPARAM)hfont, MAKELPARAM(TRUE, 0));

			//Get text dimensions on which to base control sizes.
			hdc=GetDC(hwnd);
			GetTextMetrics(hdc, &tm);
			cxFont=tm.tmAveCharWidth;
			cyFont=tm.tmHeight;
			ReleaseDC(hwnd, hdc);

			SetProp(hwnd, "type", (HANDLE)type);

			hwndEdit = CreateWindowEx(	WS_EX_NOPARENTNOTIFY,
										"EDIT", 
										"", 
										WS_CHILD|
										WS_VISIBLE|ES_NUMBER|ES_RIGHT|
										ES_MULTILINE|WS_TABSTOP |WS_GROUP,
										offset, 
										2, 
										2*cxFont,
										cyFont, 
										hwnd, 
										NULL, 
										lpcs->hInstance, 
										NULL);
			
			offset += 2*cxFont;
			SendMessage(hwndEdit, WM_SETFONT,  (WPARAM)hfont, MAKELPARAM(TRUE, 0));
			SendMessage(hwndEdit, EM_LIMITTEXT, 2, 0L);
			SetProp(hwnd, "Edit1", hwndEdit);
			SetProp( hwndEdit, "oldproc",(HANDLE)GetWindowLong( hwndEdit, GWL_WNDPROC ) ); 
			SetWindowLong( hwndEdit, GWL_WNDPROC, (DWORD)TimeDateEditSubclassWndProc );    

			if(type == PGPCOMDLG_DISPLAY_DATE)
			{
				SetWindowText(hwndEdit, itoa(today->tm_mon + 1, szNumber, 10));
				SetProp(hwndEdit, "value", (HANDLE)(today->tm_mon + 1));
				SetProp(hwndEdit, "upper", (HANDLE)12);
				SetProp(hwndEdit, "lower", (HANDLE)1);
			}
			else if(type == PGPCOMDLG_DISPLAY_TIME)
			{
				if( today->tm_hour > 12 )
				{
					today->tm_hour -= 12;
				}

				if( today->tm_hour == 0 )  /* Adjust if midnight hour. */
				{
					today->tm_hour = 12;
				}

				SetWindowText(hwndEdit, itoa(today->tm_hour, szNumber, 10));
				SetProp(hwndEdit, "value", (HANDLE)(today->tm_hour));
				SetProp(hwndEdit, "upper", (HANDLE)12);
				SetProp(hwndEdit, "lower", (HANDLE)1);
			}
			else
			{
				wsprintf(szNumber, "%.2d",0);
				SetWindowText(hwndEdit, szNumber);
				SetProp(hwndEdit, "value", (HANDLE)0);
				SetProp(hwndEdit, "upper", (HANDLE)99);
				SetProp(hwndEdit, "lower", (HANDLE)0);
			}

			/*hwndStatic = CreateWindow(	"STATIC", 
										"", 
										WS_CHILD|50020081,
										offset, 
										2, 
										cxFont,
										cyFont, 
										hwnd, 
										NULL, 
										lpcs->hInstance, 
										NULL);*/

			offset += cxFont;
			//SendMessage(hwndStatic, WM_SETFONT,  (WPARAM)hfont, MAKELPARAM(TRUE, 0));
			//SetWindowText(hwndStatic, (type == PGPCOMDLG_DISPLAY_DATE ? "/" : ":"));
			
			hwndEdit = CreateWindowEx(	WS_EX_NOPARENTNOTIFY,
										"EDIT",
										"", 
										WS_CHILD|
										WS_VISIBLE| ES_NUMBER|ES_RIGHT|
										ES_MULTILINE|WS_TABSTOP|WS_GROUP ,
										offset, 
										2, 
										2*cxFont,
										cyFont, 
										hwnd, 
										NULL, 
										lpcs->hInstance, 
										NULL);
			
			offset += 2*cxFont;
			SendMessage(hwndEdit, WM_SETFONT,  (WPARAM)hfont, MAKELPARAM(TRUE, 0));
			SendMessage(hwndEdit, EM_LIMITTEXT, 2, 0L);
			SetProp(hwnd, "Edit2", hwndEdit);
			SetProp( hwndEdit, "oldproc",(HANDLE)GetWindowLong( hwndEdit, GWL_WNDPROC ) ); 
			SetWindowLong( hwndEdit, GWL_WNDPROC, (DWORD)TimeDateEditSubclassWndProc );   

			if(type == PGPCOMDLG_DISPLAY_DATE)
			{
				SetWindowText(hwndEdit, itoa(today->tm_mday, szNumber, 10));
				SetProp(hwndEdit, "value", (HANDLE)today->tm_mday);
				SetProp(hwndEdit, "upper", (HANDLE)DaysInMonth[today->tm_mon]);
				SetProp(hwndEdit, "lower", (HANDLE)1);
			}
			else if(type == PGPCOMDLG_DISPLAY_TIME)
			{
				wsprintf(szNumber, "%.2d",today->tm_min);
				SetWindowText(hwndEdit, szNumber);
				SetProp(hwndEdit, "value", (HANDLE)today->tm_min);
				SetProp(hwndEdit, "upper", (HANDLE)59);
				SetProp(hwndEdit, "lower", (HANDLE)1);
			}
			else
			{
				wsprintf(szNumber, "%.2d",0);
				SetWindowText(hwndEdit, szNumber);
				SetProp(hwndEdit, "value", (HANDLE)0);
				SetProp(hwndEdit, "upper", (HANDLE)99);
				SetProp(hwndEdit, "lower", (HANDLE)0);
			}

			/*hwndStatic = CreateWindowEx(WS_EX_LEFT|	WS_EX_LTRREADING |
										WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY,
										"STATIC", 
										"", 
										WS_CHILD|WS_GROUP|
										WS_VISIBLE|SS_CENTER|0x80,
										offset, 
										2, 
										cxFont,
										cyFont, 
										hwnd, 
										NULL, 
										lpcs->hInstance, 
										NULL);*/

			offset += cxFont;
			//SendMessage(hwndStatic, WM_SETFONT,  (WPARAM)hfont, MAKELPARAM(TRUE, 0));
			//SetWindowText(hwndStatic, (type == PGPCOMDLG_DISPLAY_DATE ? "/" : ":"));


			hwndEdit = CreateWindowEx(	WS_EX_NOPARENTNOTIFY,
										"EDIT",
										"", 
										WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_CHILD|
										WS_VISIBLE|ES_NUMBER|ES_RIGHT|
										ES_MULTILINE|WS_TABSTOP|WS_GROUP,
										offset, 
										2, 
										(type == PGPCOMDLG_DISPLAY_DATE ? 4 : 2)*cxFont,
										cyFont, 
										hwnd, 
										NULL, 
										lpcs->hInstance, 
										NULL);

			offset += (type == PGPCOMDLG_DISPLAY_DATE ? 4 : 2) * cxFont;
			SendMessage(hwndEdit, WM_SETFONT,  (WPARAM)hfont, MAKELPARAM(TRUE, 0));
			SendMessage(hwndEdit, EM_LIMITTEXT, (type == PGPCOMDLG_DISPLAY_DATE ? 4 : 2), 0L);
			SetProp(hwnd, "Edit3", hwndEdit);
			SetProp( hwndEdit, "oldproc",(HANDLE)GetWindowLong( hwndEdit, GWL_WNDPROC ) ); 
			SetWindowLong( hwndEdit, GWL_WNDPROC, (DWORD)TimeDateEditSubclassWndProc );   

			if(type == PGPCOMDLG_DISPLAY_DATE)
			{
				SetWindowText(hwndEdit, itoa(today->tm_year + 1900, szNumber, 10));
				SetProp(hwndEdit, "value", (HANDLE)(today->tm_year + 1900));
				SetProp(hwndEdit, "upper", (HANDLE)MAX_YEAR);
				SetProp(hwndEdit, "lower", (HANDLE)MIN_YEAR);
			}
			else if(type == PGPCOMDLG_DISPLAY_TIME)
			{
				wsprintf(szNumber, "%.2d",today->tm_sec);
				SetWindowText(hwndEdit, szNumber);
				SetProp(hwndEdit, "value", (HANDLE)(today->tm_sec));
				SetProp(hwndEdit, "upper", (HANDLE)59);
				SetProp(hwndEdit, "lower", (HANDLE)0);
			}
			else
			{
				wsprintf(szNumber, "%.2d",0);
				SetWindowText(hwndEdit, szNumber);
				SetProp(hwndEdit, "value", (HANDLE)0);
				SetProp(hwndEdit, "upper", (HANDLE)99);
				SetProp(hwndEdit, "lower", (HANDLE)0);
			}

			offset += cxFont;

			SetWindowPos(	hwnd,
							NULL,
							0,
							0,
							offset,
							(cyFont*6)/5 + 2,
							SWP_NOMOVE|SWP_NOZORDER);

			hwndSpin = CreateUpDownControl(WS_CHILD|WS_VISIBLE|
								UDS_ALIGNRIGHT|UDS_ARROWKEYS|
								WS_TABSTOP,
								lpcs->x + offset,
								lpcs->y,
								0,
								(cyFont*6)/5 + 2,
								GetParent(hwnd),
								1,
								lpcs->hInstance, 
								NULL,
								1,
								0,
								0);

			GetWindowRect(hwndSpin, &rect);

			SetProp(hwnd, "Spin", hwndSpin);

			offset += rect.right - rect.left;

			SetWindowPos(	GetParent(hwnd),
							NULL,
							0,
							0,
							offset,
							(cyFont*6)/5 + 2,
							SWP_NOMOVE|SWP_NOZORDER);

			return 0;
		}

		case WM_DESTROY: 
		{
			// remove props
			RemoveProp( hwnd, "Edit1" ); 
			RemoveProp( hwnd, "Edit2" ); 
			RemoveProp( hwnd, "Edit3" ); 
			RemoveProp( hwnd, "Spin" ); 
			RemoveProp( hwnd, "type" ); 

			break;
		}

		case WM_ERASEBKGND:
		{
			HDC		hdc = (HDC) wParam; // handle of device context 
			BOOL	bEnabled = TRUE;
			//HBRUSH	hbrush, oldbrush;
			RECT	rect;

			GetClientRect(hwnd, &rect);

			bEnabled =  IsWindowEnabled(hwnd);

			if(bEnabled)
			{
				FillRect(hdc, &rect, (HBRUSH) (COLOR_WINDOW + 1)); 
			}
			else
			{
				FillRect(hdc, &rect, (HBRUSH) (COLOR_INACTIVEBORDER + 1)); 
			}

			return TRUE;
		}

		case WM_PAINT:
		{
			HDC			hdc;
			PAINTSTRUCT ps;
			RECT		rect;
			TEXTMETRIC  tm;
			LONG        cxFont;
			LONG        cyFont;
			HFONT		hfont, oldfont;
			HWND		hwndChild = GetProp(hwnd, "Edit1");
			int			type = (int)GetProp(hwnd, "type");
			BOOL		bEnabled = TRUE;

			bEnabled =  IsWindowEnabled(hwnd);

			hfont = GetStockObject(DEFAULT_GUI_FONT);

			hdc = BeginPaint(hwnd, &ps);

			if(bEnabled)
			{
				SetTextColor(hdc, RGB(0,0,0));
			}
			else
			{
				SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
			}

			oldfont = SelectObject(hdc, hfont);

			SetBkMode(hdc, TRANSPARENT);

			//Get text dimensions on which to base control sizes.
			GetTextMetrics(hdc, &tm);
			cxFont=tm.tmAveCharWidth;
			cyFont=tm.tmHeight;

			GetWindowRect(hwndChild, &rect);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rect, 2);

			rect.left = rect.right + 1;
			rect.right = rect.left + cxFont;


			DrawText(	hdc, 
						(type == PGPCOMDLG_DISPLAY_DATE ? "/" : ":"),
						1, 
						&rect, 
						DT_CENTER);

			hwndChild = GetProp(hwnd, "Edit2");

			GetWindowRect(hwndChild, &rect);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rect, 2);

			rect.left = rect.right + 1;
			rect.right = rect.left + cxFont;


			DrawText(	hdc, 
						(type == PGPCOMDLG_DISPLAY_DATE ? "/" : ":"),
						1, 
						&rect, 
						DT_CENTER);


			SelectObject(hdc, oldfont);
			//SelectObject(hdc, oldbrush);

			EndPaint(hwnd, &ps);

			break;
		}

		case WM_SIZE:
		{
			long nHeight;
			HWND hwndChild = GetProp(hwnd, "Edit1");
			RECT rect;
			int count = 0;

			while(hwndChild && count < 5)
			{
				GetWindowRect(hwnd, &rect);
				//MapWindowPoints(NULL, hwnd, (LPPOINT)&rect, 2);

				nHeight = rect.bottom - rect.top;

				GetWindowRect(hwndChild, &rect);
				MapWindowPoints(NULL, hwnd, (LPPOINT)&rect, 2);

				SetWindowPos(	hwndChild,
								NULL,
								rect.left,
								(nHeight - (rect.bottom - rect.top))/2,
								0,
								0,
								SWP_NOSIZE|SWP_NOZORDER);

				hwndChild = GetNextWindow(hwndChild, GW_HWNDNEXT);

				count++;
			}
			
			break;
		}

		/*case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam); // notification code 
			HWND hwndCtl = (HWND) lParam;      // handle of control 

			if( wNotifyCode == EN_CHANGE )
			{
				
			}

			break;
		}*/

		case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			NM_UPDOWN* pnmud = (NM_UPDOWN*) lParam;

			switch( pnmh->code )
			{
				case UDN_DELTAPOS:
				{
					LONG value;
					HWND hwndFocus = GetFocus();
					char szNumber[10] = {0x00};

					GetWindowText(hwndFocus, szNumber, sizeof(szNumber));
					value = atoi(szNumber);
					
					value += pnmud->iDelta;

					HandleNewValue(hwndFocus, value, TRUE);

					SetFocus(hwndFocus);
					break;
				}
			}

			break;
		}

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);

	}

	return ((LRESULT) TRUE);
}

BOOL 
WrapValue(	HWND hwnd, 
			LONG* value)
{
	BOOL bValid = TRUE;
	LONG upper = (LONG)GetProp(hwnd, "upper");
	LONG lower = (LONG)GetProp(hwnd, "lower");
	
	if(*value < lower )
	{
		*value = upper;
	}

	if(*value > upper )
	{
		*value = lower;
	}

	return bValid;
}

void HandleNewValue(HWND hwnd, LONG value, BOOL bWrap)
{
	HWND hwndParent = GetParent(hwnd);
	HWND hwndTime	= GetParent(hwndParent);
	HWND hwndEdit1	= GetProp(hwndParent, "Edit1");
	HWND hwndEdit2	= GetProp(hwndParent, "Edit2");
	HWND hwndEdit3	= GetProp(hwndParent, "Edit3");
	LONG type		= (LONG)GetProp(hwndParent, "type");
	LONG wrap		= 0;
	char szNumber[10] = {0x00};
	int DaysInMonth[] = {	31, 28, 31, 30, 31, 30,
							31, 31, 30, 31, 30, 31};
	wrap = value;

	WrapValue(hwnd, &wrap);

	if(wrap != value && !bWrap)
	{
		value = (LONG)GetProp(hwnd, "value");
	}
	else
	{
		/*long temp = (LONG)GetProp(hwnd, "value");

		if(value != temp)
		{
			NMHDR nmh;
					
			memset(&nmh, 0x00, sizeof(nmh));

			nmh.hwndFrom	= hwndTime;
			nmh.code		= TDN_TIMECHANGED;

			SendMessage(	GetParent(nmh.hwndFrom), 
							WM_NOTIFY, 
							0L, 
							(LPARAM)&nmh);
		}*/

		value = wrap;

		SetProp(hwnd, "value", (HANDLE)value);
	}

	if( hwnd == hwndEdit1 ) 
	{
		if(type == PGPCOMDLG_DISPLAY_DATE)
		{
			int day = Date_GetDay(hwndTime);
			int year = Date_GetYear(hwndTime);
			int leapOffset = 0;

			if( !(year % 4) && 
				( (year % 100) || !(year % 400) ) &&
				value == 2) // February
			{
				leapOffset = 1;
			}

			Date_SetDayLimits(	hwndTime, 
								DaysInMonth[value - 1] + leapOffset,
								1);


			if(day > (DaysInMonth[value - 1] + leapOffset))
			{
				Date_SetDay(hwndTime, DaysInMonth[value - 1] + leapOffset);
			}
		}

		wsprintf(szNumber, 
			(type == PGPCOMDLG_DISPLAY_DURATION ? "%.2d" : "%d"), 
			value);

		SetWindowText(hwnd, szNumber);
	}
	else if ( hwnd == hwndEdit2 )
	{
		wsprintf(szNumber, 
			(type == PGPCOMDLG_DISPLAY_DATE ? "%d" : "%.2d" ), 
			value);

		SetWindowText(hwnd, szNumber);
	}
	else if( hwnd == hwndEdit3 )
	{
		if(type == PGPCOMDLG_DISPLAY_DATE)
		{
			int year = value;
			int month = Date_GetMonth(hwndTime);
			int day = Date_GetDay(hwndTime);

			if( !(year % 4) && 
				( (year % 100) || !(year % 400) ) &&
				month == 2 ) // February
			{
				Date_SetDayLimits(hwndTime, 29, 1);
			}
			else if(month == 2) // February
			{
				Date_SetDayLimits(hwndTime, 28, 1);

				if(day > 28)
				{
					Date_SetDay(hwndTime, 28);
				}
			}

		}

		wsprintf(szNumber, 
			(type == PGPCOMDLG_DISPLAY_DATE ? "%d" : "%.2d" ), 
			value);

		SetWindowText(hwnd, szNumber);
	}

	{
		NMHDR nmh;
				
		memset(&nmh, 0x00, sizeof(nmh));

		nmh.hwndFrom	= hwndTime;
		nmh.code		= TDN_TIMECHANGED;

		SendMessage(	GetParent(nmh.hwndFrom), 
						WM_NOTIFY, 
						0L, 
						(LPARAM)&nmh);
	}

}


LRESULT 
CALLBACK 
TimeDateEditSubclassWndProc(HWND hwnd, 
					UINT msg, 
					WPARAM wParam, 
					LPARAM lParam)
{
	WNDPROC oldproc = (WNDPROC)GetProp(hwnd, "oldproc");

	switch(msg)
	{
		case WM_DESTROY: 
		{
			// put back old window proc
			SetWindowLong( hwnd, GWL_WNDPROC, (DWORD)oldproc );

			// remove props
			RemoveProp( hwnd, "oldproc" ); 
			RemoveProp( hwnd, "upper" ); 
			RemoveProp( hwnd, "lower" ); 
			RemoveProp( hwnd, "value" ); 
			break;
		}

		case WM_KEYDOWN:
		{
			int nVirtKey = (int) wParam;    // virtual-key code 
			long lKeyData = lParam;          // key data 
			HWND hwndNext = NULL;

			if(nVirtKey == VK_TAB)
			{
				SHORT ShiftKeyState = 0;

				ShiftKeyState = GetAsyncKeyState(VK_SHIFT);

				hwndNext = GetNextWindow( hwnd, 
						((ShiftKeyState & 0x8000) ? GW_HWNDPREV : GW_HWNDNEXT) );

				if(!hwndNext || hwndNext == hwnd)
				{
					hwndNext = GetNextWindow(
						((ShiftKeyState & 0x8000) ? GetParent(GetParent(hwnd)) : GetParent(hwnd)), 
						((ShiftKeyState & 0x8000) ? GW_HWNDPREV : GW_HWNDNEXT) );	
				}

				if(hwndNext)
				{
					SetFocus(hwndNext);	
				}

				return 0;
			}

			break;
		}

		case WM_KILLFOCUS:
		{
			char szCurrent[5];
			LONG value;

			GetWindowText(hwnd, szCurrent, sizeof(szCurrent));

			value = atoi(szCurrent);

			HandleNewValue(hwnd, value, FALSE);			

			break;
		}

		case WM_CHAR:
		{
			TCHAR chCharCode = (TCHAR) wParam;    // character code 
			long lKeyData = lParam;              // key data

			if(chCharCode == 0x09)
			{
				return 0;
			}

			if(chCharCode >= '0' && chCharCode <= '9')
			{
				char szCurrent[5], szProposed[5];
				//char szTest[256];
				DWORD begin, end;
				size_t maxChar = 0;

				GetWindowText(hwnd, szCurrent, sizeof(szCurrent));

				strcpy(szProposed, szCurrent);

				SendMessage(hwnd, EM_GETSEL, (WPARAM)&begin, (LPARAM)&end);

				maxChar = SendMessage(hwnd, EM_GETLIMITTEXT, 0L, 0L );

				if(end - begin) // selection?
				{
					//wsprintf(szTest, "%d - %d", begin, end);
					//MessageBox(NULL, szTest, "range", MB_OK);
					szProposed[begin] = chCharCode;

					strcpy(szProposed + begin + 1, szProposed + end);

					//MessageBox(NULL, szProposed, "szProposed", MB_OK);
				}
				else
				{
					if(strlen(szCurrent) < maxChar)
					{
						szProposed[begin] = chCharCode;

						strcpy(szProposed + begin + 1, szCurrent + begin);

						//MessageBox(NULL, szProposed, "szProposed", MB_OK);
					}
				}

				if((maxChar == 4 && strlen(szProposed) == 4) || maxChar == 2)
				{
					if(strcmp(szCurrent, szProposed))
					{
						LONG value;

						value = atoi(szProposed);

						HandleNewValue(hwnd, value, FALSE);

						SendMessage(hwnd, EM_SETSEL, strlen(szProposed), strlen(szProposed));

						return 0;
					}
				}
			}

			break;
		}

	}

	return CallWindowProc(oldproc, hwnd, msg, wParam, lParam);
}


LRESULT 
CALLBACK 
TimeDateFrameWndProc(HWND hwnd, 
				UINT msg, 
				WPARAM wParam, 
				LPARAM lParam)
{
	HWND hwndTime		= GetProp(hwnd, "TimeDateWindow");
	HWND hwndEdit		= NULL;
	HWND hwndSpin		= GetProp(hwnd, "Spin");
	LONG type			= (LONG)GetProp(hwndTime, "type");
	char szNumber[10]	= {0x00};

	switch(msg)
	{
		case WM_DESTROY: 
		{
			// remove props
			RemoveProp( hwnd, "TimeDateWindow" ); 
			break;
		}

		case WM_SIZE:
		{
			long nHeight;
			RECT rect;
			
			GetWindowRect(hwnd, &rect);
			
			nHeight = rect.bottom - rect.top;

			GetWindowRect(hwndTime, &rect);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rect, 2);

			SetWindowPos(	hwndTime,
							NULL,
							0,
							0,
							rect.right - rect.left,
							nHeight,
							SWP_NOMOVE|SWP_NOZORDER);

			GetWindowRect(hwndSpin, &rect);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rect, 2);

			SetWindowPos(	hwndSpin,
							NULL,
							0,
							0,
							rect.right - rect.left,
							nHeight,
							SWP_NOMOVE|SWP_NOZORDER);


			break;
		}

		case TD_GETHOUR:
		{
			hwndEdit = GetProp(hwndTime, "Edit1");

			GetWindowText(hwndEdit, szNumber, sizeof(szNumber));

			return( atol(szNumber) );
		}

		case TD_GETMINUTE:
		{
			hwndEdit = GetProp(hwndTime, "Edit2");

			GetWindowText(hwndEdit, szNumber, sizeof(szNumber));

			return( atol(szNumber) );
		}

		case TD_GETSECOND:
		{
			hwndEdit = GetProp(hwndTime, "Edit3");

			GetWindowText(hwndEdit, szNumber, sizeof(szNumber));

			return( atol(szNumber) );
		}

		case TD_SETHOUR:
		{
			hwndEdit = GetProp(hwndTime, "Edit1");

			wsprintf(szNumber, 
			(type == PGPCOMDLG_DISPLAY_DURATION ? "%.2d" : "%d"), 
			wParam);


			SetWindowText(hwndEdit, szNumber);

			SetProp(hwndEdit, "value", (HANDLE)wParam);

			break;
		}

		case TD_SETMINUTE:
		{
			hwndEdit = GetProp(hwndTime, "Edit2");

			wsprintf(szNumber, 
			(type == PGPCOMDLG_DISPLAY_DATE ? "%d" : "%.2d" ), 
			wParam);

			SetWindowText(hwndEdit, szNumber);

			SetProp(hwndEdit, "value", (HANDLE)wParam);

			break;
		}

		case TD_SETSECOND:
		{
			hwndEdit = GetProp(hwndTime, "Edit3");

			wsprintf(szNumber, 
			(type == PGPCOMDLG_DISPLAY_DATE ? "%d" : "%.2d" ), 
			wParam);

			SetWindowText(hwndEdit, szNumber);

			SetProp(hwndEdit, "value", (HANDLE)wParam);

			break;
		}

		case TD_SETHOURLIMIT:
		{
			hwndEdit = GetProp(hwndTime, "Edit1");

			SetProp(hwndEdit, "upper", (HANDLE)wParam);
			SetProp(hwndEdit, "lower", (HANDLE)lParam);

			break;
		}

		case TD_SETMINUTELIMIT:
		{
			hwndEdit = GetProp(hwndTime, "Edit2");

			SetProp(hwndEdit, "upper", (HANDLE)wParam);
			SetProp(hwndEdit, "lower", (HANDLE)lParam);
			break;
		}

		case TD_SETSECONDLIMIT:
		{
			hwndEdit = GetProp(hwndTime, "Edit3");

			SetProp(hwndEdit, "upper", (HANDLE)wParam);
			SetProp(hwndEdit, "lower", (HANDLE)lParam);

			break;
		}

		case WM_ENABLE:
		{
			BOOL bEnabled = (BOOL) wParam;

			hwndEdit = GetProp(hwndTime, "Edit1");
			EnableWindow(hwndEdit, bEnabled);
			hwndEdit = GetProp(hwndTime, "Edit2");
			EnableWindow(hwndEdit, bEnabled);
			hwndEdit = GetProp(hwndTime, "Edit3");
			EnableWindow(hwndEdit, bEnabled);

			EnableWindow(hwndTime, bEnabled);
			InvalidateRect(hwndTime, NULL, TRUE);
			
			EnableWindow(hwndSpin, bEnabled);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			NM_UPDOWN* pnmud = (NM_UPDOWN*) lParam;

			switch( pnmh->code )
			{
				case UDN_DELTAPOS:
				{
					SendMessage(hwndTime, msg, wParam, lParam);
					break;
				}
			}

			break;
		}


		default:
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

	}

	return ((LRESULT) TRUE);
}

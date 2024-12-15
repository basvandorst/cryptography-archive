/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: ChoiceProc.c,v 1.12 1998/08/11 15:19:59 pbj Exp $
____________________________________________________________________________*/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "pgpKeys.h"

#include "PGPcl.h"
#include "Choice.h"
#include "SearchCommon.h"
#include "resource.h"

extern HINSTANCE		g_hInst;


BOOL 
CALLBACK 
ChoiceProc(	HWND hwnd, 
			UINT msg, 
			WPARAM wParam, 
			LPARAM lParam )
{ 
	static HWND hwndPrevChild	= NULL;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			RECT rect, rectPrevChild, rectDefaultBox;
			HWND hwndDefaultBox, hwndBox;
			int height = 0;
			int defaultSelection;
			HWND hwndTime = NULL;
			HWND hwndParent = NULL;

			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO);

			GetClientRect(hwndBox, &rect);
			MapWindowPoints(hwndBox, hwnd, (LPPOINT)&rect, 2);

			hwndTime = CreateWindowEx (0, DATETIMEPICK_CLASS,
							"DateTime",
							WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
							rect.left, rect.top-1, 
							rect.right-rect.left, rect.bottom-rect.top+2, 
							hwnd, NULL, g_hInst, NULL);
			SetWindowPos (hwndTime, GetDlgItem (hwnd, IDC_VERB),
							0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			SendMessage (hwndTime, DTM_SETMCCOLOR, 
						MCSC_MONTHBK, (LPARAM)GetSysColor (COLOR_3DFACE));

			ShowWindow(hwndTime, SW_HIDE);
			SetProp( hwnd, "hwndTime", hwndTime); 

			hwndPrevChild = (HWND) lParam;

			defaultSelection = ChoiceCount(GetParent(hwnd)) + 1;

			defaultSelection = defaultSelection % 
							(IDS_ATTRIBUTE_LAST - IDS_ATTRIBUTE_1);

			InitializeControlValues(hwnd, defaultSelection);

			GetClientRect(hwnd, &rect);

			hwndDefaultBox = GetDlgItem(GetParent(hwnd), 
										IDC_SPECIFIER_COMBO);

			GetClientRect(hwnd, &rect);
			GetWindowRect(hwndDefaultBox, &rectDefaultBox);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rectDefaultBox, 2);

			if( hwndPrevChild )
			{
				GetClientRect(hwndPrevChild, &rectPrevChild);
			}
			else
			{
				HWND hwndDefaultBoxes = GetDlgItem(	GetParent(hwnd), 
													IDC_DEFAULT_GROUP);

				GetWindowRect(hwndDefaultBoxes, &rectPrevChild);
			}



			hwndParent = GetParent (hwnd);

			// subclass edit box
			SetProp(hwndTime,"hwnd", hwndParent );  
			SetProp(hwndTime,  
					"oldproc",
					(HANDLE)GetWindowLong( hwndTime, GWL_WNDPROC ) ); 
			SetWindowLong(	hwndTime,  
							GWL_WNDPROC, 
							(DWORD)SearchSubclassWndProc );    

			// subclass edit box
			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO);
			SetProp(hwndBox,"hwnd", hwndParent );  
			SetProp(hwndBox,  
					"oldproc",
					(HANDLE)GetWindowLong( hwndBox, GWL_WNDPROC ) ); 
			SetWindowLong(	hwndBox,  
							GWL_WNDPROC, 
							(DWORD)SearchSubclassWndProc );    

			// subclass edit box
			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO_LIST);
			SetProp(hwndBox,"hwnd", hwndParent );  
			SetProp(hwndBox,  
					"oldproc",
					(HANDLE)GetWindowLong( hwndBox, GWL_WNDPROC ) ); 
			SetWindowLong(	hwndBox,  
							GWL_WNDPROC, 
							(DWORD)SearchSubclassWndProc );    

			// subclass edit box
			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_EDIT);
			SetProp(hwndBox,"hwnd", hwndParent );  
			SetProp(hwndBox,  
					"oldproc",
					(HANDLE)GetWindowLong( hwndBox, GWL_WNDPROC ) ); 
			SetWindowLong(	hwndBox,  
							GWL_WNDPROC, 
							(DWORD)SearchSubclassWndProc );    

			
			MapWindowPoints(hwnd, 
							GetParent(hwnd), 
							(LPPOINT)&rect, 
							2);

			MapWindowPoints(hwndPrevChild, 
							GetParent(hwnd), 
							(LPPOINT)&rectPrevChild, 
							2);
			height = rect.bottom - rect.top;

			rect.top	= rectPrevChild.bottom + 1;
			rect.right	= rectDefaultBox.right;
			rect.bottom = rect.top + height;

			MoveWindow(	hwnd, 
						rect.left, 
						rect.top, 
						rect.right - rect.left,
						rect.bottom - rect.top, 
						TRUE);

			
			return TRUE;
		}

		case WM_SIZE:
		{
			RECT rectControl, rectDefaultBox;
			HWND hwndControl, hwndDefaultBox;

			// move the Boxes

			hwndDefaultBox = GetDlgItem(GetParent(hwnd), 
										IDC_SPECIFIER_COMBO);
			GetWindowRect(hwndDefaultBox, &rectDefaultBox);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rectDefaultBox, 2);

			hwndControl = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO);
			GetWindowRect(hwndControl, &rectControl);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						rectControl.top,
						rectDefaultBox.right - rectDefaultBox.left,
						rectControl.bottom - rectControl.top,
						TRUE);

			hwndControl = GetDlgItem(hwnd, IDC_SPECIFIER_EDIT);
			GetWindowRect(hwndControl, &rectControl);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						rectControl.top,
						rectDefaultBox.right - rectDefaultBox.left,
						rectControl.bottom - rectControl.top,
						TRUE);

			hwndControl = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO_LIST);
			GetWindowRect(hwndControl, &rectControl);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						rectControl.top,
						rectDefaultBox.right - rectDefaultBox.left,
						rectControl.bottom - rectControl.top,
						TRUE);
			
			return TRUE;
		}

		case WM_COMMAND:
		{ 
			switch( LOWORD(wParam) )
			{
				case IDC_ATTRIBUTE:
				{
					HANDLE_IDC_ATTRIBUTE(hwnd, msg, wParam, lParam);
					break;
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}



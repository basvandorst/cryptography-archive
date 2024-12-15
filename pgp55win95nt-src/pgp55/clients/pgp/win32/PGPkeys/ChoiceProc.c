/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	 

	$Id: ChoiceProc.c,v 1.6 1997/09/08 22:38:04 elrod Exp $
____________________________________________________________________________*/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "pgpKeys.h"

#include "PGPcmdlg.h"
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

			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO);

			GetClientRect(hwndBox, &rect);
			MapWindowPoints(hwndBox, hwnd, (LPPOINT)&rect, 2);

			hwndTime = PGPcomdlgCreateTimeDateControl(	
												hwnd,	
												g_hInst,
												rect.left,
												rect.top,
												PGPCOMDLG_DISPLAY_DATE);
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



/*

  {
			LPVOID lpMessageBuffer;

			FormatMessage(
			  FORMAT_MESSAGE_ALLOCATE_BUFFER |
			  FORMAT_MESSAGE_FROM_SYSTEM,
			  NULL,
			  GetLastError(),
			  //The user default language
			  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			  (LPTSTR) &lpMessageBuffer,
			  0,
			  NULL );

			// now display this string
 			MessageBox(NULL, (char*)lpMessageBuffer, 0, MB_OK);

			// Free the buffer allocated by the system
			LocalFree( lpMessageBuffer );
		}

  */
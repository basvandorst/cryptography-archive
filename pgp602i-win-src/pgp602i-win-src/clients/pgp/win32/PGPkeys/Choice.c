/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.  
   
	$Id: Choice.c,v 1.14 1998/08/11 15:19:57 pbj Exp $
____________________________________________________________________________*/
#include <windows.h>  
#include <windowsx.h>
#include <assert.h>
 
#include "pgpMem.h"  
 
#include "Choice.h"
#include "ChoiceProc.h"
#include "resource.h" 
   
 
PCHOICE NewChoice(HWND hwndParent)
{
	PCHOICE choice = NULL;
	PCHOICE ChoiceList = NULL;
	HWND hwndChoice = NULL;  
	HWND hwndSpec = NULL; 
	RECT rectParent;
 
	ChoiceList = (PCHOICE)GetProp(hwndParent, "ChoiceList");

	choice = (PCHOICE) malloc( sizeof(CHOICE) );
	    
	if(choice)  
	{    
		HWND hwndPrevChoice = NULL;     
 
		if(ChoiceList)
		{ 
			hwndPrevChoice = ChoiceList->hwnd; 
		} 
   
		memset(choice, 0x00, sizeof(choice));  
 
		GetClientRect(hwndParent, &rectParent);

		hwndChoice = CreateDialogParam(	GetModuleHandle( NULL ), 
										MAKEINTRESOURCE( IDD_CHOICE ), 
						 				hwndParent, 
										(DLGPROC) ChoiceProc, 
										(LPARAM) hwndPrevChoice);

		choice->hwnd = hwndChoice; 
		choice->next = ChoiceList;
		ChoiceList = choice;
		SetProp( hwndParent, "ChoiceList",(HANDLE) ChoiceList);
	}
 
	return choice;
} 

PCHOICE RemoveLastChoice(HWND hwndParent, HWND* hwndChoice)
{ 
	PCHOICE choice = NULL;  
	PCHOICE ChoiceList = NULL;

	ChoiceList = (PCHOICE)GetProp(hwndParent, "ChoiceList");

	if(ChoiceList)
	{
		choice = ChoiceList->next;

		*hwndChoice =ChoiceList->hwnd;

		free(ChoiceList);

		ChoiceList = choice;
		SetProp( hwndParent, "ChoiceList",(HANDLE) ChoiceList);
	}

	return choice;
}

int	ChoiceCount(HWND hwndParent)
{
	PCHOICE choice = NULL;
	int count = 0;

	choice = (PCHOICE)GetProp(hwndParent, "ChoiceList");

	while( choice )
	{
		count++;
		choice = choice->next; 
	}

	return count;
}

PCHOICE	FirstChoice(HWND hwndParent)
{
	return (PCHOICE)GetProp(hwndParent, "ChoiceList");;
}

PCHOICE NextChoice(HWND hwndParent, PCHOICE choice)
{
	assert(choice);

	if(choice)
	{
		return choice->next;
	}
	else
	{
		return NULL;
	}
}

LRESULT 
CALLBACK 
SearchSubclassWndProc(HWND hwnd, 
					UINT msg, 
					WPARAM wParam, 
					LPARAM lParam)
{
	WNDPROC oldproc = (WNDPROC)GetProp(hwnd, "oldproc");
	HWND hwndNotify = GetProp(hwnd, "hwnd");

	switch(msg)
	{
		case WM_DESTROY: 
		{
			// put back old window proc
			SetWindowLong( hwnd, GWL_WNDPROC, (DWORD)oldproc );

			// remove props
			RemoveProp( hwnd, "oldproc" ); 
			break;
		}

		case WM_GETDLGCODE :
		{
			LPMSG	lpmsg = (LPMSG)lParam;

			if (lpmsg) 
			{
				if (lpmsg->message == WM_KEYDOWN) 
				{
					if (lpmsg->wParam == VK_RETURN) 
					{
						return DLGC_WANTMESSAGE;
					}
				}
			}
			break;
		}

		case WM_KEYDOWN:
		{
			int nVirtKey = (int) wParam;    // virtual-key code 
			long lKeyData = lParam;          // key data 

			if(nVirtKey == VK_RETURN)
			{
				HWND hwndButton;

				hwndButton = GetDlgItem (hwndNotify, IDC_SEARCH);

				if (IsWindowEnabled (hwndButton) && 
					IsWindowVisible (hwndButton))
				{
					SendMessage(hwndNotify, 
							WM_COMMAND, IDC_SEARCH, (LPARAM)hwndButton);
				}
				else 
				{
					hwndButton = GetDlgItem (hwndNotify, IDC_STOP);
					if (IsWindowEnabled (hwndButton) && 
						IsWindowVisible (hwndButton))
					{
						SendMessage(hwndNotify, 
								WM_COMMAND, IDC_STOP, (LPARAM)hwndButton);
					}
				}
				return 0;
			}

			break;
		}

		case WM_CHAR:
		{
			TCHAR chCharCode = (TCHAR) wParam;    // character code 
			long lKeyData = lParam;              // key data 

			if(chCharCode == 0x0A || chCharCode == 0x0D)
			{
				return 0;
			}

			break;
		}
		
	}

	return CallWindowProc(oldproc, hwnd, msg, wParam, lParam);
}


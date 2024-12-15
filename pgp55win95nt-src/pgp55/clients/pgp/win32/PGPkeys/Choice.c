/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	 
	  
   
	$Id: Choice.c,v 1.12 1997/09/08 22:38:00 elrod Exp $
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
	HWND hwndEdit = NULL; 
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
 
		hwndEdit = GetDlgItem(hwndChoice, IDC_SPECIFIER_EDIT);
 
		SetProp(hwndEdit,"hwnd", hwndParent );  

		SetProp(hwndEdit,  
				"oldproc",
				(HANDLE)GetWindowLong( hwndEdit, GWL_WNDPROC ) ); 


		SetWindowLong(	hwndEdit,  
						GWL_WNDPROC, 
						(DWORD)EditSubclassWndProc );    

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
EditSubclassWndProc(HWND hwnd, 
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

		case WM_KEYDOWN:
		{
			int nVirtKey = (int) wParam;    // virtual-key code 
			long lKeyData = lParam;          // key data 
			HWND hwndNext = NULL;

			if(nVirtKey == VK_RETURN)
			{
				SendMessage(hwndNotify, WM_COMMAND, IDC_SEARCH, 0);
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


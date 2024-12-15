/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: SearchCommon.c,v 1.15 1997/08/29 22:04:06 elrod Exp $
____________________________________________________________________________*/
#include <windows.h>  
#include <windowsx.h>
#include <commctrl.h>

#include "pgpKeys.h"
#include "pgpErrors.h"

#include "PGPcmdlg.h"

#include "Search.h"
#include "Choice.h"
#include "SearchFilter.h"
#include "SearchCommon.h"
#include "pgpKeyServerPrefs.h"
#include "resource.h"

void HANDLE_IDC_ATTRIBUTE(	HWND hwnd, 
							UINT msg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	WORD wNotifyCode			= HIWORD(wParam); // notification code 
	HWND hwndCtl				= (HWND) lParam;  // handle of control 
	static BOOL bCancel			= FALSE;
	static BOOL bChange			= FALSE;


/*	if( wNotifyCode == CBN_DROPDOWN )
	{
		bChange = FALSE;
		bCancel = FALSE;
	}

	if( wNotifyCode == CBN_SELENDCANCEL )
	{
		bCancel = TRUE;
	}
	else if( wNotifyCode == CBN_SELENDOK && !bCancel)
	{
		bChange = TRUE;
		bCancel = FALSE;
	}*/

	if( wNotifyCode == CBN_SELCHANGE /*&& bChange*/)
	{
		ChangeAttributeSelection(hwndCtl);
		bChange = FALSE;
	}
}
	


void InitializeControlValues(HWND hwnd, int defaultSelection)
{
	HINSTANCE hinst		= GetModuleHandle(NULL);
	HWND hwndBox		= NULL;
	HWND hwndEdit		= NULL;
	HWND hwndSpin		= NULL;
	HWND hwndMonth		= NULL;
	HWND hwndDay		= NULL;
	char szString[256]	= {0x00};
	char* cp			= NULL;
	char* end			= NULL;
	int i				= 0;

	hwndBox = GetDlgItem(hwnd, IDC_LOCATION);

	if( hwndBox ) // only done for parent search window
	{
		PSEARCHPATH			searchPath;
		PGPPrefRef			prefRef;
		PGPKeyServerEntry*	keyserverList;
		PGPUInt32			keyserverCount;
		PGPError			error;

		// load keyserverprefs
		error = (PGPError)PGPcomdlgOpenClientPrefs (&prefRef);

		if (error == kPGPError_NoErr)
		{
			error = PGPGetKeyServerPrefs(	prefRef,
											&keyserverList,
											&keyserverCount);

			if( IsntPGPError(error) )
			{
				searchPath = (PSEARCHPATH) malloc(sizeof(SEARCHPATH));
				memset(searchPath, 0x00, sizeof(SEARCHPATH));

				searchPath->prefRef			= prefRef;
				searchPath->keyserverList	= keyserverList;
				searchPath->keyserverCount	= keyserverCount;


				LoadString(	hinst, 
							IDS_DEFAULT_PATH, 
							szString, 
							sizeof(szString));

				i = ComboBox_AddString(hwndBox, szString);

				ComboBox_SetItemData(hwndBox, i, (LPARAM)searchPath);
			}
		}

		LoadString(hinst, IDS_LOCAL_KEYRING, szString, sizeof(szString));
		i = ComboBox_AddString(hwndBox, szString);
		ComboBox_SetItemData(hwndBox, i, NULL);

		LoadString(hinst, IDS_CURRENT_SEARCH, szString, sizeof(szString));
		i = ComboBox_AddString(hwndBox, szString);
		ComboBox_SetItemData(hwndBox, i, NULL);

		// place listed strings in box if prefs were loaded
		if( IsntPGPError(error) )
		{
			PGPUInt32	index;

			for(index = 0; index < keyserverCount; index++)
			{
				if( IsKeyServerListed( keyserverList[index].flags) )
				{
					i = ComboBox_AddString(	hwndBox, 
											keyserverList[index].serverURL);
					ComboBox_SetItemData(hwndBox, i, (LPARAM)-1);	
				}
			}
		}
	
		SetProp( hwndBox, "LastSearchType",(HANDLE) 0);

		ComboBox_SetCurSel(hwndBox, 0);
	}

	hwndBox = GetDlgItem(hwnd, IDC_ATTRIBUTE);

	for(i = IDS_ATTRIBUTE_1; i < IDS_ATTRIBUTE_LAST; i++)
	{
		LoadString(hinst, i, szString, sizeof(szString));
		
		ComboBox_AddString(hwndBox, szString);
	}

	ComboBox_SetCurSel(hwndBox, defaultSelection);

	ChangeAttributeSelection(hwndBox);
}

void ChangeAttributeSelection(HWND hwndCtl)
{
	int index					= ComboBox_GetCurSel(hwndCtl);
	HINSTANCE hinst				= GetModuleHandle(NULL);
	HWND hwndParent				= GetParent(hwndCtl);
	HWND hwndBox				= NULL;
	HWND hwndEdit				= NULL;
	HWND hwndTime				= GetProp(hwndParent, "hwndTime");
	char* cp					= NULL;
	char* end					= NULL;
	char szString[256]			= {0x00};

	hwndBox  = GetDlgItem(hwndParent, IDC_VERB);
	ComboBox_ResetContent(hwndBox);

	LoadString(	hinst, 
				index + IDS_VERBS_ATTRIBUTE_1, 
				szString, 
				sizeof(szString));

	end = cp = szString;

	while( cp && *cp )
	{
		while(end && *end && *end != ',')
		{
			end++;
		}

		if(end)
		{
			*end = 0x00;
			ComboBox_AddString(hwndBox, cp);
			end++;
			cp = end;
		}
	}

	if(index == 0) // User Id
	{
		ComboBox_SetCurSel(hwndBox, 2);
	}
	else
	{
		ComboBox_SetCurSel(hwndBox, 0);
	}

	hwndBox = GetDlgItem(	hwndParent, 
							IDC_SPECIFIER_COMBO);
	hwndEdit = GetDlgItem(	hwndParent, 
							IDC_SPECIFIER_EDIT);

	ComboBox_ResetContent(hwndBox);
	LoadString(	hinst, 
				index + IDS_SPECIFIERS_ATTRIBUTE_1, 
				szString, 
				sizeof(szString));

	/*char szDebug[256];
	wsprintf(szDebug, "szString = \'%s\'\r\n",szString);
	OutputDebugString(szDebug);*/

	if(*szString != 0x20)
	{
		if(!strcmp(szString, "Date"))
		{
			ShowWindow(hwndBox,	 SW_HIDE);
			ShowWindow(hwndEdit, SW_HIDE);
			ShowWindow(hwndTime, SW_SHOW);
		}
		else
		{
			ShowWindow(hwndBox,  SW_SHOW);
			ShowWindow(hwndEdit, SW_HIDE);
			ShowWindow(hwndTime, SW_HIDE);

			end = cp = szString;

			while( cp && *cp )
			{
				while(end && *end && *end != ',')
				{
					end++;
				}

				if(end)
				{
					*end = 0x00;
					ComboBox_AddString(hwndBox, cp);
					end++;
					cp = end;
				}
			}

			ComboBox_SetCurSel(hwndBox, 0);
		}
	}
	else
	{
		ShowWindow(hwndBox,  SW_HIDE);
		ShowWindow(hwndEdit, SW_SHOW);
		ShowWindow(hwndTime, SW_HIDE);
	}


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
/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.	

	$Id: SearchCommon.c,v 1.28 1998/08/11 15:20:17 pbj Exp $
____________________________________________________________________________*/
#include <windows.h>  
#include <windowsx.h>
#include <commctrl.h>

#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpKeyServerPrefs.h"
#include "pgpClientPrefs.h"

#include "PGPcl.h"

#include "Search.h"
#include "Choice.h"
#include "SearchFilter.h"
#include "SearchCommon.h"

#include "resource.h"

extern PGPContextRef g_Context;

void HANDLE_IDC_ATTRIBUTE(	HWND hwnd, 
							UINT msg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	WORD wNotifyCode			= HIWORD(wParam); // notification code 
	HWND hwndCtl				= (HWND) lParam;  // handle of control 
	static BOOL bCancel			= FALSE;
	static BOOL bChange			= FALSE;

	if( wNotifyCode == CBN_SELCHANGE /*&& bChange*/)
	{
		ChangeAttributeSelection(hwndCtl);
		bChange = FALSE;
	}
}
	
void ReloadKeyserverList(HWND hwnd)
{
	HINSTANCE hinst			= GetModuleHandle(NULL);
	HWND hwndBox			= NULL;
	char szString[512]		= {0x00};
	LPARAM lastSearch		= 0;
	int	lastSearchIndex		= 0;
	LPARAM localKeyset		= 0;
	int	localKeysetIndex	= 0;
	LPARAM searchPathInfo	= 0;
	int searchPathIndex		= 0;
	char szCurSel[512]		= {0x00};
	PSEARCHPATH path		= NULL;

	hwndBox = GetDlgItem(hwnd, IDC_LOCATION);

	// get text of current selection
	ComboBox_GetText(hwndBox, szCurSel, sizeof(szCurSel));

	if( hwndBox ) 
	{
		int index = 0;

		LoadString(hinst, IDS_CURRENT_SEARCH, szString, sizeof(szString));
		lastSearchIndex = ComboBox_FindString(hwndBox, -1, szString);

		LoadString(hinst, IDS_LOCAL_KEYRING, szString, sizeof(szString));
		localKeysetIndex = ComboBox_FindString(hwndBox, -1, szString);

		lastSearch = ComboBox_GetItemData(hwndBox, lastSearchIndex);
		localKeyset = ComboBox_GetItemData(hwndBox, localKeysetIndex);

		// delete old combo entries
		ComboBox_ResetContent(hwndBox);

		// free the prefs struct I am using
		path = (PSEARCHPATH) GetProp(hwnd, "SearchPath");

		if(path)
		{
			if (path->keyserverList) 
				PGPDisposePrefData (path->prefRef, path->keyserverList);

			if (path->prefRef)
				PGPclCloseClientPrefs (path->prefRef, FALSE);

			free(path);
		}
	}

	if( hwndBox ) 
	{
		PSEARCHPATH			searchPath;
		PGPPrefRef			prefRef;
		PGPKeyServerEntry*	keyserverList;
		PGPUInt32			keyserverCount;
		PGPError			error;
		int					i;

		// load keyserverprefs
		error = PGPclOpenClientPrefs (
					PGPGetContextMemoryMgr (g_Context),
					&prefRef);

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

				SetProp(hwnd, "SearchPath", (HANDLE)searchPath);
			}
		}

		LoadString(hinst, IDS_LOCAL_KEYRING, szString, sizeof(szString));
		i = ComboBox_AddString(hwndBox, szString);
		ComboBox_SetItemData(hwndBox, i, localKeyset);

		LoadString(hinst, IDS_CURRENT_SEARCH, szString, sizeof(szString));
		i = ComboBox_AddString(hwndBox, szString);
		ComboBox_SetItemData(hwndBox, i, lastSearch);

		

		// place listed strings in box if prefs were loaded
		if( IsntPGPError(error) )
		{
			PGPUInt32	index;

			for(index = 0; index < keyserverCount; index++)
			{
				if( IsKeyServerListed( keyserverList[index].flags) )
				{
					PGPGetKeyServerURL(&(keyserverList[index]), szString);

					i = ComboBox_AddString(	hwndBox, 
											szString);

					ComboBox_SetItemData(hwndBox, i, 
										(LPARAM)&(keyserverList[index]));	
				}
			}
		}

		// restore previous selection, if still available
		ComboBox_SetCurSel(hwndBox, 2);
		ComboBox_SelectString(hwndBox, 0, szCurSel);
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
	char szString[512]	= {0x00};
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
		PGPUInt32			selection = 2;

		// load keyserverprefs
		error = PGPclOpenClientPrefs (
					PGPGetContextMemoryMgr (g_Context),
					&prefRef);

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
				
				SetProp(hwnd, "SearchPath", (HANDLE)searchPath);

				PGPGetPrefNumber (	prefRef,
							kPGPPrefLastSearchWindowSelection, 
							(PGPUInt32*)&selection);
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
					PGPGetKeyServerURL(&(keyserverList[index]), szString);

					i = ComboBox_AddString(	hwndBox, 
											szString);

					ComboBox_SetItemData(hwndBox, i, 
									(LPARAM)&(keyserverList[index]));	
				}
			}
		}
	
		SetProp( hwndBox, "LastSearchType",(HANDLE) 0);

		ComboBox_SetCurSel(hwndBox, selection);
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
	HWND hwndBoxList			= NULL;
	HWND hwnd					= NULL;
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

	LoadString(	hinst, 
				index + IDS_SPECIFIERS_ATTRIBUTE_1, 
				szString, 
				sizeof(szString));

	hwndBox = GetDlgItem(	hwndParent, 
							IDC_SPECIFIER_COMBO);
	hwndBoxList = GetDlgItem(	hwndParent, 
							IDC_SPECIFIER_COMBO_LIST);
	hwndEdit = GetDlgItem(	hwndParent, 
							IDC_SPECIFIER_EDIT);

	ComboBox_ResetContent(hwndBox);
	ComboBox_ResetContent(hwndBoxList);

	if(*szString != 0x20)
	{
		if(!strcmp(szString, "Date"))
		{
			ShowWindow(hwndBox,		SW_HIDE);
			ShowWindow(hwndBoxList,	SW_HIDE);
			ShowWindow(hwndEdit,	SW_HIDE);
			ShowWindow(hwndTime,	SW_SHOW);
		}
		else
		{
			ShowWindow(hwndEdit, SW_HIDE);
			ShowWindow(hwndTime, SW_HIDE);

			end = cp = szString;

			if (*end == '*') 
			{
				ShowWindow(hwndBox,		SW_HIDE);
				ShowWindow(hwndBoxList,	SW_SHOW);
				hwnd = hwndBoxList;
				end += 2;
				cp = end;
			}
			else 
			{
				ShowWindow(hwndBox,		SW_SHOW);
				ShowWindow(hwndBoxList,	SW_HIDE);
				hwnd = hwndBox;
			}

			while( cp && *cp )
			{
				while(end && *end && *end != ',')
				{
					end++;
				}

				if(end)
				{
					*end = 0x00;
					ComboBox_AddString(hwnd, cp);
					end++;
					cp = end;
				}
			}

			ComboBox_SetCurSel(hwnd, 0);
		}
	}
	else
	{
		ShowWindow(hwndBox,		SW_HIDE);
		ShowWindow(hwndBoxList,	SW_HIDE);
		ShowWindow(hwndEdit,	SW_SHOW);
		ShowWindow(hwndTime,	SW_HIDE);
	}


}


/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>

#include <stdio.h>

#define PGPLIB
#define KEYSERVER_DLL

#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpkeyserversupport.h"

#include "winids.h"
#include "resource.h"
#include "menus.h"
#include "getkeys.h"
#include "keyserver.h"

KSERR PGPExport PGPkeyserverBuildMenu(unsigned long FunctionsDesired,
						  HMENU hMenu,
						  UINT Position);

KSERR PGPExport PGPkeyserverBuildSubMenu(unsigned long FunctionsDesired,
							 HMENU hParentMenu,
							 HMENU hSubMenu,
							 UINT SubMenuPosition)
{
	KSERR ReturnCode = SUCCESS;

	assert(hParentMenu);
	assert(hSubMenu);


	if(hParentMenu && hSubMenu)
	{
		InsertMenu(hParentMenu,
				    SubMenuPosition,
				   MF_BYPOSITION | MF_POPUP,
				    (UINT) hSubMenu,
				    "Keyserver");

		ReturnCode = PGPkeyserverBuildMenu(FunctionsDesired, hSubMenu,
											0xFFFFFFFF);
	}
	else
		ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}

KSERR PGPExport PGPkeyserverBuildMenu(unsigned long FunctionsDesired,
						  HMENU hMenu,
						  UINT Position)
{
	UINT ReturnCode = SUCCESS;

	assert(hMenu);

	if(!((FunctionsDesired & KS_NO_GET_KEYS) == KS_NO_GET_KEYS))
	{
		InsertMenu(hMenu,
				   Position,
				   MF_BYPOSITION | MF_STRING,
				   ID_GET_KEY,
				   "Get Selected Keys");
	}

	if(!((FunctionsDesired & KS_NO_SEND_KEYS) == KS_NO_SEND_KEYS))
	{
		InsertMenu(hMenu,
				   Position,
				   MF_BYPOSITION | MF_STRING,
				   ID_SEND_KEYS_TO_SERVER,
				   "Send Selected Keys");
	}

	if(!((FunctionsDesired & KS_NO_GET_NEW_KEY) == KS_NO_GET_NEW_KEY))
	{
		InsertMenu(hMenu,
				   Position,
				   MF_BYPOSITION | MF_STRING,
				   ID_GET_NEW_KEY,
				   "Find New Key");
	}
	return(ReturnCode);
}

KSERR PGPExport PGPkeyserverProcessMessage(HWND hwnd,
								 unsigned long CommandId,
								 char *SelectedKeys,
								 BOOL *UpdatedKeys,
								 PGPKeySet *pSourceKeyRing,
								 PGPKeySet *pDestKeyRing)
{
	KSERR ReturnCode = SUCCESS;
	unsigned long NumFound = 0;

	switch(CommandId)
	{
		case ID_GET_KEY:
		{
			ReturnCode = PGPkeyserverGetAndAddKeys(hwnd,
												   SelectedKeys,
												   pSourceKeyRing,
												   pDestKeyRing,
												   &NumFound);
			if(NumFound)
				*UpdatedKeys = TRUE;
		}
		break;

		case ID_GET_NEW_KEY:
		{
			char EmailAddress[255];

			if(DialogBoxParam(g_hinst,
							  MAKEINTRESOURCE(IDD_GET_EMAIL),
							  hwnd,
							  GetEmailDlgProc,
							  (LPARAM) EmailAddress))
			{
				ReturnCode = PGPkeyserverGetAndAddKeys(hwnd,
													    EmailAddress,
													    NULL,
													    pDestKeyRing,
													    &NumFound);
				if(NumFound)
					*UpdatedKeys = TRUE;
			}
		}
		break;

		case ID_SEND_KEYS_TO_SERVER:
		{
			*UpdatedKeys = FALSE;
			ReturnCode = PGPkeyserverPutKeys(hwnd, SelectedKeys,
											pSourceKeyRing);
			break;
		}
	}

	return(ReturnCode);
}

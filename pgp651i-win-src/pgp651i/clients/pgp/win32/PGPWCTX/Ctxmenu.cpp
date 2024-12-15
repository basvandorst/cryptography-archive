/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Ctxmenu.cpp,v 1.27 1999/04/13 17:29:53 wjb Exp $
____________________________________________________________________________*/

//  MODULE:   ctxmenu.cpp
//
//  PURPOSE:   Implements the IContextMenu member 
//             functions necessary to support
//             the context menu portions of this 
//             shell extension.  Context menu
//             shell extensions are called when 
//             the user right clicks on a file
//             (of the type registered for the 
//             shell extension--see SHELLEXT.REG
//             for details on the registry entries.
//             In this sample, the relevant
//             files are of type .GAK) in the 
//             Explorer, or selects the File menu 
//             item.
//
#include "precomp.h"

#include <tchar.h>
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <stdio.h>

extern HMENU		hPlaintextMenu;
extern HMENU		hEncryptedMenu;
extern HMENU		hKeyfileMenu;
extern HINSTANCE	g_hmodThisDll; // Handle to this DLL itself.
extern void *PGPsc;
extern void *PGPtls;

char szApp[100];

#include "resource.h"
#include "priv.h"
#include "pgpwctx.hpp"
#include "pgpCodes.hpp"

#define NONCONCURRENT

HWND g_hwndShell=NULL;

typedef struct _pgpthreadcontrol
{
	UINT ActionCode;
	STGMEDIUM Medium;
#ifdef NONCONCURRENT
	HANDLE hPGPCTXSemaphore;
#endif
}PGPTHREADCONTROL;

#define STACK_SIZE 0x8000
#define PGPCTX_SEMAPHORE  "PGP_50_CONTEXT_MENU_SEMAPHORE"

void CallPGPThread(void *Arg);
HRESULT CallPGP(STGMEDIUM *pMedium, UINT ActionCode);

extern HINSTANCE g_hmodThisDll;
extern HANDLE hPGPThreadMutex;

//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//
LRESULT CALLBACK InvisiProc(HWND hwnd, UINT msg, 
							WPARAM wParam, LPARAM lParam);


STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{
    ODS("CShellExt::QueryContextMenu()\r\n");

	DWORD FileAttr;
    char szMenuText[64] = "";
    char szExt[64];
	char * pExt;
	char * pSlash;
    BOOL bAppendItems=TRUE;
	int i = 0;
	BOOL bDirectory=FALSE;
	UINT nDriveType;
	char szDrive[5];
	char szDir[MAX_PATH];
	char szFname[MAX_PATH];

	hParentMenu = hMenu;	// save for DeleteMenu() in Release()

	// return if we don't have a file name
	if (m_szFileUserClickedOn[0] == '\0')
        return ResultFromShort(0); //return zero items added
	// no context menu entry for folders and system files

	_splitpath(m_szFileUserClickedOn, szDrive, szDir, szFname, szExt);
	nDriveType = GetDriveType(szDrive);

	// We'll allow network drives now...(nDriveType == DRIVE_REMOTE)
	if (nDriveType == DRIVE_CDROM)
		return ResultFromShort(0);

	// We'll allow network drives now...
//	if (!strncmp(m_szFileUserClickedOn, "\\\\", 2))
//		return ResultFromShort(0);

	FileAttr = GetFileAttributes(m_szFileUserClickedOn);

	if (FileAttr == 0xFFFFFFFF)
		return ResultFromShort(0);

	if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		bDirectory=TRUE;
	}

	if (FileAttr & FILE_ATTRIBUTE_SYSTEM)
        return ResultFromShort(0); //return zero items added

	// get file's extension
	pExt = strrchr(m_szFileUserClickedOn, '.');	
	// get last period in file name
	pSlash = strrchr(m_szFileUserClickedOn, '\\');	
	// get last backslash in file name
	if (!pExt || pSlash > pExt)	
	// if no period or slash follows period
		*szExt = '\0';
	else
		strcpy(szExt, &pExt[1]);		// copy file's extension w/o the period
	ODS(szExt);
	ODS("\r\n");
	
	if ((uFlags & 0x000F) == CMF_NORMAL)  //Check == here, since CMF_NORMAL=0
    {
        ODS("CMF_NORMAL...\r\n");
        strcpy(szMenuText, "P&GP");
    }
    else
        if (uFlags & CMF_VERBSONLY)
        {
            ODS("CMF_VERBSONLY...\r\n");
            strcpy(szMenuText, "P&GP");
        }
    else
        if (uFlags & CMF_EXPLORE)
        {
            ODS("CMF_EXPLORE...\r\n");
            strcpy(szMenuText, "P&GP");
        }
    else
        if (uFlags & CMF_DEFAULTONLY)
        {
            ODS("CMF_DEFAULTONLY...\r\n");
            bAppendItems = FALSE;
        }
    else
        {
            char szTemp[32];

            wsprintf(szTemp, "uFlags==>%d\r\n", uFlags);
            ODS("CMF_default...\r\n");
            ODS(szTemp);
            bAppendItems = FALSE;
        }

    if (bAppendItems)
    {
        if (_strnicmp(szExt, "pgp", 3) == 0 
			|| _strnicmp(szExt, "asc", 3) == 0)
		{
			CreateEncryptedMenu(idCmdFirst);
		}
		else
		{
			if(_strnicmp(szExt, "sig", 3) == 0)
			{
				CreateVerifyMenu(idCmdFirst);
			}
			else
			{
				if(_strnicmp(szExt, "bexpk", 5) == 0 ||
				   _strnicmp(szExt, "aexpk", 5) == 0 ||
				   _strnicmp(szExt, "pubkr", 5) == 0 ||
				   _strnicmp(szExt, "prvkr", 5) == 0 ||
				   _strnicmp(szExt, "pkr", 3) == 0 ||
				   _strnicmp(szExt, "skr", 3) == 0)
				{
					CreateKeyfileMenu(idCmdFirst);
				}
				else
				{
					if(bDirectory)
						CreateDirectoryMenu(idCmdFirst);
					else
						CreatePlaintextMenu(idCmdFirst);
				}
			}
		}

		BOOL InsertRet;

		int				iMenuItemCount = GetMenuItemCount(hMenu);
		int				iPosition      = 0xFFFFFFFF;
		MENUITEMINFO	MenuItemInfo;
		TCHAR			szStringInfo[255];

		for (int i = 0; i < iMenuItemCount; i++) {

			MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
			MenuItemInfo.fMask  = MIIM_TYPE;
			MenuItemInfo.wID    = 0;
			MenuItemInfo.hSubMenu     = NULL;
			MenuItemInfo.hbmpChecked  = NULL;
			MenuItemInfo.hbmpUnchecked= NULL;
			MenuItemInfo.dwItemData   = NULL;
			MenuItemInfo.dwTypeData   = szStringInfo;
			MenuItemInfo.cch		  = sizeof(szStringInfo);

			if (GetMenuItemInfo(hMenu, i, TRUE, &MenuItemInfo)) {
				if (MenuItemInfo.fType == MFT_STRING) {
					if (MenuItemInfo.dwTypeData && 
						!stricmp(MenuItemInfo.dwTypeData, "Se&nd To")) {
						// found Send To

						for (int j = i+1; j < iMenuItemCount; j++) {

							MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
							MenuItemInfo.fMask  = MIIM_TYPE | MIIM_DATA;

							if (GetMenuItemInfo(hMenu, j, TRUE, 
								&MenuItemInfo)) {
								if (MenuItemInfo.fType == MFT_SEPARATOR) {
									iPosition = j+1;
									break;
								}
							}
						}

						if (iPosition == 0xFFFFFFFF) {
	// no end separator was found, just Send To, so add a separator
							AddMenuSeparator(hMenu, i+1);
							iPosition = i+2;
						}
						break;
					}
				}
			}
		}

		InsertRet = InsertMenu(hMenu,
								iPosition,
								MF_STRING | MF_BYPOSITION | MF_POPUP,
								(UINT)hSubMenu,
								szMenuText);
		if (!InsertRet)
		{
			char EC[64];
			DWORD ErrorCode = GetLastError();
			_ltoa(ErrorCode, EC, 16);
			ODS("InsertMenu failed!\r\n");
			ODS(EC);
			ODS("\r\n");
			SubMenuItems = 0;
		}
		else
			ODS("InsertMenu succeeded!\r\n");
        
		if (iPosition != 0xFFFFFFFF) {
			AddMenuSeparator(hMenu, iPosition+1);
		}
        return ResultFromShort(SubMenuItems);
   }

   return NOERROR;
}

//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected one of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
	PGPTHREADCONTROL *pTC = NULL;
    ODS("CShellExt::InvokeCommand()\r\n");


	HRESULT hr = E_INVALIDARG;

    //If HIWORD(lpcmi->lpVerb) then we have been called programmatically
    //and lpVerb is a command that should be invoked.  Otherwise, the shell
    //has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
    //selected.  Actually, it's (menu ID - idCmdFirst) from 
	//QueryContextMenu().
	if (!HIWORD(lpcmi->lpVerb))
    {
        UINT idCmd = LOWORD(lpcmi->lpVerb);

		if((pTC = (PGPTHREADCONTROL *) malloc(sizeof(PGPTHREADCONTROL))))
		{
			pTC->Medium = medium;

#ifdef NONCONCURRENT
			HANDLE hPGPCTXSemaphore = NULL;
			BOOL DidCreate = FALSE;

			if(!(hPGPCTXSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS,
				FALSE, PGPCTX_SEMAPHORE)))
			{
				hPGPCTXSemaphore = CreateSemaphore(NULL, 0, 1, 
					PGPCTX_SEMAPHORE);
				DidCreate = TRUE;
			}

			if(hPGPCTXSemaphore)
			{
				if(DidCreate || WaitForSingleObject(hPGPCTXSemaphore, 0) != 
					WAIT_TIMEOUT)
				{
					pTC->hPGPCTXSemaphore = hPGPCTXSemaphore;
#endif
					if(!strcmp(SubMenuType, "Directory"))
					{
						switch (idCmd)
						{
							case 0:
								break;

							case 1:
								pTC->ActionCode = PGP_CODE_ENCRYPT;
								hr = NOERROR;
								break;

							case 2:
								pTC->ActionCode = PGP_CODE_SIGN;
								hr = NOERROR;
								break;

							case 3:
								pTC->ActionCode = PGP_CODE_ENCRYPT_SIGN;
								hr = NOERROR;
								break;

							case 4:
								pTC->ActionCode = PGP_CODE_DECRYPT;
								hr = NOERROR;
								break;

							case 5:
								pTC->ActionCode = PGP_CODE_WIPE;
								hr = NOERROR;
								break;
						}
					}
					else
					{
					switch (idCmd)
					{
						case 0:
							break;

						case 1:
						if (!strcmp(SubMenuType, "Encrypted"))
						{
							pTC->ActionCode = PGP_CODE_DECRYPT;
						}
						else if (!strcmp(SubMenuType, "Keyfile"))
						{
							pTC->ActionCode = PGP_CODE_ADD_KEYS;
						}
						else if (!strcmp(SubMenuType, "AddKeys"))
						{
							pTC->ActionCode = PGP_CODE_VIEW_KEYS;
						}
						else if(!strcmp(SubMenuType, "Verify"))
						{
							pTC->ActionCode = PGP_CODE_VERIFY_SIG;
						}
						else
						{
							pTC->ActionCode = PGP_CODE_ENCRYPT;
						}
						hr = NOERROR;
						break;

					case 2:
						if (!strcmp(SubMenuType, "Plaintext"))
						{
							pTC->ActionCode = PGP_CODE_SIGN;
						}
						else
						{
							pTC->ActionCode = PGP_CODE_WIPE;
						}
						hr = NOERROR;
						break;

					case 3:
						pTC->ActionCode = PGP_CODE_ENCRYPT_SIGN;
						hr = NOERROR;
						break;

					case 4:
						pTC->ActionCode = PGP_CODE_WIPE;
						hr = NOERROR;
						break;
					}
					}

					if(hr == NOERROR) //We found a good 'un!
					{
						hPGPThreadMutex = CreateMutex(NULL, TRUE, NULL);
						pTC->Medium = medium;
						_beginthread(CallPGPThread, STACK_SIZE, (void *) pTC);
						// encrypt
					}
#ifdef NONCONCURRENT
				}
				else /*We only get here on wait timeout.*/
				{
					MessageBox(NULL, 
					 "You may only encrypt/decrypt/verify one file at a time.",
					 "PGP Busy", MB_OK|MB_SETFOREGROUND|MB_ICONSTOP);
				}
			}
#endif
		}
    }
    return hr;
}

void CallPGPThread(void *Arg)
{
    /*Make our own local copy so that when 
	 *our parent expires, our data is still good:*/
	PGPTHREADCONTROL *pTC = (PGPTHREADCONTROL *) Arg;

	CallPGP(&(pTC->Medium), pTC->ActionCode);
#ifdef NONCONCURRENT
	ReleaseSemaphore(pTC->hPGPCTXSemaphore, 1, NULL);
	CloseHandle(pTC->hPGPCTXSemaphore);
#endif

	free(pTC);

	ReleaseMutex(hPGPThreadMutex);
	_endthread();
}

//
//  FUNCTION: CShellExt::GetCommandString(UINT idCmd,
//                                       UINT uFlags,
//                                       UINT FAR *reserved,
//                                       LPSTR pszName,
//                                       UINT cchMax)
//
//  PURPOSE: Called by the shell as the mouse passes over a menu item.
//				Displays a help string on the Explorer status bar.
//

STDMETHODIMP CShellExt::GetCommandString(UINT idCmd,
                                         UINT uFlags,
                                         UINT FAR *reserved,
                                         LPSTR pszName,
                                         UINT cchMax)
{
	char		 msg[256];
	OSVERSIONINFO Version;

    ODS("CShellExt::GetCommandString()\r\n");
	ODS("Max message length: ");
	ODS(_ltoa(cchMax, msg, 10));
	ODS("\r\n");

	if(!strcmp(SubMenuType, "Directory"))
	{
		switch (idCmd)
		{
			case 0:
			break;

			case 1:
				strcpy(msg, "Encrypt this file.");
				break;

			case 2:
				strcpy(msg,"Sign this file.");
				break;

			case 3:
				strcpy(msg, 
				"Encrypt this file and sign it with an encrypted signature.");
				break;

			case 4:
				strcpy(msg, 
				"Decrypt this encrypted file.");
				break;

			case 5:
				strcpy(msg, 
				  "Secure wipe this file.");
				break;
		}
	}
	else
	{
    switch (idCmd)
    {
        case 0:
			break;

        case 1:
            if (!strcmp(SubMenuType, "Encrypted"))
				strcpy(msg, "Decrypt this encrypted file.");
			else
            if (!strcmp(SubMenuType, "Keyfile"))
				strcpy(msg, 
				  "Add keys from this file to the default public keyring.");
			else
            if (!strcmp(SubMenuType, "AddKeys"))
				strcpy(msg, "Add the keys in this file to your keyring");
			else
				strcpy(msg, "Encrypt this file.");
            break;

        case 2:
            if (!strcmp(SubMenuType, "Plaintext"))
				strcpy(msg, 
				  "Sign this file.");
			else
				strcpy(msg, 
				  "Secure wipe this file.");
            break;

        case 3:
            strcpy(msg, 
				"Encrypt this file and sign it with an encrypted signature.");
            break;

        case 4:
            strcpy(msg, 
				"Secure wipe this file.");
            break;
    }
	}
    ODS(msg);
    ODS("\r\n");
	// return the help string
	// get OS type
	Version.dwOSVersionInfoSize = (DWORD)(sizeof Version);
	GetVersionEx(&Version);
	if (Version.dwPlatformId == VER_PLATFORM_WIN32_NT)	// if WIN NT
	{
		// first set empty string
		MultiByteToWideChar(CP_ACP, 0, "", -1, 
							(LPWSTR)pszName, cchMax / 2);
		// then convert string
		if (cchMax / 2 > strlen(msg))	// if room for message
		{
			if (!(MultiByteToWideChar(CP_ACP, 0, msg, -1, 
							(LPWSTR)pszName, cchMax / 2)))
			{
#ifdef _DEBUG
				char buf1[40];
#endif
				ODS("MultiByteToWideChar() failed.\r\n");
				ODS("Error Code is: ");
				ODS(_ltoa(GetLastError(), buf1, 10));
				ODS("\r\n");
			}
		}
		else
			ODS("Buffer is not large enough for message.\r\n");
#ifdef _DEBUG
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)pszName, 
						-1, msg, sizeof msg, NULL, NULL);
		ODS(msg);
		ODS("\r\n");
#endif
	}
	else	// WIN 95
	{
		// make sure the help string isn't too long
		// first set empty string
		strcpy(pszName, "");
		// then copy string if there's room
		if (cchMax > strlen(msg))	// if buffer larger than string
			strcpy(pszName, msg);
		else
			ODS("Buffer is not large enough for message.\r\n");
	    
		ODS(pszName);
		ODS("\r\n");
	}

	return NOERROR;
}

void CShellExt::CreateEncryptedMenu(UINT idCmdFirst)
{
	MENUITEMINFO ItemInfo;
	char MText[64];

	ItemInfo.cbSize = sizeof ItemInfo;
	ItemInfo.fMask = MIIM_TYPE | MIIM_ID;
	ItemInfo.fType = MFT_STRING;
	ItemInfo.wID = idCmdFirst;
	ItemInfo.hSubMenu = NULL;
	ItemInfo.hbmpChecked = NULL;
	ItemInfo.hbmpUnchecked = NULL;
	ItemInfo.dwItemData = 0;
	ItemInfo.dwTypeData = MText;
	ItemInfo.cch = 0;


	strcpy(SubMenuType, "Encrypted");
	hSubMenu = CreatePopupMenu();
/*	
	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE;
	ItemInfo.fState = MF_DISABLED;	// for title line
	strcpy(MText, "Pretty Good Privacy");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType = MFT_SEPARATOR;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
*/

	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	ItemInfo.fState = MFS_ENABLED;		// for command lines
	
	++ItemInfo.wID;
	strcpy(MText, "&Decrypt && Verify");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType=MFT_SEPARATOR ;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	ItemInfo.fType = MFT_STRING;

	++ItemInfo.wID;
	strcpy(MText, "&Wipe");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	
/*
	++ItemInfo.wID;
	strcpy(MText, "&Verify");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
*/
	
	SubMenuItems = 3;
}

void CShellExt::CreateVerifyMenu(UINT idCmdFirst)
{
	MENUITEMINFO ItemInfo;
	char MText[64];

	ItemInfo.cbSize = sizeof ItemInfo;
	ItemInfo.wID = idCmdFirst;
	ItemInfo.hSubMenu = NULL;
	ItemInfo.hbmpChecked = NULL;
	ItemInfo.hbmpUnchecked = NULL;
	ItemInfo.dwItemData = 0;
	ItemInfo.dwTypeData = MText;
	ItemInfo.cch = 0;


	strcpy(SubMenuType, "Verify");
	hSubMenu = CreatePopupMenu();

/*	
	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE;
	ItemInfo.fState = MF_DISABLED;	// for title line
	strcpy(MText, "Pretty Good Privacy");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType = MFT_SEPARATOR;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
*/

	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	ItemInfo.fState = MFS_ENABLED;		// for command lines
	
	++ItemInfo.wID;
	strcpy(MText, "&Verify Signature");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType=MFT_SEPARATOR ;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	ItemInfo.fType = MFT_STRING;

	++ItemInfo.wID;
	strcpy(MText, "&Wipe");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	
	SubMenuItems = 3;
}


void CShellExt::CreateKeyfileMenu(UINT idCmdFirst)
{
	MENUITEMINFO ItemInfo;
	char MText[64];

	ItemInfo.cbSize = sizeof ItemInfo;
	ItemInfo.wID = idCmdFirst;
	ItemInfo.hSubMenu = NULL;
	ItemInfo.hbmpChecked = NULL;
	ItemInfo.hbmpUnchecked = NULL;
	ItemInfo.dwItemData = 0;
	ItemInfo.dwTypeData = MText;
	ItemInfo.cch = 0;


	strcpy(SubMenuType, "Keyfile");
	hSubMenu = CreatePopupMenu();

/*	
	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE;
	ItemInfo.fState = MF_DISABLED;	// for title line
	strcpy(MText, "Pretty Good Privacy");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType = MFT_SEPARATOR;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
*/

	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	ItemInfo.fState = MFS_ENABLED;		// for command lines
	
	++ItemInfo.wID;
	strcpy(MText, "&Add Keys to Keyring");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType=MFT_SEPARATOR ;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	ItemInfo.fType = MFT_STRING;

	++ItemInfo.wID;
	strcpy(MText, "&Wipe");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	
	SubMenuItems = 3;
}

void CShellExt::CreateViewKeyringMenu(UINT idCmdFirst)
{
	MENUITEMINFO ItemInfo;
	char MText[64];

	ItemInfo.cbSize = sizeof ItemInfo;
	ItemInfo.wID = idCmdFirst;
	ItemInfo.hSubMenu = NULL;
	ItemInfo.hbmpChecked = NULL;
	ItemInfo.hbmpUnchecked = NULL;
	ItemInfo.dwItemData = 0;
	ItemInfo.dwTypeData = MText;
	ItemInfo.cch = 0;


	strcpy(SubMenuType, "AddKeys");
	hSubMenu = CreatePopupMenu();

/*	
	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE;
	ItemInfo.fState = MF_DISABLED;	// for title line
	strcpy(MText, "Pretty Good Privacy");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType = MFT_SEPARATOR;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

*/
	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	ItemInfo.fState = MFS_ENABLED;		// for command lines
	
	++ItemInfo.wID;
	strcpy(MText, "&Add Keys to Keyring");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType=MFT_SEPARATOR ;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	ItemInfo.fType = MFT_STRING;

	++ItemInfo.wID;
	strcpy(MText, "&Wipe");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	
	SubMenuItems = 3;
}

void CShellExt::CreateDirectoryMenu(UINT idCmdFirst)
{
	MENUITEMINFO ItemInfo;
	char MText[64];

	ItemInfo.cbSize = sizeof ItemInfo;
	ItemInfo.wID = idCmdFirst;
	ItemInfo.hSubMenu = NULL;
	ItemInfo.hbmpChecked = NULL;
	ItemInfo.hbmpUnchecked = NULL;
	ItemInfo.dwItemData = 0;
	ItemInfo.dwTypeData = MText;
	ItemInfo.cch = 0;

	strcpy(SubMenuType, "Directory");
	hSubMenu = CreatePopupMenu();
	
	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	ItemInfo.fState = MFS_ENABLED;		// for command lines
	
	++ItemInfo.wID;
	strcpy(MText, "&Encrypt");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	
	++ItemInfo.wID;
	strcpy(MText, "&Sign");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	
	++ItemInfo.wID;
	strcpy(MText, "E&ncrypt && Sign");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType=MFT_SEPARATOR ;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	ItemInfo.fType = MFT_STRING;

	++ItemInfo.wID;
	strcpy(MText, "D&ecrypt && Verify");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType=MFT_SEPARATOR ;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	ItemInfo.fType = MFT_STRING;

	++ItemInfo.wID;
	strcpy(MText, "&Wipe");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	SubMenuItems = 6;
}

void CShellExt::CreatePlaintextMenu(UINT idCmdFirst)
{
	MENUITEMINFO ItemInfo;
	char MText[64];

	ItemInfo.cbSize = sizeof ItemInfo;
	ItemInfo.wID = idCmdFirst;
	ItemInfo.hSubMenu = NULL;
	ItemInfo.hbmpChecked = NULL;
	ItemInfo.hbmpUnchecked = NULL;
	ItemInfo.dwItemData = 0;
	ItemInfo.dwTypeData = MText;
	ItemInfo.cch = 0;

	strcpy(SubMenuType, "Plaintext");
	hSubMenu = CreatePopupMenu();
	
	ItemInfo.fType = MFT_STRING;
	ItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	ItemInfo.fState = MFS_ENABLED;		// for command lines
	
	++ItemInfo.wID;
	strcpy(MText, "&Encrypt");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	
	++ItemInfo.wID;
	strcpy(MText, "&Sign");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	
	++ItemInfo.wID;
	strcpy(MText, "E&ncrypt && Sign");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	ItemInfo.fType=MFT_SEPARATOR ;
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);
	ItemInfo.fType = MFT_STRING;

	++ItemInfo.wID;
	strcpy(MText, "&Wipe");
	InsertMenuItem(hSubMenu, 0xffffffff, TRUE, &ItemInfo);

	SubMenuItems = 5;
}

LRESULT CALLBACK ShellProc(HWND hwnd, UINT msg, 
							   WPARAM wParam, LPARAM lParam)
{
	// See if user deselects caching via prefs
	CheckForPurge(msg,wParam);

	switch(msg)
	{
		case WM_CREATE:
		{
			ShowWindow(hwnd, SW_HIDE);
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

#define szAppName "PGPshell_HiddenWindow"

HWND ShellWindow(void)
{
	HWND hwnd;
	WNDCLASS wc;

	// Register the Server Window Class

	wc.style			= 0;
	wc.lpfnWndProc		= ShellProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= g_hmodThisDll;
	wc.hIcon			= 0;
	wc.hCursor			= 0;
	wc.hbrBackground	= 0;
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= szAppName;

	RegisterClass(&wc);

	hwnd = CreateWindow(
		szAppName, szAppName, WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, NULL, NULL, g_hmodThisDll, NULL );

	return hwnd;
}

HRESULT CallPGP(STGMEDIUM *pMedium, UINT ActionCode)
{
	int i;
	UINT ReturnCode;
	HWND hwnd;
	int NumFiles;
	BOOL bEncrypt,bSign;
	char szFile[MAX_PATH+1];
	FILELIST *ListHead;

	strcpy(szApp,"PGPshell");

	ReturnCode=TRUE;

	if(PGPsc==0)
	{
		if(!InitPGPsc(NULL,&PGPsc,&PGPtls))
			return(ReturnCode);

		// For getting global windows messages
		g_hwndShell=ShellWindow();
	}

	hwnd = GetForegroundWindow();

	NumFiles = DragQueryFile((HDROP) pMedium->hGlobal, (UINT)-1, 0, 0);

	if(NumFiles==0)
		return(ReturnCode);

	ReturnCode=TRUE;
	bEncrypt=bSign=FALSE;

	if(ActionCode==PGP_CODE_ENCRYPT)
		bEncrypt = TRUE;

	if(ActionCode==PGP_CODE_SIGN)
		bSign = TRUE;

	if(ActionCode==PGP_CODE_ENCRYPT_SIGN)
		bEncrypt = bSign = TRUE;

	if ((ActionCode==PGP_CODE_ENCRYPT)||
		(ActionCode==PGP_CODE_SIGN)   ||
		(ActionCode==PGP_CODE_ENCRYPT_SIGN))
	{
		ListHead=0;

		for(i=0;i<NumFiles && ReturnCode == TRUE;i++)
		{
			DragQueryFile((HDROP) pMedium->hGlobal, 
                         i,szFile,MAX_PATH);
			AddToFileList(&ListHead,szFile,NULL);
		}

		ReturnCode = EncryptFileList(hwnd,szApp,PGPsc,PGPtls,
			ListHead,
			bEncrypt, 
			bSign);
	}

	if((ActionCode==PGP_CODE_DECRYPT)||
	   (ActionCode==PGP_CODE_VERIFY_SIG)||
	   (ActionCode==PGP_CODE_VERIFY))
	{
		ListHead=0;

		for(i=0;i<NumFiles && ReturnCode == TRUE;i++)
		{
			DragQueryFile((HDROP) pMedium->hGlobal, 
                          i,szFile,MAX_PATH);
			AddToFileList(&ListHead,szFile,NULL);
		}

		ReturnCode = DecryptFileList(hwnd,szApp,PGPsc,PGPtls,
			ListHead);
	}

	if(ActionCode==PGP_CODE_WIPE)
	{
		DWORD FileAttr;

		ListHead=0;

		for(i=0;i<NumFiles && ReturnCode == TRUE;i++)
		{
			DragQueryFile((HDROP) pMedium->hGlobal, 
				i,szFile,MAX_PATH);
				
			FileAttr = GetFileAttributes(szFile);

			if (FileAttr == 0xFFFFFFFF)
				;
			else if (FileAttr & FILE_ATTRIBUTE_SYSTEM)
				;
			else if (FileAttr & FILE_ATTRIBUTE_READONLY)
				;
			else
				AddToFileList(&ListHead,szFile,NULL);
		}

		ReturnCode = WipeFileList(hwnd,PGPsc,
			ListHead,TRUE);
	}

	ReleaseStgMedium(pMedium);

	return ReturnCode;
}

BOOL CShellExt::AddMenuSeparator(HMENU hMenu, int iPosition)
{
	MENUITEMINFO MenuItemInfo;
	BOOL		 bRC;

	MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
	MenuItemInfo.fMask  = MIIM_TYPE;
	MenuItemInfo.fType  = MFT_SEPARATOR;

	bRC = InsertMenuItem(hMenu,
			  				   iPosition,
							   TRUE,
							   &MenuItemInfo);

	if (!bRC) {
		// wow, we couldn't add the separator after Send To...
		ODS("Insert Separator failed...\r\n");
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK InvisiProc(HWND hwnd, UINT msg, 
							WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			return 0;
		}

		case WM_QUERYOPEN:
		{
			return 0;
		}

#if 0		
		case WM_CLOSE:
		{	
			PostQuitMessage(0);
	
			return 0;
		}
#endif
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


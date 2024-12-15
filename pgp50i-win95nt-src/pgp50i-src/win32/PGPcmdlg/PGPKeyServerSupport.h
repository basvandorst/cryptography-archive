/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
#ifndef __PGPKEYSERVERSUPPORT_H
#define __PGPKEYSERVERSUPPORT_H

#define KS_NO_GET_KEYS		1
#define	KS_NO_SEND_KEYS		2
#define KS_NO_SETTINGS		4
#define KS_NO_GET_NEW_KEY	8
#define KS_ALL				0

#define	KS_ID_START			5300
#define KS_ID_END			5400

#define KS_ID_IS_IN_RANGE(x)  (x >= KS_ID_START && x <= KS_ID_END)

typedef unsigned long KSERR;

#define KS_ERR_BASE						3000
#ifndef SUCCESS
#define SUCCESS							0
#endif //SUCCESS

#define ERR_UNABLE_TO_LOAD_LIBRARY		KS_ERR_BASE + 0
#define ERR_NO_FUNCS_AVAILABLE			KS_ERR_BASE + 1
#define ERR_LIBRARY_NOT_LOADED			KS_ERR_BASE + 2
#define ERR_BAD_PARAM					KS_ERR_BASE + 3
#define ERR_FUNCTION_NOT_IN_LIB			KS_ERR_BASE + 4
#define	ERR_USER_NOT_FOUND				KS_ERR_BASE + 5
#define ERR_USER_CANCEL					KS_ERR_BASE + 6
#define	ERR_KEYRING_FAILURE				KS_ERR_BASE + 7

#ifndef KEYSERVER_DLL

/*How it's expected you'll use these:
 *
 *LoadKeyServer();
 *
 *[In Message Loop]
 *
 *	HMENU hPopup;
 *
 *  case WM_INITDIALOG:
 *  case WM_CREATEWINDOW:
 *	{
 *		if((hPopup = CreatePopupMenu()))
 *		{
 *			BuildMenu(KS_ALL, hPopup);
 *		}
 *		break;
 *	}
 *
 *	case WM_CONTEXTMENU:
 *	{
 *		TrackPopupMenu(hMenu, ...);
 *		break;
 *	}
 *
 *
 *	case WM_COMMAND:
 *	{
 *		switch(LOWORD(wParam))
 *		{
 *			default:
 *				BOOL UpdatedKeys;
 *				if(KS_ID_IS_IN_RANGE(LOWORD(wParam)))
 *				{
 *					ProcessKeyServerMessage((LOWORD(wParam)), GetSelectedKeys(),
 *											&UpdatedKeys);
 *					if(UpdatedKeys)
 *						UpdateDisplay();
 *				}
 *				break;
 *		}
 *		break;
 *	}
 *
 *[End Message Loop]
 *
 *UnLoadKeyServerSupport();
 *
 *Note that GetSelectedKeys() and UpdateDisplay() are the application
 *programmer's responsibility.
 */

/*Load the keyserver DLL.  Returns SUCCESS if all OK,
 *ERR_UNABLE_TO_LOAD_LIBRARY if it can't find the DLL, or
 *ERR_NO_FUNCS_AVAILABLE if it can find the DLL but can't find any useful
 *functions in it.
 */
KSERR LoadKeyserverSupport(void);

/*Unload the keyserver DLL.  Returns SUCCESS if all OK, or
 *ERR_LIBRARY_NOT_LOADED if you didn't have a previously successful call to
 *LoadKeyServerSupport().  You can probably ignore the return.
 */
KSERR UnLoadKeyserverSupport(void);

/*Builds a popup menu for the keyserver support.
 *
 *FunctionsDesired is made up of the bitwise flags KS_NO_GET_KEYS,
 *KS_NO_SEND_KEYS, KS_NO_SETTINGS or KS_ALL.  Position is the point in the
 *menu at which you want the keyserver functions to be inserted.
 *Use 0xFFFFFFFF if you wish them to be at the end.
 *
 *Returns:
 *
 *		SUCCESS					on all OK
 *		ERR_BAD_PARAM			if hMenu is NULL
 *		ERR_LIBRARY_NOT_LOADED	if LoadKeyserverSupport() hasn't been
 *								successfully called
 *		ERR_FUNCTION_NOT_IN_LIB if the DLL doesn't support this function
 */
KSERR BuildMenu(unsigned long FunctionsDesired, HMENU hMenu, UINT Position);

/*As above, but takes a second handle to a menu which is used to create a
 *sub-menu.  Same returns.  It's assumed you do the CreateMenu() and
 *FreeMenu() on both the parent and sub menus.
 */
KSERR BuildSubMenu(unsigned long FunctionsDesired,
				   HMENU hParentMenu,
				   HMENU hSubMenu,
				   UINT SubMenuPosition);


/*Processes a KeyServerMessage.  CommandId should be the LOWORD of wParam on
 *a WM_COMMAND message.  SelectedKeys should be a character string containing
 *a list of email addresses or keyids seperated by newlines.  UpdatedKeys is
 *a pointer to a boolean, which contains TRUE if the keyserver made any changes
 *to the key files, or FALSE if it did not.  NULL is acceptable for this
 *parameter if you don't care.  SelectedKeys may also be NULL if no keys are
 *selected or this metaphor is not appropriate for this application.
 *
 *Returns:
 *
 *		SUCCESS					on all OK
 *		ERR_LIBRARY_NOT_LOADED	if LoadKeyserverSupport() hasn't been
 *								successfully called
 *		ERR_FUNCTION_NOT_IN_LIB if the DLL doesn't support this function
 */
KSERR ProcessKeyServerMessage(HWND hwnd,
							  unsigned long CommandId,
							  char *SelectedKeys,
							  BOOL *UpdatedKeys);


KSERR SetupKeyServerSettingsPage(PROPSHEETPAGE *pPSP);

#endif //#ifdef KEYSERVER_DLL

#endif //#ifdef __PGPKEYSERVERSUPPORT_H

/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>

#include <stdio.h>

#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpkeyserversupport.h"

typedef int ( __cdecl *PGKS_BUILD_MENU)(unsigned long, HMENU, UINT);
typedef int ( __cdecl *PGKS_BUILD_SUB_MENU)(unsigned long, HMENU, HMENU, UINT);
typedef int ( __cdecl *PGKS_PROCESS_MSG)(HWND, unsigned long, char *, BOOL *,
										PGPKeySet *, PGPKeySet *);
typedef int ( __cdecl *PGKS_SETTINGS_PAGE)(PROPSHEETPAGE *);
typedef int ( __cdecl *PGKS_DISPLAY_KS_ERROR)(HWND hwnd, unsigned long);
typedef int ( __cdecl *PGKS_START_PUT_KEY_INFO_THREAD)(void *);
typedef int ( __cdecl *PGKS_GET_KEYSERVER_VALUES)(char *, unsigned short *,
													BOOL *);

typedef struct KeyserverFuncs
{
	HMODULE hKSDll;
	PGKS_BUILD_MENU BuildMenu;
	PGKS_BUILD_SUB_MENU BuildSubMenu;
	PGKS_PROCESS_MSG ProcessKeyServerMessage;
	PGKS_SETTINGS_PAGE SetupKeyServerSettingsPage;
	PGKS_DISPLAY_KS_ERROR DisplayKSError;
	PGKS_START_PUT_KEY_INFO_THREAD StartPutKeyInfoThread;
	PGKS_GET_KEYSERVER_VALUES GetKeyserverValues;
}KSPROCS;

static KSPROCS KSProcs = {NULL, NULL, NULL, NULL, NULL, NULL};

#define KEYSERVERDLL			"pgpks.dll"
#define KS_BUILD_MENU			"PGPkeyserverBuildMenu"
#define KS_PROCESS_MESSAGE		"PGPkeyserverProcessMessage"
#define KS_BUILD_SUB_MENU       "PGPkeyserverBuildSubMenu"
#define KS_SETTINGS_PAGE		"PGPkeyserverSetupKeyServerSettingsPage"
#define KS_DISPLAY_KS_ERROR		"DisplayKSError"
#define	KS_GET_KEYSERVER_VALUES	"GetKeyserverValues"
#define KS_START_PUT_KEY_INFO_THREAD "StartPutKeyInfoThread"


#define KS_GET_KEYS		1
#define	KS_SEND_KEYS	2
#define KS_SETTINGS		4

KSERR LoadKeyserverSupport(void)
{
	KSERR ReturnCode = SUCCESS;

	/*Load the library*/
	if((KSProcs.hKSDll = LoadLibrary(KEYSERVERDLL)))
	{
		/*Grab the functions:*/
		if((KSProcs.BuildMenu = (PGKS_BUILD_MENU)
				GetProcAddress(KSProcs.hKSDll, KS_BUILD_MENU)) &&
		   (KSProcs.BuildSubMenu = (PGKS_BUILD_SUB_MENU)
				GetProcAddress(KSProcs.hKSDll, KS_BUILD_SUB_MENU)) &&
		   (KSProcs.SetupKeyServerSettingsPage = (PGKS_SETTINGS_PAGE)
				GetProcAddress(KSProcs.hKSDll, KS_SETTINGS_PAGE)) &&
		   (KSProcs.ProcessKeyServerMessage = (PGKS_PROCESS_MSG)
				GetProcAddress(KSProcs.hKSDll, KS_PROCESS_MESSAGE)) &&
		   (KSProcs.DisplayKSError = (PGKS_DISPLAY_KS_ERROR)
				GetProcAddress(KSProcs.hKSDll, KS_DISPLAY_KS_ERROR)) &&
		   (KSProcs.GetKeyserverValues = (PGKS_GET_KEYSERVER_VALUES)
				GetProcAddress(KSProcs.hKSDll, KS_GET_KEYSERVER_VALUES)) &&
		   (KSProcs.StartPutKeyInfoThread = (PGKS_START_PUT_KEY_INFO_THREAD)
				GetProcAddress(KSProcs.hKSDll, KS_START_PUT_KEY_INFO_THREAD)))
		{
			ReturnCode = SUCCESS;
		}
		else
			ReturnCode = ERR_NO_FUNCS_AVAILABLE;
	}
	else
		ReturnCode = ERR_UNABLE_TO_LOAD_LIBRARY;

	return(ReturnCode);
}

KSERR UnLoadKeyserverSupport(void)
{
	KSERR ReturnCode = SUCCESS;

	/*Have we loaded?*/
	if(KSProcs.hKSDll)
	{
		FreeLibrary(KSProcs.hKSDll);
		memset(&KSProcs, '\0', sizeof(KSProcs));
	}
	else
	    /*Philsophical point: we could load the library if they call us
		 *without having loaded it (not here, obviously; it would be dumb
		 *to load it to unload it).  However, we don't want to encourage
		 *people not to call the Load/Unload set (ie, people who call just
		 *PopulateKeyserverMenu() probably won't follow that up with a call
		 *to UnLoadKeyserverSupport).  If calling FreeLibrary() weren't the
		 *Right Thing, I'd remove Load/Unload and just load on the first call.
		 */
		ReturnCode = ERR_LIBRARY_NOT_LOADED;

	return(ReturnCode);
}

KSERR BuildMenu(unsigned long FunctionsDesired,
				HMENU hMenu,
				UINT Position)
{
	KSERR ReturnCode = SUCCESS;

	assert(hMenu);

	if(hMenu)
	{
		/*Ensure we're loaded*/
		if(KSProcs.hKSDll)
		{
			/*Make sure the DLL has this func*/
			if(KSProcs.BuildMenu)
			{
				char buffer[1024];
				sprintf(buffer, "hMenu = %lx; Position = %lx", hMenu, Position);
				ReturnCode = KSProcs.BuildMenu(FunctionsDesired, hMenu,
												Position);
			}
			else
				ReturnCode = ERR_FUNCTION_NOT_IN_LIB;
		}
		else
			ReturnCode = ERR_LIBRARY_NOT_LOADED;
	}
	else
		ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}

KSERR BuildSubMenu(unsigned long FunctionsDesired,
				   HMENU hParentMenu,
				   HMENU hSubMenu,
				   UINT SubMenuPosition)
{
	KSERR ReturnCode = SUCCESS;

	assert(hParentMenu);
	assert(hSubMenu);

	if(hParentMenu && hSubMenu)
	{
		/*Ensure we're loaded*/
		if(KSProcs.hKSDll)
		{
			/*Make sure the DLL has this func*/
			if(KSProcs.BuildSubMenu)
				ReturnCode = KSProcs.BuildSubMenu(FunctionsDesired,
												  hParentMenu,
												  hSubMenu,
												  SubMenuPosition);
			else
				ReturnCode = ERR_FUNCTION_NOT_IN_LIB;
		}
		else
			ReturnCode = ERR_LIBRARY_NOT_LOADED;
	}
	else
		ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}


KSERR ProcessKeyServerMessage(HWND hwnd,
							  unsigned long CommandId,
							  char *SelectedKeys,
							  BOOL *UpdatedKeys)
{
	KSERR ReturnCode = SUCCESS;

	if(UpdatedKeys)
		*UpdatedKeys = FALSE;

	if(KSProcs.hKSDll)
	{
		/*Make sure the DLL has this func*/
		if(KSProcs.ProcessKeyServerMessage)
			ReturnCode = KSProcs.ProcessKeyServerMessage(hwnd,
														 CommandId,
														 SelectedKeys,
														 UpdatedKeys,
														 NULL,
														 NULL);
		else
			ReturnCode = ERR_FUNCTION_NOT_IN_LIB;
	}
	else
		ReturnCode = ERR_LIBRARY_NOT_LOADED;

	return(ReturnCode);
}

KSERR ProcessKeyServerMessageKR(HWND hwnd,
							  unsigned long CommandId,
							  char *SelectedKeys,
							  BOOL *UpdatedKeys,
							  PGPKeySet* pSourceKeySet,
							  PGPKeySet* pDestKeySet)
{
	KSERR ReturnCode = SUCCESS;

	if(UpdatedKeys)
		*UpdatedKeys = FALSE;

	if(KSProcs.hKSDll)
	{
		/*Make sure the DLL has this func*/
		if(KSProcs.ProcessKeyServerMessage)
			ReturnCode = KSProcs.ProcessKeyServerMessage(hwnd,
														 CommandId,
														 SelectedKeys,
														 UpdatedKeys,
														 pSourceKeySet,
														 pDestKeySet);
		else
			ReturnCode = ERR_FUNCTION_NOT_IN_LIB;
	}
	else
		ReturnCode = ERR_LIBRARY_NOT_LOADED;

	return(ReturnCode);
}

KSERR SetupKeyServerSettingsPage(PROPSHEETPAGE *pPSP)
{
	KSERR ReturnCode = SUCCESS;

	assert(pPSP);

	if(pPSP)
	{
		/*Ensure we're loaded*/
		if(KSProcs.hKSDll)
		{
			/*Make sure the DLL has this func*/
			if(KSProcs.SetupKeyServerSettingsPage)
				ReturnCode = KSProcs.SetupKeyServerSettingsPage(pPSP);
			else
				ReturnCode = ERR_FUNCTION_NOT_IN_LIB;
		}
		else
			ReturnCode = ERR_LIBRARY_NOT_LOADED;
	}
	else
		ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}

KSERR GetKeyserverValues(char *Keyserver, unsigned short *Port, BOOL *AutoGet)
{
	KSERR ReturnCode = SUCCESS;
	char foo[256], *pKS;
	unsigned short bar, *pPort;

	assert(AutoGet);

	if(Keyserver)
		pKS = Keyserver;
	else
		pKS = foo;

	if(Port)
		pPort = Port;
	else
		pPort = &bar;

	if(AutoGet)
	{
		/*Ensure we're loaded*/
		if(KSProcs.hKSDll)
		{
			/*Make sure the DLL has this func*/
			if(KSProcs.GetKeyserverValues)
				ReturnCode = KSProcs.GetKeyserverValues(pKS, pPort, AutoGet);
			else
				ReturnCode = ERR_FUNCTION_NOT_IN_LIB;
		}
		else
			ReturnCode = ERR_LIBRARY_NOT_LOADED;
	}
	else
		ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}

void DisplayKSError(HWND hwnd, unsigned long ReturnCode)
{
	/*Ensure we're loaded*/
	if(KSProcs.hKSDll)
	{
		/*Make sure the DLL has this func*/
		if(KSProcs.DisplayKSError)
			KSProcs.DisplayKSError(hwnd, ReturnCode);
	}
}

void StartPutKeyInfoThread(void *pThreadArgs)
{
	/*Ensure we're loaded*/
	if(KSProcs.hKSDll)
	{
		/*Make sure the DLL has this func*/
		if(KSProcs.StartPutKeyInfoThread)
			KSProcs.StartPutKeyInfoThread(pThreadArgs);
	}
}

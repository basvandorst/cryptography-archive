/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: PluginInfo.cpp,v 1.12.8.1 1998/11/12 03:12:45 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PluginInfo.h"
#include "UIutils.h"
#include "resource.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "PGPcl.h"

#define PLUGIN_INFO_PROP		"PGP plugin"

PGPContextRef _pgpContext = NULL;		// PGP context of current instance
PGPtlsContextRef _tlsContext = NULL;	// TLS context of current instance
PGPError _errContext = kPGPError_NoErr;	// Error code if context failed
char *_szReplyText = NULL;

static BOOL DoIExist(void);
static void SetNoLoad(void);
static BOOL ShouldILoad(void);


PluginInfo * CreatePluginInfo(HWND hwnd)
{
	PGPError	err;
	PluginInfo	*plugin;

	plugin = (PluginInfo *) calloc(sizeof(PluginInfo), 1);

	plugin->bLoaded = DoIExist();
	if (!(plugin->bLoaded) || (_pgpContext == NULL))
		_errContext = PGPNewContext(kPGPsdkAPIVersion, &_pgpContext);

	if (!ShouldILoad())
	{
		free(plugin);
		return NULL;
	}

	// Check for SDK expiration or other SDK initialization failure

	if (IsPGPError(_errContext))
	{
		if (_errContext == kPGPError_FeatureNotAvailable)
			UIDisplayStringID(hwnd, IDS_E_EXPIRED);
		else
			PGPclErrorBox(hwnd, _errContext);

		SetNoLoad();
		free(plugin);
		return NULL;
	}

	// Check for beta/demo expiration

	if (PGPclIsExpired(hwnd))
	{
		SetNoLoad();
		free(plugin);
		return NULL;
	}

	if (!(plugin->bLoaded))
		PGPNewTLSContext(_pgpContext, &_tlsContext);

	// Initialize common library

	if (!(plugin->bLoaded))
	{
		err = PGPclInitLibrary(_pgpContext);
		if (IsPGPError(err))
		{
			PGPclErrorBox(hwnd, err);
			SetNoLoad();
			free(plugin);
			return NULL;
		}
	}

	plugin->pgpContext = _pgpContext;
	plugin->tlsContext = _tlsContext;
	plugin->memoryMgr = PGPGetContextMemoryMgr(_pgpContext);
	plugin->nPurgeCacheMsg = RegisterWindowMessage(PURGEPASSPHRASECACEHMSG);
	plugin->nPGPKeysButton = -1;
	plugin->nPGPKeysImage = -1;
	plugin->nPGPKeysString = -1;
	return plugin;
}


PluginInfo * GetPluginInfo(HWND hwnd)
{
	return (PluginInfo *) GetProp(hwnd, PLUGIN_INFO_PROP);
}


void SavePluginInfo(HWND hwnd, PluginInfo *plugin)
{
	if (plugin->hwndCurrent != NULL)
		RemoveProp(plugin->hwndCurrent, PLUGIN_INFO_PROP);

	SetProp(hwnd, PLUGIN_INFO_PROP, (HANDLE) plugin);
	plugin->hwndCurrent = hwnd;
	return;
}


void FreePluginInfo(PluginInfo *plugin)
{
	if (plugin->hwndCurrent != NULL)
		RemoveProp(plugin->hwndCurrent, PLUGIN_INFO_PROP);

	if (plugin->hPGPMenu != NULL)
	{
		DestroyMenu(plugin->hPGPMenu);
		plugin->hPGPMenu = NULL;
	}

	if (plugin->szOutput != NULL)
	{
		free(plugin->szOutput);
		plugin->szOutput = NULL;
	}

	if (!(plugin->bLoaded))
	{
		FreeReplyText();

		if (_tlsContext)
		{
			PGPFreeTLSContext(_tlsContext);
			_tlsContext = NULL;
		}

		if (_pgpContext && IsntPGPError(_errContext))
		{			
			PGPclCloseLibrary();
			PGPFreeContext(_pgpContext);
			_pgpContext = NULL;
		}
	}

	free(plugin);
}


void SetReplyText(char *szText)
{
	FreeReplyText();
	_szReplyText = szText;
	return;
}


char *GetReplyText()
{
	return _szReplyText;
}


void FreeReplyText()
{
	if (_szReplyText != NULL)
	{
		free(_szReplyText);
		_szReplyText = NULL;
	}
	
	return;
}


BOOL DoIExist()
{
    HANDLE hSem;

    // Create or open a named semaphore

	hSem = CreateSemaphore (NULL, 0, 1, "pgpOutlookExpressInstSem");

    // Close handle and return TRUE if existing semaphore was opened.
    if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) 
	{
        CloseHandle(hSem);
        return TRUE;
	}

	return FALSE;
}


void SetNoLoad()
{
    HANDLE hSem;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, "pgpOutlookExpressLoadSem");

    if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) 
        CloseHandle(hSem);

	return;
}


BOOL ShouldILoad()
{
    HANDLE hSem;
	BOOL bShouldLoad = TRUE;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, "pgpOutlookExpressLoadSem");

    if (hSem != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS) 
			bShouldLoad = FALSE;
		else
			bShouldLoad = TRUE;

		CloseHandle(hSem);
	}

	return bShouldLoad;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

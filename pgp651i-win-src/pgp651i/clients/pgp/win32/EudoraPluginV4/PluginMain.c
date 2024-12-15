/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: PluginMain.c,v 1.16 1999/04/09 20:09:44 dgal Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>
#include <windowsx.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpEncode.h"
#include "pgpTLS.h"


// Eudora Headers
#include "emssdk/Mimetype.h"

// Shared Headers
#include "PGPcl.h"
#include "pgpVersionHeader.h"

// Project Headers
#include "PluginMain.h"
#include "Prefs.h"
#include "TranslatorIDs.h"
#include "Translators.h"

// Global Variables
HINSTANCE			g_hinst				= NULL;
PGPContextRef		g_pgpContext		= kPGPInvalidRef;
PGPtlsContextRef	g_tlsContext		= kPGPInvalidRef;
UINT				g_nPurgeCacheMsg	= 0;
HWND				g_hwndHidden		= NULL;

static HWND CreateHiddenWindow(void);

LRESULT CALLBACK HiddenWindowProc(HWND hwnd, 
								  UINT msg, 
								  WPARAM wParam, 
								  LPARAM lParam);

BOOL 
WINAPI 
DllMain (	HINSTANCE hinstDll, 
			DWORD fdwReason,
			LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			g_hinst = hinstDll;
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;
   }
   return(TRUE);
}


/***************************************************************************/
/* TRANSLATER API FUNCTIONS */

/*****\
*
* ems_plugin_version: Get the version of the API used for this plugin
*
\*****/

long
WINAPI
ems_plugin_version(
    short FAR* api_version      /* Place to return api version */
)
{
	*api_version = EMS_VERSION;
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_init: Initialize plugin and get its basic info
*
\*****/

long
WINAPI
ems_plugin_init(
    void FAR*FAR* globals,      /* Out: Return for instance structure */
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
)
{
	long returnValue = EMSR_OK;
	PGPError error;

	// Initialize the PGPsdk for the plugin
	error = PGPNewContext(	kPGPsdkAPIVersion, 
							&g_pgpContext );

	if( IsPGPError(error) )
	{
		if(error == kPGPError_FeatureNotAvailable)
		{
			char szMessage[256];
			char szTitle[256];

			LoadString(g_hinst, IDS_E_EXPIRED, szMessage, sizeof(szMessage));
			LoadString(g_hinst, IDS_E_EXPIREDTITLE, szTitle, sizeof(szTitle));

			MessageBox(NULL, szMessage, szTitle, MB_OK);
		}
		else
		{
			PGPclErrorBox (	NULL, 
							error);
		}

		returnValue = EMSR_UNKNOWN_FAIL;
	}

	// Initialize the Common Libraries
	error = PGPclInitLibrary (g_pgpContext);

    if (IsPGPError (error)) 
	{
        PGPclErrorBox (NULL, error);
 
        returnValue = EMSR_UNKNOWN_FAIL;
    }

	// has this beta version expired?
	if(PGPclIsExpired(NULL))
	{
		PGPclCloseLibrary();
		PGPFreeContext(g_pgpContext);
		g_pgpContext = NULL;
	
		return EMSR_UNKNOWN_FAIL;
	}

	PGPNewTLSContext( g_pgpContext, &g_tlsContext );

	// Register the passphrase cache purge message

	g_nPurgeCacheMsg = RegisterWindowMessage(PURGEPASSPHRASECACEHMSG);

	// Create a hidden window to catch messages

	g_hwndHidden = CreateHiddenWindow();

	if (pluginInfo &&
		returnValue == EMSR_OK)
	{
		pluginInfo->numTrans = kNumTrans;
		pluginInfo->numAttachers = 0;
		pluginInfo->numSpecials = kNumSpecial;
		
		pluginInfo->desc = strdup(pgpVersionHeaderString);
		
		pluginInfo->icon = (HICON*)malloc(sizeof(HICON));

		if(pluginInfo->icon)
		{
			*pluginInfo->icon = LoadIcon(	g_hinst, 
											MAKEINTRESOURCE( IDI_PGP32 ) );
		}
		else
		{
			returnValue = EMSR_UNKNOWN_FAIL;
		}
		
		pluginInfo->id = kPluginID;
	}
	
	return (returnValue);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translator_info: Get details about a translator in a plugin
*
\*****/

long
WINAPI
ems_translator_info(
    void FAR* globals,          /* Out: Return for instance structure */
    emsTranslatorP transInfo    /* In/Out: Return Translator Information */
)
{
	TransInfoStruct *InfoPtr = NULL;
	long returnValue = EMSR_OK;

	if ((transInfo) && (gTransInfo))
	{
		const long id = transInfo->id;

		if ((id > 0) && 
			(id <= kNumTrans))
		{
			InfoPtr = gTransInfo + (id - 1);

			transInfo->type = InfoPtr->type;
			transInfo->flags = InfoPtr->flags;

			if(	kEncryptTranslatorID == id && 
				ByDefaultEncrypt(PGPGetContextMemoryMgr(g_pgpContext)))
			{
				transInfo->flags |= EMSF_DEFAULT_Q_ON;
			}
			else if(kSignTranslatorID == id &&
					ByDefaultSign(PGPGetContextMemoryMgr(g_pgpContext)))
			{
				transInfo->flags |= EMSF_DEFAULT_Q_ON;
			}

			transInfo->desc = strdup(InfoPtr->description);
			transInfo->icon = (HICON *)malloc(sizeof(HICON));

			if(transInfo->icon)
			{
				*transInfo->icon = LoadIcon(g_hinst, 
					MAKEINTRESOURCE(gTransInfo[id - 1].nIconID));
			}
			else
			{
				returnValue = EMSR_UNKNOWN_FAIL;
			}
		}
		else
		{
			returnValue = EMSR_INVALID_TRANS;
		}
	}
	else
	{
		returnValue = EMSR_UNKNOWN_FAIL;
	}

	return (returnValue);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_can_translate_file:
*     Check and see if a translation can be performed (file version)
*
\*****/

long
WINAPI
ems_can_translate(
    void FAR* globals,            /* Out: Return for instance structure */
    emsTranslatorP transInfo,     /* In: Translator Info */
    emsDataFileP inTransData,     /* In: What to translate */
    emsResultStatusP transStatus  /* Out: Translations Status information */
)
{
	long returnValue = EMSR_OK;

	if ((transInfo) && (inTransData))
	{
		const long id = transInfo->id;
		const long context = inTransData->context;
		const emsMIMEtypeP in_mime = inTransData->info;

		if ((id > 0) && 
			(id <= kNumTrans))
		{
			returnValue = CanPerformTranslation(	
										id, 
										in_mime, 
										context);
		}
		else
		{
			returnValue = EMSR_INVALID_TRANS;
		}
	}
	else
	{
		returnValue = EMSR_UNKNOWN_FAIL;
	}

	return returnValue;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translate_file: Actually perform a translation on a file
*
\*****/

long
WINAPI
ems_translate_file(
    void FAR* globals,           /* Out: Return for instance structure */
    emsTranslatorP transInfo,    /* In: Translator Info */
    emsDataFileP inFileData,     /* In: What to translate */
    emsProgress progress,        /* Func to report progress/check for abort */
    emsDataFileP outFileData,    /* Out: Result of the translation */
    emsResultStatusP transStatus /* Out: Translations Status information */
)
{
	long returnValue = EMSR_OK;
 
	if(	inFileData &&
		outFileData && 
		transInfo &&
		transStatus)
	{
		const char *in_file = inFileData->fileName;
		const char *out_file = outFileData->fileName;
		const long id = transInfo->id;
		emsMIMEtypeP in_mime = inFileData->info;
		emsMIMEtypeP FAR *out_mime = &(outFileData->info);
		char FAR* FAR*out_desc = &(transStatus->desc);

		// see if this is one of our id's
		if ((id > 0) && 
			(id <= kNumTrans))
		{
			HANDLE hOutput = NULL;

			returnValue = PerformTranslation(	
										id, 
										in_file, 
										out_file, 
										inFileData->header, 
										in_mime,
										out_mime);
		}
		else
		{
			returnValue = EMSR_INVALID_TRANS;
		}
	}

	return returnValue;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_config: Call the plug-ins configuration Interface
*
\*****/

long
WINAPI
ems_plugin_config(
    void FAR* globals,			/* Out: Return for instance structure */
    emsMailConfigP mailConfig	/* In: Eudora mail info */
)
{
	PGPclPreferences (g_pgpContext, NULL, PGPCL_EMAILPREFS, NULL);

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_special_info: Get details about a attacher in a plugin
*
\*****/

long WINAPI ems_special_info(
    void FAR* globals,          /* Out: Return for instance structure */
    emsMenuP specialMenu         /* Out: The menu */
)
{
	TransInfoStruct *InfoPtr = NULL;
	long returnValue = EMSR_OK;

	if ( specialMenu->id != kSpecialLaunchKeysID )
	{
		return (EMSR_INVALID_TRANS);
	}

	specialMenu->flags = gSpecialInfo->flags;
	specialMenu->desc = strdup(gSpecialInfo->description);

	specialMenu->icon = (HICON *)malloc(sizeof(HICON));

	if(specialMenu->icon)
	{
		*specialMenu->icon = LoadIcon(g_hinst, 
			MAKEINTRESOURCE(gSpecialInfo->nIconID));
	}
	else
	{
		returnValue = EMSR_UNKNOWN_FAIL;
	}

	return returnValue;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_special_hook: Get details about a attacher in a plugin
*
\*****/

long WINAPI ems_special_hook(
    void FAR* globals,          /* Out: Return for instance structure */
    emsMenuP specialMenu        /* In: The menu */
)
{
	if (!specialMenu) 
	{
		return (EMSR_UNKNOWN_FAIL);
	}
	
	if (specialMenu->id == kSpecialLaunchKeysID)
	{
		char szPath[MAX_PATH];
		char szPGPkeys[256];
		PGPError error = kPGPError_NoErr;

		LoadString(g_hinst, IDS_PGPKEYSEXE, szPGPkeys,
			sizeof(szPGPkeys));

		error = PGPclGetPGPPath (szPath, sizeof(szPath));

		if( IsntPGPError(error) )
		{
			// '/s' keeps it from showing that 
			// damn splash screen
			strcat(szPath, szPGPkeys);
			// run it...
			WinExec(szPath, SW_SHOW);
		}
		else
		{
			char szError[256];

			LoadString(g_hinst, IDS_E_LAUNCHPGPKEYS, szError,
				sizeof(szError));

			MessageBox(NULL, 
				szError, 
				0, 
				MB_OK);
		}
	}

	return (EMSR_OK);
}



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_finish: End use of a plugin and clean up
*
\*****/

long
WINAPI
ems_plugin_finish(
    void FAR* globals    /* Pointer to translator instance structure */
)
{
	if (g_hwndHidden != NULL)
		DestroyWindow(g_hwndHidden);

	if( PGPRefIsValid(g_tlsContext) )
	{
		PGPFreeTLSContext(g_tlsContext);
	}

	if( PGPRefIsValid(g_pgpContext) )
	{
		PGPclCloseLibrary();
		PGPFreeContext(g_pgpContext);
	}
	
	return (EMSR_OK); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_free: Free memory allocated by EMS plug-in
*
\*****/

long
WINAPI
ems_free(
    void FAR* mem               /* Memory to free */
)
{
	if (mem)
	{
		free(mem); 
		mem = NULL; 
	} 

	return (EMSR_OK);
}


HWND CreateHiddenWindow(void)
{
	HWND hwnd;
	WNDCLASS wc;

	// Register the Window Class

	wc.style			= 0;
	wc.lpfnWndProc		= HiddenWindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= g_hinst;
	wc.hIcon			= 0;
	wc.hCursor			= 0;
	wc.hbrBackground	= 0;
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= "EudoraPluginV4 Hidden Window";

	RegisterClass(&wc);

	hwnd = CreateWindow("EudoraPluginV4 Hidden Window", 
		"EudoraPluginV4 Hidden Window",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, NULL, NULL, g_hinst, NULL);

	return hwnd;
}


LRESULT CALLBACK HiddenWindowProc(HWND hwnd, 
								  UINT msg, 
								  WPARAM wParam, 
								  LPARAM lParam)
{
	if (msg == WM_CREATE)
	{
		ShowWindow(hwnd, SW_HIDE);
		return 0;
	}

	if (msg == g_nPurgeCacheMsg)
	{
		PGPclPurgeCachedPassphrase(wParam);
		return TRUE;
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

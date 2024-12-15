/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PluginMain.c,v 1.1.2.1 1997/11/14 06:19:45 elrod Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>
#include <windowsx.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpEncode.h"

// Eudora Headers
#include "emssdk/Mimetype.h"

// Shared Headers
#include "PGPcmdlg.h"

// Project Headers
#include "PluginMain.h"
#include "MyPrefs.h"
#include "TranslatorIDs.h"
#include "Translators.h"

// Global Variables
HINSTANCE		g_hinst			= NULL;
PGPContextRef	g_pgpContext	= kPGPInvalidRef;


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
	error = PGPNewContext( kPGPsdkAPIVersion, &g_pgpContext );

	if( IsPGPError(error) )
	{
		if(error == kPGPError_FeatureNotAvailable)
		{
			MessageBox(NULL,"The evaluation period for PGP has passed.\n"
							"The Eudora Plugin will no longer function.",
							"PGP Plugin Expired", MB_OK);
		}
		else
		{
			PGPcomdlgErrorMessage (error);
		}

		returnValue = EMSR_UNKNOWN_FAIL;
	}

	// has this beta version expired?
	if(PGPcomdlgIsExpired(NULL))
	{
		returnValue = EMSR_UNKNOWN_FAIL;
	}
 
	if (pluginInfo &&
		returnValue == EMSR_OK)
	{
		pluginInfo->numTrans = kNumTrans;
		pluginInfo->numAttachers = 0;
		pluginInfo->numSpecials = kNumSpecial;
		
		pluginInfo->desc = strdup(kPluginDescription);
		
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
				ByDefaultEncrypt() )
			{
				transInfo->flags |= EMSF_DEFAULT_Q_ON;
			}
			else if(kSignTranslatorID == id &&
					ByDefaultSign()	)
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
	PGPcomdlgPreferences (g_pgpContext, NULL, 2);

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
		PGPError error = kPGPError_NoErr;

		error = PGPcomdlgGetPGPPath (szPath, sizeof(szPath));

		if( IsntPGPError(error) )
		{
			// '/s' keeps it from showing that 
			// damn splash screen
			strcat(szPath, "PGPkeys.exe /s");
			// run it...
			WinExec(szPath, SW_SHOW);
		}
		else
		{
			MessageBox(NULL, 
				"Unable to locate the PGPkeys Application", 
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
	if( PGPRefIsValid(g_pgpContext) )
	{
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

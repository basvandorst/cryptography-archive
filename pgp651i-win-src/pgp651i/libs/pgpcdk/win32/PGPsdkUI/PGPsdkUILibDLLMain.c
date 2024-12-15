/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPsdkUILibDLLMain.c,v 1.6 1999/03/10 02:55:49 heller Exp $
____________________________________________________________________________*/

#include <windows.h>

#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "PGPsdkUILibPriv.h"

static PGPBoolean	sInitializedLibrary = FALSE;

	BOOL APIENTRY
DllMain(
	HANDLE	hModule, 
	DWORD	ul_reason_for_call, 
	LPVOID	lpReserved)
{
	BOOL	success = TRUE;
	
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			gPGPsdkUILibInst = hModule;
			
			if( IsntPGPError( PGPsdkUILibInit() ) )
			{
				sInitializedLibrary = TRUE;
			}
			else
			{
				success = FALSE;
			}

			break;
		}
		
		case DLL_PROCESS_DETACH:
		{
			if( sInitializedLibrary )
			{
				(void) PGPsdkUILibCleanup();
				sInitializedLibrary = FALSE;
			}

			break;
		}
    }

    return success;
}

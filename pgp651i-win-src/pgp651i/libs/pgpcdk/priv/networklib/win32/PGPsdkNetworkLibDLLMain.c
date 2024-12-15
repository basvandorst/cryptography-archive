/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: PGPsdkNetworkLibDLLMain.c,v 1.8 1999/03/10 02:55:49 heller Exp $
____________________________________________________________________________*/

#include <windows.h>

#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "PGPsdkNetworkLibPriv.h"

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
			if( IsntPGPError( PGPsdkNetworkLibInit() ) )
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
				(void) PGPsdkNetworkLibCleanup();
				sInitializedLibrary = FALSE;
			}

			break;
		}
    }

    return success;
}

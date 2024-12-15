/*
 * Copyright (C) 1996,1997 Network Associates Inc. and affiliated companies.
 * All rights reserved
 *
 * $Id: PGPsdkLibDLLMain.c,v 1.5 1999/03/10 02:55:37 heller Exp $
 */

#include <windows.h>

#include "pgpConfig.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpUtilitiesPriv.h"

/*  In Windows, the DllMain function is called when a process or 
    thread attaches to, or detaches from, the DLL. */

static PGPBoolean	sInitializedLibaray = FALSE;

BOOL WINAPI DllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	BOOL		success	= TRUE;
	
	(void)hInst;		/* Avoid warnings */
	(void)lpReserved;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			if ( IsntPGPError( PGPsdkInit() ) )
			{
				sInitializedLibaray = TRUE;
			}
			else
			{
				success	= FALSE;
			}

			break;
		}

		case DLL_PROCESS_DETACH:
		{
			if( sInitializedLibaray )
			{
				(void) PGPsdkCleanup();
				sInitializedLibaray = FALSE;
			}
		
			break;
		}
	}
	
	return success;
}

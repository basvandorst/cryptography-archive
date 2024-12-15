/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: PGPsdkNetworkLibDLLMain.c,v 1.2.10.1 1998/11/12 03:22:55 heller Exp $
____________________________________________________________________________*/

#include <windows.h>

#include "PGPsdkNetworkLibDLLMain.h"



CRITICAL_SECTION		gKeyServerInitMutex;
CRITICAL_SECTION		gSocketsInitMutex;


	BOOL APIENTRY
DllMain(
	HANDLE	hModule, 
	DWORD	ul_reason_for_call, 
	LPVOID	lpReserved)
{
	switch (ul_reason_for_call)	{
		case DLL_PROCESS_ATTACH:
		{
			InitializeCriticalSection(&gKeyServerInitMutex);
			InitializeCriticalSection(&gSocketsInitMutex);
		}
		break;


		case DLL_PROCESS_DETACH:
		{
			DeleteCriticalSection(&gKeyServerInitMutex);
			DeleteCriticalSection(&gSocketsInitMutex);
		}
		break;
    }

    return TRUE;
}

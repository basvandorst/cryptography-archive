/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpSDKUILib.c,v 1.6.8.1 1999/06/04 01:12:11 heller Exp $
____________________________________________________________________________*/

#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpSDKUILibPriv.h"
#include "pgpUtilities.h"

#if PGP_WIN32
#include <windows.h>

HINSTANCE	gPGPsdkUILibInst;
#endif

static PGPUInt32	sInitedCount 			= 0;
static PGPBoolean	sInitializedSDK 		= FALSE;
static PGPBoolean	sInitializedNetworkLib 	= FALSE;

#if PGP_WIN32
static PGPBoolean	sResourceDLL			= FALSE;
#endif

	PGPError
PGPsdkUILibInit(void)
{
	PGPError	err	= kPGPError_NoErr;
	
	if( sInitedCount == 0 )
	{
		/* Initialize the SDK in case the caller has not */
		
		err = PGPsdkInit();
		if( IsntPGPError( err ) )
		{
			sInitializedSDK = TRUE;

			err = PGPsdkNetworkLibInit();
			if( IsntPGPError( err ) )
			{
				sInitializedNetworkLib = TRUE;
				
				pgpLeaksBeginSession( "PGPsdkUILib" );

			#if PGP_WIN32
				if( IsNull( gPGPsdkUILibInst ) )
				{
					/*
					** If this is a DLL, DLLMain() will set gPGPsdkUILibInst before
					** calling this function so loading the resource-only DLL
					** will not be necessary. If gPGPsdkUILibInst is NULL, this is
					** a static library and we must get our resources from an external
					** DLL.
					*/
					
					sResourceDLL=TRUE;

					gPGPsdkUILibInst = LoadLibrary("PGPuiRes.DLL");
					if (gPGPsdkUILibInst == 0)
					{
						err = -1;
					}
				}
			#endif
			}
		}
	}
	
	if( IsntPGPError( err ) )
	{
		++sInitedCount;
	}
	
	return( err );
}

	PGPError
PGPsdkUILibCleanup(void)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpAssert( sInitedCount != 0 );
	
	if( sInitedCount != 0 )
	{
		--sInitedCount;
		
		if( sInitedCount == 0 )
		{
			if( sInitializedNetworkLib )
			{
				pgpLeaksEndSession();

				(void) PGPsdkNetworkLibCleanup();
				sInitializedNetworkLib = FALSE;
			}
			
			if( sInitializedSDK )
			{
				(void) PGPsdkCleanup();
				sInitializedSDK = FALSE;
			}

			#if PGP_WIN32
			// If we are using resource DLL, unload it
			if((gPGPsdkUILibInst)&&(sResourceDLL))
			{
				FreeLibrary(gPGPsdkUILibInst);
				gPGPsdkUILibInst=NULL;
				sResourceDLL=FALSE;
			}
			#endif

		}
	}
	else
	{
		err	= kPGPError_BadParams;
	}
	
	return( err );
}

	PGPError
pgpForceSDKUILibCleanup(void)
{
	PGPError	err	= kPGPError_NoErr;
	
	if( sInitedCount != 0 )
	{
		sInitedCount = 1;
	}
	
	err	= PGPsdkUILibCleanup();
	
	return( err );
}

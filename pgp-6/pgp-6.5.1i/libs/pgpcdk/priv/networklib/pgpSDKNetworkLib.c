/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpSDKNetworkLib.c,v 1.2 1998/11/20 01:01:25 heller Exp $
____________________________________________________________________________*/

#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpSDKNetworkLibPriv.h"
#include "pgpUtilities.h"

#if PGP_MACINTOSH	/* [ */

#include "MacErrors.h"

ThreadID		gMainThreadID;

#elif PGP_WIN32		/* ][ */

CRITICAL_SECTION		gKeyServerInitMutex;
CRITICAL_SECTION		gSocketsInitMutex;

#endif	/* ] */

static PGPUInt32	sInitedCount 		= 0;
static PGPBoolean	sInitializedSDK 	= FALSE;


	PGPError
PGPsdkNetworkLibInit(void)
{
	PGPError	err	= kPGPError_NoErr;
	
	if( sInitedCount == 0 )
	{
		/* Initialize the SDK in case the caller has not */
		
		err = PGPsdkInit();
		if( IsntPGPError( err ) )
		{
			sInitializedSDK = TRUE;

			pgpLeaksBeginSession( "PGPsdkNetworkLib" );
			
		#if PGP_MACINTOSH
			{
				OSErr	macErr;
				
				macErr = GetCurrentThread( &gMainThreadID );
				if( macErr != noErr )
				{
					err = MacErrorToPGPError( macErr );
				}
			}
		#elif PGP_WIN32
			{
				InitializeCriticalSection( &gKeyServerInitMutex );
				InitializeCriticalSection( &gSocketsInitMutex );
			}
		#endif
		}
	}
	
	if( IsntPGPError( err ) )
	{
		++sInitedCount;
	}
	
	return( err );
}

	PGPError
PGPsdkNetworkLibCleanup(void)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpAssert( sInitedCount != 0 );
	
	if( sInitedCount != 0 )
	{
		--sInitedCount;
		
		if( sInitedCount == 0 && sInitializedSDK )
		{
			pgpLeaksEndSession();

		#if PGP_WIN32
			DeleteCriticalSection( &gKeyServerInitMutex );
			DeleteCriticalSection( &gSocketsInitMutex );
		#endif
		
			(void) PGPsdkCleanup();
			sInitializedSDK = FALSE;
		}
	}
	else
	{
		err	= kPGPError_BadParams;
	}
	
	return( err );
}

	PGPError
pgpForceSDKNetworkLibCleanup(void)
{
	PGPError	err	= kPGPError_NoErr;
	
	if( sInitedCount != 0 )
	{
		sInitedCount = 1;
	}
	
	err	= PGPsdkNetworkLibCleanup();
	
	return( err );
}

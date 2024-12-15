/*____________________________________________________________________________
	DiskWiperExample.c
	
	Copyright (C) 1996,1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Need to include:

	pgpDebug.c
	pgpLeaks.c
	pgpMem.c
	pgpMemoryMgr.c
	pgpMemoryMgrWin32.c or pgpMemoryMgrMac.c

	and link against the SDK for RNG functionality.

	$Id: DiskWiperExample.c,v 1.3 1999/03/10 02:41:58 heller Exp $
____________________________________________________________________________*/
#include <stdio.h>
#include <limits.h>

/* PGPsdk Headers */
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h" 
#include "pgpSDKPrefs.h"
#include "pflPrefTypes.h"
#include "pgpPubTypes.h"
#include "pgpUtilities.h"

#include "pgpDiskWiper.h"

void main( int argc, char* argv[] )
{
	PGPError		error			= kPGPError_NoErr;
	PGPDiskWipeRef	wipeRef			= kPGPInvalidRef;
	PGPInt32		passes			= kPGPNumPatterns;
	PGPInt32		buffer[256];
	
	
	error = PGPCreateDiskWiper(	&wipeRef, passes);
	
	if( IsntPGPError(error) )
	{
		while( IsntPGPError( PGPGetDiskWipeBuffer(wipeRef, buffer) ) )
		{
			static int i = 1;

			printf("Pass #%d\r\n", i++);

			/* 

			Write pattern to disk here, repeating until
			end of the file. We should round the file 
			size up to next page boundary to be
			safe. Also make sure you sync the file
			after writing each pattern so that they
			actually get written to disk and not just
			an IO buffer.

			*/
		}

		error = PGPDestroyDiskWiper(wipeRef);
	}

	if(IsPGPError(error))
	{
		char buf[256];

		PGPGetClientErrorString( error,256, buf );

		printf("Error!!!\r\n%s\r\n", buf);
	}
}

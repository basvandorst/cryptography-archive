/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: pgpMilliseconds.c,v 1.2 1998/11/03 13:14:48 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#if PGP_MACINTOSH
#include <OpenTransport.h>
#elif PGP_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "pgpMilliseconds.h"

	PGPUInt32
PGPGetMilliseconds()
{
	/* Returns milliseconds since system startup */
#if PGP_MACINTOSH
	OTTimeStamp timeStamp;
	
	OTGetTimeStamp( &timeStamp );
	return OTTimeStampInMilliseconds( &timeStamp );
#elif PGP_WIN32
	return GetTickCount();
#else
	pgpAssert( 0 );	/* not implemented */
#endif
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpTimeBomb.c,v 1.10 1999/03/10 02:59:05 heller Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"
#include "pgpTimeBomb.h"

#if PGP_TIME_BOMB	/* [ */

#include <time.h>
#include "pgpMem.h"

	PGPBoolean
pgpTimeBombHasExpired( void )
{
	time_t		currentTime	= time( NULL );
	time_t		expirationTime;
	struct tm	tm;
	
	pgpAssert( kPGPTimeBombMonth >= 1 && kPGPTimeBombMonth <= 12 );
	pgpAssert( kPGPTimeBombYear >= 1997 );
	pgpAssert( kPGPTimeBombDay >= 0 && kPGPTimeBombDay <= 31 );
	
	pgpClearMemory( &tm, sizeof( tm ) );
	tm.tm_year	= kPGPTimeBombYear - 1900;	/* API requires just "97" */
	tm.tm_mon	= kPGPTimeBombMonth - 1;	/* months are [0,11] */
	tm.tm_mday	= kPGPTimeBombDay;
	
	expirationTime	= mktime( &tm );
	
	return ( currentTime >= expirationTime );
}


#endif	/* ] */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
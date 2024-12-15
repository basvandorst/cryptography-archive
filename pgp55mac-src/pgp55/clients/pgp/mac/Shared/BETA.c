/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: BETA.c,v 1.10 1997/10/22 01:08:23 build Exp $
____________________________________________________________________________*/
#include "Beta.h"

#if BETA	// [

	Boolean
BetaExpired( void )
	{
	DateTimeRec	dt;
	ulong		now;
	ulong		expirationDate;
	
	dt.month		= 11;
	dt.day			= 21;
	dt.year			= 1997;
	dt.hour			= 0;
	dt.minute		= 0;
	dt.second		= 0;
	dt.dayOfWeek	= 0;
	DateToSeconds( &dt, &expirationDate );
	
	GetDateTime( &now );
	
	return( expirationDate < now );
	}


#endif	// ]
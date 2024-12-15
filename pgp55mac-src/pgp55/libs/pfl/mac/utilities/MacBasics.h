/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacBasics.h,v 1.7 1997/09/18 01:34:40 lloyd Exp $
____________________________________________________________________________*/
#pragma once

#define PGP_MACINTOSH	1

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS



typedef short	ResID;


#ifdef __MWERKS__	/* [ */

	/* used around #pragma mark xyz */
	#define PRAGMA_MARK_SUPPORTED	1
	
#endif	/* ] */




/*____________________________________________________________________________
	utility macros 
____________________________________________________________________________*/
#define IsntErr( e )	( (e) == noErr )
#define IsErr( e )		( (e) != noErr )




#if ! powerc	/* [ */

#pragma parameter __D0 GetD0
long	GetD0( void )		ONEWORDINLINE( 0x4E71 );

#pragma parameter SetD0( __D0 )
void	SetD0( long d0 )		ONEWORDINLINE( 0x4E71 );


#pragma parameter __D0 GetD1
long	GetD1( void )		ONEWORDINLINE( 0x2001 );

#pragma parameter SetD1( __D0 )
void	SetD1( long d1 )		ONEWORDINLINE( 0x2200 );


#pragma parameter __D0 GetA0
void *	GetA0( void )		ONEWORDINLINE( 0x02008 );

#pragma parameter __D0 GetA1
void *	GetA1( void )		ONEWORDINLINE( 0x02009 );

#pragma parameter __D0 GetA4
void *	GetA4( void )		ONEWORDINLINE( 0x0200c );

#pragma parameter __D0 GetA5
void *	GetA5( void )		ONEWORDINLINE( 0x0200d );


#pragma parameter __D0 SetA0( __D0 )
void *	SetA0( const void * a0 )		ONEWORDINLINE( 0xC188 );
/* returns old A0 */

#pragma parameter __D0 SetA1( __D0 )
void *	SetA1( const void *a0 )		ONEWORDINLINE( 0xC189 );
/* returns old A1 */

#endif	/* ] */



PGP_END_C_DECLARATIONS





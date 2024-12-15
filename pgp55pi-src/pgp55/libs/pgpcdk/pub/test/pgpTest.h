/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpTest.h,v 1.9 1997/09/09 00:19:02 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpTest_h	/* [ */
#define Included_pgpTest_h


#include "pgpUtilities.h"


#ifndef IsNull
#define IsNull( p )		( (p) == NULL )
#define IsntNull( p )	( (p) != NULL )
#endif

#if PGP_INTERNAL_TESTING

#include "pgpDebug.h"
#include "pgpMem.h"

#define pgpTestAssert( x )			pgpAssert( x )
#define pgpTestDebugMsg( x )		pgpDebugMsg( x )
#define pgpTestAssertNoErr( err )	pgpAssertNoErr( err )


#else

#define	pgpTestAssert( x )	_pgpTestAssert( (PGPBoolean)((x) != 0), #x )
void		_pgpTestAssert( PGPBoolean	condition, const char *expr );
void		pgpTestDebugMsg( const char *msg );
void		pgpTestAssertNoErr( PGPError err );
#define 	pgpFixBeforeShip( x )	/* nothing */

#endif


PGP_BEGIN_C_DECLARATIONS

void	TestSymmetricCipher( PGPContextRef context );
void	TestCipherFeedback( PGPContextRef context );
void	TestHash( PGPContextRef context );

void	TestKeys( PGPContextRef context );

void	TestEncodeInteractive( PGPContextRef context );
void	TestEncodeCmdLine( PGPContextRef context, const char *commandLine );
void	TestEncodeArgs( PGPContextRef context, int argc, const char **argv );

void	TestMain( void );
PGP_END_C_DECLARATIONS



#endif /* ] Included_pgpTest_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

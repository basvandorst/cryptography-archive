/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/

#include "PGPSockets.h"

pascal OSErr	INIT_PGPldapLib(const CFragInitBlock *theInitBlock);
pascal void		TERM_PGPldapLib(void);
pascal OSErr 	__initialize(const CFragInitBlock *theInitBlock);
pascal void 	__terminate(void);

	pascal OSErr
INIT_PGPldapLib(const CFragInitBlock *theInitBlock)
{
	OSErr	err = noErr;
	
	#pragma unused( theInitBlock )
	
	err = __initialize( theInitBlock );
	if( IsntErr( err ) )
	{
		if( ! PGPSocketsInit() )
		{
			pgpDebugMsg( "Error from PGPSocketsInit()" );
			err = -1;
		}
	}
	
	pgpLeaksBeginSession( "PGPldapLib" );
	
	return( err );
}

	pascal void
TERM_PGPldapLib(void)
{
	PGPSocketsCleanup();

	pgpLeaksEndSession();

	__terminate();
}

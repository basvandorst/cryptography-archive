/*____________________________________________________________________________
	main.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: main.c,v 1.6 1997/10/09 23:54:14 lloyd Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"	/* or pgpConfig.h in the CDK */

#include <CodeFragments.h>
#include <Gestalt.h>

#include "MacBasics.h"
#include "pgpDebug.h"
#include "pgpErrorsPriv.h"
#include "pgpMem.h"
#include "pgpUtilitiesPriv.h"

PGP_BEGIN_C_DECLARATIONS

pascal OSErr 	INIT_PGPsdk(CFragInitBlock *initBlock);
pascal void		TERM_PGPsdk(void);

pascal OSErr __initialize(const CFragInitBlock *theInitBlock);
pascal OSErr __terminate(void);

PGP_END_C_DECLARATIONS

/* Disable 68020 code generation for the 68020 check itself */
#if GENERATING68K
	#pragma code68020 off
#endif

	pascal OSErr
INIT_PGPsdk(CFragInitBlock *initBlock)
{
	long	gestaltResult;
	OSErr	err = noErr;
	
	/*
	** Check for a 68020 or later processor and bail immediately if
	** none is available.
	** It is assumed that all clients of this library are doing a
	** similar chack and refusing to run.
	*/
	
	if( Gestalt( gestaltProcessorType, &gestaltResult ) == noErr &&
		gestaltResult >= gestalt68020 )
	{
		err = __initialize( initBlock );

		if ( IsntErr( err ) )
			err	= PGPsdkInit();
		pgpAssert( IsntPGPError( err ) );
	}

	return( err );
}

	pascal void
TERM_PGPsdk(void)
{
	(void)pgpsdkCleanupForce();
	
	__terminate();
}

#if GENERATING68K
	#pragma code68020 reset
#endif










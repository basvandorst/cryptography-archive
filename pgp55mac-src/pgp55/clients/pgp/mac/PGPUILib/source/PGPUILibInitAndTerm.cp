/*____________________________________________________________________________
	PGPUILibInitAndTerm.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPUILibInitAndTerm.cp,v 1.4 1997/09/17 18:16:27 heller Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"

#include <CodeFragments.h>
#include <Gestalt.h>

#include "MacBasics.h"
#include "PGPUILibUtils.h"
#include "pgpDebug.h"
#include "pgpMem.h"

PGP_BEGIN_C_DECLARATIONS

pascal OSErr 	INIT_PGPUILib(CFragInitBlock *initBlock);
pascal void		TERM_PGPUILib(void);

pascal OSErr __initialize(const CFragInitBlock *theInitBlock);
pascal OSErr __terminate(void);

PGP_END_C_DECLARATIONS

/* Disable 68020 code generation for the 68020 check itself */
#if GENERATING68K
	#pragma code68020 off
#endif

	pascal OSErr
INIT_PGPUILib(CFragInitBlock *initBlock)
{
	long	gestaltResult;
	OSErr	err = noErr;
	
	/*
	** Check for a 68020 or later processor and bail immediately if
	** none is available. It is assumed that all clients of this
	** library are doing a similar chack and refusing to run.
	*/
	
	if( Gestalt( gestaltProcessorType, &gestaltResult ) == noErr &&
		gestaltResult >= gestalt68020 )
	{
		err = __initialize( initBlock );
		if( IsntErr( err ) )
		{
			/*
			** Remember the FSSpec of the library file se we can open it's
			** resource fork later.
			*/
			
			if( IsntNull( initBlock ) )
			{
				pgpAssert( IsFileLocation( initBlock->fragLocator.where ) );
			
				err = SetLibraryFSSpec(
					initBlock->fragLocator.u.onDisk.fileSpec );
			}
			else
			{
				/* Actual error is not important here */
				
				pgpDebugMsg( "INIT_PGPUILib: No initBlock" );
				err = -1;
			}
		}

		pgpLeaksBeginSession( "PGPUILib" );
	}
	
	pgpAssert( err == noErr );

	return( err );
}

	pascal void
TERM_PGPUILib(void)
{
	long	gestaltResult;

	if( Gestalt( gestaltProcessorType, &gestaltResult ) == noErr &&
		gestaltResult >= gestalt68020 )
	{
		pgpLeaksEndSession();
	}
	
	__terminate();
}

#if GENERATING68K
	#pragma code68020 reset
#endif





/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

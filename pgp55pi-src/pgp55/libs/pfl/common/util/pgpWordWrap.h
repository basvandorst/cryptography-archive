/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpWordWrap.h,v 1.2 1997/09/12 19:19:11 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpWordWrap_h	/* [ */
#define Included_pgpWordWrap_h


#include "pgpIO.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



/* lineEnd should be one of "\r", "\n", "\r\n"  */
/* and is the type of line end that will be inserted */
/* any one of { \r, \n, \r\n } are consider line breaks for input purposes */
PGPError	pgpWordWrapIO( PGPIORef input, PGPIORef output,
				PGPUInt16 wrapLength, const char * lineEnd);
				
/* same as above, but takes file specs */
PGPError	pgpWordWrapFileSpecs( PFLFileSpecRef input,
				PFLFileSpecRef output, 
				PGPUInt16 wrapLength, const char * lineEnd);


#if PGP_MACINTOSH

/* same as above, but takes FSSpecs */
/* you may have to create a temporary PFLContext first */
PGPError	pgpWordWrapFileFSSpec( PFLContextRef context,
				const FSSpec * inFSSpec, const FSSpec * outFSSpec,
				PGPUInt16 wrapLength, const char * lineEnd);
#endif




#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS



#endif /* ] Included_pgpWordWrap_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

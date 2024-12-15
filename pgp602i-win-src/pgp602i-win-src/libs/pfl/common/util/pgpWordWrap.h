/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpWordWrap.h,v 1.3.16.1 1998/11/12 03:18:45 heller Exp $
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
/* you may have to create a temporary PGPMemoryMgrRef first */
PGPError	pgpWordWrapFileFSSpec( PGPMemoryMgrRef memoryMgr,
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

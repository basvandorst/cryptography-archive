/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Superclass for all memory IO objects.
	
	A pgpMemoryIO uses memory as a backing store, dynamically resizing
	as needed.
	
	$Id: pgpMemoryIO.h,v 1.4 1997/09/18 01:34:16 lloyd Exp $
____________________________________________________________________________*/

/* base class */
#include "pgpIO.h"
#include "pflContext.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



typedef struct PGPMemoryIO		PGPMemoryIO;
typedef PGPMemoryIO *			PGPMemoryIORef;

PGPError	PGPNewMemoryIO( PFLContextRef context, PGPMemoryIORef *outRef );

/* buffer can't change and caller is responsible for destroying the buffer */
PGPError	PGPNewMemoryIOFixedBuffer( PFLContextRef context,
				void *buffer, PGPSize bufferSize, PGPMemoryIORef *outRef );


PGPBoolean	PGPMemoryIOIsValid( PGPMemoryIORef ref );
#define PGPValidateMemoryIO( ref )	\
	PGPValidateParam( PGPMemoryIOIsValid( ref ) )



#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif
PGP_END_C_DECLARATIONS


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

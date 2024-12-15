/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	File IO that uses a FILE * for its implementation

	$Id: pgpMemoryIOPriv.h,v 1.2 1999/03/10 02:53:21 heller Exp $
____________________________________________________________________________*/

#include "pgpMemoryIO.h"
#include "pgpIOPriv.h"

PGP_BEGIN_C_DECLARATIONS


typedef PGPError	(*PGPMemoryIOReallocateProc)( PGPMemoryIORef ref,
						void **ptr, PGPSize size );

/* the virtual function table for a PGPFileIO */
typedef struct PGPMemoryIOVtbl
{
	PGPIOVtbl					parentVTBL;
	
	PGPMemoryIOReallocateProc	reallocateProc;
} PGPMemoryIOVtbl;


PGPMemoryIOVtbl const *	pgpMemoryIOGetClassVTBL( void );


/* used to initialize */
typedef struct PGPMemoryIOData
{
	PGPBoolean		useFixedBuffer;
	void *			buffer;
	PGPSize			bufferSize;
} PGPMemoryIOData;

struct PGPMemoryIO
{
	PGPIO			parent;
	
	PGPUInt32		memoryIOMagic;
	void *			buffer;
	PGPBoolean		ownsBuffer;
	PGPBoolean		canResize;
	PGPUInt32		bufferSize;
	PGPUInt32		eof;
};
#define kPGPMemoryIOMagic		0x50524f58	/* PROX */



PGP_END_C_DECLARATIONS

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

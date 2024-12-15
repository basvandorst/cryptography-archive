/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	abstract base class for file IO.

	$Id: pgpLineEndIOPriv.h,v 1.1 1997/08/21 01:55:30 lloyd Exp $
____________________________________________________________________________*/

#include "pgpLineEndIO.h"
#include "pgpProxyIOPriv.h"
#include "pgpFileSpec.h"

PGP_BEGIN_C_DECLARATIONS


/* the virtual function table for a PGPLineEndIO */
typedef struct PGPLineEndIOVtbl
{
	PGPProxyIOVtbl			parentVTBL;
} PGPLineEndIOVtbl;


PGPLineEndIOVtbl const *	pgpLineEndIOGetClassVTBL( void );


/* used to initialize */
typedef struct PGPLineEndIOData
{
	PGPProxyIOData		parentData;
	ConvertLineEndType	toType;
} PGPLineEndIOData;


struct PGPLineEndIO
{
	PGPProxyIO			parent;
	PGPUInt32			lineEndConvertMagic;
	ConvertLineEndType	toType;
	
	PGPByte				bufferSpace[ 4096 ];
	PGPByte *			buffer;
	PGPSize				bufferSize;
	PGPSize				bytesInBuffer;
	PGPBoolean			lastCharWasCR;
};
#define kPGPLineEndIOMagic		0x464c494f



PGP_END_C_DECLARATIONS

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

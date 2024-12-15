/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	A PGPIO subclass which converts TEXT file line endings.
	A subclass of PGPProxyIO.

	$Id: pgpLineEndIO.h,v 1.2 1997/09/11 05:38:32 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpLineEndIO_h	/* [ */
#define Included_pgpLineEndIO_h

#include "pflContext.h"

/* base class */
#include "pgpProxyIO.h"



typedef struct PGPLineEndIO	PGPLineEndIO;
typedef PGPLineEndIO *		PGPLineEndIORef;

enum ConvertLineEndType_
{
	kCRLineEndType,
	kLFLineEndType,
	kCRLFLineEndType
};
PGPENUM_TYPEDEF( ConvertLineEndType_, ConvertLineEndType );


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



PGPBoolean	PGPLineEndIOIsValid( PGPLineEndIORef ref );
#define PGPValidateLineEndIO( ref )	\
	PGPValidateParam( PGPLineEndIOIsValid( ref ) )
	

/* creates a new PGPLineEndIO type which converts to the specified type
	of line ending */
PGPError	PGPNewLineEndIO( PGPIORef baseIO, PGPBoolean ownsBase,
				ConvertLineEndType toType, PGPLineEndIORef *outRef );





#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpLineEndIO_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

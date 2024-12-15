/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpKeyIDPriv.h,v 1.5.22.1 1998/11/12 03:21:57 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpKeyIDPriv_h	/* [ */
#define Included_pgpKeyIDPriv_h

#include "pgpPubTypes.h"

/* the non-opaque PGPKeyID */
struct PGPKeyIDPriv
{
	PGPByte		length;
	PGPByte		reserved;
	PGPByte		bytes[ 32 ];
};

PGPError	pgpNewKeyIDFromRawData( const void *data,
				PGPSize dataSize, PGPKeyID *outID );

PGPBoolean	pgpKeyIDsEqual( const PGPKeyID *id1, const PGPKeyID *id2 );

PGPByte const *	pgpGetKeyBytes( PGPKeyID const *keyID );

PGPBoolean	pgpKeyIDIsValid( PGPKeyID const * id );
#define PGPValidateKeyID( id )	PGPValidateParam( pgpKeyIDIsValid( id ) )

#endif /* ] Included_pgpKeyIDPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

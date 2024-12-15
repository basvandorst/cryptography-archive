/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Convert data to/from big endian/little endian formats.

	$Id: pgpEndianConversion.h,v 1.4 1997/09/18 01:34:19 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpEndianConversion_h	/* [ */
#define Included_pgpEndianConversion_h

#include "pgpPFLConfig.h"

#include "pgpBase.h"

enum PGPEndianType_
{
	kPGPBigEndian,
	kPGPLittleEndian,
	
	PGP_ENUM_FORCE( PGPEndianType_ )
};
PGPENUM_TYPEDEF( PGPEndianType_, PGPEndianType );

/* use this if you need to make the endianness explicit */
PGPUInt16	PGPEndianToUInt16( PGPEndianType fromType, const PGPByte *	raw );
PGPUInt32	PGPEndianToUInt32( PGPEndianType fromType, const PGPByte *	raw );


/* convert types to our standard storage format */
/* Do NOT assume what the format is */
void		PGPUInt16ToStorage( PGPUInt16 num,
				PGPByte out[ sizeof( PGPUInt16 ) ] );
void		PGPUInt32ToStorage( PGPUInt32 num,
				PGPByte out[ sizeof( PGPUInt32 ) ] );

/* convert from our standard storage format */
/* Do NOT assume what the format is */
PGPUInt16	PGPStorageToUInt16( PGPByte const in[ sizeof( PGPUInt16 ) ] );
PGPUInt32	PGPStorageToUInt32( PGPByte const in[ sizeof( PGPUInt32 ) ] );



#endif /* ] Included_pgpEndianConversion_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

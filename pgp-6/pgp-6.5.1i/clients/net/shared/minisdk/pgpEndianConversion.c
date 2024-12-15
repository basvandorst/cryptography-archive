/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpEndianConversion.c,v 1.2 1999/03/10 02:50:08 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

#include "pgpEndianConversion.h"

	PGPUInt16
PGPEndianToUInt16(
	PGPEndianType	type,
	const PGPByte *	raw )
{
	PGPUInt16	result;
	
	if ( type == kPGPBigEndian )
	{
		result	= raw[ 0 ];
		result	<<= 8;
		result	|= raw[ 1 ];
	}
	else
	{
		result	= raw[ 1 ];
		result	<<= 8;
		result	|= raw[ 0 ];
	}
	
	return( result );
}


	PGPUInt32
PGPEndianToUInt32(
	PGPEndianType	type,
	const PGPByte *	raw )
{
	PGPUInt32	result;
	
	if ( type == kPGPBigEndian )
	{
		result	= raw[ 0 ];		result	<<= 8;
		result	|= raw[ 1 ];	result	<<= 8;
		result	|= raw[ 2 ];	result	<<= 8;
		result	|= raw[ 3 ];
	}
	else
	{
		result	= raw[ 3 ];		result	<<= 8;
		result	|= raw[ 2 ];	result	<<= 8;
		result	|= raw[ 1 ];	result	<<= 8;
		result	|= raw[ 0 ];
	}
	
	return( result );
}

	void
PGPUInt16ToEndian(
	PGPUInt16		num,
	PGPEndianType	toType,
	PGPByte			out[ sizeof( PGPUInt16 ) ] )
{
	if ( toType == kPGPBigEndian )
	{
		out[0] = num >> 8;
		out[1] = num & 0xFF;		
	}
	else
	{
		out[ 0 ]	= num & 0xFF;
		num			>>= 8;
		out[ 1 ]	= num & 0xFF;
	}
}


	void
PGPUInt32ToEndian(
	PGPUInt32		num,
	PGPEndianType	toType,
	PGPByte			out[ sizeof( PGPUInt32 ) ] )
{
	if ( toType == kPGPBigEndian )
	{
		out[0] = num >> 24;
		out[1] = (num >> 16) & 0xFF;		
		out[2] = (num >> 8) & 0xFF;		
		out[3] = num & 0xFF;		
	}
	else
	{
		out[ 0 ]	= num & 0xFF;
		num			>>= 8;
		out[ 1 ]	= num & 0xFF;
		num			>>= 8;
		out[ 2 ]	= num & 0xFF;
		num			>>= 8;
		out[ 3 ]	= num & 0xFF;
	}
}

	void
PGPUInt16ToStorage(
	PGPUInt16	num,
	PGPByte		out[ sizeof( PGPUInt16 ) ] )
{
	PGPUInt16ToEndian( num, kPGPLittleEndian, out );
}

	void
PGPUInt32ToStorage(
	PGPUInt32	num,
	PGPByte		out[ sizeof( PGPUInt32 ) ] )
{
	PGPUInt32ToEndian( num, kPGPLittleEndian, out );
}

	PGPUInt16
PGPStorageToUInt16( const PGPByte *	in )
{
	return( PGPEndianToUInt16( kPGPLittleEndian, in ) );
}

	PGPUInt32
PGPStorageToUInt32( const PGPByte *	in )
{
	return( PGPEndianToUInt32( kPGPLittleEndian, in ) );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

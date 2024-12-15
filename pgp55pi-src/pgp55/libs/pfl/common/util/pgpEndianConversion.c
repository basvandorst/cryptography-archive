/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpEndianConversion.c,v 1.1 1997/08/28 23:37:20 lloyd Exp $
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
PGPUInt16ToStorage(
	PGPUInt16	num,
	PGPByte		out[ sizeof( PGPUInt16 ) ] )
{
	/* output in little-endian format */
	out[ 0 ]	= num & 0xFF;
	num			>>= 8;
	out[ 1 ]	= num & 0xFF;
}


	void
PGPUInt32ToStorage(
	PGPUInt32	num,
	PGPByte		out[ sizeof( PGPUInt32 ) ] )
{
	/* output in little-endian format */
	out[ 0 ]	= num & 0xFF;
	num			>>= 8;
	out[ 1 ]	= num & 0xFF;
	num			>>= 8;
	out[ 2 ]	= num & 0xFF;
	num			>>= 8;
	out[ 3 ]	= num & 0xFF;
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

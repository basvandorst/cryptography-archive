/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpBitUtils.c,v 1.1.14.1 1998/11/12 03:18:37 heller Exp $
____________________________________________________________________________*/

#include "pgpBitUtils.h"

#define BitIndexToByteMask(index)		(((PGPByte) 1) << (7-((index) & 0x07)))
#define	BitIndexToByte(bitmap,index)	((PGPByte *) bitmap)[(index) >> 3]

static PGPBitCount	sSetBitsInNibble[16] =
							{ 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

#define	SetBitsInByte(byte)				( sSetBitsInNibble[(byte) & 0x0f] + \
										  sSetBitsInNibble[(byte) >> 4] )
	void
pgpSetBit(
	PGPByte 	*bitmap,
	PGPBitIndex	bitIndex)
{
	pgpAssertAddrValid( bitmap, PGPByte );
	
	BitIndexToByte( bitmap, bitIndex ) |= BitIndexToByteMask( bitIndex );
}

	void
pgpSetBitRange(
	PGPByte 	*bitmap,
	PGPBitIndex	startBitIndex,
	PGPBitCount	numBits)
{
	PGPByte		*curByte;
	PGPBitCount	remainingBits;
	
	if( numBits == 0 )
		return;
	
	curByte 		= &BitIndexToByte( bitmap, startBitIndex );
	remainingBits	= numBits;
	
	if( ( startBitIndex & 0x07 ) != 0 )
	{
		PGPUInt32	i;
		PGPUInt32	partalByteBits;

		partalByteBits = 8 - ( startBitIndex & 0x07 );
		if( partalByteBits > remainingBits )
			partalByteBits = remainingBits;
		
		for( i = 0; i < partalByteBits; i++ )
			pgpSetBit( bitmap, startBitIndex + i );
			
		remainingBits -= partalByteBits;
		++curByte;
	}
	
	while( remainingBits >= 8 )
	{
		*curByte++ 		= 0xFF;
		remainingBits 	-= 8;
	}
	
	if( remainingBits != 0 )
	{
		PGPUInt32	i;
		
		/* curByte becomes our new bitmap base for remaining bits */
		for( i = 0; i < remainingBits; i++ )
			pgpSetBit( curByte, i );
	}
}

	void
pgpClearBit(
	PGPByte 	*bitmap,
	PGPBitIndex	bitIndex)
{
	pgpAssertAddrValid( bitmap, PGPByte );
	
	BitIndexToByte( bitmap, bitIndex ) &=
				~BitIndexToByteMask( bitIndex );
}

	void
pgpClearBitRange(
	PGPByte 	*bitmap,
	PGPBitIndex	startBitIndex,
	PGPBitCount	numBits)
{
	PGPByte		*curByte;
	PGPBitCount	remainingBits;
	
	if( numBits == 0 )
		return;
	
	curByte 		= &BitIndexToByte( bitmap, startBitIndex );
	remainingBits	= numBits;
	
	if( ( startBitIndex & 0x07 ) != 0 )
	{
		PGPUInt32	i;
		PGPUInt32	partalByteBits;

		partalByteBits = 8 - ( startBitIndex & 0x07 );
		if( partalByteBits > remainingBits )
			partalByteBits = remainingBits;
		
		for( i = 0; i < partalByteBits; i++ )
			pgpClearBit( bitmap, startBitIndex + i );
			
		remainingBits -= partalByteBits;
		++curByte;
	}
	
	while( remainingBits >= 8 )
	{
		*curByte++ 		= 0;
		remainingBits 	-= 8;
	}
	
	if( remainingBits != 0 )
	{
		PGPUInt32	i;
		
		/* curByte becomes our new bitmap base for remaining bits */
		for( i = 0; i < remainingBits; i++ )
			pgpClearBit( curByte, i );
	}
}

	PGPBoolean
pgpTestBit(
	PGPByte 	*bitmap,
	PGPBitIndex	bitIndex)
{
	pgpAssertAddrValid( bitmap, PGPByte );

	return( ( BitIndexToByte( bitmap, bitIndex ) &
					BitIndexToByteMask( bitIndex ) ) != 0 );
}

	PGPBoolean
pgpTestAndSetBit(
	PGPByte 	*bitmap,
	PGPBitIndex	bitIndex)
{
	PGPByte		mask;
	PGPByte		*byteAddr;
	PGPByte		byteValue;
	
	pgpAssertAddrValid( bitmap, PGPByte );

	mask 		= BitIndexToByteMask( bitIndex );
	byteAddr 	= &BitIndexToByte( bitmap, bitIndex );
	
	byteValue 	= *byteAddr;
	*byteAddr	= byteValue | mask;
	
	return( ( byteValue & mask ) != 0 );
}

	PGPBitCount
pgpCountAndSetBitRange(
	PGPByte 	*bitmap,
	PGPBitIndex	startBitIndex,
	PGPBitCount	numBits)
{
	PGPByte		*curByte;
	PGPBitCount	remainingBits;
	PGPBitCount	numSetBits;
	
	if( numBits == 0 )
		return( 0 );
	
	curByte 		= &BitIndexToByte( bitmap, startBitIndex );
	remainingBits	= numBits;
	numSetBits		= 0;
	
	if( ( startBitIndex & 0x07 ) != 0 )
	{
		PGPUInt32	i;
		PGPUInt32	partalByteBits;

		partalByteBits = 8 - ( startBitIndex & 0x07 );
		if( partalByteBits > remainingBits )
			partalByteBits = remainingBits;
		
		for( i = 0; i < partalByteBits; i++ )
			numSetBits += pgpTestAndSetBit( bitmap, startBitIndex + i );
			
		remainingBits -= partalByteBits;
		++curByte;
	}
	
	while( remainingBits >= 8 )
	{
		numSetBits		+= SetBitsInByte( *curByte );
		*curByte++ 		= 0xFF;
		remainingBits 	-= 8;
	}
	
	if( remainingBits != 0 )
	{
		PGPUInt32	i;
		
		/* curByte becomes our new bitmap base for remaining bits */
		for( i = 0; i < remainingBits; i++ )
			numSetBits += pgpTestAndSetBit( curByte, i );
	}
	
	return( numSetBits );
}

	PGPBoolean
pgpTestAndClearBit(
	PGPByte 	*bitmap,
	PGPBitIndex	bitIndex)
{
	PGPByte		mask;
	PGPByte		*byteAddr;
	PGPByte		byteValue;
	
	pgpAssertAddrValid( bitmap, PGPByte );

	mask 		= BitIndexToByteMask( bitIndex );
	byteAddr 	= &BitIndexToByte( bitmap, bitIndex );
	
	byteValue 	= *byteAddr;
	*byteAddr	= byteValue & ~mask;
	
	return( ( byteValue & mask ) != 0 );
}

	PGPBitCount
pgpCountAndClearBitRange(
	PGPByte 	*bitmap,
	PGPBitIndex	startBitIndex,
	PGPBitCount	numBits)
{
	PGPByte		*curByte;
	PGPBitCount	remainingBits;
	PGPBitCount	numSetBits;
	
	if( numBits == 0 )
		return( 0 );
	
	curByte 		= &BitIndexToByte( bitmap, startBitIndex );
	remainingBits	= numBits;
	numSetBits		= 0;
	
	if( ( startBitIndex & 0x07 ) != 0 )
	{
		PGPUInt32	i;
		PGPUInt32	partalByteBits;

		partalByteBits = 8 - ( startBitIndex & 0x07 );
		if( partalByteBits > remainingBits )
			partalByteBits = remainingBits;
		
		for( i = 0; i < partalByteBits; i++ )
			numSetBits += pgpTestAndClearBit( bitmap, startBitIndex + i );
			
		remainingBits -= partalByteBits;
		++curByte;
	}
	
	while( remainingBits >= 8 )
	{
		numSetBits		+= SetBitsInByte( *curByte );
		*curByte++ 		= 0;
		remainingBits 	-= 8;
	}
	
	if( remainingBits != 0 )
	{
		PGPUInt32	i;
		
		/* curByte becomes our new bitmap base for remaining bits */
		for( i = 0; i < remainingBits; i++ )
			numSetBits += pgpTestAndClearBit( curByte, i );
	}
	
	return( numSetBits );
}

	PGPBitCount
pgpCountSetBitsInRange(
	PGPByte 	*bitmap,
	PGPBitIndex	startBitIndex,
	PGPBitCount	numBits)
{
	PGPByte		*curByte;
	PGPBitCount	remainingBits;
	PGPBitCount	numSetBits;
	
	if( numBits == 0 )
		return( 0 );
	
	curByte 		= &BitIndexToByte( bitmap, startBitIndex );
	remainingBits	= numBits;
	numSetBits		= 0;
	
	if( ( startBitIndex & 0x07 ) != 0 )
	{
		PGPUInt32	i;
		PGPUInt32	partalByteBits;

		partalByteBits = 8 - ( startBitIndex & 0x07 );
		if( partalByteBits > remainingBits )
			partalByteBits = remainingBits;
		
		for( i = 0; i < partalByteBits; i++ )
			numSetBits += ( pgpTestBit( bitmap, startBitIndex + i ) != 0 );
			
		remainingBits -= partalByteBits;
		++curByte;
	}
	
	while( remainingBits >= 8 )
	{
		/* Do not increment curByte in SetBitsInByte() because it is a macro */
		numSetBits		+= SetBitsInByte( *curByte );
		remainingBits 	-= 8;
		++curByte;			
	}
	
	if( remainingBits != 0 )
	{
		PGPUInt32	i;
		
		/* curByte becomes our new bitmap base for remaining bits */
		for( i = 0; i < remainingBits; i++ )
			numSetBits += ( pgpTestBit( curByte, i ) != 0 );
	}
	
	return( numSetBits );
}

	PGPBitCount
pgpCountClearBitsInRange(
	PGPByte 	*bitmap,
	PGPBitIndex	startBitIndex,
	PGPBitCount	numBits)
{
	return( numBits - pgpCountSetBitsInRange(bitmap, startBitIndex, numBits) );
}
/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
 	Portions of this code are (C) Copyright 1995-1996 by
 	David K. Heller and are provided
	to PGP without restriction.

	$Id: MacNumerics.c,v 1.5.10.1 1997/12/07 04:27:12 mhw Exp $
____________________________________________________________________________*/



#include "MacNumerics.h"

/*____________________________________________________________________________
	BCDToDecimal: Convert a 32 bit BCD value to decimal form
____________________________________________________________________________*/

	void
BCDToDecimal(UInt32 bcdValue, UInt32 *decimalValue)
{
	UInt32		value;
	UInt32		multiplier;
	UInt32		curBCD;
	ItemCount	iteration;
	
	pgpAssertAddrValid( decimalValue, UInt32 );
	
	value 		= 0;
	multiplier	= 1;
	curBCD		= bcdValue;
	
	/* Get each nibble of the BCD value,
	multiply by the appropriate multiple of
		10 and add to the result.*/
	
	for( iteration = 1; iteration <= 8; iteration++ )
	{
		UInt32	curNibble;
		
		curNibble = curBCD & 0x0000000F;
		pgpAssertMsg( curNibble <= 9, "BCDToDecimal: Invalid BCD value" );
		
		value 		+= curNibble * multiplier;
		multiplier 	*= 10;
		
		curBCD = curBCD >> 4;
		if( curBCD == 0 )
			break;
	}
	
	*decimalValue = value;	
}

	CRC16
ContinueCRC16(const void *theBytes, UInt32 numBytes, CRC16 startCRC)
{
	register CRC16	crc 	= startCRC;
	register uchar	*cur 	= (uchar *) theBytes;
	register uchar	*last	= cur + numBytes - 1;
	register SInt16	k1021	= 0x1021;
	
	while(cur <= last)
	{
		register SInt16	check;
		
		crc ^= ((SInt16) *cur) << 8;
		++cur;
		
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k1021;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k1021;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k1021;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k1021;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k1021;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k1021;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k1021;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k1021;
	}		
	
	return( crc );
}

	CRC16
ComputeCRC16(const void *theBytes, UInt32 numBytes)
{
	return( ContinueCRC16( theBytes, numBytes, 0 ) );
}

	CRC32
ContinueCRC32(const void *theBytes, UInt32 numBytes, CRC32 startCRC)
{
	register CRC32 	crc 		= startCRC;
	register uchar	*cur 		= (uchar *) theBytes;
	register uchar	*last		= cur + numBytes - 1;
	register SInt32	k4C11DB7	= 0x04C11DB7;
	
	while(cur <= last)
	{
		register SInt32	check;
		
		crc ^= ((SInt32 ) *cur) << 8;
		++cur;

		check = crc; crc <<= 1; if( check < 0 ) crc ^= k4C11DB7;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k4C11DB7;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k4C11DB7;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k4C11DB7;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k4C11DB7;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k4C11DB7;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k4C11DB7;
		check = crc; crc <<= 1; if( check < 0 ) crc ^= k4C11DB7;
	}
	
	return( crc );
}
		
	CRC32
ComputeCRC32(const void *theBytes, UInt32 numBytes)
{
	return( ContinueCRC32( theBytes, numBytes, 0 ) );
}
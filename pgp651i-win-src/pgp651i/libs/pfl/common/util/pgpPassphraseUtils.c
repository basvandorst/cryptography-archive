/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpPassphraseUtils.c,v 1.8 1999/03/25 18:14:39 melkins Exp $
____________________________________________________________________________*/

#include <math.h>
#include <string.h>

#include "pgpMem.h"

#include "pgpPassphraseUtils.h"

typedef struct ScoreStruct
{
	uchar	firstChar;
	uchar	lastChar;
} Range;
	
static const Range	sRanges[] =
{
	{ 0,		' ' - 1},
	{ ' ',		'A' - 1},
	{ 'A',		'Z'},
	{ 'Z' + 1,	'a' - 1},
	{ 'a',		'z'},
	{ '0',		'9'},
	{ 'z' + 1,	127},
	{ 128,		255},
};

#define	kNumRanges	( sizeof( sRanges ) / sizeof( sRanges[ 0 ] ) )

	PGPUInt32
pgpEstimatePassphraseQuality(
	const char *passphrase)
{
	PGPUInt16	quality;
	PGPUInt16	passphraseLength;
	PGPUInt16 	charIndex;
	PGPUInt16 	rangeIndex;
	PGPUInt16	span = 0;
	Boolean		haveRange[ kNumRanges ];
	
	pgpAssertAddrValid( passphrase, char );
	pgpClearMemory( haveRange, sizeof( haveRange ) );
	
	passphraseLength = strlen( passphrase );
	for( charIndex = 0; charIndex < passphraseLength; ++charIndex )
	{
		for( rangeIndex = 0; rangeIndex < kNumRanges; ++rangeIndex )
		{
			uchar			theChar;
			const Range *	theRange;
			
			theChar		= passphrase[ charIndex ];
			theRange	= &sRanges[ rangeIndex ];
			
			if ( theChar >= theRange->firstChar && 
				theChar <= theRange->lastChar )
			{
				haveRange[ rangeIndex ] 	= TRUE;
				break;
			}
		}
	}
	
	/* determine the total span */
	for( rangeIndex = 0; rangeIndex < kNumRanges; ++rangeIndex )
	{
		if ( haveRange[ rangeIndex ] )
		{
			span	+= 1 + ( sRanges[ rangeIndex ].lastChar -
				sRanges[ rangeIndex ].firstChar );
		}
	}
	pgpAssert( span <= 256 );
	
	if ( span != 0 )
	{
		double		bitsPerChar	= log( (double) span ) / log( 2.0 );
		PGPUInt32	totalBits	= (PGPUInt32) bitsPerChar * passphraseLength;
		
		/* assume a maximum quality of 128 bits */
		quality = ( totalBits * 100 ) / 128;
		if( quality > 100 )
			quality = 100;
	}
	else
	{
		quality	= 0;
	}
		
	return( quality );
}


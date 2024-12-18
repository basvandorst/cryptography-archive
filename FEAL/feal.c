/* FEALNX.C - Implementation of the FEALNX package.
Version of 92.12.28 by Peter Pearson.

This implementation strives primarily for portability and secondarily for
clarity, with execution speed only a tertiary consideration. On
processors with 16-bit or 32-bit registers, the quickest way to boost
execution speed is to lump the data into pieces bigger than 8 bits.
*/

#if 0
#include <alloc.h>	/* For "malloc".		*/
#include <mem.h>	/* For "memcpy" and "memset".	*/
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fealnx.h"

#define movmem	memcpy

#define S0(x,y)	Rot2((x)+(y))
#define S1(x,y)	Rot2((x)+(y)+1)

void CalcFk( unsigned char Output[4], unsigned char Alpha[4],
				      unsigned char Beta[4] )
/*
	Calculate the function "fk" of the two specified inputs,
	and write the resulting four bytes to the provided output.
*/
{
    unsigned char t1, t2 ;
    unsigned char Rot2( int A ) ;

    t1 = Alpha[0] ^ Alpha[1] ;
    t2 = Alpha[2] ^ Alpha[3] ;
    t1 = S1( t1, t2 ^ Beta[0] ) ;
    t2 = S0( t2, t1 ^ Beta[1] ) ;
    Output[1] = t1 ;
    Output[2] = t2 ;
    Output[0] = S0( Alpha[0], t1 ^ Beta[2] ) ;
    Output[3] = S1( Alpha[3], t2 ^ Beta[3] ) ;
}

void Decrypt( KeyScheduleType K, DataBlockType Cipher, DataBlockType Plain )
{
    int Rounds ;
    int i ;
    unsigned char L[4], R[4], NewR[4] ;
    unsigned char *KP ;
    unsigned char *Xor4( unsigned char A[4], unsigned char B[4] ) ;
    unsigned char *F( unsigned char K[2], unsigned char R[4] ) ;

    Rounds = K->NRounds ;
    KP = K->KSchedule + 2 * Rounds ;
    movmem(          Xor4( KP + 8,  Cipher       ), L, 4 ) ;
    movmem( Xor4( L, Xor4( KP + 12, Cipher + 4 ) ), R, 4 ) ;
    for ( i = 0 ; i < Rounds ; ++i )
    {
	KP -= 2 ;
	movmem( Xor4( L, F( KP, R ) ), NewR, 4 ) ;
	movmem( R, L, 4 ) ;
	movmem( NewR, R, 4 ) ;
    }	/* Fall out with KP pointing to KSchedule.	*/
    KP = K->KSchedule + 2 * Rounds ;
    movmem( Xor4( KP    , R            ), Plain, 4 ) ;
    movmem( Xor4( KP + 4, Xor4( R, L ) ), Plain + 4, 4 ) ;
}

void Encrypt( KeyScheduleType K, DataBlockType Plain, DataBlockType Cipher )
{
    int Rounds ;
    int i ;
    unsigned char L[4], R[4], NewR[4] ;
    unsigned char *KP ;
    unsigned char *Xor4( unsigned char A[4], unsigned char B[4] ) ;
    unsigned char *F( unsigned char K[2], unsigned char R[4] ) ;

    KP = K->KSchedule ;
    Rounds = K->NRounds ;
    movmem(          Xor4( KP + 2 * Rounds,     Plain       ), L, 4 ) ;
    movmem( Xor4( L, Xor4( KP + 2 * Rounds + 4, Plain + 4 ) ), R, 4 ) ;
    for ( i = 0 ; i < Rounds ; ++i, KP += 2 )
    {
	movmem( Xor4( L, F( KP, R ) ), NewR, 4 ) ;
	movmem( R, L, 4 ) ;
	movmem( NewR, R, 4 ) ;
    }	/* Fall out with KP pointing to KSchedule[ 2*Rounds ].	*/
    movmem( Xor4( KP + 8,  R            ), Cipher, 4 ) ;
    movmem( Xor4( KP + 12, Xor4( R, L ) ), Cipher + 4, 4 ) ;
}

unsigned char *F( unsigned char Beta[2], unsigned char Alpha[4] )
/*
	This function implements the "f" box in the Feal-N
	flowchart.

The result is returned in a static array, and will be overwritten
when this function is next called.
*/
{
    unsigned char t1, t2 ;
    static unsigned char Result[4] ;
    unsigned char Rot2( int A ) ;

    t1 = Alpha[0] ^ Alpha[1] ^ Beta[0] ;
    t2 = Alpha[2] ^ Alpha[3] ^ Beta[1] ;
    t1 = S1( t1, t2 ) ;
    t2 = S0( t2, t1 ) ;
    Result[1] = t1 ;
    Result[2] = t2 ;
    Result[0] = S0( t1, Alpha[0] ) ;
    Result[3] = S1( t2, Alpha[3] ) ;
    return Result ;
}

KeyScheduleType NewKeySchedule( int Rounds, unsigned char *Key )
{
    KeyScheduleType Result ;
    int Step ;
    unsigned char *KSP ;
    unsigned char a[4], b[4], c[4], d[4] ;
    void CalcFk( unsigned char Output[4], unsigned char In1[4],
					  unsigned char In2[4] ) ;
    unsigned char *Xor4( unsigned char A[4], unsigned char B[4] ) ;

    Result = malloc( ( sizeof( *Result ) + Rounds + 8 ) * 2 ) ;
    if ( Result != NULL )
    {
	Result->NRounds = Rounds ;
	Result->KSchedule = ( unsigned char *) ( Result + 1 ) ;
	memcpy( a, Key,   4 ) ;
	memcpy( b, Key+4, 4 ) ;
	memcpy( c, Xor4( Key + 8, Key + 12 ), 4 ) ;
	memset( d, 0, 4 ) ;
	KSP = Result->KSchedule ;
	for ( Step = 0 ; Step < Rounds/2 + 4 ; ++Step, KSP += 4 )
	{
	    switch( Step % 3 )
	    {
		case 0:
		    CalcFk( KSP, a, Xor4( d, Xor4( b, c ) ) ) ;
		    break ;

		case 1:
		    CalcFk( KSP, a, Xor4( d, Xor4( b, Key + 8 ) ) ) ;
		    break ;

		case 2:
		    CalcFk( KSP, a, Xor4( d, Xor4( b, Key + 12 ) ) ) ;
		    break ;
	    }
	    memcpy( d, a, 4 ) ;
	    memcpy( a, b, 4 ) ;
	    memcpy( b, KSP, 4 ) ;
	}
    }
    return Result ;
}

unsigned char Rot2( int X )
/*
	Return X (an 8-bit quantity) rotated left by two bits.

	(In an uncharacteristic concession to execution speed, we
	implement this function by table lookup.)
*/
{
    static First = 1 ;
    static unsigned char Result[ 256 ] ;

    if ( First )
    {
	int i ;

	for ( i = 0 ; i < 256 ; ++i )
	    Result[i] = ( i << 2 ) + ( i >> 6 ) ;
	First = 0 ;
    }

    return Result[ X & 0xFF ] ;
}

unsigned char *Xor4( unsigned char A[4], unsigned char B[4] )
/*
	Exclusive-or two 4-byte quantities. Return a pointer to
	the result.

	The pointer returned points to a static variable that will
	be overwritten the next time this function is called. We
	guarantee that calls to this function may be nested simply
	(e.g., Xor4( a, Xor4( b, Xor4( c, d ) ) ) ) but anything
	more complex may behave badly. (For example,
	Xor4( Xor4( a, b ), Xor4( c, d ) ) will result in all 0s) .
*/
{
    static unsigned char Result[4] ;

    Result[0] = A[0] ^ B[0] ;
    Result[1] = A[1] ^ B[1] ;
    Result[2] = A[2] ^ B[2] ;
    Result[3] = A[3] ^ B[3] ;
    return Result ;
}

From msuinfo!uchinews!ncar!destroyer!gumby!wupost!waikato.ac.nz!aukuni.ac.nz!cs18.cs.aukuni.ac.nz!pgut1 Mon Sep  7 12:13:02 1992
Path: msuinfo!uchinews!ncar!destroyer!gumby!wupost!waikato.ac.nz!aukuni.ac.nz!cs18.cs.aukuni.ac.nz!pgut1
Newsgroups: sci.crypt
Subject: SHS/SHA source code + random thoughts
Message-ID: <1992Sep4.060230.28313@cs.aukuni.ac.nz>
From: Peter Gutmann (pgut1@cs.aukuni.ac.nz)
Date: Fri, 4 Sep 1992 06:02:30 GMT
Sender: pgut1@cs.aukuni.ac.nz (PeterClaus          Gutmann        )
Organization: HPACK Conspiracy Secret Laboratory
Keywords: SHS SHA message digest
Summary: Source code for SHS/SHA message digest algorithm + comparison with MD5
Lines: 503

  The following is a C implementation of the NIST SHA (Secure Hash Algorithm) -
this is sometimes also referred to as SHS (Secure Hash Standard), but I've used
SHA for now since the standard hasn't been adopted yet.  It's called SHS and
SHA rather interchangeably just to confuse people, I've called it SHA in the
text but SHS in the code in anticipation of it becoming a standard.  Once it's
adopted, you can feed the text through sed and change all the names.  I'll
assume everyone has a copy of the SHS document and won't bother including it in
this posting (it's rather long).


The SHS/SHA Code
================

  It's a fairly straightforward implementation which has been tested under
MSDOS and OS/2 on a PClone, and Unix on a DECstation.  Some of the
optimizations used are mentioned in the code.  One of the results of these
optimizations is that the code isn't endianness-independant, meaning that on
little-endian machines a byteReverse() function has to be used for
endianness-reversal.  This entails defining LITTLE_ENDIAN on a little-endian
system (it's currently defined by default in SHS.H).  Being able to assume a
given data endianness makes getting data to the SHA transform() routine a lot
faster.


SHA Speed
=========

  I ran the SHA code against the distributed version of MD5 (which is
significantly less optimised than the SHA code).  The results were as follows:

	      25 MHz PClone     DECstation 2100     DECstation 5000

    SHA         31 K/sec           120 K/sec           208 K/sec
    MD5         55 K/sec           169 K/sec           278 K/sec

This comparison isn't 100% fair since the standard MD5 distribution is a
somewhat pessimal implementation (in fact an optimised PC version runs around 5
times faster).  An implementation of MD5 optimised to the level of SHA runs
around 2 times faster.

Therefore with similar levels of optimisation it appears MD5 is around three
times as fast on the PClone as SHA.  Even the pessimal MD5 on the DECstations
is around a third as fast again as SHA, and would probably also be 2 or more
times as fast if optimized (I used the standard MD5 distribution rather than
an optimized custom one to allow others to verify the results).


SHA's Awkward Block Size (Some Flamage)
========================

Will SHA be weakened by taking out h4 and E and reducing the number of rounds
to create a 128-bit MD algorithm?  This seems a lot nicer than the current one
since it's now a power-of-two size which fits in a lot better with most current
software.  Removing h4 and E and reducing the total number of rounds by 16
doesn't seem to weaken it any, and IMHO the decision to force SHA to fit an
awkward DSS data size wasn't such a hot idea, especially if it's to be used
with non-DSS code or if q is ever changed.


SHA vs MD5
==========

When implementing SHA I noticed how very similar it was to MD4.  The main
changes were the addition of an the 'expand' transformation (Step B), the
addition of the previous round's output into the next round for a faster
avalance effect, and the increasing of the whole transformation to fit the DSS
block size.  SHA is very much an MD4-variant rather than a redesign like MD5.

The design decisions behind MD5 were given in the MD5 document, the design
for SHA is never gone into in the SHS/SHA document (mind you it's pretty
obvious what's going on - everything except how the Mysterious Constants
were chosen can be seen at a glance).  Presumably some of the changes made
were to avoid the known attacks for MD4, but again no design details are
given.  Anyway, using what I had available I took the points raised in the
MD5 design and compared them with what SHA did:

- A fourth round has been added.

  SHA does this too.  However in SHA the fourth round uses the same f-function
  as the second round (not obvious whether this is a problem or not, I'll have
  to look at it a bit more).

- Each step now has a unique additive constant.

  SHA keeps the MD4 scheme where it reuses the constants for each group of
  rounds (in this case for 20 rounds at a time).  Actually MD4 only has the
  additive constants in the last two rounds, not for all 4, but the principal
  is the same - the constants are reused many times.

- The function g in round 2 was changed from ( XY v XZ v YZ ) to
  ( XZ v Y not( Z ) ) to make g less symmetric.

  SHA uses the MD4 version ( XY v XZ v YZ ) (SHA calls it f2 rather than g).

- Each step now adds in the result of the previous step.  This promotes a faster
  "avalanche effect".

  This change has been made in SHA as well.

- The order in which input words are accessed in rounds 2 and 3 is changed, to
  make these patterns less like each other.
      
  SHA always access the words the same way, like MD4.

- The shift amounts in each round have been approximately optimized, to yield a
  faster "avalanche effect". The shifts in different rounds are distinct.

  SHA uses a constant shift amount in each round.  This shift amount is
  relatively prime to the word size (as in MD4).

If you take SHA and remove h4, E, and a few rounds, the result is (basically)
MD4 with one more round, the addition of the output of step n-1 to step n
(giving a faster avalanche effect), and the addition of the initial "expand"
transformation.  This initial transformation is important, since it spreads the
input data bits out over an area four times as large as the original, and then
mixes in the expanded version of the data in each round.  This means that
instead of reusing the input data in each group of rounds, SHA uses different
permutations of the input data in each group of rounds.  This is definitely A
Good Thing.

This leads to the following situation:  

  SHA = MD4 + 'expand' transformation + extra round + better-avalanche

  MD5 = MD4 + improved bit-bashing + extra round + better-avalanche

Which is stronger, MD5 with its improved bit-bashing or SHA with it's 'expand'
transformation?  (Ain't no way I'm going to answer this one :-).

One point is that the only "extra" in SHA which MD5 doesn't have, namely the
'expand' transformation, can be easily added to MD5, but that the MD5
improvements can't be added to SHA without redesigning the whole algorithm.
Thus the paranoid types can hedge their bets by adding 'expand' to MD5, giving
them the best of both worlds (its very easy to simply change MD5 to have the
'expand' transformation - maybe this is an MD6 in the making?).
			  

Conclusion
==========

  This positing is beginning to sound as if I'm the official net.apologist for
MD5 :-).  Maybe I'm being a bit harsh on SHA, but I think someone should point
out that it may not be the be-all and end-all of message digest algorithms.  I
welcome any email on the subject, or post your flames here....

Peter.

------------------------------- Chainsaw here ---------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* --------------------------------- SHS.H ------------------------------- */

/* NIST proposed Secure Hash Standard.

   Written 2 September 1992, Peter C. Gutmann.
   This implementation placed in the public domain.

   Comments to pgut1@cs.aukuni.ac.nz */

/* Useful defines/typedefs */

typedef unsigned char   BYTE;
typedef unsigned long   LONG;

/* The SHS block size and message digest sizes, in bytes */

#define SHS_BLOCKSIZE   64
#define SHS_DIGESTSIZE  20

/* The structure for storing SHS info */

typedef struct {
	       LONG digest[ 5 ];            /* Message digest */
	       LONG countLo, countHi;       /* 64-bit bit count */
	       LONG data[ 16 ];             /* SHS data buffer */
	       } SHS_INFO;

/* Whether the machine is little-endian or not */

#define LITTLE_ENDIAN

/* --------------------------------- SHS.C ------------------------------- */

/* NIST proposed Secure Hash Standard.

   Written 2 September 1992, Peter C. Gutmann.
   This implementation placed in the public domain.

   Comments to pgut1@cs.aukuni.ac.nz */

#include <string.h>

/* The SHS f()-functions */

#define f1(x,y,z)   ( ( x & y ) | ( ~x & z ) )              /* Rounds  0-19 */
#define f2(x,y,z)   ( x ^ y ^ z )                           /* Rounds 20-39 */
#define f3(x,y,z)   ( ( x & y ) | ( x & z ) | ( y & z ) )   /* Rounds 40-59 */
#define f4(x,y,z)   ( x ^ y ^ z )                           /* Rounds 60-79 */

/* The SHS Mysterious Constants */

#define K1  0x5A827999L     /* Rounds  0-19 */
#define K2  0x6ED9EBA1L     /* Rounds 20-39 */
#define K3  0x8F1BBCDCL     /* Rounds 40-59 */
#define K4  0xCA62C1D6L     /* Rounds 60-79 */

/* SHS initial values */

#define h0init  0x67452301L
#define h1init  0xEFCDAB89L
#define h2init  0x98BADCFEL
#define h3init  0x10325476L
#define h4init  0xC3D2E1F0L

/* 32-bit rotate - kludged with shifts */

#define S(n,X)  ( ( X << n ) | ( X >> ( 32 - n ) ) )

/* The initial expanding function */

#define expand(count)   W[ count ] = W[ count - 3 ] ^ W[ count - 8 ] ^ W[ count - 14 ] ^ W[ count - 16 ]

/* The four SHS sub-rounds */

#define subRound1(count)    \
    { \
    temp = S( 5, A ) + f1( B, C, D ) + E + W[ count ] + K1; \
    E = D; \
    D = C; \
    C = S( 30, B ); \
    B = A; \
    A = temp; \
    }

#define subRound2(count)    \
    { \
    temp = S( 5, A ) + f2( B, C, D ) + E + W[ count ] + K2; \
    E = D; \
    D = C; \
    C = S( 30, B ); \
    B = A; \
    A = temp; \
    }

#define subRound3(count)    \
    { \
    temp = S( 5, A ) + f3( B, C, D ) + E + W[ count ] + K3; \
    E = D; \
    D = C; \
    C = S( 30, B ); \
    B = A; \
    A = temp; \
    }

#define subRound4(count)    \
    { \
    temp = S( 5, A ) + f4( B, C, D ) + E + W[ count ] + K4; \
    E = D; \
    D = C; \
    C = S( 30, B ); \
    B = A; \
    A = temp; \
    }

/* The two buffers of 5 32-bit words */

LONG h0, h1, h2, h3, h4;
LONG A, B, C, D, E;

/* Initialize the SHS values */

void shsInit( SHS_INFO *shsInfo )
    {
    /* Set the h-vars to their initial values */
    shsInfo->digest[ 0 ] = h0init;
    shsInfo->digest[ 1 ] = h1init;
    shsInfo->digest[ 2 ] = h2init;
    shsInfo->digest[ 3 ] = h3init;
    shsInfo->digest[ 4 ] = h4init;

    /* Initialise bit count */
    shsInfo->countLo = shsInfo->countHi = 0L;
    }

/* Perform the SHS transformation.  Note that this code, like MD5, seems to
   break some optimizing compilers - it may be necessary to split it into
   sections, eg based on the four subrounds */

void shsTransform( SHS_INFO *shsInfo )
    {
    LONG W[ 80 ], temp;
    int i;

    /* Step A.  Copy the data buffer into the local work buffer */
    for( i = 0; i < 16; i++ )
	W[ i ] = shsInfo->data[ i ];

    /* Step B.  Expand the 16 words into 64 temporary data words */
    expand( 16 ); expand( 17 ); expand( 18 ); expand( 19 ); expand( 20 );
    expand( 21 ); expand( 22 ); expand( 23 ); expand( 24 ); expand( 25 );
    expand( 26 ); expand( 27 ); expand( 28 ); expand( 29 ); expand( 30 );
    expand( 31 ); expand( 32 ); expand( 33 ); expand( 34 ); expand( 35 );
    expand( 36 ); expand( 37 ); expand( 38 ); expand( 39 ); expand( 40 );
    expand( 41 ); expand( 42 ); expand( 43 ); expand( 44 ); expand( 45 );
    expand( 46 ); expand( 47 ); expand( 48 ); expand( 49 ); expand( 50 );
    expand( 51 ); expand( 52 ); expand( 53 ); expand( 54 ); expand( 55 );
    expand( 56 ); expand( 57 ); expand( 58 ); expand( 59 ); expand( 60 );
    expand( 61 ); expand( 62 ); expand( 63 ); expand( 64 ); expand( 65 );
    expand( 66 ); expand( 67 ); expand( 68 ); expand( 69 ); expand( 70 );
    expand( 71 ); expand( 72 ); expand( 73 ); expand( 74 ); expand( 75 );
    expand( 76 ); expand( 77 ); expand( 78 ); expand( 79 );

    /* Step C.  Set up first buffer */
    A = shsInfo->digest[ 0 ];
    B = shsInfo->digest[ 1 ];
    C = shsInfo->digest[ 2 ];
    D = shsInfo->digest[ 3 ];
    E = shsInfo->digest[ 4 ];

    /* Step D.  Serious mangling, divided into four sub-rounds */
    subRound1( 0 ); subRound1( 1 ); subRound1( 2 ); subRound1( 3 );
    subRound1( 4 ); subRound1( 5 ); subRound1( 6 ); subRound1( 7 );
    subRound1( 8 ); subRound1( 9 ); subRound1( 10 ); subRound1( 11 );
    subRound1( 12 ); subRound1( 13 ); subRound1( 14 ); subRound1( 15 );
    subRound1( 16 ); subRound1( 17 ); subRound1( 18 ); subRound1( 19 );
    subRound2( 20 ); subRound2( 21 ); subRound2( 22 ); subRound2( 23 );
    subRound2( 24 ); subRound2( 25 ); subRound2( 26 ); subRound2( 27 );
    subRound2( 28 ); subRound2( 29 ); subRound2( 30 ); subRound2( 31 );
    subRound2( 32 ); subRound2( 33 ); subRound2( 34 ); subRound2( 35 );
    subRound2( 36 ); subRound2( 37 ); subRound2( 38 ); subRound2( 39 );
    subRound3( 40 ); subRound3( 41 ); subRound3( 42 ); subRound3( 43 );
    subRound3( 44 ); subRound3( 45 ); subRound3( 46 ); subRound3( 47 );
    subRound3( 48 ); subRound3( 49 ); subRound3( 50 ); subRound3( 51 );
    subRound3( 52 ); subRound3( 53 ); subRound3( 54 ); subRound3( 55 );
    subRound3( 56 ); subRound3( 57 ); subRound3( 58 ); subRound3( 59 );
    subRound4( 60 ); subRound4( 61 ); subRound4( 62 ); subRound4( 63 );
    subRound4( 64 ); subRound4( 65 ); subRound4( 66 ); subRound4( 67 );
    subRound4( 68 ); subRound4( 69 ); subRound4( 70 ); subRound4( 71 );
    subRound4( 72 ); subRound4( 73 ); subRound4( 74 ); subRound4( 75 );
    subRound4( 76 ); subRound4( 77 ); subRound4( 78 ); subRound4( 79 );

    /* Step E.  Build message digest */
    shsInfo->digest[ 0 ] += A;
    shsInfo->digest[ 1 ] += B;
    shsInfo->digest[ 2 ] += C;
    shsInfo->digest[ 3 ] += D;
    shsInfo->digest[ 4 ] += E;
    }

#ifdef LITTLE_ENDIAN

/* When run on a little-endian CPU we need to perform byte reversal on an
   array of longwords.  It is possible to make the code endianness-
   independant by fiddling around with data at the byte level, but this
   makes for very slow code, so we rely on the user to sort out endianness
   at compile time */

static void byteReverse( LONG *buffer, int byteCount )
    {
    LONG value;
    int count;

    byteCount /= sizeof( LONG );
    for( count = 0; count < byteCount; count++ )
	{
	value = ( buffer[ count ] << 16 ) | ( buffer[ count ] >> 16 );
	buffer[ count ] = ( ( value & 0xFF00FF00L ) >> 8 ) | ( ( value & 0x00FF00FFL ) << 8 );
	}
    }
#endif /* LITTLE_ENDIAN */

/* Update SHS for a block of data.  This code assumes that the buffer size
   is a multiple of SHS_BLOCKSIZE bytes long, which makes the code a lot
   more efficient since it does away with the need to handle partial blocks
   between calls to shsUpdate() */

void shsUpdate( SHS_INFO *shsInfo, BYTE *buffer, int count )
    {
    /* Update bitcount */
    if( ( shsInfo->countLo + ( ( LONG ) count << 3 ) ) < shsInfo->countLo )
	shsInfo->countHi++; /* Carry from low to high bitCount */
    shsInfo->countLo += ( ( LONG ) count << 3 );
    shsInfo->countHi += ( ( LONG ) count >> 29 );

    /* Process data in SHS_BLOCKSIZE chunks */
    while( count >= SHS_BLOCKSIZE )
	{
	memcpy( shsInfo->data, buffer, SHS_BLOCKSIZE );
#ifdef LITTLE_ENDIAN
	byteReverse( shsInfo->data, SHS_BLOCKSIZE );
#endif /* LITTLE_ENDIAN */
	shsTransform( shsInfo );
	buffer += SHS_BLOCKSIZE;
	count -= SHS_BLOCKSIZE;
	}

    /* Handle any remaining bytes of data.  This should only happen once
       on the final lot of data */
    memcpy( shsInfo->data, buffer, count );
    }

void shsFinal( SHS_INFO *shsInfo )
    {
    int count;
    LONG lowBitcount = shsInfo->countLo, highBitcount = shsInfo->countHi;

    /* Compute number of bytes mod 64 */
    count = ( int ) ( ( shsInfo->countLo >> 3 ) & 0x3F );

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    ( ( BYTE * ) shsInfo->data )[ count++ ] = 0x80;

    /* Pad out to 56 mod 64 */
    if( count > 56 )
	{
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset( ( BYTE * ) &shsInfo->data + count, 0, 64 - count );
#ifdef LITTLE_ENDIAN
	byteReverse( shsInfo->data, SHS_BLOCKSIZE );
#endif /* LITTLE_ENDIAN */
	shsTransform( shsInfo );

	/* Now fill the next block with 56 bytes */
	memset( &shsInfo->data, 0, 56 );
	}
    else
	/* Pad block to 56 bytes */
	memset( ( BYTE * ) &shsInfo->data + count, 0, 56 - count );
#ifdef LITTLE_ENDIAN
    byteReverse( shsInfo->data, SHS_BLOCKSIZE );
#endif /* LITTLE_ENDIAN */

    /* Append length in bits and transform */
    shsInfo->data[ 14 ] = highBitcount;
    shsInfo->data[ 15 ] = lowBitcount;

    shsTransform( shsInfo );
#ifdef LITTLE_ENDIAN
    byteReverse( shsInfo->data, SHS_DIGESTSIZE );
#endif /* LITTLE_ENDIAN */
    }

/* ----------------------------- SHS Test code --------------------------- */

/* Size of buffer for SHS speed test data */

#define TEST_BLOCK_SIZE     ( SHS_DIGESTSIZE * 100 )

/* Number of bytes of test data to process */

#define TEST_BYTES          10000000L
#define TEST_BLOCKS         ( TEST_BYTES / TEST_BLOCK_SIZE )

void main( void )
    {
    SHS_INFO shsInfo;
    time_t endTime, startTime;
    BYTE data[ TEST_BLOCK_SIZE ];
    long i;

    /* Test output data (this is the only test data given in the SHS
       document, but chances are if it works for this it'll work for
       anything) */
    shsInit( &shsInfo );
    shsUpdate( &shsInfo, ( BYTE * ) "abc", 3 );
    shsFinal( &shsInfo );
    if( shsInfo.digest[ 0 ] != 0x0164B8A9L || \
	shsInfo.digest[ 1 ] != 0x14CD2A5EL || \
	shsInfo.digest[ 2 ] != 0x74C4F7FFL || \
	shsInfo.digest[ 3 ] != 0x082C4D97L || \
	shsInfo.digest[ 4 ] != 0xF1EDF880L )
	{
	puts( "Error in SHS implementation" );
	exit( -1 );
	}

    /* Now perform time trial, generating MD for 10MB of data.  First,
       initialize the test data */
    memset( data, 0, TEST_BLOCK_SIZE );

    /* Get start time */
    printf( "SHS time trial.  Processing %ld characters...\n", TEST_BYTES );
    time( &startTime );

    /* Calculate SHS message digest in TEST_BLOCK_SIZE byte blocks */
    shsInit( &shsInfo );
    for( i = TEST_BLOCKS; i > 0; i-- )
	shsUpdate( &shsInfo, data, TEST_BLOCK_SIZE );
    shsFinal( &shsInfo );

    /* Get finish time and time difference */
    time( &endTime );
    printf( "Seconds to process test input: %ld\n", endTime - startTime );
    printf( "Characters processed per second: %ld\n", TEST_BYTES / ( endTime - startTime ) );
    }
--
     pgut1@cs.aukuni.ac.nz || peterg@kcbbs.gen.nz || peter@nacjack.gen.nz
       or sling a bottle in the ocean, it'd be about as reliable as our
			       email currently is.



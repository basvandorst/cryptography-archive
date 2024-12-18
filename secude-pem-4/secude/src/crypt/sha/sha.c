/* NIST proposed Secure Hash Standard.
 
   Written 2 September 1992, Peter C. Gutmann.
   This implementation placed in the public domain.
 
   Modified 1 June 1993, Colin Plumb.
   These modifications placed in the public domain.
 
   Comments to pgut1@cs.aukuni.ac.nz */

#include "sha.h" 
#include <string.h>
 
/* The SHS f()-functions.  The f1 and f3 functions can be optimized to
   save one boolean operation each - thanks to Rich Schroeppel,
   rcs@cs.arizona.edu for discovering this */
 
/*#define f1(x,y,z)     ( (x & y) | (~x & z) )          // Rounds  0-19 */
#define f1(x,y,z)       ( z ^ (x & (y ^ z) ) )          /* Rounds  0-19 */
#define f2(x,y,z)       ( x ^ y ^ z )                   /* Rounds 20-39 */
/*#define f3(x,y,z)     ( (x & y) | (x & z) | (y & z) ) // Rounds 40-59 */
#define f3(x,y,z)       ( (x & y) | (z & (x | y) ) )    /* Rounds 40-59 */
#define f4(x,y,z)       ( x ^ y ^ z )                   /* Rounds 60-79 */
 
/* The SHS Mysterious Constants */
 
#define K1      0x5A827999L     /* Rounds  0-19 */
#define K2      0x6ED9EBA1L     /* Rounds 20-39 */
#define K3      0x8F1BBCDCL     /* Rounds 40-59 */
#define K4      0xCA62C1D6L     /* Rounds 60-79 */
 
/* SHS initial values */
 
#define h0init  0x67452301L
#define h1init  0xEFCDAB89L
#define h2init  0x98BADCFEL
#define h3init  0x10325476L
#define h4init  0xC3D2E1F0L
 
/* Note that it may be necessary to add parentheses to these macros
   if they are to be called with expressions as arguments. */
 
/* 32-bit rotate left - kludged with shifts */
 
#define ROTL(n,X)  ( (X << n) | ( X >> (32-n) ) )
 
/* The initial expanding function */
/* The hash function is defined over an 80-word expanded input array W,
   where the first 16 are copies of the input data, and the remaining 64
   are defined by W[i] = W[i-16] ^ W[i-14] ^ W[i-8] ^ W[i-3].  This
   implementation generates these values on the fly in a circular buffer. */
 
#define expand(W,i) ( W[i & 15] ^= W[i-14 & 15] ^ W[i-8 & 15] ^ W[i-3 & 15] )
 
/* The prototype SHS sub-round */
/* The fundamental sub-round is
   a' = e + ROTL(5,a) + f(b, c, d) + k + data;
   b' = a;
   c' = ROTL(30,b);
   d' = c;
   e' = d;
   ... but this is implemented by unrolling the loop 5 times and renaming
   the variables (e,a,b,c,d) = (a',b',c',d',e') each iteration. */
 
#define subRound(a, b, c, d, e, f, k, data) \
        ( e += ROTL(5,a) + f(b, c, d) + k + data, b = ROTL(30, b) )
 
/* The above code is replicated 20 times for each of the 4 functions,
   using the next 20 values from the W[] array each time. */
 
/* Initialize the SHS values */
 
/***************************************************************
 *
 * Procedure shsInit
 *
 ***************************************************************/
#ifdef __STDC__

void shsInit(
	SHS_INFO	 *shsInfo
)

#else

void shsInit(
	shsInfo
)
SHS_INFO	 *shsInfo;

#endif

{
        /* Set the h-vars to their initial values */
        shsInfo->digest[ 0 ] = h0init;
        shsInfo->digest[ 1 ] = h1init;
        shsInfo->digest[ 2 ] = h2init;
        shsInfo->digest[ 3 ] = h3init;
        shsInfo->digest[ 4 ] = h4init;
 
        /* Initialise bit count */
#ifdef HAVE64
        shsInfo->count = 0;
#else
        shsInfo->countLo = shsInfo->countHi = 0;
#endif
}
 
/* Perform the SHS transformation.  Note that this code, like MD5, seems to
   break some optimizing compilers due to the complexity of the expressions
   and the size of the basic block.  It may be necessary to split it into
   sections, e.g. based on the four subrounds
 
   Note that this corrupts the shsInfo->data area */
 
/***************************************************************
 *
 * Procedure shsTransform
 *
 ***************************************************************/
#ifdef __STDC__

void shsTransform(
	SHS_INFO	 *shsInfo
)

#else

void shsTransform(
	shsInfo
)
SHS_INFO	 *shsInfo;

#endif

{
        register WORD32 A, B, C, D, E;
 
        /* Set up first buffer */
        A = shsInfo->digest[ 0 ];
        B = shsInfo->digest[ 1 ];
        C = shsInfo->digest[ 2 ];
        D = shsInfo->digest[ 3 ];
        E = shsInfo->digest[ 4 ];
 
        /* Heavy mangling, in 4 sub-rounds of 20 interations each. */
        subRound( A, B, C, D, E, f1, K1, shsInfo->data[ 0] );
        subRound( E, A, B, C, D, f1, K1, shsInfo->data[ 1] );
        subRound( D, E, A, B, C, f1, K1, shsInfo->data[ 2] );
        subRound( C, D, E, A, B, f1, K1, shsInfo->data[ 3] );
        subRound( B, C, D, E, A, f1, K1, shsInfo->data[ 4] );
        subRound( A, B, C, D, E, f1, K1, shsInfo->data[ 5] );
        subRound( E, A, B, C, D, f1, K1, shsInfo->data[ 6] );
        subRound( D, E, A, B, C, f1, K1, shsInfo->data[ 7] );
        subRound( C, D, E, A, B, f1, K1, shsInfo->data[ 8] );
        subRound( B, C, D, E, A, f1, K1, shsInfo->data[ 9] );
        subRound( A, B, C, D, E, f1, K1, shsInfo->data[10] );
        subRound( E, A, B, C, D, f1, K1, shsInfo->data[11] );
        subRound( D, E, A, B, C, f1, K1, shsInfo->data[12] );
        subRound( C, D, E, A, B, f1, K1, shsInfo->data[13] );
        subRound( B, C, D, E, A, f1, K1, shsInfo->data[14] );
        subRound( A, B, C, D, E, f1, K1, shsInfo->data[15] );
        subRound( E, A, B, C, D, f1, K1, expand(shsInfo->data, 16) );
        subRound( D, E, A, B, C, f1, K1, expand(shsInfo->data, 17) );
        subRound( C, D, E, A, B, f1, K1, expand(shsInfo->data, 18) );
        subRound( B, C, D, E, A, f1, K1, expand(shsInfo->data, 19) );
 
        subRound( A, B, C, D, E, f2, K2, expand(shsInfo->data, 20) );
        subRound( E, A, B, C, D, f2, K2, expand(shsInfo->data, 21) );
        subRound( D, E, A, B, C, f2, K2, expand(shsInfo->data, 22) );
        subRound( C, D, E, A, B, f2, K2, expand(shsInfo->data, 23) );
        subRound( B, C, D, E, A, f2, K2, expand(shsInfo->data, 24) );
        subRound( A, B, C, D, E, f2, K2, expand(shsInfo->data, 25) );
        subRound( E, A, B, C, D, f2, K2, expand(shsInfo->data, 26) );
        subRound( D, E, A, B, C, f2, K2, expand(shsInfo->data, 27) );
        subRound( C, D, E, A, B, f2, K2, expand(shsInfo->data, 28) );
        subRound( B, C, D, E, A, f2, K2, expand(shsInfo->data, 29) );
        subRound( A, B, C, D, E, f2, K2, expand(shsInfo->data, 30) );
        subRound( E, A, B, C, D, f2, K2, expand(shsInfo->data, 31) );
        subRound( D, E, A, B, C, f2, K2, expand(shsInfo->data, 32) );
        subRound( C, D, E, A, B, f2, K2, expand(shsInfo->data, 33) );
        subRound( B, C, D, E, A, f2, K2, expand(shsInfo->data, 34) );
        subRound( A, B, C, D, E, f2, K2, expand(shsInfo->data, 35) );
        subRound( E, A, B, C, D, f2, K2, expand(shsInfo->data, 36) );
        subRound( D, E, A, B, C, f2, K2, expand(shsInfo->data, 37) );
        subRound( C, D, E, A, B, f2, K2, expand(shsInfo->data, 38) );
        subRound( B, C, D, E, A, f2, K2, expand(shsInfo->data, 39) );
 
        subRound( A, B, C, D, E, f3, K3, expand(shsInfo->data, 40) );
        subRound( E, A, B, C, D, f3, K3, expand(shsInfo->data, 41) );
        subRound( D, E, A, B, C, f3, K3, expand(shsInfo->data, 42) );
        subRound( C, D, E, A, B, f3, K3, expand(shsInfo->data, 43) );
        subRound( B, C, D, E, A, f3, K3, expand(shsInfo->data, 44) );
        subRound( A, B, C, D, E, f3, K3, expand(shsInfo->data, 45) );
        subRound( E, A, B, C, D, f3, K3, expand(shsInfo->data, 46) );
        subRound( D, E, A, B, C, f3, K3, expand(shsInfo->data, 47) );
        subRound( C, D, E, A, B, f3, K3, expand(shsInfo->data, 48) );
        subRound( B, C, D, E, A, f3, K3, expand(shsInfo->data, 49) );
        subRound( A, B, C, D, E, f3, K3, expand(shsInfo->data, 50) );
        subRound( E, A, B, C, D, f3, K3, expand(shsInfo->data, 51) );
        subRound( D, E, A, B, C, f3, K3, expand(shsInfo->data, 52) );
        subRound( C, D, E, A, B, f3, K3, expand(shsInfo->data, 53) );
        subRound( B, C, D, E, A, f3, K3, expand(shsInfo->data, 54) );
        subRound( A, B, C, D, E, f3, K3, expand(shsInfo->data, 55) );
        subRound( E, A, B, C, D, f3, K3, expand(shsInfo->data, 56) );
        subRound( D, E, A, B, C, f3, K3, expand(shsInfo->data, 57) );
        subRound( C, D, E, A, B, f3, K3, expand(shsInfo->data, 58) );
        subRound( B, C, D, E, A, f3, K3, expand(shsInfo->data, 59) );
 
        subRound( A, B, C, D, E, f4, K4, expand(shsInfo->data, 60) );
        subRound( E, A, B, C, D, f4, K4, expand(shsInfo->data, 61) );
        subRound( D, E, A, B, C, f4, K4, expand(shsInfo->data, 62) );
        subRound( C, D, E, A, B, f4, K4, expand(shsInfo->data, 63) );
        subRound( B, C, D, E, A, f4, K4, expand(shsInfo->data, 64) );
        subRound( A, B, C, D, E, f4, K4, expand(shsInfo->data, 65) );
        subRound( E, A, B, C, D, f4, K4, expand(shsInfo->data, 66) );
        subRound( D, E, A, B, C, f4, K4, expand(shsInfo->data, 67) );
        subRound( C, D, E, A, B, f4, K4, expand(shsInfo->data, 68) );
        subRound( B, C, D, E, A, f4, K4, expand(shsInfo->data, 69) );
        subRound( A, B, C, D, E, f4, K4, expand(shsInfo->data, 70) );
        subRound( E, A, B, C, D, f4, K4, expand(shsInfo->data, 71) );
        subRound( D, E, A, B, C, f4, K4, expand(shsInfo->data, 72) );
        subRound( C, D, E, A, B, f4, K4, expand(shsInfo->data, 73) );
        subRound( B, C, D, E, A, f4, K4, expand(shsInfo->data, 74) );
        subRound( A, B, C, D, E, f4, K4, expand(shsInfo->data, 75) );
        subRound( E, A, B, C, D, f4, K4, expand(shsInfo->data, 76) );
        subRound( D, E, A, B, C, f4, K4, expand(shsInfo->data, 77) );
        subRound( C, D, E, A, B, f4, K4, expand(shsInfo->data, 78) );
        subRound( B, C, D, E, A, f4, K4, expand(shsInfo->data, 79) );
 
        /* Build message digest */
        shsInfo->digest[ 0 ] += A;
        shsInfo->digest[ 1 ] += B;
        shsInfo->digest[ 2 ] += C;
        shsInfo->digest[ 3 ] += D;
        shsInfo->digest[ 4 ] += E;
}
 
/* When run on a little-endian CPU we need to perform byte reversal on an
   array of longwords.  It is possible to make the code endianness-
   independant by fiddling around with data at the byte level, but this
   makes for very slow code, so we rely on the user to sort out endianness
   at compile time */
 
#ifdef LITTLE_ENDIAN
 
/***************************************************************
 *
 * Procedure byteReverse
 *
 ***************************************************************/
#ifdef __STDC__

static void byteReverse(
	WORD32		 *buffer,
	unsigned	  byteCount
)

#else

static void byteReverse(
	buffer,
	byteCount
)
WORD32		 *buffer;
unsigned	  byteCount;

#endif

{
        WORD32 value;
 
        byteCount /= sizeof(WORD32);
        while ( byteCount-- ) {
                value = *buffer;
                value = ( value & 0xFF00FF00L ) >> 8  | 
                        ( value & 0x00FF00FFL ) << 8;
                *buffer++ = value << 16  |  value >> 16 ;
        }
}
 
#else /* !LITTLE_ENDIAN */
 
#define byteReverse(buf, count)	/* nothing */
 
#endif /* !LITTLE_ENDIAN */
 
/* Update SHS for a block of data. */
 
/***************************************************************
 *
 * Procedure shsUpdate
 *
 ***************************************************************/
#ifdef __STDC__

void shsUpdate(
	SHS_INFO	 *shsInfo,
	BYTE		 *buffer,
	unsigned	  count
)

#else

void shsUpdate(
	shsInfo,
	buffer,
	count
)
SHS_INFO	 *shsInfo;
BYTE		 *buffer;
unsigned	  count;

#endif

{
        WORD32 t;
 
        /* Update bitcount */
 
#ifdef HAVE64
        t = ( (WORD32)shsInfo->count >> 3) & 0x3f;
        shsInfo->count += (WORD64)count << 3;
#else
        t = shsInfo->countLo;
        if ( ( shsInfo->countLo = t + ( (WORD32)count << 3) ) < t )
                shsInfo->countHi++;     /* Carry from low to high */
        shsInfo->countHi += count >> 29;
 
        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */
#endif
 
        /* Handle any leading odd-sized chunks */
 
        if ( t ) {
                BYTE *p = (BYTE *)shsInfo->data + t;
 
                t = 64-t;
                if (count < t) {
                        memcpy( (char *)p, (char *)buffer, count );
                        return;
 
               }
/* OLD
                memcpy( (char *)p, (char *)buffer, t );
*/
                memcpy( (char *)p, (char *)buffer, (unsigned) t );
                byteReverse( shsInfo->data, SHS_BLOCKSIZE );
                shsTransform( shsInfo );
                buffer += t;
                count -= t;
        }
 
        /* Process data in SHS_BLOCKSIZE chunks */
 
        while( count >= SHS_BLOCKSIZE ) {
                memcpy( (char *)shsInfo->data, (char *)buffer, SHS_BLOCKSIZE );
                byteReverse( shsInfo->data, SHS_BLOCKSIZE );
                shsTransform( shsInfo );
                buffer += SHS_BLOCKSIZE;
                count -= SHS_BLOCKSIZE;
        }
 
        /* Handle any remaining bytes of data. */
 
        memcpy( (char *)shsInfo->data, (char *)buffer, count );
}
 
/* Final wrapup - pad to 64-byte boundary with the bit pattern 
   1 0* (64-bit count of bits processed, MSB-first) */
 
/***************************************************************
 *
 * Procedure shsFinal
 *
 ***************************************************************/
#ifdef __STDC__

void shsFinal(
	SHS_INFO	 *shsInfo
)

#else

void shsFinal(
	shsInfo
)
SHS_INFO	 *shsInfo;

#endif

{
        int count;
        BYTE *p;
 
        /* Compute number of bytes mod 64 */
#ifdef HAVE64
        count = (int)shsInfo->count;
#else
        count = (int)shsInfo->countLo;
#endif
        count = ( count >> 3 ) & 0x3F;
 
        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = (BYTE *)shsInfo->data + count;
        *p++ = 0x80;
 
        /* Bytes of padding needed to make 64 bytes */
        count = SHS_BLOCKSIZE - 1 - count;
 
        /* Pad out to 56 mod 64 */
        if( count < 8 ) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                memset( (char *)p, 0, count );
                byteReverse( shsInfo->data, SHS_BLOCKSIZE );
                shsTransform( shsInfo );
 
                /* Now fill the next block with 56 bytes */
                memset( (char *)shsInfo->data, 0, SHS_BLOCKSIZE - 8 );
        } else {
                /* Pad block to 56 bytes */
                memset( (char *)p, 0, count - 8 );
        }
        byteReverse( shsInfo->data, SHS_BLOCKSIZE-8 );
 
        /* Append length in bits and transform */
#if HAVE64
        shsInfo->data[ 14 ] = (WORD32)( shsInfo->count >> 32 );
        shsInfo->data[ 15 ] = (WORD32)shsInfo->count;
#else
        shsInfo->data[ 14 ] = shsInfo->countHi;
        shsInfo->data[ 15 ] = shsInfo->countLo;
#endif
 
        shsTransform( shsInfo );
}

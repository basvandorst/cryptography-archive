/*
 * pgpSHADouble.c - Double width version of the NIST SHA-1 hash
 *
 * Used for generalizations of DSA signatures up to 4K bits
 *
 * We use a variation of the modified Benes (double butterfly) construction
 * described in "Foiling Birthday Attacks in Length-Doubling Transformations",
 * William Aiello and Ramarathnam Venkatesan, Eurocrypt 96.
 *
 * The idea is that we perform the following hashes:
 *
 *              Half            Half
 *              Input		   Input
 *                |  \        /  |
 *                |  SHA1  SHA2  |
 *              SHA0    \   /   SHA3
 *                |       X      |
 *                |     /   \    |
 *                |   /       \  |
 *               XOR   	   	    XOR
 *                |  \        /  |
 *                |  SHA5   SHA6 |
 *              SHA4    \   /   SHA7
 *                |       X      |
 *                |     /   \    |
 *                |   /       \  |
 *               XOR   	   	    XOR
 *                |              |
 *              Left Half     Right Half
 *
 *
 * The numbers on the hashes above represent differently keyed SHA-1 hashes.
 * We key each one differently by prepending different numbers of bytes of
 * zeros.
 *
 * $Id: pgpSHADouble.c,v 1.5 1997/10/14 01:48:24 heller Exp $
 */

#include "pgpConfig.h"

#include <string.h>

#include "pgpHash.h"
#include "pgpSHADouble.h"
#include "pgpSHA.h"
#include "pgpUsuals.h"
#include "pgpMem.h"
#include "pgpDebug.h"


/*
 * Size of our odd-even buffers.  Doesn't really matter how big it is.
 * Choose 64 to have a match with the SHA input buffer size, but that is
 * not important.
 */
#define BUFSIZE		64

/* Includes four instances of the SHA context for the top butterfly */
typedef struct SHADoubleContext {
	SHAContext			sha0, sha1, sha2, sha3;
	PGPBoolean			odd;
	PGPByte				result[SHA_HASHBYTES*2];
	PGPHashVTBL	const	*shaVTBL;
	DEBUG_STRUCT_CONSTRUCTOR( SHADoubleContext )
} SHADoubleContext;


/* Initialize the SHA values */

static void
shaDoubleInit(void *priv)
{
	SHADoubleContext *ctx = (SHADoubleContext *)priv;
	PGPByte zeros[3];

	ctx->shaVTBL = pgpHashByNumber( kPGPHashAlgorithm_SHA );
	ctx->shaVTBL->init (&ctx->sha0);
	ctx->shaVTBL->init (&ctx->sha1);
	ctx->shaVTBL->init (&ctx->sha2);
	ctx->shaVTBL->init (&ctx->sha3);

	/* Preload hashes with n bytes of 0's to key them differently */
	pgpClearMemory( &zeros, sizeof(zeros) );
	ctx->shaVTBL->update( &ctx->sha1, zeros, 1 );
	ctx->shaVTBL->update( &ctx->sha2, zeros, 2 );
	ctx->shaVTBL->update( &ctx->sha3, zeros, 3 );
	pgpAssert( sizeof(zeros) >= 3 );

	ctx->odd = FALSE;		/* Byte 0 will count as even */
}


/* Update SHA for a block of data. */

static void
shaDoubleUpdate(void *priv, void const *bufParam, PGPSize len)
{
	SHADoubleContext *ctx = (SHADoubleContext *)priv;
	PGPByte evenbuf[BUFSIZE], *evenp;		/* Even bytes of buf */
	PGPByte oddbuf[BUFSIZE], *oddp;			/* Odd bytes of buf */
	const PGPByte *buf = (const PGPByte *) bufParam;
	
	/* Divide the bytes of the input into two buffers, one to hold the
	 * even bytes and one to hold the odd bytes.  (The first byte is
	 * counted as even.)  We pass the even bytes through SHA's 0 and 1,
	 * and the odd bytes through 2 and 3.
	 */
	oddp = oddbuf;
	evenp = evenbuf;

#if 0
	/* Naive algorithm for splitting even/odd bytes */

	newodd = ctx->odd ^ (len & 1);
	if( ctx->odd && len != 0 ) {
		*oddp++ = *buf++;
		len--;
	}
	ctx->odd = newodd;

	while ( len != 0 ) {

		while( oddp < oddbuf + BUFSIZE ) {
			if (len-- == 0)
				break;
			*evenp++ = *buf++;
			if (len-- == 0)
				break;
			*oddp++ = *buf++;
		}
		ctx->shaVTBL->update( &ctx->sha0, evenbuf, evenp-evenbuf );
		ctx->shaVTBL->update( &ctx->sha1, evenbuf, evenp-evenbuf );
		ctx->shaVTBL->update( &ctx->sha2, oddbuf, oddp-oddbuf );
		ctx->shaVTBL->update( &ctx->sha3, oddbuf, oddp-oddbuf );
	
	}

#else
	/* If we were left at an odd point last time, handle first byte */
	if( ctx->odd && len != 0) {
		*oddp++ = *buf++;
		/* We intentionally don't decrement len here, handled below */
	}
	ctx->odd ^= (len & 1);

	/* Deal with blocks of data which fill our odd/even buffers */
	while( len >= 2*BUFSIZE ) {
		while (oddp < oddbuf + BUFSIZE ) {
			*evenp++ = *buf++;
			*oddp++ = *buf++;
		}
		pgpAssert( oddp == oddbuf + BUFSIZE );
		pgpAssert( evenp == evenbuf + BUFSIZE ||
				   evenp == evenbuf + BUFSIZE - 1 );
		ctx->shaVTBL->update( &ctx->sha0, evenbuf, evenp-evenbuf );
		ctx->shaVTBL->update( &ctx->sha1, evenbuf, evenp-evenbuf );
		ctx->shaVTBL->update( &ctx->sha2, oddbuf, BUFSIZE );
		ctx->shaVTBL->update( &ctx->sha3, oddbuf, BUFSIZE );
		len -= BUFSIZE + (evenp-evenbuf);
		oddp = oddbuf;
		evenp = evenbuf;
	}
		
	/* Deal with leftover bytes of input which are less than our block */
	if (oddp != oddbuf)
		len -= 1;

	while (oddp < oddbuf + BUFSIZE && len >= 2 ) {
		*evenp++ = *buf++;
		*oddp++ = *buf++;
		len -= 2;
	}
	if (evenp < evenbuf + BUFSIZE && len >= 1 ) {
		*evenp++ = *buf++;
		len -= 1;
	}

	pgpAssert( len == 0 );

	ctx->shaVTBL->update( &ctx->sha0, evenbuf, evenp-evenbuf );
	ctx->shaVTBL->update( &ctx->sha1, evenbuf, evenp-evenbuf );
	ctx->shaVTBL->update( &ctx->sha2, oddbuf, oddp-oddbuf );
	ctx->shaVTBL->update( &ctx->sha3, oddbuf, oddp-oddbuf );
#endif
}

/*
 * Final wrapup
 */
static void const *
shaDoubleFinal(void *priv)
{
	SHADoubleContext *ctx = (SHADoubleContext *)priv;
	PGPByte			*sha0Final, *sha1Final;
	PGPByte const	*sha2Final, *sha3Final;
	PGPByte 		*sha4Final, *sha5Final;
	PGPByte const	*sha6Final, *sha7Final;
	SHAContext		sha4, sha5, sha6, sha7;
	PGPByte			zeros[7];
	int				i;

	/* Pick up output from the four hash functions */
	sha0Final = (PGPByte *)ctx->shaVTBL->final( &ctx->sha0 );
	sha1Final = (PGPByte *)ctx->shaVTBL->final( &ctx->sha1 );
	sha2Final = (PGPByte *)ctx->shaVTBL->final( &ctx->sha2 );
	sha3Final = (PGPByte *)ctx->shaVTBL->final( &ctx->sha3 );

	/* XOR sha2 into sha0, and sha3 into sha1 */
	for (i=0; i<SHA_HASHBYTES; ++i) {
		sha0Final[i] ^= sha2Final[i];
		sha1Final[i] ^= sha3Final[i];
	}

	/*
	 * Do a butterfly mix on the output from this pass.  The amount of data
	 * to be hashed is only 40 bytes, passed 20 bytes at a time through four
	 * hashes, so the costs of doing this are small.
	 */
	ctx->shaVTBL->init (&sha4);
	ctx->shaVTBL->init (&sha5);
	ctx->shaVTBL->init (&sha6);
	ctx->shaVTBL->init (&sha7);

	/*
	 * Butterfly consists of four keyed hashes, which we key by prepending
	 * different numbers of bytes of zeros, as we did above.
	 */
	pgpClearMemory( &zeros, sizeof(zeros) );
	ctx->shaVTBL->update( &sha4, zeros, 4 );
	ctx->shaVTBL->update( &sha5, zeros, 5 );
	ctx->shaVTBL->update( &sha6, zeros, 6 );
	ctx->shaVTBL->update( &sha7, zeros, 7 );
	pgpAssert( sizeof(zeros) >= 7 );

	/* Perform hashes for second butterfly */
	ctx->shaVTBL->update( &sha4, sha0Final, SHA_HASHBYTES );
	ctx->shaVTBL->update( &sha5, sha0Final, SHA_HASHBYTES );
	ctx->shaVTBL->update( &sha6, sha1Final, SHA_HASHBYTES );
	ctx->shaVTBL->update( &sha7, sha1Final, SHA_HASHBYTES );
	sha4Final = (PGPByte *)ctx->shaVTBL->final( &sha4 );
	sha5Final = (PGPByte *)ctx->shaVTBL->final( &sha5 );
	sha6Final = (PGPByte *)ctx->shaVTBL->final( &sha6 );
	sha7Final = (PGPByte *)ctx->shaVTBL->final( &sha7 );

	/* Left half of output is xor of sha4 and sha6 */
	for (i=0; i<SHA_HASHBYTES; ++i)
		sha4Final[i] ^= sha6Final[i];

	/* Right half of output is xor of sha5 and sha7 */
	for (i=0; i<SHA_HASHBYTES; ++i)
		sha5Final[i] ^= sha7Final[i];

	/* Assemble these two halves to produce the final output */
	pgpCopyMemory(sha4Final, ctx->result, SHA_HASHBYTES);
	pgpCopyMemory(sha5Final, ctx->result + SHA_HASHBYTES, SHA_HASHBYTES);
	return (PGPByte const *)ctx->result;
}


PGPHashVTBL const HashSHADouble = {
	"SHA1x", kPGPHashAlgorithm_SHADouble,
	SHADERprefix, sizeof(SHADERprefix),
	SHA_HASHBYTES*2,
	sizeof(SHADoubleContext),
	sizeof(struct{char _a; SHAContext _b;}) -
		sizeof(SHADoubleContext),
	shaDoubleInit, shaDoubleUpdate, shaDoubleFinal
};


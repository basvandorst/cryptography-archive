/*____________________________________________________________________________
	pgpShamir.c
	
	Shamir threshold secret sharing (polynomial based)

	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpShamir.c,v 1.5 1999/03/10 02:55:49 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpKeys.h"
#include "pgpRandomX9_17.h"

/* Output array consists of a share header followed by the share body, for
 * each share, all concatenated together.
 */
struct PGPShareHeader
{
	PGPByte			version;		/* Version/algorithm of share */
	PGPByte			xCoordinate;	/* X coordinate of share */
	PGPByte			threshold;		/* Number of shares needed to combine */
	PGPByte			lagrange;		/* Temp value used during split/join */
};

/* Version for this algorithm */
#define kPGPShare_Version1	0x21

/* X coordinate of secret value */
#define X0		0


/* Pointer to nth share body in output */
#define BODY(array,bodysize,n) ((PGPByte *)(array) + \
		(n)*((bodysize)+kPGPShareHeaderSize) + kPGPShareHeaderSize)

/* Pointer to nth share header in output */
#define HEADER(array,bodysize,n) ((struct PGPShareHeader *) \
		((PGPByte *)(array) + (n)*((bodysize)+kPGPShareHeaderSize)))


/*
 * The magic of Lagrange polynomial interpolation...
 *
 * Given (x1,y1), (x2,y2)...,(xn,yn) and x0, you want to find y0.
 * This is hardly unique unless you specify that you want a polynomial in x
 * fitted to the given points and evaluated at x0.  If the polynomial is
 * constrained to be of degree <= n-1 (in the unknown x), then it is unique.
 *
 * We use this to make shares, by letting y1 be the secret,
 * choosing y2, ..., yn arbitrarily, and using that to find the
 * extra shares, and to reassemble shares, by trying to recover y1.
 *
 * The easy way to find this polynomial is to use a set of degreen n-1 basis
 * polynomials bi(x) which are 1 at a single xi and 0 at all the other
 * xj, j != i.  (Needless to say, this is only possible if xi != xj.)
 * Then you can just evaluate y = b1(x)*y1 + ... + bn(x)*yn.
 *
 * The zeroness is assured by including the factors (x-x1), ... (x-xn)
 * in the numerator of the polynomial bi(x), omitting only (x-xi).
 * This makes a polynomial of degree n-1 in x, as desired.  Ensuring
 * that bi(xi) == 1 requires dividing by the appropriate constant,
 * (xi-x1)*...*(xi-xn), the numerator evaluated at xi, again omitting the
 * (xi-xi) term.
 *
 * In more concrete terms,
 *
 *                   (x0-x2)*(x0-x3)*(x0-x4)*...*(x0-xn)
 * y0 = y1 * -------------------------------------------
 *                   (x1-x2)*(x1-x3)*(x1-x4)*...*(x1-xn)
 *
 *           (x0-x1)*        (x0-x3)*(x0-x4)*...*(x0-xn)
 *    + y2 * -------------------------------------------
 *           (x2-x0)*        (x2-x3)*(x2-x4)*...*(x2-xn)
 * 
 *    + ...
 *
 * It turns out to be easier in practice to pre-compute (x0-x1)*...*(x0-xn)
 * and then add the appropriate (x0-xi) term to the denominator to cancel
 * the unneeded numerator term.
 *
 * Oh, by the way, this is done over the finite field F_256 (encoded
 * in a PGPByte type).  Multiplication is done through log and antilog
 * tables, f_log[] and f_exp[].  The f_exp[] table is double-sized
 * so that the sum of two logs can be looked up in it without range
 * reduction, although larger accumulations require reduction modulo
 * the order of the multiplicative group, namely 256-1 = 255.
 *
 * Most compilers can optimize division and remainder by a power
 * of 2, but this one-off is usually handled by a general division,
 * which is slow on most machines.  There is a much faster version,
 * using the fact that x*FIELD_SIZE == x (mod FIELD_SIZE-1).  Thus,
 * x == (x%FIELD_SIZE) + (x/FIELD_SIZE) (mod FIELD_SIZE-1), using
 * truncating integer division.  This obviously reduces the range of
 * the output x.  It can't reduce it below 0..FIELD_SIZE-1, but we
 * the antilog table is already big enough for that.
 *
 * The smallest x which is mapped to FIELD_SIZE or larger under this
 * reduction is 2*FIELD_SIZE-1.  The smallest x which is reduced to
 * *that* or larger is FIELD_SIZE^2 - 1.  So adding up FIELD_SIZE+1
 * entries with all values of FIELD_SIZE-1 wil require a third
 * iteration to achieve complete reduction, but less than that (all
 * that we ever do here) requires only two.
 */


#define FIELD_SIZE 256
#define FIELD_POLY 0x169

/* Just to make the application clear... */
#define f_add(x,y) ((x)^(y))
#define f_sub(x,y) f_add(x,y)

/*
 * The possible primitive polynomials of degree 8 are
 * 0x11d, 0x12b, 0x12d, 0x14d, 0x15f, 0x163, 0x165, 0x169,
 * 0x171, 0x187, 0x18d, 0x1a9, 0x1c3, 0x1cf, 0x1e7, 0x1f5
 * Other polynomials (irreducible but not primitive) are possible
 * if you use something other than x as the generator.
 */

#if PGP_STATIC_SHAMIR_ARRAYS

/* Code to dynamically construct these arrays is below as an alternative */

static const PGPByte f_exp[2*FIELD_SIZE] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x69, 0xd2, 0xcd, 0xf3, 0x8f, 0x77, 0xee, 0xb5,
    0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0xe9,
    0xbb, 0x1f, 0x3e, 0x7c, 0xf8, 0x99, 0x5b, 0xb6,
    0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0, 0x29, 0x52,
    0xa4, 0x21, 0x42, 0x84, 0x61, 0xc2, 0xed, 0xb3,
    0x0f, 0x1e, 0x3c, 0x78, 0xf0, 0x89, 0x7b, 0xf6,
    0x85, 0x63, 0xc6, 0xe5, 0xa3, 0x2f, 0x5e, 0xbc,
    0x11, 0x22, 0x44, 0x88, 0x79, 0xf2, 0x8d, 0x73,
    0xe6, 0xa5, 0x23, 0x46, 0x8c, 0x71, 0xe2, 0xad,
    0x33, 0x66, 0xcc, 0xf1, 0x8b, 0x7f, 0xfe, 0x95,
    0x43, 0x86, 0x65, 0xca, 0xfd, 0x93, 0x4f, 0x9e,
    0x55, 0xaa, 0x3d, 0x7a, 0xf4, 0x81, 0x6b, 0xd6,
    0xc5, 0xe3, 0xaf, 0x37, 0x6e, 0xdc, 0xd1, 0xcb,
    0xff, 0x97, 0x47, 0x8e, 0x75, 0xea, 0xbd, 0x13,
    0x26, 0x4c, 0x98, 0x59, 0xb2, 0x0d, 0x1a, 0x34,
    0x68, 0xd0, 0xc9, 0xfb, 0x9f, 0x57, 0xae, 0x35,
    0x6a, 0xd4, 0xc1, 0xeb, 0xbf, 0x17, 0x2e, 0x5c,
    0xb8, 0x19, 0x32, 0x64, 0xc8, 0xf9, 0x9b, 0x5f,
    0xbe, 0x15, 0x2a, 0x54, 0xa8, 0x39, 0x72, 0xe4,
    0xa1, 0x2b, 0x56, 0xac, 0x31, 0x62, 0xc4, 0xe1,
    0xab, 0x3f, 0x7e, 0xfc, 0x91, 0x4b, 0x96, 0x45,
    0x8a, 0x7d, 0xfa, 0x9d, 0x53, 0xa6, 0x25, 0x4a,
    0x94, 0x41, 0x82, 0x6d, 0xda, 0xdd, 0xd3, 0xcf,
    0xf7, 0x87, 0x67, 0xce, 0xf5, 0x83, 0x6f, 0xde,
    0xd5, 0xc3, 0xef, 0xb7, 0x07, 0x0e, 0x1c, 0x38,
    0x70, 0xe0, 0xa9, 0x3b, 0x76, 0xec, 0xb1, 0x0b,
    0x16, 0x2c, 0x58, 0xb0, 0x09, 0x12, 0x24, 0x48,
    0x90, 0x49, 0x92, 0x4d, 0x9a, 0x5d, 0xba, 0x1d,
    0x3a, 0x74, 0xe8, 0xb9, 0x1b, 0x36, 0x6c, 0xd8,
    0xd9, 0xdb, 0xdf, 0xd7, 0xc7, 0xe7, 0xa7, 0x27,
    0x4e, 0x9c, 0x51, 0xa2, 0x2d, 0x5a, 0xb4, 0x01,
    0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x69,
    0xd2, 0xcd, 0xf3, 0x8f, 0x77, 0xee, 0xb5, 0x03,
    0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0xe9, 0xbb,
    0x1f, 0x3e, 0x7c, 0xf8, 0x99, 0x5b, 0xb6, 0x05,
    0x0a, 0x14, 0x28, 0x50, 0xa0, 0x29, 0x52, 0xa4,
    0x21, 0x42, 0x84, 0x61, 0xc2, 0xed, 0xb3, 0x0f,
    0x1e, 0x3c, 0x78, 0xf0, 0x89, 0x7b, 0xf6, 0x85,
    0x63, 0xc6, 0xe5, 0xa3, 0x2f, 0x5e, 0xbc, 0x11,
    0x22, 0x44, 0x88, 0x79, 0xf2, 0x8d, 0x73, 0xe6,
    0xa5, 0x23, 0x46, 0x8c, 0x71, 0xe2, 0xad, 0x33,
    0x66, 0xcc, 0xf1, 0x8b, 0x7f, 0xfe, 0x95, 0x43,
    0x86, 0x65, 0xca, 0xfd, 0x93, 0x4f, 0x9e, 0x55,
    0xaa, 0x3d, 0x7a, 0xf4, 0x81, 0x6b, 0xd6, 0xc5,
    0xe3, 0xaf, 0x37, 0x6e, 0xdc, 0xd1, 0xcb, 0xff,
    0x97, 0x47, 0x8e, 0x75, 0xea, 0xbd, 0x13, 0x26,
    0x4c, 0x98, 0x59, 0xb2, 0x0d, 0x1a, 0x34, 0x68,
    0xd0, 0xc9, 0xfb, 0x9f, 0x57, 0xae, 0x35, 0x6a,
    0xd4, 0xc1, 0xeb, 0xbf, 0x17, 0x2e, 0x5c, 0xb8,
    0x19, 0x32, 0x64, 0xc8, 0xf9, 0x9b, 0x5f, 0xbe,
    0x15, 0x2a, 0x54, 0xa8, 0x39, 0x72, 0xe4, 0xa1,
    0x2b, 0x56, 0xac, 0x31, 0x62, 0xc4, 0xe1, 0xab,
    0x3f, 0x7e, 0xfc, 0x91, 0x4b, 0x96, 0x45, 0x8a,
    0x7d, 0xfa, 0x9d, 0x53, 0xa6, 0x25, 0x4a, 0x94,
    0x41, 0x82, 0x6d, 0xda, 0xdd, 0xd3, 0xcf, 0xf7,
    0x87, 0x67, 0xce, 0xf5, 0x83, 0x6f, 0xde, 0xd5,
    0xc3, 0xef, 0xb7, 0x07, 0x0e, 0x1c, 0x38, 0x70,
    0xe0, 0xa9, 0x3b, 0x76, 0xec, 0xb1, 0x0b, 0x16,
    0x2c, 0x58, 0xb0, 0x09, 0x12, 0x24, 0x48, 0x90,
    0x49, 0x92, 0x4d, 0x9a, 0x5d, 0xba, 0x1d, 0x3a,
    0x74, 0xe8, 0xb9, 0x1b, 0x36, 0x6c, 0xd8, 0xd9,
    0xdb, 0xdf, 0xd7, 0xc7, 0xe7, 0xa7, 0x27, 0x4e,
    0x9c, 0x51, 0xa2, 0x2d, 0x5a, 0xb4, 0x01, 0x02
};

static const PGPByte f_log[FIELD_SIZE] =
{
    0xff, 0x00, 0x01, 0x10, 0x02, 0x20, 0x11, 0xcc,
    0x03, 0xdc, 0x21, 0xd7, 0x12, 0x7d, 0xcd, 0x30,
    0x04, 0x40, 0xdd, 0x77, 0x22, 0x99, 0xd8, 0x8d,
    0x13, 0x91, 0x7e, 0xec, 0xce, 0xe7, 0x31, 0x19,
    0x05, 0x29, 0x41, 0x4a, 0xde, 0xb6, 0x78, 0xf7,
    0x23, 0x26, 0x9a, 0xa1, 0xd9, 0xfc, 0x8e, 0x3d,
    0x14, 0xa4, 0x92, 0x50, 0x7f, 0x87, 0xed, 0x6b,
    0xcf, 0x9d, 0xe8, 0xd3, 0x32, 0x62, 0x1a, 0xa9,
    0x06, 0xb9, 0x2a, 0x58, 0x42, 0xaf, 0x4b, 0x72,
    0xdf, 0xe1, 0xb7, 0xad, 0x79, 0xe3, 0xf8, 0x5e,
    0x24, 0xfa, 0x27, 0xb4, 0x9b, 0x60, 0xa2, 0x85,
    0xda, 0x7b, 0xfd, 0x1e, 0x8f, 0xe5, 0x3e, 0x97,
    0x15, 0x2c, 0xa5, 0x39, 0x93, 0x5a, 0x51, 0xc2,
    0x80, 0x08, 0x88, 0x66, 0xee, 0xbb, 0x6c, 0xc6,
    0xd0, 0x4d, 0x9e, 0x47, 0xe9, 0x74, 0xd4, 0x0d,
    0x33, 0x44, 0x63, 0x36, 0x1b, 0xb1, 0xaa, 0x55,
    0x07, 0x65, 0xba, 0xc5, 0x2b, 0x38, 0x59, 0xc1,
    0x43, 0x35, 0xb0, 0x54, 0x4c, 0x46, 0x73, 0x0c,
    0xe0, 0xac, 0xe2, 0x5d, 0xb8, 0x57, 0xae, 0x71,
    0x7a, 0x1d, 0xe4, 0x96, 0xf9, 0xb3, 0x5f, 0x84,
    0x25, 0xa0, 0xfb, 0x3c, 0x28, 0x49, 0xb5, 0xf6,
    0x9c, 0xd2, 0x61, 0xa8, 0xa3, 0x4f, 0x86, 0x6a,
    0xdb, 0xd6, 0x7c, 0x2f, 0xfe, 0x0f, 0x1f, 0xcb,
    0x90, 0xeb, 0xe6, 0x18, 0x3f, 0x76, 0x98, 0x8c,
    0x16, 0x8a, 0x2d, 0xc9, 0xa6, 0x68, 0x3a, 0xf4,
    0x94, 0x82, 0x5b, 0x6f, 0x52, 0x0a, 0xc3, 0xbf,
    0x81, 0x6e, 0x09, 0xbe, 0x89, 0xc8, 0x67, 0xf3,
    0xef, 0xf0, 0xbc, 0xf1, 0x6d, 0xbd, 0xc7, 0xf2,
    0xd1, 0xa7, 0x4e, 0x69, 0x9f, 0x3b, 0x48, 0xf5,
    0xea, 0x17, 0x75, 0x8b, 0xd5, 0x2e, 0x0e, 0xca,
    0x34, 0x53, 0x45, 0x0b, 0x64, 0xc4, 0x37, 0xc0,
    0x1c, 0x95, 0xb2, 0x83, 0xab, 0x5c, 0x56, 0x70
};

#else	/* PGP_STATIC_SHAMIR_ARRAYS */

static PGPByte f_exp[2*FIELD_SIZE];
static PGPByte f_log[FIELD_SIZE];


/* Code to dynamically struct f_log and f_exp arrays */
/*
 * Initialize the f_exp and f_log arrays (if necessary).
 * Safe (and fast) to call redundantly, so any convenient time will do.
 */
	static void
s_FSetup(void)
{
	unsigned i, x;

	if (!f_log[0]) {
		x = 1;
		for (i = 0; i < FIELD_SIZE-1; i++) {
			f_exp[i] = x;
			f_exp[i+FIELD_SIZE-1] = x;
			f_log[x] = i;
			x <<= 1;
			if (x & FIELD_SIZE)
				x ^= FIELD_POLY;
		}
		/* x should be 1 here */
		f_exp[2*FIELD_SIZE-2] = f_exp[0];
		f_exp[2*FIELD_SIZE-1] = f_exp[1];
		f_log[0] = i;	/* Bogus value, FIELD_SIZE-1 */
	}
}

#endif	/* PGP_STATIC_SHAMIR_ARRAYS */

/*
 * This is the core of secret sharing.  This computes the coefficients
 * used in Lagrange polynomial interpolation, returning the
 * vector of logarithms of b1(xtarget), b2(xtarget), ..., bn(xtarget).
 * Takes values from the "xCoordinate" header element, inserts the
 * results in the "lagrange" header element.
 * The interpolation values come from the headers of the "shares" array,
 * plus one additional value, xInput, which is the value we are going
 * to interpolate to.
 * 
 * Returns kPGPError_OK on success, error if not all x[i] are unique.
 */
	static PGPError
sInterp(PGPByte *shares, PGPSize bodySize, PGPByte xInput, PGPUInt32 nShares)
{
	PGPUInt32		i, j;
	PGPByte			xi, xj;
	PGPUInt32		numer, denom;

#if !PGP_STATIC_SHAMIR_ARRAYS
	s_FSetup();
#endif /* !PGP_STATIC_SHAMIR_ARRAYS */

	/* First, accumulate the numerator, Prod(xInput-x[i],i=0..n) */
	numer = 0;
	for (i = 0; i < nShares; i++)
	{
		xi = HEADER(shares,bodySize,i)->xCoordinate;
		numer += f_log[ f_sub(xi, xInput) ];
	}
	/* Preliminary partial reduction */
	numer = (numer%FIELD_SIZE) + (numer/FIELD_SIZE);

	/* Then, for each coefficient, compute the corresponding denominator */
	for (i = 0; i < nShares; i++) {
		xi = HEADER(shares,bodySize,i)->xCoordinate;
		denom = 0;
		for (j = 0; j < nShares; j++) {
			xj = (i == j) ? xInput : HEADER(shares,bodySize,j)->xCoordinate;
			if (xi == xj)
				return -1;
			denom += f_log[f_sub(xi,xj)];
		}
		denom = (denom%FIELD_SIZE)+(denom/FIELD_SIZE);
		/* 0 <= denom < 2*FIELD_SIZE-1. */

		/* Now find numer/denom.  In log form, that's a subtract. */
		denom = numer + 2*FIELD_SIZE-2 - denom;
		denom = (denom%FIELD_SIZE)+(denom/FIELD_SIZE);
		denom = (denom%FIELD_SIZE)+(denom/FIELD_SIZE);

		HEADER(shares,bodySize,i)->lagrange = (PGPByte)denom;
	}
	return kPGPError_NoErr;	/* Success */
}



/*
 * This actually does the interpolation, using the coefficients
 * computed by sInterp().  Uses shares at offset byteNumber within the
 * bodies.
 * The Lagrange values come from the headers of the "shares" array.
 */
	static PGPByte
sDoInterp(PGPByte *shares, PGPSize bodySize, PGPUInt32 nShares,
	PGPUInt32 byteNumber)
{
	PGPByte x, y;
	PGPByte lagrange;
	PGPUInt32 i;

	x = 0;
	for( i=0; i < nShares; ++i )
	{
		y = BODY(shares, bodySize, i)[byteNumber];
		if (y != 0)
		{
			lagrange = HEADER(shares, bodySize, i)->lagrange;
			y = f_exp[lagrange + f_log[y]];
		}
		x = f_add(x,y);
	}

	return x;
}



	PGPError
PGPSecretShareData(
	PGPContextRef		context,
	void const *		input,
	PGPSize				inputBytes,
	PGPUInt32			threshold,
	PGPUInt32			nShares,
	void *				output
	)
{
	PGPUInt32			i, j;
	PGPByte				xupdate;
	PGPRandomContext	*rng;			/* Random state */

	rng = pgpContextGetX9_17RandomContext( context );

	/* Set X coordinate randomly for each share */
	for( i=0; i<nShares; ++i )
	{
		PGPBoolean xok = FALSE;
		/* Pick a unique, random x coordinate != X0 */
		while( !xok )
		{
			pgpRandomGetBytes( rng,
						   &(HEADER(output,inputBytes,i)->xCoordinate), 1 );
			if( HEADER(output,inputBytes,i)->xCoordinate != X0 )
			{
				for( j=0; j<i; ++j )
				{
					if( HEADER(output,inputBytes,i)->xCoordinate ==
						HEADER(output,inputBytes,j)->xCoordinate )
						break;
				}
				if( j == i )
				{
					xok = TRUE;
				}
			}
		}
		/* Set other header values as well */
		HEADER(output,inputBytes,i)->version = kPGPShare_Version1;
		HEADER(output,inputBytes,i)->threshold = threshold;
		HEADER(output,inputBytes,i)->lagrange = 0;
	}

	/* Initialize thresh-1 bodies to random numbers */
	for( i=0; i<threshold-1; ++i )
	{
		pgpRandomGetBytes( rng, BODY(output,inputBytes,i), inputBytes );
	}

	/* Copy input to the first share body past the random ones */
	pgpCopyMemory( input, BODY(output,inputBytes,threshold-1), inputBytes );
		
	/* Put X0 into xCoordinate for that header */
	/* xupdate holds the X value for the share we will be updating */
	xupdate = HEADER(output,inputBytes,threshold-1)->xCoordinate;
	HEADER(output,inputBytes,threshold-1)->xCoordinate = X0;

	/*
	 * Now set each of the remaining bodies via interpolation.
	 * Work from last to threshold-1 so we can leave our input copy in
	 * the threshold slot.
	 */
	for( i=nShares-1; i!=threshold-2; --i )
	{
		PGPByte tmp;

		/* Interpolate to that value */
		sInterp( (unsigned char *) output, inputBytes, xupdate, threshold );
		for( j=0; j<inputBytes; ++j )
		{
			BODY(output, inputBytes, i)[j] =
				sDoInterp( (unsigned char *) output, inputBytes, threshold, j );
		}
		/* Swap in xupdate value for share we just calculated */
		tmp = HEADER(output,inputBytes,i)->xCoordinate;
		HEADER(output,inputBytes,i)->xCoordinate = xupdate;
		xupdate = tmp;
	}

	/* Zero lagrange values, were just temporary */
	for( i=0; i<nShares; ++i )
	{
		HEADER(output,inputBytes,i)->lagrange = 0;
	}
	return kPGPError_NoErr;
}


	PGPError
PGPSecretReconstructData(
	PGPContextRef		context,
	void *				input,
	PGPSize				outputBytes,
	PGPUInt32			nShares,
	void *				output
	)
{
	PGPUInt32			i, j;
	PGPByte				threshold;
	PGPError			err = kPGPError_NoErr;

	(void) context;

	/* Verify version and threshold consistency */
	threshold = HEADER(input,outputBytes,0)->threshold;

	for( i=0; i<nShares; ++i )
	{
		if( HEADER(input,outputBytes,i)->version != kPGPShare_Version1 )
		{
			pgpDebugMsg( "Error: invalid Reconstruction version" );
			err = kPGPError_BadParams;
			goto error;
		}
		if( HEADER(input,outputBytes,i)->threshold != threshold )
		{
			pgpDebugMsg( "Error: inconsistent threshold in reconstruction" );
			err = kPGPError_BadParams;
			goto error;
		}
	}

	/* Make sure we have enough shares */
	if (nShares < threshold)
	{
		pgpDebugMsg( "Error: insufficient shares in reconstruction" );
		err = kPGPError_BadParams;
		goto error;
	}

	/* Set up Lagrange coefficients to interpolate to x=X0 */
	sInterp( (unsigned char *) input, outputBytes, X0, threshold );

	/* For each byte j, interpolate to output[j] using coordinates */
	for( j=0; j<outputBytes; ++j )
	{
		((PGPByte *)output)[j] =
				sDoInterp( (PGPByte *)input, outputBytes, threshold, j );
	}
	
	/* Zero lagrange values, were just temporary */
	for( i=0; i<nShares; ++i )
	{
		HEADER(input,outputBytes,i)->lagrange = 0;
	}

error:

	return err;
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

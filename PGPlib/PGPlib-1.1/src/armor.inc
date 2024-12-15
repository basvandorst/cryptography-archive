/* (Hey, Emacs, this is actually -*-C-*-)
 * 
 * The code below was taken from PGP, armor.c in PGP 2.6.3.a.  
 * There, the following note can be found

   (c) Copyright 1990-1996 by Philip Zimmermann.  All rights reserved.
   The author assumes no liability for damages resulting from the use
   of this software, even if the damage results from defects in this
   software.  No warranty is expressed or implied.

   Note that while most PGP source modules bear Philip Zimmermann's
   copyright notice, many of them have been revised or entirely written
   by contributors who frequently failed to put their names in their
   code.  Code that has been incorporated into PGP from other authors
   was either originally published in the public domain or is used with
   permission from the various authors.

   PGP is available for free to the public under certain restrictions.
   See the PGP User's Guide (included in the release package) for
   important information about licensing, patent restrictions on
   certain algorithms, trademarks, copyrights, and export controls.

 * Modified by Tage Stabell-Kulø <tage@acm.org> for PGPlib to work with 
 * buffers rather than with FILEs.
 * Also, cleaned up usage of char rather than unsigned char.
 */

/*====================BEGIN armor.c from PGP==========*/

/************************************************************************
 * CRC Routines.
 *    These CRC functions are derived from code in chapter 19 of the book 
 *    "C Programmer's Guide to Serial Communications", by Joe Campbell.
 *    Generalized to any CRC width by Philip Zimmermann.
 */

#define	crcword		unsigned long

#define CRCBITS 	24		/* may be 16, 24, or 32 */
#define maskcrc(crc)	((crc) & 0xffffffL) /* if CRCBITS is 24 */
#define CRCHIBIT 	((crcword) (1L<<(CRCBITS-1))) /* 0x8000 if CRCBITS is 16 */
#define CRCSHIFTS       (CRCBITS-8)

/*
 * Notes on making a good 24-bit CRC--
 * The primitive irreducible polynomial of degree 23 over GF(2),
 * 040435651 (octal), comes from Appendix C of "Error Correcting Codes,
 * 2nd edition" by Peterson and Weldon, page 490.  This polynomial was
 * chosen for its uniform density of ones and zeros, which has better
 * error detection properties than polynomials with a minimal number of
 * nonzero terms.  Multiplying this primitive degree-23 polynomial by
 * the polynomial x+1 yields the additional property of detecting any
 * odd number of bits in error, which means it adds parity.  This 
 * approach was recommended by Neal Glover.
 *
 * To multiply the polynomial 040435651 by x+1, shift it left 1 bit and
 * bitwise add (xor) the unshifted version back in.  Dropping the unused 
 * upper bit (bit 24) produces a CRC-24 generator bitmask of 041446373 
 * octal, or 0x864cfb hex.  
 *
 * You can detect spurious leading zeros or framing errors in the 
 * message by initializing the CRC accumulator to some agreed-upon 
 * nonzero value, but the value used is a bit nonstandard.  
 */

#define CCITTCRC 0x1021			/* CCITT's 16-bit CRC generator polynomial */
#define PRZCRC	0x864cfbL		/* PRZ's 24-bit CRC generator polynomial */
#define CRCINIT	0xB704CEL		/* Init value for CRC accumulator */

static crcword crctable[256];		/* Table for speeding up CRC's */

/*
 * mk_crctbl derives a CRC lookup table from the CRC polynomial.
 * The table is used later by the crcbytes function given below.
 * mk_crctbl only needs to be called once at the dawn of time.
 *
 * The theory behind mk_crctbl is that table[i] is initialized
 * with the CRC of i, and this is related to the CRC of i>>1,
 * so the CRC of i>>1 (pointed to by p) can be used to derive
 * the CRC of i (pointed to by q).
 */

static void
mk_crctbl(crcword poly)
{
    int 	i;
    crcword 	t, *p, *q;
    
    p = q = crctable;
    *q++ = 0;
    *q++ = poly;
    for (i = 1; i < 128; i++) {
	t = *++p;
	if (t & CRCHIBIT) {
	    t <<= 1;
	    *q++ = t ^ poly;
	    *q++ = t;
	} else {
	    t <<= 1;
	    *q++ = t;
	    *q++ = t ^ poly;
	}
    }
}


/*
 * Accumulate a buffer's worth of bytes into a CRC accumulator,
 * returning the new CRC value.
 */
crcword
crcbytes(u_char		*buf, 
	 u_long		len, 
	 u_long	 	accum)
{
    do {
	accum = accum << 8 ^ crctable[(u_char) (accum >> CRCSHIFTS) ^ *buf++];
    } while (--len);
    return maskcrc(accum);
}				/* crcbytes */

/* Initialize the CRC table using our codes */
void
init_crc(void)
{
    mk_crctbl(PRZCRC);
}


/*====================END armor.c from PGP==========*/

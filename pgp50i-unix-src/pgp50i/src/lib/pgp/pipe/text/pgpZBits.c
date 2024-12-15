/*
* pgpZBits.c
*
* Copyright (C) 1990,1991 Mark Adler, Richard B. Wales, and Jean-loup
* Gailly. Permission is granted to any individual or institution to use,
* copy, or redistribute this software so long as all of the original
* files are included unmodified, that it is not sold for profit, and
* that this copyright notice is retained.
*
* Hacked up for PGP by Colin Plumb
*
* $Id: pgpZBits.c,v 1.1.2.1 1997/06/07 09:51:10 mhw Exp $
*/

/*
* bits.c by Jean-loup Gailly.
*
* This is a new version of im_bits.c originally written by Richard B. Wales
*
* PURPOSE
*
* Output variable-length bit strings.
*
* DISCUSSION
*
*	The PKZIP "deflate" file format interprets compressed file data
*	as a sequence of bits. Multi-bit strings in the file may cross
*	byte boundaries without restriction.
*
*	The first bit of each byte is the low-order bit.
*
*	The routines in this file allow a variable-length bit value to
*	be output right-to-left (useful for literal values). For
*	left-to-right output (useful for code strings from the tree routines),
*	the bits must have been reversed first with bi_reverse().
*
* INTERFACE
*
*	void bi_init (struct PgpFifoContext *zipfifo)
*	Initialize the bit string routines. If zipfifo is
*	non-NULL, does a hard reset. If it is NULL, just
*	sets variables to re-fetch the FIFO write pointer,
*	because it has been invalidated by FIFO reads.
*
*	void send_bits (int value, int length)
*	Write out a bit string, taking the source bits right to
*	left.
*
*	int bi_reverse (int value, int length)
*	Reverse the bits of a bit string, taking the source bits left to
*	right and emitting them right to left.
*
*	void bi_windup (void)
*	Write out any remaining bits in an incomplete byte.
*
*	void copy_block(char const far *buf, unsigned len, int header)
*	Copy a stored block to the zip file, storing first the length and
*	its one's complement if requested.
*
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpByteFIFO.h"
#include "pgpZip.h"

/* ===========================================================================
* Local data used by the "bit string" routines.
*/

static unsigned bi_buf;
/* Output buffer. bits are inserted starting at the bottom (least significant
* bits). High-order unused bits are always zero.
*/

#define Buf_size 16
/* Number of bits used within bi_buf. (bi_buf might be implemented on
* more than 16 bits on some systems.)
*/

static int bi_valid; /* number of valid bits in bi_buf */
/* All bits above the last valid bit are always zero.
*/

#ifdef ZIPDEBUG
word32 bits_sent; /* bit length of the compressed data */
#endif

/* The current output buffer */
static unsigned char *outptr;
static unsigned outtotal;
static unsigned outleft;

/* How to do output. */
struct PgpFifoContext *zfifo;

#define bi_getspace(len) byteFifoGetSpace(zfifo, (len))
#define bi_skipspace(len) byteFifoSkipSpace(zfifo, (len))

#define PUTBYTE(c) (outleft = (outleft ? outleft : \
(bi_skipspace(outtotal), \
outptr = bi_getspace(&outtotal), \
outtotal))-1, \
*outptr++ = (unsigned char)(c))

#define PUTSHORT(w) (PUTBYTE(w), PUTBYTE((w)>>8))


void
bi_init(struct PgpFifoContext *zipfifo)
{
 outleft = outtotal = 0;
 zfifo = zipfifo;

 bi_buf = 0;
 bi_valid = 0;
#ifdef ZIPDEBUG
bits_sent = 0L;
#endif
}

/*
* Make sure that all data written out is recorded, and invalidate
* the cached pointers because reading the output buffer might
* move things around. PUTBYTE will re-fetch the appropriate data.
*/
void
bi_flush(void)
{
 bi_skipspace(outtotal-outleft);
 outleft = outtotal = 0;
}

void
bi_windup(void)
	{
		if (bi_valid > 8) {
		 PUTSHORT(bi_buf);
		} else if (bi_valid > 0) {
		 PUTBYTE(bi_buf);
		}
		bi_buf = 0;
		bi_valid = 0;
#ifdef ZIPDEBUG
		bits_sent = (bits_sent+7) & ~7;
#endif

		bi_skipspace(outtotal-outleft);
		outptr = bi_getspace(&outtotal);
		outleft = outtotal;
	}

void
copy_block(char const far *buf, unsigned len, int header)
{
  bi_windup(); 		/* align on byte boundary */

#ifdef ZIPDEBUG
  bits_sent += (word32)len<<3;
#endif

		if (header) {
			PUTSHORT((word16)len);
			PUTSHORT((word16)~len);
#ifdef ZIPDEBUG
			bits_sent += 2*16;
#endif
		}
		while (len >= outleft) {
		 memcpy(outptr, buf, outleft);
		 len -= outleft;
		 buf += outleft;
		 bi_skipspace(outtotal);
		 outptr = bi_getspace(&outtotal);
		 outleft = outtotal;
		}
		if (len) {
			memcpy(outptr, buf, len);
			outptr += len;
			outleft -= len;
		}
}


/* ===========================================================================
* Send a value made up of a given number of bits.
* IN assertion: length <= 15 and value < (1 << length).
*/
void
send_bits(int value, int length)
{
#ifdef ZIPDEBUG
		Tracevv((stderr," l %2d v %4x ", length, value));
		ZipAssert(length > 0 && length <= 15, "invalid length");
		ZipAssert(value < (1 << length), "invalid value");
		bits_sent += (word32)length;
	#endif
		/*
		* Add extra bits to bi_buf and add length to bi_valid. If this
		* fills bi_buf, copy Buf_size bits to the output, decrementing
		* bi_valid appropriately and set bi_buf to the remaining bi_valid
		* high bits of value. (value >> (length - bi_valid)).
		*/
		bi_buf |= (value << bi_valid);
		bi_valid += length;
		if (bi_valid >= (int)Buf_size) {
			PUTSHORT(bi_buf);
			bi_valid -= (int)Buf_size;
			bi_buf = (unsigned)value >> (length - bi_valid);
		}
}

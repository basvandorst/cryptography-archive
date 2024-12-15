/*
 *
 * Copyright (C) 1990,1991 Mark Adler, Richard B. Wales, and Jean-loup
 * Gailly.  Permission is granted to any individual or institution to use,
 * copy, or redistribute this software so long as all of the original
 * files are included unmodified, that it is not sold for profit, and
 * that this copyright notice is retained.
 *
 * Hacked up for PGP by Colin Plumb
 *
 * $Id: pgpZBits.c,v 1.12 1997/12/12 01:14:10 heller Exp $
 */

/*
 *  bits.c by Jean-loup Gailly.
 *
 *  This is a new version of im_bits.c originally written by Richard B. Wales
 *
 *  PURPOSE
 *
 *      Output variable-length bit strings.
 *
 *  DISCUSSION
 *
 *      The PKZIP "deflate" file format interprets compressed file data
 *      as a sequence of bits.  Multi-bit strings in the file may cross
 *      byte boundaries without restriction.
 *
 *      The first bit of each byte is the low-order bit.
 *
 *      The routines in this file allow a variable-length bit value to
 *      be output right-to-left (useful for literal values). For
 *      left-to-right output (useful for code strings from the tree routines),
 *      the bits must have been reversed first with bi_reverse().
 *
 *  INTERFACE
 *
 *      void bi_init (PGPFifoContext *zipfifo)
 *          Initialize the bit string routines.  If zipfifo is
 *          non-NULL, does a hard reset.  If it is NULL, just
 *          sets variables to re-fetch the FIFO write pointer,
 *          because it has been invalidated by FIFO reads.
 *
 *      void send_bits (int value, int length)
 *          Write out a bit string, taking the source bits right to
 *          left.
 *
 *      int bi_reverse (int value, int length)
 *          Reverse the bits of a bit string, taking the source bits left to
 *          right and emitting them right to left.
 *
 *      void bi_windup (void)
 *          Write out any remaining bits in an incomplete byte.
 *
 *      void copy_block(char const far *buf, unsigned len, int header)
 *          Copy a stored block to the zip file, storing first the length and
 *          its one's complement if requested.
 *
 */

#include "pgpConfig.h"

#include "pgpByteFIFO.h"
#include "pgpContext.h"
#include "pgpZip.h"

/* ===========================================================================
 * Local data used by the "bit string" routines.
 */

#define Buf_size 16
/* Number of bits used within bi_buf. (bi_buf might be implemented on
 * more than 16 bits on some systems.)
 */

typedef struct ZBitsContext {

	PGPContextRef		cdkContext;

	unsigned	 bi_buf;
	/* Output buffer. bits are inserted starting at the bottom (least
	 * significant bits).  High-order unused bits are always zero.
	 */

	int	 bi_valid;                /* number of valid bits in bi_buf */
	/* All bits above the last valid bit are always zero.
	 */

	/* The current output buffer */
	unsigned char *outptr;
	PGPSize	outtotal;
	PGPSize	outleft;

	/* How to do output. */
	PGPFifoContext *zfifo;
} ZBitsContext;

#ifdef ZIPDEBUG
PGPUInt32 bits_sent;   /* bit length of the compressed data */
#endif



#define bi_getspace(ctx, len) byteFifoGetSpace((ctx)->zfifo, (len))
#define bi_skipspace(ctx, len) byteFifoSkipSpace((ctx)->zfifo, (len))

#define PUTBYTE(ctx, c) ((ctx)->outleft = ((ctx)->outleft ? (ctx)->outleft : \
                                     (bi_skipspace((ctx), (ctx)->outtotal), \
                                     (ctx)->outptr = \
										bi_getspace((ctx), &(ctx)->outtotal), \
                                         (ctx)->outtotal))-1, \
                    	*(ctx)->outptr++ = (unsigned char)(c))

#define PUTSHORT(ctx, w) (PUTBYTE((ctx), w), PUTBYTE((ctx), (w)>>8))


ZBitsContext *
bi_init(PGPContextRef cdkContext, PGPFifoContext *zipfifo)
{
	ZBitsContext *ctx;

	ctx = (ZBitsContext *) pgpContextMemAlloc (cdkContext, sizeof(*ctx),
									kPGPMemoryMgrFlags_Clear);
	if (IsNull (ctx) )
		return NULL;
	ctx->cdkContext = cdkContext;

	ctx->outleft = ctx->outtotal = 0;
	ctx->zfifo = zipfifo;

	ctx->bi_buf = 0;
	ctx->bi_valid = 0;
#ifdef ZIPDEBUG
	bits_sent = 0L;
#endif
	return ctx;
}

void
bi_free(ZBitsContext *ctx)
{
	/* Caller is responsible for freeing fifo */
	PGPContextRef cdkContext;

	cdkContext = ctx->cdkContext;
	pgpClearMemory( ctx, sizeof(*ctx) );
	pgpContextMemFree( cdkContext, ctx );
}

/*
 * Make sure that all data written out is recorded, and invalidate
 * the cached pointers because reading the output buffer might
 * move things around.  PUTBYTE will re-fetch the appropriate data.
 */
void
bi_flush(ZBitsContext *ctx)
{
	bi_skipspace(ctx, ctx->outtotal-ctx->outleft);
	ctx->outleft = ctx->outtotal = 0;
}

void
bi_windup(ZBitsContext *ctx)
{
	if (ctx->bi_valid > 8) {
		PUTSHORT(ctx, ctx->bi_buf);
	} else if (ctx->bi_valid > 0) {
		PUTBYTE(ctx, ctx->bi_buf);
	}
	ctx->bi_buf = 0;
	ctx->bi_valid = 0;
#ifdef ZIPDEBUG
	bits_sent = (bits_sent+7) & ~7;
#endif

	bi_skipspace(ctx, ctx->outtotal-ctx->outleft);
	ctx->outptr = bi_getspace(ctx, &ctx->outtotal);
	ctx->outleft = ctx->outtotal;
}

void
copy_block(ZBitsContext *ctx, char const far *buf, unsigned len, int header)
{
	bi_windup(ctx);           /* align on byte boundary */

#ifdef ZIPDEBUG
	bits_sent += (PGPUInt32)len<<3;
#endif

	if (header) {
		PUTSHORT(ctx, (PGPUInt16)len);
		PUTSHORT(ctx, (PGPUInt16)~len);
#ifdef ZIPDEBUG
		bits_sent += 2*16;
#endif
	}
	while (len >= ctx->outleft) {
		memcpy(ctx->outptr, buf, ctx->outleft);
		len -= ctx->outleft;
		buf += ctx->outleft;
		bi_skipspace(ctx, ctx->outtotal);
		ctx->outptr = bi_getspace(ctx, &ctx->outtotal);
		ctx->outleft = ctx->outtotal;
	}
	if (len) {
		memcpy(ctx->outptr, buf, len);
		ctx->outptr += len;
		ctx->outleft -= len;
	}
}


/* ===========================================================================
 * Send a value made up of a given number of bits.
 * IN assertion: length <= 15 and value < (1 << length).
 */
void
send_bits(ZBitsContext *ctx, int value, int length)
{
#ifdef ZIPDEBUG
	Tracevv((stderr," l %2d v %4x ", length, value));
	ZipAssert(length > 0 && length <= 15, "invalid length");
	ZipAssert(value < (1 << length), "invalid value");
	bits_sent += (PGPUInt32)length;
#endif
	/*
	 * Add extra bits to bi_buf and add length to bi_valid.  If this
	 * fills bi_buf, copy Buf_size bits to the output, decrementing
	 * bi_valid appropriately and set bi_buf to the remaining bi_valid
	 * high bits of value.  (value >> (length - bi_valid)).
	 */
	ctx->bi_buf |= (value << ctx->bi_valid);
	ctx->bi_valid += length;
	if (ctx->bi_valid >= (int)Buf_size) {
		PUTSHORT(ctx, ctx->bi_buf);
		ctx->bi_valid -= (int)Buf_size;
		ctx->bi_buf = (unsigned)value >> (length - ctx->bi_valid);
	}
}

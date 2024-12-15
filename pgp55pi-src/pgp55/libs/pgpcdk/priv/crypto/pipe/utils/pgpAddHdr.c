/*
 * pgpAddHdr.c -- Add a PGP Packet Header to a data stream.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpAddHdr.c,v 1.22 1997/08/27 00:23:12 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpAddHdr.h"
#include "pgpPktByte.h"
#include "pgpAnnotate.h"
#include "pgpFIFO.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpErrors.h"
#include "pgpContext.h"

#define ADDHEADERMAGIC	0xadd0b11e
#define BLOCK_SIZE 12
#define BUFFER_SIZE (1 << BLOCK_SIZE)	/* 2^10 == 4k */

typedef struct AddHdrContext {
	PGPPipeline		pipe;
	
	PGPFifoDesc const *fifod;
	PGPFifoContext *fifo;
	PGPPipeline *tail;
	PGPByte header[5];			/* pktbyte + 4byte length */
	PGPByte *ptr;
	int hdrlen;			/* actual header length */
	int dowrite;
	int sizeknown;			/* bytes is valid */
	unsigned long bytes;		/* sizeAdvise() promise */
	PgpVersion version;
	PGPByte *buffer;
	PGPByte *bufwrite;
	PGPByte *bufptr;
	int buflen;
	int midflush;
	int scope_depth;
	DEBUG_STRUCT_CONSTRUCTOR( AddHdrContext )
} AddHdrContext;

/*
 * Return the largest x such that 2^x <= n, n>0
 * Return -1 if n==0
 */
static int
NumToBits(unsigned int n)
{
	int i;

	if (!n)
		return -1;

	for (i=0; n > 1; i++)
		n >>= 1;

	return i;
}

static PGPError
FlushHeader(AddHdrContext *context)
{
	PGPError	error;
	size_t retlen;

	while (context->hdrlen) {
		retlen = context->tail->write(context->tail,
					       context->ptr,
					       context->hdrlen,
					       &error);
		context->ptr += retlen;
		context->hdrlen -= retlen;
		if (error)
			return error;
	}
	return( kPGPError_NoErr );
}

static PGPError
FlushBuffer(AddHdrContext *context)
{
	PGPError	error;
	size_t retlen;

	while (context->buflen) {
		retlen = context->tail->write(context->tail,
					       context->bufptr,
					       context->buflen,
					       &error);
		context->bufptr += retlen;
		context->buflen -= retlen;
		if (error)
			return error;
	}
	return( kPGPError_NoErr );
}

static PGPError
ForceFlush(AddHdrContext *context)
{
	PGPError	error;
	int bufl, flushed;

	while (context->buflen) {
		/* First, flush out any mid-flush we might have */

		bufl = context->buflen;
		flushed = context->midflush;
		context->buflen = context->midflush;

		error = FlushBuffer(context);

		flushed -= context->buflen;
		context->midflush = context->buflen;
		context->buflen = bufl - flushed;

		if (error)
			return error;

		/*
		 * Now, figure out how much should be sent in the
		 * next sub-packet
		 */
		bufl = NumToBits(context->buflen);
		if (bufl < 0)
			break;

		context->bufptr--;
		context->buflen++;
		*(context->bufptr) = 0xE0 + bufl;
		context->midflush = (2^bufl) + 1;
	}

	return( kPGPError_NoErr );
}

static PGPError
DoFlush(AddHdrContext *context)
{
	PGPError	error = kPGPError_NoErr;
	size_t retlen;
	PGPByte const *p;
	PGPSize len;

	error = FlushHeader(context);
	if (error)
		return error;

	/* Next, try to flush anything that we have buffered in the fifo */
	p = pgpFifoPeek (context->fifod, context->fifo, &len);
	while (len) {
		retlen = context->tail->write(context->tail,
					       p, len, &error);
		pgpFifoSeek (context->fifod, context->fifo, retlen);
		if (error)
			return error;
		
		p = pgpFifoPeek (context->fifod, context->fifo, &len);
	}
	return error;
}

static PGPError
Flush(PGPPipeline *myself)
{
	AddHdrContext *context;
	PGPError	error;

	pgpAssert(myself);
	pgpAssert(myself->magic == ADDHEADERMAGIC);

	context = (AddHdrContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	if (context->buffer) {
		error = FlushHeader(context);
		if (error)
			return error;

		/* Using the new packets -- force a flush of the buffer */
		if (context->dowrite) {
			error = FlushBuffer(context);
			if (error)
				return error;
		} else {
			error = ForceFlush(context);
			if (error)
				return error;
		}

	} else {
		/* Using the fifo */

		if (!context->dowrite)
			return kPGPError_BadParams;

		error = DoFlush(context);
		if (error)
			return error;
	}

	return context->tail->flush(context->tail);
}

/* Only called after args are checked, so I don't have to check them */
static size_t
BufferWrite(PGPPipeline *myself, PGPByte const *buf, size_t size,
	     PGPError *error)
{
	AddHdrContext *context;
	size_t size0 = size;
	int t;

	pgpAssert(myself);
	pgpAssert(myself->magic == ADDHEADERMAGIC);
	pgpAssert(error);

	context = (AddHdrContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);
	pgpAssert(context->buffer);

	*error = FlushHeader(context);
	if (*error)
		return 0;

	do {
		/* If we are in the middle of a block, flush it out */
		if (context->dowrite) {
			*error = FlushBuffer(context);
			if (*error)
				break;

			context->dowrite = 0;
			context->bufptr = context->buffer + 2;
			context->bufwrite = context->bufptr;
		}

		/* Try to fill up the buffer */
		t = pgpMin(((size_t)(BUFFER_SIZE - context->buflen)), size);
		if (t) {
			memcpy(context->bufwrite, buf, t);
			buf += t;
			size -= t;
			context->bufwrite += t;
			context->buflen += t;
		}

		/*
		 * If we have a full block, write out the length
		 * and then write out the block
		 */
		if (context->buflen == BUFFER_SIZE) {
			context->dowrite = 1;
			context->bufptr--;
			context->buflen++;
			*(context->bufptr) = 0xE0 + BLOCK_SIZE;

			continue;
		}

	} while (size);

	return size0 - size;
}

static size_t
Write(PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	AddHdrContext *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == ADDHEADERMAGIC);
	pgpAssert(error);

	context = (AddHdrContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);
	pgpAssert(context->fifo);

	if (!context->dowrite) {
		/* Buffer into the fifo until we get a sizeadvise */

		return pgpFifoWrite (context->fifod, context->fifo, buf, size);
	}

	/*
	 * Ok, we've been given our size.  Flush the fifo and then be
	 * a write-through
	 */
	*error = DoFlush(context);
	if (*error)
		return 0;
	if (context->sizeknown && size > context->bytes) {
		pgpAssert(0);
		*error = kPGPError_SizeAdviseFailure;
		return 0;
	}

	size = context->tail->write(context->tail, buf, size, error);
	context->bytes -= size;
	return size;
}

static PGPError
Annotate(PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	AddHdrContext *context;
	PGPError	error;

	pgpAssert(myself);
	pgpAssert(myself->magic == ADDHEADERMAGIC);

	context = (AddHdrContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	error = context->tail->annotate(context->tail, origin, type,
					string, size);
	if (!error)
		PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);
	return error;
}

static PGPError
BufferSizeAdvise(PGPPipeline *myself, unsigned long size)
{
	AddHdrContext *context;
	PGPError	error;
	int i;

	pgpAssert(myself);
	pgpAssert(myself->magic == ADDHEADERMAGIC);

	context = (AddHdrContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);
	pgpAssert(context->buffer);

	/* Do not pass non-zero sizeAdvise -- I can't do that! */
	if (size || context->scope_depth)
		return( kPGPError_NoErr );

	/* Okay, we're at end of input. */
	if (context->buflen) {
		/* We have a bug when file length is zero; we get here without having
		 * flushed the header.  We can't easily distinguish between a zero
		 * length file and the EOF sizeadvise, but we don't need to.
		 */
		error = FlushHeader(context);
		if (error)
			return error;
		if (context->dowrite) {
			error = FlushBuffer(context);
		} else if (context->midflush) {
			error = ForceFlush(context);
		} else {
			i = context->buflen;
			if (PKTLEN_ONE_BYTE(i)) {
				context->bufptr--;
				context->buflen++;
				context->bufptr[0] = PKTLEN_1BYTE(i);
			} else {
				context->bufptr -= 2;
				context->buflen += 2;
				context->bufptr[0] = PKTLEN_BYTE0(i);
				context->bufptr[1] = PKTLEN_BYTE1(i);
			}
			context->midflush = context->buflen;
			error = ForceFlush(context);
		}
		if (error)
			return error;
	}
	
	return context->tail->sizeAdvise(context->tail, 0);
}

static PGPError
SizeAdvise(PGPPipeline *myself, unsigned long len)
{
	AddHdrContext *context;
	unsigned long size;
	PGPError	error;
	int i;

	pgpAssert(myself);
	pgpAssert(myself->magic == ADDHEADERMAGIC);

	context = (AddHdrContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);
	pgpAssert(context->fifo);

	if (context->scope_depth)
		return( kPGPError_NoErr );	/* Can't pass it through */

	/*
	 * The actual size myself module will output is the byte size
	 * passed in (size), plus the size of the buffered header,
	 * plus the size of the data in the fifo.  However the packet
	 * size does not include the header size, so add in the header
	 * size after we compute the header.
	 */

	/*
	 * Set the header packet up properly, if we haven't already
	 * done so.  We know how long the length-of-length should be,
	 * so fill that many bytes of the header in MSB-first order.
	 */
	if (!context->dowrite) {
		size = len + pgpFifoSize (context->fifod, context->fifo);
	
		i = PKTBYTE_LLEN(context->header[0]);
		if (size >= 0x10000 && i <= 2)
			i = 2;
		else if (size >= 0x100 && i <= 1)
			i = 1;
		context->header[0] = (context->header[0] & ~3) | i;

		i = LLEN_TO_BYTES(i);
		context->hdrlen = i+1;
		while (i--) {
			context->header[i+1] = (PGPByte)(size & 0xff);
			size >>= 8;
		}
	}

	if (!context->sizeknown) {
		context->bytes = len;
		context->sizeknown = 1;
	} else if (context->bytes != len) {
		return kPGPError_SizeAdviseFailure;
	}

	/*
	 * Now add in the header size, sizeAdvise the next module, and
	 * if everything is ok, set the state to dowrite and flush.
	 */	   
	size = len + pgpFifoSize (context->fifod, context->fifo)
		+ context->hdrlen;
	error = context->tail->sizeAdvise(context->tail, size);
	if (error)
		return error;

	context->dowrite = 1;

	error = DoFlush(context);
	/*
	 * Just in case we got a sizeAdvise(0) without anything else.
	 * calling sizeAdvise(0) twice is ok!
	 */
	if (!error && !len)
		error = context->tail->sizeAdvise(context->tail, 0);

	return error;
}

static PGPError
Teardown(PGPPipeline *myself)
{
	AddHdrContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert(myself);
	pgpAssert(myself->magic == ADDHEADERMAGIC);

	context = (AddHdrContext *)myself->priv;
	pgpAssert(context);

	if (context->tail)
		context->tail->teardown(context->tail);

	if (context->fifo)
		pgpFifoDestroy (context->fifod, context->fifo);

	if (context->buffer) {
		pgpClearMemory(context->buffer, BUFFER_SIZE+2);
		pgpContextMemFree( cdkContext, context->buffer);
	}
	
	pgpClearMemory(context, sizeof(*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

PGPPipeline **
pgpAddHeaderCreate(
	PGPContextRef		cdkContext,
	PGPPipeline **		head,
	PgpVersion			vers,
	PGPFifoDesc const *	fd,
	PGPByte				pkttype,
	PGPByte llen,
	PGPByte *hdr,
	size_t headerlen)
{
	PGPPipeline *mod;
	PGPFifoContext *fifop = NULL;
	AddHdrContext *context;
	PGPByte *buf = NULL;
	PGPByte pktbite;

	if (!head)
		return NULL;

	pgpAssert(fd);

	/* XXX If we have versions > 2.6, ignore llen */
	if (vers > PGPVERSION_2_6)
		pktbite = PKTBYTE_BUILD_NEW(pkttype);
	else
		pktbite = PKTBYTE_BUILD(pkttype, llen);

	context = (AddHdrContext *)pgpContextMemAlloc( cdkContext,
		sizeof(*context), kPGPMemoryFlags_Clear );
	if (!context)
		return NULL;
	mod = &context->pipe;

	if (vers > PGPVERSION_2_6) {
		buf = (PGPByte *)pgpContextMemAlloc( cdkContext,
			BUFFER_SIZE + 2, kPGPMemoryFlags_Clear);
		if (!buf) {
			pgpContextMemFree( cdkContext, context);
			return NULL;
		}
		memcpy(buf+2, hdr, headerlen);

	} else {
		fifop = pgpFifoCreate ( cdkContext, fd);
		if (!fifop) {
			pgpContextMemFree( cdkContext, context);
			return NULL;
		}

		if (pgpFifoWrite (fd, fifop, hdr, headerlen) != headerlen) {
			pgpContextMemFree( cdkContext, context);
			pgpFifoDestroy (fd, fifop);
			return NULL;
		}
	}

	mod->magic = ADDHEADERMAGIC;
	mod->flush = Flush;
	mod->write = Write;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Add Header Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	if (buf) {
		mod->write = BufferWrite;
		mod->sizeAdvise = BufferSizeAdvise;
	}

	context->fifod = fd;
	context->fifo = fifop;
	context->ptr = context->header;
	context->header[0] = pktbite;
	context->hdrlen = 1;
	context->version = vers;
	context->buffer = buf;
	context->bufptr = buf + 2;
	context->bufwrite = context->bufptr + headerlen;

	if (vers > PGPVERSION_2_6)
		context->buflen = headerlen;
	else if (llen == 3)
		context->dowrite = 1;
	
	context->tail = *head;
	*head = mod;
	return &context->tail;
}

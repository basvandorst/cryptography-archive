/*
 * pgpFlexFIFO.c
 * Use a memory buffer as a fifo unless it runs out, at which point we
 * switch to a disk file fifo.
 *
 * $Id: pgpFlexFIFO.c,v 1.20 1997/08/27 00:23:18 lloyd Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>	/* For BUFSIZ */

#if HAVE_UNISTD_H
#include <unistd.h>	/* For unlink() */
#endif

#include "pgpDebug.h"
#include "pgpFIFO.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

#if (BUFSIZ < 16384) && (PGP_MACINTOSH || PGP_WIN32)
#define kPGPFIFOBufSize		16384
#else
#define kPGPFIFOBufSize		BUFSIZ
#endif

/*
 * Size at which we will switch from byte to disk fifos.  Will also
 * do so if we run out of memory so set this very large to switch only
 * in that case.
 */
#define MAXBYTEFIFOSIZE (48*1024L)	/* We might want to adjust this */

PGPFifoDesc const *bf = &pgpByteFifoDesc;
PGPFifoDesc const *ff = &pgpFileFifoDesc;

struct PGPFifoContext {
	PGPContextRef	context;
	PGPFifoContext *bytefifo;
	PGPFifoContext *filefifo;
	PGPByte infile;
	size_t bytefifosize;
	size_t maxbytefifosize;
	DEBUG_STRUCT_CONSTRUCTOR( PGPFifoContext )
};

static void
flexFifoInit(PGPFifoContext *fifo)
{
	fifo->bytefifo = NULL;
	fifo->filefifo = NULL;
	fifo->infile = 0;
	fifo->bytefifosize = 0;
	fifo->maxbytefifosize = MAXBYTEFIFOSIZE;
}

static void
flexFifoFlush(PGPFifoContext *fifo)
{
	if (fifo->infile)
		pgpFifoFlush(ff, fifo->filefifo);
	else
		pgpFifoFlush(bf, fifo->bytefifo);
	flexFifoInit(fifo);
}

static PGPSize
flexFifoSize(PGPFifoContext const *fifo)
{
	if (fifo->infile)
		return pgpFifoSize(ff, fifo->filefifo);
	else
		return pgpFifoSize(bf, fifo->bytefifo);
}

static PGPFifoContext *
flexFifoCreate( PGPContextRef	context )
{
	PGPFifoContext *fifo;

	fifo = (PGPFifoContext *)pgpContextMemAlloc( context,
		sizeof(*fifo), 0 );
	if (fifo) {
		fifo->context	= context;
		fifo->infile = 0;
		fifo->filefifo = NULL;
		fifo->bytefifo = pgpFifoCreate( context, bf);
		if (!fifo->bytefifo) {
			pgpContextMemFree( context, fifo);
			return NULL;
		}
		fifo->bytefifosize = 0;
		fifo->maxbytefifosize = MAXBYTEFIFOSIZE;
	}
	return fifo;
}

static void
flexFifoDestroy(PGPFifoContext *fifo)
{
	if (fifo->infile)
		pgpFifoDestroy(ff, fifo->filefifo);
	else
		pgpFifoDestroy(bf, fifo->bytefifo);
	pgpContextMemFree( fifo->context, fifo);
}


static PGPByte const *
flexFifoPeek(PGPFifoContext *fifo, PGPSize *len)
{
	if (fifo->infile)
		return pgpFifoPeek(ff, fifo->filefifo, len);
	else
		return pgpFifoPeek(bf, fifo->bytefifo, len);
}

static void
flexFifoSeek(PGPFifoContext *fifo, PGPSize len)
{
	if (fifo->infile)
		pgpFifoSeek(ff, fifo->filefifo, len);
	else {
		pgpFifoSeek(bf, fifo->bytefifo, len);
		fifo->bytefifosize -= len;
	}
}

static size_t
flexFifoRead(PGPFifoContext *fifo, PGPByte *buf, size_t len)
{
	if (fifo->infile)
		return pgpFifoRead(ff, fifo->filefifo, buf, len);
	else {
		size_t nread = pgpFifoRead(bf, fifo->bytefifo, buf, len);
		if (nread > 0)
			fifo->bytefifosize -= nread;
		return nread;
	}
}

/*
 * If bytefifo fails to take the requested data, assume that memory is
 * full.  Switch to using a file fifo and transfer all data from the
 * byte fifo to the file.  Hopefully this will free up enough memory
 * that we can complete.
 *
 * We can also be configured to have a maximum size for the byte fifo
 * and if we exceed that we will switch over.
 */
static size_t
flexFifoWrite(PGPFifoContext *fifo, PGPByte const *buf, size_t len)
{
	PGPByte *xbuf = NULL;
	size_t written;
	PGPSize	n;
	long err;
	PGPContextRef		cdkContext;
	
	pgpAssertAddrValid( fifo, PGPFifoContext );
	cdkContext	= fifo->context;

	if (fifo->infile)
		return pgpFifoWrite(ff, fifo->filefifo, buf, len);

	written = 0;
	if (fifo->bytefifosize + len <= fifo->maxbytefifosize)
		written = pgpFifoWrite(bf, fifo->bytefifo, buf, len);
	fifo->bytefifosize += written;
	if (written < len) {
		/* switch to file fifo */
		if (xbuf == NULL) {
			xbuf = (PGPByte *)pgpContextMemAlloc( cdkContext,
				kPGPFIFOBufSize * 2, 0);
			if (xbuf == NULL)
				return kPGPError_OutOfMemory;
		}
		/* Try to free up some memory from byte fifo */
		n = pgpFifoRead(bf, fifo->bytefifo, xbuf, sizeof(xbuf));
		fifo->filefifo = pgpFifoCreate( fifo->context, ff);
		if (!fifo->filefifo) {
			if( IsntNull( xbuf ) )
				pgpContextMemFree( cdkContext, xbuf);
			pgpFifoDestroy(bf, fifo->bytefifo);
			return kPGPError_OutOfMemory;
		}
		/* Transfer data from byte to file fifo */
		do {
			pgpAssert (n <= sizeof(xbuf));
			err = (long)pgpFifoWrite(ff, fifo->filefifo, xbuf, n);
			if (err < 0)
			{
				if( IsntNull( xbuf ) )
					pgpContextMemFree( cdkContext, xbuf);
				return (size_t)err;
			}
		} while ((n = pgpFifoRead(bf, fifo->bytefifo, xbuf,
					  sizeof(xbuf))) > 0);
		pgpFifoDestroy(bf, fifo->bytefifo);
		fifo->bytefifo = NULL;
		fifo->bytefifosize = 0;
		fifo->infile = 1;
		written += pgpFifoWrite(ff, fifo->filefifo, buf+written,
					len-written);
	}
	if( IsntNull( xbuf ) )
		pgpContextMemFree( cdkContext, xbuf);
	return written;
}

PGPFifoDesc const pgpFlexFifoDesc = {
	"Flex Byte/File Fifo",
	flexFifoCreate,
	flexFifoRead,
	flexFifoWrite,
	flexFifoPeek,
	flexFifoSeek,
	flexFifoFlush,
	flexFifoDestroy,
	flexFifoSize
};

/*
 * pgpFlexFIFO.c -- Use a memory buffer as a fifo unless it runs out,
 * at which point we switch to a disk file fifo.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpFlexFIFO.c,v 1.4.2.1 1997/06/07 09:50:06 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>      /* For BUFSIZ */

#ifdef HAVE_UNISTD_H
#include <unistd.h>     /* For unlink() */
#endif

#include "pgpDebug.h"
#include "pgpFIFO.h"
#include "pgpErr.h"
#include "pgpMem.h"
#include "pgpUsuals.h"

#if (BUFSIZ < 16384) && (MACINTOSH || WIN32)
#define kPGPFIFOBufSize         16384
#else
#define kPGPFIFOBufSize         BUFSIZ
#endif

/*
 * Size at which we will switch from byte to disk fifos. Will also
 * do so if we run out of memory so set this very large to switch only
 * in that case.
 */
#define MAXBYTEFIFOSIZE (48*1024L)  /* We might want to adjust this */

struct PgpFifoDesc const *bf = &pgpByteFifoDesc;
struct PgpFifoDesc const *ff = &pgpFileFifoDesc;

struct PgpFifoContext {
    struct PgpFifoContext *bytefifo;
    struct PgpFifoContext *filefifo;
    byte infile;
    size_t bytefifosize;
    size_t maxbytefifosize;
};

static void
flexFifoInit(struct PgpFifoContext *fifo)
{
    fifo->bytefifo = NULL;
    fifo->filefifo = NULL;
    fifo->infile = 0;
    fifo->bytefifosize = 0;
    fifo->maxbytefifosize = MAXBYTEFIFOSIZE;
}

static void
flexFifoFlush(struct PgpFifoContext *fifo)
{
    if (fifo->infile)
        pgpFifoFlush(ff, fifo->filefifo);
    else
        pgpFifoFlush(bf, fifo->bytefifo);
    flexFifoInit(fifo);
}

static unsigned long
flexFifoSize(struct PgpFifoContext const *fifo)
{
    if (fifo->infile)
        return pgpFifoSize(ff, fifo->filefifo);
    else
        return pgpFifoSize(bf, fifo->bytefifo);
}

static struct PgpFifoContext *
flexFifoCreate()
{
    struct PgpFifoContext *fifo;

    fifo = (struct PgpFifoContext *)pgpMemAlloc(sizeof(*fifo));
    if (fifo) {
        fifo->infile = 0;
        fifo->filefifo = NULL;
        fifo->bytefifo = pgpFifoCreate(bf);
        if (!fifo->bytefifo) {
            pgpMemFree(fifo);
            return NULL;
        }
        fifo->bytefifosize = 0;
        fifo->maxbytefifosize = MAXBYTEFIFOSIZE;
    }
    return fifo;
}

static void
flexFifoDestroy(struct PgpFifoContext *fifo)
{
    if (fifo->infile)
        pgpFifoDestroy(ff, fifo->filefifo);
    else
        pgpFifoDestroy(bf, fifo->bytefifo);
    pgpMemFree(fifo);
}


static byte const *
flexFifoPeek(struct PgpFifoContext *fifo, unsigned *len)
{
    if (fifo->infile)
        return pgpFifoPeek(ff, fifo->filefifo, len);
    else
        return pgpFifoPeek(bf, fifo->bytefifo, len);
}

static void
flexFifoSeek(struct PgpFifoContext *fifo, unsigned len)
{
    if (fifo->infile)
        pgpFifoSeek(ff, fifo->filefifo, len);
    else {
        pgpFifoSeek(bf, fifo->bytefifo, len);
        fifo->bytefifosize -= len;
    }
}

static size_t
flexFifoRead(struct PgpFifoContext *fifo, byte *buf, size_t len)
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
 * full. Switch to using a file fifo and transfer all data from the
 * byte fifo to the file. Hopefully this will free up enough memory
 * that we can complete.
 *
 * We can also be configured to have a maximum size for the byte fifo
 * and if we exceed that we will switch over.
 */
static size_t
flexFifoWrite(struct PgpFifoContext *fifo, byte const *buf, size_t len)
{
    byte *xbuf = NULL;
    size_t written;
    unsigned long n;
    long err;

    if (fifo->infile)
        return pgpFifoWrite(ff, fifo->filefifo, buf, len);

    written = 0;
    if (fifo->bytefifosize + len <= fifo->maxbytefifosize)
        written = pgpFifoWrite(bf, fifo->bytefifo, buf, len);
    fifo->bytefifosize += written;
    if (written < len) {
        /* switch to file fifo */
        if (xbuf == NULL) {
            xbuf = (byte *)pgpAlloc(kPGPFIFOBufSize * 2);
            if (xbuf == NULL)
                return PGPERR_NOMEM;
        }
        /* Try to free up some memory from byte fifo */
        n = pgpFifoRead(bf, fifo->bytefifo, xbuf, sizeof(xbuf));
        fifo->filefifo = pgpFifoCreate(ff);
        if (!fifo->filefifo) {
            pgpMemFree(xbuf);
            pgpFifoDestroy(bf, fifo->bytefifo);
            return PGPERR_NOMEM;
        }
        /* Transfer data from byte to file fifo */
        do {
            pgpAssert (n <= sizeof(xbuf));
            err = (long)pgpFifoWrite(ff, fifo->filefifo, xbuf, n);
            if (err < 0)
            {
                pgpMemFree(xbuf);
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
    pgpMemFree(xbuf);
    return written;
}

struct PgpFifoDesc const pgpFlexFifoDesc = {
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

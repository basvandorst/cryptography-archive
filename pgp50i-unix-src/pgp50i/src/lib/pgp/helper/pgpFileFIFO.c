/*
 * pgpFileFIFO.c -- Use a disk file as a fifo.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * File grows indefinitely until fifo empties, at which pointers reset
 * to the beginning. (File never shrinks though.)
 * So file size will be the maximum of the number of bytes written since
 * the previous time the fifo was empty.
 *
 * $Id: pgpFileFIFO.c,v 1.7.2.2 1997/06/07 09:50:04 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>        /* For BUFSIZ */

#include "pgpDebug.h"
#include "pgpCFB.h"
#include "pgpCipher.h"
#include "pgpFIFO.h"
#include "pgpMem.h"
#include "pgpUsuals.h"
#include "pgpErr.h"
#include "pgpRndom.h"
#include "pgpFileRef.h"

#if (BUFSIZ < 16384) && (MACINTOSH || WIN32)
#define kPGPFIFOBufSize   16384
#else
#define kPGPFIFOBufSize   BUFSIZ
#endif

/* Use this cipher for encrypting data to disk. */
#define DEFAULTCIPHER    PGP_CIPHER_CAST5

/*
 * putoff is offset in file where next written byte will go
 * getoff is offset in file where next read byte will come from
 * peekoff-getoff is # bytes in buf we have read and decrypted
 */
struct PgpFifoContext {
    FILE *f;
    PGPFileRef *fileRef;
    byte *buf;
    unsigned long putoff, getoff, peekoff;
    struct PgpCfbContext *rdcfb, *wrcfb;
};

static void
fileFifoFlush(struct PgpFifoContext *fifo)
{
    fifo->putoff = 0;
    fifo->getoff = 0;
    fifo->peekoff = 0;
}

static unsigned long
fileFifoSize(struct PgpFifoContext const *fifo)
{
    return (unsigned long)(fifo->putoff - fifo->getoff);
}

static void
fileFifoFreeCfbs(struct PgpFifoContext *fifo)
{
    pgpCfbDestroy(fifo->rdcfb);
    pgpCfbDestroy(fifo->wrcfb);
}

/* Return negative on error, 0 on success */
static int
fileFifoInitCfbs(struct PgpFifoContext *fifo)
{
    struct PgpRandomContext *rc;
    struct PgpCipher const *cipher;
    unsigned cfbkeysize;
    byte *cfbkey;
    byte cfbiv[PGP_CFB_MAXBLOCKSIZE];

    /* XXX Interface doesn't provide env to choose dflt cipher */
    cipher = pgpCipherByNumber(DEFAULTCIPHER);
    if (!cipher) {
        return PGPERR_GENERIC;
    }
    cfbkeysize = cipher->keysize;
    cfbkey = pgpMemAlloc(cfbkeysize);
    if (!cfbkey) {
        return PGPERR_GENERIC;
    }
    rc = pgpRandomCreate();
    pgpAssert(rc);
    pgpRandomGetBytes(rc, cfbkey, cfbkeysize);
    pgpRandomGetBytes(rc, cfbiv, sizeof(cfbiv));
    pgpRandomDestroy(rc);
    fifo->rdcfb = pgpCfbCreate(cipher);
    pgpCfbInit(fifo->rdcfb, cfbkey, cfbiv);
    pgpClearMemory(cfbkey, cfbkeysize);
    pgpClearMemory(cfbiv, sizeof(cfbiv));
    pgpMemFree(cfbkey);
    fifo->wrcfb = pgpCfbCopy(fifo->rdcfb);
    if (!fifo->wrcfb) {
        fileFifoFreeCfbs(fifo);
        return PGPERR_GENERIC;
    }
    return 0;
}

static struct PgpFifoContext *
fileFifoCreate()
{
    struct PgpFifoContext *fifo;

    fifo = (struct PgpFifoContext *)pgpMemAlloc(sizeof(*fifo));
    if (!fifo)
        return NULL;
    fifo->buf = (byte *)pgpMemAlloc(kPGPFIFOBufSize);
    if (!fifo->buf) {
        pgpMemFree(fifo);
        return NULL;
    }
    if (fileFifoInitCfbs(fifo) < 0) {
        pgpMemFree(fifo->buf);
        pgpMemFree(fifo);
        return NULL;
    }
    fifo->f = pgpStdIOOpenTempFile(&fifo->fileRef, NULL);
    if (!fifo->f) {
        fileFifoFreeCfbs(fifo);
        pgpMemFree(fifo->buf);
        pgpMemFree(fifo);
        return NULL;
    }
    fifo->putoff = 0;
    fifo->getoff = 0;
    fifo->peekoff = 0;

    return fifo;
}

static void
fileFifoDestroy(struct PgpFifoContext *fifo)
{
    fclose(fifo->f);
    if (fifo->fileRef != NULL)
    {
        pgpDeleteFile(fifo->fileRef);
        pgpFreeFileRef(fifo->fileRef);
    }
    fileFifoFreeCfbs(fifo);
    pgpClearMemory(fifo->buf, kPGPFIFOBufSize);
    pgpMemFree(fifo->buf);
    pgpMemFree(fifo);
}


/* This could definitely use some optimizing */
static byte const *
fileFifoPeek(struct PgpFifoContext *fifo, unsigned *len)
{
    unsigned dsklen;

    if (fifo->putoff == fifo->getoff) {
        *len = 0;
        return NULL;
    }

    if (fifo->peekoff > fifo->getoff) {
        /* Have peeked data in buffer already */
        *len = fifo->peekoff - fifo->getoff;
        return fifo->buf;
    }

    if (fifo->putoff - fifo->getoff > kPGPFIFOBufSize)
        dsklen = kPGPFIFOBufSize;
    else
        dsklen = (unsigned)(fifo->putoff - fifo->getoff);

    if (fseek(fifo->f, fifo->getoff, SEEK_SET) != 0) {
        *len = 0;
        return NULL;
    }

    if (fread(fifo->buf, 1, dsklen, fifo->f) != dsklen) {
        *len = 0;
        return NULL;
    }

    pgpCfbDecrypt(fifo->rdcfb, fifo->buf, fifo->buf, dsklen);
    fifo->peekoff = fifo->getoff + dsklen;

    *len = dsklen;
    return fifo->buf;
}

static void
fileFifoSeek(struct PgpFifoContext *fifo, unsigned len)
{
  if (!len)
    return;

  pgpAssert(fifo->putoff - fifo->getoff >= (long)len);
  pgpAssert(fifo->peekoff - fifo->getoff >= (long)len);
  pgpAssert(fifo->putoff >= fifo->peekoff);

  fifo->getoff += len;
  if (fifo->getoff < fifo->peekoff) {
    /* Move data in fifo down */
    pgpCopyMemory(fifo->buf+len, fifo->buf, fifo->peekoff-fifo->getoff);
  }

  /* If fifo becomes empty, reset pointers to beginning of file */
  if (fifo->putoff == fifo->getoff)
      fifo->putoff = fifo->getoff = fifo->peekoff = 0;
}

static size_t
fileFifoWrite(struct PgpFifoContext *fifo, byte const *buf, size_t len)
{
    size_t lenleft;
    size_t buflen;

    if (fseek(fifo->f, fifo->putoff, SEEK_SET) != 0)
        return PGPERR_FILEFIFO_SEEK;
    lenleft = len;
    while (lenleft) {
        buflen = (lenleft < kPGPFIFOBufSize) ? lenleft : kPGPFIFOBufSize;
        pgpCfbEncrypt(fifo->wrcfb, buf, fifo->buf, buflen);
        if (fwrite(fifo->buf, 1, buflen, fifo->f) != buflen)
            return PGPERR_FILEFIFO_WRITE;
        buf += buflen;
        lenleft -= buflen;
        fifo->putoff += buflen;
    }
    return len;
}

static size_t
fileFifoRead(struct PgpFifoContext *fifo, byte *buf, size_t len)
{
    size_t avail;

    /* First get data out of peekahead buffer if any */
    if (fifo->peekoff > fifo->getoff) {
        avail = fifo->peekoff - fifo->getoff;
        if (avail > len)
            avail = len;
        pgpCopyMemory(fifo->buf, buf, avail);
        len -= avail;
        buf += avail;
        fifo->getoff += avail;
        if (fifo->peekoff > fifo->getoff) {
            /* Move data in fifo down */
            pgpCopyMemory(fifo->buf+avail, fifo->buf,
                   fifo->peekoff-fifo->getoff);
        }
        if (len == 0)
            return avail;
    }

    avail = fifo->putoff - fifo->getoff;
    if (avail > len)
        avail = len;

    if (fseek(fifo->f, fifo->getoff, SEEK_SET) != 0)
        return PGPERR_FILEFIFO_SEEK;
    if (fread(buf, 1, avail, fifo->f) != avail)
        return PGPERR_FILEFIFO_READ;

    pgpCfbDecrypt(fifo->rdcfb, buf, buf, avail);

    fifo->getoff += avail;
    if (fifo->putoff == fifo->getoff)
        fifo->putoff = fifo->getoff = 0;

    return avail;
}

struct PgpFifoDesc const pgpFileFifoDesc = {
    "File Fifo",
    fileFifoCreate,
    fileFifoRead,
    fileFifoWrite,
    fileFifoPeek,
    fileFifoSeek,
    fileFifoFlush,
    fileFifoDestroy,
    fileFifoSize
};


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

/*
 * pgpByteFIFO.h -- A first-in-first-out pipe of bytes.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpByteFIFO.h,v 1.2.2.1 1997/06/07 09:50:00 mhw Exp $
 */

#ifndef PGPBYTEFIFO_H
#define PGPBYTEFIFO_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpFifoContext;

struct PgpFifoContext *byteFifoCreate (void);
void byteFifoDestroy (struct PgpFifoContext *fifo);

byte *byteFifoGetSpace (struct PgpFifoContext *fifo, unsigned *len);
void byteFifoSkipSpace (struct PgpFifoContext *fifo, unsigned len);

byte const *byteFifoPeek (struct PgpFifoContext *fifo, unsigned *len);
void byteFifoSeek (struct PgpFifoContext *fifo, unsigned len);

size_t byteFifoWrite (struct PgpFifoContext *fifo, byte const *buf,
                      size_t len);
size_t byteFifoRead (struct PgpFifoContext *fifo, byte *buf, size_t len);

void byteFifoFlush(struct PgpFifoContext *fifo);
unsigned long byteFifoSize (struct PgpFifoContext const *fifo);

#ifdef __cplusplus
}
#endif

#endif /* PGPBYTEFIFO_H */

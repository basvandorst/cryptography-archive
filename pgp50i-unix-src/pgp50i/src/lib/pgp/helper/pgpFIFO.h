/*
 * pgpFIFO.h -- A general interface to a First-In First-Out system used
 * by PGP. There can be many FIFO implementations, and this is the
 * interface that each must support.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpFIFO.h,v 1.3.2.1 1997/06/07 09:50:03 mhw Exp $
 */

#ifndef PGPFIFO_H
#define PGPFIFO_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpFifoContext;
#ifndef TYPE_PGPFIFOCONTEXT
#define TYPE_PGPFIFOCONTEXT 1
typedef struct PgpFifoContext PgpFifoContext;
#endif

struct PgpFifoDesc {
        char const *name;
        struct PgpFifoContext * (*create) (void);

        size_t (*read) (struct PgpFifoContext *fifo, byte *buf, size_t len);
        size_t (*write) (struct PgpFifoContext *fifo, byte const *buf,
                         size_t len);

        byte const * (*peek) (struct PgpFifoContext *fifo, unsigned *len);
        void (*seek) (struct PgpFifoContext *fifo, unsigned len);

        void (*flush) (struct PgpFifoContext *fifo);
        void (*destroy) (struct PgpFifoContext *fifo);

        unsigned long (*size) (struct PgpFifoContext const *fifo);
};
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
#endif

#define pgpFifoCreate(fd) (fd)->create()
#define pgpFifoRead(fd,f,b,l) (fd)->read(f,b,l)
#define pgpFifoWrite(fd,f,b,l) (fd)->write(f,b,l)
#define pgpFifoPeek(fd,f,l) (fd)->peek(f,l)
#define pgpFifoSeek(fd,f,l) (fd)->seek(f,l)
#define pgpFifoFlush(fd,f) (fd)->flush(f)
#define pgpFifoDestroy(fd,f) (fd)->destroy(f)
#define pgpFifoSize(fd,f) (fd)->size(f)

extern struct PgpFifoDesc const PGPExport pgpByteFifoDesc;
extern struct PgpFifoDesc const PGPExport pgpFileFifoDesc;
extern struct PgpFifoDesc const PGPExport pgpFlexFifoDesc;

#ifdef __cplusplus
}
#endif

#endif /* PGPFIFO_H */

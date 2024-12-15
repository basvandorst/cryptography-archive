/*
 * pgpFIFO.c -- This is just an abstraction on top of the bytefifo.
 * Yes, I am forcing the issue. I don't care, since the bytefifo
 * is going to be there anyways, so I might as well put this code
 * here.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpFIFO.c,v 1.1.2.1 1997/06/07 09:50:02 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpByteFIFO.h"
#include "pgpFIFO.h"

struct PgpFifoDesc const pgpByteFifoDesc = {
        "Byte Fifo",
        byteFifoCreate,
        byteFifoRead,
        byteFifoWrite,
        byteFifoPeek,
        byteFifoSeek,
        byteFifoFlush,
        byteFifoDestroy,
        byteFifoSize
};

/*
 * pgpFIFO.c -- This is just an abstraction on top of the bytefifo.
 * Yes, I am forcing the issue.  I don't care, since the bytefifo
 * is going to be there anyways, so I might as well put this code
 * here.
 *
 * $Id: pgpFIFO.c,v 1.4 1997/06/25 19:41:06 lloyd Exp $
 */

#include "pgpConfig.h"

#include "pgpByteFIFO.h"
#include "pgpFIFO.h"

PGPFifoDesc const pgpByteFifoDesc = {
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

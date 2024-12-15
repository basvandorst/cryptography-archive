/*
 * pgpByteFIFO.h -- A first-in-first-out pipe of bytes.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpByteFIFO.h,v 1.11.4.1 1998/09/14 21:48:20 hal Exp $
 */

#ifndef Included_pgpByteFIFO_h
#define Included_pgpByteFIFO_h

#include "pgpPubTypes.h"
#include "pgpOpaqueStructs.h"
#include "pgpMemoryMgr.h"

PGP_BEGIN_C_DECLARATIONS


PGPFifoContext *byteFifoCreate ( PGPContextRef context );
PGPFifoContext *byteFifoCreateSecure ( PGPContextRef context );
void byteFifoDestroy (PGPFifoContext *fifo);

PGPByte *byteFifoGetSpace (PGPFifoContext *fifo, PGPSize *len);
void byteFifoSkipSpace (PGPFifoContext *fifo, PGPSize len);

PGPByte const *byteFifoPeek (PGPFifoContext *fifo, PGPSize *len);
void byteFifoSeek (PGPFifoContext *fifo, PGPSize len);

PGPSize byteFifoWrite (PGPFifoContext *fifo, PGPByte const *buf,
		      PGPSize len);
PGPSize byteFifoRead (PGPFifoContext *fifo, PGPByte *buf, PGPSize len);

void byteFifoFlush(PGPFifoContext *fifo);
PGPSize byteFifoSize (PGPFifoContext const *fifo);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpByteFIFO_h */

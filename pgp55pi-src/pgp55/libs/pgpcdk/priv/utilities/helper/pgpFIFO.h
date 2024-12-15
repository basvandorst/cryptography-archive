/*
 * pgpFIFO.h -- A general interface to a First-In First-Out system used
 * by PGP.  There can be many FIFO implementations, and this is the
 * interface that each must support.
 *
 * $Id: pgpFIFO.h,v 1.12 1997/08/27 02:50:43 lloyd Exp $
 */

#ifndef Included_pgpFIFO_h
#define Included_pgpFIFO_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

struct PGPFifoDesc
{
	char const *		name;
	PGPFifoContext *	(*create) ( PGPContextRef context );

	PGPSize (*read)		(PGPFifoContext *fifo, PGPByte *buf, PGPSize len);
	PGPSize (*write)	(PGPFifoContext *fifo, PGPByte const *buf,
							 PGPSize len);

	PGPByte const *		(*peek) (PGPFifoContext *fifo, PGPSize *len);
	void (*seek)		(PGPFifoContext *fifo, PGPSize len);

	void (*flush)		(PGPFifoContext *fifo);
	void (*destroy)		(PGPFifoContext *fifo);

	PGPSize				(*size)	(PGPFifoContext const *fifo);
};


#define pgpFifoCreate(context, fd) (fd)->create( context )
#define pgpFifoRead(fd,f,b,l) (fd)->read(f,b,l)
#define pgpFifoWrite(fd,f,b,l) (fd)->write(f,b,l)
#define pgpFifoPeek(fd,f,l) (fd)->peek(f,l)
#define pgpFifoSeek(fd,f,l) (fd)->seek(f,l)
#define pgpFifoFlush(fd,f) (fd)->flush(f)
#define pgpFifoDestroy(fd,f) (fd)->destroy(f)
#define pgpFifoSize(fd,f) (fd)->size(f)

extern PGPFifoDesc const  pgpByteFifoDesc;
extern PGPFifoDesc const  pgpFileFifoDesc;
extern PGPFifoDesc const  pgpFlexFifoDesc;

PGP_END_C_DECLARATIONS

#endif /* Included_pgpFIFO_h */
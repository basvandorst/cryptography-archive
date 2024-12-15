/*
 * pgpJoin.h -- Append multiple streams together into a single output, and
 * buffer as needed
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpJoin.h,v 1.9 1997/06/25 19:40:24 lloyd Exp $
 */

/*
 * A join module will combine multiple inputs into a single output.
 * It will first take all the data from the first input (the input
 * from the creation function) until it gets a SizeAdvise(0).  Then it
 * will start reading from the next input in line, until it is done,
 * and so on down the list of inputs.
 *
 * If an input pipeline that is not the current writer tries to write
 * data, it will be buffered in that input's fifo.
 *
 * Data that is buffered will be flushed out when that input is
 * allowed to write, which is when all previous writers have sent
 * a sizeAdvise(0).
 */

#ifndef Included_pgpJoin_h
#define Included_pgpJoin_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

/*
 * Create a join module.  The caller needs to save the head pointer
 * for later use.  */
PGPPipeline  **pgpJoinCreate (PGPContextRef cdkContext, PGPPipeline **head,
				    PGPFifoDesc const *fd);

/*
 * Append a new input to the join module.  This will insert the
 * new pipeline just after the pointer to the head passed in.  This
 * function returns a pointer to the join module associated with
 * myself input.
 */
PGPPipeline  *pgpJoinAppend ( PGPPipeline *head);

/*
 * Add data to the buffer in the pipeline module passed in.  This
 * data will be flushed out later.
 */
size_t  pgpJoinBuffer (PGPPipeline *myself, PGPByte const *buf,
	size_t len);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpJoin_h */

/*
* pgpJoin.h -- Append multiple streams together into a single output, and
* buffer as needed
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpJoin.h,v 1.4.2.1 1997/06/07 09:51:16 mhw Exp $
*/

/*
* A join module will combine multiple inputs into a single output.
* It will first take all the data from the first input (the input
* from the creation function) until it gets a SizeAdvise(0). Then it
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

#ifndef PGPJOIN_H
#define PGPJOIN_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpFifoDesc;
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
#endif

/*
* Create a join module. The caller needs to save the head pointer
* for later use. */
struct PgpPipeline PGPExport **pgpJoinCreate (struct PgpPipeline **head,
				struct PgpFifoDesc const *fd);

/*
* Append a new input to the join module. This will insert the
* new pipeline just after the pointer to the head passed in. This
* function returns a pointer to the join module associated with
* myself input.
*/
struct PgpPipeline PGPExport *pgpJoinAppend (struct PgpPipeline *head);

/*
* Add data to the buffer in the pipeline module passed in. This
* data will be flushed out later.
*/
size_t PGPExport pgpJoinBuffer (struct PgpPipeline *myself, byte const *buf,
	size_t len);

#ifdef __cplusplus
}
#endif

#endif /* PGPJOIN_H */

/*
 * pgpFileMod.h -- Pipeline Module to (read and) write files.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpFileMod.h,v 1.15 1997/12/11 23:12:35 lloyd Exp $
 */

#ifndef Included_pgpFileMod_h
#define Included_pgpFileMod_h

#include <stdio.h>

#include "pgpUsuals.h"
#include "pgpTypes.h"
#include "pgpErrors.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpMemoryMgr.h"


typedef PGPError (*PGPFileReadCallBack)(void *arg, PGPFileOffset soFar,
										PGPFileOffset total);

/*
 * Create a module to write into the appropriate file.  Will perform a
 * fileClose() is close is non-zero when a sizeAdvise(0) is received.
 */
PGPPipeline  *
pgpFileWriteCreate ( PGPContextRef context,
	PGPPipeline **head, PGPFile *file, int close_at_eof);

/*
 * Create a context that is used to pump a file through a pipeline.
 * It will close the file when it reaches the end if close is non-zero
 */
PGPFileRead  *pgpFileReadCreate ( PGPContextRef context,
				FILE *stdioFile, int close_at_eof);
PGPFileRead  *pgpPGPFileReadCreate ( PGPContextRef context,
				PGPFile *input, int close_at_eof);

void  pgpFileReadSetCallBack (PGPFileRead *context,
	PGPFileReadCallBack callBack, void *callBackArg);

/*
 * Forcibly close a FileRead Context.  This is only necessary if you
 * need to cause the fileread module to close itself forcibly, rather
 * than waiting for the end.  Basically it just sends an end annotation
 */
PGPError  pgpFileReadClose (PGPFileRead *context,
	PGPPipeline *head);

/*
 * Destroy a FileReadContext.  Close the file if necessary
 */
void  pgpFileReadDestroy (PGPFileRead *context);

/* 
 * pgpFileReadPump takes the context and pipes the file down the pipeline
 * until it hits an error.  It then returns the error.  It returns 0
 * when it reaches the end of the file and successfully sends its data.
 */
PGPError  pgpFileReadPump (PGPFileRead *context,
	PGPPipeline *head);

/*
 * Peek at the input buffer of a PGPFileRead object.  This is useful to
 * check if it is a text or binary file, or to use the file contents
 * to obtain some amount of entropy.  This will return a pointer to
 * the buffer, and will fill in len with the amount of data in the
 * buffer.
 */
PGPByte const  *pgpFileReadPeek (PGPFileRead *context,
	PGPSize *len);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpFileMod_h */

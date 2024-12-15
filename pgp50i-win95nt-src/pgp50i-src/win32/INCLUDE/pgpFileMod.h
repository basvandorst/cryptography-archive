/*
* pgpFileMod.h -- Pipeline Module to (read and) write files.
*
* Copyright (C) 1993-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* This is a Public API Function Header.
*
* $Id: pgpFileMod.h,v 1.6.2.2 1997/06/07 09:50:56 mhw Exp $
*/

#ifndef PGPFILEMOD_H
#define PGPFILEMOD_H

#include "pgpUsuals.h"
#include "pgpTypes.h"
#include "pgpErr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpFileRead;
#ifndef TYPE_PGPFILEREAD
#define TYPE_PGPFILEREAD 1
typedef struct PgpFileRead PgpFileRead;
#endif

struct PgpFile;
#ifndef TYPE_PGPFILE
#define TYPE_PGPFILE 1
typedef struct PgpFile PgpFile;
#endif

typedef PGPError (*PGPFileReadCallBack)(void *arg, size_t soFar,
					size_t total);

/*
* Create a module to write into the appropriate file. Will perform a
* fileClose() is close is non-zero when a sizeAdvise(0) is received.
*/
struct PgpPipeline PGPExport *
pgpFileWriteCreate (struct PgpPipeline **head, struct PgpFile *file,
		int close_at_eof);

/*
* Create a context that is used to pump a file through a pipeline.
* It will close the file when it reaches the end if close is non-zero
*/
struct PgpFileRead PGPExport *pgpFileReadCreate (FILE *stdioFile,
	int close_at_eof);
struct PgpFileRead PGPExport *pgpPGPFileReadCreate (PgpFile *input,
	int close_at_eof);

void PGPExport pgpFileReadSetCallBack (PgpFileRead *context,
	PGPFileReadCallBack callBack, void *callBackArg);

/*
* Forcibly close a FileRead Context. This is only necessary if you
* need to cause the fileread module to close itself forcibly, rather
* than waiting for the end. Basically it just sends an end annotation
*/
int PGPExport pgpFileReadClose (struct PgpFileRead *context,
	struct PgpPipeline *head);

/*
* Destroy a FileReadContext. Close the file if necessary
*/
void PGPExport pgpFileReadDestroy (struct PgpFileRead *context);

/*
* pgpFileReadPump takes the context and pipes the file down the pipeline
* until it hits an error. It then returns the error. It returns 0
* when it reaches the end of the file and successfully sends its data.
*/
int PGPExport pgpFileReadPump (struct PgpFileRead *context,
	struct PgpPipeline *head);

/*
* Peek at the input buffer of a PgpFileRead object. This is useful to
* check if it is a text or binary file, or to use the file contents
* to obtain some amount of entropy. This will return a pointer to
* the buffer, and will fill in len with the amount of data in the
* buffer.
*/
byte const PGPExport *pgpFileReadPeek (struct PgpFileRead *context,
	size_t *len);

#ifdef __cplusplus
}
#endif

#endif /* PGPFILEMOD_H */

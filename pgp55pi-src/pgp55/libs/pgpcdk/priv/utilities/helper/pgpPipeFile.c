/*
 * pgpPipeFile.c -- a PGPFile interface to write to Pipelines
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPipeFile.c,v 1.18 1997/10/06 18:01:36 lloyd Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>

#include "pgpFile.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpErrors.h"
#include "pgpContext.h"

typedef struct File {
	PGPFileError err;
	PGPPipeline *head;
	PGPError	error;
	DEBUG_STRUCT_CONSTRUCTOR( File )
} File;

static void
setError (PGPFile *file, PGPError code)
{
	File *fp = (File *)file->priv;

	fp->err.f = file;
	fp->err.error = code;
	fp->error = code;
}

static size_t
pipeRead (void *ptr, size_t size, PGPFile *file)
{
	(void) ptr;
	(void) size;
	setError (file, kPGPError_IllegalFileOp);
	return 0;
}

static size_t
pipeWrite (void const *ptr, size_t size, PGPFile *file)
{
	File *fp = (File *) file->priv;
	PGPError code = kPGPError_NoErr;

	size = fp->head->write (fp->head, (PGPByte const *)ptr, size, &code);
	setError (file, code);
	return size;
}

static PGPError
pipeFlush (PGPFile *file)
{
	File *fp = (File *) file->priv;
	PGPError	code;

	code = fp->head->flush (fp->head);
	setError (file, code);
	return code;
}

static PGPError
pipeClose (PGPFile *file)
{
	File *fp = (File *) file->priv;
	PGPError code = kPGPError_NoErr;
	PGPContextRef	cdkContext;

	pgpAssertAddrValid( file, PGPFile );
	cdkContext	= file->context;

	code = fp->head->sizeAdvise (fp->head, 0);
	setError (file, code);
	if (code)
		return code;

	fp->head->teardown (fp->head);

	pgpClearMemory( fp,  sizeof (*fp));
	pgpContextMemFree( cdkContext, fp);
	pgpClearMemory( file,  sizeof (*file));
	pgpContextMemFree( cdkContext, file);
	return kPGPError_NoErr;
}

static long
pipeTell (PGPFile *file)
{
	(void) file;
	return 0;
}

static PGPError
pipeSeek (PGPFile *file, long offset, int whence)
{
	(void) file;
	(void) offset;
	(void) whence;

	return kPGPError_IllegalFileOp;
}

static PGPBoolean
pipeEof (PGPFile const *file)
{
	(void) file;
	return FALSE;
}

static PGPFileError const *
pipeError (PGPFile const *file)
{
	File *fp = (File *) file->priv;

	if (fp->error)
		return &(fp->err);
	return NULL;
}

static void
pipeClearError (PGPFile *file)
{
	File *fp = (File *) file->priv;

	fp->error = kPGPError_NoErr;
}

static PGPError
pipeWrite2read (PGPFile *file)
{
	(void) file;
	return kPGPError_IllegalFileOp;
}

static PGPCFBContext *
pipeCfb (PGPFile const *file)
{
	(void) file;
	return NULL;
}

PGPFile *
pgpFilePipelineOpen (PGPPipeline *mod)
{
	PGPFile *fp;
	File *ffp;
	PGPContextRef		cdkContext;

	pgpAssertAddrValid( mod, PGPPipeline );
	cdkContext	= mod->cdkContext;
	
	if (!mod)
		return NULL;

	fp = (PGPFile *)pgpContextMemAlloc( cdkContext,
		sizeof (*fp), kPGPMemoryFlags_Clear);
	if (!fp)
		return NULL;

	ffp = (File *)pgpContextMemAlloc( cdkContext,
		sizeof (*ffp), kPGPMemoryFlags_Clear);
	if (!ffp) {
		pgpContextMemFree( cdkContext, fp);
		return NULL;
	}

	ffp->head = mod;

	fp->priv = ffp;
	fp->read = pipeRead;
	fp->write = pipeWrite;
	fp->flush = pipeFlush;
	fp->close = pipeClose;
	fp->tell = pipeTell;
	fp->seek = pipeSeek;
	fp->eof = pipeEof;
	fp->error = pipeError;
	fp->clearError = pipeClearError;
	fp->write2read = pipeWrite2read;
	fp->cfb = pipeCfb;
	fp->context = cdkContext;

	return fp;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

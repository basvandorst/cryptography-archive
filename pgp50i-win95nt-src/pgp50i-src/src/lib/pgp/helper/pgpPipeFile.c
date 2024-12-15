/*
* pgpPipeFile.c -- a PgpFile interface to write to Pipelines
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpPipeFile.c,v 1.2.2.1 1997/06/07 09:50:12 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpFile.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpErr.h"
#include "pgpUsuals.h"

struct File {
	struct PgpFileError err;
	struct PgpPipeline *head;
	int error;
};

static void
setError (struct PgpFile *file, int code)
{
	struct File *fp = (struct File *)file->priv;

 fp->err.f = file;
	fp->err.error = code;
	fp->error = code;
}

static size_t
pipeRead (void *ptr, size_t size, struct PgpFile *file)
	{
			(void) ptr;
			(void) size;
			setError (file, PGPERR_FILE_BADOP);
			return 0;
	}

static size_t
pipeWrite (void const *ptr, size_t size, struct PgpFile *file)
	{
			struct File *fp = (struct File *) file->priv;
			int code = 0;

size = fp->head->write (fp->head, (byte const *)ptr, size, &code);
setError (file, code);
return size;
}

static int
pipeFlush (struct PgpFile *file)
	{
			struct File *fp = (struct File *) file->priv;
			int code;

			code = fp->head->flush (fp->head);
			setError (file, code);
			return code;
	}

static int
pipeClose (struct PgpFile *file)
	{
			struct File *fp = (struct File *) file->priv;
			int code = 0;

			code = fp->head->sizeAdvise (fp->head, 0);
			setError (file, code);
			if (code)
				 return code;

			fp->head->teardown (fp->head);

			memset (fp, 0, sizeof (*fp));
			pgpMemFree (fp);
			memset (file, 0, sizeof (*file));
			pgpMemFree (file);
		return 0;
	}

static long
pipeTell (struct PgpFile *file)
{
	(void) file;
	return 0;
}

static int
pipeSeek (struct PgpFile *file, long offset, int whence)
	{
			(void) file;
			(void) offset;
			(void) whence;

			return PGPERR_FILE_BADOP;
	}

static int
pipeEof (struct PgpFile const *file)
{
	(void) file;
	return 0;
}

static struct PgpFileError const *
pipeError (struct PgpFile const *file)
{
	struct File *fp = (struct File *) file->priv;

	if (fp->error)
		return &(fp->err);
return NULL;
}

static void
pipeClearError (struct PgpFile *file)
{
	struct File *fp = (struct File *) file->priv;

	fp->error = 0;
}

static int
pipeWrite2read (struct PgpFile *file)
{
	(void) file;
	return PGPERR_FILE_BADOP;
}

static struct PgpCfbContext *
pipeCfb (struct PgpFile const *file)
{
	(void) file;
 return NULL;
}

struct PgpFile *
pgpFilePipelineOpen (struct PgpPipeline *mod)
	{
			struct PgpFile *fp;
			struct File *ffp;

			if (!mod)
				 return NULL;

fp = (struct PgpFile *)pgpMemAlloc (sizeof (*fp));
if (!fp)
	return NULL;

ffp = (struct File *)pgpMemAlloc (sizeof (*ffp));
if (!ffp) {
	pgpMemFree (fp);
	return NULL;
}

			memset (ffp, 0, sizeof (*ffp));
			memset (fp, 0, sizeof (*fp));

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

			return fp;
}

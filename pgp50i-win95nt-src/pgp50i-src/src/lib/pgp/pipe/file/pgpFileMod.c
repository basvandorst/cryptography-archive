/*
 * pgpFileMod.c -- File I/O modules.
 *
 * Copyright (C) 1993-1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFileMod.c,v 1.9.2.3 1997/06/24 13:26:08 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef MACINTOSH
#include <unix.h>
#include <stat.h>
#else
#include <sys/types.h>		/* required for Ultrix */
#include <sys/stat.h>
#endif

#include "pgpDebug.h"
#include "pgpFileMod.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpFile.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"

#include "pgpKludge.h"

#define FILEMODMAGIC	0xfeedf11e

struct FileModContext {
	struct PgpFile *file;
	int closeFlag;
	int in_newfile;
	int scope_depth;
};

/*
* If we have a file, flush it. Then, if we are allowed to close
* it, then do so. If we successfully closed it, then set the file
* pointer to 0. Return from any error with the error number.
*/
static int
fileModClose (struct FileModContext *context)
{
	int error = 0;

	if (context->file) {
		error = pgpFileFlush (context->file);
		if (error)
			return error;

		if (context->closeFlag) {
			error = pgpFileClose (context->file);
			if (error)
				return error;
			context->file = 0;
		}
	}

	return 0;
}

static int
Flush (struct PgpPipeline *myself)
{
	struct FileModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (struct FileModContext *)myself->priv;
	pgpAssert (context);

	if (!context->file)
		return 0;

	return pgpFileFlush (context->file);
}

static size_t
fileWrite (struct PgpPipeline *myself, byte const *buffer, size_t size,
	int *error)
{
	struct FileModContext *context;
	size_t retval;

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (struct FileModContext *)myself->priv;
	pgpAssert (context);

	if (!context->file) {
		if (error)
			*error = PGPERR_NO_FILE; /* No such file or directory*/
		return 0;
	}

	retval = pgpFileWrite (buffer, size, context->file);
	if (retval) {
		if (error)
			*error = 0;
		return retval;
	}
	if (error && pgpFileError (context->file))
		*error = PGPERR_FILE_OPFAIL;
	pgpFileClearError (context->file);
	return 0;
}

/*
* We accept certain annotations to tell us that we should change
* the file we are writing. If we have an open file, then a change
* will either:
*	1) Return an error (if context->closeFlag != 0), or
*	2) Be ignored (if context->closeFlag == 0)
*
* For the end context, close the file (similar to sizeAdvise) and
* return the error we get.
*/
static int
fileAnnotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
	byte const *string, size_t size)
{
	struct FileModContext *context;
	FILE *fp;
	char filename[FILENAME_MAX];
	size_t len;
	int error;

	(void)origin;	 /* Not used */

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (struct FileModContext *)myself->priv;
	pgpAssert (context);

	switch (type) {
	case PGPANN_NEWFILE_START:
		pgpAssert(!context->file || !context->closeFlag);

		if (context->file) {
			error = fileModClose (context);
			if (error)
				return error;
		}

		pgpAssert (!context->file);
		{
			struct PgpFile *pgpf;

			len = min (size, sizeof (filename) - 1);
			memcpy (filename, string, len);
			filename[len] = '\0';
			
			/* I need to know if this should be binary or not */
			fp = fopen (filename, "wb");
			if (!fp)
				return PGPERR_NO_FILE;
			pgpf = pgpFileWriteOpen (fp, NULL);
			if (!pgpf) {
				fclose (fp);
				return PGPERR_NOMEM;
			}
			context->file = pgpf;
			context->closeFlag = 1;
		}
		context->in_newfile = 1;
		break;
	default:
		;		 /* do nothing */
	}
		

	PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);

	return 0;
}

static int
fileSizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
{
	struct FileModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (struct FileModContext *)myself->priv;
	pgpAssert (context);

	if (bytes || context->scope_depth)
		return 0;
	if (context->in_newfile && !context->closeFlag)
		return 0;

	return fileModClose (context);
}

static void
fileTeardown (struct PgpPipeline *myself)
{
	struct FileModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (struct FileModContext *)myself->priv;
	pgpAssert (context);
	fileModClose (context);

		
	memset (context, 0, sizeof (*context));
	pgpMemFree (context);
	memset (myself, 0, sizeof (*myself));
	pgpMemFree (myself);
}

struct PgpPipeline *
pgpFileWriteCreate (struct PgpPipeline **head, struct PgpFile *file,
			int closeFlag)
{
	struct PgpPipeline *mod;
	struct FileModContext *context;

	if (!head)
		return NULL;

	*head = 0;

	context = (struct FileModContext *)pgpMemAlloc (sizeof(*context));
	if (!context)
		return NULL;
	mod = (struct PgpPipeline *)pgpMemAlloc (sizeof(*mod));
	if (!mod) {
		pgpMemFree(context);
		return NULL;
	}

	mod->magic = FILEMODMAGIC;
	mod->write = fileWrite;
	mod->flush = Flush;
	mod->sizeAdvise = fileSizeAdvise;
	mod->annotate = fileAnnotate;
	mod->teardown = fileTeardown;
	mod->name = "Write file";
	mod->priv = context;

	memset (context, 0, sizeof (*context));
	context->file = file;
	context->closeFlag = closeFlag;

	*head = mod;
	return mod;
}

#if (BUFSIZ < 16384) && (MACINTOSH || WIN32)
#define kPGPReadBufSize		16384
#else
#define kPGPReadBufSize		BUFSIZ
#endif

#if 0
/* Enable this for debugging - it stresses the hell out of the pipeline */
#undef kPGPReadBufSize
#define kPGPReadBufSize 1
#endif

struct PgpFileRead
{
	PgpPipeline			mod;
	PgpFile *	 		 file;
	byte				buffer[kPGPReadBufSize];
	byte *				p;
	off_t	 		 	filesize;
	off_t	 		 	sizeleft;
	size_t				len;
	PGPFileReadCallBack	callBack;
	void *				callBackArg;
	int					annsent;
	int					sizesent;
	int					closeFlag;
};

struct PgpFileRead *
pgpFileReadCreate (FILE *stdioFile, int closeFlag)
{
			/*
	* XXX Potential space leak if <closeFlag> is false, although it's
	* never called that way from anywhere in PGP 5.0
	 *(BAT 19970621 - Except when it is, in pgp.c)
			*/
	return pgpPGPFileReadCreate (pgpFileReadOpen (stdioFile, NULL, NULL),
								closeFlag);
}

struct PgpFileRead *
pgpPGPFileReadCreate (PgpFile *input, int closeFlag)
{
	struct PgpFileRead *context;
	long sizeAdvise;

	pgpAssert (input);

	context = (struct PgpFileRead *)pgpMemAlloc (sizeof (*context));
	if (!context)
		return NULL;
	
	sizeAdvise = pgpFileSizeAdvise (input);

	memset (context, 0, sizeof (*context));
	context->file = input;
	context->p = context->buffer;
	if (sizeAdvise < 0)
	{
		/* Dont trust the size if its not a regular file */
		context->filesize = 0;
		context->sizesent = -1;
	}
	else
	{
		context->filesize = sizeAdvise;
		context->sizesent = 0;
	}
	context->sizeleft = context->filesize;
	context->callBack = NULL;
	context->closeFlag = closeFlag;
	context->mod.name = "File Read Module";

	return context;
}

void
pgpFileReadSetCallBack (PgpFileRead *context, PGPFileReadCallBack callBack,
						void *callBackArg)
{
	context->callBack = callBack;
	context->callBackArg = callBackArg;
}

byte const *
pgpFileReadPeek (struct PgpFileRead *context, size_t *len)
{
	pgpAssert (context);

	/* Compress data to beginning of buffer */
	if (context->p != context->buffer) {
		memmove (context->buffer, context->p, context->len);
		context->p = context->buffer;
	}

	/* Now try to fill the buffer */
	if (!pgpFileEof (context->file))
		context->len += pgpFileRead (context->p + context->len,
				sizeof(context->buffer) - context->len,
				context->file);

	if (len)
		*len = context->len;

	return context->p;
}

void
pgpFileReadDestroy (struct PgpFileRead *context)
{
	pgpAssert (context);

	if (context->closeFlag)
		(void) pgpFileClose (context->file);

	pgpMemFree (context);
}	

int
pgpFileReadClose (struct PgpFileRead *context, struct PgpPipeline *head)
{
	int error = 0;

	if (context->annsent < 2) {
		error = head->annotate (head, &context->mod, PGPANN_FILE_END,
					NULL, 0);
		if (error)
			return error;
		context->annsent = 2;
	}
	return error;
}

int
pgpFileReadPump (struct PgpFileRead *context, struct PgpPipeline *head)
{
	int error = 0;
	size_t retlen;

	pgpAssert (context);
	pgpAssert (head);

	/*
	* Size should be sent outside scope. Also send it inside for consistency
	* with past behavior; that is not necessary but should not hurt.
	*/
	if (!context->sizesent) {
		error = head->sizeAdvise (head, context->sizeleft);
		if (error)
			return error;
	}

	if (!context->annsent) {
		error = head->annotate (head, &context->mod, PGPANN_FILE_BEGIN,
					NULL, 0);
		if (error)
			return error;

		context->annsent = 1;
	}

	if (!context->sizesent) {
		error = head->sizeAdvise (head, context->sizeleft);
		if (error)
			return error;

		context->sizesent = 1;
	}

	do {
		/* Write out anything in my buffer */
		while (context->len) {
			retlen = head->write (head, context->p,
					context->len, &error);
			context->len -= retlen;
			context->p += retlen;
			context->sizeleft -= retlen;
			if (error)
				return error;
			if (context->callBack != NULL)
			{
				error = (*context->callBack) (context->callBackArg,
							context->filesize - context->sizeleft,
							context->filesize);
				if (error != PGPERR_OK)
					return error;
			}
		}

		context->p = context->buffer;
		if (pgpFileEof (context->file))
			break;
		context->len = pgpFileRead (context->p, sizeof(context->buffer),
		context->file);

	} while (context->len);

	/* If I get here, then I'm at the end of the file */

	if (context->sizeleft && context->sizesent > 0)
		return PGPERR_SIZEADVISE;


	error = head->sizeAdvise (head, 0);
	if (error)
		return error;


	error = pgpFileReadClose (context, head);
	if (error)
		return error;

	return PGPERR_OK;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/

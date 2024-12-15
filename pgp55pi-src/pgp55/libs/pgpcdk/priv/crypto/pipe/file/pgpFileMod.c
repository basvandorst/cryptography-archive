/*
 * File I/O modules.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFileMod.c,v 1.27 1997/09/17 01:33:13 lloyd Exp $
 */

#include "pgpConfig.h"
#include <string.h>

#include <stdio.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef PGP_MACINTOSH
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
#include "pgpErrors.h"
#include "pgpFile.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#include "pgpKludge.h"

#define FILEMODMAGIC	0xfeedf11e

typedef struct FileModContext
{
	PGPPipeline	pipe;
	
	PGPFile *file;
	int closeFlag;
	int in_newfile;
	int scope_depth;
	DEBUG_STRUCT_CONSTRUCTOR( FileModContext )
} FileModContext;

/*
 * If we have a file, flush it.  Then, if we are allowed to close
 * it, then do so.  If we successfully closed it, then set the file
 * pointer to 0.  Return from any error with the error number.
 */
static PGPError
fileModClose (FileModContext *context)
{
	PGPError	error = kPGPError_NoErr;

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

	return kPGPError_NoErr;
}

static PGPError
Flush (PGPPipeline *myself)
{
	FileModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (FileModContext *)myself->priv;
	pgpAssert (context);

	if (!context->file)
		return kPGPError_NoErr;

	return pgpFileFlush (context->file);
}

static size_t
fileWrite (PGPPipeline *myself, PGPByte const *buffer, size_t size,
	   PGPError *error)
{
	FileModContext *context;
	size_t retval;

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (FileModContext *)myself->priv;
	pgpAssert (context);

	if (!context->file) {
		if (error)
			*error = kPGPError_CantOpenFile; /* No such file or directory*/
		return 0;
	}

	retval = pgpFileWrite (buffer, size, context->file);
	if (retval) {
		if (error)
			*error = kPGPError_NoErr;
		return retval;
	}
	if (error && pgpFileError (context->file))
		*error = kPGPError_FileOpFailed;
	pgpFileClearError (context->file);
	return 0;
}

/*
 * We accept certain annotations to tell us that we should change
 * the file we are writing.  If we have an open file, then it is an
 * assertion error if it doesn't have closeFlag true.
 *
 */
static PGPError
fileAnnotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	      PGPByte const *string, size_t size)
{
	FileModContext *context;
	FILE *fp;
	char filename[FILENAME_MAX];
	size_t len;
	PGPError	error;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	(void)origin;	/* Not used */

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (FileModContext *)myself->priv;
	pgpAssert (context);

	switch (type) {
	case PGPANN_NEWFILE_START:
		pgpAssert(!context->file || context->closeFlag);

		if (context->file) {
			error = fileModClose (context);
			if (error)
				return error;
		}

		pgpAssert (!context->file);
		{
			PGPFile *pgpf;

			len = pgpMin (size, sizeof (filename) - 1);
			memcpy (filename, string, len);
			filename[len] = '\0';
			
			/* I need to know if this should be binary or not */
			fp = fopen (filename, "wb");
			if (!fp)
				return kPGPError_CantOpenFile;
			pgpf = pgpFileWriteOpen ( cdkContext, fp, NULL);
			if (!pgpf) {
				fclose (fp);
				return kPGPError_OutOfMemory;
			}
			context->file = pgpf;
			context->closeFlag = 1;
		}
		context->in_newfile = 1;
		break;
	default:
		;		/* do nothing */
	}
		

	PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);

	return kPGPError_NoErr;
}

static PGPError
fileSizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	FileModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (FileModContext *)myself->priv;
	pgpAssert (context);

	if (bytes || context->scope_depth)
		return kPGPError_NoErr;
 	if (context->in_newfile && !context->closeFlag)
		return kPGPError_NoErr;

	return fileModClose (context);
}

static PGPError
fileTeardown (PGPPipeline *myself)
{
	FileModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == FILEMODMAGIC);

	context = (FileModContext *)myself->priv;
	pgpAssert (context);

	fileModClose (context);
		
	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return( kPGPError_NoErr );
}

PGPPipeline *
pgpFileWriteCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPFile *file,
	int closeFlag)
{
	PGPPipeline *mod;
	FileModContext *context;

	if (!head)
		return NULL;

	*head = 0;

	context = (FileModContext *)pgpContextMemAlloc(
		cdkContext, sizeof(*context), kPGPMemoryFlags_Clear );
	if ( IsNull( context ) )
		return NULL;
	mod = &context->pipe;

	mod->magic = FILEMODMAGIC;
	mod->write = fileWrite;
	mod->flush = Flush;
	mod->sizeAdvise = fileSizeAdvise;
	mod->annotate = fileAnnotate;
	mod->teardown = fileTeardown;
	mod->name = "Write file";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->file = file;
	context->closeFlag = closeFlag;

	*head = mod;
	return mod;
}

#if (BUFSIZ < 16384) && (PGP_MACINTOSH || PGP_WIN32)
#define kPGPReadBufSize		16384
#else
#define kPGPReadBufSize		BUFSIZ
#endif

#if 0
/* Enable this for debugging - it stresses the hell out of the pipeline */
#undef kPGPReadBufSize
#define kPGPReadBufSize 1
#endif

struct PGPFileRead
{
	PGPContextRef		cdkContext;
	
	PGPPipeline			mod;
	
	PGPFile *			file;
	PGPByte				buffer[kPGPReadBufSize];
	PGPByte *				p;
	off_t				filesize;
	off_t				sizeleft;
	size_t				len;
	PGPFileReadCallBack	callBack;
	void *				callBackArg;
	int					annsent;
	int					sizesent;
	int					closeFlag;
} ;

PGPFileRead *
pgpFileReadCreate(
	PGPContextRef	cdkContext,
	FILE *			stdioFile,
	int				closeFlag)
{
	/*
	 * XXX Potential space leak if <closeFlag> is false, although it's
	 *     never called that way from anywhere except possibly on Un*x.
	 *     never called that way from anywhere in PGP 5.0
	 */
	/* pgpFixBeforeShip("Potential space leak if <closeFlag> is false"); */
	
	return pgpPGPFileReadCreate( cdkContext,
		pgpFileReadOpen ( cdkContext, stdioFile, NULL, NULL), closeFlag);
}

PGPFileRead *
pgpPGPFileReadCreate(
	PGPContextRef	cdkContext,
	PGPFile *input, int closeFlag)
{
	PGPFileRead *context;
	long sizeAdvise;

	pgpAssert (input);

	context = (PGPFileRead *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryFlags_Clear );
	if (!context)
		return NULL;
	context->cdkContext	= cdkContext;
	
	sizeAdvise = pgpFileSizeAdvise (input);

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
pgpFileReadSetCallBack (PGPFileRead *context, PGPFileReadCallBack callBack,
						void *callBackArg)
{
	context->callBack = callBack;
	context->callBackArg = callBackArg;
}

PGPByte const *
pgpFileReadPeek (PGPFileRead *context, PGPSize *len)
{
	pgpAssert (context);

	/* Compress data to beginning of buffer */
	if (context->p != context->buffer) {
		pgpCopyMemory (context->p, context->buffer, context->len);
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
pgpFileReadDestroy (PGPFileRead *context)
{
	pgpAssert (context);

	if (context->closeFlag)
		(void) pgpFileClose (context->file);

	pgpContextMemFree( context->cdkContext, context);
}	

PGPError
pgpFileReadClose (PGPFileRead *context, PGPPipeline *head)
{
	PGPError	error = kPGPError_NoErr;

	if (context->annsent < 2) {
		error = head->annotate (head, &context->mod, PGPANN_FILE_END,
					NULL, 0);
		if (error)
			return error;
		context->annsent = 2;
	}
	return error;
}

PGPError
pgpFileReadPump (PGPFileRead *context, PGPPipeline *head)
{
	PGPError	error = kPGPError_NoErr;
	size_t retlen;

	pgpAssert (context);
	pgpAssert (head);

	/*
	 * Size should be sent outside scope.  Also send it inside for consistency
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
				if (error != kPGPError_NoErr)
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
		return kPGPError_SizeAdviseFailure;


	error = head->sizeAdvise (head, 0);
	if (error)
		return error;


	error = pgpFileReadClose (context, head);
	if (error)
		return error;

	return kPGPError_NoErr;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

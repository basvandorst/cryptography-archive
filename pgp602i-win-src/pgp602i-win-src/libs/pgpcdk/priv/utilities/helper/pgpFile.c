/*
 * pgpFile.c -- An abstraction to files
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFile.c,v 1.27.4.1 1998/10/06 01:20:45 heller Exp $
 */

#include "pgpConfig.h"

#include <errno.h>
#include <stdio.h>
#if PGP_MACINTOSH
#include <unix.h>
#include <stat.h>
#include <unix.mac.h>		/* For fileno() */
#endif

#if HAVE_SYS_STAT_H
#include <sys/types.h>		/* required for Ultrix */
#include <sys/stat.h>
#endif

#include "pgpDebug.h"
#include "pgpFile.h"
#include "pgpCFBPriv.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpContext.h"

struct File {
	PGPError	(*doClose) (FILE *file, void *arg);
	void *		closeArg;
	PGPFileError err;
	PGPCFBContext *cfb;
	FILE *f;
	PGPError	error;
	int flags;
	long maybeSize;		/* Size or -1 if not a regular file, for sizeAdvise */
	DEBUG_STRUCT_CONSTRUCTOR( File )
};
typedef struct File File;

/*
 * These are the different flags, which define the various operations
 * * * and types.
 */
#define FLAGS_READ		0x01
#define FLAGS_WRITE		0x02
#define FLAGS_FILE		0x04
#define FLAGS_PROC		0x08
#define FLAGS_DONTCLOSE		0x10

#define PGP_FILE_READ	(FLAGS_FILE|FLAGS_READ)
#define PGP_FILE_WRITE	(FLAGS_FILE|FLAGS_WRITE)
#define PGP_PROC_WRITE	(FLAGS_PROC|FLAGS_WRITE)

/***************************************************/
/* Stdio Functions */

static void
setError (PGPFile *file, int code)
{
	File *fp = (File *)file->priv;

	fp->err.f = file;
	fp->err.syserrno = errno;
	fp->err.error = (PGPError)code;
	fp->error = (PGPError)ferror (fp->f);
	if (fp->error) {
		fp->err.fpos = pgpFileTell (file);
	}
}

static size_t
stdioRead (void *ptr, size_t size, PGPFile *file)
{
	File *fp = (File *)file->priv;
	size_t ret;

	if (! (fp->flags & FLAGS_READ)) {
		setError (file, kPGPError_IllegalFileOp);
		fp->error = kPGPError_IllegalFileOp;
		return 0;
	}

	ret = fread (ptr, 1, size, fp->f);

	if (!ret) 
		setError (file, kPGPError_FileOpFailed);

	return ret;
}

static size_t
stdioWrite (void const *ptr, size_t size, PGPFile *file)
{
	File *fp = (File *)file->priv;
	size_t ret;

	if (! (fp->flags & FLAGS_WRITE)) {
		setError (file, kPGPError_FileLocked);
		fp->error = kPGPError_FileLocked;
		return 0;
	}

	ret = fwrite (ptr, 1, size, fp->f);

	if (!ret)
		setError (file, kPGPError_FileOpFailed);

	return ret;
}

static PGPError
stdioFlush (PGPFile *file)
{
	File *fp = (File *)file->priv;
	PGPError	 ret;

	if (! (fp->flags & FLAGS_WRITE))
		return kPGPError_FileLocked;

	ret = (PGPError)fflush (fp->f);

#if PGP_MACINTOSH
	if (!ret) {
		IOParam		pb;

		pb.ioRefNum = fileno( fp->f );
		ret = (PGPError)PBFlushFileSync( (ParamBlockRec *)&pb );
	}
#endif

	if (ret) {
		ret = kPGPError_FileOpFailed;
		setError (file, ret);
		fp->error = ret;
	}

	return ret;
}

static long
stdioTell (PGPFile *file)
{
	File *fp = (File *)file->priv;

	return ftell (fp->f);
}

static PGPError
stdioSeek (PGPFile *file, long offset, int whence)
{
	File *fp = (File *)file->priv;
	PGPError code;

	if (! (fp->flags & FLAGS_READ))
		return kPGPError_IllegalFileOp;

	code = (PGPError)fseek (fp->f, offset, whence);

	if (code) {
		code = kPGPError_FileOpFailed;
		setError (file, code);
		fp->error = code;
	}

	return code;
}

/**********************************************************/
/* Non-specific functions (stdio or encrypted) */

static PGPError
fileClose (PGPFile *file)
{
	File *fp = (File *)file->priv;
	PGPError code = kPGPError_NoErr;

	pgpAssertAddrValid( file, PGPFile );

	if ((fp->flags & FLAGS_FILE) && !(fp->flags & FLAGS_DONTCLOSE))
		code = (PGPError)fclose (fp->f);
	else if (fp->flags & FLAGS_PROC) {
		if (fp->doClose)
			code = (PGPError)fp->doClose (fp->f, fp->closeArg);
		/* We may not be able to 'close' here.. I hope that is ok */
	}

	if (code) {
		code = kPGPError_FileOpFailed;
		setError (file, (PGPError)code);
		fp->error = (PGPError)code;
		return code;
	}

	if (fp->cfb)
		PGPFreeCFBContext(fp->cfb);

	pgpClearMemory( fp,  sizeof (*fp));
	PGPFreeData( fp);
	pgpClearMemory( file,  sizeof (*file));
	PGPFreeData( file);

	return code;
}

static PGPBoolean
fileEof (PGPFile const *file)
{
	File *fp = (File *)file->priv;

	return feof (fp->f);
}

static long
fileSizeAdvise (PGPFile const *file)
{
	File *fp = (File *)file->priv;

	return fp->maybeSize;
}

static PGPFileError const *
fileError (PGPFile const *file)
{
	File *fp = (File *)file->priv;

	if (fp->error)
		return &(fp->err);

	return NULL;
}

static void
fileClearError (PGPFile *file)
{
	File *fp = (File *)file->priv;

	clearerr (fp->f);
	fp->error = kPGPError_NoErr;
}

/* Converts a writing file to a reading file */
static PGPError
fileWrite2read (PGPFile *file)
{
	File *fp = (File *)file->priv;

	if ((fp->flags & PGP_FILE_WRITE) != PGP_FILE_WRITE)
		return kPGPError_FileLocked;

	fp->flags &= ~FLAGS_READ;
	fp->flags |= FLAGS_WRITE;

	/* XXX -- should I rewind this file? */
	/* return pgpFileSeek (file, 0, SEEK_SET); */
	return kPGPError_NoErr;
}

/* Creates a new PGPCFBContext and returns it.
 Returns NULL otherwise */
static PGPCFBContextRef
fileCfb (PGPFile const *file)
{
	File *fp = (File *)file->priv;

	if ( IsntNull( fp->cfb ))
	{
		PGPCFBContextRef	newRef;
		PGPError		err;
		
		err	= PGPCopyCFBContext( fp->cfb, &newRef );
		return( newRef );
	}

	return NULL;
}

/*************************************************************/
/* Open functions */

/* take a FILE* and convert it to a PGPFile* */
static PGPFile *
doOpen(
	PGPContextRef	cdkContext,
	FILE *file, int fflags)
{
	PGPFile *fp;
	File *ffp;
	struct stat buf;
	PGPMemoryMgrRef	memoryMgr	= PGPGetContextMemoryMgr( cdkContext );

	if (!file)
		return NULL;

	fp = (PGPFile *)PGPNewData( memoryMgr,
		sizeof (*fp), kPGPMemoryMgrFlags_Clear);
	if (!fp)
		return NULL;
	fp->context		= cdkContext;
	fp->dataType 	= kPGPFileDataType_Unknown;

	ffp = (File *)PGPNewData( memoryMgr,
		sizeof (*ffp), kPGPMemoryMgrFlags_Clear);
	if (!ffp) {
		PGPFreeData( fp );
		return NULL;
	}

	if (fstat (fileno (file), &buf) == 0 &&
			(S_IFMT & buf.st_mode) == S_IFREG)
		ffp->maybeSize = buf.st_size;
	else
		ffp->maybeSize = -1;
	
	rewind (file);
	ffp->f = file;
	ffp->flags = fflags;

	fp->priv = ffp;
	fp->read = stdioRead;
	fp->write = stdioWrite;
	fp->flush = stdioFlush;
	fp->close = fileClose;
	fp->tell = stdioTell;
	fp->seek = stdioSeek;
	fp->eof = fileEof;
	fp->sizeAdvise = fileSizeAdvise;
	fp->error = fileError;
	fp->clearError = fileClearError;
	fp->write2read = fileWrite2read;
	fp->cfb = fileCfb;

	return fp;
}

/* Convert a FILE* to PGPFile* in Write Mode */
PGPFile *
pgpFileWriteOpen (PGPContextRef	cdkContext,
	FILE *file, PGPCFBContext *cfbp)
{
	(void)cfbp;		/* Avoid warning */
	pgpAssert (cfbp == NULL);
	return doOpen ( cdkContext, file, PGP_FILE_WRITE);
} 

/* Same as above, but don't close the FILE automatically */
PGPFile *
pgpFileWriteOpenDontClose (PGPContextRef context,
	FILE *file, PGPCFBContext *cfbp)
{
	(void)cfbp;		/* Avoid warning */
	pgpAssert (cfbp == NULL);
	return doOpen ( context,
			file, PGP_FILE_WRITE | FLAGS_DONTCLOSE);
} 

/* Convert a FILE* to PGPFile* in Read Mode */
PGPFile *
pgpFileReadOpen ( PGPContextRef context,
	FILE *file, PGPUICb const *ui, void *ui_arg)
{
	(void)ui;
	(void)ui_arg;
	/*
	 * We still need to check the file for encryption and obtain
	 * the decryption key, somehow.
	 */

	return doOpen ( context, file, PGP_FILE_READ);
}

PGPFile *
pgpFileProcWriteOpen ( 	PGPContextRef context,
	FILE *file, PGPError (*doClose) (FILE *file, void *arg),
		      void *arg)
{
	PGPFile *fp;
	File *f;

	fp = doOpen ( context, file, PGP_PROC_WRITE);
	f = (File *)fp->priv;
	pgpAssert (f);

	f->doClose = doClose;
	f->closeArg = arg;
	return fp;
}






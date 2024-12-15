/*
 * pgpMemFile.c -- PGPFile implementation for memory files, based on PFL IO
 *
 * $Id: pgpMemFile.c,v 1.15 1997/12/19 19:09:45 lloyd Exp $
 */

#include "pgpConfig.h"

#include <errno.h>
#include <stdio.h>

#include "pgpFile.h"
#include "pgpCFB.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpIO.h"
#include "pgpMemoryIO.h"

/* This is the private data for memfiles */
typedef struct MemFile_
{
	PGPContextRef	context;
	PGPIORef		ioRef;
	PGPFileError	err;
	PGPError		error;
} MemFile;

static long memFileTell(PGPFile *file);

	static void
memSetError(PGPFile *file, PGPError code)
{
	MemFile *	mf = (MemFile *)file->priv;

	mf->err.f = file;
	mf->err.fpos = memFileTell(file);
	mf->error = mf->err.error = code;
	mf->err.syserrno = 0;
}

	static size_t
memFileRead(void *ptr, size_t size, PGPFile *file)
{
	MemFile *	mf = (MemFile *)file->priv;
	PGPSize		bytesRead = 0;
	PGPError	err;

	if( IsPGPError( err = PGPIORead( mf->ioRef, size, ptr, &bytesRead ) ) )
		memSetError( file, err );
	return (size_t)bytesRead;
}

	static size_t
memFileWrite(void const *ptr, size_t size, PGPFile *file)
{
	MemFile *	mf = (MemFile *)file->priv;
	PGPError	err;

	if( IsPGPError( err = PGPIOWrite( mf->ioRef, size, ptr ) ) )
		memSetError( file, err );
	return (size_t)size;
}

	static PGPError
memFileFlush(PGPFile *file)
{
	MemFile *	mf = (MemFile *)file->priv;
	PGPError	err;

	if( IsPGPError( err = PGPIOFlush( mf->ioRef ) ) )
		memSetError( file, err );
	return err;
}

	static PGPError
memFileClose(PGPFile *file)
{
	MemFile *	mf = (MemFile *)file->priv;
	
	pgpAssertAddrValid( file, PGPFile );

	(void)PGPFreeIO( mf->ioRef );
	PGPFreeData( mf );
	PGPFreeData( file );
	return kPGPError_NoErr;
}

	static long
memFileTell(PGPFile *file)
{
	MemFile *	mf = (MemFile *)file->priv;
	PGPFileOffset curPos = (PGPFileOffset)-1;

	/* Ignore error as we are called from error-setting routine */
	(void)PGPIOGetPos( mf->ioRef, &curPos );
	return (long) curPos;
}

	static PGPError
memFileSeek(PGPFile *file, long offset, int whence)
{
	MemFile *		mf = (MemFile *)file->priv;
	PGPFileOffset	newPos;
	PGPError		err;

	switch (whence)
	{
		case SEEK_SET:
			newPos = (PGPFileOffset)offset;
			break;
		case SEEK_CUR:
			PGPIOGetPos( mf->ioRef, &newPos );
			newPos += offset;
			break;
		case SEEK_END:
			PGPIOGetEOF( mf->ioRef, &newPos );
			newPos += offset;
			break;
	}

	if( IsPGPError( err = PGPIOSetPos( mf->ioRef, newPos ) ) )
		memSetError( file, err );

	return err;
}

	static PGPBoolean
memFileEof(PGPFile const *file)
{
	MemFile *	mf = (MemFile *)file->priv;

	return PGPIOIsAtEOF( mf->ioRef );
}

	static long
memFileSizeAdvise(PGPFile const *file)
{
	(void)file;
	return 0L;
}

	static PGPFileError const *
memFileError(PGPFile const *file)
{
	MemFile *	mf = (MemFile *)file->priv;

	if (mf->error != kPGPError_NoErr)
		return &mf->err;
	else
		return NULL;
}

	static void
memFileClearError(PGPFile *file)
{
	MemFile *	mf = (MemFile *)file->priv;

	mf->error = kPGPError_NoErr;
}

	static PGPError
memFileWrite2Read(PGPFile *file)
{
	MemFile *	mf = (MemFile *)file->priv;
	PGPError	err;

	/* Just rewind */
	if( IsPGPError( err = PGPIOSetPos( mf->ioRef, 0 ) ) )
		memSetError( file, err );
	return err;
}

	static PGPCFBContext *
memFileCfb(PGPFile const *file)
{
	/* For future expansion */
	(void)file;
	return NULL;
}

	PGPFile *
pgpFileMemOpen(
	PGPContextRef	context,
	void *base, size_t len)
{
	PGPFile *	file;
	MemFile *	mf;
	PGPError	err;

	file = (PGPFile *)pgpContextMemAlloc( context,
		sizeof(*file), kPGPMemoryMgrFlags_Clear);
	if (file == NULL)
		return NULL;
	file->context	= context;

	mf = (MemFile *)pgpContextMemAlloc( context,
		sizeof(*mf), kPGPMemoryMgrFlags_Clear);
	if (mf == NULL)
	{
		pgpContextMemFree( context, file);
		return NULL;
	}

	mf->context	= context;

	/* Allocate ioRef structure */
	if( IsPGPError( err = PGPNewMemoryIO( PGPGetContextMemoryMgr( context ),
										  (PGPMemoryIORef *)&mf->ioRef ) ) )
	{
		pgpContextMemFree( context, mf );
		pgpContextMemFree( context, file );
		return NULL;
	}

	/* Load ioRef with our data */
	if( len != 0 )
	{
		if( IsPGPError( err = PGPIOWrite( mf->ioRef, len, base ) ) ||
			IsPGPError( err = PGPIOSetPos( mf->ioRef, 0 ) ) )
		{
			PGPFreeIO( mf->ioRef );
			pgpContextMemFree( context, mf );
			pgpContextMemFree( context, file );
			return NULL;
		}
	}

	file->priv = mf;
	file->read = memFileRead;
	file->write = memFileWrite;
	file->flush = memFileFlush;
	file->close = memFileClose;
	file->tell = memFileTell;
	file->seek = memFileSeek;
	file->eof = memFileEof;
	file->sizeAdvise = memFileSizeAdvise;
	file->error = memFileError;
	file->clearError = memFileClearError;
	file->write2read = memFileWrite2Read;
	file->cfb = memFileCfb;
	return file;
}

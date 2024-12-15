/*
* pgpMemFile.c -- PgpFile implementation for memory files
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpMemFile.c,v 1.6.2.1 1997/06/07 09:50:11 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>

#include "pgpFile.h"
#include "pgpCFB.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpUsuals.h"

/* This is the private data for memfiles */
typedef struct MemFile_
	{
			char *					base;
			char *					end;
			char *					cur;
			PgpFileError	 err;
			PGPError	 			error;
} MemFile;

static long memFileTell(PgpFile *file);

	static void
memSetError(PgpFile *file, PGPError code)
{
			MemFile *	mf = (MemFile *)file->priv;

			mf->err.f = file;
			mf->err.fpos = memFileTell(file);
			mf->error = mf->err.error = code;
			mf->err.syserrno = 0;
}

			static size_t
memFileRead(void *ptr, size_t size, PgpFile *file)
{
			MemFile *	mf = (MemFile *)file->priv;

			if (size > (size_t)(mf->end - mf->cur))
				 size = mf->end - mf->cur;

			if (size <= 0)
		{
				memSetError(file, PGPERR_FILE_OPFAIL);
				return 0;
		}
			else
			{
					pgpCopyMemory(mf->cur, ptr, size);
					mf->cur += size;
					return size;
			}
	}

		static size_t
memFileWrite(void const *ptr, size_t size, PgpFile *file)
{
			(void)ptr;	/* Quiet the compiler warnings */
			(void)size;

memSetError(file, PGPERR_FILE_BADOP);
return 0;
}

		static int
memFileFlush(PgpFile *file)
{
(void)file;	 /* Quiet compiler warning */

/* Does nothing */
return PGPERR_OK;
}

		static int
memFileClose(PgpFile *file)
{
MemFile *	mf = (MemFile *)file->priv;

pgpFree(mf);
pgpFree(file);
return PGPERR_OK;
}

		static long
memFileTell(PgpFile *file)
{
MemFile *	mf = (MemFile *)file->priv;

return mf->cur - mf->base;
}

		static int
memFileSeek(PgpFile *file, long offset, int whence)
{
			MemFile *	mf = (MemFile *)file->priv;

switch (whence)
	{
				case SEEK_SET:
					/* offset is correct, as is */
					break;
				case SEEK_CUR:
					offset += mf->cur - mf->base;
					break;
				case SEEK_END:
				 	offset += mf->end - mf->base;
				 	break;
		}

			if(offset < 0 || offset > mf->end - mf->base)
			{
				 memSetError(file, PGPERR_FILE_OPFAIL);
				 return PGPERR_FILE_OPFAIL;
			}

			mf->cur = mf->base + offset;
			return PGPERR_OK;
}

			static int
memFileEof(PgpFile const *file)
{
	MemFile *	 mf = (MemFile *)file->priv;

	return mf->cur >= mf->end;
}

	static long
memFileSizeAdvise(PgpFile const *file)
{
	MemFile *	 mf = (MemFile *)file->priv;

	return mf->end - mf->end;
}

	static PgpFileError const *
memFileError(PgpFile const *file)
{
			MemFile *	mf = (MemFile *)file->priv;

			if (mf->error != PGPERR_OK)
				 return &mf->err;
			else
				 return NULL;
}

			static void
memFileClearError(PgpFile *file)
{
MemFile *	mf = (MemFile *)file->priv;

mf->error = PGPERR_OK;
}

static int
memFileWrite2Read(PgpFile *file)
{
memSetError(file, PGPERR_FILE_BADOP);
return PGPERR_FILE_BADOP;
}

		static PgpCfbContext *
memFileCfb(PgpFile const *file)
{
			(void)file;	/* Quiet compiler warning */
			/* XXX: I'm not really sure what should be done here */
			return NULL;
}

			PgpFile *
pgpFileMemReadOpen(void *base, size_t len)
{
			PgpFile *	file;
			MemFile *	mf;

			file = (PgpFile *)pgpAlloc(sizeof(*file));
			if (file == NULL)
			return NULL;

			mf = (MemFile *)pgpAlloc(sizeof(*mf));
			if (mf == NULL)
			{
				 pgpFree(file);
				 return NULL;
			}

		pgpClearMemory(mf, sizeof(*mf));
			pgpClearMemory(file, sizeof(*file));

			mf->base = mf->cur = (char *)base;
			mf->end = mf->base + len;

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

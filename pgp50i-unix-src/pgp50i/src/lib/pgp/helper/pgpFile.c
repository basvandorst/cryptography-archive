/*
 * pgpFile.c -- An abstraction to files
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by: Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFile.c,v 1.5.2.2 1997/06/07 09:50:03 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#ifdef MACINTOSH
#include <unix.h>
#include <stat.h>
#else
#include <sys/types.h> /* required for Ultrix */
#include <sys/stat.h>
#endif

#include "pgpDebug.h"
#include "pgpFile.h"
#include "pgpCFB.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpUsuals.h"

struct File {
    int (*doClose) (FILE *file, void *arg);
    void *closeArg;
    struct PgpFileError err;
    struct PgpCfbContext *cfb;
    FILE *f;
    int error;
    int flags;
    long maybeSize; /* Size or -1 if not a regular file, for sizeAdvise */
};

/*
 * These are the different flags, which define the various operations
 * * * and types.
 */
#define FLAGS_READ  1
#define FLAGS_WRITE 2
#define FLAGS_FILE  4
#define FLAGS_PROC  8

#define PGP_FILE_READ  (FLAGS_FILE|FLAGS_READ)
#define PGP_FILE_WRITE  (FLAGS_FILE|FLAGS_WRITE)
#define PGP_PROC_WRITE  (FLAGS_PROC|FLAGS_WRITE)

/***************************************************/
/* Stdio Functions */

static void
setError (struct PgpFile *file, int code)
{
    struct File *fp = (struct File *)file->priv;

    fp->err.f = file;
    fp->err.syserrno = errno;
    fp->err.error = code;
    fp->error = ferror (fp->f);
    if (fp->error) {
        fp->err.fpos = pgpFileTell (file);
    }
}

static size_t
stdioRead (void *ptr, size_t size, struct PgpFile *file)
{
    struct File *fp = (struct File *)file->priv;
    size_t ret;

    if (! (fp->flags & FLAGS_READ)) {
        setError (file, PGPERR_FILE_BADOP);
        fp->error = PGPERR_FILE_BADOP;
        return 0;
    }

    ret = fread (ptr, 1, size, fp->f);

    if (!ret)
        setError (file, PGPERR_FILE_OPFAIL);

    return ret;
}

static size_t
stdioWrite (void const *ptr, size_t size, struct PgpFile *file)
{
    struct File *fp = (struct File *)file->priv;
    size_t ret;

    if (! (fp->flags & FLAGS_WRITE)) {
        setError (file, PGPERR_FILE_BADOP);
        fp->error = PGPERR_FILE_BADOP;
        return 0;
    }

    ret = fwrite (ptr, 1, size, fp->f);

    if (!ret)
        setError (file, PGPERR_FILE_OPFAIL);

    return ret;
}

static int
stdioFlush (struct PgpFile *file)
{
    struct File *fp = (struct File *)file->priv;
    int ret;

    if (! (fp->flags & FLAGS_WRITE))
        return PGPERR_FILE_BADOP;

    ret = fflush (fp->f);

    if (ret) {
    ret = PGPERR_FILE_OPFAIL;
    setError (file, ret);
    fp->error = ret;
    }

    return ret;
}

static long
stdioTell (struct PgpFile *file)
{
    struct File *fp = (struct File *)file->priv;

    return ftell (fp->f);
}

static int
stdioSeek (struct PgpFile *file, long offset, int whence)
{
    struct File *fp = (struct File *)file->priv;
    int code;

    if (! (fp->flags & FLAGS_READ))
        return PGPERR_FILE_BADOP;

    code = fseek (fp->f, offset, whence);

    if (code) {
        code = PGPERR_FILE_OPFAIL;
        setError (file, code);
    fp->error = code;
    }

    return code;
}

/**********************************************************/
/* Non-specific functions (stdio or encrypted) */

static int
fileClose (struct PgpFile *file)
{
    struct File *fp = (struct File *)file->priv;
    int code = 0;

    if (fp->flags & FLAGS_FILE)
        code = fclose (fp->f);
    else if (fp->flags & FLAGS_PROC) {
        if (fp->doClose)
      code = fp->doClose (fp->f, fp->closeArg);
    /* We may not be able to 'close' here.. I hope that is ok */
    }

    if (code) {
        code = PGPERR_FILE_OPFAIL;
        setError (file, code);
    fp->error = code;
    return code;
    }

    if (fp->cfb)
        pgpCfbDestroy (fp->cfb);

    memset (fp, 0, sizeof (*fp));
    pgpMemFree (fp);
    memset (file, 0, sizeof (*file));
    pgpMemFree (file);

    return code;
}

static int
fileEof (struct PgpFile const *file)
{
    struct File *fp = (struct File *)file->priv;

    return feof (fp->f);
}

static long
fileSizeAdvise (struct PgpFile const *file)
{
    struct File *fp = (struct File *)file->priv;

    return fp->maybeSize;
}

static struct PgpFileError const *
fileError (struct PgpFile const *file)
{
    struct File *fp = (struct File *)file->priv;

    if (fp->error)
        return &(fp->err);

    return NULL;
}

static void
fileClearError (struct PgpFile *file)
{
    struct File *fp = (struct File *)file->priv;

    clearerr (fp->f);
    fp->error = 0;
}

/* Converts a writing file to a reading file */
static int
fileWrite2read (struct PgpFile *file)
{
    struct File *fp = (struct File *)file->priv;

    if (fp->flags != PGP_FILE_WRITE)
        return PGPERR_FILE_BADOP;

    fp->flags = PGP_FILE_READ;

    /* XXX -- should I rewind this file? */
    /* return pgpFileSeek (file, 0, SEEK_SET); */
    return 0;
}

/* Creates a new PgpCfbContext and returns it. Returns NULL otherwise */
static struct PgpCfbContext *
fileCfb (struct PgpFile const *file)
{
    struct File *fp = (struct File *)file->priv;

    if (fp->cfb)
        return pgpCfbCopy (fp->cfb);

    return NULL;
}

/*************************************************************/
/* Open functions */

/* take a FILE* and convert it to a PgpFile* */
static struct PgpFile *
doOpen (FILE *file, int fflags)
{
    struct PgpFile *fp;
    struct File *ffp;
    struct stat buf;

    if (!file)
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

/* Convert a FILE* to PgpFile* in Write Mode */
struct PgpFile *
pgpFileWriteOpen (FILE *file, struct PgpCfbContext *cfbp)
{
    (void)cfbp;   /* Avoid warning */
    pgpAssert (cfbp == NULL);
    return doOpen (file, PGP_FILE_WRITE);
}

/* Convert a FILE* to PgpFile* in Read Mode */
struct PgpFile *
pgpFileReadOpen (FILE *file, struct PgpUICb const *ui, void *ui_arg)
{
    (void)ui;
    (void)ui_arg;
    /*
     * We still need to check the file for encryption and obtain
     * the decryption key, somehow.
     */

    return doOpen (file, PGP_FILE_READ);
}

struct PgpFile *
pgpFileProcWriteOpen (FILE *file, int (*doClose) (FILE *file, void *arg),
                      void *arg)
{
    struct PgpFile *fp;
    struct File *f;

    fp = doOpen (file, PGP_PROC_WRITE);
    f = (struct File *)fp->priv;
    pgpAssert (f);

    f->doClose = doClose;
    f->closeArg = arg;
    return fp;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

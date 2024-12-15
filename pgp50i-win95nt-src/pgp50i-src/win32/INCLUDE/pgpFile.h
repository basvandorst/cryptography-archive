/*
 * pgpFile.h -- an abstraction to files
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written By: Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFile.h,v 1.6.2.1 1997/06/07 09:50:03 mhw Exp $
 */

#ifndef PGPFILE_H
#define PGPFILE_H

#include "pgpTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpUICb;
#ifndef TYPE_PGPUICB
#define TYPE_PGPUICB 1
typedef struct PgpUICb PgpUICb;
#endif

struct PgpCfbContext;
#ifndef TYPE_PGPCFBCONTEXT
#define TYPE_PGPCFBCONTEXT 1
typedef struct PgpCfbContext PgpCfbContext;
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpFile;
#ifndef TYPE_PGPFILE
#define TYPE_PGPFILE 1
typedef struct PgpFile PgpFile;
#endif

struct PgpFileError {
        struct PgpFile *f;     /* The PgpFile for I/O Errors */
        long fpos;             /* file position */
        int error;             /* PGP error code (PGPERR_*) */
        int syserrno;          /* don't use errno; that's a macro! */
};
#ifndef TYPE_PGPFILEERROR
#define TYPE_PGPFILEERROR 1
typedef struct PgpFileError PgpFileError;
#endif

struct PgpFile {
    void *priv;
    size_t (*read) (void *ptr, size_t size, struct PgpFile *file);
    size_t (*write) (void const *ptr, size_t size, struct PgpFile *file);
    int (*flush) (struct PgpFile *file);
    int (*close) (struct PgpFile *file);
    long (*tell) (struct PgpFile *file);
    int (*seek) (struct PgpFile *file, long offset, int whence);
    int (*eof) (struct PgpFile const *file);
    long (*sizeAdvise) (struct PgpFile const *file);
    struct PgpFileError const * (*error) (struct PgpFile const *file);
    void (*clearError) (struct PgpFile *file);
    int (*write2read) (struct PgpFile *file);
    struct PgpCfbContext * (*cfb) (struct PgpFile const *file);
};
#ifndef TYPE_PGPFILE
#define TYPE_PGPFILE 1
typedef struct PgpFile PgpFile;
#endif

struct PgpFile PGPExport *pgpFileReadOpen (FILE *file,
        struct PgpUICb const *ui, void *ui_arg);
struct PgpFile PGPExport *pgpFileWriteOpen (FILE *file,
        struct PgpCfbContext *cfb);

struct PgpFile PGPExport *pgpFileProcWriteOpen (FILE *file,
                                      int (*doClose) (FILE *file, void *arg),
                                      void *arg);

struct PgpFile PGPExport *pgpFilePipelineOpen (struct PgpPipeline *head);

struct PgpFile PGPExport *pgpFileMemReadOpen (void *base, size_t len);

#ifdef __cplusplus
}
#endif

#define pgpFileRead(p,s,f) (f)->read(p,s,f)
#define pgpFileWrite(p,s,f) (f)->write(p,s,f)
#define pgpFileFlush(f) (f)->flush(f)
#define pgpFileClose(f) (f)->close(f)
#define pgpFileTell(f) (f)->tell(f)
#define pgpFileSeek(f,o,w) (f)->seek(f,o,w)
#define pgpFileEof(f) (f)->eof(f)
#define pgpFileSizeAdvise(f) (f)->sizeAdvise(f)
#define pgpFileError(f) (f)->error(f)
#define pgpFileClearError(f) (f)->clearError(f)
#define pgpFileWrite2Read(f) (f)->write2read(f)
#define pgpFileCfb(f) (f)->cfb(f)

#endif /* PGPFILE_H */

/*
 * pgpFile.h -- an abstraction to files
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFile.h,v 1.18 1997/10/06 18:01:26 lloyd Exp $
 */

#ifndef Included_pgpFile_h
#define Included_pgpFile_h

#include <stdio.h>

#include "pgpTypes.h"
#include "pgpUtilities.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

struct PGPFileError {
	PGPFile *	f;	/* The PGPFile for I/O Errors */
	long		fpos;		/* file position */
	PGPError	error;		/* PGP error code (kPGPError_*) */
	int			syserrno;	/* don't use errno; that's a macro! */
};


struct PGPFile {
	PGPContextRef	context;
	
	void *priv;
	size_t (*read) (void *ptr, size_t size, PGPFile *file);
	size_t (*write) (void const *ptr, size_t size, PGPFile *file);
	PGPError (*flush) (PGPFile *file);
	PGPError (*close) (PGPFile *file);
	long (*tell) (PGPFile *file);
	PGPError (*seek) (PGPFile *file, long offset, int whence);
	PGPBoolean (*eof) (PGPFile const *file);
	long (*sizeAdvise) (PGPFile const *file);
	PGPFileError const * (*error) (PGPFile const *file);
	void (*clearError) (PGPFile *file);
	PGPError (*write2read) (PGPFile *file);
	PGPCFBContext * (*cfb) (PGPFile const *file);
};


PGPFile  *pgpFileReadOpen (PGPContextRef	context, FILE *file,
	PGPUICb const *ui, void *ui_arg);
PGPFile  *pgpFileWriteOpen (PGPContextRef	context, FILE *file,
	PGPCFBContext *cfb);

PGPFile  *pgpFileProcWriteOpen ( PGPContextRef	context, FILE *file,
				      PGPError (*doClose) (FILE *file, void *arg),
				      void *arg);

/* Same as above, but don't close the FILE automatically */
PGPFile *pgpFileWriteOpenDontClose (PGPContextRef context,
	FILE *file, PGPCFBContext *cfbp);

PGPFile  *pgpFilePipelineOpen (PGPPipeline *head);

PGPFile  *pgpFileMemOpen ( PGPContextRef context,
			void *base, size_t len);

PGP_END_C_DECLARATIONS

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

#endif /* Included_pgpFile_h */

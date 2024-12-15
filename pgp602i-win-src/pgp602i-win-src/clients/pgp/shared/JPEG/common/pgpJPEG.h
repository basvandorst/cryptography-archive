/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpJPEG.h,v 1.2.8.1 1998/11/12 03:11:18 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpJPEG_h	/* [ */
#define Included_pgpJPEG_h

#include <setjmp.h>
#include <stdio.h>

#include "jpeglib.h"
#include "pgpPubTypes.h"

#define	kPGPDefaultJPEQQuality		70

typedef struct JPEGDecompressionContext
{
	struct jpeg_decompress_struct	dInfo;		/* Must be first item */
	struct jpeg_error_mgr			errMgr;
	FILE							*sourceFile;
	jmp_buf							exceptionBuffer;
	
} JPEGDecompressionContext;

typedef struct JPEGCompressionContext
{
	struct jpeg_compress_struct		cInfo;		/* Must be first item */
	struct jpeg_error_mgr			errMgr;
	FILE							*destFile;
	jmp_buf							exceptionBuffer;
	
} JPEGCompressionContext;

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/* Compression: */

void		JPEGCloseCompressionFile(JPEGCompressionContext *context);
PGPError	JPEGCreateCompress(JPEGCompressionContext *context);
PGPError	JPEGDefaultColorSpace(JPEGCompressionContext *context);
void		JPEGDestroyCompress(JPEGCompressionContext *context);
PGPError	JPEGFinishDecompress(JPEGDecompressionContext *context);
void		JPEGInitCompressionContext(JPEGCompressionContext *context);
PGPError	JPEGOpenCompressionFile(JPEGCompressionContext *context,
					PGPFileSpecRef fileSpec);
PGPError	JPEGSetColorSpace(JPEGCompressionContext *context,
					J_COLOR_SPACE colorspace);
PGPError	JPEGSetDefaults(JPEGCompressionContext *context);
PGPError	JPEGSetQuality(JPEGCompressionContext *context, PGPInt32 quality,
					PGPBoolean forceBaseline);
PGPError	JPEGStartDecompress(JPEGDecompressionContext *context);
PGPError	JPEGWriteScanlines(JPEGCompressionContext *context,
					JSAMPARRAY scanlines, JDIMENSION maxScanlines,
					JDIMENSION *numWrittenScanlines);
	
/* Decompression: */

void		JPEGCloseDecompressionFile(JPEGDecompressionContext *context);
PGPError	JPEGCreateDecompress(JPEGDecompressionContext *context);
void		JPEGDestroyDecompress(JPEGDecompressionContext *context);
PGPError	JPEGFinishCompress(JPEGCompressionContext *context);
void		JPEGInitDecompressionContext(JPEGDecompressionContext *context);
PGPError	JPEGOpenDecompressionFile(JPEGDecompressionContext 	*context,
					PGPFileSpecRef fileSpec, PGPBoolean readOnly);
PGPError	JPEGReadHeader(JPEGDecompressionContext *context,
					PGPBoolean requireImage);
PGPError	JPEGReadScanlines(JPEGDecompressionContext *context,
					JSAMPARRAY scanlines, JDIMENSION maxScanlines,
					JDIMENSION *numReadScanlines);
PGPError	JPEGStartCompress(JPEGCompressionContext *context,
					PGPBoolean writeAllTables);


/* Other: */

PGPError	JPEGCopyCriticalParameters(JPEGDecompressionContext *dContext,
					JPEGCompressionContext *cContext);
	
#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpKeys_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

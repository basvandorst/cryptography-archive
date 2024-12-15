/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpJPEG.c,v 1.2.8.1 1998/11/12 03:11:18 heller Exp $
____________________________________________________________________________*/

#if PGP_MACINTOSH
#include <FSp_fopen.h>
#endif

#include "jinclude.h"
#include "jpeglib.h"

#include "pgpErrors.h"
#include "pgpJPEG.h"
#include "pgpMem.h"
#include "pgpUtilities.h"

	static void
JPEGErrorExit(j_common_ptr cinfo)
{
	PGPError	err;
	
	/* Always display the message */
	(*cinfo->err->output_message) (cinfo);

	/* Let the memory manager delete any temp files before we die */
	jpeg_destroy(cinfo);

	pgpDebugMsg( "JPEGErrorExit(): Calling longjmp. Pray really hard..." );
	
pgpFixBeforeShip( "Use better errors here" );
	/* Should do real error mapping here */
	err = cinfo->err->msg_code;
	if( IsntPGPError( err ) )
		err = -1;
	
	if( cinfo->is_decompressor )
	{
		JPEGDecompressionContext	*context;
		
		context = (JPEGDecompressionContext *) cinfo;
		
		longjmp( context->exceptionBuffer, err );
	}
	else
	{
		JPEGCompressionContext	*context;
		
		context = (JPEGCompressionContext *) cinfo;
		
		longjmp( context->exceptionBuffer, err );
	}	
}

	static void
JPEGOutputMessage(j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];
	
	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

	pgpDebugMsg( buffer );
}

	PGPError
JPEGCreateCompress(JPEGCompressionContext *context)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_create_compress( &context->cInfo );
	}
	
	return( err );
}

	PGPError
JPEGCreateDecompress(JPEGDecompressionContext *context)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_create_decompress( &context->dInfo );
	}
	
	return( err );
}

	void
JPEGDestroyCompress(JPEGCompressionContext *context)
{
	pgpAssertAddrValid( context, JPEGCompressionContext );
	
	jpeg_destroy_compress( &context->cInfo );
}

	void
JPEGDestroyDecompress(JPEGDecompressionContext *context)
{
	pgpAssertAddrValid( context, JPEGDecompressionContext );
	
	jpeg_destroy_decompress( &context->dInfo );
}

	void
JPEGInitCompressionContext(JPEGCompressionContext *context)
{
	pgpAssertAddrValid( context, JPEGCompressionContext );
	
	pgpClearMemory( context, sizeof( *context ) );
	
	context->cInfo.err 				= jpeg_std_error( &context->errMgr );
	context->errMgr.error_exit		= JPEGErrorExit;
	context->errMgr.output_message	= JPEGOutputMessage;
}

	void
JPEGInitDecompressionContext(JPEGDecompressionContext *context)
{
	pgpAssertAddrValid( context, JPEGDecompressionContext );
	
	pgpClearMemory( context, sizeof( *context ) );
	
	context->dInfo.err 				= jpeg_std_error( &context->errMgr );
	context->errMgr.error_exit		= JPEGErrorExit;
	context->errMgr.output_message	= JPEGOutputMessage;
}

	void
JPEGCloseCompressionFile(JPEGCompressionContext *context)
{
	if( IsntNull( context->destFile ) )
	{
		fclose( context->destFile );
		context->destFile = FALSE;
	}
}

	void
JPEGCloseDecompressionFile(JPEGDecompressionContext *context)
{
	if( IsntNull( context->sourceFile ) )
	{
		fclose( context->sourceFile );
		context->sourceFile = FALSE;
	}
}

	static PGPError
OpenFile(
	PGPFileSpecRef	fileSpecRef,
	PGPBoolean		readOnly,
	FILE			**fileRef)
{
	PGPError	err;
	
	*fileRef = NULL;

#if PGP_MACINTOSH	/* [ */
	{
		FSSpec	fileSpec;
		
		err = PGPGetFSSpecFromFileSpec( fileSpecRef, &fileSpec );
		if( IsntPGPError( err ) )
		{
			*fileRef = FSp_fopen( &fileSpec, readOnly ? "rb" : "wb" );
		}
	}
#else	/* ][ */
	{
		char	*path;
		
		err = PGPGetFullPathFromFileSpec( fileSpecRef, &path );
		if( IsntPGPError( err ) )
		{
			*fileRef = fopen( path, readOnly ? "rb" : "wb" );
			
			PGPFreeData( path );
		}
	}
#endif	/* ] */

	if( IsntPGPError( err ) && IsNull( *fileRef ) )
		err = kPGPError_FileNotFound;

	return( err );
}
	
	PGPError
JPEGOpenCompressionFile(
	JPEGCompressionContext 	*context,
	PGPFileSpecRef			fileSpecRef)
{
	PGPError	err = kPGPError_NoErr;

	pgpAssert( IsNull( context->destFile ) );

	err = OpenFile( fileSpecRef, FALSE, &context->destFile );
	if( IsntPGPError( err ) )
	{
		err = setjmp( context->exceptionBuffer );
		if( err == kPGPError_NoErr )
		{
			jpeg_stdio_dest( &context->cInfo, context->destFile );
		}
		else
		{
			JPEGCloseCompressionFile( context );
		}
	}
	else
	{
		err = kPGPError_FileNotFound;
	}

	return( err );
}

	PGPError
JPEGOpenDecompressionFile(
	JPEGDecompressionContext 	*context,
	PGPFileSpecRef				fileSpecRef,
	PGPBoolean					readOnly)
{
	PGPError	err = kPGPError_NoErr;
	
	pgpAssert( IsNull( context->sourceFile ) );
	
	err = OpenFile( fileSpecRef, readOnly, &context->sourceFile );
	if( IsntPGPError( err ) )
	{
		err = setjmp( context->exceptionBuffer );
		if( err == kPGPError_NoErr )
		{
			jpeg_stdio_src( &context->dInfo, context->sourceFile );
		}
		else
		{
			JPEGCloseDecompressionFile( context );
		}
	}
	else
	{
		err = kPGPError_FileNotFound;
	}

	return( err );
}

	PGPError
JPEGReadHeader(
	JPEGDecompressionContext 	*context,
	PGPBoolean 					requireImage)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		int	jpegResult;
		
		jpegResult = jpeg_read_header( &context->dInfo, requireImage );
		if( jpegResult != JPEG_HEADER_OK )
		{
			err = kPGPError_CorruptData;
		}
	}
	
	return( err );
}

	PGPError
JPEGStartCompress(
	JPEGCompressionContext 	*context,
	PGPBoolean				writeAllTables)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_start_compress( &context->cInfo, writeAllTables );
	}
	
	return( err );
}

	PGPError
JPEGStartDecompress(JPEGDecompressionContext *context)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_start_decompress( &context->dInfo );
	}
	
	return( err );
}

	PGPError
JPEGFinishCompress(JPEGCompressionContext *context)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_finish_compress( &context->cInfo );
	}
	
	return( err );
}

	PGPError
JPEGFinishDecompress(JPEGDecompressionContext *context)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_finish_decompress( &context->dInfo );
	}
	
	return( err );
}

	PGPError
JPEGSetDefaults(JPEGCompressionContext *context)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_set_defaults( &context->cInfo );
	}
	
	return( err );
}

	PGPError
JPEGWriteScanlines(
	JPEGCompressionContext 	*context,
	JSAMPARRAY 				scanlines,
	JDIMENSION 				maxScanlines,
	JDIMENSION 				*numWrittenScanlines)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		*numWrittenScanlines = jpeg_write_scanlines( &context->cInfo,
									scanlines, maxScanlines );
	}
	else
	{
		*numWrittenScanlines = 0;
	}
	
	return( err );
}

	PGPError
JPEGReadScanlines(
	JPEGDecompressionContext 	*context,
	JSAMPARRAY 					scanlines,
	JDIMENSION 					maxScanlines,
	JDIMENSION 					*numReadScanlines)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		*numReadScanlines = jpeg_read_scanlines( &context->dInfo,
								scanlines, maxScanlines);
	}
	else
	{
		*numReadScanlines = 0;
	}
	
	return( err );
}

	PGPError
JPEGCopyCriticalParameters(
	JPEGDecompressionContext 	*dContext,
	JPEGCompressionContext 		*cContext)
{
	PGPError	err;
	
	err = setjmp( dContext->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_copy_critical_parameters( &dContext->dInfo, &cContext->cInfo );
	}
	
	return( err );
}

	PGPError
JPEGSetColorSpace(
	JPEGCompressionContext 	*context,
	J_COLOR_SPACE 			colorspace)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_set_colorspace( &context->cInfo, colorspace );
	}
	
	return( err );
}

	PGPError
JPEGDefaultColorSpace(JPEGCompressionContext *context)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_default_colorspace( &context->cInfo );
	}
	
	return( err );
}

	PGPError
JPEGSetQuality(
	JPEGCompressionContext 	*context,
	PGPInt32				 quality,
	PGPBoolean				forceBaseline)
{
	PGPError	err;
	
	err = setjmp( context->exceptionBuffer );
	if( IsntPGPError( err ) )
	{
		jpeg_set_quality( &context->cInfo, quality, forceBaseline );
	}
	
	return( err );
}


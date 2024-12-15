/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Implementation of FileIO based on a FILE *.  This is a concrete subclass
	that can be instantiated using a FILE *.

	$Id: pgpStdFileIO.c,v 1.13 1997/09/18 01:34:17 lloyd Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include <stdio.h>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpStdFileIOPriv.h"

#define PGP_USE_FILE_SPEC_PRIV
#include "pgpFileSpecPriv.h"


#define GetParentVTBL()		pgpFileIOGetClassVTBL()

#define GetSelfVTBL( ref ) \
	(PGPStdFileIOVtbl const *)pgpioGetObjectVTBL( ref )


	PGPBoolean
PGPStdFileIOIsValid( PGPStdFileIORef self )
{
	if ( ! PGPFileIOIsValid( (PGPFileIORef)self ) )
		return( FALSE );
	
	return( self->stdFileIOMagic == kPGPStdFileIOMagic );
}



	PGPError
PGPNewStdFileIO(
	PFLContextRef		context,
	FILE *				stdioFILE,
	Boolean				autoClose,
	PGPStdFileIORef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	PGPIORef			newRef;
	PGPStdFileIOData	data;
	
	data.autoClose	= autoClose;
	data.stdioFILE	= stdioFILE;
	
	err	= pgpNewIOFromVTBL( context,
			(PGPIOVtbl const *)pgpStdFileIOGetClassVTBL(),
			sizeof( PGPStdFileIO ), &data, &newRef );
	
	*outRef	= (PGPStdFileIORef)newRef;
	return( err );
}


	static PGPError
sStdFileIOInitProc(
	PGPIORef	inRef,
	void *		data )
{
	PGPError					err		= kPGPError_NoErr;
	PGPStdFileIORef				self	= (PGPStdFileIORef)inRef;
	const PGPStdFileIOData *	myData	= (PGPStdFileIOData *)data;
	
	PGPValidateIO( inRef );
	
	err	= pgpioInheritInit( inRef, (PGPIOVtbl const *)GetParentVTBL(), data );
	
	self->stdFileIOMagic	= kPGPStdFileIOMagic;
	self->stdioFILE			= myData->stdioFILE;
	self->autoClose			= myData->autoClose;
	
	return( err );
}



	static PGPError
sStdFileIODestroyProc( PGPIORef	inRef )
{
	PGPError		err	= kPGPError_NoErr;
	PGPStdFileIORef	self	= (PGPStdFileIORef)inRef;
	int				closeResult;

	PGPValidateStdFileIO( self );
	PGPValidatePtr( self->stdioFILE );
	
	closeResult	= fclose( self->stdioFILE );
	self->stdioFILE	= NULL;
	
	self->stdFileIOMagic	= 0;
	
	err	= pgpioInheritDestroy( inRef, (PGPIOVtbl const *)GetParentVTBL() );
	
	if ( closeResult != 0 )
		err	= kPGPError_FileOpFailed;
	
	return( err );
}




	static PGPError
sStdFileSetPosProc(
	PGPIORef		ref,
	PGPFileOffset	newPos )
{
	PGPError		err	= kPGPError_NoErr;
	PGPStdFileIORef	self	= (PGPStdFileIORef)ref;
	int			result;
	fpos_t		tempPos;
	
	PGPValidatePtr( self->stdioFILE );
	
	tempPos	= newPos;
	result	= fsetpos( self->stdioFILE, &tempPos );
	if ( result != 0 )
	{
		err	= kPGPError_FileOpFailed;
	}
	
	return( err );
}




	static PGPError
sStdFileIOReadProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	void *			buffer,
	PGPSize *		bytesRead )
{
	PGPError		err		= kPGPError_NoErr;
	PGPStdFileIORef	self	= (PGPStdFileIORef)ref;
	size_t			result;
	
	PGPValidatePtr( self->stdioFILE );
	
	result	= fread( buffer, 1, requestCount, self->stdioFILE );
	if ( result != requestCount )
	{
		err	= kPGPError_EOF;
	}
	*bytesRead	= result;
	
	return( err );
}

						
	static PGPError
sStdFileIOWriteProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	const void *	buffer )
{
	PGPError		err		= kPGPError_NoErr;
	PGPStdFileIORef	self	= (PGPStdFileIORef)ref;
	size_t			result;
	
	PGPValidatePtr( self->stdioFILE );
	
	result	= fwrite( buffer, 1, requestCount, self->stdioFILE );
	if ( result != requestCount )
	{
		err	= kPGPError_WriteFailed;
	}
	
	return( err );
}

						
	static PGPError
sStdFileIOGetEOFProc(
	PGPIORef		ref,
	PGPFileOffset *	eof )
{
	PGPError		err	= kPGPError_NoErr;
	PGPStdFileIORef	self	= (PGPStdFileIORef)ref;
	FILE *			stdioFILE	= self->stdioFILE;
	fpos_t			startPos;
	fpos_t			eofPos;
	int				fResult;
	
	*eof	= 0;
	PGPValidatePtr( self->stdioFILE );
	
	/* save current file position */
	fResult	= fgetpos( stdioFILE, &startPos );
	if ( fResult == 0 )
	{
		/* seek to the end of the file so we can get its length */
		fResult	= fseek( stdioFILE, 0, SEEK_END );
		if ( fResult == 0 )
			fResult	= fgetpos( stdioFILE, &eofPos);
		
		if ( fResult != 0 )
			err	= kPGPError_FileOpFailed;
			
		/* restore previous file position */
		fResult	= fsetpos( stdioFILE, &startPos );
		if ( fResult != 0 )
			err	= kPGPError_FileOpFailed;
	}
	else
	{
		err	= kPGPError_FileOpFailed;
	}
	
	
	*eof	= eofPos;
	return( err );
}




#if PGPIO_EOF
	static PGPError
sChangeFileSize(
	FILE *			file,
	PGPFileOffset	newEOF)
{
	PGPError	err	= kPGPError_NoErr;
	
	(void)file;
	(void)newEOF;
#if defined( PGP_UNIX )

#elif defined( PGP_WIN32 )

#elif defined( PGP_MACINTOSH )

#endif
	return( err );
}



	static PGPError
sStdFileIOSetEOFProc(
	PGPIORef		ref,
	PGPFileOffset 	newEOF )
{
	PGPError		err	= kPGPError_IllegalFileOp;
	PGPStdFileIORef	self	= (PGPStdFileIORef)ref;
	FILE *			stdioFILE	= self->stdioFILE;
	
	PGPValidatePtr( stdioFILE );
	
	err	= sChangeFileSize( stdioFILE, newEOF);
	if ( IsntPGPError( err ) )
	{
		/* stdio buffers may now be whacked */
		/* how to flush them? */
		pgpDebugMsg( "flush buffers" );
	}

	return( err );
}

#endif
					
	static PGPError
sStdFileIOFlushProc( PGPIORef		ref)
{
	PGPError		err		= kPGPError_NoErr;
	PGPStdFileIORef	self	= (PGPStdFileIORef)ref;
	
	PGPValidatePtr( self->stdioFILE );
	if ( fflush( self->stdioFILE ) != 0 )
	{
		err	= kPGPError_FileOpFailed;
	}
	
	return( err );
}




	PGPStdFileIOVtbl const *
pgpStdFileIOGetClassVTBL()
{
	static PGPStdFileIOVtbl	sVTBL	= { {{NULL,}} };
	PGPFileIOVtbl *	parent	= &sVTBL.parentVTBL;
	
	if ( IsNull( parent->parentVTBL.initProc ) )
	{
		/* get default routines */
		*parent	= *pgpFileIOGetClassVTBL();
		
		parent->parentVTBL.initProc		= sStdFileIOInitProc;
		parent->parentVTBL.destroyProc	= sStdFileIODestroyProc;
		
		parent->parentVTBL.readProc		= sStdFileIOReadProc;
		parent->parentVTBL.writeProc	= sStdFileIOWriteProc;
		
		parent->parentVTBL.setPosProc	= sStdFileSetPosProc;
		
		parent->parentVTBL.getEOFProc	= sStdFileIOGetEOFProc;
		
#if PGPIO_EOF
		parent->parentVTBL.setEOFProc	= sStdFileIOSetEOFProc;
#endif
		
		parent->parentVTBL.flushProc	= sStdFileIOFlushProc;
	}
	
	return( &sVTBL );
}


























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

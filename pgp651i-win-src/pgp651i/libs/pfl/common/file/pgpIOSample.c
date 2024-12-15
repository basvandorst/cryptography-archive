/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Sample code for using PGPIO.
	
	$Id: pgpIOSample.c,v 1.3 1999/03/10 02:51:42 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include <string.h>

#include "pgpFileUtilities.h"
#include "pgpPFLErrors.h"
#include "pgpMemoryIO.h"
#include "pgpIOUtilities.h"

					
/*____________________________________________________________________________
	Create a PGPMemoryIO from specified data and position at beginning of IO.
____________________________________________________________________________*/
	static PGPError
sCreateIOFromData(
	PGPMemoryMgrRef 	memoryMgr,
	const PGPByte *		data,
	PGPSize				numBytes,
	PGPIORef *	outRef )
{
	PGPIORef		ioRef	= NULL;
	PGPError		err		= kPGPError_NoErr;

	err	= PGPNewMemoryIO( memoryMgr, (PGPMemoryIORef *)&ioRef );
	if ( IsntPGPError( err ) )
	{
		/* write our string into our memory I/O */
		err	= PGPIOWrite( ioRef, numBytes, data );
		if ( IsntPGPError( err ) )
		{
			/* reposition at beginning */
			err	= PGPIOSetPos( ioRef, 0 );
		}
		
		/* clean up if we got an error */
		if ( IsPGPError( err ) )
		{
			PGPFreeIO( ioRef );
			ioRef	= NULL;
		}
	}
	
	*outRef	= ioRef;
	return( err );
}


/*____________________________________________________________________________
	Create and open a test IO which uses the specified file.
____________________________________________________________________________*/
	static PGPError
sOpenTestFile(
	PGPMemoryMgrRef 	memoryMgr,
	PGPIORef *			outRef )
{
	PGPError		err	= kPGPError_NoErr;
	PFLFileSpecRef	fileSpec	= NULL;
	PGPIORef		ioRef		= NULL;
	
#if PGP_MACINTOSH
	FSSpec	fsSpec;
	
	(void)FSMakeFSSpec( -1, fsRtDirID, "\pPGPIOTestFile", &fsSpec );
	err	= PFLNewFileSpecFromFSSpec( memoryMgr, &fsSpec, &fileSpec );
#else
	/* create a file spec from a path string */
	const char	kPath[]	= "C:\\PGPIOTestFile.bin";
	err	= PFLNewFileSpecFromFullPath( memoryMgr, kPath, &fileSpec );
#endif

	if ( IsntPGPError( err ) )
	{
		/* delete it in case it's already there */
		(void)PFLFileSpecDelete( fileSpec );
		
		/* create a new file */
		err	= PFLFileSpecCreate( fileSpec );
		if ( IsntPGPError( err ) )
		{
			err	= PGPOpenFileSpec( fileSpec,
					kPFLFileOpenFlags_ReadWrite, (PGPFileIORef *)&ioRef );
		}
		
		PFLFreeFileSpec( fileSpec );
	}
	
	*outRef	= ioRef;
	return( err );
}


/*____________________________________________________________________________
	Write a string into a PGPMemoryIO, then copy that into a file.
____________________________________________________________________________*/
	static PGPError
SampleCode( PGPMemoryMgrRef 	memoryMgr )
{
	const char		kTestString[]	= "hello world";
	const char		kPath[]			= "C:\\pgpIOTest.bin";
	
	PGPError		err	= kPGPError_NoErr;
	PGPIORef		inRef	= NULL;
	PGPIORef		outRef	= NULL;
	
	/* create a memory IO which has some stuff in it */
	err	= sCreateIOFromData( memoryMgr,
			(PGPByte const *)kTestString, strlen( kTestString ), &inRef );
	if ( IsntPGPError( err ) )
	{
		/* open a test file to write into */
		err	= sOpenTestFile( memoryMgr, &outRef );
	}
	
	if ( IsntPGPError( err ) )
	{
		PGPFileOffset	length;
		
		/* see how big input is */
		err	= PGPIOGetEOF( inRef, &length );
		if ( IsntPGPError( err ) )
		{
			/* copy entire input to output */
			err	= PGPCopyIO( inRef, length, outRef );
		}
	}
	
	if ( outRef != NULL )
		PGPFreeIO( outRef );
	if ( inRef != NULL )
		PGPFreeIO( inRef );
	
	return( err );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

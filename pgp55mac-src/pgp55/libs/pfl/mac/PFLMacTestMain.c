/*____________________________________________________________________________
	Sample main program to demonstrate Debug
____________________________________________________________________________*/
#include "MacDebug.h"
#include "MacDebugPatches.h"
#include "pgpMem.h"
#include "MacMemory.h"
#include "MacStrings.h"

#include "pflContext.h"
#include "pgpStdFileIO.h"
#include "pgpProxyIO.h"
#include "pgpMemoryIO.h"
#include "pgpFileUtilities.h"

static void	TestIO( void );

	static void
InitMac()
{
	UnloadScrap();
	
	InitGraf( &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();
	MaxApplZone();
}


	void
main(void)
	{
	InitMac();
	
	DebugPatches_PatchDisposeTraps();
	
	pgpLeaksBeginSession( "main" );
	
	TestIO();
	
	pgpLeaksEndSession( );
	}
	
	
	
	static void
TestFileSpec(
	PFLContextRef	context,
	const FSSpec *	spec )
{
	PFLFileSpecRef	specRef;
	PGPError		err	= kPGPError_NoErr;
	
	err	= PFLNewFileSpecFromFSSpec( context, spec, &specRef );
	if ( IsntPGPError( err ) )
	{
		PFLFileSpecRef	tempSpec;
		PGPByte *		outData	= NULL;
		PGPSize			outSize;
		char *			outName	= NULL;
		
		err	= PFLCopyFileSpec( specRef, &tempSpec );
		pgpAssertNoErr( err );
		
		err	= PFLFreeFileSpec( tempSpec );
		pgpAssertNoErr( err );
		tempSpec	= NULL;
		
		context	= PFLGetFileSpecContext( specRef );
		pgpAssertNoErr( err );
		
		err	= PFLGetFileSpecName( specRef, &outName );
		pgpAssertNoErr( err );
		PFLContextMemFree( context, outName );
		err	= PFLSetFileSpecName( specRef, "hello there" );
		pgpAssertNoErr( err );
		err	= PFLGetFileSpecName( specRef, &outName );
		pgpAssertNoErr( err );
		PFLContextMemFree( context, outName );
		
		err	= PFLExportFileSpec( specRef, &outData, &outSize );
		pgpAssertNoErr( err );
		
		err	= PFLFreeFileSpec( specRef );
		pgpAssertNoErr( err );
		specRef	= NULL;
		
		err	= PFLImportFileSpec( context, outData, outSize, &specRef );
		pgpAssertNoErr( err );
		
		PFLContextMemFree( context, outData );
		
		
		err	= PFLFreeFileSpec( specRef );
		pgpAssertNoErr( err );
	}
	pgpAssertNoErr( err );
}



	static PGPError
TestActualIO( PGPIORef	io )
{
	#define kTestSize	1025
	#define kTestLoops	1024
	PGPUInt32	loops;
	void *		buffer	= NULL;
	void *		writeBuffer	= NULL;
	void *		readBuffer	= NULL;
	PGPError	err	= kPGPError_NoErr;
	PGPFileOffset	curPos;
	
	err	= PGPIOSetPos( io, 0);
	pgpAssertNoErr( err );
	err	= PGPIOGetEOF( io, &curPos );
	
	buffer	= pgpAlloc( kTestSize * 2 );
	if ( IsNull( buffer ) )
		return( kPGPError_OutOfMemory );
	pgpClearMemory( buffer, kTestSize * 2);
	
	writeBuffer	= buffer;
	readBuffer	= (char *)buffer + kTestSize;
	
	loops	= kTestLoops;
	while ( loops-- != 0 )
	{
		err	= PGPIOWrite( io, kTestSize, writeBuffer );
		pgpAssertNoErr( err );
		
	}
	err	= PGPIOGetPos( io, &curPos );
	pgpAssert( curPos == kTestSize * kTestLoops );
	err	= PGPIOGetEOF( io, &curPos );
	pgpAssert( curPos == kTestSize * kTestLoops );
	
	/* reposition at beginning of file and read all the data,
	 * making sure it's the same as what we wrote */
	err		= PGPIOSetPos( io, 0 );
	pgpAssertNoErr( err );
	err	= PGPIOGetPos( io, &curPos );
	pgpAssertNoErr( err );
	pgpAssert( curPos == 0 );
	
	loops	= kTestLoops;
	while ( loops-- != 0 )
	{
		PGPSize	bytesRead;
		
		pgpFillMemory( readBuffer, kTestSize, 0xDD );
		
		err	= PGPIORead( io, kTestSize, readBuffer, &bytesRead );
		pgpAssertNoErr( err );
		pgpAssert( bytesRead == kTestSize );
		pgpAssert( pgpMemoryEqual( readBuffer, writeBuffer, kTestSize ) );
		
	}
	
	pgpFree( buffer );
	
	return( err );
}



	static void
TestIOWithSpec(
	PFLContextRef	context,
	const FSSpec *	fsSpec )
{
	PFLFileSpecRef	specRef;
	PGPError		err	= kPGPError_NoErr;
	
	(void)FSpCreate( fsSpec, 'PFLt', 'Junk', smSystemScript );
	
	err	= PFLNewFileSpecFromFSSpec( context, fsSpec, &specRef );
	if ( IsntPGPError( err ) )
	{
		PGPFileIORef		ioRef	= NULL;
		PGPProxyIORef		proxyIO;
		
		err	= PGPOpenFileSpec( specRef, kPFLFileOpenFlags_ReadWrite, &ioRef );
		pgpAssertNoErr( err );
		
		if ( IsntPGPError( err ) )
		{
			err	= PFLFreeFileSpec( specRef );
			pgpAssertNoErr( err );
		}
		
		if ( IsntPGPError( err ) )
		{
			err	= TestActualIO( (PGPIORef) ioRef );
			pgpAssertNoErr( err );
			err	= TestActualIO( (PGPIORef) ioRef );
			pgpAssertNoErr( err );
		}
		
		/* now test it with a proxy */
		err	= PGPNewProxyIO( (PGPIORef)ioRef, FALSE, &proxyIO );
		if ( IsntPGPError( err ) )
		{
			err	= TestActualIO( (PGPIORef) proxyIO );
			pgpAssertNoErr( err );
			PGPFreeIO( (PGPIORef)proxyIO );
			proxyIO	= NULL;
		}
		
		PGPFreeIO( (PGPIORef)ioRef );
	}
	pgpAssertNoErr( err );
}


	static void
TestMemoryIO( PFLContextRef	context)
{
	PGPMemoryIORef		memoryRef	= NULL;
	PGPError		err	= kPGPError_NoErr;
	
	err	= PGPNewMemoryIO( context, &memoryRef );
	pgpAssertNoErr( err );
	
	if ( IsntPGPError( err ) )
	{
		err	= TestActualIO( (PGPIORef) memoryRef );
		pgpAssertNoErr( err );
		err	= TestActualIO( (PGPIORef) memoryRef );
		pgpAssertNoErr( err );
	}
	
	PGPFreeIO( (PGPIORef)memoryRef );
}




	static void
TestIO( void )
{
	PGPError		err	= kPGPError_NoErr;
	PFLContextRef	context;
	
	err	= PFLNewContext( &context );
	if ( IsntPGPError( err ) )
	{
		FSSpec	spec;
		
		spec.vRefNum	= -1;
		spec.parID		= fsRtDirID;
		CopyPString( "\pPFL Test\r", spec.name );
		
		TestFileSpec( context, &spec );
		TestIOWithSpec( context, &spec );
		TestMemoryIO( context );
		
		PFLFreeContext( context );
	}
}

















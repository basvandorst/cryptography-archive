/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Sample code for using PGPIO
	
	$Id: pgpIOUtilities.c,v 1.4 1999/03/10 02:51:43 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpPFLErrors.h"
#include "pgpMem.h"
#include "pgpIOUtilities.h"


/*____________________________________________________________________________
	Copy bytes from one I/O to another using supplied buffer. 
____________________________________________________________________________*/
	PGPError
PGPCopyIOUsingBuffer(
	PGPIORef	fromIO,
	PGPSize		numBytes,
	PGPIORef	toIO,
	void *		buffer,
	PGPSize		bufferSize )
{
	PGPSize		remaining;
	PGPError	err	= kPGPError_NoErr;
	
	remaining	= numBytes;
	while ( remaining != 0 )
	{	
		PGPSize	count;
		
		count	= pgpMin( remaining, bufferSize );
		err	= PGPIORead( fromIO, count, buffer, NULL);
		if ( IsPGPError( err ) )
			break;
		
		err	= PGPIOWrite( toIO, count, buffer );
		if ( IsPGPError( err ) )
			break;
		
		remaining	-= count;
	}
	
	return( err );
}

/*____________________________________________________________________________
	Copy bytes from one I/O to another. 
	Caller will want to call PGPIOSetPos to set file mark first.
____________________________________________________________________________*/
	PGPError
PGPCopyIO(
	PGPIORef	fromIO,
	PGPSize		numBytes,
	PGPIORef	toIO )
{
	void *			buffer	= NULL;
	PGPSize			bufferSize;
	PGPError		err	= kPGPError_NoErr;
	
	/* a reasonable cross-platform size */
	#define kBufferSize		( 32UL * 1024UL )
	
	bufferSize	= pgpMin( numBytes, kBufferSize );
	buffer		= PGPNewData( PGPIOGetMemoryMgr( fromIO ), bufferSize, 0);
	if ( IsntNull( buffer ) )
	{
		err	= PGPCopyIOUsingBuffer( fromIO, numBytes, toIO,
				buffer, bufferSize );
			
		PGPFreeData( buffer );
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	return( err );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

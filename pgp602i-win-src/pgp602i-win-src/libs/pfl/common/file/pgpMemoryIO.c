/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	

	$Id: pgpMemoryIO.c,v 1.7.16.1 1998/11/12 03:18:25 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include <stdio.h>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpMemoryIOPriv.h"


#define GetParentVTBL()		pgpIOGetClassVTBL()

#define GetSelfVTBL( ref ) \
	(PGPMemoryIOVtbl const *)pgpioGetObjectVTBL( ref )

#define CallReallocate( ref, vtbl, ptr, size) \
		((vtbl)->reallocateProc)( ref, ptr, size)
		

	PGPBoolean
PGPMemoryIOIsValid( PGPMemoryIORef self )
{
	if ( ! PGPIOIsValid( (PGPIORef)self ) )
		return( FALSE );
	
	return( self->memoryIOMagic == kPGPMemoryIOMagic );
}




	PGPError
PGPNewMemoryIO(
	PGPMemoryMgrRef		context,
	PGPMemoryIORef *	outRef )
{
	PGPError	err	= kPGPError_NoErr;
	PGPIORef		newRef;
	PGPMemoryIOData	data;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	
	data.useFixedBuffer	= FALSE;
	
	err	= pgpNewIOFromVTBL( context,
		(PGPIOVtbl const *)pgpMemoryIOGetClassVTBL(),
			sizeof( PGPMemoryIO ), (void *)&data, &newRef );
	
	*outRef	= (PGPMemoryIORef)newRef;
	return( err );
}


	PGPError
PGPNewMemoryIOFixedBuffer(
	PGPMemoryMgrRef		context,
	void *				buffer,
	PGPSize				bufferSize,
	PGPMemoryIORef *	outRef )
{
	PGPError	err	= kPGPError_NoErr;
	PGPIORef		newRef;
	PGPMemoryIOData	data;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidatePtr( buffer );
	
	data.useFixedBuffer	= TRUE;
	data.buffer			= buffer;
	data.bufferSize		= bufferSize;
	
	err	= pgpNewIOFromVTBL( context,
		(PGPIOVtbl const *)pgpMemoryIOGetClassVTBL(),
			sizeof( PGPMemoryIO ), (void *)&data, &newRef );
	
	*outRef	= (PGPMemoryIORef)newRef;
	return( err );
}


	static PGPError
sMemoryIOInitProc(
	PGPIORef	inRef,
	void *		data )
{
	PGPError				err		= kPGPError_NoErr;
	PGPMemoryIORef			self	= (PGPMemoryIORef)inRef;
	const PGPMemoryIOData *	myData= (PGPMemoryIOData *)data;
#define kDefaultBufferSize		512UL
	
	PGPValidateIO( inRef );
	
	err	= pgpioInheritInit( inRef, (PGPIOVtbl const *)GetParentVTBL(), NULL );
	
	self->memoryIOMagic	= kPGPMemoryIOMagic;
	self->buffer		= NULL;
	self->bufferSize	= kDefaultBufferSize;
	self->ownsBuffer	= TRUE;
	self->eof			= 0;
	self->canResize		= ! myData->useFixedBuffer;
	
	if ( myData->useFixedBuffer )
	{
		self->buffer		= myData->buffer;
		self->bufferSize	= myData->bufferSize;
		self->eof			= myData->bufferSize;
		self->ownsBuffer	= FALSE;
	}
	else
	{
		/* allocate a small buffer for starters */
		err		= CallReallocate( self, GetSelfVTBL( self ),
			&self->buffer, kDefaultBufferSize );
	}
		
	
	return( err );
}



	static PGPError
sMemoryIODestroyProc( PGPIORef	inRef )
{
	PGPError		err	= kPGPError_NoErr;
	PGPMemoryIORef	self	= (PGPMemoryIORef)inRef;

	PGPValidateMemoryIO( self );
	
	if ( IsntNull( self->buffer ) && self->ownsBuffer )
	{
		pgpClearMemory( self->buffer, self->bufferSize );
		PGPFreeData( self->buffer );
	}
	self->buffer		= NULL;
	
	self->memoryIOMagic	= ~self->memoryIOMagic;
	
	err	= pgpioInheritDestroy( inRef, (PGPIOVtbl const *)GetParentVTBL() );
	
	return( err );
}




	static PGPError
sMemoryIOReadProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	void *			buffer,
	PGPSize *		bytesRead )
{
	PGPMemoryIORef	self	= (PGPMemoryIORef)ref;
	PGPError		err		= kPGPError_NoErr;
	PGPFileOffset	offset	= ref->offset;
	PGPSize			bytesAvailable	= 0;
	PGPSize			actualCount;
	
	PGPValidateMemoryIO( self );
	
	pgpAssert( IsntNull( self->buffer ) );
	if ( offset <= (PGPFileOffset)self->eof )
	{
		bytesAvailable	= (PGPSize)(self->eof - offset);
	}
	
	if ( bytesAvailable < requestCount )
		actualCount	=  bytesAvailable;
	else
		actualCount	= requestCount;
		
	pgpCopyMemory( (char *)self->buffer + offset, buffer, actualCount );
	
	if ( actualCount!= requestCount )
		err	= kPGPError_EOF;
	*bytesRead	= actualCount;
	
	return( err );
}

	
	static PGPError
sSetBufferSize(
	PGPMemoryIORef	self,
	PGPSize			newSize )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpAssert( self->canResize );
		
	if ( self->canResize)
	{
		err	= CallReallocate( self, GetSelfVTBL( self ),
				&self->buffer, newSize);
		if ( IsntPGPError( err ) )
		{
			self->bufferSize	= newSize;
		}
	}
	else
	{
		err	= kPGPError_IllegalFileOp;
	}
	
	return( err );
}


	static PGPError
sGrowBuffer(
	PGPMemoryIORef	self,
	PGPSize			minimumSize )
{
	PGPSize		allocSize;
	PGPError	err	= kPGPError_NoErr;
	
	if ( ! self->canResize )
		return( kPGPError_DiskFull );
		
	/* grow existing size proportionally, or to minimum */
	allocSize	= self->bufferSize;
	allocSize	+= allocSize / 2;
	if ( allocSize < minimumSize )
		allocSize	= minimumSize;
	
	/* grow the buffer to the size needed plus 1/8 more */
	err	= sSetBufferSize( self, allocSize);
	if ( IsPGPError( err ) )
	{
		/* try again with minimum size */
		allocSize	= minimumSize;
		err	= sSetBufferSize( self, allocSize );
	}
	if ( IsntPGPError( err ) )
	{
		self->bufferSize	= allocSize;
	}
	return( err );
}


					
	static PGPError
sMemoryIOWriteProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	const void *	buffer )
{
	PGPMemoryIORef	self	= (PGPMemoryIORef)ref;
	PGPError		err	= kPGPError_NoErr;
	PGPFileOffset	offset	= ref->offset;
	PGPSize			bufferSizeNeeded;
	
	PGPValidateMemoryIO( self );
	
	bufferSizeNeeded	= (PGPSize)(offset + requestCount);
	if ( bufferSizeNeeded > self->bufferSize )
	{
		err	= sGrowBuffer( self, bufferSizeNeeded );
	}
	
	if ( IsntPGPError( err ) )
	{
		pgpCopyMemory( buffer, (char *)self->buffer + offset, requestCount );
		
		if ( offset + requestCount > self->eof )
			self->eof	= (PGPSize)(offset + requestCount);
	}
	
	
	return( err );
}

						
	static PGPError
sMemoryIOGetEOFProc(
	PGPIORef		ref,
	PGPFileOffset *	eof )
{
	PGPMemoryIORef	self	= (PGPMemoryIORef)ref;
	
	PGPValidateMemoryIO( self );
	
	*eof	= self->eof;
	return( kPGPError_NoErr );
}


#if PGPIO_EOF
	static PGPError
sMemoryIOSetEOFProc(
	PGPIORef		ref,
	PGPFileOffset 	newEOF )
{
	PGPMemoryIORef	self	= (PGPMemoryIORef)ref;
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidateMemoryIO( self );
	
	if ( self->canResize )
	{
		err	= sSetBufferSize( self, newEOF );
	}
	else
	{
		/* buffer cannot be changed */
		if ( self->eof > self->bufferSize )
		{
			err	= kPGPError_IllegalFileOp;
		}
	}
	
	if ( IsntPGPError( err ) )
	{
		self->eof	= newEOF;
	}
	
	return( kPGPError_NoErr );
}
#endif


	static PGPError
sMemoryIOReallocateProc(
	PGPMemoryIORef		self,
	void **				ptr,
	PGPSize				newSize )
{
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidateMemoryIO( self );
	
	/* *ptr may be NULL, which is OK */
	err	= PGPReallocData( self->parent.context, ptr, newSize, 0);
	
	return( err );
}



	PGPMemoryIOVtbl const *
pgpMemoryIOGetClassVTBL()
{
	static PGPMemoryIOVtbl	sVTBL	= { {NULL,}, NULL, };
	PGPIOVtbl *				parent	= &sVTBL.parentVTBL;
	
	if ( IsNull( parent->initProc ) )
	{
		/* get default routines */
		*parent	= *pgpIOGetClassVTBL();
		
		parent->initProc	= sMemoryIOInitProc;
		parent->destroyProc	= sMemoryIODestroyProc;
		
		parent->readProc	= sMemoryIOReadProc;
		parent->writeProc	= sMemoryIOWriteProc;
		
		parent->getEOFProc	= sMemoryIOGetEOFProc;
#if PGPIO_EOF
		parent->setEOFProc	= sMemoryIOSetEOFProc;
#endif
		
		sVTBL.reallocateProc	= sMemoryIOReallocateProc;
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

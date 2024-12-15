/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Abstract base class for all PGPLineEndIO objects.

	$Id: pgpLineEndIO.c,v 1.2 1997/09/07 00:28:43 lloyd Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpPFLErrors.h"

#include "pgpLineEndIOPriv.h"
#include "pgpMem.h"


#define GetParentVTBL()		pgpProxyIOGetClassVTBL()

#define GetSelfVTBL( ref ) \
	(PGPIOVtbl const *)pgpLineEndIOGetObjectVTBL( ref )

static PGPError		sLineEndIOFlushProc( PGPIORef ref );

	PGPBoolean
PGPLineEndIOIsValid( PGPLineEndIORef ref )
{
	if ( ! PGPIOIsValid( (PGPIORef)ref ) )
		return( FALSE );
	
	return( ref->lineEndConvertMagic == kPGPLineEndIOMagic );
}





	static PGPError
sLineEndIOInitProc(
	PGPIORef	inRef,
	void *		data )
{
	PGPError			err		= kPGPError_NoErr;
	PGPLineEndIORef		self	= (PGPLineEndIORef)inRef;
	PGPLineEndIOData *	myData	= (PGPLineEndIOData *)data;
	
	PGPValidateIO( inRef );
	
	err	= pgpioInheritInit( inRef,
			(PGPIOVtbl const *)GetParentVTBL(), data );
	
	self->lineEndConvertMagic	= kPGPLineEndIOMagic;
	self->toType		= myData->toType;
	self->lastCharWasCR	= FALSE;
	
	self->buffer		= self->bufferSpace;
	self->bufferSize	= sizeof( self->bufferSpace );
	self->bytesInBuffer	= 0;
	
	return( err );
}



	static PGPError
sLineEndIODestroyProc( PGPIORef	inRef )
{
	PGPError		err		= kPGPError_NoErr;
	PGPError		err2	= kPGPError_NoErr;
	PGPLineEndIORef	self	= (PGPLineEndIORef)inRef;

	PGPValidateLineEndIO( self );
	
	/* don't use virtual method */
	err	= sLineEndIOFlushProc( inRef );
	
	self->lineEndConvertMagic	= ~self->lineEndConvertMagic;
	
	err2	= pgpioInheritDestroy( inRef,
			(PGPIOVtbl const *)GetParentVTBL() );
	if ( IsntPGPError( err ) )
		err	= err2;
		
	
	return( err );
}


	static PGPError
sLineEndIOReadProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	void *			buffer,
	PGPSize *		actualCount )
{
	(void)ref;
	(void)requestCount;
	(void)buffer;
	
	pgpClearMemory( buffer, requestCount );
	*actualCount	= 0;
	
	/* we don't support reading yet (not implemented) */
	return( kPGPError_IllegalFileOp );
}


	static PGPError
AddToBuffer(
	PGPLineEndIORef		self,
	char				theChar )
{
	PGPError	err;
	
	self->buffer[ self->bytesInBuffer ]	= theChar;
	self->bytesInBuffer	+= 1;
	if ( self->bytesInBuffer == self->bufferSize )
	{
		err	= PGPIOWrite( self->parent.baseIO, self->bufferSize, self->buffer );
		self->bytesInBuffer	= 0;
	}
	else
	{
		err	= kPGPError_NoErr;
	}
	
	return( err );
}

	
#define IsLineEndChar( c )	((c) == '\r' || (c) == '\n' )
	static PGPError
sLineEndIOWriteProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	const void *	buffer )
{
	PGPSize				remaining;
	const char *		cur;
	PGPLineEndIORef		self	= (PGPLineEndIORef)ref;
	PGPError			err	= kPGPError_NoErr;
	PGPInt32			delta	= 0;
	
	if ( requestCount == 0 )
		return( kPGPError_NoErr );
	
	remaining	= requestCount;
	cur			= (const char *)buffer;
	
	if ( self->lastCharWasCR &&
		*(char const *)buffer == '\n' )
	{
		/* eat line feed; it followed CR */
		++cur;
		--remaining;
		--delta;
	}

	while ( remaining != 0 )
	{
		char	curChar;
		
		curChar	= *cur++;
		--remaining;
		
		if ( IsLineEndChar( curChar ) )
		{
			/* eat newline if it follows carriage return */
			if ( remaining != 0  &&
				curChar == '\r' &&
				*cur == '\n' )
			{
				++cur;
				--remaining;
				--delta;
			}
			
			/* output correct line end */
			if ( self->toType == kCRLFLineEndType )
			{
				err	= AddToBuffer( self, '\r' );
				if ( IsntPGPError( err ) )
					err	= AddToBuffer( self, '\n' );
				++delta;
			}
			else if ( self->toType == kCRLineEndType)
			{
				err	= AddToBuffer( self, '\r' );
			}
			else
			{
				err	= AddToBuffer( self, '\n' );
			}
		}
		else
		{
			err	= AddToBuffer( self, curChar );
		}
	}
	
	/* must remember this for next write which starts with '\n' */
	self->lastCharWasCR	=
		((const char *)buffer)[ requestCount - 1 ] == '\r';
	
	/* PGPIOWrite function will add the request count in;
		we add in the delta bytes to account for more, fewer we output */
	if ( IsntPGPError( err ) )
		ref->offset	+= delta;
		
	return( err );
}

				
	static PGPError
sLineEndIOFlushProc( PGPIORef		ref)
{
	PGPError		err		= kPGPError_NoErr;
	PGPLineEndIORef	self	= (PGPLineEndIORef)ref;
	
	if ( self->bytesInBuffer != 0 )
	{
		err	= PGPIOWrite( self->parent.baseIO,
				self->bytesInBuffer, self->buffer );
		self->bytesInBuffer	= 0;
	}
	if ( IsntPGPError( err ) )
	{
		err	= PGPIOFlush( self->parent.baseIO );
	}
	
	return( err );
}


/*____________________________________________________________________________
	We need to override this because we may output a different number of bytes
	than we read.
____________________________________________________________________________*/
	static PGPError
sLineEndIOGetPosProc(
	PGPIORef		ref,
	PGPFileOffset *	curPos)
{
	PGPLineEndIORef		self	= (PGPLineEndIORef)ref;
	
	return( PGPIOGetPos( self->parent.baseIO, curPos ) );
}


	static PGPError
sLineEndIOSetPosProc(
	PGPIORef		ref,
	PGPFileOffset	newPos )
{
	PGPLineEndIORef		self	= (PGPLineEndIORef)ref;
	PGPError			err	= kPGPError_NoErr;
	
	PGPValidateIO( ref );
	PGPValidateParam( newPos >= 0 );
	
	self->lastCharWasCR	= FALSE;
	
	err	= PGPIOSetPos( self->parent.baseIO, newPos );
	if ( IsntPGPError( err ) )
	{
		ref->offset	= newPos;
	}
	
	return( err );
}




	PGPLineEndIOVtbl const *
pgpLineEndIOGetClassVTBL()
{
	static PGPLineEndIOVtbl	sVTBL	= { {{NULL,},} };
	PGPLineEndIOVtbl *			vtbl	= &sVTBL;
	
	if ( IsNull( vtbl->parentVTBL.parentVTBL.initProc ) )
	{
		PGPProxyIOVtbl *	parent	= &vtbl->parentVTBL;
		
		/* get default routines */
		*parent	= *pgpProxyIOGetClassVTBL();
		parent->parentVTBL.initProc		= sLineEndIOInitProc;
		parent->parentVTBL.destroyProc	= sLineEndIODestroyProc;
		parent->parentVTBL.readProc		= sLineEndIOReadProc;
		parent->parentVTBL.writeProc	= sLineEndIOWriteProc;
		parent->parentVTBL.getPosProc	= sLineEndIOGetPosProc;
		parent->parentVTBL.setPosProc	= sLineEndIOSetPosProc;
		parent->parentVTBL.flushProc	= sLineEndIOFlushProc;
	}
	
	return( vtbl );
}



	PGPError
PGPNewLineEndIO(
	PGPIORef			baseIO,
	PGPBoolean			ownsBase,
	ConvertLineEndType	toType,
	PGPLineEndIORef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	PGPIORef			newRef;
	PGPLineEndIOData	data;
	
	data.parentData.ownsBaseIO	= ownsBase;
	data.parentData.baseIO		= baseIO;
	
	data.toType	= toType;
	
	err	= pgpNewIOFromVTBL( baseIO->context,
			(PGPIOVtbl const *)pgpLineEndIOGetClassVTBL(),
			sizeof( PGPLineEndIO ), &data, &newRef );
	
	*outRef	= (PGPLineEndIORef)newRef;
	return( err );
}


























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

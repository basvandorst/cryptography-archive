/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	

	$Id: pgpProxyIO.c,v 1.7.34.1 1998/11/12 03:18:27 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpProxyIOPriv.h"


#define GetParentVTBL()		pgpIOGetClassVTBL()

#define GetSelfVTBL( ref ) \
	(PGPProxyIOVtbl const *)pgpioGetObjectVTBL( ref )


	PGPBoolean
PGPProxyIOIsValid( PGPProxyIORef self )
{
	if ( ! PGPIOIsValid( (PGPIORef)self ) )
		return( FALSE );
	
	return( self->proxyIOMagic == kPGPProxyIOMagic );
}



	PGPError
PGPNewProxyIO(
	PGPIORef		baseIO,
	PGPBoolean		ownsBaseIO,
	PGPProxyIORef *	outRef )
{
	PGPError	err	= kPGPError_NoErr;
	PGPIORef		newRef;
	PGPProxyIOData	data;
	
	PGPValidateIO( baseIO );
	
	data.ownsBaseIO	= ownsBaseIO;
	data.baseIO		= baseIO;
	
	err	= pgpNewIOFromVTBL( baseIO->context,
		(PGPIOVtbl const *)pgpProxyIOGetClassVTBL(),
			sizeof( PGPProxyIO ), (void *)&data, &newRef );
	
	*outRef	= (PGPProxyIORef)newRef;
	return( err );
}


	static PGPError
sProxyIOInitProc(
	PGPIORef	inRef,
	void *		data )
{
	PGPError		err		= kPGPError_NoErr;
	PGPProxyIORef	self	= (PGPProxyIORef)inRef;
	const PGPProxyIOData *	myData= (PGPProxyIOData *)data;
	
	PGPValidateIO( inRef );
	PGPValidateIO( myData->baseIO );
	
	err	= pgpioInheritInit( inRef, (PGPIOVtbl const *)GetParentVTBL(), NULL );
	
	self->proxyIOMagic	= kPGPProxyIOMagic;
	self->baseIO		= myData->baseIO;
	self->ownsBaseIO	= myData->ownsBaseIO;
	
	return( err );
}



	static PGPError
sProxyIODestroyProc( PGPIORef	inRef )
{
	PGPError		err	= kPGPError_NoErr;
	PGPProxyIORef	self	= (PGPProxyIORef)inRef;

	PGPValidateProxyIO( self );
	
	if ( self->ownsBaseIO )
	{
		PGPFreeIO( self->baseIO );
	}
	self->baseIO		= NULL;
	
	self->proxyIOMagic	= 0;
	
	err	= pgpioInheritDestroy( inRef, (PGPIOVtbl const *)GetParentVTBL() );
	
	return( err );
}




	static PGPError
sProxySetPosProc(
	PGPIORef		ref,
	PGPFileOffset	newPos )
{
	PGPProxyIORef	self	= (PGPProxyIORef)ref;
	
	return( PGPIOSetPos( self->baseIO, newPos ) );
}




	static PGPError
sProxyIOReadProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	void *			buffer,
	PGPSize *		bytesRead )
{
	PGPProxyIORef	self	= (PGPProxyIORef)ref;
	PGPError		err	= kPGPError_NoErr;
	
	err	= PGPIORead( self->baseIO, requestCount, buffer, bytesRead );
	
	return( err );
}

						
	static PGPError
sProxyIOWriteProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	const void *	buffer )
{
	PGPProxyIORef	self	= (PGPProxyIORef)ref;
	PGPError		err	= kPGPError_NoErr;
	
	err	= PGPIOWrite( self->baseIO, requestCount, buffer );
	
	return( err );
}

						
	static PGPError
sProxyIOGetEOFProc(
	PGPIORef		ref,
	PGPFileOffset *	eof )
{
	PGPProxyIORef	self	= (PGPProxyIORef)ref;
	
	return( PGPIOGetEOF( self->baseIO, eof ) );
}


					
	static PGPError
sProxyIOFlushProc( PGPIORef		ref)
{
	PGPError		err		= kPGPError_NoErr;
	PGPProxyIORef	self	= (PGPProxyIORef)ref;
	
	err	= PGPIOFlush( self->baseIO );
	
	return( err );
}



	PGPProxyIOVtbl const *
pgpProxyIOGetClassVTBL()
{
	static PGPProxyIOVtbl	sVTBL	= { {NULL,} };
	PGPIOVtbl *				parent	= &sVTBL.parentVTBL;
	
	if ( IsNull( parent->initProc ) )
	{
		/* get default routines */
		*parent	= *pgpIOGetClassVTBL();
		
		parent->initProc	= sProxyIOInitProc;
		parent->destroyProc	= sProxyIODestroyProc;
		
		parent->readProc	= sProxyIOReadProc;
		parent->writeProc	= sProxyIOWriteProc;
		
		parent->setPosProc	= sProxySetPosProc;
		
		parent->getEOFProc	= sProxyIOGetEOFProc;
		
		parent->flushProc	= sProxyIOFlushProc;
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

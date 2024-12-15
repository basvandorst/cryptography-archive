/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Abstract superclass for all PGPIO objects.  PGPIO objects form a class
	hierarchy which implements C++ style inheritance and virtual functions.
	Subclasses can override parent methods and can inherit as part of their
	own implementation.
	
	Each PGPIO object type consists of three files:
		pgpXXXIO.c			code for the class
		pgpXXXIO.h			public API for the class
		pgpXXXIOPriv.h		private class implementation details
	
	Note that the pgpXXXIOPriv.h file contents should *only* be used
	by subclasses.  It is off-limits to clients.

	$Id: pgpIO.c,v 1.13 1997/10/09 23:54:10 lloyd Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpPFLErrors.h"
#include "pgpIOPriv.h"
#include "pgpMem.h"

/* note: convention used here is that static functions begin with "s" */



	PFLContextRef
PGPIOGetContext( PGPIORef ref )
{
	return( ref->context );
}



#define CallInit( ref, vtbl, data ) \
		((vtbl)->initProc)( ref, data )
		
#define CallDestroy( ref, vtbl ) \
		((vtbl)->destroyProc)( ref )
		
#define CallRead( ref, vtbl, reqCount, buffer, bytesRead ) \
		((vtbl)->readProc)( ref, reqCount, buffer, bytesRead )
		
#define CallWrite( ref, vtbl, reqCount, buffer ) \
		((vtbl)->writeProc)( ref, reqCount, buffer )
		
#define CallGetPos( ref, vtbl, pos ) \
		((vtbl)->getPosProc)( ref, pos )
		
#define CallSetPos( ref, vtbl, pos ) \
		((vtbl)->setPosProc)( ref, pos )
		
#define CallGetEOF( ref, vtbl, pos ) \
		((vtbl)->getEOFProc)( ref, pos )
		
#if PGPIO_EOF
#define CallSetEOF( ref, vtbl, pos ) \
		((vtbl)->setEOFProc)( ref, pos )
#endif
		
#define CallFlush( ref, vtbl ) \
		((vtbl)->flushProc)( ref )


	PGPError
pgpioInheritInit(
	PGPIORef			ref,
	PGPIOVtbl const *	vtbl,
	void *				data )
{
	pgpAssert( PGPIOIsValid( ref ) && IsntNull( vtbl ) );
	return( CallInit( ref, vtbl, data ) );
}

	PGPError
pgpioInheritDestroy(
	PGPIORef			ref,
	PGPIOVtbl const *	vtbl )
{
	pgpAssert( PGPIOIsValid( ref ) && IsntNull( vtbl ) );
	return( CallDestroy( ref, vtbl ) );
}


	PGPError
pgpioInheritRead(
	PGPIORef			ref,
	PGPIOVtbl const *	vtbl,
	PGPSize				requestCount,
	void *				buffer,
	PGPSize *			bytesRead )
{
	pgpAssert( PGPIOIsValid( ref ) && IsntNull( vtbl ) );
	return( CallRead( ref, vtbl, requestCount, buffer, bytesRead ) );
}


	PGPError
pgpioInheritWrite(
	PGPIORef			ref,
	PGPIOVtbl const *	vtbl,
	PGPSize				requestCount,
	const void *		buffer )
{
	pgpAssert( PGPIOIsValid( ref ) && IsntNull( vtbl ) );
	return( CallWrite( ref, vtbl, requestCount, buffer ) );
}


	PGPError
pgpioInheritSetPos(
	PGPIORef			ref,
	PGPIOVtbl const *	vtbl,
	PGPFileOffset		newPos )
{
	pgpAssert( PGPIOIsValid( ref ) && IsntNull( vtbl ) );
	return( CallSetPos( ref, vtbl, newPos ) );
}


	PGPError
pgpioInheritGetEOF(
	PGPIORef			ref,
	PGPIOVtbl const *	vtbl,
	PGPFileOffset *		eof )
{
	pgpAssert( PGPIOIsValid( ref ) && IsntNull( vtbl ) );
	return( CallGetEOF( ref, vtbl, eof ) );
}


#if PGPIO_EOF
	PGPError
pgpioInheritSetEOF(
	PGPIORef			ref,
	PGPIOVtbl const *	vtbl,
	PGPFileOffset 		eof )
{
	pgpAssert( PGPIOIsValid( ref ) && IsntNull( vtbl ) );
	return( CallSetEOF( ref, vtbl, eof ) );
}
#endif


	PGPError
pgpioInheritFlush(
	PGPIORef			ref,
	PGPIOVtbl const *	vtbl)
{
	pgpAssert( PGPIOIsValid( ref ) && IsntNull( vtbl ) );
	return( CallFlush( ref, vtbl ) );
}

				
				
				
	PGPBoolean
PGPIOIsValid( PGPIORef ref )
{
	return( IsntNull( ref ) &&
		ref->magic == kPGPIOMagic &&
		IsntNull( ref->vtbl ) );
}


/*____________________________________________________________________________
	Create a new PGPIO object.  The vtbl and object size are specified so
	that subclasses can use this routine.
____________________________________________________________________________*/
	PGPError
pgpNewIOFromVTBL(
	PFLContextRef		context,
	PGPIOVtbl const *	vtbl,
	PGPSize				size,
	void *				data,
	PGPIORef *			outRef )
{
	PGPError	err	= kPGPError_NoErr;
	PGPIORef	newRef;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PFLValidateContext( context );
	
	newRef	= (PGPIORef)PFLContextMemAlloc( context,
				size, kPFLMemoryFlags_Clear );
	if ( IsntNull( newRef ) )
	{
		/* use cast so we can assign to this const * const field */
		*(PGPIOVtbl const **)(&newRef->vtbl)		= vtbl;
		
		newRef->context		= context;
		newRef->magic		= kPGPIOMagic;
		/* initialize remaining fields in sInitProc */
		
		err	= CallInit( newRef, newRef->vtbl, data );
		if ( IsPGPError( err ) )
		{
			CallDestroy( newRef, newRef->vtbl);
			PFLContextMemFree( context, newRef );
			newRef	= NULL;
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outRef	= newRef;
	return( err );
}


	PGPError
PGPFreeIO( PGPIORef ref )
{
	PGPError	err		= kPGPError_NoErr;
	
	PGPValidateIO( ref );
	
	err	= CallDestroy( ref, ref->vtbl);
	
	PFLContextMemFree( ref->context, ref );
	
	return( err );
}


	PGPError
PGPIORead(
	PGPIORef	ref,
	PGPSize		requestCount,
	void *		buffer,
	PGPSize *	bytesReadOut )
{
	PGPError	err	= kPGPError_NoErr;
	PGPSize		bytesRead	= 0;
	
	if ( IsntNull( bytesReadOut ) )
	{
		*bytesReadOut	= 0;
	}
	PGPValidateIO( ref );
	PGPValidateParam( requestCount != 0 );
	PGPValidatePtr( buffer );
	
	err	= CallRead( ref, ref->vtbl, requestCount, buffer, &bytesRead );
	if ( IsntPGPError( err ) )
		ref->offset	+= bytesRead;
	
	if ( IsntNull( bytesReadOut ) )
		*bytesReadOut	= bytesRead;
	
	return( err );
}
						
	PGPError
PGPIOWrite(
	PGPIORef		ref,
	PGPSize			requestCount,
	const void *	buffer )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidateIO( ref );
	PGPValidateParam( requestCount != 0 );
	PGPValidatePtr( buffer );
	
	err	= CallWrite( ref, ref->vtbl, requestCount, buffer );
	
	if ( IsntPGPError( err ) )
		ref->offset	+= requestCount;
	
	return( err );
}					


/*____________________________________________________________________________
	The real position is *always stored* in this object.  The position
	in the underlying implementation should be kept in sync, but not
	relied upon.
	
	Read & Write update the position accordingly.
____________________________________________________________________________*/
	PGPError
PGPIOSetPos(
	PGPIORef		ref,
	PGPFileOffset	newPos )
{
	PGPValidateIO( ref );
	PGPValidateParam( newPos >= 0 );
	
	ref->offset	= newPos;
	
	return( CallSetPos( ref, ref->vtbl, newPos ) );
}


/*____________________________________________________________________________
	The real position is *always stored* in this object.  The position
	in the underlying implementation is irrelevant.
	
	Subclasses may override in unusual circumstances.
____________________________________________________________________________*/	
	PGPError
PGPIOGetPos(
	PGPIORef		ref,
	PGPFileOffset *	curPos )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( curPos );
	*curPos	= 0;
	PGPValidateIO( ref );
	
	err	= CallGetPos( ref, ref->vtbl, curPos );
	
	return( err );
}				
						
	PGPError
PGPIOGetEOF(
	PGPIORef		ref,
	PGPFileOffset *	eof )
{
	PGPValidatePtr( eof );
	*eof	= 0;
	PGPValidateIO( ref );
	
	return( CallGetEOF( ref, ref->vtbl, eof ) );
}


#if PGPIO_EOF
	PGPError
PGPIOSetEOF(
	PGPIORef		ref,
	PGPFileOffset 	newEOF )
{
	PGPError		err	= kPGPError_NoErr;
	PGPFileOffset	curPos;
	
	PGPValidateIO( ref );

	
	/* it's illegal to make the eof less than the current pos */
	err	= PGPIOGetPos( ref, &curPos );
	if ( IsPGPError( err ))
		return( err );
	if ( curPos > newEOF )
		return( kPGPError_IllegalFileOp );
	
	/* not implemented properly pgpStdFileIO yet */
	pgpDebugMsg( "not yet implemented fully" );
	return( kPGPError_IllegalFileOp );
	
	return( CallSetEOF( ref, ref->vtbl, newEOF ) );
}
#endif




	PGPError
PGPIOFlush( PGPIORef ref )
{
	PGPValidateIO( ref );
	
	return( CallFlush( ref, ref->vtbl ) );
}


	static PGPError
sInitProc(
	PGPIORef	ref,
	void *		data )
{
	PGPError	err	= kPGPError_NoErr;
	
	(void)ref;
	(void)data;
	PGPValidateIO( ref );
	
	ref->offset		= 0;
	
	return( err );
}



	static PGPError
sDestroyProc( PGPIORef	ref )
{
	PGPError	err	= kPGPError_NoErr;

	PGPValidateIO( ref );
	
	/* nothing to destroy; caller release ref itself */
	
	return( err );
}


	PGPBoolean
PGPIOIsAtEOF( PGPIORef ref )
{
	PGPFileOffset	eofPos;
	PGPFileOffset	curPos;
	
	PGPIOGetEOF(ref, &eofPos);
	PGPIOGetPos(ref, &curPos);
	
	return( curPos >= eofPos );
}



	static PGPError
sReadProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	void *			buffer,
	PGPSize *		bytesRead )
{
	PGPError	err	= kPGPError_IllegalFileOp;
	
	(void)ref;
	(void)requestCount;
	(void)buffer;
	(void)bytesRead;
	
	return( err );
}


						
	static PGPError
sWriteProc(
	PGPIORef		ref,
	PGPSize			requestCount,
	const void *	buffer )
{
	PGPError	err	= kPGPError_IllegalFileOp;
	
	(void)ref;
	(void)requestCount;
	(void)buffer;
	
	return( err );
}


						
	static PGPError
sSetPosProc(
	PGPIORef		ref,
	PGPFileOffset	newPos )
{
	PGPError	err	= kPGPError_NoErr;
	
	/* already set by PGPIOSetPos() */
	(void)newPos;
	PGPValidateIO( ref );
	
	return( err );
}


						
	static PGPError
sGetPosProc(
	PGPIORef		ref,
	PGPFileOffset *	curPos )
{
	PGPError	err	= kPGPError_NoErr;
	
	*curPos	= ref->offset;
	
	return( err );
}


						
	static PGPError
sGetEOFProc(
	PGPIORef		ref,
	PGPFileOffset *	eof )
{
	PGPError	err	= kPGPError_IllegalFileOp;
	
	(void)ref;
	(void)eof;
	
	return( err );
}

					
#if PGPIO_EOF	
	static PGPError
sSetEOFProc(
	PGPIORef		ref,
	PGPFileOffset 	eof )
{
	PGPError	err	= kPGPError_IllegalFileOp;
	
	(void)ref;
	(void)eof;
	
	return( err );
}
#endif

	static PGPError
sFlushProc( PGPIORef	ref)
{
	(void)ref;
	/* default does nothing */
	return( kPGPError_NoErr );
}




	PGPIOVtbl const *
pgpIOGetClassVTBL(  )
{
	static const PGPIOVtbl	sVTBL =
	{
		sInitProc,
		sDestroyProc,
		sReadProc,
		sWriteProc,
		sGetPosProc,
		sSetPosProc,
		sGetEOFProc,
#if PGPIO_EOF
		sSetEOFProc,
#endif
		sFlushProc
	};
	
	return( &sVTBL );
}


























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

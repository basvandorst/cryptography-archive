/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Abstract base class for all PGPFileIO objects.

	$Id: pgpFileIO.c,v 1.3 1997/07/17 01:33:03 lloyd Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpPFLErrors.h"
#include "pgpFileIOPriv.h"
#include "pgpMem.h"


#define GetParentVTBL()		pgpIOGetClassVTBL()

#define GetSelfVTBL( ref ) \
	(PGPFileIOVtbl const *)pgpioGetObjectVTBL( ref )


	PGPBoolean
PGPFileIOIsValid( PGPFileIORef ref )
{
	if ( ! PGPIOIsValid( (PGPIORef)ref ) )
		return( FALSE );
	
	return( ref->fileIOMagic == kPGPFileIOMagic );
}





	static PGPError
sFileIOInitProc(
	PGPIORef	inRef,
	void *		data )
{
	PGPError		err		= kPGPError_NoErr;
	PGPFileIORef	self	= (PGPFileIORef)inRef;
	
	PGPValidateIO( inRef );
	
	err	= pgpioInheritInit( inRef, GetParentVTBL(), data );
	
	self->fileIOMagic	= kPGPFileIOMagic;
	self->autoClose		= TRUE;
	
	return( err );
}



	static PGPError
sFileIODestroyProc( PGPIORef	inRef )
{
	PGPError		err	= kPGPError_NoErr;
	PGPFileIORef	self	= (PGPFileIORef)inRef;

	PGPValidateFileIO( self );
	
	self->fileIOMagic	= 0;
	
	err	= pgpioInheritDestroy( inRef, GetParentVTBL() );
	
	return( err );
}


	PGPFileIOVtbl const *
pgpFileIOGetClassVTBL()
{
	static PGPFileIOVtbl	sVTBL	= { {NULL,} };
	PGPFileIOVtbl *			vtbl	= &sVTBL;
	
	if ( IsNull( vtbl->parentVTBL.initProc ) )
	{
		PGPIOVtbl *	parent	= &vtbl->parentVTBL;
		
		/* get default routines */
		*parent	= *pgpIOGetClassVTBL();
		parent->initProc	= sFileIOInitProc;
		parent->destroyProc	= sFileIODestroyProc;
	}
	
	return( vtbl );
}


























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

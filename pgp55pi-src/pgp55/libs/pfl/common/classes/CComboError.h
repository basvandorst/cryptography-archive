/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Class intended to solve the problem of distinguishing a PGPError from
	an os-specific error or other errors.

	$Id: CComboError.h,v 1.5 1997/09/15 02:37:53 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_ComboError_h	/* [ */
#define Included_ComboError_h

#include "pgpPFLErrors.h"

#if PGP_MACINTOSH
#include "MacErrors.h"
#endif


/*____________________________________________________________________________
	The field "PGPError" contains a PGPError value as defined in PFL and CDK.
	
	The field "err" contains an operating system-dependent error code.  Both
	can exist with different values at the same time. Simply assign to the
	appropriate field and then use the supplied function to test for errors.
	
	Sample Usage:
	CComboError		err;
	
	err.pgpErr	= PGPNewContext( .. );
	if ( err.IsntError() )
		err.err		= FSWrite( ... );
	
	if ( err.IsError() )
		return( err );
		
Routine overview:
	CComboError		(no args) initializes both error values to no error
	CComboError		creates a new with specified error
	
	IsError			returns TRUE if either error value contains an error
	IsntError		returns TRUE if neither error value contains an error
	
	IsCancelError	returns TRUE if either error value contains a cancel error
	
	HavePGPError	returns TRUE if pgpErr is not kPGPError_NoErr
	HaveNonPGPError	returns TRUE if err is not zero (kNoErr)
					both HavePGPError and HaveNonPGPError can be TRUE
	
____________________________________________________________________________*/


class CComboError
{
public:
	PGPError	pgpErr;
	PGPError	err;	/* platform specific error */
	
	enum	ComboErrorType{ kPlatformError, kPGPError };
	enum 	{ kNoErr	= 0 };
					
				CComboError( void )
				{
					pgpErr	= kPGPError_NoErr;
					err		= kNoErr;
				}

				/* no type difference between a PGPError, OSStatus, int
				so we need an enum type to distinguis in the constructor */
				CComboError( ComboErrorType type, SInt32 errCode )
				{
					if ( type == kPlatformError )
					{
						err		= errCode;
						pgpErr	= kPGPError_NoErr;
					}
					else
					{
						pgpErr	= errCode;
						err		= kNoErr;
					}
				}
	
	
	virtual PGPBoolean	IsError( void )
				{
					return( IsPGPError( pgpErr ) || err != kNoErr );
				}
				
	virtual PGPBoolean	IsntError( void )
				{
					return( ! IsError() );
				}
				
	virtual PGPBoolean	IsCancelError( void )
				{
					if ( pgpErr == kPGPError_UserAbort && err == kNoErr )
						return( TRUE );
				#if PGP_MACINTOSH
					if( err == userCanceledErr && pgpErr == kPGPError_NoErr )
						return( TRUE );
				#endif
					return( FALSE );
				}
	
	/* unusual, but both HavePGPError() and HaveNonPGPError() can be TRUE */
	PGPBoolean	HavePGPError( void )
				{
					return( pgpErr != kPGPError_NoErr );
				}
	
	virtual PGPBoolean	HaveNonPGPError( void )
				{
					return( err != 0 );
				}
	
#if PGP_MACINTOSH
		virtual PGPError
	ConvertToPGPError( void )
	{
		if ( HavePGPError() )
			return( pgpErr );
		else if ( IsError() )
			return( MacErrorToPGPError( err ) );
		else
			return( kPGPError_NoErr );
	}
#endif		
};





#endif /* ] Included_ComboError_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Standard implementation of a file specification.  Assumes system uses
	full paths of the form /a/b/c or \a\b\c.

	$Id: pgpFileSpecStd.h,v 1.7 1997/09/11 05:38:29 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpFileSpecStd_h	/* [ */
#define Included_pgpFileSpecStd_h


/* this file is included by pgpFileSpec.h and shouldn't be included directly */
#ifndef _pgpFileSpecIncludeRightStuff_
#error "don't include this file directly.  Use pgpFileSpec.h instead."
#endif


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


#if ! PGP_MACINTOSH /* [ */

PGPError 	PFLNewFileSpecFromFullPath( PFLContextRef context,
					char const * path, PFLFileSpecRef * outRef );


PGPError  	PFLGetFullPathFromFileSpec(PFLConstFileSpecRef fileRef,
						char **fullPathPtr);
						
#endif	/* ] */



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif	/* ] Included_pgpFileSpecStd_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

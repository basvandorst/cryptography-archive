/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	Standard implementation of a file specification.  Assumes system uses
	full paths of the form /a/b/c or \a\b\c.

	$Id: pgpFileSpecStd.h,v 1.8.16.1 1998/11/12 03:18:14 heller Exp $
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

PGPError 	PFLNewFileSpecFromFullPath( PGPMemoryMgrRef memoryMgr,
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

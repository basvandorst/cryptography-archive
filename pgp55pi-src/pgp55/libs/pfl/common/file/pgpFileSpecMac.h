/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Mac implementation of a file specification.  Uses FSSpec as its
	location.

	$Id: pgpFileSpecMac.h,v 1.6 1997/09/11 05:38:29 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpFileSpecMac_h	/* [ */
#define Included_pgpFileSpecMac_h

/* this file is included by pgpFileSpec.h and
	shouldn't be included directly */
#ifndef _pgpFileSpecIncludeRightStuff_
#error don't include this file directly.  Use pgpFileSpec.h instead.
#endif


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



typedef struct PFLFileSpecMacMetaInfo
{
	FInfo	fInfo;
} PFLFileSpecMacMetaInfo;

PGPError 	PFLNewFileSpecFromFSSpec( PFLContextRef context,
					const FSSpec * spec, PFLFileSpecRef * outRef );
					
PGPError 	PFLGetFSSpecFromFileSpec( PFLConstFileSpecRef ref,
					FSSpec * spec );



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif	/* ] Included_pgpFileSpecMac_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

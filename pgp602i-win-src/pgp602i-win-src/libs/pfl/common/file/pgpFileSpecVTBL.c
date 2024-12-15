/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpFileSpecVTBL.c,v 1.2.34.1 1998/11/12 03:18:15 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpMem.h"

#define PGP_USE_FILE_SPEC_PRIV	1
#include "pgpFileSpecMacPriv.h"
#include "pgpFileSpecStdPriv.h"
#include "pgpFileSpecVTBL.h"

/*____________________________________________________________________________
	This routine is needed in order to reconstruct a FileSpec that has been
	exported and now needs to be imported and formed into a valid object.
	
	Since the object doesn't exist, we don't have its vtbl and since we
	don't have its vtbl, we can't tell it to create itself.  So this forms
	the glue that allows this to happen.
____________________________________________________________________________*/
	const PGPFileSpecVTBL *
pgpGetFileSpecVTBLForType( PFLFileSpecType type )
{
	const PGPFileSpecVTBL *vtbl	= NULL;
	
	switch( type )
	{
		default:	vtbl	= NULL;	break;
		
#if PGP_MACINTOSH		/* [ */
		case kPFLFileSpecMacType:		vtbl	= GetFileSpecVTBLMac();	break;
#else					/* ] PGP_MACINTOSH [ */
		case kPFLFileSpecFullPathType:	vtbl	= GetFileSpecVTBLStd();	break;
#endif					/* ] */
	}
	
	pgpAssert( IsntNull( vtbl ) );
	return( vtbl );
}































/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
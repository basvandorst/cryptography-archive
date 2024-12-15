/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpMacFileMapping.h,v 1.5 1999/03/10 02:53:05 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpMacmapping_h	/* [ */
#define Included_pgpMacmapping_h

#include "pgpBase.h"


PGP_BEGIN_C_DECLARATIONS


#if PGP_MACINTOSH	/* [ */

/* determine if turning off MacBinary is appropriate for this file */
PGPBoolean	pgpOKToEncodeFSSpecWithoutMacBinary( const FSSpec *spec );

/* map actual file data into mac creator/type pair */
PGPError	pgpMapFileDataToMacCreatorType( const void *data,
				OSType *creator, OSType *type );
				

/* map file name extension into mac creator/type pair */
/* name is a pascal string */
PGPError	pgpMapFileNameToMacCreatorType( const unsigned char *name,
				OSType *creator, OSType *type );


#else /* ] PGP_MACINTOSH [ */

typedef PGPUInt32	OSType;

#endif	/* ] PGP_MACINTOSH */


/* map a creator/type pair to a file name extension */
/* extension is of the form "doc", "bin", etc + null char */
PGPError	pgpMapMacCreatorTypeToExtension( OSType creator, OSType type,
				char	extension[ 3 + 1 ] );

PGPBoolean	pgpIsValidExtensionForMacType( OSType creator, OSType type,
				char const * extension );
				


PGPError	pgpInitMacBinaryMappings( void );
PGPError	pgpDisposeMacBinaryMappings( void );

PGP_END_C_DECLARATIONS



#endif /* ] Included_pgpMacmapping_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
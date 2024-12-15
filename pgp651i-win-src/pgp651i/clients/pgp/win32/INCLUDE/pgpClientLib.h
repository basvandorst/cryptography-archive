/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpClientLib.h,v 1.4 1999/03/10 21:31:35 pbj Exp $
____________________________________________________________________________*/

#ifndef Included_pgpClientLib_h	/* [ */
#define Included_pgpClientLib_h

#include "pgpPubTypes.h"
#include "pgpUtilities.h"
#include "pgpOptionList.h"

#include "pflPrefs.h"

PGP_BEGIN_C_DECLARATIONS

enum
{
	kPGPSelect509CanSignOnly		= (1L << 0),
	kPGPSelect509NoSplit			= (1L << 1),
	kPGPSelect509CAOnly				= (1L << 2),

	kPGPSelect509Default = 0
};

	PGPError
PGPX509CertFromExport(
	PGPContextRef		context,
	PGPUInt32			uX509Alg,
	PGPByte*			expkeyidX509,
	PGPByte*			pIASNX509,
	PGPUInt32			uX509IASNLength,
	PGPKeySetRef		keyset,
	PGPKeyRef*			pkeyX509,
	PGPSigRef*			psigX509 );

	PGPError
PGPX509CertToExport(
	PGPContextRef		context,
	PGPKeyRef			keyCert,
	PGPSigRef			sigCert,
	PGPUInt32*			puX509Alg,
	PGPByte*			pexpkeyidX509,
	PGPByte**			ppIASNX509,
	PGPUInt32*			puX509IASNLength );


/* Caller must free data with PGPFreeData */

	PGPError
PGPAVPairsToData(
	PGPMemoryMgrRef		memoryMgr, 
	PGPAttributeValue*	avPairs, 
	PGPUInt32			numAV, 
	PGPByte**			data,
	PGPSize*			dataSize);


/* Caller must supply function to free data allocated for the avPairs
   array */

	PGPError 
PGPDataToAVPairs(
	PGPMemoryMgrRef		memoryMgr, 
	PGPByte*			data,
	PGPSize				dataSize, 
	PGPAttributeValue**	avPairs, 
	PGPUInt32*			numAV);

/* Caller must call PGPFreeData on any non-null pointer returned */

	PGPError 
PGPParseCommonName(
	PGPMemoryMgrRef	memoryMgr, 
	char*			commonname, 
	char**			pfirstname,
	char**			pmiddlename,
	char**			plastname);


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpClientLib_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

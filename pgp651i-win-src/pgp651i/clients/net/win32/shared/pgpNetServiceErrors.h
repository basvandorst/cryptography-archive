/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: pgpNetServiceErrors.h,v 1.8 1999/05/18 08:49:16 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_PGPnetServiceError_h	/* [ */
#define Included_PGPnetServiceError_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

enum PGPnetServiceError_
{
	kPGPnetSrvcError_NoErr				= 0,
	
	kPGPnetSrvcError_NoAuthPassphrase	= 1,	
	kPGPnetSrvcError_NoAuthKey			= 2,
	kPGPnetSrvcError_NTon95				= 3,
	kPGPnetSrvcError_ReadConfig			= 4,
	kPGPnetSrvcError_NoNetwork			= 5,
	kPGPnetSrvcError_NoDriver			= 6,
	kPGPnetSrvcError_DrvEvent			= 7,
	kPGPnetSrvcError_DrvSharedMemory	= 8,
	kPGPnetSrvcError_DrvCommunication	= 9,
	kPGPnetSrvcError_SAFailed			= 10,
	kPGPnetSrvcError_95onNT				= 11,
	kPGPnetSrvcError_AuthenticatedKey	= 12,
	kPGPnetSrvcError_AuthenticatedCert	= 13,
	kPGPnetSrvcError_RevokedCert		= 14,
	kPGPnetSrvcError_ExpiredCert		= 15,
	kPGPnetSrvcError_RevokedKey			= 16,
	kPGPnetSrvcError_ExpiredKey			= 17,

	PGP_ENUM_FORCE( PGPnetServiceError_ )
};
PGPENUM_TYPEDEF( PGPnetServiceError_, PGPnetServiceError );
PGP_END_C_DECLARATIONS

#endif // Included_PGPnetServiceError_h

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

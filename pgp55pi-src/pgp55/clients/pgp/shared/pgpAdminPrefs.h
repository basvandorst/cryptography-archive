/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpAdminPrefs.h,v 1.8 1997/10/15 21:36:54 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpAdminPrefs_h	/* [ */
#define Included_pgpAdminPrefs_h


#define PGP_ADMINPREFVERSION	1

typedef enum _PGPAdminPrefsBool
{
	kPGPAdminPrefBoolFirst						=	0,
	kPGPPrefUseOutgoingADK = kPGPAdminPrefBoolFirst,
	kPGPPrefUseDHADK							=	1,
	kPGPPrefEnforceIncomingADK					=	2,
	kPGPPrefEnforceMinChars						=	3,
	kPGPPrefEnforceMinQuality					=	4,
	kPGPPrefWarnNotCertByCorp					=	5,
	kPGPPrefAutoSignTrustCorp					=	6,
	kPGPPrefMetaIntroducerCorp					=	7,
	kPGPPrefAllowKeyGen							=	8,
	kPGPPrefAllowRSAKeyGen						=	9,
	kPGPPrefAllowConventionalEncryption			=	10,
	kPGPPrefEnforceOutgoingADK					=	11,		/* 5.5.1 */
	kPGPPrefEnforceRemoteADKClass				=	12,		/* 5.5.1 */
	kPGPAdminPrefBoolLast
} PGPPrefsBool;

typedef enum _PGPAdminPrefsNumber
{
	kPGPAdminPrefNumberFirst					=	200,
	kPGPAdminPrefVersion = kPGPAdminPrefNumberFirst,
	kPGPPrefMinChars							=	201,
	kPGPPrefMinQuality							=	202,
	kPGPPrefCorpKeyPublicKeyAlgorithm			=	203,
	kPGPPrefOutADKPublicKeyAlgorithm			=	204,
	kPGPPrefMinimumKeySize						=	205,
	kPGPAdminPrefNumberLast
} PGPPrefsLong;

typedef enum _PGPAdminPrefsString
{
	kPGPAdminPrefStringFirst					=	400,
	kPGPPrefComments = kPGPAdminPrefStringFirst,
	kPGPPrefSiteLicenseNumber					=	401,	/* 5.5.1 */
	kPGPAdminPrefStringLast
} PGPPrefsString;

typedef enum _PGPAdminPrefsByte
{
	kPGPAdminPrefByteFirst						=	600,
	kPGPPrefOutgoingADKID = kPGPAdminPrefByteFirst,
	kPGPPrefDHADKID								=	601,
	kPGPPrefCorpKeyID							=	602,
	kPGPPrefDefaultKeys							=	603,
	kPGPAdminPrefByteLast
} PGPAdminPrefsByte;

#include "pgpBase.h"
#include "pgpPFLErrors.h"
#include "pflPrefs.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



PGPError PGPLoadAdminDefaults(	PGPPrefRef prefRef);


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpAdmin_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.

	$Id: pgpAdminPrefs.h,v 1.17.8.1 1998/11/12 03:11:03 heller Exp $
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
	kPGPPrefUpdateAllKeys						=	13,
	kPGPPrefUpdateTrustedIntroducers			=	14,
	kPGPPrefAutoAddRevoker						=	15,		/* 6.0 */
	kPGPPrefUsePGPdiskADK						= 	16,
	kPGPAdminPrefBoolLast
} PGPAdminPrefsBool;

typedef enum _PGPAdminPrefsNumber
{
	kPGPAdminPrefNumberFirst					=	200,
	kPGPAdminPrefVersion = kPGPAdminPrefNumberFirst,
	kPGPPrefMinChars							=	201,
	kPGPPrefMinQuality							=	202,
	kPGPPrefCorpKeyPublicKeyAlgorithm			=	203,
	kPGPPrefOutADKPublicKeyAlgorithm			=	204,
	kPGPPrefMinimumKeySize						=	205,
	kPGPPrefDaysUpdateAllKeys					=	206,
	kPGPPrefDaysUpdateTrustedIntroducers		=	207,
	kPGPPrefLastAllKeysUpdate					=	208,
	kPGPPrefLastTrustedIntroducersUpdate		=	209,
	kPGPPrefRevokerPublicKeyAlgorithm			=	210,	/* 6.0 */
	kPGPPrefPGPdiskADKPublicKeyAlgorithm		=	211,
	kPGPAdminPrefNumberLast
} PGPAdminPrefsNumber;

typedef enum _PGPAdminPrefsString
{
	kPGPAdminPrefStringFirst					=	400,
	kPGPPrefComments = kPGPAdminPrefStringFirst,
	kPGPPrefSiteLicenseNumber					=	401,	/* 5.5.1 */
	kPGPPrefAdminCompanyName					=	402,
	kPGPAdminPrefStringLast
} PGPAdminPrefsString;

typedef enum _PGPAdminPrefsByte
{
	kPGPAdminPrefByteFirst						=	600,
	kPGPPrefOutgoingADKID = kPGPAdminPrefByteFirst,
	kPGPPrefDHADKID								=	601,
	kPGPPrefCorpKeyID							=	602,
	kPGPPrefDefaultKeys							=	603,
	kPGPPrefRevokerKeyID						=	604,	/* 6.0 */
	kPGPPrefPGPdiskADKKeyID						=	605,
	kPGPAdminPrefByteLast
} PGPAdminPrefsByte;

#include "pgpBase.h"
#include "pgpPFLErrors.h"
#include "pflPrefs.h"
#include "pgpKeys.h"

static const PGPPrefDefault adminDefaults[] = 
{
{kPGPPrefUseOutgoingADK,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefUseDHADK,						kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefEnforceIncomingADK,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefEnforceMinChars,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefEnforceMinQuality,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefWarnNotCertByCorp,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefAutoSignTrustCorp,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefMetaIntroducerCorp,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefAllowKeyGen,					kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefAllowRSAKeyGen,				kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefAllowConventionalEncryption,	kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefEnforceOutgoingADK,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefEnforceRemoteADKClass,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefUpdateAllKeys,					kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefUpdateTrustedIntroducers,		kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefAutoAddRevoker,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefUsePGPdiskADK,					kPGPPrefType_Boolean, (void *) FALSE},

{kPGPPrefMinChars,						kPGPPrefType_Number, (void *) 5},
{kPGPPrefMinQuality,					kPGPPrefType_Number, (void *) 60},
{kPGPPrefCorpKeyPublicKeyAlgorithm,		kPGPPrefType_Number, 
										(void *) kPGPPublicKeyAlgorithm_DSA},
{kPGPPrefOutADKPublicKeyAlgorithm,		kPGPPrefType_Number, 
										(void *) kPGPPublicKeyAlgorithm_DSA},
{kPGPPrefMinimumKeySize,				kPGPPrefType_Number, (void *) 768},
{kPGPPrefDaysUpdateAllKeys,				kPGPPrefType_Number, (void *) 0},
{kPGPPrefDaysUpdateTrustedIntroducers,	kPGPPrefType_Number, (void *) 0},
{kPGPPrefLastAllKeysUpdate,				kPGPPrefType_Number, (void *) 0},
{kPGPPrefLastTrustedIntroducersUpdate,	kPGPPrefType_Number, (void *) 0},
{kPGPPrefRevokerPublicKeyAlgorithm,		kPGPPrefType_Number,
										(void *) kPGPPublicKeyAlgorithm_DSA},
{kPGPPrefPGPdiskADKPublicKeyAlgorithm,	kPGPPrefType_Number,
										(void *) kPGPPublicKeyAlgorithm_DSA},

{kPGPPrefComments,			kPGPPrefType_String, (void *) ""},
{kPGPPrefSiteLicenseNumber, kPGPPrefType_String, (void *) ""},
{kPGPPrefAdminCompanyName,	kPGPPrefType_String, (void *) ""}
};

static const PGPSize adminDefaultsSize = 
						sizeof(adminDefaults) / sizeof(PGPPrefDefault);


#endif /* ] Included_pgpAdminPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

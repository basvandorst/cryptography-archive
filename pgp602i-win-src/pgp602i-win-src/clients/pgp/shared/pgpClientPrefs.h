/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpClientPrefs.h,v 1.37.8.1 1998/11/12 03:11:05 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpClientPrefs_h	/* [ */
#define Included_pgpClientPrefs_h

enum
{
	kPGPPrefMacBinaryOff	= 1,
	kPGPPrefMacBinarySmart	= 2,
	kPGPPrefMacBinaryOn		= 3
};

typedef enum _PGPClientPrefsBool
{
	kPGPPrefBoolFirst							=	0,
	kPGPPrefEncryptToSelf = kPGPPrefBoolFirst,
	kPGPPrefWordWrapEnable						=	1,
	kPGPPrefDecryptCacheEnable					=	2,
	kPGPPrefSignCacheEnable						=	3,
	kPGPPrefMailEncryptPGPMIME					=	4,
	kPGPPrefMailSignPGPMIME						=	5,
	kPGPPrefFastKeyGen							=	6,
	kPGPPrefWarnOnMixRSAElGamal					=	7,
	kPGPPrefMailEncryptDefault					=	8,
	kPGPPrefMailSignDefault						=	9,
	kPGPPrefMarginalIsInvalid					=	10,
	kPGPPrefDisplayMarginalValidity				=	11,
	kPGPPrefFirstKeyGenerated					=	12,
	kPGPPrefWarnOnRSARecipAndNonRSASigner		=	13,
	kPGPPrefKeyServerSyncUnknownKeys			=	14,
	kPGPPrefKeyServerSyncOnAdd					=	15,
	kPGPPrefKeyServerSyncOnKeySign				=	16,
	kPGPPrefKeyServerSyncOnRevocation			=	17,
	kPGPPrefWarnOnReadOnlyKeyRings				=	18,
	kPGPPrefWarnOnWipe							=	19,
	kPGPPrefAutoDecrypt							=	20,
	kPGPPrefWarnOnADK							=	21,
	kPGPPrefKeyServerSyncOnVerify				=	22,		/* 6.0 */
	kPGPPrefPGPkeysMacShowToolbar				=	23,		/* 6.0 */
	kPGPPrefWarnOnMemLockFailure				=	24,		/* 6.0 */
	kPGPPrefExportKeysCompatible				=	25,		/* 6.0 */
	kPGPPrefBoolLast
} PGPClientPrefsBool;

typedef enum _PGPClientPrefsNumber
{
	kPGPPrefNumberFirst							=	200,
	kPGPPrefVersion = kPGPPrefNumberFirst,
	kPGPPrefWordWrapWidth						=	201,
	kPGPPrefDecryptCacheSeconds					=	202,
	kPGPPrefSignCacheSeconds					=	203,
	kPGPPrefDateOfLastSplashScreen				=	204,	/* PGPTime */
	kPGPPrefPreferredAlgorithm					=	205,
	kPGPPrefAllowedAlgorithms					=	206,	/* OBSOLETE */
	kPGPPrefMacBinaryDefault					=	207,
	kPGPPrefDateOfLastFreewareNotice			=	208,	/* OBSOLETE */
	kPGPPrefFileWipePasses						=	209,	/* 6.0 */
	kPGPPrefDiskWipePasses						=	210,	/* 6.0 */
	kPGPPrefLastSearchWindowSelection			=	211,	/* 6.0 */
	kPGPPrefNumberLast
} PGPClientPrefsNumber;

typedef enum _PGPClientPrefsString
{
	kPGPPrefStringFirst							=	400,
	kPGPPrefOwnerName = kPGPPrefStringFirst,
	kPGPPrefCompanyName							=	401,
	kPGPPrefLicenseNumber						=	402,
	kPGPPrefComment								=	403,
	kPGPPrefStringLast
} PGPClientPrefsString;

typedef enum _PGPClientPrefsByte
{
	kPGPPrefByteFirst							=	600,
	kPGPPrefPGPtoolsMacPrivateData = kPGPPrefByteFirst,
	kPGPPrefPGPmenuMacAppSignatures				=	601,
	kPGPPrefPGPkeysMacMainWinPos				=	602,
	kPGPPrefPGPkeysMacColumnData				=	603,
	kPGPPrefKeyServerList						=	604,
	kPGPPrefPGPkeysWinMainWinPos				=	605,
	kPGPPrefPGPkeysWinColumnData				=	606,
	kPGPPrefPGPkeysWinGroupsData				=	607,
	kPGPPrefAllowedAlgorithmsList				=	608,	/* 5.5.1 */
	kPGPPrefByteLast
} PGPClientPrefsByte;

#include "pgpBase.h"
#include "pgpPFLErrors.h"
#include "pflPrefs.h"
#include "pgpSymmetricCipher.h"
#include "pgpKeyServerPrefs.h"

static const PGPKeyServerEntry sDefaultKeyServer[] =
{
	{ sizeof(PGPKeyServerEntry), kPGPKeyServerType_LDAP, "", 
		"certserver.pgp.com", 0, "", kPGPPublicKeyAlgorithm_Invalid,
		(kKeyServerListed|kKeyServerIsRoot), "" },
	/*{ sizeof(PGPKeyServerEntry), kPGPKeyServerType_LDAPS, "pgp.com", 
		"internalkeys.pgp.com", 0, "", kPGPPublicKeyAlgorithm_Invalid,
		(kKeyServerListed), "" },*/
	{ sizeof(PGPKeyServerEntry), kPGPKeyServerType_HTTP, "", 
		"keys.pgp.com", 11371, "", kPGPPublicKeyAlgorithm_Invalid,
		(kKeyServerListed), "" },
	{ sizeof(PGPKeyServerEntry), kPGPKeyServerType_HTTP, "", 
		"wwwkeys.ch.pgp.net", 11371, "", kPGPPublicKeyAlgorithm_Invalid,
		(kKeyServerListed), "" },
	{ sizeof(PGPKeyServerEntry), kPGPKeyServerType_HTTP, "", 
		"wwwkeys.cz.pgp.net", 11371, "", kPGPPublicKeyAlgorithm_Invalid,
		(kKeyServerListed), "" },
	{ sizeof(PGPKeyServerEntry), kPGPKeyServerType_HTTP, "", 
		"wwwkeys.nl.pgp.net", 11371, "", kPGPPublicKeyAlgorithm_Invalid,
		(kKeyServerListed), "" },
	{ sizeof(PGPKeyServerEntry), kPGPKeyServerType_HTTP, "", 
		"wwwkeys.us.pgp.net", 11371, "", kPGPPublicKeyAlgorithm_Invalid,
		(kKeyServerListed), "" }
};

static const PGPCipherAlgorithm sAllowedAlgorithms[] =
{
	kPGPCipherAlgorithm_CAST5,
	kPGPCipherAlgorithm_3DES,
	kPGPCipherAlgorithm_IDEA
};

#if	PGP_MACINTOSH	/* [ */

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

typedef struct PGPmenuApps
{
	PGPBoolean	privateScrap;
	PGPBoolean	useOutputDialog;
	OSType		creator;
	Str31		appName;
} PGPmenuApps;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

static const PGPmenuApps	sInitPGPmenuApps[] =
{
	{FALSE,	FALSE,	'ttxt', "\pSimpleText"},
	{TRUE,	TRUE,	'MOSS', "\pNetscape Communicator"},	/* Verified -> 4.01 */
	{TRUE,	TRUE,	'AOp3', "\pAmerica Online"},		/* Verified -> 3.01 */
	{TRUE,	FALSE,	'R*ch', "\pBBEdit"},				/* Verified -> 4.5	*/
	{TRUE,	TRUE,	'MSNM', "\pOutlook Express"}
};	

#endif	/* ] */

static const PGPPrefDefault clientDefaults[] =
{
{kPGPPrefEncryptToSelf,					kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefWordWrapEnable,				kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefDecryptCacheEnable,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefSignCacheEnable,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefMailEncryptPGPMIME,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefMailSignPGPMIME,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefFastKeyGen,					kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefWarnOnMixRSAElGamal,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefMailEncryptDefault,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefMailSignDefault,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefMarginalIsInvalid,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefDisplayMarginalValidity,		kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefFirstKeyGenerated,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefWarnOnRSARecipAndNonRSASigner,	kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefKeyServerSyncUnknownKeys,		kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefKeyServerSyncOnAdd,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefKeyServerSyncOnKeySign,		kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefKeyServerSyncOnRevocation,		kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefKeyServerSyncOnVerify,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefWarnOnReadOnlyKeyRings,		kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefWarnOnWipe,					kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefAutoDecrypt,					kPGPPrefType_Boolean, (void *) FALSE},
{kPGPPrefWarnOnADK,						kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefPGPkeysMacShowToolbar,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefWarnOnMemLockFailure,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPPrefExportKeysCompatible,			kPGPPrefType_Boolean, (void *) TRUE},

{kPGPPrefWordWrapWidth,				kPGPPrefType_Number, (void *) 70},
{kPGPPrefDecryptCacheSeconds,		kPGPPrefType_Number, (void *) 120},
{kPGPPrefSignCacheSeconds,			kPGPPrefType_Number, (void *) 120},
{kPGPPrefDateOfLastSplashScreen,	kPGPPrefType_Number, (void *) 0},
{kPGPPrefPreferredAlgorithm,		kPGPPrefType_Number,
									(void *) kPGPCipherAlgorithm_CAST5},
{kPGPPrefAllowedAlgorithms,			kPGPPrefType_Number, (void *) 0xE0000000},
{kPGPPrefMacBinaryDefault,			kPGPPrefType_Number, 
									(void *) kPGPPrefMacBinaryOn},
{kPGPPrefDateOfLastFreewareNotice,	kPGPPrefType_Number, (void *) 0},
{kPGPPrefFileWipePasses,			kPGPPrefType_Number, (void *) 8},
{kPGPPrefDiskWipePasses,			kPGPPrefType_Number, (void *) 3},

#if	PGP_MACINTOSH	/* [ */
{kPGPPrefLastSearchWindowSelection,	kPGPPrefType_Number, (void *) 3},
#elif PGP_WIN32
{kPGPPrefLastSearchWindowSelection,	kPGPPrefType_Number, (void *) 2},
#endif

{kPGPPrefOwnerName,		kPGPPrefType_String, (void *) ""},
{kPGPPrefCompanyName,	kPGPPrefType_String, (void *) ""},
{kPGPPrefLicenseNumber, kPGPPrefType_String, (void *) ""},
{kPGPPrefComment,		kPGPPrefType_String, (void *) ""},

#if	PGP_MACINTOSH	/* [ */

{kPGPPrefPGPmenuMacAppSignatures, kPGPPrefType_Byte, 
	(void *) sInitPGPmenuApps, sizeof(sInitPGPmenuApps)},

#endif	/* ] */

{kPGPPrefKeyServerList, kPGPPrefType_Byte, 
	(void *) sDefaultKeyServer, sizeof(sDefaultKeyServer)},
{kPGPPrefAllowedAlgorithmsList, kPGPPrefType_Byte, 
	(void *) sAllowedAlgorithms, sizeof(sAllowedAlgorithms)},
};

static const PGPSize clientDefaultsSize = 
						sizeof(clientDefaults) / sizeof(PGPPrefDefault);


#endif /* ] Included_pgpClientPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

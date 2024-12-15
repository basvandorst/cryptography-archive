/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpClientPrefs.h,v 1.10.2.1 1997/11/11 00:27:16 wprice Exp $
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
	kPGPPrefKeyServerSyncOnAddName				=	15,
	kPGPPrefKeyServerSyncOnKeySign				=	16,
	kPGPPrefKeyServerSyncOnRevocation			=	17,
	kPGPPrefWarnOnReadOnlyKeyRings				=	18,
	kPGPPrefWarnOnWipe							=	19,
	kPGPPrefAutoDecrypt							=	20,
	kPGPPrefWarnOnADK							=	21,
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
	kPGPPrefDateOfLastFreewareNotice			=	208,
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


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


PGPError PGPLoadClientDefaults(	PGPPrefRef prefRef);


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpClientPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

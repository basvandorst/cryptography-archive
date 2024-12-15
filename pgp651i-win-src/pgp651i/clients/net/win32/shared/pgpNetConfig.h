/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPnetConfig.h - PGPnet configuration utilities header
	

	$Id: pgpNetConfig.h,v 1.24 1999/04/23 13:59:09 pbj Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetConfig_h
#define _Included_pgpNetConfig_h

// includes
#include "pgpPubTypes.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpIke.h"

#include "pgpNetPrefs.h"

PGP_BEGIN_C_DECLARATIONS

// typedefs
typedef struct _PGPNetHostEntry
{
	PGPUInt32				hostType;
	PGPUInt32				ipAddress;
	PGPUInt32				ipMask;
	PGPInt32				childOf;
	char					hostName[kMaxNetHostNameLength + 1];
	PGPUInt32				identityType;
	PGPUInt32				identityIPAddress;
	char					identity[kMaxNetHostIdentityLength + 1];
	char					sharedSecret[kMaxNetHostSharedSecretLength + 1];
	
	PGPPublicKeyAlgorithm	authKeyAlg;
	PGPByte					authKeyExpKeyID[kMaxNetHostKeyIDSize];
	PGPByte					authCertIASN[kMaxNetHostIASNLength];
	PGPUInt32				authCertIASNLength;
} PGPNetHostEntry;

typedef struct {
	PGPBoolean				bPGPnetEnabled;
	PGPBoolean				bAllowUnconfigHost;
	PGPBoolean				bRequireSecure;
	PGPBoolean				bRequireValidKey;
	PGPBoolean				bCachePassphrases;
	PGPBoolean				bExpertMode;
	PGPBoolean				bFirstExecution;
	PGPBoolean				bWarnReSharedSecret;

	PGPBoolean				bIkeKByteExpiration;
	PGPBoolean				bIkeTimeExpiration;
	PGPBoolean				bIpsecKByteExpiration;
	PGPBoolean				bIpsecTimeExpiration;

	PGPUInt32				uIkeKByteExpiration;
	PGPUInt32				uIkeTimeExpiration;
	PGPUInt32				uIpsecKByteExpiration;
	PGPUInt32				uIpsecTimeExpiration;

	PGPUInt32				uPGPAuthKeyAlg;
	PGPByte					expkeyidPGPAuthKey[kPGPMaxExportedKeyIDSize];
	PGPUInt32				uX509AuthKeyAlg;
	PGPByte					expkeyidX509AuthKey[kPGPMaxExportedKeyIDSize];
	PGPByte*				pX509AuthCertIASN;
	PGPUInt32				uX509AuthCertIASNLength;

	PGPUInt32				uHostCount;
	PGPNetHostEntry*		pHostList;

	PGPikeMTPref			IkeExpirationPrefs;
	PGPikeMTPref			IkeIkeProposalPrefs;
	PGPikeMTPref			IkeIpsecProposalPrefs;
	PGPikeMTPref			IkeAlgorithmPrefs;
	PGPikeGroupID			IkeIpsecGroupID;
} PNCONFIG, *PPNCONFIG;

// prototypes
PGPError
PGPnetOpenPrefs (
		PGPMemoryMgrRef	memmgr, 
		PGPPrefRef*		pprefref);

PGPError
PGPnetClosePrefs (
		PGPPrefRef		prefref, 
		PGPBoolean		bSave);

PGPError
PGPnetLoadConfiguration (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig);

PGPError
PGPnetFreeConfiguration (
		PPNCONFIG		pPNConfig);

PGPError
PGPnetReloadConfiguration (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig);

PGPError
PGPnetSaveConfiguration (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig);

PGPError
PGPnetAddPromiscuousHostToConfiguration (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig,
		PGPUInt32		uIPAddressHost,
		PGPUInt32		uIPAddressLocal,
		PGPKeyRef		keyAuth,
		PGPSigRef		sigAuth);

PGPError
PGPnetGetConfiguredAuthKeys (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig,
		PGPKeySetRef	keyset,
		PGPKeyRef*		pkeyPGP,
		PGPKeyRef*		pkeyX509,
		PGPSigRef*		psigX509);

PGPError
PGPnetGetConfiguredHostAuthKeys (
		PGPContextRef		context,
		PGPNetHostEntry*	pHost,
		PGPKeySetRef		keyset,
		PGPKeyRef*			pkey,
		PGPSigRef*			psig);

PGPError
PGPnetGetIkeAlgorithmPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig);

PGPError
PGPnetGetIkeProposalPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig);

PGPError
PGPnetGetIpsecProposalPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig);

PGP_END_C_DECLARATIONS

#endif //_Included_pgpNetConfig_h
/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPconfig.c - PGPnet read/write configuration info
	

	$Id: pgpNetConfig.c,v 1.38 1999/04/23 13:59:09 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>

#include "pgpNetConfig.h"
#include "pgpNetPaths.h"

#include "pflPrefTypes.h"
#include "pgpMemoryMgr.h"
#include "pgpFileSpec.h"
#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpEndianConversion.h"

// macro definitions
#define CKERR		if (IsPGPError (err)) goto done


//	______________________________________________
//
//  convert Host list from prefs (storage) format to
//	local format

static PGPError
sConvertHostListFromStorage (
		PGPContextRef			context,
		PPNCONFIG				pPNConfig, 
		PGPNetPrefHostEntry*	pStorageList)
{
	PGPSize		sizeList;
	PGPUInt32	u;

	if (pPNConfig->uHostCount == 0)
		pPNConfig->pHostList = NULL;
	else
	{
		sizeList = 
			pPNConfig->uHostCount * 
			sizeof (PGPNetHostEntry);

		pPNConfig->pHostList = 
				PGPNewData (PGPGetContextMemoryMgr (context),
							sizeList,
							kPGPMemoryMgrFlags_Clear);

		if (pPNConfig->pHostList)
		{
			for (u=0; u<pPNConfig->uHostCount; u++)
			{
				pPNConfig->pHostList[u].hostType = 
								pStorageList[u].hostType;
				pPNConfig->pHostList[u].ipAddress =
								pStorageList[u].ipAddress;
				pPNConfig->pHostList[u].ipMask =
								pStorageList[u].ipMask;
				pPNConfig->pHostList[u].childOf =
								pStorageList[u].childOf;
				pgpCopyMemory (pStorageList[u].hostName, 
								pPNConfig->pHostList[u].hostName, 
								kMaxNetHostNameLength + 1);
				pPNConfig->pHostList[u].identityType =
								pStorageList[u].identityType;
				pgpCopyMemory (pStorageList[u].identity, 
								&pPNConfig->pHostList[u].identityIPAddress, 
								sizeof(PGPUInt32));
				pgpCopyMemory (pStorageList[u].identity, 
								pPNConfig->pHostList[u].identity, 
								kMaxNetHostIdentityLength + 1);
				pgpCopyMemory (pStorageList[u].sharedSecret, 
								pPNConfig->pHostList[u].sharedSecret, 
								kMaxNetHostSharedSecretLength + 1);

				pPNConfig->pHostList[u].authKeyAlg =
								pStorageList[u].authKeyAlg;
				pgpCopyMemory (pStorageList[u].authKeyExpKeyID, 
								pPNConfig->pHostList[u].authKeyExpKeyID, 
								kMaxNetHostKeyIDSize);
				pgpCopyMemory (pStorageList[u].authCertIASN, 
								pPNConfig->pHostList[u].authCertIASN, 
								pStorageList[u].authCertIASNLength);
				pPNConfig->pHostList[u].authCertIASNLength =
								pStorageList[u].authCertIASNLength;
			}
		}
		else
			return kPGPError_OutOfMemory;
	}

	return kPGPError_NoErr;
}


//	______________________________________________
//
//  convert Host list from local format to
//	prefs (storage) format

static PGPError
sConvertHostListToStorage (
		PGPContextRef			context,
		PPNCONFIG				pPNConfig, 
		PGPNetPrefHostEntry**	ppStorageList)
{
	PGPSize		sizeList;
	PGPUInt32	u;

	if (pPNConfig->uHostCount == 0)
		*ppStorageList = NULL;
	else
	{
		sizeList = 
			pPNConfig->uHostCount * 
			sizeof (PGPNetPrefHostEntry);

		*ppStorageList = 
				PGPNewData (PGPGetContextMemoryMgr (context),
							sizeList,
							kPGPMemoryMgrFlags_Clear);

		if (*ppStorageList)
		{
			for (u=0; u<pPNConfig->uHostCount; u++)
			{
				(*ppStorageList)[u].hostType = 
									pPNConfig->pHostList[u].hostType;
				(*ppStorageList)[u].ipAddress =
									pPNConfig->pHostList[u].ipAddress;
				(*ppStorageList)[u].ipMask =
									pPNConfig->pHostList[u].ipMask;
				(*ppStorageList)[u].childOf =
									pPNConfig->pHostList[u].childOf;
				pgpCopyMemory (	pPNConfig->pHostList[u].hostName, 
									(*ppStorageList)[u].hostName, 
									kMaxNetHostNameLength + 1);
				(*ppStorageList)[u].identityType =
									pPNConfig->pHostList[u].identityType;
				if (pPNConfig->pHostList[u].identityType == 
													kPGPike_ID_IPV4_Addr)
				{
					(*ppStorageList)[u].identityLen = 4;
					pgpCopyMemory (&pPNConfig->pHostList[u].identityIPAddress, 
									(*ppStorageList)[u].identity, 
									(*ppStorageList)[u].identityLen);
				}
				else
				{
					(*ppStorageList)[u].identityLen = 
									strlen (pPNConfig->pHostList[u].identity);
					pgpCopyMemory (	pPNConfig->pHostList[u].identity, 
									(*ppStorageList)[u].identity, 
									kMaxNetHostIdentityLength + 1);
				}
				pgpCopyMemory (	pPNConfig->pHostList[u].sharedSecret, 
								(*ppStorageList)[u].sharedSecret,
								kMaxNetHostSharedSecretLength + 1);

				(*ppStorageList)[u].authKeyAlg =
									pPNConfig->pHostList[u].authKeyAlg;
				pgpCopyMemory (	pPNConfig->pHostList[u].authKeyExpKeyID, 
								(*ppStorageList)[u].authKeyExpKeyID,
								kMaxNetHostKeyIDSize);
				pgpCopyMemory (	pPNConfig->pHostList[u].authCertIASN, 
								(*ppStorageList)[u].authCertIASN,
								pPNConfig->pHostList[u].authCertIASNLength);
				(*ppStorageList)[u].authCertIASNLength =
								pPNConfig->pHostList[u].authCertIASNLength;
			}
		}
		else
			return kPGPError_OutOfMemory;
	}

	return kPGPError_NoErr;
}


//	______________________________________________
//
//  convert expiration prefs to form used by IKE

static PGPError
sComputeIkeExpirationPrefs (
		PGPContextRef		context,
		PPNCONFIG			pPNConfig)
{
	pPNConfig->IkeExpirationPrefs.pref = kPGPike_PF_Expiration;

	if (pPNConfig->bIkeKByteExpiration)
		pPNConfig->IkeExpirationPrefs.u.expiration.kbLifeTimeIKE =
			pPNConfig->uIkeKByteExpiration;
	else
		pPNConfig->IkeExpirationPrefs.u.expiration.kbLifeTimeIKE = 0;

	if (pPNConfig->bIkeTimeExpiration)
		pPNConfig->IkeExpirationPrefs.u.expiration.secLifeTimeIKE =
			pPNConfig->uIkeTimeExpiration;
	else
		pPNConfig->IkeExpirationPrefs.u.expiration.secLifeTimeIKE = 0;

	if (pPNConfig->bIpsecKByteExpiration)
		pPNConfig->IkeExpirationPrefs.u.expiration.kbLifeTimeIPSEC =
			pPNConfig->uIpsecKByteExpiration;
	else
		pPNConfig->IkeExpirationPrefs.u.expiration.kbLifeTimeIPSEC = 0;

	if (pPNConfig->bIpsecTimeExpiration)
		pPNConfig->IkeExpirationPrefs.u.expiration.secLifeTimeIPSEC =
			pPNConfig->uIpsecTimeExpiration;
	else
		pPNConfig->IkeExpirationPrefs.u.expiration.secLifeTimeIPSEC = 0;

	return kPGPError_NoErr;
}

//	______________________________________________
//
//  load algorithm prefs into structure used by IKE

PGPError
PGPnetGetIkeAlgorithmPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig)
{
	PGPError	err;

	pPNConfig->IkeAlgorithmPrefs.pref = kPGPike_PF_AllowedAlgorithms;

	// get cipher algorithm booleans
	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowCipherCAST, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.cast5); 
	CKERR;

	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowCipher3DES, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.tripleDES); 
	CKERR;

	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowCipherDES56, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.singleDES); 
	CKERR;

	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowCipherNone, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.espNULL); 
	CKERR;

	// get hash algorithm booleans
	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowHashSHA1, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.sha1); 
	CKERR;

	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowHashMD5, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.md5); 
	CKERR;

	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowHashNone, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.noAuth); 
	CKERR;

	// get compression algorithm booleans
	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowCompLZS, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.lzs); 
	CKERR;

	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowCompDeflate, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.deflate); 
	CKERR;

	// get Diffie-Hellman algorithm booleans
	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowDH768, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.modpOne768); 
	CKERR;

	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowDH1024, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.modpTwo1024); 
	CKERR;

	err = PGPGetPrefBoolean (prefref, 
		kPGPNetPrefAllowDH1536, 
		&pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.modpFive1536); 
	CKERR;

	err = kPGPError_NoErr;

done :
	return err;
}


//	______________________________________________
//
//  save algorithm prefs from structure used by IKE

static PGPError
sSetIkeAlgorithmPrefs (
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig)
{
	PGPError	err		= kPGPError_NoErr;

	// set cipher algorithm booleans
	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowCipherCAST, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.cast5); 
	CKERR;

	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowCipher3DES, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.tripleDES); 
	CKERR;

	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowCipherDES56, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.singleDES); 
	CKERR;

	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowCipherNone, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.espNULL); 
	CKERR;

	// set hash algorithm booleans
	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowHashSHA1, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.sha1); 
	CKERR;

	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowHashMD5, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.md5); 
	CKERR;

	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowHashNone, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.noAuth); 
	CKERR;

	// set compression algorithm booleans
	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowCompLZS, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.lzs); 
	CKERR;

	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowCompDeflate, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.deflate); 
	CKERR;

	// set Diffie-Hellman algorithm booleans
	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowDH768, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.modpOne768); 
	CKERR;

	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowDH1024, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.modpTwo1024); 
	CKERR;

	err = PGPSetPrefBoolean (prefref, 
		kPGPNetPrefAllowDH1536, 
		pPNConfig->IkeAlgorithmPrefs.u.allowedAlgorithms.modpFive1536); 
	CKERR;

done :
	return err;
}


//	______________________________________________
//
//  load IKE proposal prefs into structure used by IKE

PGPError
PGPnetGetIkeProposalPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig)
{
	PGPError			err		= kPGPError_NoErr;

	PGPSize				size;
	PGPUInt32			u, uNumProposals;
	PGPikeTransform*	pIke;

	PGPNetPrefIKEProposalEntry*		prefEntry;

	err = PGPGetNetIKEProposalPrefs (
				prefref, &prefEntry, &uNumProposals); CKERR;

	pPNConfig->IkeIkeProposalPrefs.pref = kPGPike_PF_IKEProposals;
	pPNConfig->
		IkeIkeProposalPrefs.u.ikeProposals.numTransforms = uNumProposals;

	if (uNumProposals > 0)
	{
		size = uNumProposals * sizeof(PGPikeTransform);
		pPNConfig->IkeIkeProposalPrefs.u.ikeProposals.t =
				PGPNewData (PGPGetContextMemoryMgr (context),
							size, kPGPMemoryMgrFlags_Clear);

		if (pPNConfig->IkeIkeProposalPrefs.u.ikeProposals.t)
		{
			pIke = pPNConfig->IkeIkeProposalPrefs.u.ikeProposals.t;

			for (u=0; u<uNumProposals; u++)
			{
				pIke[u].authMethod	= prefEntry[u].authMethod;
				pIke[u].hash		= prefEntry[u].hash;
				pIke[u].cipher		= prefEntry[u].cipher;
				pIke[u].modpGroup	= TRUE;
				pIke[u].groupID		= prefEntry[u].groupID;
			}
		}
		else
			err = kPGPError_OutOfMemory;

		PGPDisposePrefData (prefref, prefEntry);
	}

done :
	return err;
}


//	______________________________________________
//
//  save IKE proposal prefs from structure used by IKE

static PGPError
sSetIkeProposalPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig)
{
	PGPError			err		= kPGPError_NoErr;


	PGPSize				size;
	PGPUInt32			u, uNumProposals;
	PGPikeTransform*	pIke;

	PGPNetPrefIKEProposalEntry*		prefEntry;

	uNumProposals = 
		pPNConfig->IkeIkeProposalPrefs.u.ikeProposals.numTransforms;

	if (uNumProposals > 0)
	{
		size = uNumProposals * sizeof(PGPNetPrefIKEProposalEntry);

		prefEntry = PGPNewData (PGPGetContextMemoryMgr (context),
							size, kPGPMemoryMgrFlags_Clear);

		if (prefEntry)
		{
			pIke = pPNConfig->IkeIkeProposalPrefs.u.ikeProposals.t;

			for (u=0; u<uNumProposals; u++)
			{
				prefEntry[u].authMethod		= pIke[u].authMethod;
				prefEntry[u].hash			= pIke[u].hash;
				prefEntry[u].cipher			= pIke[u].cipher;
				prefEntry[u].groupID		= pIke[u].groupID;
			}

			err = PGPSetNetIKEProposalPrefs (
						prefref, prefEntry, uNumProposals);
			PGPFreeData (prefEntry);
		}
		else
			err = kPGPError_OutOfMemory;
	}

	return err;
}


//	______________________________________________
//
//  load IPSEC proposal prefs into structure used by IKE

PGPError
PGPnetGetIpsecProposalPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig)
{
	PGPError			err		= kPGPError_NoErr;

	PGPSize				size;
	PGPUInt32			u, uNumProposals;
	PGPipsecTransform*	pIpsec;

	PGPNetPrefIPSECProposalEntry*		prefEntry;

	err = PGPGetPrefNumber (
				prefref, kPGPnetPrefIPSECGroupID, 
				&pPNConfig->IkeIpsecGroupID); CKERR;

	err = PGPGetNetIPSECProposalPrefs (
				prefref, &prefEntry, &uNumProposals); CKERR;

	pPNConfig->IkeIpsecProposalPrefs.pref = kPGPike_PF_IPSECProposals;
	pPNConfig->
		IkeIpsecProposalPrefs.u.ipsecProposals.numTransforms = uNumProposals;

	if (uNumProposals > 0)
	{
		size = uNumProposals * sizeof(PGPipsecTransform);
		pPNConfig->IkeIpsecProposalPrefs.u.ipsecProposals.t =
				PGPNewData (PGPGetContextMemoryMgr (context),
							size, kPGPMemoryMgrFlags_Clear);

		if (pPNConfig->IkeIpsecProposalPrefs.u.ipsecProposals.t)
		{
			pIpsec = pPNConfig->IkeIpsecProposalPrefs.u.ipsecProposals.t;

			for (u=0; u<uNumProposals; u++)
			{
				pIpsec[u].useESP			= prefEntry[u].useESP;
				pIpsec[u].esp.cipher		= prefEntry[u].espCipher;
				pIpsec[u].esp.authAttr		= prefEntry[u].espHash;

				pIpsec[u].useAH				= prefEntry[u].useAH;
				pIpsec[u].ah.authAlg		= prefEntry[u].ahAuthAlg;
				pIpsec[u].ah.authAttr		= prefEntry[u].ahHash;

				pIpsec[u].useIPCOMP			= prefEntry[u].useIPCOMP;
				pIpsec[u].ipcomp.compAlg	= prefEntry[u].ipcomp;

				pIpsec[u].groupID			= pPNConfig->IkeIpsecGroupID;
			}
		}
		else
			err = kPGPError_OutOfMemory;

		PGPDisposePrefData (prefref, prefEntry);
	}

done :
	return err;
}


//	______________________________________________
//
//  save IPSEC proposal prefs from structure used by IKE

static PGPError
sSetIpsecProposalPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PPNCONFIG			pPNConfig)
{
	PGPError			err		= kPGPError_NoErr;

	PGPSize				size;
	PGPUInt32			u, uNumProposals;
	PGPipsecTransform*	pIpsec;

	PGPNetPrefIPSECProposalEntry*		prefEntry;

	uNumProposals = 
		pPNConfig->IkeIpsecProposalPrefs.u.ipsecProposals.numTransforms;

	if (uNumProposals > 0)
	{
		size = uNumProposals * sizeof(PGPNetPrefIPSECProposalEntry);

		prefEntry = PGPNewData (PGPGetContextMemoryMgr (context),
							size, kPGPMemoryMgrFlags_Clear);

		if (prefEntry)
		{
			pIpsec = pPNConfig->IkeIpsecProposalPrefs.u.ipsecProposals.t;

			for (u=0; u<uNumProposals; u++)
			{
				prefEntry[u].useESP			= pIpsec[u].useESP;
				prefEntry[u].espCipher		= pIpsec[u].esp.cipher;
				prefEntry[u].espHash		= pIpsec[u].esp.authAttr;
				prefEntry[u].useAH			= pIpsec[u].useAH;
				prefEntry[u].ahAuthAlg		= pIpsec[u].ah.authAlg;
				prefEntry[u].ahHash			= pIpsec[u].ah.authAttr;
				prefEntry[u].useIPCOMP		= pIpsec[u].useIPCOMP;
				prefEntry[u].ipcomp			= pIpsec[u].ipcomp.compAlg;
			}

			err = PGPSetNetIPSECProposalPrefs (
						prefref, prefEntry, uNumProposals);
			PGPFreeData (prefEntry);
		}
		else
			err = kPGPError_OutOfMemory;
	}

	return err;
}


//	______________________________________________
//
//  get preferences file reference

PGPError
PGPnetOpenPrefs (
		PGPMemoryMgrRef	memmgr, 
		PGPPrefRef*		pprefref) 
{
	CHAR			szPath[MAX_PATH];
	PGPError		err;
	PFLFileSpecRef	fileref;
	PGPPrefRef		prefref;
	
	err = PGPnetGetPrefsFullPath (szPath, sizeof(szPath)); CKERR;

	err = PFLNewFileSpecFromFullPath (memmgr, szPath, &fileref); CKERR;

	err = PGPOpenPrefFile (fileref, netDefaults, 
								netDefaultsSize, &prefref);

	if ((err == kPGPError_FileOpFailed) ||
		(err == kPGPError_FileNotFound)) 
	{
		err = PFLFileSpecCreate (fileref);
		pgpAssert (IsntPGPError (err));
		err = PGPOpenPrefFile (fileref, netDefaults, 
								netDefaultsSize, &prefref);
		pgpAssert (IsntPGPError (err));
	}

	if (fileref)
		PFLFreeFileSpec (fileref);

	if (IsPGPError (err)) 
	{
		if (PGPPrefRefIsValid (prefref)) 
			PGPClosePrefFile (prefref);
	}
	else 	
		*pprefref = prefref;

done:
	return err;
}

//	_______________________________________
//
//  Save and close down client preferences file

PGPError
PGPnetClosePrefs (
		PGPPrefRef	prefref, 
		PGPBoolean	bSave) 
{
	PGPError		err;

	if (bSave) 
	{
		err = PGPSavePrefFile (prefref);
		pgpAssert (IsntPGPError(err));
	}

	err = PGPClosePrefFile (prefref);
	pgpAssert (IsntPGPError(err));

	return err;
}

//	____________________________________
//
//	load configuration information from prefs file

PGPError
PGPnetLoadConfiguration (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig)
{
	PGPPrefRef					prefref			= kInvalidPGPPrefRef;
	PGPNetPrefHostEntry*		pHostList		= NULL;

	PGPError			err;

	pgpFillMemory(pPNConfig, sizeof(PNCONFIG), '\0');

	err = PGPnetOpenPrefs (PGPGetContextMemoryMgr (context), &prefref); 
	CKERR;

	// get booleans
	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefEnablePGPnet, 
					&pPNConfig->bPGPnetEnabled); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefAllowUnconfiguredHosts, 
					&pPNConfig->bAllowUnconfigHost); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefRequireSecure, 
					&pPNConfig->bRequireSecure); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefRequireValidKey, 
					&pPNConfig->bRequireValidKey); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefEnablePassphraseCache, 
					&pPNConfig->bCachePassphrases); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefExpertMode, 
					&pPNConfig->bExpertMode); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefFirstExecution, 
					&pPNConfig->bFirstExecution); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefWarnReSharedSecret, 
					&pPNConfig->bWarnReSharedSecret); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefEnableIKEKByteExpiration, 
					&pPNConfig->bIkeKByteExpiration); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefEnableIKETimeExpiration, 
					&pPNConfig->bIkeTimeExpiration); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefEnableIPSECKByteExpiration, 
					&pPNConfig->bIpsecKByteExpiration); CKERR;

	err = PGPGetPrefBoolean (prefref, 
					kPGPNetPrefEnableIPSECTimeExpiration, 
					&pPNConfig->bIpsecTimeExpiration); CKERR;

	// get numbers
	err = PGPGetPrefNumber (prefref, 
					kPGPNetPrefIKEKByteExpiration, 
					&pPNConfig->uIkeKByteExpiration); CKERR;

	err = PGPGetPrefNumber (prefref, 
					kPGPNetPrefIKETimeExpiration, 
					&pPNConfig->uIkeTimeExpiration); CKERR;

	err = PGPGetPrefNumber (prefref, 
					kPGPNetPrefIPSECKByteExpiration, 
					&pPNConfig->uIpsecKByteExpiration); CKERR;

	err = PGPGetPrefNumber (prefref, 
					kPGPNetPrefIPSECTimeExpiration, 
					&pPNConfig->uIpsecTimeExpiration); CKERR;

	// get PGP Authentication keyid
	err = PGPGetPrefNumber (prefref, 
					kPGPNetPrefPGPAuthKeyAlgorithm, 
					&pPNConfig->uPGPAuthKeyAlg); CKERR;

	if (pPNConfig->uPGPAuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		err = PGPGetPrefStringBuffer (prefref, 
					kPGPNetPrefPGPAuthKeyID, 
					kPGPMaxExportedKeyIDSize, 
					pPNConfig->expkeyidPGPAuthKey); CKERR;
	}

	// get X509 Authentication keyid
	err = PGPGetPrefNumber (prefref, 
					kPGPNetPrefX509AuthKeyAlgorithm, 
					&pPNConfig->uX509AuthKeyAlg); CKERR;

	if (pPNConfig->uX509AuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		PGPByte*	pbyte;
		PGPSize		size;

		err = PGPGetPrefData (prefref, 
					kPGPNetPrefX509AuthKeyID, 
					&size, 
					&pbyte); CKERR;
		if (IsntPGPError (err) && pbyte)
		{
			pgpCopyMemory (pbyte, pPNConfig->expkeyidX509AuthKey, size);
			PGPDisposePrefData (prefref, pbyte);
		}

		err = PGPGetPrefData (prefref, 
					kPGPNetPrefX509AuthCertIASN, 
					&pPNConfig->uX509AuthCertIASNLength,
					&pPNConfig->pX509AuthCertIASN); CKERR;
	}

	// fill host entry structure
	err = PGPGetNetHostPrefs (prefref,
					&pHostList, 
					&pPNConfig->uHostCount); CKERR;

	err = sConvertHostListFromStorage (context,
					pPNConfig, pHostList); CKERR;

	// fill in IKE prefs structures
	err = PGPnetGetIkeAlgorithmPrefs (context, prefref, pPNConfig); CKERR;
	err = PGPnetGetIkeProposalPrefs (context, prefref, pPNConfig); CKERR;
	err = PGPnetGetIpsecProposalPrefs (context, prefref, pPNConfig); CKERR;
	err = sComputeIkeExpirationPrefs (context, pPNConfig); CKERR;

done:
	if (pHostList)
		PGPDisposePrefData (prefref, pHostList);

	if (PGPPrefRefIsValid (prefref))
		PGPnetClosePrefs (prefref, FALSE);

	return err;
}


//	____________________________________
//
//	free all the points in the config structure

PGPError
PGPnetFreeConfiguration (
		PPNCONFIG		pPNConfig)
{
	// if host list exists, dispose of it
	if (pPNConfig->pHostList)
	{
		PGPFreeData (pPNConfig->pHostList);
		pPNConfig->pHostList = NULL;
	}

	// if X509 AuthCert IASN exists, dispose of it
	if (pPNConfig->pX509AuthCertIASN)
	{
		PGPFreeData (pPNConfig->pX509AuthCertIASN);
		pPNConfig->pX509AuthCertIASN = NULL;
	}

	// dispose of IKE prefs proposal arrays
	if (pPNConfig->IkeIkeProposalPrefs.u.ikeProposals.t)
	{
		PGPFreeData (pPNConfig->IkeIkeProposalPrefs.u.ikeProposals.t);
		pPNConfig->IkeIkeProposalPrefs.u.ikeProposals.t = NULL;
	}

	if (pPNConfig->IkeIpsecProposalPrefs.u.ipsecProposals.t)
	{
		PGPFreeData (pPNConfig->IkeIpsecProposalPrefs.u.ipsecProposals.t);
		pPNConfig->IkeIpsecProposalPrefs.u.ipsecProposals.t = NULL;
	}

	return kPGPError_NoErr;
}


//	____________________________________
//
//	reload configuration information from prefs file

PGPError
PGPnetReloadConfiguration (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig)
{
	PGPError	err;

	err = PGPnetFreeConfiguration (pPNConfig);
	if (IsntPGPError (err))
		err = PGPnetLoadConfiguration (context, pPNConfig);

	return err;
}


//	____________________________________
//
//	save configuration information to prefs file

PGPError
PGPnetSaveConfiguration (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig)
{
	PGPNetPrefHostEntry*		pHostList		= NULL;

	PGPPrefRef					prefref;
	PGPError					err;

	err = PGPnetOpenPrefs (PGPGetContextMemoryMgr (context), &prefref); 
	CKERR;

	// save booleans
	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefEnablePGPnet, 
					pPNConfig->bPGPnetEnabled); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefAllowUnconfiguredHosts, 
					pPNConfig->bAllowUnconfigHost); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefRequireSecure, 
					pPNConfig->bRequireSecure); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefRequireValidKey, 
					pPNConfig->bRequireValidKey); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefEnablePassphraseCache, 
					pPNConfig->bCachePassphrases); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefExpertMode, 
					pPNConfig->bExpertMode); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefFirstExecution, 
					pPNConfig->bFirstExecution); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefWarnReSharedSecret, 
					pPNConfig->bWarnReSharedSecret); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefEnableIKEKByteExpiration, 
					pPNConfig->bIkeKByteExpiration); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefEnableIKETimeExpiration, 
					pPNConfig->bIkeTimeExpiration); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefEnableIPSECKByteExpiration, 
					pPNConfig->bIpsecKByteExpiration); CKERR;

	err = PGPSetPrefBoolean (prefref, 
					kPGPNetPrefEnableIPSECTimeExpiration, 
					pPNConfig->bIpsecTimeExpiration); CKERR;

	// save numbers
	err = PGPSetPrefNumber (prefref, 
					kPGPNetPrefPGPAuthKeyAlgorithm, 
					pPNConfig->uPGPAuthKeyAlg); CKERR;

	err = PGPSetPrefNumber (prefref, 
					kPGPNetPrefX509AuthKeyAlgorithm, 
					pPNConfig->uX509AuthKeyAlg); CKERR;

	err = PGPSetPrefNumber (prefref, 
					kPGPNetPrefIKEKByteExpiration, 
					pPNConfig->uIkeKByteExpiration); CKERR;

	err = PGPSetPrefNumber (prefref, 
					kPGPNetPrefIKETimeExpiration, 
					pPNConfig->uIkeTimeExpiration); CKERR;

	err = PGPSetPrefNumber (prefref, 
					kPGPNetPrefIPSECKByteExpiration, 
					pPNConfig->uIpsecKByteExpiration); CKERR;

	err = PGPSetPrefNumber (prefref, 
					kPGPNetPrefIPSECTimeExpiration, 
					pPNConfig->uIpsecTimeExpiration); CKERR;

	err = PGPSetPrefNumber (prefref,
					kPGPnetPrefIPSECGroupID,
					pPNConfig->IkeIpsecGroupID); CKERR;

	// save PGP authentication key info
	err = PGPSetPrefData (prefref, 
					kPGPNetPrefPGPAuthKeyID, 
					kPGPMaxExportedKeyIDSize, 
					pPNConfig->expkeyidPGPAuthKey);

	// save X509 authentication key info
	if (pPNConfig->uX509AuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		err = PGPSetPrefData (prefref, 
						kPGPNetPrefX509AuthKeyID, 
						kPGPMaxExportedKeyIDSize, 
						pPNConfig->expkeyidX509AuthKey);

		if (pPNConfig->uX509AuthCertIASNLength > 0)
		{
			err = PGPSetPrefData (prefref, 
							kPGPNetPrefX509AuthCertIASN, 
							pPNConfig->uX509AuthCertIASNLength, 
							pPNConfig->pX509AuthCertIASN);
		}
	}

	// save host list
	sConvertHostListToStorage (context, pPNConfig, &pHostList);
	err = PGPSetNetHostPrefs (prefref,
					pHostList,
					pPNConfig->uHostCount); CKERR;

	// save IKE prefs
	err = sSetIkeAlgorithmPrefs (prefref, pPNConfig); CKERR;
	err = sSetIkeProposalPrefs (context, prefref, pPNConfig); CKERR;
	err = sSetIpsecProposalPrefs (context, prefref, pPNConfig); CKERR;

done:
	err = PGPnetClosePrefs (prefref, TRUE);

	if (pHostList)
		PGPFreeData (pHostList);

	return err;
}


//	____________________________________
//
//	load configuration information from prefs file

PGPError
PGPnetAddPromiscuousHostToConfiguration (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig,
		PGPUInt32		uIPAddressHost,
		PGPUInt32		uIPAddressLocal,
		PGPKeyRef		keyAuth,
		PGPSigRef		sigAuth)
{
	PGPError			err			= kPGPError_NoErr;
	PGPSize				size;
	PGPNetHostEntry*	pNewList;
	PGPUInt32			u;
	PGPKeyID			keyid;

	size = pPNConfig->uHostCount * sizeof (PGPNetHostEntry);

	pNewList = PGPNewData (PGPGetContextMemoryMgr (context),
							size + sizeof (PGPNetHostEntry),
							kPGPMemoryMgrFlags_Clear);
	if (pNewList)
	{
		// copy old list to new
		if (pPNConfig->pHostList)
			pgpCopyMemory (pPNConfig->pHostList, pNewList, size);

		// fill in new list element
		u = pPNConfig->uHostCount;
		pNewList[u].hostType			= kPGPnetSecureHost;
		pNewList[u].ipAddress			= uIPAddressHost;
		pNewList[u].ipMask				= 0xFFFFFFFF;
		pNewList[u].childOf				= -1;
		strcpy (pNewList[u].hostName, "Untitled Host");
		pNewList[u].identityType		= kPGPike_ID_IPV4_Addr;
		pNewList[u].identityIPAddress	= uIPAddressLocal;
		pNewList[u].identity[0]			= '\0';
		pNewList[u].sharedSecret[0]		= '\0';

		pNewList[u].authKeyAlg = kPGPPublicKeyAlgorithm_Invalid;
		pNewList[u].authCertIASNLength = 0;

		if (PGPKeyRefIsValid (keyAuth))
		{
			err = PGPGetKeyIDFromKey (keyAuth, &keyid); CKERR;
			err = PGPExportKeyID (&keyid, 
							pNewList[u].authKeyExpKeyID, &size); CKERR;

			err = PGPGetKeyNumber (keyAuth, 
							kPGPKeyPropAlgID,
							&pNewList[u].authKeyAlg); CKERR;

			if (PGPSigRefIsValid (sigAuth))
			{
				PGPGetSigPropertyBuffer (sigAuth, 
							kPGPSigPropX509IASN, 0, NULL,
							&pNewList[u].authCertIASNLength);

				if (pNewList[u].authCertIASNLength > 0)
				{
					err = PGPGetSigPropertyBuffer (sigAuth, 
							kPGPSigPropX509IASN, 
							sizeof(pNewList[u].authCertIASN), 
							pNewList[u].authCertIASN, 
							&pNewList[u].authCertIASNLength);
					CKERR;
				}
			}
		}
	}
	else
		err = kPGPError_OutOfMemory;

done :
	if (IsntPGPError (err))
	{
		if (pPNConfig->pHostList)
			PGPFreeData (pPNConfig->pHostList);

		pPNConfig->pHostList = pNewList;
		pPNConfig->uHostCount++;
	}

	return err;
}


//	____________________________________
//
//	get authentication key and sig refs from keyset

PGPError
PGPnetGetConfiguredAuthKeys (
		PGPContextRef	context,
		PPNCONFIG		pPNConfig,
		PGPKeySetRef	keyset,
		PGPKeyRef*		pkeyPGP,
		PGPKeyRef*		pkeyX509,
		PGPSigRef*		psigX509)
{
	PGPKeyListRef	keylist			= kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyiter			= kInvalidPGPKeyIterRef;
	PGPKeySetRef	keysetX509		= kInvalidPGPKeySetRef;
	PGPKeyRef		key				= kInvalidPGPKeyRef;
	PGPUserIDRef	userid			= kInvalidPGPUserIDRef;
	PGPSigRef		sig				= kInvalidPGPSigRef;
	PGPError		err				= kPGPError_NoErr;
	PGPByte*		pIASN			= NULL;
	PGPBoolean		bPGPKeyNotFound	= FALSE;

	PGPKeyID		keyid;
	PGPSize			size;

	PGPValidatePtr (pkeyPGP);
	PGPValidatePtr (pkeyX509);
	PGPValidatePtr (psigX509);

	*pkeyPGP = kInvalidPGPKeyRef;
	*pkeyX509 = kInvalidPGPKeyRef;
	*psigX509 = kInvalidPGPSigRef;

	if (!PGPKeySetRefIsValid (keyset))
		return kPGPError_BadParams;

	// get PGP key ref
	if (pPNConfig->uPGPAuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		err = PGPImportKeyID (pPNConfig->expkeyidPGPAuthKey, &keyid); CKERR;
		err = PGPGetKeyByKeyID (
				keyset, &keyid, pPNConfig->uPGPAuthKeyAlg, pkeyPGP);

		// if key is missing, we still want to look for X.509 key
		if (err == kPGPError_ItemNotFound)
		{
			err = kPGPError_NoErr;
			bPGPKeyNotFound = TRUE;
		}
		CKERR;
	}

	// get X.509 key 
	if ((pPNConfig->pX509AuthCertIASN != NULL) &&
		(pPNConfig->uX509AuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid))
	{
		err = PGPImportKeyID (pPNConfig->expkeyidX509AuthKey, &keyid); CKERR;
		err = PGPGetKeyByKeyID (
				keyset, &keyid, pPNConfig->uX509AuthKeyAlg, pkeyX509); CKERR;
	}

	// get X.509 sig
	if (PGPKeyRefIsValid (*pkeyX509))
	{
		err = PGPNewSingletonKeySet (*pkeyX509, &keysetX509); CKERR;
		err = PGPOrderKeySet (keysetX509, kPGPAnyOrdering, &keylist); CKERR;
		err = PGPNewKeyIter (keylist, &keyiter); CKERR;

		err = PGPKeyIterNext (keyiter, &key); CKERR;
		err = PGPKeyIterNextUserID (keyiter, &userid); CKERR;
		while (userid) 
		{
			PGPKeyIterNextUIDSig (keyiter, &sig);

			while (	(sig != kInvalidPGPSigRef) &&
					(*psigX509 == kInvalidPGPSigRef))
			{
				size = 0;
				PGPGetSigPropertyBuffer (sig,
							kPGPSigPropX509IASN, 0, NULL, &size);
				if (size == pPNConfig->uX509AuthCertIASNLength)
				{
					pIASN = PGPNewData (
								PGPGetContextMemoryMgr (context),
								size, kPGPMemoryMgrFlags_Clear);
					if (pIASN)
					{
						err = PGPGetSigPropertyBuffer (sig, 
							kPGPSigPropX509IASN, size, pIASN, &size); CKERR;

						if (pgpMemoryEqual (pIASN, 
									pPNConfig->pX509AuthCertIASN, size))
						{
							*psigX509 = sig;
						}

						PGPFreeData (pIASN);
						pIASN = NULL;
					}
				}
				PGPKeyIterNextUIDSig (keyiter, &sig);
			}
			PGPKeyIterNextUserID (keyiter, &userid);
		}
	}

done :
	if (pIASN != NULL)
		PGPFreeData (pIASN);
	if (PGPKeySetRefIsValid (keysetX509))
		PGPFreeKeySet (keysetX509);
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);

	if (IsntPGPError (err) && (bPGPKeyNotFound))
		err = kPGPError_ItemNotFound;

	return err;
}


//	____________________________________
//
//	get host authentication key and sig refs from keyset

PGPError
PGPnetGetConfiguredHostAuthKeys (
		PGPContextRef		context,
		PGPNetHostEntry*	pHost,
		PGPKeySetRef		keyset,
		PGPKeyRef*			pkey,
		PGPSigRef*			psig)
{
	PGPKeyListRef	keylist			= kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyiter			= kInvalidPGPKeyIterRef;
	PGPKeySetRef	keysetX509		= kInvalidPGPKeySetRef;
	PGPKeyRef		key				= kInvalidPGPKeyRef;
	PGPUserIDRef	userid			= kInvalidPGPUserIDRef;
	PGPSigRef		sig				= kInvalidPGPSigRef;
	PGPError		err				= kPGPError_NoErr;
	PGPByte*		pIASN			= NULL;

	PGPKeyID		keyid;
	PGPSize			size;

	PGPValidatePtr (pkey);
	PGPValidatePtr (psig);

	*pkey = kInvalidPGPKeyRef;
	*psig = kInvalidPGPSigRef;

	// if no authentication key, just return
	if (pHost->authKeyAlg == kPGPPublicKeyAlgorithm_Invalid)
	{
		goto done;
	}

	// if no IASN, must be a PGP key
	if (pHost->authCertIASNLength == 0)
	{
		err = PGPImportKeyID (pHost->authKeyExpKeyID, &keyid); CKERR;
		err = PGPGetKeyByKeyID (
				keyset, &keyid, pHost->authKeyAlg, pkey); CKERR;
		goto done;
	}

	// otherwise, must be an X.509 key 
	err = PGPImportKeyID (pHost->authKeyExpKeyID, &keyid); CKERR;
	err = PGPGetKeyByKeyID (
			keyset, &keyid, pHost->authKeyAlg, pkey); CKERR;

	// get X.509 sig
	if (PGPKeyRefIsValid (*pkey))
	{
		err = PGPNewSingletonKeySet (*pkey, &keysetX509); CKERR;
		err = PGPOrderKeySet (keysetX509, kPGPAnyOrdering, &keylist); CKERR;
		err = PGPNewKeyIter (keylist, &keyiter); CKERR;

		err = PGPKeyIterNext (keyiter, &key); CKERR;
		err = PGPKeyIterNextUserID (keyiter, &userid); CKERR;
		while (userid) 
		{
			PGPKeyIterNextUIDSig (keyiter, &sig);

			while (	(sig != kInvalidPGPSigRef) &&
					(*psig == kInvalidPGPSigRef))
			{
				size = 0;
				PGPGetSigPropertyBuffer (sig,
							kPGPSigPropX509IASN, 0, NULL, &size);
				if (size == pHost->authCertIASNLength)
				{
					pIASN = PGPNewData (
								PGPGetContextMemoryMgr (context),
								size, kPGPMemoryMgrFlags_Clear);
					if (pIASN)
					{
						err = PGPGetSigPropertyBuffer (sig, 
							kPGPSigPropX509IASN, size, pIASN, &size); CKERR;

						if (pgpMemoryEqual (pIASN, 
									pHost->authCertIASN, size))
						{
							*psig = sig;
						}

						PGPFreeData (pIASN);
						pIASN = NULL;
					}
				}
				PGPKeyIterNextUIDSig (keyiter, &sig);
			}
			PGPKeyIterNextUserID (keyiter, &userid);
		}

		if (*psig == kInvalidPGPSigRef)
			err = kPGPError_ItemNotFound;
	}

done :
	if (pIASN != NULL)
		PGPFreeData (pIASN);
	if (PGPKeySetRefIsValid (keysetX509))
		PGPFreeKeySet (keysetX509);
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);

	return err;
}


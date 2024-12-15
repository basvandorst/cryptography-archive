/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Prefs.c,v 1.2.8.1 1998/11/12 03:14:11 heller Exp $
____________________________________________________________________________*/
// System Headers
#include <windows.h> 
#include <windowsx.h>
#include <assert.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpSymmetricCipher.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpDebug.h"

// Shared Headers
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pgpBuildFlags.h"
#include "PGPcl.h"
#include "Prefs.h"


BOOL UsePGPMimeForEncryption(PGPMemoryMgrRef memoryMgr)
{
	PGPBoolean	bUseMime	= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMailEncryptPGPMIME, &bUseMime);
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bUseMime;
}


BOOL UsePGPMimeForSigning(PGPMemoryMgrRef memoryMgr)
{
	PGPBoolean	bUseMime	= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMailSignPGPMIME, &bUseMime);
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bUseMime;
}

BOOL ByDefaultEncrypt(PGPMemoryMgrRef memoryMgr)
{
	PGPBoolean	bEncrypt	= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMailEncryptDefault, &bEncrypt);
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bEncrypt;
}


BOOL ByDefaultSign(PGPMemoryMgrRef memoryMgr)
{
	PGPBoolean	bSign		= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMailSignDefault, &bSign);
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bSign;
}


BOOL ByDefaultWordWrap(PGPMemoryMgrRef memoryMgr, long* pThreshold)
{
	PGPBoolean	bWrap		= FALSE;
	PGPUInt32	width		= 0;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefWordWrapEnable, &bWrap);
	PGPGetPrefNumber(prefRef, kPGPPrefWordWrapWidth, &width);
	PGPclCloseClientPrefs(prefRef, FALSE);

	*pThreshold = width;
	return (BOOL) bWrap;
}


BOOL GetCommentString(PGPMemoryMgrRef memoryMgr, 
					  char *szComment, 
					  int nLength)
{
	PGPBoolean	bGotString	= FALSE;
	PGPPrefRef	prefRef		= NULL;

#if PGP_BUSINESS_SECURITY

#if PGP_BUSINESS_SECURITY && PGP_ADMIN_BUILD
	PGPclOpenAdminPrefs(memoryMgr, &prefRef, TRUE);
#else
	PGPclOpenAdminPrefs(memoryMgr, &prefRef, FALSE);
#endif

	PGPGetPrefStringBuffer(prefRef, kPGPPrefComments, nLength, szComment);
	if (strlen(szComment) > 0)
		bGotString = TRUE;
	PGPclCloseAdminPrefs(prefRef, FALSE);

	if (bGotString)
		return (BOOL) bGotString;

#endif	/* PGP_BUSINESS_SECURITY */

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefStringBuffer(prefRef, kPGPPrefComment, nLength, szComment);
	if (strlen(szComment) > 0)
		bGotString = TRUE;
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bGotString;
}


BOOL GetPreferredAlgorithm(PGPMemoryMgrRef memoryMgr, 
						   PGPCipherAlgorithm *prefAlg)
{
	BOOL		bGotAlgorithm	= FALSE;
	PGPUInt32	alg				= 0;
	PGPPrefRef	prefRef			= NULL;
	PGPError	err				= kPGPError_NoErr;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	err = PGPGetPrefNumber(prefRef, kPGPPrefPreferredAlgorithm, &alg);
	PGPclCloseClientPrefs(prefRef, FALSE);

	if (IsntPGPError(err))
	{
		*prefAlg = (PGPCipherAlgorithm) alg;
		bGotAlgorithm = TRUE;
	}
	else
		*prefAlg = kPGPCipherAlgorithm_CAST5;

	return bGotAlgorithm;
}


BOOL GetAllowedAlgorithms(PGPMemoryMgrRef memoryMgr, 
						  PGPCipherAlgorithm *pAllowedAlgs, 
						  int *pNumAlgs)
{
	BOOL				bGotAlgorithms	= FALSE;
	PGPPrefRef			prefRef			= NULL;
	PGPCipherAlgorithm *pData			= NULL;
	PGPSize				dataLength		= 0;
	PGPError			err				= kPGPError_NoErr;
	int					i				= 0;

	pgpAssert(pAllowedAlgs != NULL);
	pgpAssert(pNumAlgs != NULL);

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	err = PGPGetPrefData(prefRef, kPGPPrefAllowedAlgorithmsList, 
			&dataLength, (void **) &pData);

	if (IsntPGPError(err))
	{
		*pNumAlgs = dataLength / sizeof(PGPCipherAlgorithm);
		for (i=0; i<*pNumAlgs; i++)
			pAllowedAlgs[i] = pData[i];

		bGotAlgorithms = TRUE;
		PGPDisposePrefData(prefRef, (void *) pData);
	}
	else
		*pNumAlgs = 0;

	PGPclCloseClientPrefs(prefRef, FALSE);
	return bGotAlgorithms;
}


BOOL AutoDecrypt(PGPMemoryMgrRef memoryMgr)
{
	PGPBoolean	bAuto		= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefAutoDecrypt, &bAuto);
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bAuto;
}


BOOL SyncOnVerify(PGPMemoryMgrRef memoryMgr)
{
	PGPBoolean	bSync		= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefKeyServerSyncOnVerify, &bSync);
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bSync;
}


BOOL MarginalIsInvalid(PGPMemoryMgrRef memoryMgr)
{
	PGPBoolean	bInvalid	= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMarginalIsInvalid, &bInvalid);
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bInvalid;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

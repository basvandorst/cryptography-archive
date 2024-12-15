/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ExchPrefs.cpp,v 1.10 1997/10/10 23:33:28 dgal Exp $
____________________________________________________________________________*/
#include <windows.h>
#include "ExchPrefs.h"
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpSymmetricCipher.h"
#include "PGPcmdlg.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pgpBuildFlags.h"
#include "pgpDebug.h"

BOOL UsePGPMime()
{
	PGPBoolean	bUseMime	= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPcomdlgOpenClientPrefs(&prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMailEncryptPGPMIME, &bUseMime);
	PGPcomdlgCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bUseMime;
}


BOOL UsePGPMimeForSigning()
{
	PGPBoolean	bUseMime	= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPcomdlgOpenClientPrefs(&prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMailSignPGPMIME, &bUseMime);
	PGPcomdlgCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bUseMime;
}

BOOL ByDefaultEncrypt()
{
	PGPBoolean	bEncrypt	= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPcomdlgOpenClientPrefs(&prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMailEncryptDefault, &bEncrypt);
	PGPcomdlgCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bEncrypt;
}


BOOL ByDefaultSign()
{
	PGPBoolean	bSign		= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPcomdlgOpenClientPrefs(&prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefMailSignDefault, &bSign);
	PGPcomdlgCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bSign;
}


BOOL ByDefaultWordWrap(long* pThreshold)
{
	PGPBoolean	bWrap		= FALSE;
	PGPUInt32	width		= 0;
	PGPPrefRef	prefRef		= NULL;

	PGPcomdlgOpenClientPrefs(&prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefWordWrapEnable, &bWrap);
	PGPGetPrefNumber(prefRef, kPGPPrefWordWrapWidth, &width);
	PGPcomdlgCloseClientPrefs(prefRef, FALSE);

	*pThreshold = width;
	return (BOOL) bWrap;
}


BOOL GetCommentString(char *szComment, int nLength)
{
	PGPBoolean	bGotString	= FALSE;
	PGPPrefRef	prefRef		= NULL;

#if PGP_BUSINESS_SECURITY

#if PGP_BUSINESS_SECURITY && PGP_ADMIN_BUILD
	PGPcomdlgOpenAdminPrefs(&prefRef, TRUE);
#else
	PGPcomdlgOpenAdminPrefs(&prefRef, FALSE);
#endif

	PGPGetPrefStringBuffer(prefRef, kPGPPrefComments, nLength, szComment);
	if (strlen(szComment) > 0)
		bGotString = TRUE;
	PGPcomdlgCloseAdminPrefs(prefRef, FALSE);

	if (bGotString)
		return (BOOL) bGotString;

#endif	/* PGP_BUSINESS_SECURITY */

	PGPcomdlgOpenClientPrefs(&prefRef);
	PGPGetPrefStringBuffer(prefRef, kPGPPrefComment, nLength, szComment);
	if (strlen(szComment) > 0)
		bGotString = TRUE;
	PGPcomdlgCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bGotString;
}


BOOL GetPreferredAlgorithm(PGPCipherAlgorithm *prefAlg)
{
	BOOL		bGotAlgorithm	= FALSE;
	PGPUInt32	alg				= 0;
	PGPPrefRef	prefRef			= NULL;
	PGPError	err				= kPGPError_NoErr;

	PGPcomdlgOpenClientPrefs(&prefRef);
	err = PGPGetPrefNumber(prefRef, kPGPPrefPreferredAlgorithm, &alg);
	PGPcomdlgCloseClientPrefs(prefRef, FALSE);

	if (IsntPGPError(err))
	{
		*prefAlg = (PGPCipherAlgorithm) alg;
		bGotAlgorithm = TRUE;
	}
	else
		*prefAlg = kPGPCipherAlgorithm_CAST5;

	return bGotAlgorithm;
}


BOOL GetAllowedAlgorithms(PGPCipherAlgorithm *pAllowedAlgs, int *pNumAlgs)
{
	BOOL				bGotAlgorithms	= FALSE;
	PGPPrefRef			prefRef			= NULL;
	PGPCipherAlgorithm *pData			= NULL;
	PGPSize				dataLength		= 0;
	PGPError			err				= kPGPError_NoErr;
	int					i				= 0;

	pgpAssert(pAllowedAlgs != NULL);
	pgpAssert(pNumAlgs != NULL);

	PGPcomdlgOpenClientPrefs(&prefRef);
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

	PGPcomdlgCloseClientPrefs(prefRef, FALSE);
	return bGotAlgorithms;
}


BOOL AutoDecrypt(void)
{
	PGPBoolean	bAuto		= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPcomdlgOpenClientPrefs(&prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefAutoDecrypt, &bAuto);
	PGPcomdlgCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bAuto;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

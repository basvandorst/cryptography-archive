/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MyPrefs.c,v 1.7 1997/09/20 11:30:12 elrod Exp $
____________________________________________________________________________*/
// System Headers
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpClientPrefs.h"
#include "pflPrefTypes.h"

// Shared Headers
#include "PGPcmdlg.h"

// Project Headers
#include "MyPrefs.h"

// Global Variables
extern PGPContextRef	g_pgpContext;

BOOL AutoDecrypt()
{
	PGPBoolean Decrypt = FALSE;
	PGPError error;
	PGPPrefRef PrefRef;

	error = PGPcomdlgOpenClientPrefs( &PrefRef );

	if(IsntPGPError(error) )
	{
		PGPGetPrefBoolean(	PrefRef, 
							kPGPPrefAutoDecrypt, 
							&Decrypt);

		PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
	}

	return (BOOL)Decrypt;
}


BOOL UsePGPMimeForEncryption()
{
	PGPBoolean UseMime = FALSE;
	PGPError error;
	PGPPrefRef PrefRef;

	error = PGPcomdlgOpenClientPrefs( &PrefRef );

	if(IsntPGPError(error) )
	{
		PGPGetPrefBoolean(	PrefRef, 
							kPGPPrefMailEncryptPGPMIME, 
							&UseMime);

		PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
	}

	return (BOOL)UseMime;
}


BOOL UsePGPMimeForSigning()
{
	PGPBoolean UseMime = FALSE;
	PGPError error;
	PGPPrefRef PrefRef;

	error = PGPcomdlgOpenClientPrefs( &PrefRef );

	if(IsntPGPError(error) )
	{
		PGPGetPrefBoolean(	PrefRef, 
							kPGPPrefMailSignPGPMIME, 
							&UseMime);

		PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
	}

	return (BOOL)UseMime;
}

BOOL ByDefaultEncrypt()
{
	PGPBoolean Encrypt = FALSE;
	PGPError error;
	PGPPrefRef PrefRef;

	error = PGPcomdlgOpenClientPrefs( &PrefRef );

	if(IsntPGPError(error) )
	{
		PGPGetPrefBoolean(	PrefRef, 
							kPGPPrefMailEncryptDefault, 
							&Encrypt);

		PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
	}

	return (BOOL)Encrypt;
}


BOOL ByDefaultSign()
{
	PGPBoolean Sign = FALSE;
	PGPError error;
	PGPPrefRef PrefRef;

	error = PGPcomdlgOpenClientPrefs( &PrefRef );

	if(IsntPGPError(error) )
	{
		PGPGetPrefBoolean(	PrefRef, 
							kPGPPrefMailSignDefault, 
							&Sign);

		PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
	}

	return (BOOL)Sign;
}

BOOL ByDefaultWordWrap(long* pThreshold)
{
	PGPBoolean Wrap = FALSE;
	PGPError error;
	PGPPrefRef PrefRef;

	*pThreshold = 0;

	error = PGPcomdlgOpenClientPrefs( &PrefRef );

	if(IsntPGPError(error) )
	{

		PGPGetPrefBoolean(	PrefRef, 
							kPGPPrefWordWrapEnable, 
							&Wrap);

		PGPGetPrefNumber (	PrefRef,
							kPGPPrefWordWrapWidth, 
							(PGPUInt32*)pThreshold);

		PGPcomdlgCloseClientPrefs (PrefRef, FALSE);

	}
	
	return (BOOL)Wrap;
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
	{
		*prefAlg = kPGPCipherAlgorithm_CAST5;
	}

	return bGotAlgorithm;
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

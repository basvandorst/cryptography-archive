//////////////////////////////////////////////////////////////////////////////
// CPublicKeyDialog.cpp
//
// Implementation of the CPublicKeyDialog class.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPublicKeyDialog.cpp,v 1.1.2.7 1998/07/22 01:19:25 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"

#if PGPDISK_PUBLIC_KEY

#include "PGPclx.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpAdminPrefs.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"

#include "PGPdiskPublicKeyUtils.h"
#include "CPublicKeyDialog.h"


////////////
// Constants
////////////

PGPUInt32 kPGPdiskRecipReadOnlyCheckID = 1;

static LPCSTR kPGPdiskRecipReadOnlyCheckTitle = "Read-only";


/////////////////////////////////////////////////
// Class CPublicKeyDialog public member functions
/////////////////////////////////////////////////

// The CPublicKeyDialog constructor.
CPublicKeyDialog::CPublicKeyDialog()
{
	mGotNewKeys = FALSE;

	mNewRecipKeys = NULL;
	mNumNewKeys = 0;
}

// The CPublicKeyDialog destructor.
CPublicKeyDialog::~CPublicKeyDialog()
{
	CleanUp();
}

// CleanUp frees the associated keys if allocated.

void 
CPublicKeyDialog::CleanUp()
{
	if (mGotNewKeys)
	{
		PGPFreeData(mNewRecipKeys);
		PGPFreeKeySet(mNewKeySet);

		mNewRecipKeys = NULL;
		mNumNewKeys = 0;

		mGotNewKeys = FALSE;
	}
}

// AskForKeys displays the recipient dialog.

DualErr	
CPublicKeyDialog::AskForKeys(
	PGPContextRef		context, 
	PGPKeySetRef		allKeys, 
	PGPRecipientSpec	*oldRecipKeys, 
	PGPUInt32			numRecipKeys)
{
	DualErr				derr;
	PGPBoolean			gotClientPrefs, gotOptionsList, marginalIsInvalid;
	PGPBoolean			showMarginalValidity, warnOnADKs;
	PGPMemoryMgrRef		memoryMgr;
	PGPPrefRef			clientPrefsRef	= kInvalidPGPPrefRef;
	PGPOptionListRef	recipientOptions;
	PGPUInt32			newKeysReadOnly;

#if PGP_BUSINESS_SECURITY

	PGPBoolean			gotAdminPrefs	= FALSE;
	PGPPrefRef			adminPrefsRef	= kInvalidPGPPrefRef;	

#endif	// PGP_BUSINESS_SECURITY

	gotClientPrefs = gotOptionsList = FALSE;

	pgpAssert(PGPContextRefIsValid(context));
	pgpAssert(PGPKeySetRefIsValid(allKeys));

	// Cleanup.
	CleanUp();

	// Get memory manager ref.
	memoryMgr = PGPGetContextMemoryMgr(context);

	// Open client prefs.
	derr = PGPclOpenClientPrefs(memoryMgr, &clientPrefsRef);
	gotClientPrefs = derr.IsntError();

#if PGP_BUSINESS_SECURITY

	// Open admin prefs.
	if (derr.IsntError())
	{
		derr = PGPclOpenAdminPrefs(memoryMgr, &adminPrefsRef, TRUE);
		gotAdminPrefs = derr.IsntError();
	}

#endif	// PGP_BUSINESS_SECURITY

	// Build option list for the recipient dialog.
	if (derr.IsntError())
	{
		PGPOptionListRef defRecipOption;

		PGPGetPrefBoolean(clientPrefsRef, kPGPPrefDisplayMarginalValidity, 
			&showMarginalValidity);
		PGPGetPrefBoolean(clientPrefsRef, kPGPPrefMarginalIsInvalid,
			&marginalIsInvalid);
		PGPGetPrefBoolean(clientPrefsRef, kPGPPrefWarnOnADK, &warnOnADKs);

		if (numRecipKeys > 0)
		{
			defRecipOption = PGPOUIDefaultRecipients(context, numRecipKeys, 
				oldRecipKeys);
		}
		else
		{
			defRecipOption = PGPONullOption(context);
		}

		derr = PGPBuildOptionList(context, &recipientOptions, defRecipOption, 
			PGPOUIDisplayMarginalValidity(context, showMarginalValidity), 
			PGPOUIIgnoreMarginalValidity(context, marginalIsInvalid), 
			PGPOUIRecipientList(context, &mNumNewKeys, &mNewRecipKeys), 

			PGPOUIDialogOptions(context, 
				PGPOUICheckbox(context, kPGPdiskRecipReadOnlyCheckID, 
					kPGPdiskRecipReadOnlyCheckTitle, kEmptyString, 0, 
					&newKeysReadOnly), 
				PGPOLastOption(context)), 

			PGPOLastOption(context));

		gotOptionsList = derr.IsntError();
	}

	if (derr.IsntError())
	{
		PGPAdditionalRecipientRequestEnforcement arrEnforcement;

		// Check enforcement.
		arrEnforcement = kPGPARREnforcement_Warn;

	#if PGP_BUSINESS_SECURITY

		{
			DualErr		dummyErr;
			PGPBoolean	enforceRemoteADKClass;
			
			dummyErr = PGPGetPrefBoolean(adminPrefsRef, 
				kPGPPrefEnforceRemoteADKClass, &enforceRemoteADKClass);
				
			if (dummyErr.IsntError() && enforceRemoteADKClass)
			{
				arrEnforcement = kPGPARREnforcement_Strict;
			}
		}

	#endif // PGP_BUSINESS_SECURITY

		// Show the recipient dialog.
		derr = PGPRecipientDialog(context, allKeys, TRUE, &mNewKeySet,
			recipientOptions, PGPOUIEnforceAdditionalRecipientRequests(
			context, arrEnforcement, warnOnADKs), PGPOLastOption(context));

		mGotNewKeys = derr.IsntError();
	}

	if (derr.IsntError())
	{
		mNewKeysReadOnly = (newKeysReadOnly == 1);
	}

	if (gotOptionsList)
		PGPFreeOptionList(recipientOptions);

#if PGP_BUSINESS_SECURITY

	if (gotAdminPrefs)
		PGPclCloseAdminPrefs(adminPrefsRef, FALSE);

#endif	// PGP_BUSINESS_SECURITY

	if (gotClientPrefs)
		PGPclCloseClientPrefs(clientPrefsRef, FALSE);

	return derr;
}

#endif	// PGPDISK_PUBLIC_KEY

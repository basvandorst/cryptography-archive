/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpDialogs.cpp,v 1.49.10.1 1998/11/12 03:22:59 heller Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpMem.h"
#include "pgpUserInterface.h"

#include "pgpContext.h"
#include "pgpDialogs.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpKeyServer.h"
#include "pgpOptionListPriv.h"

#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))

#define	kCommonAllowedOptions						\
			kPGPOptionType_ParentWindowHandle,      \
			kPGPOptionType_TextUI,      \
			kPGPOptionType_WindowTitle

#define	kCommonAllowedPassphraseOptions				\
			kCommonAllowedOptions,					\
			kPGPOptionType_DialogOptions,			\
			kPGPOptionType_DialogPrompt,			\
			kPGPOptionType_OutputPassphrase,		\
			kPGPOptionType_MinPassphraseLength,		\
			kPGPOptionType_MinPassphraseQuality

#define	kCommonAllowedKeyServerOptions				\
			kCommonAllowedOptions

			
CPGPDialogOptions::CPGPDialogOptions(void)
{
	mContext				= kInvalidPGPContextRef;
	mClientOptions 			= kInvalidPGPOptionListRef;
	mWindowTitle			= NULL;
	mServerList				= NULL;
	mServerCount			= 0;
	mSearchBeforeDisplay	= FALSE;
	mTextUI					= FALSE;
	mNewKeys				= NULL;
	mTLSContext				= kInvalidPGPtlsContextRef;
	mPrompt					= NULL;
	
#if PGP_WIN32
	 mHwndParent	= NULL;
#endif
}

CPGPDialogOptions::~CPGPDialogOptions(void)
{
}
	
	PGPError
CPGPDialogOptions::GatherOptions(
	PGPContextRef 		context,
	PGPOptionListRef	optionList)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateContext( context );
	PGPValidateParam( pgpOptionListIsValid( optionList ) );

	mContext		= context;
	mClientOptions	= optionList;

	err = pgpFindOptionArgs( optionList,
				kPGPOptionType_WindowTitle, FALSE, "%p",
				&mWindowTitle );

	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_DialogPrompt, FALSE, "%p",
					&mPrompt );
	}
	
	if( IsntPGPError( err ) )
	{
		PGPBoolean	haveOption;
		PGPUInt32	textUI;
		
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_TextUI, FALSE,
						 "%b%d", &haveOption, &textUI );
		
		if( haveOption )
		{	 
			mTextUI = ( textUI != 0 );
		}
	}

#if PGP_WIN32
	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_ParentWindowHandle, FALSE,
						 "%p", &mHwndParent );
	}
#endif

	if( IsntPGPError( err ) &&
		IsntPGPError( pgpCheckNetworklibAvailability() ) )
	{
		PGPOUIKeyServerParamsDesc	*desc = NULL;
		
		// Key server params are stored at this level because this is the
		// class common to all users of key servers. The legal option
		// checking will filter out illegal calls.
	
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_KeyServerUpdateParams, FALSE,
						 "%p", &desc );
		
		if( IsntNull( desc ) )
		{
			mServerList 			= desc->serverList;
			mServerCount 			= desc->serverCount;
			mTLSContext				= desc->tlsContext;
			mSearchBeforeDisplay 	= desc->searchBeforeDisplay;
			mNewKeys 				= desc->foundKeys;
		}
	}

	return( err );
}

CPGPRecipientDialogOptions::CPGPRecipientDialogOptions(void)
{
	mDialogOptions				= kInvalidPGPOptionListRef;
	mNumDefaultRecipients		= 0;
	mDefaultRecipients			= NULL;
	mDisplayMarginalValidity	= FALSE;
	mIgnoreMarginalValidity		= FALSE;
	mGroupSet					= kInvalidPGPGroupSetRef;
	mClientKeySet				= kInvalidPGPKeySetRef;
	mRecipientKeysPtr			= NULL;
	mEnforcement				= kPGPARREnforcement_None;
	mAlwaysDisplay				= FALSE;
	mAlwaysDisplayWithARRs		= FALSE;
	mRecipientCount				= NULL;
	mRecipientList				= NULL;
}

CPGPRecipientDialogOptions::~CPGPRecipientDialogOptions(void)
{
	if( IsntNull( mDefaultRecipients ) )
	{
		PGPFreeData( (void *) mDefaultRecipients );
		mDefaultRecipients = NULL;
	}
}

	PGPError
CPGPRecipientDialogOptions::GatherOptions(
	PGPContextRef 		context,
	PGPOptionListRef	optionList)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateContext( context );
	PGPValidateParam( pgpOptionListIsValid( optionList ) );

	err = CPGPDialogOptions::GatherOptions( context, optionList );
	if( IsntPGPError( err ) )
	{
		PGPOption	optionData;

		err = pgpSearchOptionSingle( optionList,
					kPGPOptionType_DialogOptions, &optionData );
		if( IsntPGPError( err ) &&
			optionData.type == kPGPOptionType_DialogOptions &&
			PGPOptionListRefIsValid( optionData.subOptions ) )
		{
			mDialogOptions = optionData.subOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPUInt32	optionIndex = 0;
		PGPOption	optionData;
		
		// Multiple default recipient options are allowed, so we
		// loop and look for all of them.
		
		while( IsntPGPError( pgpGetIndexedOptionType( optionList,
					kPGPOptionType_DefaultRecipients, optionIndex, TRUE,
					&optionData, NULL ) ) )
		{
			PGPUInt32			numNewRecipients;
			PGPUInt32			numTotalRecipients;
			PGPRecipientSpec	*newRecipientList;
			
			numNewRecipients = optionData.valueSize /
										sizeof( PGPRecipientSpec );
			numTotalRecipients	= numNewRecipients + mNumDefaultRecipients;
			
			newRecipientList = (PGPRecipientSpec *) PGPNewData(
										PGPGetContextMemoryMgr( context ),
										numTotalRecipients *
											sizeof( PGPRecipientSpec ), 0 );
			if( IsNull( newRecipientList ) )
			{
				err = kPGPError_OutOfMemory;
				break;
			}
			
			if( IsntNull( mDefaultRecipients ) )
			{
				pgpCopyMemory( mDefaultRecipients, newRecipientList,
						mNumDefaultRecipients * sizeof( PGPRecipientSpec ) );
				PGPFreeData( (void *) mDefaultRecipients );
				mDefaultRecipients = NULL;
			}
			
			pgpCopyMemory( 	optionData.value.asPtr,
							&newRecipientList[mNumDefaultRecipients],
							numNewRecipients * sizeof( PGPRecipientSpec ) );
			
			mNumDefaultRecipients 	= numTotalRecipients;
			mDefaultRecipients		= newRecipientList;
		
			++optionIndex;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPBoolean	haveOption;
		PGPUInt32	displayMarginalValidity;

		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_DisplayMarginalValidity, FALSE,
						 "%b%d", &haveOption, &displayMarginalValidity );
	
		if( haveOption )
		{	 
			mDisplayMarginalValidity = displayMarginalValidity;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPBoolean			haveOption;
		PGPOUIARRParamsDesc	arrOptions;

		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_ARREnforcement, FALSE,
						 "%b%d", &haveOption, &arrOptions );
	
		if( haveOption )
		{	 
			mEnforcement = (PGPAdditionalRecipientRequestEnforcement)
					arrOptions.enforcement;
			mAlwaysDisplayWithARRs = arrOptions.displayDialog;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPBoolean	haveOption;
		PGPUInt32	ignoreMarginalValidity;

		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_IgnoreMarginalValidity, FALSE,
						 "%b%d", &haveOption, &ignoreMarginalValidity );
	
		if( haveOption )
		{	 
			mIgnoreMarginalValidity = ignoreMarginalValidity;
		}
	}

	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_RecipientGroups, FALSE,
						 "%p", &mGroupSet );
	}

	if( IsntPGPError( err ) )
	{
		PGPOUIRecipientListDesc	*desc = NULL;

		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_RecipientList, FALSE,
						 "%p", &desc );
	
		if( IsntNull( desc ) )
		{	
			mRecipientCount = desc->recipientCount;
			mRecipientList	= desc->recipientList;
		}
	}
	
	return( err );
}

CPGPRandomDataDialogOptions::CPGPRandomDataDialogOptions(void)
{
	mNeededEntropyBits 	= 0;
}

CPGPRandomDataDialogOptions::~CPGPRandomDataDialogOptions(void)
{
}

CPGPKeyServerDialogOptions::CPGPKeyServerDialogOptions(void)
{
}

CPGPKeyServerDialogOptions::~CPGPKeyServerDialogOptions(void)
{
}


CPGPSearchKeyServerDialogOptions::CPGPSearchKeyServerDialogOptions(void)
{
	mSearchAllServers 	= FALSE;
	mServerList			= NULL;
	mServerCount		= 0;
	mFilter				= kInvalidPGPFilterRef;
	mKeyDescription[0] 	= 0;
}

CPGPSearchKeyServerDialogOptions::~CPGPSearchKeyServerDialogOptions(void)
{
	if( PGPFilterRefIsValid( mFilter ) )
		PGPFreeFilter( mFilter );
}

	PGPError
CPGPSearchKeyServerDialogOptions::NewKeyIDListSearchFilter(
	PGPContextRef	context,
	const PGPKeyID	*keyIDList,
	PGPUInt32		keyIDCount,
	PGPFilterRef	*filter)
{
	PGPError		err 		= kPGPError_NoErr;
	PGPFilterRef	outFilter 	= kInvalidPGPFilterRef;
	PGPUInt32		keyIDIndex;
	
	for( keyIDIndex = 0; keyIDIndex < keyIDCount; keyIDIndex++ )
	{
		PGPFilterRef	curFilter;
		
		err = PGPNewKeyIDFilter( context, &keyIDList[keyIDIndex], &curFilter );
		if( IsntPGPError( err ) )
		{
			if( PGPFilterRefIsValid( outFilter ) )
			{
				PGPFilterRef	newFilter;
				
				err = PGPUnionFilters( curFilter, outFilter, &newFilter );
				if( IsntPGPError( err ) )
				{
					outFilter = newFilter;
				}
			}
			else
			{
				outFilter = curFilter;
			}
		}
	
		if( IsPGPError( err ) )
			break;
	}
	
	if( IsPGPError( err ) &&
		PGPFilterRefIsValid( outFilter ) )
	{
		PGPFreeFilter( outFilter );
		outFilter = kInvalidPGPFilterRef;
	}
	
	*filter = outFilter;
	
	return( err );
}

	PGPError
CPGPSearchKeyServerDialogOptions::NewKeySetSearchFilter(
	PGPContextRef	context,
	PGPKeySetRef	keySet,
	PGPFilterRef	*filter)
{
	PGPError		err 		= kPGPError_NoErr;
	PGPFilterRef	outFilter 	= kInvalidPGPFilterRef;
	PGPKeyListRef	keyList;
	
	err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keyList );
	if( IsntPGPError( err ) )
	{
		PGPKeyIterRef	iter;
		
		err = PGPNewKeyIter( keyList, &iter );
		if( IsntPGPError( err ) )
		{
			PGPKeyRef	theKey;
			
			err = PGPKeyIterNext( iter, &theKey );
			while( IsntPGPError( err ) )
			{
				PGPKeyID	keyID;
				
				err = PGPGetKeyIDFromKey( theKey, &keyID );
				if( IsntPGPError( err ) )
				{
					PGPFilterRef	keyIDFilter;
					
					err = PGPNewKeyIDFilter( context, &keyID, &keyIDFilter );
					if( IsntPGPError( err ) )
					{
						if( PGPFilterRefIsValid( outFilter ) )
						{
							PGPFilterRef	newFilter;
							
							err = PGPUnionFilters( outFilter, keyIDFilter,
										&newFilter );
							if( IsntPGPError( err ) )
							{
								outFilter = newFilter;
							}
						}
						else
						{
							outFilter = keyIDFilter;
						}
					}
				}
				
				if( IsntPGPError( err ) )
					err = PGPKeyIterNext( iter, &theKey );
			}
			
			if( err == kPGPError_EndOfIteration )
				err = kPGPError_NoErr;
				
			PGPFreeKeyIter( iter );
		}
		
		PGPFreeKeyList( keyList );
	}
	
	if( IsPGPError( err ) &&
		PGPFilterRefIsValid( outFilter ) )
	{
		PGPFreeFilter( outFilter );
		outFilter = kInvalidPGPFilterRef;
	}
	
	*filter = outFilter;
	
	return( err );
}

	PGPError
CPGPSearchKeyServerDialogOptions::GatherOptions(
	PGPContextRef 		context,
	PGPOptionListRef	optionList)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateContext( context );
	PGPValidateParam( pgpOptionListIsValid( optionList ) );

	err = CPGPKeyServerDialogOptions::GatherOptions( context, optionList );
	if( IsntPGPError( err ) )
	{
		PGPFilterRef	filter = kInvalidPGPFilterRef;
		PGPUInt32		optionIndex = 0;
		PGPOption		optionData;
		char			keyDescription[256];
		PGPUInt32		numSearchOptions = 0;
		
		// Multiple search source options are allowed, so we
		// loop and look for all of them. If there is a single
		// search key or a key list with one item, we grab a string
		// description of the item for display in the dialog.

		keyDescription[0] = 0;
		
		while( IsntPGPError( pgpGetIndexedOption( optionList, optionIndex,
					TRUE, &optionData ) ) && IsntPGPError( err ) )
		{
			PGPFilterRef	curFilter = kInvalidPGPFilterRef;

			switch( optionData.type )
			{
				case kPGPOptionType_KeyServerSearchFilter:
				{
					curFilter = optionData.value.asFilterRef;
					(void) PGPIncFilterRefCount( curFilter );

					++numSearchOptions;
					break;
				}
				
				case kPGPOptionType_KeyServerSearchKey:
				{
					PGPKeyRef	keyRef = optionData.value.asKeyRef;
					
					if( numSearchOptions == 0 )
					{
						PGPSize	len;
						
						err = PGPGetPrimaryUserIDNameBuffer( keyRef, 
									sizeof( keyDescription ),
									keyDescription, &len );
						keyDescription[len] = 0;
					}
					
					if( IsntPGPError( err ) )
					{
						PGPKeyID	keyID;
						
						err = PGPGetKeyIDFromKey( keyRef, &keyID );
						if( IsntPGPError( err ) )
						{
							err = PGPNewKeyIDFilter( context, &keyID,
										&curFilter );
						}
					}
					
					++numSearchOptions;
					break;
				}
				
				case kPGPOptionType_KeyServerSearchKeyIDList:
				{
					PGPKeyID	*keyIDList;
					PGPUInt32	keyIDCount;
					
					keyIDList 	= (PGPKeyID *) optionData.value.asPtr;
					keyIDCount 	= optionData.valueSize / sizeof( *keyIDList );
					
					if( numSearchOptions == 0 && keyIDCount == 1 )
					{
						err = PGPGetKeyIDString( keyIDList,
									kPGPKeyIDString_Abbreviated,
									keyDescription );
					}

					if( IsntPGPError( err ) )
					{
						err = NewKeyIDListSearchFilter( context, keyIDList,
										keyIDCount, &curFilter );
					}
					
					++numSearchOptions;
					break;
				}

				case kPGPOptionType_KeyServerSearchKeySet:
				{
					err = NewKeySetSearchFilter( context,
								optionData.value.asKeySetRef, &curFilter );
					
					++numSearchOptions;
					break;
				}
			}
			
			if( PGPFilterRefIsValid( curFilter ) &&
				IsntPGPError( err ) )
			{
				if( PGPFilterRefIsValid( filter ) )
				{
					PGPFilterRef	newFilter;
					
					err = PGPUnionFilters( filter, curFilter, &newFilter );
					if( IsntPGPError( err ) )
					{
						filter = newFilter;
					}
				}
				else
				{
					filter = curFilter;
				}
			}
			
			++optionIndex;
		}
		
		if( IsntPGPError( err ) )
		{
			if( numSearchOptions == 0 )
			{
				pgpDebugMsg( "No key server subjects found" );
				err = kPGPError_BadParams;
			}
			else
			{
				pgpAssert( PGPFilterRefIsValid( filter ) );
					
				mFilter = filter;
			
				if( numSearchOptions == 1 && keyDescription[0] != 0 )
					strcpy( mKeyDescription, keyDescription );
			}
		}
		else
		{
			if( PGPFilterRefIsValid( filter ) )
				PGPFreeFilter( filter );
		}
	}

	return( err );
}

CPGPSendToKeyServerDialogOptions::CPGPSendToKeyServerDialogOptions(void)
{
	mKeysToSend	= kInvalidPGPKeySetRef;
	mFailedKeys	= NULL;
}

CPGPSendToKeyServerDialogOptions::~CPGPSendToKeyServerDialogOptions(void)
{
}

CPGPPassphraseDialogOptions::CPGPPassphraseDialogOptions(void)
{
	mPassphrasePtr			= NULL;
	mDialogOptions			= kInvalidPGPOptionListRef;
	mMinPassphraseLength	= 0;
	mMinPassphraseQuality	= 0;
}

CPGPPassphraseDialogOptions::~CPGPPassphraseDialogOptions(void)
{
}

	PGPError
CPGPPassphraseDialogOptions::GatherOptions(
	PGPContextRef 		context,
	PGPOptionListRef	optionList)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateContext( context );
	PGPValidateParam( pgpOptionListIsValid( optionList ) );

	err = CPGPDialogOptions::GatherOptions( context, optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_OutputPassphrase, TRUE,
						 "%p", &mPassphrasePtr );
	}

	if( IsntPGPError( err ) )
	{
		PGPOption	optionData;
		
		err = pgpSearchOptionSingle( optionList,
					kPGPOptionType_DialogOptions, &optionData );
		if( IsntPGPError( err ) &&
			optionData.type == kPGPOptionType_DialogOptions &&
			PGPOptionListRefIsValid( optionData.subOptions ) )
		{
			mDialogOptions = optionData.subOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_MinPassphraseLength, FALSE,
						 "%d", &mMinPassphraseLength );
	}

	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_MinPassphraseQuality, FALSE,
						 "%d", &mMinPassphraseQuality );
	}

	return( err );
}

CPGPConfirmationPassphraseDialogOptions::
	CPGPConfirmationPassphraseDialogOptions(void)
{
	mShowPassphraseQuality = TRUE;
}

CPGPConfirmationPassphraseDialogOptions::
	~CPGPConfirmationPassphraseDialogOptions(void)
{
}

	PGPError
CPGPConfirmationPassphraseDialogOptions::GatherOptions(
	PGPContextRef 		context,
	PGPOptionListRef	optionList)
{
	PGPError	err;
	
	PGPValidateContext( context );
	PGPValidateParam( pgpOptionListIsValid( optionList ) );
	
	err = CPGPPassphraseDialogOptions::GatherOptions( context, optionList );
	if( IsntPGPError( err ) )
	{
		PGPBoolean	haveOption;
		PGPUInt32	showPassphraseQuality;

		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_ShowPassphraseQuality, FALSE,
						 "%b%d", &haveOption, &showPassphraseQuality );
	
		if( haveOption )
		{	 
			mShowPassphraseQuality = ( showPassphraseQuality != 0 );
		}
	}
	
	return( err );
}

CPGPKeyPassphraseDialogOptions::CPGPKeyPassphraseDialogOptions(void)
{
	mVerifyPassphrase 	= TRUE;
	mDefaultKey			= kInvalidPGPKeyRef;
}

CPGPKeyPassphraseDialogOptions::~CPGPKeyPassphraseDialogOptions(void)
{
}

	PGPError
CPGPKeyPassphraseDialogOptions::GatherOptions(
	PGPContextRef 		context,
	PGPOptionListRef	optionList)
{
	PGPError	err;
	
	PGPValidateContext( context );
	PGPValidateParam( pgpOptionListIsValid( optionList ) );

	err = CPGPPassphraseDialogOptions::GatherOptions( context, optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_DefaultKey, FALSE,
						 "%p", &mDefaultKey );
	}

	if( IsntPGPError( err ) )
	{
		PGPBoolean	haveOption;
		PGPUInt32	verifyPassphrase;
		
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_VerifyPassphrase, FALSE,
						 "%b%d", &haveOption, &verifyPassphrase );
		
		if( haveOption )
		{	 
			mVerifyPassphrase = ( verifyPassphrase != 0 );
		}
	}
		
	return( err );
}

CPGPKeySetPassphraseDialogOptions::CPGPKeySetPassphraseDialogOptions(void)
{
	mFindMatchingKey	= TRUE;
	mKeySet				= kInvalidPGPKeySetRef;
	mPassphraseKeyPtr	= NULL;
}

CPGPKeySetPassphraseDialogOptions::~CPGPKeySetPassphraseDialogOptions(void)
{
}

	PGPError
CPGPKeySetPassphraseDialogOptions::GatherOptions(
	PGPContextRef 		context,
	PGPOptionListRef	optionList)
{
	PGPError	err;
	
	PGPValidateContext( context );
	PGPValidateParam( pgpOptionListIsValid( optionList ) );

	err = CPGPKeyPassphraseDialogOptions::GatherOptions(context, optionList);
	if( IsntPGPError( err ) )
	{
		PGPBoolean	haveOption;
		PGPUInt32	findMatchingKey;
		
		err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_FindMatchingKey, FALSE,
						 "%b%d", &haveOption, &findMatchingKey );
		
		if( haveOption )
		{	 
			mFindMatchingKey = ( findMatchingKey != 0 );
		}
	}
		
	return( err );
}

CPGPSigningPassphraseDialogOptions::CPGPSigningPassphraseDialogOptions(void)
{
}

CPGPSigningPassphraseDialogOptions::~CPGPSigningPassphraseDialogOptions(void)
{
}

CPGPDecryptionPassphraseDialogOptions::
	CPGPDecryptionPassphraseDialogOptions(void)
{
	mMissingKeyIDList 	= NULL;
	mMissingKeyIDCount	= 0;
}

CPGPDecryptionPassphraseDialogOptions::
	~CPGPDecryptionPassphraseDialogOptions(void)
{
	if( IsntNull( mMissingKeyIDList ) )
		PGPFreeData( mMissingKeyIDList );
}

	PGPError
CPGPDecryptionPassphraseDialogOptions::RemoveFoundKeysFromSet(
	PGPKeySetRef keySet)
{
	PGPInt32	keyIDIndex;
	PGPError	err = kPGPError_NoErr;
	
	/*
	** Process keySet and remove any keys in the set from the missing
	** keys list. The following is ugly N-squared behavior. We need to
	** walk the key ID list and eliminate keys already in mKeySet, however
	** PGPGetKeyByKeyID requires an algorithm, which we do not have.
	** In addition, the key ID list contains subkey IDs, not the
	** signing key IDs. There is no lookup function for this case.
	** Thus, we walk the key id list and then iterate the key set
	** to see if we have the key. If not, the key ID is added to the
	** list of missing keys.
	*/

	for( keyIDIndex = mMissingKeyIDCount - 1; keyIDIndex >= 0; keyIDIndex-- )
	{
		PGPKeyListRef	keyList;
		PGPBoolean		foundKeyID = FALSE;
		
		err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keyList );
		if( IsntPGPError( err ) )
		{
			PGPKeyIterRef	iter;
			
			err = PGPNewKeyIter( keyList, &iter );
			if( IsntPGPError( err ) )
			{
				PGPKeyRef	theKey;
				
				err = PGPKeyIterNext( iter, &theKey );
				while( IsntPGPError( err ) )
				{
					PGPKeyID	keyID;
								
					err = PGPGetKeyIDFromKey( theKey, &keyID );
					if( IsntPGPError( err ) )
					{
						if( PGPCompareKeyIDs( &keyID,
									&mMissingKeyIDList[keyIDIndex] ) == 0 )
						{
							foundKeyID = TRUE;
						}
						else
						{
							PGPSubKeyRef	subKey;
							
							err = PGPKeyIterNextSubKey( iter, &subKey );
							while( IsntPGPError( err ) )
							{
								err = PGPGetKeyIDFromSubKey( subKey, &keyID );
								if( IsntPGPError( err ) )
								{
									if( PGPCompareKeyIDs( &keyID,
										&mMissingKeyIDList[keyIDIndex] ) == 0 )
									{
										foundKeyID = TRUE;
										break;
									}

									err = PGPKeyIterNextSubKey(iter, &subKey);
								}
							}

							if( err == kPGPError_EndOfIteration )
								err = kPGPError_NoErr;
						}
					}
						
					if( IsntPGPError( err ) )
						err = PGPKeyIterNext( iter, &theKey );
				}
				
				if( err == kPGPError_EndOfIteration )
					err = kPGPError_NoErr;
					
				PGPFreeKeyIter( iter );
			}
			
			PGPFreeKeyList( keyList );
		}
	
		if( IsntPGPError( err ) && foundKeyID )
		{
			PGPUInt32	keysToMove;
			
			keysToMove = mMissingKeyIDCount - keyIDIndex - 1;
			if( keysToMove != 0 )
			{
				pgpCopyMemory( &mMissingKeyIDList[keyIDIndex + 1],
						&mMissingKeyIDList[keyIDIndex],
						keysToMove * sizeof( mMissingKeyIDList[0] ) );
			}
			
			--mMissingKeyIDCount;
		}
	
		if( IsPGPError( err ) )
			break;
	}
	
	return( err );
}

	PGPError
CPGPDecryptionPassphraseDialogOptions::GatherMissingKeys()
{
	PGPError	err = kPGPError_NoErr;

	if( IsntNull( mMissingKeyIDList ) )
	{
		PGPFreeData( mMissingKeyIDList );
		
		mMissingKeyIDList 	= NULL;
		mMissingKeyIDCount	= 0;
	}

	if( IsntNull( mKeyIDList ) && mKeyIDCount != 0 )
	{
		PGPSize	dataSize = mKeyIDCount * sizeof( mKeyIDList[0] );
		
		/* Assume all keys are missing. Make a copy of the key ID list */
		
		mMissingKeyIDList = (PGPKeyID *) PGPNewData(
										PGPGetContextMemoryMgr( mContext ),
										dataSize, kPGPMemoryMgrFlags_Clear );
		if( IsntNull( mMissingKeyIDList ) )
		{
			pgpCopyMemory( mKeyIDList, mMissingKeyIDList, dataSize );
			mMissingKeyIDCount = mKeyIDCount;
			
			err = RemoveFoundKeysFromSet( mKeySet );
			if( IsntPGPError( err ) &&
				IsntNull( mNewKeys ) &&
				PGPKeySetRefIsValid( *mNewKeys ) )
			{
				err = RemoveFoundKeysFromSet( *mNewKeys );
			}
			
			if( IsPGPError( err ) || mMissingKeyIDCount == 0 )
			{
				PGPFreeData( mMissingKeyIDList );
				mMissingKeyIDList 	= NULL;
				mMissingKeyIDCount 	= 0;
			}
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
	}
	
	return( err );
}

	PGPError
CPGPDecryptionPassphraseDialogOptions::SearchForMissingKeys(
	void		*hwndParent,
	PGPBoolean	*foundNewKeys)
{
#if PGP_MACINTOSH
	(void) hwndParent;
#endif

	PGPError	err = kPGPError_NoErr;
	
	(void) hwndParent;
	
	pgpAssert( IsntNull( foundNewKeys ) );
	*foundNewKeys = FALSE;

	if( IsntNull( mServerList ) &&
		IsntNull( mMissingKeyIDList ) &&
		IsntNull( mNewKeys ) )
	{
		pgpAssert( ! PGPKeySetRefIsValid( *mNewKeys ) );
		
		err = PGPNewKeySet( mContext, mNewKeys );
		if( IsntPGPError( err ) )
		{
			PGPUInt32	serverIndex;
			PGPUInt32	numNewKeys = 0;
			
			for( serverIndex = 0; serverIndex < mServerCount; serverIndex++ )
			{
				if( IsntNull( mMissingKeyIDList ) )
				{
					PGPKeyServerType		serverType;
					const PGPKeyServerSpec	*serverSpec;
					
					serverSpec = &mServerList[serverIndex];
					
					err = PGPGetKeyServerType( serverSpec->server,
								&serverType );
					if( IsntPGPError( err ) &&
						serverType != kPGPKeyServerType_HTTP &&
						serverType != kPGPKeyServerType_HTTPS )
					{
						PGPFilterRef	filter = kInvalidPGPFilterRef;
						PGPUInt32		keyIndex;
						
						for( keyIndex = 0; keyIndex < mMissingKeyIDCount;
								keyIndex++ )
						{
							PGPFilterRef	curFilter;
							
							err = PGPNewSubKeyIDFilter( mContext,
										&mMissingKeyIDList[keyIndex],
										&curFilter );
							if( IsntPGPError( err ) )
							{
								if( PGPFilterRefIsValid( filter ) )
								{
									PGPFilterRef	newFilter;
									
									err = PGPUnionFilters( curFilter, filter,
												&newFilter );
									if( IsntPGPError( err ) )
									{
										filter = newFilter;
									}
								}
								else
								{
									filter = curFilter;
								}
							}
							
							if( IsPGPError( err ) )
								break;
						}
						
						if( IsntPGPError( err ) )
						{
							PGPKeySetRef	foundKeys = kInvalidPGPKeySetRef;
							
							err = PGPSearchKeyServerDialog( mContext, 1,
									serverSpec, mTLSContext, TRUE, &foundKeys,
									PGPOUIKeyServerSearchFilter( mContext,
										filter ),
			#if PGP_WIN32
									PGPOUIParentWindowHandle( mContext,
										hwndParent),
			#endif
									PGPOLastOption( mContext ) );
							if( IsntPGPError( err ) &&
								PGPKeySetRefIsValid( foundKeys ) )
							{
								err = PGPAddKeys( foundKeys, *mNewKeys );
								if( IsntPGPError( err ) )
								{
									err = PGPCommitKeyRingChanges( *mNewKeys );
									if( IsntPGPError( err ) )
									{
										err = GatherMissingKeys();
									}
								}
								
								PGPFreeKeySet( foundKeys );
							}
						}
						
						if( PGPFilterRefIsValid( filter ) )
							PGPFreeFilter( filter );
					}
					
				} 
			
				if( IsPGPError( err ) )
					break;
			}
			
			(void) PGPCountKeys( *mNewKeys, &numNewKeys );
			if( numNewKeys != 0 )
			{
				*foundNewKeys = TRUE;
			}
			else
			{
				PGPFreeKeySet( *mNewKeys );
				*mNewKeys = kInvalidPGPKeySetRef;
			}
		}
	}
	
	return( err );
}

CPGPOptionsDialogOptions::CPGPOptionsDialogOptions(void)
{
}

CPGPOptionsDialogOptions::~CPGPOptionsDialogOptions(void)
{
}

static const PGPOptionType sPassphraseOptionSet[] =
{
	kCommonAllowedPassphraseOptions
};

	static PGPError
pgpPassphraseDialog(
	PGPContextRef		context,
	PGPOptionListRef 	optionList)
{
	PGPError	err;

	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sPassphraseOptionSet,
					elemsof( sPassphraseOptionSet ) );
		if( IsntPGPError( err ) )
		{
			CPGPPassphraseDialogOptions	options;
			
			err = options.GatherOptions( context, optionList );
			if( IsntPGPError( err ) )
			{
				err = pgpPassphraseDialogPlatform( context, &options );
			}
		}
	}
	
	return( err );
}

	PGPError
PGPPassphraseDialog(
	PGPContextRef		context,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );
	
		err = pgpPassphraseDialog( context, optionList );
	
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}

	PGPError
PGPConventionalDecryptionPassphraseDialog(
	PGPContextRef		context,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );
	
		err = pgpPassphraseDialog( context, optionList );
	
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}

static const PGPOptionType sOptionsOptionSet[] =
{
	kCommonAllowedOptions,
	kPGPOptionType_DialogPrompt,
	kPGPOptionType_Checkbox,
	kPGPOptionType_PopupList
};

	static PGPError
pgpOptionsDialog(
	PGPContextRef		context,
	PGPOptionListRef	optionList)
{
	PGPError			err;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sOptionsOptionSet,
					elemsof( sOptionsOptionSet ) );
		if( IsntPGPError( err ) )
		{
			CPGPOptionsDialogOptions	options;
			
			err = options.GatherOptions( context, optionList );
			if( IsntPGPError( err ) )
			{
				err = pgpOptionsDialogPlatform( context, &options );
			}
		}
	}
	
	return( err );
}

	PGPError
PGPOptionsDialog(
	PGPContextRef		context,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );

		err = pgpOptionsDialog( context, optionList );
		
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	} 
	
	return( err );
}

static const PGPOptionType sKeyOptionSet[] =
{
	kCommonAllowedPassphraseOptions,
	kPGPOptionType_VerifyPassphrase
};

	static PGPError
pgpKeyPassphraseDialog(
	PGPContextRef		context,
	PGPKeyRef			theKey,
	PGPOptionListRef	optionList)
{
	PGPError			err;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sKeyOptionSet,
					elemsof( sKeyOptionSet ) );
		if( IsntPGPError( err ) )
		{
			CPGPKeyPassphraseDialogOptions	options;
			
			err = options.GatherOptions( context, optionList );
			if( IsntPGPError( err ) )
			{
				options.mDefaultKey	= theKey;	/* Overloading mDefaultKey */
				
				err = pgpKeyPassphraseDialogPlatform( context, &options );
			}
		}
	}
	
	return( err );
}

	PGPError
PGPKeyPassphraseDialog(
	PGPContextRef		context,
	PGPKeyRef			theKey,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( PGPKeyRefIsValid( theKey ) );

	if( pgpContextIsValid( context ) &&
		PGPKeyRefIsValid( theKey ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );

		err = pgpKeyPassphraseDialog( context, theKey, optionList );
		
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}

	static PGPError
FindValidSecretKey(
	PGPKeySetRef 	keySet,
	Boolean			signing,
	PGPKeyRef		preferredKey,
	PGPKeyRef		*secretKeyPtr)
{
	PGPError		err;
	PGPKeyListRef	keyList;
	PGPKeyPropName	keyProperty;
	PGPKeyRef		splitKeyRef 		= kInvalidPGPKeyRef;
	PGPKeyRef		secretKeyRef 		= kInvalidPGPKeyRef;
	PGPKeyRef		defaultKey			= kInvalidPGPKeyRef;
	PGPBoolean		foundDefaultKey		= FALSE;
	PGPBoolean		foundPreferredKey	= FALSE;
	PGPBoolean		preferredIsSplitKey	= FALSE;
	PGPBoolean		defaultIsSplitKey	= FALSE;
	
	*secretKeyPtr = kInvalidPGPKeyRef;
	
	/* Note that defaultKey may be a split key */
	(void) PGPGetDefaultPrivateKey( keySet, &defaultKey );
	if( ! PGPKeyRefIsValid( preferredKey ) )
		preferredKey = defaultKey;
		
	if( signing )
	{
		keyProperty = kPGPKeyPropCanSign;
	}
	else
	{
		keyProperty = kPGPKeyPropCanDecrypt;
	}
	
	err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keyList );
	if( IsntPGPError( err ) )
	{
		PGPKeyIterRef	keyIterator;
		
		err = PGPNewKeyIter( keyList, &keyIterator );
		if( IsntPGPError( err ) )
		{
			PGPKeyRef	theKey;
			
			err = PGPKeyIterNext( keyIterator, &theKey );
			while( IsntPGPError( err ) )
			{
				PGPBoolean	canOperate;
				
				err = PGPGetKeyBoolean( theKey, keyProperty, &canOperate );
				if( IsntPGPError( err ) )
				{
					if( canOperate )
					{
						PGPBoolean	isSplitKey = FALSE;
						
						(void) PGPGetKeyBoolean( theKey,
									kPGPKeyPropIsSecretShared, &isSplitKey );

						if( theKey == defaultKey )
						{
							foundDefaultKey = TRUE;
							if( isSplitKey )
								defaultIsSplitKey = TRUE;
						}
						
						if( theKey == preferredKey )
						{
							foundPreferredKey = TRUE;
							if( isSplitKey )
								preferredIsSplitKey = TRUE;
						}
						
						if( isSplitKey )
						{
							if( ! PGPKeyRefIsValid( splitKeyRef ) )
								splitKeyRef = theKey;
						}
						else
						{
							if( ! PGPKeyRefIsValid( secretKeyRef ) )
								secretKeyRef = theKey;

							/*
							** If we've found the preferred key and we have
							** at least one non-split key, we can stop
							*/
							
							if( foundPreferredKey )
								break;
						}
					}
					
					err = PGPKeyIterNext( keyIterator, &theKey );
				}
			}
			
			if( err == kPGPError_EndOfIteration )
				err = kPGPError_NoErr;
				
			PGPFreeKeyIter( keyIterator );
		}
		
		PGPFreeKeyList( keyList );
	}

	if( IsntPGPError( err ) )
	{
		if( PGPKeyRefIsValid( secretKeyRef ) )
		{
			/*
			** Do not allow a split key as the best secret key
			** if we're decrypting and at least one other secret
			** key is available.
			*/
			
			if( ! signing )
			{
				if( preferredIsSplitKey )
					foundPreferredKey = FALSE;

				if( defaultIsSplitKey )
					foundDefaultKey = FALSE;
			}
			
			if( foundPreferredKey )
			{
				*secretKeyPtr = preferredKey;
			}
			else if( foundDefaultKey )
			{
				*secretKeyPtr = defaultKey;
			}
			else
			{	
				*secretKeyPtr = secretKeyRef;
			}
		}
		else if( PGPKeyRefIsValid( splitKeyRef ) )
		{
			if( foundPreferredKey )
			{
				*secretKeyPtr = preferredKey;
			}
			else if( foundDefaultKey )
			{
				*secretKeyPtr = defaultKey;
			}
			else
			{	
				*secretKeyPtr = splitKeyRef;
			}
			
			if( signing )
			{
				err = kPGPError_KeyUnusableForSignature;
			}
			else
			{
				err = kPGPError_KeyUnusableForDecryption;
			}
		}
		else
		{
			err = kPGPError_SecretKeyNotFound;
		}
	}
	
	return( err );
}

static const PGPOptionType sSigningOptionSet[] =
{
	kCommonAllowedPassphraseOptions,
	kPGPOptionType_DefaultKey,
	kPGPOptionType_VerifyPassphrase,
	kPGPOptionType_FindMatchingKey
};

	static PGPError
pgpSigningPassphraseDialog(
	PGPContextRef		context,
	PGPKeySetRef		allKeys,
	PGPOptionListRef	optionList,
	PGPKeyRef			*signingKey)
{
	PGPError	err;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sSigningOptionSet,
					elemsof( sSigningOptionSet ) );
		if( IsntPGPError( err ) )
		{
			CPGPSigningPassphraseDialogOptions	options;
			
			err = options.GatherOptions( context, optionList );
			if( IsntPGPError( err ) )
			{
				err = FindValidSecretKey( allKeys, TRUE, options.mDefaultKey,
								&options.mDefaultKey );
				if( IsntPGPError( err ) )
				{
					options.mKeySet				= allKeys;
					options.mPassphraseKeyPtr	= signingKey;
					
					err = pgpSigningPassphraseDialogPlatform( context,
									&options );
				}
				else if( err == kPGPError_KeyUnusableForSignature )
				{
					/* Special case. We return a split key, if found */
					*signingKey = options.mDefaultKey;
				}
			}
		}
	}
	
	return( err );
}

	PGPError
PGPSigningPassphraseDialog(
	PGPContextRef		context,
	PGPKeySetRef		allKeys,
	PGPKeyRef			*signingKey,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( PGPKeySetRefIsValid( allKeys ) );

	if( IsntNull( signingKey ) )
		*signingKey = kInvalidPGPKeyRef;

	if( pgpContextIsValid( context ) &&
		PGPKeySetRefIsValid( allKeys ) &&
		IsntNull( signingKey ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );

		err = pgpSigningPassphraseDialog( context, allKeys,
						optionList, signingKey );
		
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}

static const PGPOptionType sDecryptionOptionSet[] =
{
	kCommonAllowedPassphraseOptions,
	kPGPOptionType_DefaultKey,
	kPGPOptionType_VerifyPassphrase,
	kPGPOptionType_FindMatchingKey,
	kPGPOptionType_KeyServerUpdateParams
};

	static PGPError
pgpDecryptionPassphraseDialog(
	PGPContextRef		context,
	PGPKeySetRef		recipientKeys,
	const PGPKeyID		keyIDList[],
	PGPUInt32			keyIDCount,
	PGPOptionListRef	optionList,
	PGPKeyRef			*decryptionKey)
{
	PGPError	err;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sDecryptionOptionSet,
					elemsof( sDecryptionOptionSet ) );
		if( IsntPGPError( err ) )
		{
			CPGPDecryptionPassphraseDialogOptions	options;

			err = options.GatherOptions( context, optionList );
			if( IsntPGPError( err ) )
			{
				err = FindValidSecretKey( recipientKeys, FALSE,
							options.mDefaultKey, &options.mDefaultKey );
				
				/* Proceed with dialog even when no keys are found */
				if( err == kPGPError_SecretKeyNotFound )
					err = kPGPError_NoErr;
					
				if( IsntPGPError( err ) )
				{
					options.mKeySet				= recipientKeys;
					options.mPassphraseKeyPtr	= decryptionKey;
					options.mKeyIDList			= keyIDList;
					options.mKeyIDCount			= keyIDCount;
					
					err = options.GatherMissingKeys();
					if( IsntPGPError( err ) )
					{
						err = pgpDecryptionPassphraseDialogPlatform( context,
										&options );
					}
					
					if( IsPGPError( err ) &&
						IsntNull( options.mNewKeys ) &&
						PGPKeySetRefIsValid( *options.mNewKeys ) )
					{
						PGPFreeKeySet( *options.mNewKeys );
						*options.mNewKeys = kInvalidPGPKeySetRef;
					}
				}
				else if( err == kPGPError_KeyUnusableForDecryption )
				{
					/* Special case. We return a split key, if found */
					*decryptionKey = options.mDefaultKey;
				}
			}
		}
	}
	
	return( err );
}

	PGPError
PGPDecryptionPassphraseDialog(
	PGPContextRef		context,
	PGPKeySetRef		recipientKeys,
	PGPUInt32			keyIDCount,
	const PGPKeyID		keyIDList[],
	PGPKeyRef			*decryptionKey,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( PGPKeySetRefIsValid( recipientKeys ) );
	
	if( IsntNull( decryptionKey ) )
		*decryptionKey = kInvalidPGPKeyRef;

	if( pgpContextIsValid( context ) &&
		PGPKeySetRefIsValid( recipientKeys ) &&
		IsntNull( decryptionKey ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );

		err = pgpDecryptionPassphraseDialog( context, recipientKeys, keyIDList,
						keyIDCount, optionList, decryptionKey );
		
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}


static const PGPOptionType sConfirmationPassphraseOptionSet[] =
{
	kCommonAllowedPassphraseOptions,
	kPGPOptionType_ShowPassphraseQuality
};

	static PGPError
pgpConfirmationPassphraseDialog(
	PGPContextRef		context,
	PGPOptionListRef 	optionList,
	PGPUInt32			minPassphraseLength)
{
	PGPError	err;

	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList,
					sConfirmationPassphraseOptionSet,
					elemsof( sConfirmationPassphraseOptionSet ) );
		if( IsntPGPError( err ) )
		{
			CPGPConfirmationPassphraseDialogOptions	options;
			
			err = options.GatherOptions( context, optionList );
			if( IsntPGPError( err ) )
			{
				if( options.mMinPassphraseLength < minPassphraseLength )
				{
					options.mMinPassphraseLength = minPassphraseLength;
				}
					
				err = pgpConfirmationPassphraseDialogPlatform( context, 
									&options );
			}
		}
	}
	
	return( err );
}

	PGPError
PGPConfirmationPassphraseDialog(
	PGPContextRef		context,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );
	
		err = pgpConfirmationPassphraseDialog( context, optionList, 0 );
	
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}

	PGPError
PGPConventionalEncryptionPassphraseDialog(
	PGPContextRef		context,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );
	
		err = pgpConfirmationPassphraseDialog( context, optionList, 0 );
	
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}

static const PGPOptionType sRecipientOptionSet[] =
{
	kCommonAllowedOptions,
	kPGPOptionType_DialogPrompt,
	kPGPOptionType_DialogOptions,
	kPGPOptionType_DefaultRecipients,
	kPGPOptionType_DisplayMarginalValidity,
	kPGPOptionType_IgnoreMarginalValidity,
	kPGPOptionType_RecipientGroups,
	kPGPOptionType_ARREnforcement,
	kPGPOptionType_KeyServerUpdateParams,
	kPGPOptionType_RecipientList
};

	static PGPError
pgpRecipientDialog(
	PGPContextRef		context,
	PGPKeySetRef		allKeys,
	PGPBoolean			alwaysDisplay,
	PGPOptionListRef	optionList,
	PGPKeySetRef 		*recipientKeys)
{
	PGPError			err;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sRecipientOptionSet,
					elemsof( sRecipientOptionSet ) );
		if( IsntPGPError( err ) )
		{
			CPGPRecipientDialogOptions	options;
			
			err = options.GatherOptions( context, optionList );
			if( IsntPGPError( err ) )
			{
				PGPKeySetRef	workingKeySet;
				
				// Copy the set of known keys into an in-memory set so we can
				// make additions via key server searches
				
				err = PGPNewKeySet( context, &workingKeySet );
				if( IsntPGPError( err ) )
				{
					err = PGPAddKeys( allKeys, workingKeySet );
					if( IsntPGPError( err ) )
					{
						err = PGPCommitKeyRingChanges( workingKeySet );
						if( IsntPGPError( err ) )
						{
							options.mClientKeySet		= workingKeySet;
							options.mAlwaysDisplay		= alwaysDisplay;
							options.mRecipientKeysPtr	= recipientKeys;
							
							err = pgpRecipientDialogPlatform( context,
											&options );
													
							if( IsPGPError( err ) &&
								IsntNull( options.mNewKeys ) &&
								PGPKeySetRefIsValid( *options.mNewKeys ) )
							{
								PGPFreeKeySet( *options.mNewKeys );
								*options.mNewKeys = kInvalidPGPKeySetRef;
							}
							
							PGPFreeKeySet( workingKeySet );
						}
					}
				}
			}
		}
	}
	
	return( err );
}

	PGPError
PGPRecipientDialog(
	PGPContextRef		context,
	PGPKeySetRef		allKeys,
	PGPBoolean			alwaysDisplayDialog,
	PGPKeySetRef 		*recipientKeys,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( PGPKeySetRefIsValid( allKeys ) );
	pgpAssert( IsntNull( recipientKeys ) );

	if( pgpContextIsValid( context ) &&
		PGPKeySetRefIsValid( allKeys ) &&
		IsntNull( recipientKeys ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context, FALSE, firstOption,
								args );
		va_end( args );

		err = pgpRecipientDialog( context, allKeys, alwaysDisplayDialog,
						optionList, recipientKeys );
		
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	} 
	
	return( err );
}

static const PGPOptionType sRandomDataOptionSet[] =
{
	kCommonAllowedOptions,
	kPGPOptionType_DialogPrompt
};

	static PGPError
pgpCollectRandomDataDialog(
	PGPContextRef		context,
	PGPUInt32			neededEntropyBits,
	PGPOptionListRef	optionList)
{
	PGPError	err;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sRandomDataOptionSet,
					elemsof( sRandomDataOptionSet ) );
		if( IsntPGPError( err ) )
		{
			CPGPRandomDataDialogOptions	options;
			
			err = options.GatherOptions( context, optionList );
			if( IsntPGPError( err ) )
			{
				options.mNeededEntropyBits = neededEntropyBits;
				
				err = pgpCollectRandomDataDialogPlatform( context, &options );
			}
		}
	}
	
	return( err );
}

	PGPError
PGPCollectRandomDataDialog(
	PGPContextRef 		context,
	PGPUInt32			neededEntropyBits,
	PGPOptionListRef 	firstOption,
	... )
{
	PGPError	err;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context, FALSE, firstOption,
								args );
		va_end( args );

		err = pgpCollectRandomDataDialog( context, neededEntropyBits,
						optionList );
		
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	} 
	
	return( err );
}

static const PGPOptionType sSearchKeyServerOptionSet[] =
{
	kCommonAllowedKeyServerOptions,
	kPGPOptionType_KeyServerSearchFilter,
	kPGPOptionType_KeyServerSearchKeyIDList,
	kPGPOptionType_KeyServerSearchKey,
	kPGPOptionType_KeyServerSearchKeySet
};

#if PGP_WIN32
	PGPError
pgpCheckNetworklibAvailability(void)
{
	// We're simply static linking for now.
	return( kPGPError_NoErr );
}

#endif

	static PGPError
pgpSearchKeyServerDialog(
	PGPContextRef 			context,
	const PGPKeyServerSpec 	serverList[],
	PGPUInt32				serverCount,
	PGPtlsContextRef		tlsContext,
	PGPBoolean				searchAllServers,
	PGPKeySetRef 			*foundKeys,
	PGPOptionListRef		optionList)
{
	PGPError	err;
	
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckNetworklibAvailability();
		if( IsntPGPError( err ) )
		{
			err = pgpCheckOptionsInSet( optionList, sSearchKeyServerOptionSet,
						elemsof( sSearchKeyServerOptionSet ) );
			if( IsntPGPError( err ) )
			{
				CPGPSearchKeyServerDialogOptions	options;
				
				err = options.GatherOptions( context, optionList );
				if( IsntPGPError( err ) )
				{
					options.mServerList			= serverList;
					options.mServerCount		= serverCount;
					options.mTLSContext			= tlsContext;
					options.mSearchAllServers	= searchAllServers;
					options.mNewKeys			= foundKeys;
					
					err = pgpSearchKeyServerDialogPlatform(context, &options);
				}
			}
		}
	}
	
	return( err );
}

	PGPError
PGPSearchKeyServerDialog(
	PGPContextRef 			context,
	PGPUInt32				serverCount,
	const PGPKeyServerSpec 	serverList[],
	PGPtlsContextRef		tlsContext,
	PGPBoolean				searchAllServers,
	PGPKeySetRef 			*foundKeys,
	PGPOptionListRef 		firstOption,
	... )
{
	PGPError	err;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( IsntNull( serverList ) );
	pgpAssert( serverCount >= 1 );
	pgpAssert( IsntNull( foundKeys ) );
	
	if( IsntNull( foundKeys ) )
		*foundKeys = kInvalidPGPKeySetRef;
		
	if( pgpContextIsValid( context ) &&
		IsntNull( serverList ) &&
		serverCount >= 1 &&
		IsntNull( foundKeys ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context, FALSE, firstOption,
								args );
		va_end( args );

		err = pgpSearchKeyServerDialog( context, serverList, serverCount,
						tlsContext, searchAllServers, foundKeys, optionList );
		
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	} 
	
	return( err );
}

	PGPKeyRef
GetKeyForPassphrase(
	PGPKeySetRef	keySet,
	const char *	passphrase,
	PGPBoolean		signing)
{
	PGPKeyRef		theKey	= kInvalidPGPKeyRef;
	PGPError		err;
	PGPKeyListRef	keyListRef;
	PGPBoolean		foundValidKey	= FALSE;
	
	err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keyListRef );
	if( IsntPGPError( err ) )
	{
		PGPKeyIterRef	keyIterator;
	
		err = PGPNewKeyIter( keyListRef, &keyIterator );
		if( IsntPGPError( err ) )
		{
			err = PGPKeyIterNext( keyIterator, &theKey );
			while( IsntPGPError( err ) )
			{
				PGPBoolean	tryKey = FALSE;
				
				if( signing )
				{
					PGPBoolean	canSign;
					
					if( IsntPGPError( PGPGetKeyBoolean( theKey,
							kPGPKeyPropCanSign, &canSign ) ) && canSign )
					{
						tryKey = TRUE;
					}
				}
				else
				{
					PGPBoolean	canDecrypt;
					
					if( IsntPGPError( PGPGetKeyBoolean( theKey,
							kPGPKeyPropCanDecrypt, &canDecrypt ) ) &&
								canDecrypt )
					{
						tryKey = TRUE;
					}
				}
				
				if ( tryKey )
				{
					PGPContextRef	context	= PGPGetKeyContext( theKey );
					
					if ( PGPPassphraseIsValid( theKey,
							PGPOPassphrase( context, passphrase),
							PGPOLastOption( context ) ) )
					{
						foundValidKey	= TRUE;
						break;
					}
				}
				
				err = PGPKeyIterNext( keyIterator, &theKey );
			}
			
			PGPFreeKeyIter( keyIterator );
		}
		
		PGPFreeKeyList( keyListRef );
	}

	return( foundValidKey ? theKey : NULL );
}

static const PGPOptionType sSendToKeyServerOptionSet[] =
{
	kCommonAllowedKeyServerOptions
};

	static PGPError
pgpSendToKeyServerDialog(
	PGPContextRef 			context,
	const PGPKeyServerSpec 	*server,
	PGPtlsContextRef		tlsContext,
	PGPKeySetRef 			keysToSend,
	PGPKeySetRef 			*failedKeys,
	PGPOptionListRef		optionList)
{
	PGPError	err;
	
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckNetworklibAvailability();
		if( IsntPGPError( err ) )
		{
			err = pgpCheckOptionsInSet( optionList, sSendToKeyServerOptionSet,
						elemsof( sSendToKeyServerOptionSet ) );
			if( IsntPGPError( err ) )
			{
				CPGPSendToKeyServerDialogOptions	options;
				
				err = options.GatherOptions( context, optionList );
				if( IsntPGPError( err ) )
				{
					options.mServerList		= server;
					options.mServerCount	= 1;
					options.mTLSContext		= tlsContext;
					options.mKeysToSend		= keysToSend;
					options.mFailedKeys		= failedKeys;
					
					err = pgpSendToKeyServerDialogPlatform(context, &options);
				}
			}
		}
	}
	
	return( err );
}

	PGPError
PGPSendToKeyServerDialog(
	PGPContextRef 			context,
	const PGPKeyServerSpec 	*server,
	PGPtlsContextRef		tlsContext,
	PGPKeySetRef 			keysToSend,
	PGPKeySetRef 			*failedKeys,
	PGPOptionListRef 		firstOption,
	... )
{
	PGPError	err;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( IsntNull( server ) );
	pgpAssert( PGPKeySetRefIsValid( keysToSend ) );
	pgpAssert( IsntNull( failedKeys ) );
	
	if( IsntNull( failedKeys ) )
		*failedKeys = kInvalidPGPKeySetRef;
		
	if( pgpContextIsValid( context ) &&
		IsntNull( server ) &&
		PGPKeySetRefIsValid( keysToSend ) &&
		IsntNull( failedKeys ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context, FALSE, firstOption,
								args );
		va_end( args );

		err = pgpSendToKeyServerDialog( context, server, tlsContext,
						keysToSend, failedKeys, optionList );
		
		PGPFreeOptionList( optionList );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args );
		va_end( args );
		
		err = kPGPError_BadParams;
	} 
	
	return( err );
}

/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: PGPRecipientGrafPort.cp,v 1.63.2.4 1997/11/20 20:41:38 heller Exp $
____________________________________________________________________________*/

#include <LGACaption.h>
#include <LGACheckbox.h>
#include <LGAPushButton.h>
#include <PP_KeyCodes.h>
#include <UDrawingUtils.h>
#include <UGAColorRamp.h>

#include "pgpErrors.h"
#include "pgpGroupsUtil.h"
#include "pgpKeyServer.h"
#include "pgpMem.h"

#include <string.h>
#include "MacStrings.h"

#include "PGPRecipientGrafPort.h"
#include "PGPUILibDialogs.h"
#include "PGPUILibUtils.h"
#include "PGPSharedEncryptDecrypt.h"
#include "CUserIDTable.h"
#include "CPassphraseEdit.h"
#include "CProgressBar.h"
#include "CFocusBorder.h"
#include "pgpDebug.h"
#include "CPrivateKeysPopup.h"
#include "pflPrefTypes.h"
#include "pgpClientPrefs.h"
#include "pgpEncode.h"
#include "pgpKeys.h"

#if PGP_BUSINESS_SECURITY
#include "pgpAdminPrefs.h"
#endif


enum
{
	kMacBinaryPopupNoneMenuItem	= 1,
	kMacBinaryPopupSmartMenuItem,
	kMacBinaryPopupYesMenuItem
};


const ResID		kWarnSpecialADKDialogID = 4752;

const PaneIDT	kUserIDScrollerPaneID			= 3;
const PaneIDT	kUserIDTablePaneID				= 4;
const PaneIDT	kFileOptionsBoxPaneID			= 6;
const PaneIDT	kTextOutputCheckboxPaneID		= 7;
const PaneIDT	kMacBinaryPopupPaneID			= 8;
const PaneIDT	kRecipientScrollerPaneID		= 9;
const PaneIDT	kRecipientTablePaneID			= 10;
const PaneIDT	kFileConvEncryptCheckboxPaneID	= 11;
const PaneIDT	kWipeOriginalCheckboxPaneID		= 12;

const PaneIDT	kTextOptionsBoxPaneID			= 13;
const PaneIDT	kTextConvEncryptCheckboxPaneID	= 14;
const PaneIDT	kInstructionsCaptionPaneID		= 15;
const PaneIDT	kUpdateFromServerButtonPaneID	= 16;

const PaneIDT	kUserIDFocusBorderPaneID		= 17;
const PaneIDT	kRecipientsFocusBorderPaneID	= 18;

CPGPRecipientGrafPort::CPGPRecipientGrafPort(LStream *inStream)
	: CPGPLibGrafPortView(inStream)
{
	mMacBinaryPopup				= NULL;
	mTextOutputCheckbox			= NULL;
	mWipeOriginalCheckbox		= NULL;
	mConvEncryptCheckbox		= NULL;
	mUpdateFromServerButton		= NULL;
	mWindowIsMoveable			= TRUE;
	mWindowIsResizeable			= TRUE;
	mMinMaxSize.top				= 300;	// Minimum resize height
	mMinMaxSize.bottom			= 32000;// Maximum resize height
	mMinMaxSize.left			= 440;	// Minimum resize width
	mMinMaxSize.right			= 640;	// Maximum resize width
	
	mUserIDListsEnabled		= TRUE;
	mHaveNewKeys			= FALSE;
	mHaveKeyServerSupport	= CFM_AddressIsResolved_( PGPKeyServerInit );
	mClientPrefsRef			= kInvalidPGPPrefRef;
	mAdminPrefsRef			= kInvalidPGPPrefRef;
	mContext				= kInvalidPGPContextRef;
	mEnforceRemoteADKClass	= FALSE;
	mGroupSet				= kInvalidPGPGroupSetRef;
	
#if PGP_BUSINESS_SECURITY
	mEnforceOutgoingADK		= FALSE;
	mCorporateKeyRef		= kInvalidPGPKeyRef;
	mOutgoingADKRef			= kInvalidPGPKeyRef;
#endif
}

CPGPRecipientGrafPort::~CPGPRecipientGrafPort()
{
	/* Delete the backing store for the name list */
	PGPRecipientTableItem::DisposeNameData();
}

	void
CPGPRecipientGrafPort::FinishCreateSelf()
{
	CFocusBorder	*userIDBorder;

	CPGPLibGrafPortView::FinishCreateSelf();
	
	mRecipientTable =
		(CUserIDTable *) FindPaneByID( kRecipientTablePaneID );
	mUserIDTable =
		(CUserIDTable *) FindPaneByID( kUserIDTablePaneID );
	
	mUserIDTable->InsertCols(1, 1, NULL, 0, TRUE);
	mUserIDTable->AddListener(this);
	
	mRecipientTable->InsertCols(1, 1, NULL, 0, TRUE);
	mRecipientTable->AddListener(this);
	mRecipientTable->SetAllowDeleteKeyToSend( TRUE );

	mUpdateFromServerButton =
		(LGAPushButton *) FindPaneByID( kUpdateFromServerButtonPaneID );

	if( mHaveKeyServerSupport )
	{
		mUpdateFromServerButton->AddListener( this );
	}
	else
	{
		mUpdateFromServerButton->Hide();
	}
	
	userIDBorder = (CFocusBorder *)
					FindPaneByID( kUserIDFocusBorderPaneID );

	SwitchTarget( userIDBorder );
	AdjustButtons();
}

	PGPError
CPGPRecipientGrafPort::BuildRecipientList()
{
	PGPError		err;
	PGPKeyListRef	keyList;
	
	err = PGPOrderKeySet( mAllKeys, kPGPAnyOrdering, &keyList );
	if( IsntPGPError( err ) )
	{
		PGPKeyIterRef	keyIterator;
		PGPKeyRef		defaultKey	= kInvalidPGPKeyRef;
		
		if( IsPGPError( PGPGetDefaultPrivateKey( mAllKeys, &defaultKey ) ) )
		{
			defaultKey = (PGPKeyRef) kInvalidPGPKeyRef;
		}
		
		err = PGPNewKeyIter( keyList, &keyIterator );
		if( IsntPGPError( err ) )
		{
			PGPKeyRef	theKey;
			
			err = PGPKeyIterNext( keyIterator, &theKey );
			while( IsntPGPError( err ) )
			{
				PGPBoolean	keyCanEncrypt;
				
				if( IsntPGPError( PGPKeyCanEncrypt( theKey,
						&keyCanEncrypt ) ) && keyCanEncrypt )
				{
					Boolean	isDefault	= theKey == defaultKey;
					
					err = mUserIDTable->AddKey( theKey, isDefault, FALSE );
					if( IsntPGPError( err ) )
					{
						// The user value of a key is set to a non-zero
						// value when it has been added to a display list.
						// This is convenient for checking for new keys added
						// to the set via key server searches, etc.
						
						err = PGPSetKeyUserVal( theKey,
								(PGPUserValue) kKeyStateAddedFromClient );
					}
				}
				
				if( IsPGPError( err ) )
					break;
				
				err = PGPKeyIterNext( keyIterator, &theKey );
			}
			
			if( err == kPGPError_EndOfIteration )
				err = kPGPError_NoErr;
				
			PGPFreeKeyIter( keyIterator );
		}
		
		PGPFreeKeyList( keyList );
	}
	
	/* now add all groups */
	if ( IsntNull( mGroupSet ) )
	{
		PGPUInt32			numGroups;
		
		err	= PGPCountGroupsInSet( mGroupSet, &numGroups );
		if ( IsntPGPError( err ) )
		{
			PGPUInt32	index;
			
			for( index = 0; index < numGroups; ++index )
			{
				PGPGroupID	id;
				
				err	= PGPGetIndGroupID( mGroupSet, index, &id );
				if ( IsPGPError( err ) )
					break;
				
				err	= mUserIDTable->AddGroup( mAllKeys,
					mGroupSet, id );
				if ( IsPGPError( err ) )
					break;
			}
		}
	}

	return( err );
}

	void
CPGPRecipientGrafPort::LockRecipient(UInt32 rowIndex)
{
	PGPRecipientTableItem	tableItem;
	STableCell			cell(rowIndex, 1);
	
	mRecipientTable->GetCellData( cell, &tableItem );
		tableItem.locked = TRUE;
	mRecipientTable->SetCellData( cell, &tableItem );
}

#if PGP_BUSINESS_SECURITY	// [


// Returns number of recipient keys not signed by the corporate key

	PGPUInt32
CPGPRecipientGrafPort::GetNumKeysNotSignedByCorporateKey(
	CUserIDTable	*whichList,
	PGPKeyRef 		corporateKeyRef,
	Boolean			markedOnly)
{
	PGPUInt32		numUnsignedKeys	= 0;
	PGPKeySetRef	recipients;
	PGPError		err	= kPGPError_NoErr;
	
	err = whichList->CreateKeySet( mAllKeys, markedOnly, &recipients );
	if ( IsntPGPError( err ) )
	{
		PGPKeyListRef	keyList;
		
		err = PGPOrderKeySet( recipients, kPGPAnyOrdering, &keyList );
		if( IsntPGPError( err ) )
		{
			PGPKeyIterRef	iter;

			err = PGPNewKeyIter( keyList, &iter );
			if( IsntPGPError( err ) )
			{
				PGPKeyRef		key;
				
				while( IsntPGPError( err = PGPKeyIterNext( iter, &key ) ) )
				{
					PGPUserIDRef	userID;
					PGPBoolean		signedByCorporateKey	= FALSE;
					
					/* check all user ids for a corporate sig */
					/* only one need be signed to qualify */
					while( IsntPGPError( err =
						PGPKeyIterNextUserID( iter, &userID)))
					{
						PGPSigRef	sig;
						
						while( IsntPGPError( err =
							PGPKeyIterNextUIDSig( iter, &sig)))
						{
							PGPKeyRef	certifierKey;
						
							err = PGPGetSigCertifierKey( sig,
									mAllKeys, &certifierKey);
							if( IsntPGPError(err) )
							{
								if(certifierKey == corporateKeyRef)
								{
									signedByCorporateKey	= TRUE;
									break;
								}
							}
						}
					}
						
					if ( ! signedByCorporateKey )
					{
						++numUnsignedKeys;
					}
				}
				if ( err == kPGPError_EndOfIteration )
					err	= kPGPError_NoErr;
					
				PGPFreeKeyIter( iter );
			}
			PGPFreeKeyList( keyList );
		}
		PGPFreeKeySet( recipients );
	}
	pgpAssertNoErr( err );
	
	return( numUnsignedKeys );
}


	PGPError
CPGPRecipientGrafPort::MoveOutgoingAdditionalKey(void)
{
	PGPError	err = kPGPError_NoErr;
	
	if( PGPKeyRefIsValid( mOutgoingADKRef ) )
	{
		UInt32	row;
		
		if( mUserIDTable->HaveKey( mOutgoingADKRef, &row ) )
		{
			mUserIDTable->MarkOneRow( row );
			SendMarkedToRecipients();
		}
		
		if( mRecipientTable->HaveKey( mOutgoingADKRef, &row ) )
		{
			if( mEnforceOutgoingADK )
			{
				LockRecipient( row );
			}
		}
		else if( mEnforceOutgoingADK )
		{
			/* Display an error dialog and return to the caller */
			PGPUIWarningAlert( kPGPUIGenericOKAlert,
					kPGPLibDialogsStringListResID,
					kCorporateADKDisabledOrMissingStrIndex );
			
			err = kPGPError_UserAbort;
		}
	}
	
	return( err );
}

#endif	// ] PGP_BUSINESS_SECURITY


	PGPError
CPGPRecipientGrafPort::AddSearchedKeysToUserIDTable(void)
{
	// Process the large key list, looking for unprocessed keys
	// and add them to the user ID list.
	
	PGPKeyListRef	keyList;
	PGPError		err;
	
	err = PGPOrderKeySet( mAllKeys, kPGPAnyOrdering, &keyList );
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
				PGPUserValue	keyState;
				
				err = PGPGetKeyUserVal( theKey, &keyState );
				if( IsntPGPError( err ) )
				{
					if( keyState == (PGPUserValue) kKeyStateNew )
					{
						PGPBoolean	keyCanEncrypt;
						
						if( IsntPGPError( PGPKeyCanEncrypt( theKey,
								&keyCanEncrypt ) ) && keyCanEncrypt )
						{
							err = mUserIDTable->AddKey( theKey, FALSE, FALSE );
							if( IsntPGPError( err ) )
							{
								err = PGPSetKeyUserVal( theKey,
								(PGPUserValue) kKeyStateAddedFromKeySearch );
							}
						}
					}
				}
				
				if( IsPGPError( err ) )
					break;
				
				err = PGPKeyIterNext( keyIterator, &theKey );
			}
			
			if( err == kPGPError_EndOfIteration )
				err = kPGPError_NoErr;
				
			PGPFreeKeyIter( keyIterator );
		}
		
		PGPFreeKeyList( keyList );
	}
	
	return( err );
}

/*
	Locate missing recipient keys and missing group keys and add them
	to the set of all known keys and, in the case of missing keys,
	create list items in the user ID table. The group counts, group
	validity, and key validities are all recomputed.
*/

	PGPError
CPGPRecipientGrafPort::FindMissingRecipients(void)
{
	PGPError		err = kPGPError_NoErr;
	PGPKeySetRef	searchResults;
	
	err = PGPNewKeySet( mContext, &searchResults );
	if( IsntPGPError( err ) )
	{
		TableIndexT		numRows;
		TableIndexT		numColumns;
		TableIndexT 	rowIndex;
		Boolean			foundNewKeys;
		
		foundNewKeys = FALSE;
		
		mRecipientTable->GetTableSize( numRows, numColumns );
		
		for( rowIndex = 1; rowIndex <= numRows; rowIndex++ )
		{
			PGPRecipientTableItem	tableItem;
			STableCell				cell(rowIndex, 1);
			PGPKeySetRef			newKeys;
			
			newKeys = kInvalidPGPKeySetRef;
			
			mRecipientTable->GetCellData( cell, &tableItem );
			
			if( tableItem.itemKind == kTableItemKindMissingKey )
			{
				err = PGPGetKeyFromServerInternal( mContext, mClientPrefsRef,
							NULL, kInvalidPGPKeyRef,
							tableItem.GetNamePtr(), 30, &newKeys );
			}
			else if( tableItem.itemKind == kTableItemKindGroup )
			{
				err = PGPGetGroupFromServerInternal( mContext, mClientPrefsRef,
							tableItem.group.set, tableItem.group.id,
							NULL, 30, &newKeys );
			}

			pgpAssertNoErr( err );
			
			if( IsntPGPError( err ) && PGPKeySetRefIsValid( newKeys ) )
			{
				foundNewKeys = TRUE;
			
				err = PGPAddKeys( newKeys, searchResults );
				if( IsntPGPError( err ) )
				{
					err = PGPCommitKeyRingChanges( searchResults );
				}
			}
			
			if( PGPKeySetRefIsValid( newKeys ) )
				(void) PGPFreeKeySet( newKeys );
				
			if( IsPGPError( err ) )
				break;
		}
		
		if( IsntPGPError( err ) && foundNewKeys )
		{
			err = PGPAddKeys( searchResults, mAllKeys );
			if( IsntPGPError( err ) )
			{
				err = PGPCheckKeyRingSigs( searchResults,
						mAllKeys, FALSE, NULL, 0 );
				if( IsntPGPError( err ) )
				{
					err = PGPPropagateTrust( mAllKeys );
					if( IsntPGPError( err ) )
					{
						err = PGPCommitKeyRingChanges( mAllKeys );
					}
				}
			}
		}
		
		(void) PGPFreeKeySet( searchResults );
		
		if( IsntPGPError( err ) && foundNewKeys )
		{
			mHaveNewKeys = TRUE;
			
			err = AddSearchedKeysToUserIDTable();
		}
		
		/* Always update validity because we may have partial success */
		mUserIDTable->UpdateGroupValidityAndKeyCounts( mAllKeys );
		mRecipientTable->UpdateGroupValidityAndKeyCounts( mAllKeys );
	}
		
	return( err );
}

/*
	Update missing recipients. Find missing recipients and remove
	them from the recipient table and replace with one or more
	user id's if they were added to the user ID list.
*/

	void
CPGPRecipientGrafPort::UpdateMissingRecipients(
	Boolean	*foundAmbiguousKeys)
{
	*foundAmbiguousKeys = FALSE;
	
	if( mHaveKeyServerSupport )
	{
		PGPError	err = kPGPError_NoErr;
	
		err = FindMissingRecipients();
		if( IsntPGPError( err ) )
		{
			TableIndexT		numRows;
			TableIndexT		numColumns;
			TableIndexT 	rowIndex;
			
			/*
				Mark the users in the user table which should be
				moved to the recipient table and mark the missing keys
				in the recipient table which should be deleted
			*/
			
			mRecipientTable->UnmarkAll();
			mUserIDTable->UnmarkAll();
			
			mRecipientTable->GetTableSize( numRows, numColumns );
			
			for( rowIndex = 1; rowIndex <= numRows; rowIndex++ )
			{
				PGPRecipientTableItem	tableItem;
				STableCell				cell(rowIndex, 1);
				
				mRecipientTable->GetCellData( cell, &tableItem );
				
				if( tableItem.itemKind == kTableItemKindMissingKey )
				{
					Boolean	matchedMultiple;
					
					if( FindAndMarkUser( tableItem.GetNamePtr(),
							&matchedMultiple ) )
					{
						if( matchedMultiple )
							*foundAmbiguousKeys = TRUE;

						tableItem.marked = TRUE;
						mRecipientTable->SetCellData( cell, &tableItem );
					}
				}
			}
			
			SendMarkedToRecipients();

			/* Delete found (marked) recipient table items */
			
			mRecipientTable->GetTableSize( numRows, numColumns );

			for( rowIndex = numRows; rowIndex >= 1; rowIndex-- )
			{
				PGPRecipientTableItem	tableItem;
				STableCell				cell(rowIndex, 1);
				
				mRecipientTable->GetCellData( cell, &tableItem );
				
				if( tableItem.marked )
					mRecipientTable->RemoveRows( 1, rowIndex, FALSE );
			}
			
			mRecipientTable->Refresh();
			mUserIDTable->Refresh();
			AdjustButtons();
		}	
		
		if( IsPGPError( err ) && err != kPGPError_UserAbort )
		{
			Str255	errorStr;
			
			PGPGetErrorString( err, sizeof( errorStr ),
				(char *) errorStr );
			CToPString( (char *) errorStr, errorStr );
		
			PGPUIWarningAlert( kPGPUIGenericOKAlert,
				kPGPLibDialogsStringListResID,
				kErrorSearchingServerStrIndex,
				errorStr );
		}
	}
}

typedef struct FoundUserItem
{
	TableIndexT				row;
	PGPRecipientTableItem	tableItem;
	
} FoundUserItem;
	
	Boolean
CPGPRecipientGrafPort::FindAndMarkUser(
	const char 	*userID,
	Boolean		*foundMultiple)
{
	TableIndexT	curRow;
	TableIndexT	foundRow;
	PGPError	err;
	
	pgpAssertAddrValid( userID, char );
	pgpAssertAddrValid( foundMultiple, Boolean );

	// Simplest case first. Assume only one match.
	
	*foundMultiple 	= FALSE;
	err				= kPGPError_NoErr;
	curRow 			= 1;
	foundRow		= 0;
	
	while( mUserIDTable->FindUserID( userID, curRow, &curRow ) )
	{
		if( foundRow == 0 )
		{
			foundRow = curRow++;
		}
		else
		{
			*foundMultiple = TRUE;
			break;
		}
	}
	
	if( *foundMultiple )
	{
		FoundUserItem	**itemList;
		PGPUInt32		numFoundItems;
		PGPUInt32		numRemainingItems;
		UInt32 			itemIndex;
		
		// Build a list of matching rows and then analyze the
		// list to determine if we actually have multiple keys
		// or just multiple user ID's on a single key.
		//
		// In addition, we look for preferred secret keys and
		// preferred user ID's

		itemList 		= (FoundUserItem **) NewHandle( 0 );
		numFoundItems	= 0;
		curRow 			= 1;

		while( mUserIDTable->FindUserID( userID, curRow, &curRow ) )
		{
			FoundUserItem	item;
			STableCell		cell;
			
			item.row = curRow;
			
			cell.row = curRow;
			cell.col = 1;
			
			mUserIDTable->GetCellData( cell, &item.tableItem );
			
			err = PtrAndHand( &item, (Handle) itemList, sizeof( item ) );
			if( IsErr( err ) )
				break;
		
			++numFoundItems;
			++curRow;
		}
		
		HLock( (Handle) itemList );
		
		// Now, analyze the list. We whack the row number of the item
		// to zero if it should not be marked
		
		for( itemIndex = 0; itemIndex < numFoundItems; itemIndex++ )
		{
			FoundUserItem	*foundItem = &(*itemList)[itemIndex];
			
			if( foundItem->row != 0 )
			{
				if( foundItem->tableItem.itemKind == kTableItemKindKey )
				{
					UInt32 	checkItemIndex;
					
					// Scan the rest of the items in the list to find a
					// better match
					
					for( checkItemIndex = itemIndex + 1;
						 checkItemIndex < numFoundItems;
						 checkItemIndex++ )
					{
						FoundUserItem	*checkItem;
						
						checkItem = &(*itemList)[checkItemIndex];
						
						if( checkItem->row != 0 &&
							checkItem->tableItem.itemKind == kTableItemKindKey )
						{
							if( checkItem->tableItem.user.keyRef ==
								foundItem->tableItem.user.keyRef )
							{
								// Same key.
								// See if there is a preferred user ID.
								// If not, whack the check item.
								
								if( checkItem->tableItem.user.isPrimaryUserID )
								{
									foundItem->row = 0;
									break;
								}
								else
								{
									checkItem->row = 0;
								}
							}
							else if( foundItem->tableItem.user.isSecretKey &&
									checkItem->tableItem.user.isSecretKey )
							{
								// Both items are secret keys. Pick the better
								// match i.e. default key first or DSA/DH
								// over RSA
							
								if( checkItem->tableItem.user.isDefaultKey )
								{
									foundItem->row = 0;
									break;
								}
								else if (
									foundItem->tableItem.user.isDefaultKey )
								{
									checkItem->row = 0;
								}
								else if( foundItem->tableItem.user.algorithm ==
												kPGPPublicKeyAlgorithm_DSA &&
										 checkItem->tableItem.user.algorithm !=
												kPGPPublicKeyAlgorithm_DSA)
								{
									checkItem->row = 0;
								}
								else if( foundItem->tableItem.user.algorithm !=
												kPGPPublicKeyAlgorithm_DSA &&
										 checkItem->tableItem.user.algorithm ==
												kPGPPublicKeyAlgorithm_DSA)
								{
									foundItem->row = 0;
									break;
								}
							}
							else
							{
								// Legitimate multiple match.
								// We'll note this later
							}
						}
					}
				}
				else if( foundItem->tableItem.itemKind == kTableItemKindGroup )
				{
					// Groups are always marked. Nothing to do here
				}
			}
		}

		numRemainingItems = 0;
		
		for( itemIndex = 0; itemIndex < numFoundItems; itemIndex++ )
		{
			FoundUserItem	*foundItem = &(*itemList)[itemIndex];
			
			if( foundItem->row != 0 )
			{
				mUserIDTable->MarkOneRow( foundItem->row );
				++numRemainingItems;
			}
		}
		
		*foundMultiple = ( numRemainingItems > 1 );
		
		DisposeHandle( (Handle) itemList );
	}
	else if( foundRow > 0 )
	{
		mUserIDTable->MarkOneRow( foundRow );
	}
	
	return( foundRow != 0 );
}

	PGPError
CPGPRecipientGrafPort::MoveDefaultUserIDs(PGPBoolean *allRecipientsValid)
{
	PGPUInt32	recipientIndex;
	Boolean		haveAmbiguousRecipients	= FALSE;
	Boolean		haveDefaultRecipient 	= FALSE;
	Boolean		haveBadPolicyRecipient	= FALSE;
	Boolean		foundDefaultRecipient 	= FALSE;
	PGPBoolean	encryptToSelf 			= FALSE;
	PGPError	err 					= kPGPError_NoErr;
	short		instructionStrIndex		= 0;

	*allRecipientsValid = ( mNumDefaultRecipients != 0 );
	
	/* mark all items that should be moved to recipient list */
	for(	recipientIndex = 0;
			recipientIndex < mNumDefaultRecipients;
			recipientIndex++ )
	{
		Boolean				matchedMultiple;
		PGPKeySpec const *	recipient;
		
		recipient = &mDefaultRecipients[recipientIndex];
		
		if( FindAndMarkUser( recipient->userIDStr, &matchedMultiple ) )
		{
			if( matchedMultiple )
				haveAmbiguousRecipients = TRUE;
		}
		else
		{
			mRecipientTable->AddMissingUserID( recipient->userIDStr,
						FALSE );
		}
	}
	
	if ( IsntPGPError( err ) )
	{
		SendMarkedToRecipients();
		
	#if PGP_BUSINESS_SECURITY
		if( PGPPrefRefIsValid( mAdminPrefsRef ) )
		{
			err = MoveOutgoingAdditionalKey();
		}
	#endif
	}
	
	// If the "Encrypt to Self" option is set, also add the default user ID
	// to the list
	if( IsntPGPError( err ) )
	{
		err = PGPGetPrefBoolean( mClientPrefsRef,
				kPGPPrefEncryptToSelf, &encryptToSelf );
		if( IsntPGPError( err ) && encryptToSelf )
		{
			PGPKeyRef	defaultKey;
			
			haveDefaultRecipient = TRUE;

		
			if( IsntPGPError( PGPGetDefaultPrivateKey( mAllKeys,
								&defaultKey ) ) )
			{
				UInt32	row;
				
				if( mRecipientTable->HaveKey( defaultKey, &row ) )
				{
					foundDefaultRecipient = TRUE;
				}
				else if( mUserIDTable->HaveKey( defaultKey, &row ) )
				{
					STableCell	theCell;
					
					theCell.col	= 1;
					theCell.row	= row;
					
					mUserIDTable->MarkOneRow( theCell.row );
					SendMarkedToRecipients();
					
					foundDefaultRecipient = TRUE;
				}
			}
			
			if( ! foundDefaultRecipient )
			{
				PGPUIWarningAlert( kPGPUIGenericOKAlert,
					kPGPLibDialogsStringListResID,
					kNoDefaultPrivateKeyFoundStrIndex );
			}
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPUInt32	numUsers;
		PGPUInt32	numGroups;
		PGPUInt32	numIncompleteGroups;
		PGPUInt32	numMissingUsers;
		PGPValidity	minValidity;
		Boolean		haveMissingRecipients 	= FALSE;
		Boolean		haveInvalidRecipients 	= FALSE;
	
		/* Validate the recipient list for missing or invalid keys */

		mRecipientTable->ClassifyItems( &numUsers, &numGroups,
					&numIncompleteGroups, &numMissingUsers, NULL,
					&minValidity, NULL );
	
		if( numIncompleteGroups != 0 ||
			numMissingUsers != 0 )
		{
			haveMissingRecipients = TRUE;
		}
		
		if( haveMissingRecipients && mHaveKeyServerSupport )
		{
			PGPBoolean	searchForUnknownKeys = FALSE;

			(void) PGPGetPrefBoolean( mClientPrefsRef,
				kPGPPrefKeyServerSyncUnknownKeys,
				&searchForUnknownKeys);

			if( searchForUnknownKeys )
			{
				Boolean	foundAmbiguousKeys;
				
				// If we can search the key server, then try to
				// resolve missing keys
		
				UpdateMissingRecipients( &foundAmbiguousKeys );
				if( foundAmbiguousKeys )
					haveAmbiguousRecipients = TRUE;
					
				mRecipientTable->ClassifyItems( &numUsers,
					&numGroups, &numIncompleteGroups,
					&numMissingUsers, NULL, &minValidity, NULL );

				if( numIncompleteGroups != 0 ||
					numMissingUsers != 0 )
				{
					haveMissingRecipients = TRUE;
				}
				else
				{
					haveMissingRecipients = FALSE;
				}
			}
		}

		if( numUsers + numGroups + numMissingUsers > 0 )
		{
			if( minValidity == kPGPValidity_Marginal )
			{
				PGPBoolean	disallowMarginallyValidKeys;

				if( IsPGPError( PGPGetPrefBoolean( mClientPrefsRef, 
							kPGPPrefMarginalIsInvalid,
							&disallowMarginallyValidKeys ) ) )
				{
					disallowMarginallyValidKeys = TRUE;
				}
				
				if( disallowMarginallyValidKeys )
				{
					haveInvalidRecipients = TRUE;
				}
			}
			else if( minValidity != kPGPValidity_Complete )
			{
				haveInvalidRecipients = TRUE;
			}
		}
		
		if( haveDefaultRecipient && ! foundDefaultRecipient )
		{
			*allRecipientsValid = FALSE;
		}
		else if(haveMissingRecipients ||
				haveInvalidRecipients ||
				haveAmbiguousRecipients )
		{
			*allRecipientsValid = FALSE;
		}
		else
		{
			/*
			** Check to see if we should warn i.e display the dialog
			** if there are any adk's
			*/
			
			PGPBoolean	warnHaveADKs;
			
			if( IsPGPError( PGPGetPrefBoolean( mClientPrefsRef,
						kPGPPrefWarnOnADK, &warnHaveADKs) ) )
			{
				warnHaveADKs = TRUE;
			}
			
			if( warnHaveADKs )
			{
				STableCell	cell(0, 0);

				while( mRecipientTable->GetNextCell( cell ) )
				{
					PGPRecipientTableItem	testItem;
					
					mRecipientTable->GetCellData( cell, &testItem );
					
					if( testItem.itemKind == kTableItemKindKey &&
						testItem.user.warnThisADK )
					{
						*allRecipientsValid = FALSE;
						break;
					}
				}
			}
		}
		
	#if PGP_BUSINESS_SECURITY
		if( PGPKeyRefIsValid( mCorporateKeyRef ) )
		{
			if( GetNumKeysNotSignedByCorporateKey( mRecipientTable,
					mCorporateKeyRef, FALSE ) > 0 )
			{
				haveBadPolicyRecipient = TRUE;
				*allRecipientsValid = FALSE;
			}
		}
	#endif

		// Set the user instruction string
		if( haveAmbiguousRecipients )
		{
			instructionStrIndex = kMultipleKeysFoundStrIndex;
		}
		else if( haveMissingRecipients )
		{
			instructionStrIndex = kKeysNotFoundStrIndex;
		}
		else if( haveInvalidRecipients )
		{
			instructionStrIndex = kInvalidKeysStrIndex;
		}
		else if( haveBadPolicyRecipient )
		{
			instructionStrIndex = kKeysNotSignedByCorporateKeyStrIndex;
		}
		
		if( instructionStrIndex != 0 )
		{
			LGACaption	*caption;
			
			caption =
				(LGACaption *)FindPaneByID( kInstructionsCaptionPaneID );
			if( IsntNull( caption ) )
			{
				Str255		msg;

				GetIndString( msg, kPGPLibDialogsStringListResID,
							instructionStrIndex );
				caption->SetDescriptor( msg );
			}
		}
	}
	
	AdjustButtons();
	
	return( err );
}


	void
CPGPRecipientGrafPort::DrawSelf()
{
	ApplyForeAndBackColors();
}

	UInt32
CPGPRecipientGrafPort::SendSelectedToRecipients(void)
{
	mUserIDTable->MarkSelected();
	
#if PGP_BUSINESS_SECURITY
	if(	PGPKeyRefIsValid( mCorporateKeyRef ) &&
		GetNumKeysNotSignedByCorporateKey( mUserIDTable, mCorporateKeyRef,
				TRUE ) > 0)
	{
		PGPUIWarningAlert(kPGPUIGenericOKAlert,
							kPGPLibDialogsStringListResID,
							kKeysNotSignedByCorporateKeyStrIndex);
	}
#endif				

	return( SendMarkedToRecipients() );
}



	Boolean
CPGPRecipientGrafPort::MarkADKeysForUserToRecipientMove( PGPKeyRef key )
{
	PGPError	err;
	UInt32 		adkIndex;
	PGPUInt32	numADKeys;
	Boolean		haveAllMandatoryADKs;
	Boolean		processKeys;
	
	pgpAssert( IsntNull( mAllKeys ) );
	
	err = PGPCountAdditionalRecipientRequests( key, &numADKeys );
	if ( IsPGPError( err ) )
		return ( TRUE );	/* probably kPGPError_ItemNotFound */
	
	haveAllMandatoryADKs 	= TRUE;
	processKeys				= TRUE;

	/* Verify that all of the required adk's are available */
	for( adkIndex = 0; adkIndex < numADKeys; adkIndex++ )
	{
		PGPKeyRef	adkey = nil;
		PGPByte		adkClass;

		err = PGPGetIndexedAdditionalRecipientRequest( key,
					mAllKeys, adkIndex, &adkey, &adkClass );
		if( IsntPGPError( err ) )
		{
			if( ( adkClass & 0x80 ) != 0 && mEnforceRemoteADKClass )
			{
				PGPBoolean	keyCanEncrypt;
				
				/* Verify that the key is in our lists */
				
				err = PGPKeyCanEncrypt( adkey, &keyCanEncrypt );
				if( IsPGPError( err ) || ! keyCanEncrypt )
				{
					haveAllMandatoryADKs = FALSE;
				}
			}
		}
		else
		{
			PGPKeyID	keyID;
			
			/*
				The adkClass parameter from
				PGPGetIndexedAdditionalRecipientRequest
				is not valid in an error case. Use
				PGPGetIndexedAdditionalRecipientRequestKeyID instead.
			*/
			
			err = PGPGetIndexedAdditionalRecipientRequestKeyID( key,
					adkIndex, &keyID, &adkClass );
			if( IsntPGPError( err ) )
			{
				if( ( adkClass & 0x80 ) != 0 && mEnforceRemoteADKClass )
				{
					haveAllMandatoryADKs = FALSE;
				}
			}
			else
			{
				haveAllMandatoryADKs = FALSE;
			}
		}
		
		if( ! haveAllMandatoryADKs )
			break;
	}

#if PGP_BUSINESS_SECURITY
	processKeys = haveAllMandatoryADKs;
#endif

	if( processKeys )
	{
		for( adkIndex = 0; adkIndex < numADKeys; adkIndex++ )
		{
			PGPKeyRef	adkey = nil;
			PGPByte		adkClass;

			err = PGPGetIndexedAdditionalRecipientRequest( key,
						mAllKeys, adkIndex, &adkey, &adkClass );
			if( IsntPGPError( err )  )
			{
				UInt32					adkeyRow;
				PGPRecipientTableItem	adkTableItem;
				
				pgpAssert( PGPKeyRefIsValid( adkey ) );
				
				if( mUserIDTable->HaveKey( adkey, &adkeyRow ) )
				{
					STableCell	cell(adkeyRow, 1);
					
					// Move this key to the recipient table as well.
					// Lock it if we're enforcing ADK's
					
					mUserIDTable->GetCellData( cell, &adkTableItem );

					adkTableItem.moveToOtherList = TRUE;
					++adkTableItem.user.adkRefCount;

					if( ( adkClass & 0x80 ) != 0 )
					{
						adkTableItem.user.isEnforcedADKClass = TRUE;

						if( mEnforceRemoteADKClass )
						{
							adkTableItem.locked = TRUE;
						}
					}

				#if PGP_BUSINESS_SECURITY	// [
					if( PGPKeyRefIsValid( mOutgoingADKRef ) &&
						adkTableItem.user.keyRef != mOutgoingADKRef )
					{
						adkTableItem.user.warnThisADK = TRUE;
					}
					else
					{
						adkTableItem.user.warnThisADK = FALSE;
					}
				#else
					adkTableItem.user.warnThisADK = TRUE;
				#endif	// ]
					
					mUserIDTable->SetCellData( cell, &adkTableItem );
				}
				else if( mRecipientTable->HaveKey( adkey, &adkeyRow ))
				{
					STableCell	cell(adkeyRow, 1);
				
					// Increment the adkRefCount for this adkey
				
					mRecipientTable->GetCellData( cell, &adkTableItem);
						++adkTableItem.user.adkRefCount;
						
					if( ( adkClass & 0x80 ) != 0 )
					{
						adkTableItem.user.isEnforcedADKClass = TRUE;

						if( mEnforceRemoteADKClass )
						{
							adkTableItem.locked = TRUE;
						}
					}

				#if PGP_BUSINESS_SECURITY	// [
					if( PGPKeyRefIsValid( mOutgoingADKRef ) &&
						adkTableItem.user.keyRef != mOutgoingADKRef )
					{
						adkTableItem.user.warnThisADK = TRUE;
					}
					else
					{
						adkTableItem.user.warnThisADK = FALSE;
					}
				#else
					adkTableItem.user.warnThisADK = TRUE;
				#endif	// ]

					mRecipientTable->SetCellData( cell, &adkTableItem);
				}
			}
		}
	}
	
	return( haveAllMandatoryADKs );
}


// Returns number of moved items
	UInt32
CPGPRecipientGrafPort::SendMarkedToRecipients(void)
{
	TableIndexT		numRows;
	TableIndexT		numColumns;
	UInt32			numMovedItems;
	TableIndexT 	rowIndex;
	
	numMovedItems = 0;
	
	mUserIDTable->GetTableSize( numRows, numColumns );

	// First pass: Find all dependent rows and set their moveToOtherList flags
	// Need to iterate backwards because we're removing rows.
	for( rowIndex = numRows; rowIndex >= 1; rowIndex-- )
	{
		PGPRecipientTableItem	tableItem;
		STableCell			cell(rowIndex, 1);
		
		mUserIDTable->GetCellData( cell, &tableItem );
		
		if( tableItem.marked )
		{
			Boolean	haveADKs = TRUE;
			
			tableItem.marked = FALSE;

			if( tableItem.itemKind == kTableItemKindKey )
			{
				mUserIDTable->SetCellData( cell, &tableItem );
				
				if( tableItem.user.numADKeys != 0 )
				{
					haveADKs = MarkADKeysForUserToRecipientMove(
						tableItem.user.keyRef );
					if( ! haveADKs )
					{
						Str255	pUserID;
						short	strIndex;
						
						CToPString( tableItem.GetNamePtr(), pUserID );
						
					#if PGP_BUSINESS_SECURITY
						strIndex = kKeyADKNotFoundErrorStrIndex;
					#else
						strIndex = kKeyADKNotFoundWarningStrIndex;
					#endif
					
						PGPUIWarningAlert( kPGPUIGenericOKAlert,
							kPGPLibDialogsStringListResID,
							strIndex, pUserID );
					}
				}
			}
			else if ( tableItem.itemKind == kTableItemKindGroup )
			{
				haveADKs = MarkADKeysInGroupForUserToRecipientMove( 
					tableItem.group.set, tableItem.group.id );
				if( ! haveADKs )
				{
					Str255	pGroupName;
					short	strIndex;
						
					CToPString( tableItem.GetNamePtr(), pGroupName );
					
				#if PGP_BUSINESS_SECURITY
					strIndex = kGroupADKNotFoundErrorStrIndex;
				#else
					strIndex = kGroupADKNotFoundWarningStrIndex;
				#endif

					PGPUIWarningAlert( kPGPUIGenericOKAlert,
						kPGPLibDialogsStringListResID,
						strIndex, pGroupName );
				}
			}
		
			if( haveADKs )
			{
				tableItem.movedManually 	= TRUE;
				tableItem.moveToOtherList	= TRUE;
			}

			mUserIDTable->SetCellData( cell, &tableItem );
		}
	}

	// Second pass. Move all of the marked and dependent rows.
	for( rowIndex = numRows; rowIndex >= 1; rowIndex-- )
	{
		PGPRecipientTableItem	tableItem;
		STableCell			cell(rowIndex, 1);
		
		mUserIDTable->GetCellData( cell, &tableItem );
		if( tableItem.moveToOtherList )
		{
			tableItem.moveToOtherList = FALSE;
			mUserIDTable->SetCellData( cell, &tableItem );
			
			mUserIDTable->RemoveRows( 1, rowIndex, FALSE );
			mRecipientTable->AddTableItem( &tableItem, FALSE );
			
			++numMovedItems;
		}
	}
	
	if( numMovedItems != 0 )
	{
		mRecipientTable->Refresh();
		mUserIDTable->Refresh();
	
		AdjustButtons();
	}
	
	return( numMovedItems );
}

	UInt32
CPGPRecipientGrafPort::SendSelectedToUserIDs(void)
{
	mRecipientTable->MarkSelected();
	
	return( SendMarkedToUserIDs() );
}


/*____________________________________________________________________________
	Mark ADK's in preparation for moving from the recipient list back to the
	user ID list.
____________________________________________________________________________*/
	void
CPGPRecipientGrafPort::MarkADKeysForRecipientToUserMove( PGPKeyRef	key )
{
	PGPError	err	= kPGPError_NoErr;
	UInt32 		adkIndex;
	PGPUInt32	numADKeys;
	
	pgpAssert( IsntNull( key ) );
	pgpAssert( IsntNull( mAllKeys ) );
	
	err = PGPCountAdditionalRecipientRequests( key, &numADKeys );
	pgpAssertNoErr( err );
	if ( IsPGPError( err ) )
		return;
		
	for( adkIndex = 0; adkIndex < numADKeys; adkIndex++ )
	{
		PGPKeyRef	adkey = NULL;
		
		err = PGPGetIndexedAdditionalRecipientRequest( key,
					mAllKeys, adkIndex, &adkey, nil );
		pgpAssert( (IsntPGPError( err ) && IsntNull( adkey) ) ||
			( IsPGPError( err ) && IsNull( adkey ) ) );
			
		if( IsntPGPError( err ) )
		{
			UInt32					adkeyRow;
			PGPRecipientTableItem	adkTableItem;
			
			if( mRecipientTable->HaveKey( adkey, &adkeyRow ) )
			{
				STableCell	adkCell(adkeyRow, 1);
			
				// Decrement the adkRefCount for this
				// adkey and move to the UserID table if
				// the refcount becomes zero.
			
				mRecipientTable->GetCellData( adkCell, &adkTableItem );
				pgpAssert( adkTableItem.user.adkRefCount > 0 );
				
				--adkTableItem.user.adkRefCount;
				
				if( adkTableItem.user.adkRefCount == 0 )
				{
					if( adkTableItem.movedManually )
					{
						// Unlock the key if it was
						// originally moved manually
						adkTableItem.locked 					= FALSE;
						adkTableItem.user.isEnforcedADKClass	= FALSE;
					}
					else
					{
						adkTableItem.moveToOtherList = TRUE;
						// Dont warn if the key is being moved
						// back automatically.
						adkTableItem.user.isEnforcedADKClass = FALSE;
					}
				}
				
				mRecipientTable->SetCellData( adkCell, &adkTableItem );
			}
			else if( mUserIDTable->HaveKey( adkey, &adkeyRow ) )
			{
				STableCell	adkCell(adkeyRow, 1);
				
				// Decrement the adkRefCount for this adkey

				mUserIDTable->GetCellData( adkCell, &adkTableItem );
				pgpAssert(adkTableItem.user.adkRefCount > 0 );
				--adkTableItem.user.adkRefCount;
				mUserIDTable->SetCellData( adkCell, &adkTableItem );
			}
		}
	}
}

/*____________________________________________________________________________
	Call MarkADKeysForRecipientToUserMove() for each key in the group.
____________________________________________________________________________*/
	void
CPGPRecipientGrafPort::MarkADKeysInGroupForRecipientToUserMove(
	PGPGroupSetRef	set,
	PGPGroupID		id )
{
	PGPError			err	= kPGPError_NoErr;
	PGPGroupItemIterRef	iter;
	
	err	= PGPNewGroupItemIter( set, id,
				kPGPGroupIterFlags_AllKeysRecursive, &iter );
	if ( IsntPGPError( err ) )
	{
		PGPGroupItem	item;
		PGPContextRef	context;
		
		context	= PGPGetKeySetContext( mAllKeys );
		
		while ( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
		{
			PGPKeyID	keyID;
			
			pgpAssert( item.type == kPGPGroupItem_KeyID );
			err	= PGPImportKeyID( item.u.key.exportedKeyID, &keyID );
			if ( IsntPGPError( err ) )
			{
				PGPKeyRef	key;
				
				err	= PGPGetKeyByKeyID( mAllKeys, &keyID,
						(PGPPublicKeyAlgorithm)item.u.key.algorithm, &key );
				if ( IsntPGPError( err ) )
				{
					MarkADKeysForRecipientToUserMove( key );
				}
			}
		}
		
		PGPFreeGroupItemIter( iter );
	}
}

	Boolean
CPGPRecipientGrafPort::MarkADKeysInGroupForUserToRecipientMove(
	PGPGroupSetRef	set,
	PGPGroupID		id )
{
	PGPError			err	= kPGPError_NoErr;
	PGPGroupItemIterRef	iter;
	Boolean				haveAllMandatoryADKs;
	Boolean				processGroup;

	haveAllMandatoryADKs 	= TRUE;
	processGroup			= TRUE;
			
	err	= PGPNewGroupItemIter( set, id,
				kPGPGroupIterFlags_AllKeysRecursive, &iter );
	if ( IsntPGPError( err ) )
	{
		PGPGroupItem	item;
		PGPContextRef	context;
		
		context	= PGPGetKeySetContext( mAllKeys );
		
		while ( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
		{
			PGPKeyID	keyID;
			
			err	= PGPImportKeyID( item.u.key.exportedKeyID, &keyID );
			if ( IsntPGPError( err ) )
			{
				PGPKeyRef	key;
				
				err	= PGPGetKeyByKeyID( mAllKeys, &keyID,
						(PGPPublicKeyAlgorithm)item.u.key.algorithm, &key );
				if ( IsntPGPError( err ) )
				{
					PGPUInt32	numADKeys;
					
					if( IsntPGPError( PGPCountAdditionalRecipientRequests( key,
							&numADKeys ) ) && numADKeys > 0 )
					{
						PGPUInt32	adkIndex;
						
						for( adkIndex = 0; adkIndex < numADKeys; adkIndex++ )
						{
							PGPKeyRef	adkey = nil;
							PGPByte		adkClass;

							err = PGPGetIndexedAdditionalRecipientRequest( key,
										mAllKeys, adkIndex, &adkey, &adkClass );
							if( IsntPGPError( err ) )
							{
								if( ( adkClass & 0x80 ) != 0 &&
									mEnforceRemoteADKClass )
								{
									PGPBoolean	keyCanEncrypt;
									
									/* Verify that the key is in our lists */
									
									err = PGPKeyCanEncrypt( adkey,
												&keyCanEncrypt );
									if( IsPGPError( err ) || ! keyCanEncrypt )
									{
										haveAllMandatoryADKs = FALSE;
									}
								}
							}
							else
							{
								PGPKeyID	adkKeyID;
								
								err =
								PGPGetIndexedAdditionalRecipientRequestKeyID(
										key, adkIndex, &adkKeyID, &adkClass );

								if( IsntPGPError( err ) )
								{
									if( ( adkClass & 0x80 ) != 0 &&
										mEnforceRemoteADKClass )
									{
										haveAllMandatoryADKs = FALSE;
									}
								}
								else
								{
									haveAllMandatoryADKs = FALSE;
								}
							}
							
							if( ! haveAllMandatoryADKs )
								break;
						}
					}
				}
			}
			
			if( ! haveAllMandatoryADKs )
				break;
		}
		
		PGPFreeGroupItemIter( iter );
	}

#if PGP_BUSINESS_SECURITY
	processGroup = haveAllMandatoryADKs;
#endif

	if( processGroup )
	{
		err	= PGPNewGroupItemIter( set, id,
					kPGPGroupIterFlags_AllKeysRecursive, &iter );
		if ( IsntPGPError( err ) )
		{
			PGPGroupItem	item;
			PGPContextRef	context;
			
			context	= PGPGetKeySetContext( mAllKeys );
			
			while ( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
			{
				PGPKeyID	keyID;
				
				pgpAssert( item.type == kPGPGroupItem_KeyID );
				err	= PGPImportKeyID( item.u.key.exportedKeyID, &keyID );
				if ( IsntPGPError( err ) )
				{
					PGPKeyRef	key;
					
					err	= PGPGetKeyByKeyID( mAllKeys, &keyID,
							(PGPPublicKeyAlgorithm)item.u.key.algorithm, &key );
					if ( IsntPGPError( err ) )
					{
						if( ! MarkADKeysForUserToRecipientMove( key ) )
							haveAllMandatoryADKs = FALSE;
					}
				}
			}
			
			PGPFreeGroupItemIter( iter );
		}
	}
	
	return( haveAllMandatoryADKs );
}

	UInt32
CPGPRecipientGrafPort::SendMarkedToUserIDs(void)
{
	TableIndexT		numRows;
	TableIndexT		numColumns;
	UInt32			numMovedItems;
	Boolean			showADKWarning;
	TableIndexT 	rowIndex;
	
	numMovedItems 	= 0;
	showADKWarning	= FALSE;
	
	mRecipientTable->GetTableSize( numRows, numColumns );

	// Need to iterate backwards because we're removing rows.
	for( rowIndex = numRows; rowIndex >= 1; rowIndex-- )
	{
		PGPRecipientTableItem	tableItem;
		STableCell			cell(rowIndex, 1);
		
		mRecipientTable->GetCellData( cell, &tableItem );
		if( tableItem.marked )
		{
			tableItem.moveToOtherList = TRUE;
			
			if( tableItem.itemKind == kTableItemKindKey )
			{
				PGPKeyRef	key;
				
				// Check for ADK dependencies.
				key	= tableItem.user.keyRef;
				if( tableItem.locked )
				{
					tableItem.moveToOtherList = FALSE;
				}
				else if( tableItem.movedManually )
				{
					if( tableItem.user.numADKeys != 0 )
					{
						MarkADKeysForRecipientToUserMove( key );
					}
				}
			}
			else if ( tableItem.itemKind == kTableItemKindGroup )
			{
				MarkADKeysInGroupForRecipientToUserMove( tableItem.group.set,
					tableItem.group.id );
			}
			
			mRecipientTable->SetCellData( cell, &tableItem );
		}
	}

	// Second pass. Move all of the marked and dependent rows.
	for( rowIndex = numRows; rowIndex >= 1; rowIndex-- )
	{
		PGPRecipientTableItem	tableItem;
		STableCell				cell(rowIndex, 1);
		
		mRecipientTable->GetCellData( cell, &tableItem );
		if( tableItem.moveToOtherList )
		{
			tableItem.moveToOtherList 	= FALSE;
			tableItem.movedManually		= FALSE;
			tableItem.marked 			= FALSE;
			
			if( TableItemKindIsKey( tableItem.itemKind ) )
			{
				if ( tableItem.user.isEnforcedADKClass )
				{
					// Item is a special ADK key that needs a warning if moved
					// back manually
					showADKWarning = TRUE;
				}
				tableItem.user.isEnforcedADKClass	= FALSE;
				tableItem.locked					= FALSE;
			}

			mRecipientTable->SetCellData( cell, &tableItem );
			
			mRecipientTable->RemoveRows( 1, rowIndex, FALSE );

			if( tableItem.itemKind == kTableItemKindKey ||
				tableItem.itemKind == kTableItemKindGroup )
			{
				mUserIDTable->AddTableItem( &tableItem, FALSE );
				++numMovedItems;
			}
		}
	}
	
	if( showADKWarning )
	{
		PGPUIWarningAlert( kWarnSpecialADKDialogID );
		FocusDraw();
		ApplyForeAndBackColors();
	}
	
	if( numMovedItems > 0 )
	{
		mUserIDTable->Refresh();
		mRecipientTable->Refresh();
	}
	
	AdjustButtons();
	
	return( numMovedItems );
}

	void
CPGPRecipientGrafPort::EnableUserIDLists(void)
{
	mUserIDListsEnabled = TRUE;

	FindPaneByID( kUserIDScrollerPaneID )->Enable();
	FindPaneByID( kRecipientScrollerPaneID )->Enable();

	SwitchTarget( mUserIDTable );
	
	AdjustButtons();
}

	void
CPGPRecipientGrafPort::DisableUserIDLists(void)
{
	mUserIDListsEnabled = FALSE;
	
	FindPaneByID( kUserIDScrollerPaneID )->Disable();
	FindPaneByID( kRecipientScrollerPaneID )->Disable();

	SwitchTarget( this );
	
	AdjustButtons();
}

	void
CPGPRecipientGrafPort::ListenToMessage(MessageT inMessage, void *ioParam)
{
	long	value;
	
	value = (long) ioParam;
	switch	(inMessage)
	{
		case 'Send':
		{
			TableIndexT	numRows;
			TableIndexT	numColumns;
			
			// Send the selected cells from the table object in ioParam to
			// the other table.
			
			if( (CUserIDTable *) ioParam == mUserIDTable )
			{
				SendSelectedToRecipients();
			}
			else if( (CUserIDTable *) ioParam == mRecipientTable ) 
			{
				SendSelectedToUserIDs();
			}
								
			mRecipientTable->GetTableSize( numRows, numColumns );
			
			break;
		}
		
		case 'Recv':
		{
			TableIndexT	numRows;
			TableIndexT	numColumns;
			
			// Send the selected cells to the table object in ioParam from
			// the other table.
			
			if( (CUserIDTable *)ioParam == mUserIDTable )
			{
				SendSelectedToUserIDs();
			}
			else if( (CUserIDTable *) ioParam == mRecipientTable )
			{
				SendSelectedToRecipients();
			}
								
			mRecipientTable->GetTableSize( numRows, numColumns );

			break;
		}
		
		case kFileConvEncryptCheckboxPaneID:
		case kTextConvEncryptCheckboxPaneID:
		{
			if( mConvEncryptCheckbox->GetValue() != 0 )
			{
				DisableUserIDLists();
			}
			else
			{
				EnableUserIDLists();
			}
		
			break;
		}
		
		case kUpdateFromServerButtonPaneID:
		{
			Boolean	dummy;
			
			UpdateMissingRecipients( &dummy );
			break;
		}

		default:
			CPGPLibGrafPortView::ListenToMessage( inMessage, ioParam );
			break;
	}
}

	CComboError
CPGPRecipientGrafPort::SetParams(
	PGPContextRef			context,
	PGPPrefRef				clientPrefsRef,
	PGPPrefRef				adminPrefsRef,
	PGPKeySetRef			allKeys,
	PGPUInt32				numDefaultRecipients,
	const PGPKeySpec		*defaultRecipients,
	PGPRecipientOptions		dialogOptions,	
	PGPRecipientSettings	defaultSettings,
	PGPGroupSetRef			groupSet,
	PGPBoolean				*allRecipientsValid)
{
	CComboError		err;
	PGPBoolean		showMarginalValidity;
	PGPBoolean		marginalIsInvalid;
	LPane			*hiddenOptionsPane;
	short			macBinaryMenuItem;
	
	mAllKeys				= allKeys;
	mNumDefaultRecipients	= numDefaultRecipients;
	mDefaultRecipients		= defaultRecipients;
	showMarginalValidity	= TRUE;
	mClientPrefsRef			= clientPrefsRef;
	mAdminPrefsRef			= adminPrefsRef;
	mContext				= context;
	mGroupSet 				= groupSet;
	
	(void) PGPGetPrefBoolean( clientPrefsRef, kPGPPrefDisplayMarginalValidity,
				&showMarginalValidity);
	(void) PGPGetPrefBoolean( clientPrefsRef, kPGPPrefMarginalIsInvalid,
				&marginalIsInvalid);
	
	mRecipientTable->SetPreferences( showMarginalValidity, marginalIsInvalid );
	mUserIDTable->SetPreferences( showMarginalValidity, marginalIsInvalid );
	
	if( ( dialogOptions & kPGPRecipientOptionsHideFileOptions ) != 0 )
	{
		mHaveFileOptions 	= FALSE;
		hiddenOptionsPane	= FindPaneByID( kFileOptionsBoxPaneID );

		mConvEncryptCheckbox = 
			(LGACheckbox *) FindPaneByID( kTextConvEncryptCheckboxPaneID );
	}
	else
	{
		mHaveFileOptions 	= TRUE;
		hiddenOptionsPane	= FindPaneByID( kTextOptionsBoxPaneID );

		mConvEncryptCheckbox = 
			(LGACheckbox *) FindPaneByID( kFileConvEncryptCheckboxPaneID );
	}
	
	mWipeOriginalCheckbox = 
		(LGACheckbox *) FindPaneByID( kWipeOriginalCheckboxPaneID );
	mTextOutputCheckbox = 
		(LGACheckbox *) FindPaneByID( kTextOutputCheckboxPaneID );
	mMacBinaryPopup =
		(LGAPopup *) FindPaneByID( kMacBinaryPopupPaneID );

	mWipeOriginalCheckbox->SetValue(
		( defaultSettings & kPGPRecipientSettingsWipeOriginal ) != 0 );
	mTextOutputCheckbox->SetValue(
		( defaultSettings & kPGPUISettingsTextOutput ) != 0 );

	/* always default MacBinary to on */
	macBinaryMenuItem = kMacBinaryPopupYesMenuItem;
	
	if ( ( defaultSettings & kPGPUISettingsNoMacBinary ) != 0 )
	{
		macBinaryMenuItem = kMacBinaryPopupNoneMenuItem;
	}
	else if ( ( defaultSettings & kPGPUISettingsSmartMacBinary ) != 0 )
	{
		macBinaryMenuItem = kMacBinaryPopupSmartMenuItem;
	}
	else if ( ( defaultSettings & kPGPUISettingsForceMacBinary ) != 0 )
	{
		macBinaryMenuItem = kMacBinaryPopupYesMenuItem;
	}
	else
	{
		PGPUInt32	macBinaryPref;
		
		if( IsntPGPError( PGPGetPrefNumber( mClientPrefsRef,
				kPGPPrefMacBinaryDefault, &macBinaryPref ) ) )
		{
			switch( macBinaryPref )
			{
				case kPGPPrefMacBinaryOff:
					macBinaryMenuItem = kMacBinaryPopupNoneMenuItem;
					break;
				
				case kPGPPrefMacBinarySmart:
					macBinaryMenuItem = kMacBinaryPopupSmartMenuItem;
					break;

				case kPGPPrefMacBinaryOn:
				default:
					macBinaryMenuItem = kMacBinaryPopupYesMenuItem;
					break;
			}
		}
	}
	
	mMacBinaryPopup->SetValue( macBinaryMenuItem );

#if PGP_BUSINESS_SECURITY	// [

	if( PGPPrefRefIsValid( mAdminPrefsRef ) )
	{
		PGPBoolean	allowConvEncryption;
		PGPBoolean	warnNotSignedByCorpKey;
		
		if( IsPGPError( PGPGetPrefBoolean( mAdminPrefsRef,
					kPGPPrefEnforceOutgoingADK, &mEnforceOutgoingADK) ) )
		{
			mEnforceOutgoingADK = TRUE;
		}

		if( IsPGPError( PGPGetPrefBoolean( mAdminPrefsRef,
					kPGPPrefEnforceRemoteADKClass, &mEnforceRemoteADKClass) ) )
		{
			mEnforceRemoteADKClass = TRUE;
		}
		
		if( IsPGPError( PGPGetPrefBoolean( mAdminPrefsRef,
					kPGPPrefWarnNotCertByCorp, &warnNotSignedByCorpKey) ) )
		{
			warnNotSignedByCorpKey = TRUE;
		}
	
		// Get the corporate key if we're supposed to verify that all
		// recipients have been signed with it.
		if( warnNotSignedByCorpKey )
		{
			PGPByte	*corpKeyID;
			PGPSize	corpKeyIDSize;
			
			err.pgpErr = PGPGetPrefData( mAdminPrefsRef, kPGPPrefCorpKeyID,
						&corpKeyIDSize, &corpKeyID );
			if( err.IsntError() )
			{
				PGPUInt32	corpKeyAlgorithm;
				
				err.pgpErr = PGPGetPrefNumber( mAdminPrefsRef,
							kPGPPrefCorpKeyPublicKeyAlgorithm,
							&corpKeyAlgorithm );
				if( err.IsntError() )
				{
					PGPKeyID	keyID;
					
					err.pgpErr = PGPImportKeyID( corpKeyID, &keyID );
					(void) PGPDisposePrefData( mAdminPrefsRef, corpKeyID );
					
					if( err.IsntError() )
					{
						err.pgpErr = PGPGetKeyByKeyID ( mAllKeys, &keyID,
							(PGPPublicKeyAlgorithm) corpKeyAlgorithm,
							&mCorporateKeyRef );
					}
				}
			}
			
			if( err.IsError() )
			{
				/*
				** Tell the user that we don't have the corporate key and
				** abort the dialog.
				*/
				
				PGPUIWarningAlert( kPGPUIGenericOKAlert,
						kPGPLibDialogsStringListResID,
						kCorporateSigningKeyMissingStrIndex );
				
				err.pgpErr = kPGPError_UserAbort;
			}
		}
		
		if( err.IsntError() )
		{
			PGPBoolean	haveOutgoingADK;
			
			err.pgpErr = PGPGetPrefBoolean( mAdminPrefsRef,
								kPGPPrefUseOutgoingADK,
								&haveOutgoingADK );
						
			if( err.IsntError() && haveOutgoingADK )
			{
				PGPByte	*adKeyID;
				PGPSize	adKeyIDSize;
				
				err.pgpErr = PGPGetPrefData( mAdminPrefsRef,
								kPGPPrefOutgoingADKID, &adKeyIDSize,
								&adKeyID );
				if( err.IsntError() )
				{
					PGPKeyID	keyID;
					PGPUInt32	outADKKeyAlgorithm;
					
					err.pgpErr = PGPGetPrefNumber( mAdminPrefsRef,
								kPGPPrefOutADKPublicKeyAlgorithm,
								&outADKKeyAlgorithm );
					if( err.IsntError() )
					{
						err.pgpErr = PGPImportKeyID( adKeyID, &keyID );
						
						if( err.IsntError() )
						{
							err.pgpErr = PGPGetKeyByKeyID ( mAllKeys,
								&keyID,
								(PGPPublicKeyAlgorithm) outADKKeyAlgorithm,
								&mOutgoingADKRef );
						}
					}
					
					(void) PGPDisposePrefData( mAdminPrefsRef, adKeyID);
				}
				
				if( err.IsError() )
				{
					if( mEnforceOutgoingADK )
					{
						/* Display an error dialog and return to the caller */
						PGPUIWarningAlert( kPGPUIGenericOKAlert,
								kPGPLibDialogsStringListResID,
								kCorporateADKDisabledOrMissingStrIndex );
						
						err.pgpErr = kPGPError_UserAbort;
					}
					else
					{
						err.pgpErr = kPGPError_NoErr;
					}
				}
			}
		}
		
		if( err.IsntError() )
		{
			if( IsPGPError( PGPGetPrefBoolean( mAdminPrefsRef,
						kPGPPrefAllowConventionalEncryption,
						&allowConvEncryption) ) )
			{
				allowConvEncryption = FALSE;
			}
			
			if( ! allowConvEncryption )
			{
				defaultSettings &= ~kPGPRecipientSettingsConvEncrypt;
				mConvEncryptCheckbox->Disable();
			}
		}
	}
#endif	// ] PGP_BUSINESS_SECURITY

	if( err.IsntError() )
	{
		mConvEncryptCheckbox->SetValue(
			( defaultSettings & kPGPRecipientSettingsConvEncrypt ) != 0 );

		mConvEncryptCheckbox->AddListener( this );
		
		pgpAssertAddrValid( hiddenOptionsPane, LPane );
		hiddenOptionsPane->Hide();
		
		ApplyForeAndBackColors();
		
		err.pgpErr = BuildRecipientList();
		if( err.IsntError() )
		{
			err.pgpErr = MoveDefaultUserIDs( allRecipientsValid );
		}
	}
	
	return( err );
}

	PGPError
CPGPRecipientGrafPort::GetAllKeysInRecipientList(
	PGPKeySetRef *outSet )
{
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outSet );
	*outSet	= NULL;
	
	pgpAssert( mUserIDListsEnabled );
	if( ! mUserIDListsEnabled )
		return( kPGPError_ImproperInitialization );
	
	err = mRecipientTable->CreateKeySet( mAllKeys, FALSE, outSet );
	
	return( err );
}


	PGPError
CPGPRecipientGrafPort::GetParams(
	PGPRecipientSettings	*userSettings,
	PGPKeySetRef			*actualRecipients,
	PGPBoolean				*haveNewKeys)
{
	PGPError		err;
	UInt32			macBinarySetting;
	
	err				= kPGPError_NoErr;
	*userSettings 	= 0;
	*haveNewKeys	= mHaveNewKeys;
	
	if( mConvEncryptCheckbox->GetValue() == 1 )
		*userSettings |= kPGPRecipientSettingsConvEncrypt;

	if( mTextOutputCheckbox->GetValue() == 1 )
		*userSettings |= kPGPUISettingsTextOutput;

	if( mWipeOriginalCheckbox->GetValue() == 1 )
		*userSettings |= kPGPRecipientSettingsWipeOriginal;

	macBinarySetting = mMacBinaryPopup->GetValue();
	switch( macBinarySetting )
	{
		case kMacBinaryPopupNoneMenuItem:
			*userSettings |= kPGPUISettingsNoMacBinary;
			break;

		case kMacBinaryPopupSmartMenuItem:
			*userSettings |= kPGPUISettingsSmartMacBinary;
			break;

		case kMacBinaryPopupYesMenuItem:
			*userSettings |= kPGPUISettingsForceMacBinary;
			break;
	}	
	

	if ( IsntPGPError( err ) && mUserIDListsEnabled)
	{
		err	= GetAllKeysInRecipientList( actualRecipients );
	}
	
	return( err );
}

	Boolean
CPGPRecipientGrafPort::HandleKeyPress(const EventRecord& inKeyEvent)
{
	Boolean		keyHandled = TRUE;
	Int16		theKey = inKeyEvent.message & charCodeMask;
	
	if( ( inKeyEvent.modifiers & cmdKey ) != 0 )
	{
		// Handle cmd-E and cmd-S to toggle checkboxes
		// and Cmd-. to abort the dialog.
		switch(theKey)
		{
			case 'W':
			case 'w':
			{
				if( mWipeOriginalCheckbox->IsEnabled() )
				{
					mWipeOriginalCheckbox->SetValue(
							!mWipeOriginalCheckbox->GetValue());
				}
				
				break;
			}

			case 'E':
			case 'e':
			{
				if( mConvEncryptCheckbox->IsEnabled() )
				{
					mConvEncryptCheckbox->SetValue(
							!mConvEncryptCheckbox->GetValue());
				}
				
				break;
			}

			case 'T':
			case 't':
			{
				if( mTextOutputCheckbox->IsEnabled() )
				{
					mTextOutputCheckbox->SetValue(
							!mTextOutputCheckbox->GetValue());
				}
				
				break;
			}
			
			default:
				keyHandled = CPGPLibGrafPortView::HandleKeyPress( inKeyEvent );
				break;
		}
	}
	else
	{
		keyHandled = CPGPLibGrafPortView::HandleKeyPress( inKeyEvent );
	}
	
	return keyHandled;
}

	void
CPGPRecipientGrafPort::AdjustButtons(void)
{
	PGPUInt32	numUsers;
	PGPUInt32	numGroups;
	PGPUInt32	numIncompleteGroups;
	PGPUInt32	numMissingUsers;
	PGPUInt32	numMissingGroups;
	
	mRecipientTable->ClassifyItems( &numUsers, &numGroups,
				&numIncompleteGroups, &numMissingUsers, &numMissingGroups,
				NULL, NULL );
	
	if( mUserIDListsEnabled )
	{
		Boolean	enable = FALSE;
		
		if( numUsers  > 0 )
		{
			enable = TRUE;
		}
		else if( numGroups > 0 )
		{
			if( numGroups != numMissingGroups )
			{
				enable = TRUE;
			}
		}
		
		if( enable )
		{
			mOKButton->Enable();
		}
		else
		{
			mOKButton->Disable();
		}
	}
	else
	{
		mOKButton->Enable();
	}
	
	if( mUpdateFromServerButton->IsVisible() )
	{
		if( mUserIDListsEnabled &&
			( numMissingUsers > 0 ||
			  numIncompleteGroups > 0 ) )
		{
			mUpdateFromServerButton->Enable();
		}
		else
		{
			mUpdateFromServerButton->Disable();
		}
	}
	
	// Stupid PP bugs
	ApplyForeAndBackColors();
}


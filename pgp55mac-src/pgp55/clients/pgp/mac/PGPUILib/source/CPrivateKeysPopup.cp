/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPrivateKeysPopup.cp,v 1.13 1997/10/21 00:27:40 lloyd Exp $
____________________________________________________________________________*/

#include <LArrayIterator.h>
#include <LComparator.h>

#include "MacStrings.h"

#include "CPrivateKeysPopup.h"
#include "PGPUILibDialogs.h"
#include "PGPUILibUtils.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpMem.h"
#include "pgpUserInterface.h"

typedef struct PopupKeyItem
{
	PGPKeyRef	keyRef;
	Str255		userID;		// Pascal string
	UInt16		algorithm;
	UInt16		keyBits;
	Boolean		isDefaultKey;

} PopupKeyItem; 

class CComparator : public LComparator
{
	virtual			~CComparator(void);
	virtual Int32	Compare(const void *inItemOne, const void *inItemTwo,
								Uint32 inSizeOne, Uint32 inSizeTwo) const;
};

CComparator::~CComparator(void)
{
}

	Int32
CComparator::Compare(
	const void*		inItemOne,
	const void*		inItemTwo,
	Uint32			inSizeOne,
	Uint32			inSizeTwo) const
{
	Int32				result;
	const PopupKeyItem	*keyItemOne = (PopupKeyItem *) inItemOne;
	const PopupKeyItem	*keyItemTwo = (PopupKeyItem *) inItemTwo;
	
	pgpAssertAddrValid( keyItemOne, PopupKeyItem );
	pgpAssertAddrValid( keyItemTwo, PopupKeyItem );
	
#if PGP_DEBUG
	pgpAssert( inSizeOne == sizeof( PopupKeyItem ) );
	pgpAssert( inSizeTwo == sizeof( PopupKeyItem ) );
#else
	#pragma unused( inSizeOne, inSizeTwo )
#endif
	
	result = CompareText( &keyItemOne->userID[1], &keyItemTwo->userID[1],
					keyItemOne->userID[0], keyItemTwo->userID[0], nil );

	return( result );
}

CPrivateKeysPopup::CPrivateKeysPopup(LStream *inStream)
	: CGAPopupFix( inStream )
{
	mKeyItemList = new( LArray )( sizeof(PopupKeyItem),
						new( CComparator ), TRUE );
}

CPrivateKeysPopup::~CPrivateKeysPopup()
{
	delete( mKeyItemList );
}

	Boolean
CPrivateKeysPopup::BuildKeyList(PGPKeySetRef allKeys)
{
	PGPError		err;
	MenuHandle		menuH;
	PGPKeyListRef	keyList;
	Boolean			haveSecretKeys;
	
	haveSecretKeys = FALSE;
	
	// Find and add the private keys.
	
	err = PGPOrderKeySet( allKeys, kPGPAnyOrdering, &keyList );
	if( IsntPGPError( err ) )
	{
		PGPKeyIterRef	keyIterator;
		
		err = PGPNewKeyIter( keyList, &keyIterator );
		if( IsntPGPError( err ) )
		{
			PGPKeyRef	theKey;
			PGPKeyRef	defaultPrivateKey;
			
			if( IsPGPError( PGPGetDefaultPrivateKey( allKeys,
						&defaultPrivateKey ) ) )
			{
				defaultPrivateKey = (PGPKeyRef) kInvalidPGPKeyRef;
			}
			
			err = PGPKeyIterNext( keyIterator, &theKey );
			while( IsntPGPError( err ) )
			{
				PGPBoolean	keyCanSign;
				
				err = PGPKeyCanSign( theKey, &keyCanSign );
				if( IsntPGPError( err ) )
				{
					if( keyCanSign )
					{
						PopupKeyItem	keyItem;
						PGPInt32		tempInt32;
						
						keyItem.keyRef			= theKey;
						keyItem.isDefaultKey = (theKey == defaultPrivateKey );
						
						err = PGPGetKeyNumber( theKey,
									kPGPKeyPropAlgID, &tempInt32 );
						if( IsntPGPError( err ) )
						{
							keyItem.algorithm = tempInt32;

							err = PGPGetKeyNumber( theKey,
										kPGPKeyPropBits, &tempInt32 );
							if( IsntPGPError( err ) )
							{
								PGPSize	nameSize;
								
								keyItem.keyBits = tempInt32;
								
								err = PGPGetPrimaryUserIDNameBuffer(
											theKey,
											sizeof( keyItem.userID ) - 1,
											(char *)&keyItem.userID[1],
											&nameSize );
								if( IsntPGPError( err ) )
								{
									/* nameSize includes the 0 terminator */
									keyItem.userID[0] = nameSize - 1;
								}
							}
						}
						
						// Add the key info to our array
						if( IsntPGPError( err ) )
						{
							mKeyItemList->InsertItemsAt( 1, 10000, &keyItem );
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

	// Walk our sorted array and setup the popup menu handle
	
	menuH = GetMacMenuH();
	if( IsntNull( menuH ) )
	{
		Str255	menuItemStr;
		UInt32	value;
	
		value = 1;
		
		haveSecretKeys = ( mKeyItemList->GetCount() > 0 );
		if( haveSecretKeys )
		{
			LArrayIterator	iterator( *mKeyItemList );
			PopupKeyItem	keyItem;
			short			menuItemIndex;
			
			menuItemIndex = 1;
			
			FocusDraw();
			
			while( iterator.Next( &keyItem ) )
			{
				Str255	tempStr;
				short	strIndex;
				
				GetIndString( menuItemStr, kPGPLibDialogsStringListResID,
							kPrivateKeyPopupFormatStrIndex );
				CopyPString( keyItem.userID, tempStr );
				
				/* Truncate the string so we can add our key bits info */
				if( tempStr[0] > 225 )
					tempStr[0] = 225;
					
				PrintPString( menuItemStr, menuItemStr, tempStr );
				
				switch( keyItem.algorithm )
				{
					case kPGPPublicKeyAlgorithm_RSA:
						strIndex = kRSAAlgorithmStringIndex;
						break;
						
					case kPGPPublicKeyAlgorithm_ElGamal:
					case kPGPPublicKeyAlgorithm_DSA:
						strIndex = kDSAAlgorithmStringIndex;
						break;
						
					default:
						strIndex = kUnknownAlgorithmStringIndex;
						break;
				}
				
				GetIndString( tempStr, kPGPLibDialogsStringListResID,
							strIndex );
				PrintPString( menuItemStr, menuItemStr, tempStr );
				
				NumToString( keyItem.keyBits, tempStr );
				PrintPString( menuItemStr, menuItemStr, tempStr );
				
				#define kMaxPopupTextWidth	400
				
				if( StringWidth( menuItemStr ) > kMaxPopupTextWidth )
				{
					TruncString( kMaxPopupTextWidth, menuItemStr,
								smTruncMiddle );
				}
				
				AppendMenu( menuH, "\pxx" );
				SetMenuItemText( menuH, menuItemIndex, menuItemStr );
				
				if( keyItem.isDefaultKey )
				{
					value = menuItemIndex;
					SetItemStyle( menuH, menuItemIndex, bold );
				}
				
				++menuItemIndex;
			}
		}
		else
		{
			// Set the popup to the "no signing keys" found and disable
			// ourselves
			GetIndString( menuItemStr, kPGPLibDialogsStringListResID,
						kNoSigningKeysFoundStrIndex );
			AppendMenu( menuH, menuItemStr );
		}
		
		// Recalculate the popup dimensions
		SetPopupMinMaxValues ();

		// If the popup has been setup to have a variable width and
		// the popup is not being dispalyed as only the arrow portion,then
		// we also need to make sure that we get the width adjusted
		if ( !mFixedWidth && !IsArrowOnly ())
			AdjustPopupWidth ();

		SetValue( value );
		
		if( ! haveSecretKeys )
			Disable();
	}
	
	return( haveSecretKeys );
}

	void
CPrivateKeysPopup::GetCurrentItemTitle(Str255 outItemTitle)
{
	if( mKeyItemList->GetCount() > 0 )
	{
		PopupKeyItem	keyItem;
		
		mKeyItemList->FetchItemAt( GetValue(), &keyItem );
		CopyPString( keyItem.userID, outItemTitle );
	}
	else
	{
		inherited::GetCurrentItemTitle( outItemTitle );
	}
}

	PGPKeyRef
CPrivateKeysPopup::GetSigningKey(void)
{
	PGPKeyRef	signingKey;
	
	if( mKeyItemList->GetCount() > 0 )
	{
		PopupKeyItem	keyItem;
		
		mKeyItemList->FetchItemAt( GetValue(), &keyItem );

		signingKey = keyItem.keyRef;
	}
	else
	{
		pgpDebugMsg( "GetSigningKey(): No signing keys!" );
		
		signingKey = kInvalidPGPKeyRef;
	}
	
	return( signingKey );
}

	void
CPrivateKeysPopup::SetSigningKey(PGPKeyRef signingKey)
{
	if( mKeyItemList->GetCount() > 0 )
	{	
		LArrayIterator	iterator( *mKeyItemList );
		PopupKeyItem	keyItem;
		Int32			newValue;
		Int32			itemIndex;
		
		newValue 	= 0;
		itemIndex	= 1;

		// Walk the items and try to match the deault key spec. Note
		// that the user ID must be a fully expanded user ID.
		
		while( iterator.Next( &keyItem ) )
		{
			if( keyItem.keyRef == signingKey )
			{
				newValue = itemIndex;
				break;
			}
			
			++itemIndex;
		}

		if( newValue != 0 )
		{
			RGBColor	saveForeColor;
			RGBColor	saveBackColor;
			
			// Stupid PowerPlant class will whack the fore and back colors of
			// our dialog when we the the popup value here. Save and restore
			// them.
			
			GetForeColor( &saveForeColor );
			GetBackColor( &saveBackColor );
			
			SetValue( newValue );

			RGBForeColor( &saveForeColor );
			RGBBackColor( &saveBackColor );
		}
	}
}




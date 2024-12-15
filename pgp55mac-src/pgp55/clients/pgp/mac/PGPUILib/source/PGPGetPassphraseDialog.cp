/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: PGPGetPassphraseDialog.cp,v 1.32 1997/10/21 00:27:40 lloyd Exp $
____________________________________________________________________________*/

#include <string.h>

#include <PP_Messages.h>
#include <URegistrar.h>

#include "CSecureMemory.h"
#include "MacDebug.h"
#include "MacResources.h"
#include "MacStrings.h"

#include "CPGPGetPassphraseGrafPort.h"
#include "CPGPGetConvPassphraseGrafPrt.h"
#include "pgpUserInterface.h"
#include "PGPUILibUtils.h"
#include "PGPUILibDialogs.h"
#include "WarningAlert.h"
#include "pgpKeys.h"
#include "pgpMem.h"
#include "pgpUtilities.h"
#include "pgpErrors.h"

const ResIDT kPGPGetPassphraseDialogID					= 4748;
const ResIDT kPGPGetDecryptionPassphraseDialogID		= 4749;
const ResIDT kPGPGetSigningPassphraseDialogID			= 4750;
const ResIDT kPGPGetSigningPassphraseOptionsDialogID	= 4751;
const ResIDT kIncorrectPassphraseDialogID				= 4754;
const ResIDT kPGPGetConvPassphraseDialogID				= 4758;


static PGPKeyRef	FindKeyForPassphrase( PGPKeySetRef keySet,
						const char * passphrase,
						Boolean signing );
static PGPBoolean	ConfirmMatchingKeyOK( PGPKeyRef	matchingKey );


	
	static PGPError
RealPGPGetPassphraseDialog(
	const char 	*	prompt,
	PGPKeySetRef	recipientSet,
	PGPUInt32		numMissingRecipients,
	PGPKeyRef		preferredKey,
	char			passphrase[256],
	PGPKeyRef *		outKey )
{
	PGPError					err = kPGPError_NoErr;
	ResID						dialogID;
	CPGPGetPassphraseGrafPort	*passphraseGrafPortView;
	PGPBoolean					haveValidKey = FALSE;
	PGPBoolean					haveValidSet = FALSE;
	
	pgpClearMemory( passphrase, 256 );
	*outKey	= kInvalidPGPKeyRef;
	
	InitializePowerPlant();
	
	haveValidSet = PGPKeySetRefIsValid( recipientSet );
	if( haveValidSet )
	{
		PGPUInt32	numKeys;
		
		if( IsPGPError( PGPCountKeys( recipientSet, &numKeys ) ) ||
			numKeys == 0 )
		{
			haveValidSet = FALSE;
		}
	}
	
	if( haveValidSet )
	{
		dialogID = kPGPGetDecryptionPassphraseDialogID;
		
		if( ! PGPRefIsValid( preferredKey ) )
		{
			// If the caller does not pass a preferred key, then
			// use the first secret key found in the recipients list.
			
			PGPKeyListRef	keyListRef;
			Boolean			foundValidKey	= FALSE;
			
			err = PGPOrderKeySet( recipientSet, kPGPAnyOrdering, &keyListRef );
			if( IsntPGPError( err ) )
			{
				PGPKeyIterRef	keyIterator;
			
				err = PGPNewKeyIter( keyListRef, &keyIterator );
				if( IsntPGPError( err ) )
				{
					err = PGPKeyIterNext( keyIterator, &preferredKey );
					while( IsntErr( err ) )
					{
						PGPBoolean	keyCanDecrypt;
						
						err = PGPKeyCanDecrypt( preferredKey,
									&keyCanDecrypt );
						if( IsntPGPError( err ) && keyCanDecrypt )
						{
							break;
						}
						
						err = PGPKeyIterNext( keyIterator, &preferredKey );
					}
					
					PGPFreeKeyIter( keyIterator );
				}
				
				PGPFreeKeyList( keyListRef );
			}
			
			if( IsPGPError( err ) )
			{
				// Don't care if we got an error here
				err 			= kPGPError_NoErr;
				preferredKey	= kInvalidPGPKeyRef;
			}
		}

		haveValidKey = PGPKeyRefIsValid( preferredKey );
	}
	else
	{
		dialogID = kPGPGetPassphraseDialogID;
	}
	
	// Create standard Macintosh window and overlay PowerPlant onto it
	passphraseGrafPortView = (CPGPGetPassphraseGrafPort *)
								CPGPLibGrafPortView::CreateDialog( dialogID );
	if( IsntNull( passphraseGrafPortView ) )
	{
		DialogRef		getPassphraseDialog;
		ModalFilterUPP	filterUPP;
		MessageT		dismissMessage;
		short			itemHit;

		getPassphraseDialog = passphraseGrafPortView->GetDialog();
		
		if( IsntNull( prompt ) )
		{
			Str255	pPrompt;
			
			CToPString( prompt, pPrompt );
			passphraseGrafPortView->SetPrompt( pPrompt );
		}
		
		if( haveValidKey )
		{
			passphraseGrafPortView->SetTargetKey( preferredKey );
		}
		
		if( haveValidSet )
		{
			passphraseGrafPortView->SetRecipients( recipientSet,
						numMissingRecipients );
		}
		
		InitCursor();
		ShowWindow( getPassphraseDialog );
		SelectWindow( getPassphraseDialog );

		filterUPP = NewModalFilterProc( CPGPLibGrafPortView::ModalFilterProc );

		passphraseGrafPortView->SetDismissMessage( msg_Nothing );

		do
		{
			itemHit = 0;
			while( itemHit == 0 )
			{
				ModalDialog( filterUPP, &itemHit );
			}
		
			dismissMessage = passphraseGrafPortView->GetDismissMessage();
			if( dismissMessage == msg_OK )
			{
				passphraseGrafPortView->GetPassphrase( passphrase );
				
				if ( (! haveValidKey) && (! haveValidSet) )
				{
					/* we can't confirm whether it's good,
					so just return the passphrase */
					break;
				}
				
				if ( haveValidKey &&
						PGPPassphraseIsValid( preferredKey, passphrase ))
				{
					*outKey	= preferredKey;
					break;
				}
				
				if ( haveValidSet )
				{
					PGPKeyRef	key;
					
					key	= FindKeyForPassphrase( recipientSet, passphrase,
								FALSE );
					if ( IsntNull( key ) )
					{
						*outKey	= key;
						break;
					}
				}
				
				passphraseGrafPortView->SetDismissMessage( msg_Nothing );
				PGPUIWarningAlert(kIncorrectPassphraseDialogID);
				passphraseGrafPortView->ResetTarget();
			}
			else
			{
				err		= kPGPError_UserAbort;
				break;
			}
			
		} while ( TRUE );
		
		DisposeRoutineDescriptor( filterUPP );
		
		delete passphraseGrafPortView;
		DisposeDialog( getPassphraseDialog );
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}

	return ( err );
}

	static PGPError
RealPGPGetDecryptionPassphraseDialog(
	PGPContextRef	context,
	const char		*prompt,
	PGPKeySetRef	recipientSet,
	PGPUInt32		numMissingRecipients,
	PGPKeyRef		preferredKey,	/* Can be NULL */
	char			passphrase[256],
	PGPKeyRef *		outKey )
{
	PGPUILibState	state;
	CComboError		err;
	
	err = EnterPGPUILib( context, &state );
	if( err.IsntError() )
	{
		err.pgpErr = RealPGPGetPassphraseDialog( prompt,
					recipientSet, numMissingRecipients,
					preferredKey, passphrase, outKey);
	}
		
	ExitPGPUILib( &state );
	
	return( err.ConvertToPGPError() );
}

	PGPError
PGPGetDecryptionPassphraseDialog(
	PGPContextRef	context,
	const char		*prompt,
	PGPKeySetRef	recipientSet,
	PGPUInt32		numMissingRecipients,
	PGPKeyRef		preferredKey,	/* Can be NULL */
	char			passphrase[256],
	PGPKeyRef *		outKey )
{
	PGPError	err;
	
	PGPValidatePtr( outKey );
	*outKey	= kInvalidPGPKeyRef;
	PGPValidatePtr( context );
	PGPValidateParam( PGPKeySetRefIsValid( recipientSet ) );
	PGPValidatePtr( passphrase );
	pgpClearMemory( passphrase, sizeof( passphrase ) );
	
	err = RealPGPGetDecryptionPassphraseDialog( context,
					prompt, recipientSet,
					numMissingRecipients,
					preferredKey, passphrase, outKey);
	
	return( err );
}

	PGPError
PGPGetConventionalDecryptionPassphraseDialog(
	PGPContextRef	context,
	const char		*prompt,
	char			passphrase[256])
{
	PGPKeyRef	outKey;
	
	return( RealPGPGetDecryptionPassphraseDialog( context, prompt,
				NULL, 0, kInvalidPGPKeyRef, passphrase, &outKey ) );
}

	static PGPError
RealPGPGetConventionalEncryptionPassphraseDialog(
	const char		*prompt,
	char			passphrase[256])
{
	PGPError						err = kPGPError_NoErr;
	CPGPGetConvPassphraseGrafPort	*passphraseGrafPortView;
	
	pgpClearMemory( passphrase, 256 );
	
	InitializePowerPlant();
	
	// Create standard Macintosh window and overlay PowerPlant onto it
	passphraseGrafPortView = (CPGPGetConvPassphraseGrafPort *)
		CPGPLibGrafPortView::CreateDialog(kPGPGetConvPassphraseDialogID);
	if( IsntNull( passphraseGrafPortView ) )
	{
		ModalFilterUPP	filterUPP;
		MessageT		dismissMessage;
		short			itemHit;
		DialogRef		getPassphraseDialog;

		getPassphraseDialog = passphraseGrafPortView->GetDialog();
		
		if( IsntNull( prompt ) )
		{
			Str255	pPrompt;
			
			CToPString( prompt, pPrompt );
			passphraseGrafPortView->SetPrompt( pPrompt );
		}
		
		InitCursor();
		ShowWindow( getPassphraseDialog );
		SelectWindow( getPassphraseDialog );

		filterUPP = NewModalFilterProc( CPGPLibGrafPortView::ModalFilterProc );

		passphraseGrafPortView->SetDismissMessage( msg_Nothing );

		do
		{
			itemHit = 0;
			while( itemHit == 0 )
			{
				ModalDialog( filterUPP, &itemHit );
			}
		
			dismissMessage = passphraseGrafPortView->GetDismissMessage();
			if( dismissMessage == msg_OK )
			{
				CSecureCString256	testPassphrase;
				CSecureCString256	testConfirmation;
				
				passphraseGrafPortView->GetPassphrase( testPassphrase,
							testConfirmation );
				if( strcmp( testPassphrase, testConfirmation ) == 0 )
				{
					CopyCString( testPassphrase, passphrase );
					break;
				}
				else
				{
					PGPUIWarningAlert( kPGPUIGenericOKAlert,
								kPGPLibDialogsStringListResID,
								kConvEncryptionPassphraseNotConfirmedStrIndex );
								
					passphraseGrafPortView->SetDismissMessage( msg_Nothing );
					passphraseGrafPortView->ClearPassphrase();
				}
			}
			else
			{
				err = kPGPError_UserAbort;
				break;
			}
			
		} while ( TRUE );
		
		DisposeRoutineDescriptor( filterUPP );
		
		delete passphraseGrafPortView;
		DisposeDialog( getPassphraseDialog );
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}

	return ( err );
}

	PGPError
PGPGetConventionalEncryptionPassphraseDialog(
	PGPContextRef	context,
	const char		*prompt,
	char			passphrase[256])
{
	PGPUILibState	state;
	CComboError		err;
	
	PGPValidatePtr( context );
	PGPValidatePtr( passphrase );
	pgpClearMemory( passphrase, sizeof( passphrase ) );
	
	err = EnterPGPUILib( context, &state );
	if( err.IsntError() )
	{
		err.pgpErr = RealPGPGetConventionalEncryptionPassphraseDialog( prompt,
					passphrase);
	}
		
	ExitPGPUILib( &state );
	
	return( err.ConvertToPGPError() );
}


/*____________________________________________________________________________
	Caller must call PGPFreeKey() on *outKey
____________________________________________________________________________*/
	static PGPKeyRef
FindKeyForPassphrase(
	PGPKeySetRef	keySet,
	const char *	passphrase,
	Boolean			signing )
{
	PGPKeyRef		theKey	= kInvalidPGPKeyRef;
	PGPError		err;
	PGPKeyListRef	keyListRef;
	Boolean			foundValidKey	= FALSE;
	
	err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keyListRef );
	if( IsntPGPError( err ) )
	{
		PGPKeyIterRef	keyIterator;
	
		err = PGPNewKeyIter( keyListRef, &keyIterator );
		if( IsntPGPError( err ) )
		{
			err = PGPKeyIterNext( keyIterator, &theKey );
			while( IsntErr( err ) )
			{
				Boolean	tryKey = FALSE;
				
				if( signing )
				{
					PGPBoolean	keyCanSign;
					
					if( IsntPGPError( PGPKeyCanSign( theKey,
							&keyCanSign ) ) && keyCanSign )
					{
						tryKey = TRUE;
					}
				}
				else
				{
					PGPBoolean	keyCanDecrypt;
					
					if( IsntPGPError( PGPKeyCanDecrypt( theKey,
							&keyCanDecrypt ) ) && keyCanDecrypt )
					{
						tryKey = TRUE;
					}
				}
				
				if ( tryKey && PGPPassphraseIsValid( theKey, passphrase ) )
				{
					foundValidKey	= TRUE;
					break;
				}
				
				err = PGPKeyIterNext( keyIterator, &theKey );
			}
			PGPFreeKeyIter( keyIterator );
		}
		PGPFreeKeyList( keyListRef );
	}

	return( foundValidKey ? theKey : NULL );
}


/* return TRUE if it's OK to use the key */
	static PGPBoolean
ConfirmMatchingKeyOK( PGPKeyRef	matchingKey )
{
	PGPUserIDRef	userID;
	Str255			pName;
	char			cName[ sizeof(pName) ];
	PGPBoolean		okToUse	= FALSE;
	PGPError		err	= kPGPError_NoErr;

	err	= PGPGetPrimaryUserID( matchingKey, &userID );
	if ( IsntPGPError( err ) )
	{
		err	= PGPGetUserIDStringBuffer( userID,
			kPGPUserIDPropName, sizeof( cName ),
			cName, NULL);
	}

	if ( IsntPGPError( err ) )
	{
		MessageT		result;
		
		CToPString( cName, pName );
		result	= PGPUIWarningAlert( kPGPUIGenericOKCancelAlert,
					kPGPLibDialogsStringListResID,
					kConfirmMatchingKeyStrIndex, pName );
				
		if ( result == msg_OK )
		{
			okToUse	= TRUE;
		}
	}
	return( okToUse );
}


/*____________________________________________________________________________
	Caller must call PGPFreeKey() on *actualSigningKey
____________________________________________________________________________*/
	static PGPError
RealPGPGetSigningPassphraseDialog(
	PGPPrefRef					clientPrefsRef,
	PGPKeySetRef				allKeys,
	const char 					*prompt,
	PGPGetPassphraseOptions		dialogOptions,
	PGPGetPassphraseSettings	defaultSettings,
	PGPKeyRef					defaultSigningKey,
	char						passphrase[256],
	PGPGetPassphraseSettings	*userSettings,
	PGPKeyRef 					*actualSigningKey)
{
	PGPError					err = kPGPError_NoErr;
	ResID						dialogID;
	CPGPGetPassphraseGrafPort	*passphraseGrafPortView;
	
	pgpAssert( IsntNull( actualSigningKey ) );
	
	*passphrase	= NULL;
	
	InitializePowerPlant();
	
	if( IsNull( userSettings ) ||
		( dialogOptions & kPGPGetPassphraseOptionsHideFileOptions ) != 0 )
	{
		// Hide the options pane if the caller cannot receive settings
		dialogID = kPGPGetSigningPassphraseDialogID;
	}
	else
	{
		dialogID = kPGPGetSigningPassphraseOptionsDialogID;
	}
	
	// Create standard Macintosh window and overlay PowerPlant onto it
	passphraseGrafPortView = (CPGPGetPassphraseGrafPort *)
								CPGPLibGrafPortView::CreateDialog( dialogID);
	if( IsntNull( passphraseGrafPortView ) )
	{
		DialogRef	getPassphraseDialog;

		getPassphraseDialog = passphraseGrafPortView->GetDialog();
	
		passphraseGrafPortView->SetSettings( clientPrefsRef, defaultSettings,
					dialogOptions );

		if( IsntNull( prompt ) )
		{
			Str255	pPrompt;
			
			CToPString( prompt, pPrompt );
			passphraseGrafPortView->SetPrompt( pPrompt );
		}
		
		if( IsntNull( actualSigningKey ) )
		{
			if( ! passphraseGrafPortView->BuildKeyList( allKeys ) )
				err = kPGPError_SecretKeyNotFound;
		}
		
		if( IsntPGPError( err ) )
		{
			ModalFilterUPP	filterUPP;
			MessageT		dismissMessage;
			short			itemHit;
		
			if( IsntNull( defaultSigningKey ) )
			{
				passphraseGrafPortView->SetTargetKey( defaultSigningKey );
			}
			
			InitCursor();
			ShowWindow( getPassphraseDialog );
			SelectWindow( getPassphraseDialog );

			filterUPP =
				NewModalFilterProc( CPGPLibGrafPortView::ModalFilterProc );

			passphraseGrafPortView->SetDismissMessage( msg_Nothing );

			Boolean	done	= FALSE;
			do
			{
				itemHit = 0;
				while( itemHit == 0 )
				{
					ModalDialog( filterUPP, &itemHit );
				}
				
				dismissMessage = passphraseGrafPortView->GetDismissMessage();
				if( dismissMessage == msg_OK )
				{
					PGPKeyRef	signingKey;
							
					passphraseGrafPortView->GetPassphrase( passphrase );
					
					signingKey = passphraseGrafPortView->GetTargetKey();
					
					if ( PGPPassphraseIsValid( signingKey, passphrase ) )
					{
						done	= TRUE;
					}
					else
					{
						signingKey	= FindKeyForPassphrase( allKeys,
										passphrase, TRUE );
						if ( IsntNull( signingKey ) )
						{
							if ( ConfirmMatchingKeyOK( signingKey ) ) 
								done	= TRUE;
						}
						else
						{
							PGPUIWarningAlert( kIncorrectPassphraseDialogID );
						}
						
						if ( ! done )
						{
							passphraseGrafPortView->ResetTarget();
							passphraseGrafPortView->SetDismissMessage(
										msg_Nothing );
						}
					}
					
					if ( done )
					{
						*actualSigningKey	= signingKey;
					}
				}
				else
				{
					err		= kPGPError_UserAbort;
					done	= TRUE;
				}
			} while ( ! done );
			
			DisposeRoutineDescriptor( filterUPP );
		}
		
		if( IsntPGPError( err ) && IsntNull( userSettings ) )
		{
			*userSettings = passphraseGrafPortView->GetSettings();
		}

		delete passphraseGrafPortView;
		DisposeDialog( getPassphraseDialog );
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}

	return ( err );
}

	PGPError
PGPGetSigningPassphraseDialog(
	PGPContextRef				context,
	PGPKeySetRef				allKeys,
	const char 					*prompt,
	PGPGetPassphraseOptions		dialogOptions,
	PGPGetPassphraseSettings	defaultSettings,
	PGPKeyRef					defaultSigningKey,
	char						passphrase[256],
	PGPGetPassphraseSettings	*userSettings,
	PGPKeyRef 					*actualSigningKey)
{
	PGPUILibState	state;
	CComboError		err;
	
	if ( IsntNull( passphrase ) )
		*passphrase	= 0;
	if ( IsntNull( actualSigningKey ) )
		*actualSigningKey	= NULL;
	if ( IsntNull( userSettings ) )
		pgpClearMemory( userSettings, sizeof( *userSettings ) );
		
	PGPValidatePtr( passphrase );
	pgpClearMemory( passphrase, 256 );
	PGPValidatePtr( actualSigningKey );
	
	AssertAddrNullOrValid( prompt, uchar, "PGPGetSigningPassphraseDialog" );
	AssertAddrNullOrValid( defaultSigningKey, VoidAlign,
				"PGPGetSigningPassphraseDialog" );
	AssertAddrNullOrValid( userSettings, PGPGetPassphraseSettings,
				"PGPGetSigningPassphraseDialog" );
	AssertAddrNullOrValid( actualSigningKey, PGPKeyRef,
			"PGPGetSigningPassphraseDialog" );

	err = EnterPGPUILib( context, &state );
	if( err.IsntError() )
	{
		err.pgpErr = RealPGPGetSigningPassphraseDialog(
						state.clientPrefsRef, allKeys, prompt,
						dialogOptions, defaultSettings,
						defaultSigningKey, passphrase,
						userSettings, actualSigningKey );
	}
		
	ExitPGPUILib( &state );
	
	return( err.ConvertToPGPError() );
}












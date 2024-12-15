/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: PGPRecipientDialog.cp,v 1.18.6.1 1997/11/20 20:41:37 heller Exp $
____________________________________________________________________________*/

#include <LGACheckbox.h>
#include <PP_Messages.h>
#include <URegistrar.h>

#include "MacDebug.h"
#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpMem.h"

#include "pgpUserInterface.h"
#include "PGPRecipientGrafPort.h"
#include "PGPUILibUtils.h"
#include "PGPUILibDialogs.h"
#include "pflPrefTypes.h"
#include "pgpClientPrefs.h"
#include "pgpOpenPrefs.h"
#include "pgpKeys.h"
#include "CComboError.h"

	static CComboError
FindNewKeysInWorkingSet(
	PGPContextRef	context,
	PGPKeySetRef	workingKeySet,
	PGPKeySetRef	*newKeySet)
{
	CComboError		err;

	err.pgpErr = PGPNewKeySet( context, newKeySet );
	if( err.IsntError() )
	{
		PGPKeyListRef	keyList;
		PGPBoolean		haveNewKeys = FALSE;
		
		err.pgpErr = PGPOrderKeySet( workingKeySet, kPGPAnyOrdering,
					&keyList );
		if( err.IsntError() )
		{
			PGPKeyIterRef	keyIterator;

			err.pgpErr = PGPNewKeyIter( keyList, &keyIterator );
			if( err.IsntError() )
			{
				PGPKeyRef	theKey;
				
				err.pgpErr = PGPKeyIterNext( keyIterator, &theKey );
				while( err.IsntError() )
				{
					PGPUserValue	keyState;
					
					err.pgpErr = PGPGetKeyUserVal( theKey, &keyState );
					if( err.IsntError() )
					{
						if( keyState == (PGPUserValue)
								kKeyStateAddedFromKeySearch )
						{
							PGPKeySetRef	additionalKeys;
							
							haveNewKeys = TRUE;
							
							err.pgpErr = PGPNewSingletonKeySet( theKey,
											&additionalKeys );
							if( err.IsntError() )
							{
								err.pgpErr = PGPAddKeys( additionalKeys,
										*newKeySet );
										
								(void) PGPFreeKeySet( additionalKeys );
							}
						}
					}
					
					if( err.IsError() )
						break;
					
					err.pgpErr = PGPKeyIterNext( keyIterator, &theKey );
				}
				
				if( err.pgpErr == kPGPError_EndOfIteration )
					err.pgpErr = kPGPError_NoErr;
					
				PGPFreeKeyIter( keyIterator );
			}
			
			PGPFreeKeyList( keyList );
		}

		if( err.IsntError() && haveNewKeys )
		{
			err.pgpErr = PGPCommitKeyRingChanges( *newKeySet );
		}
		else
		{
			(void) PGPFreeKeySet( *newKeySet );
			*newKeySet = kInvalidPGPKeySetRef;
		}
	}
	
	return( err );
}

	static CComboError
RealPGPRecipientDialog(
	PGPContextRef			context,
	PGPPrefRef				clientPrefsRef,
	PGPPrefRef				adminPrefsRef,
	PGPKeySetRef			allKeys,
	PGPUInt32				numDefaultRecipients,
	const PGPKeySpec		*defaultRecipients,
	PGPRecipientOptions		dialogOptions,	
	PGPRecipientSettings	defaultSettings,
	PGPRecipientSettings	*userSettings,
	PGPKeySetRef			*actualRecipients,
	PGPBoolean				*didShowDialog,
	PGPKeySetRef			*newKeys)
{
	CPGPRecipientGrafPort	*recipientGrafPortView;
	PGPBoolean				didShow = FALSE;
	CComboError				err;

	*actualRecipients = kInvalidPGPKeySetRef;
	
	if( IsntNull( newKeys ) )
		*newKeys = kInvalidPGPKeySetRef;
		
	InitializePowerPlant();
	
	// Return TRUE if the user pressed OK with valid inputs
	// or FALSE if the user pressed Cancel or an error occurred.
	
	// Create standard Macintosh window and overlay PowerPlant onto it
	recipientGrafPortView = (CPGPRecipientGrafPort *)
			CPGPLibGrafPortView::CreateDialog( kPGPRecipientDialogID );
	if( IsntNull( recipientGrafPortView ) )
	{
		PGPBoolean		haveAllRecipients;
		PGPKeySetRef	workingKeySet	= kInvalidPGPKeySetRef;
		PGPGroupSetRef	groupSet		= kInvalidPGPGroupSetRef;
		DialogRef		recipientDialog;
		PFLContextRef	pflContext		= NULL;
		FSSpec			spec;

		err.pgpErr = PGPGetGroupsFileFSSpec( &spec );
		if ( err.IsntError() )
			err.pgpErr	= PFLNewContext( &pflContext );
		if ( err.IsntError() )
		{
			err.pgpErr = PGPNewGroupSetFromFSSpec( pflContext, &spec,
						&groupSet);
			if ( err.IsError() )
			{
				/* don't fail if we can't create groups */
				/* create an empty one we can work with */
				err.pgpErr	= PGPNewGroupSet( pflContext, &groupSet );
			}
		}
		
		recipientDialog = recipientGrafPortView->GetDialog();
		
		if ( err.IsntError() )
		{
			// Copy the set of known keys into an in-memory set so we can
			// make additions via key server searches
			
			err.pgpErr = PGPNewKeySet( context, &workingKeySet );
			if ( err.IsntError() )
			{
				err.pgpErr = PGPAddKeys( allKeys, workingKeySet );
				if ( err.IsntError() )
				{
					err.pgpErr = PGPCommitKeyRingChanges( workingKeySet );
				}
			}
		}
		
		if ( err.IsntError() )
		{
			err = recipientGrafPortView->SetParams(
								context,
								clientPrefsRef,
								adminPrefsRef,
								workingKeySet,
								numDefaultRecipients,
								defaultRecipients,
								dialogOptions,
								defaultSettings,
								groupSet,
								&haveAllRecipients);
		}
		
		if( err.IsntError() )
		{
			// Show the dialog if we have default recipients and one ore more
			// are not found or have no validity. Also show if the caller
			// explicitly wants the dialog shown.
			
			if( (! haveAllRecipients) ||
				( defaultRecipients == NULL ) || 
				(( dialogOptions & kPGPRecipientOptionsAlwaysShowDialog) != 0))
			{
				ModalFilterUPP	filterUPP;
				MessageT		dismissMessage;
				short			itemHit;
				
				InitCursor();
				ShowWindow( recipientDialog );
				SelectWindow( recipientDialog );
				
				filterUPP 	= NewModalFilterProc(
									CPGPLibGrafPortView::ModalFilterProc );
				didShow		= TRUE;
				itemHit 	= 0;

				recipientGrafPortView->SetDismissMessage( msg_Nothing );
				
				while( itemHit == 0 )
				{
					ModalDialog( filterUPP, &itemHit );
				}

				dismissMessage =
							recipientGrafPortView->GetDismissMessage();
				if( dismissMessage == msg_Cancel )
				{
					err.pgpErr = kPGPError_UserAbort;
				}
			
				DisposeRoutineDescriptor( filterUPP );
			}

			if( err.IsntError() )
			{
				PGPBoolean	haveNewKeys;
				
				err.pgpErr = recipientGrafPortView->GetParams(
						userSettings, actualRecipients,
						&haveNewKeys );
						
				if( err.IsntError() && haveNewKeys && IsntNull( newKeys ) )
				{
					/*
						Find all keys added from a key server search and add
					   	these keys to a speial set we return to the caller.
					*/
					
					err = FindNewKeysInWorkingSet( context, workingKeySet,
							newKeys );
				}
			}
		}
		
		delete recipientGrafPortView;
		
		if( PGPKeySetRefIsValid( workingKeySet ) )
			PGPFreeKeySet( workingKeySet );

		if ( PGPGroupSetRefIsValid( groupSet ) )
			PGPFreeGroupSet( groupSet );

		if ( PFLContextRefIsValid( pflContext ) )
			PFLFreeContext( pflContext );
			
		DisposeDialog( recipientDialog );
	}
	else
	{
		err.pgpErr = kPGPError_OutOfMemory;
	}
	
	if( IsntNull( didShowDialog ) )
		*didShowDialog = didShow;
	
	return ( err );
}

	PGPError
PGPRecipientDialog(
	PGPContextRef			context,				// Input
	PGPKeySetRef			allKeys,				// Input
	PGPUInt32				numDefaultRecipients,	// Input (optional)
	const PGPKeySpec		*defaultRecipients,		// Input (optional)
	PGPRecipientOptions		dialogOptions,			// Input
	PGPRecipientSettings	defaultSettings,		// Input
	PGPRecipientSettings	*userSettings,			// Output
	PGPKeySetRef			*actualRecipients,		// Output
	PGPBoolean				*didShowDialog,			// Output (optional)
	PGPKeySetRef			*newKeys)				// Output (optional)
{
	PGPUILibState	state;
	CComboError		err;
	
	pgpAssert( PGPContextRefIsValid( context ) );
	pgpAssert( PGPKeySetRefIsValid( allKeys ) );
	AssertAddrNullOrValid( defaultRecipients, PGPKeySpec, nil );
	pgpAssertAddrValid( userSettings, PGPRecipientSettings);
	pgpAssertAddrValid( actualRecipients, PGPKeySetRef );
	AssertAddrNullOrValid( didShowDialog, Boolean, nil );
	AssertAddrNullOrValid( didShowDialog, PGPKeySetRef, nil );

	err = EnterPGPUILib( context, &state );
	if( err.IsntError() )
	{
		err = RealPGPRecipientDialog( context, state.clientPrefsRef,
					state.adminPrefsRef, allKeys, numDefaultRecipients,
					defaultRecipients, dialogOptions, defaultSettings,
					userSettings, actualRecipients, didShowDialog,
					newKeys );
	}
		
	ExitPGPUILib( &state );
	
	return( err.ConvertToPGPError() );
}

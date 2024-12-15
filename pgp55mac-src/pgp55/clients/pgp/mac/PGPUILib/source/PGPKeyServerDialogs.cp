/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPKeyServerDialogs.cp,v 1.22 1997/10/22 09:29:44 wprice Exp $
____________________________________________________________________________*/

#include <OpenTptInternet.h>

#include "pgpUserInterface.h"
#include "CServerStatusDialog.h"
#include "PGPUILibUtils.h"
#include "PGPUILibDialogs.h"
#include "WarningAlert.h"
#include "MacStrings.h"

#include "pgpMem.h"
#include "pflContext.h"
#include "PGPOpenPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpGroupsUtil.h"
#include "pgpKeyServer.h"
#include "pgpKeyServerPrefs.h"
#include "pgpKeyServerTypes.h"
#include "pgpErrors.h"
#include <string.h>

#include "pgpKeys.h"

const ResIDT kPGPServerStatusDialogID			= 4757;
const PGPInt16	kMaxUserIDLength				= 256;

typedef struct KSCallbackInfo
{
	CServerStatusDialog	*dialog;
	PGPUInt32			lastState;
	ModalFilterUPP		filterUPP;
	Boolean				cancelled; /* KS lib not very good at reporting this */
	Boolean				dialogVisible;
	UInt32				displayTicks;
	DialogRef			dialogRef;
	
} KSCallbackInfo;

static PGPError KSCallback(	PGPContextRef	context,
							PGPEvent		*event,
							PGPUserValue	userValue);
pascal Boolean PGPKeyServerModalFilterProc(
							DialogPtr 	theDialog,
							EventRecord *theEvent,
							short 		*itemHit);
							
enum	{
			kKSStringListID			= 8945,
			kKSConnectingStringID	= 1,
			kKSWaitingStringID,
			kKSReceivingStringID,
			kKSSendingStringID,
			kKSDisconnectingStringID
		};

	PGPError
KSCallback(
	PGPContextRef	context,
	PGPEvent		*event,
	PGPUserValue	userValue)
{
	PGPError	err = kPGPError_NoErr;
	
	(void) context;
	
	if( event->type == kPGPEvent_KeyServerEvent )
	{
		KSCallbackInfo	*callbackInfo;
		SInt16			saveWindowKind;
		
		callbackInfo = (KSCallbackInfo *)userValue;

		/*
		** If the window kind is not a dialog, set it to dialogKind
		** during the callback
		*/
		
		saveWindowKind = GetWindowKind( callbackInfo->dialogRef );
		if( saveWindowKind != kDialogWindowKind )
			SetWindowKind( callbackInfo->dialogRef, kDialogWindowKind );
			
		if( ! callbackInfo->dialogVisible )
		{
			if( TickCount() >= callbackInfo->displayTicks )
			{
				SetPort( callbackInfo->dialogRef );
				ShowWindow( callbackInfo->dialogRef );
				SelectWindow( callbackInfo->dialogRef );
				UpdateDialog( callbackInfo->dialogRef,
						callbackInfo->dialogRef->visRgn );
				
				callbackInfo->dialogVisible = TRUE;
			}
		}
		
		if( callbackInfo->dialogVisible )
		{
			MessageT	dismissMessage;
			short		itemHit;

			ModalDialog( callbackInfo->filterUPP, &itemHit );
			dismissMessage = callbackInfo->dialog->GetDismissMessage();
			if( dismissMessage == msg_Cancel )
			{
				callbackInfo->cancelled = TRUE;
				err = kPGPError_UserAbort;
			}
		}
		
		if( IsntPGPError( err ) )
		{
			PGPInt16	stringID;
			Str255		caption;
			
			if( callbackInfo->lastState !=
					event->data.keyServerData.state )
			{
				switch(event->data.keyServerData.state)
				{
					case kPGPKeyServerStateConnect:
						stringID = kKSConnectingStringID;
						break;
					case kPGPKeyServerStateWait:
						stringID = kKSWaitingStringID;
						break;
					case kPGPKeyServerStateReceive:
						stringID = kKSReceivingStringID;
						break;
					case kPGPKeyServerStateSend:
						stringID = kKSSendingStringID;
						break;
					case kPGPKeyServerStateDisconnect:
						stringID = kKSDisconnectingStringID;
						break;
				}
				
				::GetIndString(caption, kKSStringListID, stringID);
				callbackInfo->dialog->SetStatusCaption(caption);
				callbackInfo->lastState = event->data.keyServerData.state;
			}
		}

		SetWindowKind( callbackInfo->dialogRef, kApplicationWindowKind );
	}
	
	return err;
}

	pascal Boolean
PGPKeyServerModalFilterProc(
	DialogPtr 	theDialog,
	EventRecord *theEvent,
	short 		*itemHit)
{
	Boolean	result;
	
	result = CPGPLibGrafPortView::ModalFilterProc(theDialog, theEvent, itemHit);
	if(theEvent->what == nullEvent)
	{
		*itemHit = 1;
		return TRUE;
	}
	else
		return result;
}

	static PGPError
MyPGPQueryKeyServer(
	PGPKeyServerRef 		keyServer, 
	PGPFilterRef 			filter, 
	PGPEventHandlerProcPtr 	callBack,
	PGPUserValue 			callBackArg, 
	PGPKeySetRef 			*resultSet,
	PGPFlags 				*resultInfo)
{
	PGPError	err;
	WindowRef	frontWindow;
	SInt16		saveWindowKind;
	
	/*
	**	OT/PPP has a bug which will cause any dialog in the foreground
	**	to be disposed within OT/PPP if the user cancels the connection.
	**	Our fix is to whack the windowKind of the foreground window if
	**	it is dialogKind.
	*/

	frontWindow = LMGetWindowList();
	if( IsntNull( frontWindow ) )
	{
		saveWindowKind = GetWindowKind( frontWindow );
		if( saveWindowKind == kDialogWindowKind )
			SetWindowKind( frontWindow, kApplicationWindowKind );
	}
	
	err = PGPQueryKeyServer( keyServer, filter, callBack, callBackArg,
				resultSet, resultInfo );
				
	if( IsntNull( frontWindow ) )
	{
		SetWindowKind( frontWindow, saveWindowKind );
	}
	
	return( err );
}

	static PGPError
QueryServer(
	PGPContextRef	context,
	PGPPrefRef		clientPrefsRef,
	PGPFilterRef	filter,
	const char		*domain,
	PGPUInt32		dialogDelayTicks,
	PGPKeySetRef	*resultSet)
{
	CServerStatusDialog		*statusDialog;
	DialogRef				statusDialogRef;
	ModalFilterUPP			filterUPP;
	PGPKeyServerRef			server;
	PGPError				err	= kPGPError_NoErr;
	Str255					caption;
	PGPUInt32				numKeys;
	Boolean					found = FALSE;
	PGPKeyServerEntry		*ksEntries;
	PGPUInt32				numKSEntries;
	GrafPtr					savePort;
	
	PGPValidatePtr( resultSet );
	PGPValidatePtr( context );
	PGPValidatePtr( domain );
	PGPValidatePtr( filter );
	
	GetPort( &savePort );
	
	*resultSet = kInvalidPGPKeySetRef;
	
	InitializePowerPlant();

	statusDialog = (CServerStatusDialog *)
				CPGPLibGrafPortView::CreateDialog ( kPGPServerStatusDialogID );
	filterUPP = NewModalFilterProc( PGPKeyServerModalFilterProc);

	statusDialogRef = statusDialog->GetDialog();
	
	InitCursor();
	SetPort( statusDialogRef );

	::GetIndString( caption, kKSStringListID, kKSConnectingStringID );
	statusDialog->SetStatusCaption( caption );
	
	err = PGPCreateKeyServerPath( clientPrefsRef, domain,
									&ksEntries, &numKSEntries);
	if( IsntPGPError( err ) && IsntNull(ksEntries) && numKSEntries > 0)
	{
		for(Int16 serverIndex = 0; serverIndex < numKSEntries && !found;
					serverIndex++)
		{
			err = PGPNewKeyServerFromURL(context,
									ksEntries[serverIndex].serverURL,
									kPGPKSAccess_Normal,
									kPGPKSKeySpaceNormal,
									&server);
			if( IsntPGPError( err ) )
			{
				PGPKeySetRef	resultKeys = kInvalidPGPKeySetRef;
				KSCallbackInfo 	callbackInfo;
				
				CToPString(ksEntries[serverIndex].serverURL, caption);
				statusDialog->SetServerCaption(caption);

				pgpClearMemory( &callbackInfo, sizeof( callbackInfo ) );
				
				callbackInfo.filterUPP		= filterUPP;
				callbackInfo.dialog			= statusDialog;
				callbackInfo.displayTicks	= TickCount() + dialogDelayTicks;
				callbackInfo.dialogRef		= statusDialogRef;
				
				err = MyPGPQueryKeyServer(server, filter,
										KSCallback, &callbackInfo,
										&resultKeys, NULL);
				
				if( IsntPGPError( err ) &&
					PGPKeySetRefIsValid( resultKeys ) &&
					! callbackInfo.cancelled )
				{
					err = PGPCountKeys(resultKeys, &numKeys);

					if(numKeys >= 1)
					{
						found = TRUE;
						*resultSet = resultKeys;
					}
				}
				
				if( PGPKeySetRefIsValid( resultKeys ) && ! found )
				{
					(void) PGPFreeKeySet(resultKeys);
				}
				
				PGPFreeKeyServer(server);
				
				if(callbackInfo.cancelled)
					err = kPGPError_UserAbort;
			}
		}

		PGPDisposeKeyServerPath( ksEntries );
	}	
	
	DisposeRoutineDescriptor( filterUPP );
	delete statusDialog;
	
	DisposeDialog( statusDialogRef );
	SetPort( savePort );
	
	return err;
}

	PGPError
PGPGetKeyFromServerInternal(
	PGPContextRef	context,
	PGPPrefRef		clientPrefsRef,
	PGPKeyID const *		keyID,
	PGPKeyRef		key,			/* if not-NULL, used for domain info */
	const char		*domain,		/* if not-NULL, textual domain lookup */
	PGPUInt32		dialogDelayTicks,
	PGPKeySetRef	*resultSet)
{
	PGPError	err	= kPGPError_NoErr;
	char		domainStr[kMaxUserIDLength];
	
	if( ! CFM_AddressIsResolved_( PGPKeyServerInit ) )
		return( kPGPError_FeatureNotAvailable );
		
	domainStr[0] = '\0';
	
	if( IsntNull( key ) )
	{
		char	userID[kMaxUserIDLength];
		PGPSize	len;
		
		// If we know the domain, let's see if we have a keyserver
		// for that domain
		
		len = kMaxUserIDLength;
		err = PGPGetPrimaryUserIDNameBuffer(key, sizeof( userID ),
											userID, &len);
		PGPFindEmailDomain( userID, domainStr );
	}
	else if( IsntNull( domain ) )
	{
		PGPFindEmailDomain( domain, domainStr );
	}
	
	if( IsntPGPError( err ) )
	{
		PGPFilterRef	filter;

		if( IsntNull( keyID ) )
		{
			err = PGPNewKeyIDFilter( context, keyID, &filter );
		}
		else
		{
			err = PGPNewUserIDStringFilter( context, domain,
											kPGPMatchSubString, &filter );
		}
		
		pgpAssertAddrValid( filter, PGPFilterRef);
		
		if( IsntPGPError( err ) )
		{
			err = QueryServer( context, clientPrefsRef, filter,
						domainStr, dialogDelayTicks, resultSet );

			PGPFreeFilter(filter);
		}
	}
		
	return err;
}

	PGPError
PGPGetKeyFromServer(
	PGPContextRef	context,
	PGPKeyID const *keyID,
	PGPKeyRef		key,			/* if not-NULL, used for domain info */
	const char		*domain,		/* if not-NULL, textual domain lookup */
	PGPUInt32		dialogDelayTicks,
	PGPKeySetRef	*resultSet)
{
	PGPUILibState	state;
	PGPError		err;
	
	*resultSet = kInvalidPGPKeySetRef;
	
	err = EnterPGPUILib( context, &state ).ConvertToPGPError();
	if( IsntPGPError( err ) )
	{
		err = PGPGetKeyFromServerInternal( context, state.clientPrefsRef,
					keyID, key, domain, dialogDelayTicks, resultSet );
		
		ExitPGPUILib( &state );
	}
	
	return err;
}

	static PGPError
GroupItemToKeyIDFilter(
	PGPContextRef		context,
	const PGPGroupItem 	*groupItem,
	PGPFilterRef		*filter)
{
	PGPError	err;
	PGPKeyID	keyID;
	
	PGPValidatePtr( filter );
	*filter = kInvalidPGPFilterRef;
	PGPValidatePtr( groupItem );
	PGPValidatePtr( context );
	
	pgpAssert( groupItem->type == kPGPGroupItem_KeyID );
	
	err = PGPImportKeyID( groupItem->u.key.exportedKeyID, &keyID );
	if( IsntPGPError( err ) )
	{
		err = PGPNewKeyIDFilter( context, &keyID, filter );
	}
	
	return( err );
}

	PGPError
PGPGetGroupFromServerInternal(
	PGPContextRef	context,
	PGPPrefRef		clientPrefsRef,
	PGPGroupSetRef	groupSet,
	PGPGroupID		groupID,
	const char		*domain,		/* if not-NULL, used for domain info */
	PGPUInt32		dialogDelayTicks,
	PGPKeySetRef	*resultSet)
{
	PGPError			err;
	char				domainStr[kMaxUserIDLength];
	PGPGroupSetRef		flattenedGroupSet;
	PGPGroupID			flattenedGroupID;
	PGPGroupItemIterRef	iterator;
	PGPKeySetRef		foundSet;
	
	if( ! CFM_AddressIsResolved_( PGPKeyServerInit ) )
		return( kPGPError_FeatureNotAvailable );
		
	flattenedGroupSet	= kInvalidPGPGroupSetRef;
	*resultSet 			= kInvalidPGPKeySetRef;
	iterator			= kInvalidPGPGroupItemIterRef;
	foundSet			= kInvalidPGPKeySetRef;
	
	domainStr[0] = '\0';

	if( IsntNull( domain) )
		PGPFindEmailDomain( domain, domainStr );
	
	/* Create a flattened group with unique key ID's and other setup*/
	err = PGPNewGroupSet( PGPGetGroupSetContext( groupSet ),
			&flattenedGroupSet );
	if( IsntPGPError( err ) )
	{
		err = PGPNewFlattenedGroupFromGroup( groupSet, groupID,
					flattenedGroupSet, &flattenedGroupID );
		if( IsntPGPError( err ) )
		{
			err = PGPNewGroupItemIter( flattenedGroupSet,
				flattenedGroupID,
				kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys,
				&iterator );
			if( IsntPGPError( err ) )
			{
				err = PGPNewKeySet( context, &foundSet );
			}
		}
	}
	
	if( IsntPGPError( err ) )
	{
		CServerStatusDialog		*statusDialog;
		DialogRef				statusDialogRef;
		ModalFilterUPP			filterUPP;
		PGPKeyServerEntry		*ksEntries;
		PGPUInt32				numKSEntries;
		GrafPtr					savePort;
		KSCallbackInfo 			callbackInfo;
		
		GetPort( &savePort );
	
		InitializePowerPlant();

		statusDialog = (CServerStatusDialog *)
				CPGPLibGrafPortView::CreateDialog( kPGPServerStatusDialogID );
		filterUPP = NewModalFilterProc( CPGPLibGrafPortView::ModalFilterProc );
	
		statusDialogRef = statusDialog->GetDialog();
		
		InitCursor();
		SetPort( statusDialogRef );

		pgpClearMemory( &callbackInfo, sizeof( callbackInfo ) );
		
		callbackInfo.filterUPP		= filterUPP;
		callbackInfo.dialog			= statusDialog;
		callbackInfo.displayTicks	= TickCount() + dialogDelayTicks;
		callbackInfo.dialogRef		= statusDialogRef;

		err = PGPCreateKeyServerPath( clientPrefsRef, domainStr,
										&ksEntries, &numKSEntries);
		if( IsntPGPError( err ) &&
			IsntNull( ksEntries) &&
			numKSEntries > 0 )
		{
			PGPGroupItem	groupItem;
			
			err = PGPGroupItemIterNext( iterator, &groupItem );
			while( IsntPGPError( err ) )
			{
				PGPFilterRef	searchFilter;
				
				err = GroupItemToKeyIDFilter( context, &groupItem,
						&searchFilter );
				if( IsntPGPError( err ) )
				{
					Boolean	found = FALSE;
					Str255	caption;

					GetIndString( caption, kKSStringListID,
							kKSConnectingStringID );
					statusDialog->SetStatusCaption( caption );
			
					for(Int16 serverIndex = 0; serverIndex < numKSEntries;
								serverIndex++)
					{
						PGPKeyServerRef	server;

						err = PGPNewKeyServerFromURL(context,
										ksEntries[serverIndex].serverURL,
										kPGPKSAccess_Normal,
										kPGPKSKeySpaceNormal,
										&server);
						if( IsntPGPError( err ) )
						{
							PGPKeySetRef resultKeys = kInvalidPGPKeySetRef;
							
							CToPString( ksEntries[serverIndex].serverURL,
									caption );
							statusDialog->SetServerCaption(caption);

							err = MyPGPQueryKeyServer(server, searchFilter,
													KSCallback,
													&callbackInfo,
													&resultKeys,
													NULL);
							
							if( IsntPGPError( err ) &&
								PGPKeySetRefIsValid( resultKeys ) &&
								! callbackInfo.cancelled )
							{
								PGPUInt32	numKeys;

								err = PGPCountKeys(resultKeys, &numKeys);
								if( IsntPGPError( err ) && numKeys >= 1 )
								{
									found = TRUE;
									
									err = PGPAddKeys( resultKeys, foundSet );
									if( IsntPGPError( err ) )
									{
										err = PGPCommitKeyRingChanges(
													foundSet );
									}
								}
							}
							
							if( PGPKeySetRefIsValid( resultKeys ) )
								(void) PGPFreeKeySet( resultKeys );
							
							PGPFreeKeyServer( server );
							
							if( IsntPGPError( err ) &&
								callbackInfo.cancelled )
							{
								err = kPGPError_UserAbort;
							}
						}
					
						if( found || IsPGPError( err ) )
							break;
					}
					
					(void) PGPFreeFilter( searchFilter );
				}
				
				if( IsPGPError( err ) )
					break;
					
				err = PGPGroupItemIterNext( iterator, &groupItem );
			}
			
			if( err == kPGPError_EndOfIteration )
				err = kPGPError_NoErr;
				
			PGPDisposeKeyServerPath( ksEntries );
		}	
		
		DisposeRoutineDescriptor( filterUPP );
		delete statusDialog;
	
		DisposeDialog( statusDialogRef );
		SetPort( savePort );
	}
	
	if( IsntPGPError( err ) )
	{
		*resultSet = foundSet;
	}
	else if( PGPKeySetRefIsValid( foundSet ) )
	{
		PGPFreeKeySet( foundSet );
	}
		
	if( PGPGroupItemIterRefIsValid( iterator ) )
		(void) PGPFreeGroupItemIter( iterator );
		
	if( PGPGroupSetRefIsValid( flattenedGroupSet ) )
		(void) PGPFreeGroupSet( flattenedGroupSet );
	
	
	return( err );
}

	PGPError
PGPGetGroupFromServer(
	PGPContextRef	context,
	PGPGroupSetRef	groupSet,
	PGPGroupID		groupID,
	const char		*domain,		/* if not-NULL, used for domain info */
	PGPUInt32		dialogDelayTicks,
	PGPKeySetRef	*resultSet)
{
	PGPUILibState	state;
	PGPError		err;
	
	*resultSet = kInvalidPGPKeySetRef;
	
	err = EnterPGPUILib( context, &state ).ConvertToPGPError();
	if( IsntPGPError( err ) )
	{
		err = PGPGetGroupFromServerInternal( context, state.clientPrefsRef,
					groupSet, groupID, domain, dialogDelayTicks, resultSet );
		
		ExitPGPUILib( &state );
	}
	
	return err;
}

	static PGPError
MyPGPUploadToKeyServer(
	PGPKeyServerRef 		keyServer, 
	PGPKeySetRef 			keysToUpload, 
	PGPEventHandlerProcPtr 	callBack, 
	PGPUserValue 			callBackArg,
	PGPKeySetRef 			*keysThatFailed)
{
	PGPError	err;
	WindowRef	frontWindow;
	SInt16		saveWindowKind;
	
	/*
	**	OT/PPP has a bug which will cause any dialog in the foreground
	**	to be disposed within OT/PPP if the user cancels the connection.
	**	Our fix is to whack the windowKind of the foreground window if
	**	it is dialogKind.
	*/

	frontWindow = LMGetWindowList();
	if( IsntNull( frontWindow ) )
	{
		saveWindowKind = GetWindowKind( frontWindow );
		if( saveWindowKind == kDialogWindowKind )
			SetWindowKind( frontWindow, kApplicationWindowKind );
	}
	
	err = PGPUploadToKeyServer( keyServer, keysToUpload, callBack,
				callBackArg, keysThatFailed );
				
	if( IsntNull( frontWindow ) )
	{
		SetWindowKind( frontWindow, saveWindowKind );
	}
	
	return( err );
}

	PGPError
PGPSendKeyToServer(
	PGPContextRef		context,
	PGPKeyRef			key,
	PGPUInt32			dialogDelayTicks,
	PGPKeyServerEntry	*targetServer)
{
	CServerStatusDialog		*statusDialog;
	DialogRef				statusDialogRef;
	ModalFilterUPP			filterUPP;
	PGPUILibState			state;
	PGPKeySetRef			singleKeySet,
							failedKeys;
	char					userID[kMaxUserIDLength],
							domain[kMaxUserIDLength],
							serverName[kMaxServerNameLength];
	Str255					caption;
	PGPSize					len;
	PGPKeyServerEntry		*ksEntries;
	PGPUInt32				numKSEntries;
	PGPKeyServerRef			server;
	PGPError				err;

	PGPValidatePtr( key );
	PGPValidatePtr( context );
	
	/* Check for waek link failure */
	
	if( ! CFM_AddressIsResolved_( PGPKeyServerInit ) )
		return( kPGPError_FeatureNotAvailable );
		
	err = EnterPGPUILib( context, &state ).ConvertToPGPError();
	if(IsPGPError(err))
		return err;

	InitializePowerPlant();

	statusDialog = (CServerStatusDialog *)
			CPGPLibGrafPortView::CreateDialog( kPGPServerStatusDialogID);
	filterUPP = NewModalFilterProc( PGPKeyServerModalFilterProc);

	statusDialogRef = statusDialog->GetDialog();
	
	InitCursor();

	::GetIndString(caption, kKSStringListID, kKSConnectingStringID);
	statusDialog->SetStatusCaption(caption);

	if(IsntNull(targetServer))
	{
		CopyCString(targetServer->serverURL, serverName);
	}
	else
	{
		domain[0] = serverName[0] = '\0';
		len = kMaxUserIDLength;
		err = PGPGetPrimaryUserIDNameBuffer(key, sizeof( userID ),
											userID, &len);
		pgpAssertNoErr(err);
		PGPFindEmailDomain(userID, domain);
		serverName[0] = '\0';
		err = PGPCreateKeyServerPath(state.clientPrefsRef, domain,
									&ksEntries, &numKSEntries);
		pgpAssertNoErr(err);
		if(IsntPGPError(err))
		{
			if(IsntNull(ksEntries) && numKSEntries > 0)
				CopyCString(ksEntries[0].serverURL, serverName);
			PGPDisposeKeyServerPath(ksEntries);
		}
	}
	if(IsntPGPError(err) && serverName[0] != '\0')
	{
		err = PGPNewSingletonKeySet(key, &singleKeySet);
		pgpAssertNoErr(err);
		err = PGPNewKeyServerFromURL(context, serverName,
									kPGPKSAccess_Normal,
									kPGPKSKeySpaceNormal,
									&server);
		pgpAssertNoErr(err);
		if(IsntPGPError(err))
		{
			KSCallbackInfo callbackInfo;
			
			CToPString(serverName, caption);
			statusDialog->SetServerCaption(caption);
			
			pgpClearMemory( &callbackInfo, sizeof( callbackInfo ) );
			
			callbackInfo.filterUPP		= filterUPP;
			callbackInfo.dialog			= statusDialog;
			callbackInfo.displayTicks	= TickCount() + dialogDelayTicks;
			callbackInfo.dialogRef		= statusDialogRef;

			failedKeys = kInvalidPGPKeySetRef;
			
			err = MyPGPUploadToKeyServer(server, singleKeySet,
								KSCallback, &callbackInfo,
								&failedKeys);
			
			if(err == kPGPError_ServerKeyAlreadyExists)
				err = kPGPError_NoErr;
			if(callbackInfo.cancelled)
				err = kPGPError_UserAbort;
			if(PGPKeySetRefIsValid(failedKeys))
				PGPFreeKeySet(failedKeys);
			PGPFreeKeyServer(server);
		}
		PGPFreeKeySet(singleKeySet);
	}
	DisposeRoutineDescriptor( filterUPP );
	delete statusDialog;
	DisposeDialog( statusDialogRef );
	
	ExitPGPUILib( &state );

	return err;
}


/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpKeyServerDialogCommon.cpp,v 1.11.2.1.2.1 1998/11/12 03:23:00 heller Exp $
____________________________________________________________________________*/

#include "pgpKeys.h"
#include "pgpKeyServer.h"
#include "pgpMem.h"
#include "pgpUserInterface.h"

#include "pgpKeyServerDialogCommon.h"

typedef struct KeyServerCallbackInfo
{
	PGPEventHandlerProcPtr	dialogHandler;
	PGPUserValue			dialogUserValue;
	
	PGPEventHandlerProcPtr	clientIdleHandler;
	PGPUserValue			clientIdleUserValue;
	
	PGPEventHandlerProcPtr	clientEventHandler;
	PGPUserValue			clientEventUserValue;

} KeyServerCallbackInfo;

typedef struct KeyServerOpInfo
{
	enum
	{
		kSearchServerOp,
		kSendToServerOp
	} op;
	
	union
	{
		struct
		{
			PGPFilterRef	filter;
			PGPKeySetRef	*searchResults;
		} search;
		
		struct
		{
			PGPKeySetRef	keysToSend;
			PGPKeySetRef	*failedKeys;
		} send;
	};
	
} KeyServerOpInfo;

	static PGPError
KeyServerCallbackProc(
	PGPContextRef 	context,
	struct PGPEvent *event,
	PGPUserValue 	userValue)
{
	PGPError				err = kPGPError_NoErr;
	KeyServerCallbackInfo	*info;
	
	info = (KeyServerCallbackInfo *) userValue;
	
	switch( event->type )
	{
		case kPGPEvent_KeyServerIdleEvent:
		{
			if( IsntNull( info->clientIdleHandler ) )
			{
				err = (*info->clientIdleHandler)( context, event,
								info->clientIdleUserValue );
			}
			
			break;
		}
			
		default:
		{
			if( IsntNull( info->clientEventHandler ) )
			{
				err = (*info->clientEventHandler)( context, event,
								info->clientEventUserValue );
			}
				
			break;
		}
	}

	if( IsntPGPError( err ) )
	{
		err = (*info->dialogHandler)( context, event, info->dialogUserValue );
	}
		
	return( err );
}

	static PGPError
DoKeyServerOp(
	PGPContextRef			context,
	const PGPKeyServerSpec	*serverSpec,
	PGPtlsContextRef		tlsContext,
	KeyServerOpInfo			*opInfo,
	PGPEventHandlerProcPtr	eventHandler,
	PGPUserValue			eventUserValue)
{
	PGPError						err;
	KeyServerCallbackInfo			info;
	PGPKeyServerThreadStorageRef	previousStorage;

	
	PGPKeyServerInit();
	PGPKeyServerCreateThreadStorage(&previousStorage);

	pgpClearMemory( &info, sizeof( info ) );
	
	info.dialogHandler 		= eventHandler;
	info.dialogUserValue	= eventUserValue;
	
	/* Override the event handlers with ours and query the server. */
	
	err = PGPGetKeyServerIdleEventHandler( &info.clientIdleHandler,
					&info.clientIdleUserValue );
	if( IsntPGPError( err ) )
	{
		err = PGPGetKeyServerEventHandler( serverSpec->server,
						&info.clientEventHandler,
						&info.clientEventUserValue );
	}
	
	if( IsntPGPError( err ) )
	{
#if ! PGP_WIN32
		err = PGPSetKeyServerIdleEventHandler( KeyServerCallbackProc,
								&info );
#endif
		if( IsntPGPError( err ) )
		{
			err = PGPSetKeyServerEventHandler( serverSpec->server,
						KeyServerCallbackProc, &info );
			if( IsntPGPError( err ) )
			{
				PGPtlsSessionRef	tls = kInvalidPGPtlsSessionRef;
				PGPEvent			theEvent;
				
				/*
				** Generate "fake" connecting event to give UI a chance to
				** make key server calls when the server is not really busy
				*/
				
				pgpClearMemory( &theEvent, sizeof( theEvent ) );
				
				theEvent.type = kPGPEvent_KeyServerEvent;
				theEvent.data.keyServerData.keyServerRef =
							serverSpec->server;
				theEvent.data.keyServerData.state =
							kPGPKeyServerState_Opening;
				
				err = KeyServerCallbackProc( context, &theEvent, &info);
				if( IsntPGPError( err ) &&
					PGPtlsContextRefIsValid( tlsContext ) )
				{
					PGPKeyServerType	serverType;
					
					err = PGPGetKeyServerType( serverSpec->server,
								&serverType );
					if( IsntPGPError( err ) &&
						serverType == kPGPKeyServerType_LDAPS )
					{
						err = PGPNewTLSSession( tlsContext, &tls );
					}
				}
				
				if( IsntPGPError( err ) )
				{
					err = PGPKeyServerOpen( serverSpec->server, tls );
					if( IsntPGPError( err ) )
					{
						if( opInfo->op == KeyServerOpInfo::kSearchServerOp )
						{
							err = PGPQueryKeyServer( serverSpec->server,
										opInfo->search.filter,
										opInfo->search.searchResults );
						}
						else
						{
							pgpAssert( opInfo->op ==
									KeyServerOpInfo::kSendToServerOp );
									
							err = PGPUploadToKeyServer( serverSpec->server,
										opInfo->send.keysToSend,
										opInfo->send.failedKeys );
						}
						
						(void) PGPKeyServerClose( serverSpec->server );
					}
				}
				
				if( PGPtlsSessionRefIsValid( tls ) )
					(void) PGPFreeTLSSession( tls );
					
				(void) PGPSetKeyServerEventHandler( serverSpec->server,
								info.clientEventHandler,
								info.clientEventUserValue);
			}
		}
		
		(void) PGPSetKeyServerIdleEventHandler( info.clientIdleHandler,
						info.clientIdleUserValue );
	}
	
	PGPKeyServerDisposeThreadStorage(previousStorage);

	PGPKeyServerCleanup();

	return( err );
}

	PGPError
PGPSearchKeyServerDialogCommon(
	PGPContextRef 			context,
	const PGPKeyServerSpec 	serverList[],
	PGPUInt32				serverCount,
	PGPtlsContextRef		tlsContext,
	PGPFilterRef 			filterRef,
	PGPEventHandlerProcPtr	eventHandler,
	PGPUserValue			eventUserValue,
	PGPBoolean				searchAllServers,
	PGPKeySetRef 			*foundKeys)
{
	PGPError	err = kPGPError_NoErr;
	
	err = PGPNewKeySet( context, foundKeys );
	if( IsntPGPError( err ) )
	{
		PGPUInt32		serverIndex;
		PGPBoolean		doneSearching = FALSE;
		PGPBoolean		foundSomeKeys = FALSE;
		KeyServerOpInfo	opInfo;
		
		opInfo.op 					= KeyServerOpInfo::kSearchServerOp;
		opInfo.search.filter 		= filterRef;
		
		for( serverIndex = 0; serverIndex < serverCount; serverIndex++ )
		{
			PGPKeySetRef	searchResults = kInvalidPGPKeySetRef;
			PGPError		searchError;
			
			opInfo.search.searchResults = &searchResults;

			searchError = DoKeyServerOp( context, &serverList[serverIndex],
							tlsContext, &opInfo, eventHandler,
							eventUserValue );
			if( IsntPGPError( searchError ) )
			{
				PGPUInt32	numKeys;
				
				pgpAssert( PGPKeySetRefIsValid( searchResults ) );
				
				err = PGPCountKeys( searchResults, &numKeys );
				if( IsntPGPError( err ) && numKeys > 0 )
				{
					foundSomeKeys = TRUE;
					
					err = PGPAddKeys( searchResults, *foundKeys );
					if( IsntPGPError( err ) )
						err = PGPCommitKeyRingChanges( *foundKeys );
						
					if( ! searchAllServers )
						doneSearching = TRUE;
				}
				
				PGPFreeKeySet( searchResults );
			}
			else if( searchError == kPGPError_UserAbort )
			{
				err = kPGPError_UserAbort;
			}
			else
			{
				/*
				** If this is the last (or only) server in the list and we
				** get an error, propagate this error to the user if no keys
				** have been found. Otherwise, fail silently and search the
				** next server.
				*/
				
				if( serverIndex == serverCount - 1 && ! foundSomeKeys )
					err = searchError;
			}
			
			if( IsPGPError( err ) || doneSearching )
				break;
		}
	
		if( IsPGPError( err ) )
		{
			PGPFreeKeySet( *foundKeys );
			*foundKeys = kInvalidPGPKeySetRef;
		}
	}
	
	return( err );
}

	PGPError
PGPSendToKeyServerDialogCommon(
	PGPContextRef 			context,
	const PGPKeyServerSpec 	*server,
	PGPtlsContextRef		tlsContext,
	PGPKeySetRef			keysToSend,
	PGPEventHandlerProcPtr 	eventHandler,
	PGPUserValue 			eventUserValue,
	PGPKeySetRef 			*failedKeys)
{
	PGPError		err = kPGPError_NoErr;
	KeyServerOpInfo	opInfo;
	
	opInfo.op 				= KeyServerOpInfo::kSendToServerOp;
	opInfo.send.keysToSend 	= keysToSend;
	opInfo.send.failedKeys 	= failedKeys;
			
	err = DoKeyServerOp( context, server, tlsContext, &opInfo,
					eventHandler, eventUserValue );
	
	return( err );
}
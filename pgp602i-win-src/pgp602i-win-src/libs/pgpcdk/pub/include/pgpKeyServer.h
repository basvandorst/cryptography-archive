/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: pgpKeyServer.h,v 1.31.10.1 1998/11/12 03:23:36 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyServer_h
#define Included_pgpKeyServer_h

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#include "pgpOptionList.h"
#include "pgpErrors.h"
#include "pgpGroups.h"
#include "pgpTLS.h"


enum PGPKeyServerState_
{
	kPGPKeyServerState_Invalid						= 0,
	kPGPKeyServerState_Opening						= 1,
	kPGPKeyServerState_Querying						= 2,
	kPGPKeyServerState_ProcessingResults			= 3,
	kPGPKeyServerState_Uploading					= 4,
	kPGPKeyServerState_Deleting						= 5,
	kPGPKeyServerState_Disabling					= 6,
	kPGPKeyServerState_Closing						= 7,

	kPGPKeyServerState_TLSUnableToSecureConnection	= 8,
	kPGPKeyServerState_TLSConnectionSecured			= 9,

	PGP_ENUM_FORCE(PGPKeyServerState_)
};

PGPENUM_TYPEDEF(PGPKeyServerState_, PGPKeyServerState);

enum PGPKeyServerType_
{
	kPGPKeyServerType_Invalid			= 0,
	kPGPKeyServerType_LDAP				= 1,
	kPGPKeyServerType_HTTP				= 2,
	kPGPKeyServerType_LDAPS				= 3,
	kPGPKeyServerType_HTTPS				= 4,
	
	PGP_ENUM_FORCE( PGPKeyServerType_ )
};

PGPENUM_TYPEDEF( PGPKeyServerType_, PGPKeyServerType );

enum PGPKeyServerKeySpace_ /* These are only valid for LDAP keyservers */
{
	kPGPKeyServerKeySpace_Invalid	= 0,
	kPGPKeyServerKeySpace_Default	= 1,
	kPGPKeyServerKeySpace_Normal	= 2,
	kPGPKeyServerKeySpace_Pending	= 3,

	PGP_ENUM_FORCE( PGPKeyServerKeySpace_ )
};

PGPENUM_TYPEDEF( PGPKeyServerKeySpace_, PGPKeyServerKeySpace );

enum PGPKeyServerAccessType_ /* These are only valid for LDAP keyservers */
{
	kPGPKeyServerAccessType_Invalid			= 0,
	kPGPKeyServerAccessType_Default			= 1,
	kPGPKeyServerAccessType_Normal			= 2,
	kPGPKeyServerAccessType_Administrator	= 3,

	PGP_ENUM_FORCE( PGPKeyServerAccessType_ )
};

PGPENUM_TYPEDEF( PGPKeyServerAccessType_, PGPKeyServerAccessType );

/*	PGPKeyServerMonitorValues are null terminated linked lists.
	The values member is a null terminated array of char*s.
*/

typedef struct PGPKeyServerMonitorValues
{
	char *								name;
	char **								values;
	struct PGPKeyServerMonitorValues *	next;
} PGPKeyServerMonitorValues;

typedef struct PGPKeyServerMonitor 
{
	PGPKeyServerRef				keyServerRef;
	PGPKeyServerMonitorValues *	valuesHead;
} PGPKeyServerMonitor;


typedef struct PGPKeyServerThreadStorage *	PGPKeyServerThreadStorageRef;
# define kInvalidPGPKeyServerThreadStorageRef				\
			((PGPKeyServerThreadStorageRef) NULL)
#define PGPKeyServerThreadStorageRefIsValid(ref)			\
			((ref) != kInvalidPGPKeyServerThreadStorageRef)



/*	Use the idle event handler to receive periodic idle events during
	network calls. Usually this is used only in non-preemptive multi-tasking
	OSes to allow yielding in threads. Pre-emptive multi-tasking systems
	should probably not use the call as it interrupts the efficient wait state
	of threads waiting on network calls.
	
	Idle event handlers need to be added on a per thread basis.
	
	Returning an error from the idle event handler will cause the keyserver
	to quit processing and to return a kPGPError_UserAbort. */
PGPError			PGPSetKeyServerIdleEventHandler(
						PGPEventHandlerProcPtr inCallback,
						PGPUserValue inUserData);

PGPError			PGPGetKeyServerIdleEventHandler(
						PGPEventHandlerProcPtr * outCallback,
						PGPUserValue * outUserData);


/* Static storage creation */
PGPError		PGPKeyServerCreateThreadStorage(
					PGPKeyServerThreadStorageRef * outPreviousStorage);
PGPError		PGPKeyServerDisposeThreadStorage(
					PGPKeyServerThreadStorageRef inPreviousStorage);
					
/* Initialize and close the keyserver library */
PGPError			PGPKeyServerInit(void);

PGPError			PGPKeyServerCleanup(void);


/*	Creating and freeing a keyserver ref. */
PGPError 			PGPNewKeyServerFromURL(PGPContextRef inContext, 
						const char * inURL, 
						PGPKeyServerAccessType inAccessType,
						PGPKeyServerKeySpace inKeySpace,
						PGPKeyServerRef * outKeyServerRef);

PGPError			PGPNewKeyServerFromHostName(PGPContextRef inContext,
						const char * inHostName,
						PGPUInt16 inPort, /* default for protocol if 0 */
						PGPKeyServerType inType,
						PGPKeyServerAccessType inAccessType,
						PGPKeyServerKeySpace inKeySpace,
						PGPKeyServerRef * outKeyServerRef);
						
PGPError			PGPNewKeyServerFromHostAddress(PGPContextRef inContext,
						PGPUInt32 inAddress,
						PGPUInt16 inPort, /* default for protocol if 0 */
						PGPKeyServerType inType,
						PGPKeyServerAccessType inAccessType,
						PGPKeyServerKeySpace inKeySpace,
						PGPKeyServerRef * outKeyServerRef);

PGPError 			PGPFreeKeyServer(PGPKeyServerRef inKeyServerRef);
PGPError			PGPIncKeyServerRefCount(PGPKeyServerRef inKeyServerRef);


/*	Set and get the keyserver's event handler. Note that returning an error
	for a keyserver event will abort the current call. */
PGPError			PGPSetKeyServerEventHandler(
						PGPKeyServerRef inKeyServerRef,
						PGPEventHandlerProcPtr inCallback,
						PGPUserValue inUserData);

PGPError			PGPGetKeyServerEventHandler(
						PGPKeyServerRef inKeyServerRef,
						PGPEventHandlerProcPtr * outCallback,
						PGPUserValue * outUserData);
						
						
/*	Canceling a call to a keyserver. This is the only call that can be made
	to a keyserver that is currently in another call. Also, once you have
	returned from a canceled call, you may only close the keyserver. */
PGPError			PGPCancelKeyServerCall(PGPKeyServerRef inKeyServerRef);
						
						
/*	Opening and closing the keyserver. A keyserver ref can be opened and
	closed multiple times as necessary. */
PGPError			PGPKeyServerOpen(PGPKeyServerRef inKeyServerRef,
						PGPtlsSessionRef inTLSSessionRef);

PGPError			PGPKeyServerClose(PGPKeyServerRef inKeyServerRef);


/*	Get keyserver info. */
PGPError			PGPGetKeyServerTLSSession(PGPKeyServerRef inKeyServerRef,
						PGPtlsSessionRef * outTLSSessionRef);

PGPError			PGPGetKeyServerType(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerType * outType);

PGPError			PGPGetKeyServerAccessType(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerAccessType * outAccessType);

PGPError			PGPGetKeyServerKeySpace(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerKeySpace * outKeySpace);

PGPError			PGPGetKeyServerPort(PGPKeyServerRef inKeyServerRef,
						PGPUInt16 * outPort);

PGPError			PGPGetKeyServerHostName(PGPKeyServerRef inKeyServerRef,
						char ** outHostName); /* Use PGPFreeData to free */

PGPError			PGPGetKeyServerAddress(PGPKeyServerRef inKeyServerRef,
						PGPUInt32 * outAddress);

/*	If there was an error string returned from the server, you can get it with
	this function. Note that if there is no string, the function will return
	kPGPError_NoErr and *outErrorString will be	NULL */
PGPError			PGPGetLastKeyServerErrorString(
						PGPKeyServerRef inKeyServerRef,
						char ** outErrorString); /* Use PGPFreeData to free */


/* These functions may be used with both HTTP and LDAP keyservers */
PGPError 			PGPQueryKeyServer(PGPKeyServerRef inKeyServerRef, 
						PGPFilterRef inFilterRef, 
						PGPKeySetRef * outFoundKeys);

PGPError 			PGPUploadToKeyServer(PGPKeyServerRef inKeyServerRef, 
						PGPKeySetRef inKeysToUpload, 
						PGPKeySetRef * outKeysThatFailed);
						
PGPError 			PGPUploadPrivateKeysToKeyServer(
						PGPKeyServerRef inKeyServerRef, 
						PGPKeySetRef inKeysToUpload, 
						PGPKeySetRef * outKeysThatFailed);
						

/* These functions may only be used with LDAP keyservers */
PGPError			PGPDeleteFromKeyServer(PGPKeyServerRef inKeyServerRef,
						PGPKeySetRef inKeysToDelete,
						PGPKeySetRef * outKeysThatFailed);

PGPError			PGPDisableFromKeyServer(PGPKeyServerRef inKeyServerRef,
						PGPKeySetRef inKeysToDisable,
						PGPKeySetRef * outKeysThatFailed);
						
PGPError			PGPSendGroupsToServer(PGPKeyServerRef inKeyServerRef,
						PGPGroupSetRef inGroupSetRef);
						
PGPError			PGPRetrieveGroupsFromServer(
						PGPKeyServerRef inKeyServerRef,
						PGPGroupSetRef * outGroupSetRef);

PGPError			PGPNewServerMonitor(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerMonitor ** outMonitor);
						
PGPError			PGPFreeServerMonitor(PGPKeyServerMonitor * inMonitor);


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif

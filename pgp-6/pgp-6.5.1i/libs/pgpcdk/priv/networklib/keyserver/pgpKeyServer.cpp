/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: pgpKeyServer.cpp,v 1.42.6.2 1999/06/04 02:14:40 heller Exp $
____________________________________________________________________________*/

#include "pgpContext.h"
#include "pgpEncodePriv.h"
#include "pgpFileIO.h"
#include "pgpFileUtilities.h"
#include "pgpKeyServer.h"
#include "pgpMem.h"
#include "pgpOptionListPriv.h"
#include "pgpSockets.h"
#include "pgpSDKNetworkLibPriv.h"

#include "CKeyServer.h"

#if PGP_WIN32 || PGP_UNIX
#include "pgpRMWOLock.h"

static PGPRMWOLock			sBusyLock;
static PGPRMWOLock			sCancelLock;
#endif

#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))

static PGPInt32				sNumberOfInits = 0;

class StKeyServerBusy {
public:
			StKeyServerBusy(CKeyServer * inKeyServer,
				PGPBoolean inClosing = false);
	virtual ~StKeyServerBusy();

protected:
	CKeyServer *	mKeyServer;
	PGPBoolean		mClosing;
};



StKeyServerBusy::StKeyServerBusy(
	CKeyServer *	inKeyServer,
	PGPBoolean		inClosing)
	: mKeyServer(inKeyServer), mClosing(inClosing)
{
#if PGP_WIN32 || PGP_UNIX
	PGPRMWOLockStartWriting(&sBusyLock);
	PGPRMWOLockStartReading(&sCancelLock);
#endif
	
	if (mKeyServer->IsBusy() || (! mClosing && mKeyServer->IsCanceled())) 
	{
#if PGP_WIN32 || PGP_UNIX
	PGPRMWOLockStopReading(&sCancelLock);
	PGPRMWOLockStopWriting(&sBusyLock);
#endif
		ThrowPGPError_(kPGPError_ServerInProgress);
 	} else {
 	 	mKeyServer->SetBusy(true);
 	}
	
#if PGP_WIN32 || PGP_UNIX
	PGPRMWOLockStopReading(&sCancelLock);
	PGPRMWOLockStopWriting(&sBusyLock);
#endif
}



StKeyServerBusy::~StKeyServerBusy()
{
	if (mClosing) {
#if PGP_WIN32 || PGP_UNIX
	PGPRMWOLockStartWriting(&sCancelLock);
#endif

		mKeyServer->ResetCanceled();

#if PGP_WIN32 || PGP_UNIX
	PGPRMWOLockStopWriting(&sCancelLock);
#endif
	}

#if PGP_WIN32 || PGP_UNIX
	PGPRMWOLockStartWriting(&sBusyLock);
#endif

	mKeyServer->SetBusy(false);	

#if PGP_WIN32 || PGP_UNIX
	PGPRMWOLockStopWriting(&sBusyLock);
#endif
}



	static PGPError
MapErrors(
	PGPError	inError)
{
	PGPError	result = inError;
	
	switch (inError) {
		case kPGPError_SocketsInProgress:
		{
			result = kPGPError_ServerInProgress;
		}
		break;
	}
	
	return result;
}



	PGPError
PGPSetKeyServerIdleEventHandler(
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue			inUserData)
{
	PGPError	result = kPGPError_NoErr;
	
	try {
		CKeyServer::SetIdleEventHandler(inCallback, inUserData);
	}

	catch (PGPError exception) {
		result = MapErrors(exception);
	}

#if ! PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
		
	return result;
}



	PGPError
PGPGetKeyServerIdleEventHandler(
	PGPEventHandlerProcPtr *	outCallback,
	PGPUserValue *				outUserData)
{
	PGPError	result = kPGPError_NoErr;
	
	try {
		CKeyServer::GetIdleEventHandler(outCallback, outUserData);
	}

	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if ! PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
		
	return result;
}




	PGPError
PGPKeyServerCreateThreadStorage(
	PGPKeyServerThreadStorageRef *	outPreviousStorage)
{
	PGPError	err = kPGPError_NoErr;
	
	try {
		if (outPreviousStorage == 0) {
			ThrowPGPError_(kPGPError_BadParams);
		}
		*outPreviousStorage = kInvalidPGPKeyServerThreadStorageRef;
					
		CKeyServer::CreateThreadStorage(
					(SThreadContext **) outPreviousStorage);
	}
	
	catch (PGPError exception) {
		err = MapErrors(exception);
	}
	
#if ! PGP_WIN32
	catch (...) {
		err = kPGPError_UnknownError;
	}
#endif

	return err;
}



	PGPError
PGPKeyServerDisposeThreadStorage(
	PGPKeyServerThreadStorageRef	inPreviousStorage)
{
	PGPError	err = kPGPError_NoErr;
	
	try {
		CKeyServer::DisposeThreadStorage(
					(SThreadContext *) inPreviousStorage);
	}
	
	catch (PGPError exception) {
		err = MapErrors(exception);
	}
	
#if ! PGP_WIN32
	catch (...) {
		err = kPGPError_UnknownError;
	}
#endif

	return err;
}


	PGPError
PGPKeyServerInit()
{
	PGPError	result = kPGPError_NoErr;

#if PGP_WIN32
	EnterCriticalSection(&gKeyServerInitMutex);
#endif

	if (sNumberOfInits == 0) {
		try {
			CKeyServer::Initialize();
		}

		catch (PGPError exception) {
			result = MapErrors(exception);
		}
		
		catch (...) {
			result = kPGPError_UnknownError;

#if PGP_WIN32
		LeaveCriticalSection(&gKeyServerInitMutex);
		throw;
#endif
		}

#if PGP_WIN32 || PGP_UNIX
		if (IsntPGPError(result)) {
			InitializePGPRMWOLock(&sBusyLock);
			InitializePGPRMWOLock(&sCancelLock);
		}
#endif

	}
	if (IsntPGPError(result)) {
		sNumberOfInits++;
	}

#if PGP_WIN32
	LeaveCriticalSection(&gKeyServerInitMutex);
#endif
	
	return result;
}



	PGPError 
PGPKeyServerCleanup()
{
	PGPError	result = kPGPError_NoErr;
	
#if PGP_WIN32
	EnterCriticalSection(&gKeyServerInitMutex);
#endif

	sNumberOfInits--;
	if (sNumberOfInits == 0) {
		try {
			CKeyServer::Cleanup();
		}

		catch (PGPError exception) {
			result = MapErrors(exception);
		}
		
		catch (...) {
			result = kPGPError_UnknownError;

#if PGP_WIN32
			LeaveCriticalSection(&gKeyServerInitMutex);
			throw;
#endif
		}
			
#if PGP_WIN32 || PGP_UNIX
		DeletePGPRMWOLock(&sCancelLock);
		DeletePGPRMWOLock(&sBusyLock);
#endif
	} else if (sNumberOfInits < 0) {
		sNumberOfInits = 0;
	}
	
#if PGP_WIN32
		LeaveCriticalSection(&gKeyServerInitMutex);
#endif

	return result;
}



	PGPError
PGPNewKeyServerFromURL(
	PGPContextRef			inContext, 
	const char *			inURL, 
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace,
	PGPKeyServerRef *		outKeyServerRef)
{
	PGPError						result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outKeyServerRef);
		*outKeyServerRef = kInvalidPGPKeyServerRef;
		PGPValidateContext(inContext);
		PGPValidatePtr(inURL);
		PGPValidateParam((inAccessType >= kPGPKeyServerAccessType_Default) &&
			(inAccessType <= kPGPKeyServerAccessType_Administrator));
		PGPValidateParam((inKeySpace >= kPGPKeyServerKeySpace_Default) &&
			(inKeySpace <= kPGPKeyServerKeySpace_Pending));
	
		*outKeyServerRef = (PGPKeyServerRef) CKeyServer::NewKeyServerFromURL(
															inContext,
															inURL,
															inAccessType,
															inKeySpace);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}

#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPNewKeyServerFromHostAddress(
	PGPContextRef			inContext,
	PGPUInt32				inAddress,
	PGPUInt16				inPort,
	PGPKeyServerProtocol		inType,
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace,
	PGPKeyServerRef *		outKeyServerRef)
{
	PGPError	result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outKeyServerRef);
		*outKeyServerRef = kInvalidPGPKeyServerRef;
		PGPValidateContext(inContext);
		PGPValidateParam((inType >= kPGPKeyServerProtocol_LDAP) &&
			(inType <= kPGPKeyServerProtocol_HTTPS));
		PGPValidateParam((inAccessType >= kPGPKeyServerAccessType_Default) &&
			(inAccessType <= kPGPKeyServerAccessType_Administrator));
		PGPValidateParam((inKeySpace >= kPGPKeyServerKeySpace_Default) &&
			(inKeySpace <= kPGPKeyServerKeySpace_Pending));

		*outKeyServerRef =
			(PGPKeyServerRef) CKeyServer::NewKeyServerFromHostAddress(
															inContext,
															inAddress,
															inPort,
															inType,
															inAccessType,
															inKeySpace);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPNewKeyServerFromHostName(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt16				inPort,
	PGPKeyServerProtocol		inType,
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace,
	PGPKeyServerRef *		outKeyServerRef)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outKeyServerRef);
		*outKeyServerRef = kInvalidPGPKeyServerRef;
		PGPValidateContext(inContext);
		PGPValidatePtr(inHostName);
		PGPValidateParam((inType >= kPGPKeyServerProtocol_LDAP) &&
			(inType <= kPGPKeyServerProtocol_HTTPS));
		PGPValidateParam((inAccessType >= kPGPKeyServerAccessType_Default) &&
			(inAccessType <= kPGPKeyServerAccessType_Administrator));
		PGPValidateParam((inKeySpace >= kPGPKeyServerKeySpace_Default) &&
			(inKeySpace <= kPGPKeyServerKeySpace_Pending));
	
		*outKeyServerRef =
				(PGPKeyServerRef) CKeyServer::NewKeyServerFromHostName(
															inContext,
															inHostName,
															inPort,
															0,
															inType,
															inAccessType,
															inKeySpace);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}


typedef struct KeyServerOptions
{
	PGPKeyServerProtocol	protocol;
	PGPKeyServerKeySpace	keySpace;
	PGPKeyServerAccessType	accessType;
	const char 				*url;
	const char 				*hostName;
	PGPUInt32 				hostAddress;
	PGPUInt16				hostPort;

} KeyServerOptions;

	static PGPError
GatherKeyServerOptions(
	PGPOptionListRef	optionList,
	KeyServerOptions	*options)
{
	PGPError	err 				= kPGPError_NoErr;
	PGPUInt32	numAddressOptions	= 0;
	PGPInt32	tempInt;
	PGPBoolean	haveOption;
	
	pgpAssert( pgpOptionListIsValid( optionList ) );
	pgpAssert( IsntNull( options ) );
	
	pgpClearMemory( options, sizeof( *options ) );
	
	options->protocol 	= kPGPKeyServerProtocol_LDAP;
	options->keySpace 	= kPGPKeyServerKeySpace_Default;
	options->accessType	= kPGPKeyServerAccessType_Default;
	
	err = pgpFindOptionArgs( optionList,
				kPGPOptionType_KeyServerProtocol, FALSE, "%b%d",
				&haveOption, &tempInt );
	if( haveOption )
	{
		options->protocol = (PGPKeyServerProtocol) tempInt;
	}
	
	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_KeyServerKeySpace, FALSE, "%b%d",
					&haveOption, &tempInt );
		if( haveOption )
		{
			options->keySpace = (PGPKeyServerKeySpace) tempInt;
		}
	}

	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_KeyServerAccessType, FALSE, "%b%d",
					&haveOption, &tempInt );
		if( haveOption )
		{
			options->accessType = (PGPKeyServerAccessType) tempInt;
		}
	}

	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_URL, FALSE, "%p", &options->url );
		if( IsntNull( options->url ) )
		{
			++numAddressOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPONetHostNameDesc	*desc = 0;
		
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_HostName, FALSE, "%p", &desc );
		if( IsntNull( desc ) )
		{
			options->hostName	= desc->hostName;
			options->hostPort	= desc->port;
			
			++numAddressOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPONetHostAddressDesc	*desc = 0;
		
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_HostAddress, FALSE, "%p", &desc );
		if( IsntNull( desc ) )
		{
			options->hostAddress	= desc->hostAddress;
			options->hostPort		= desc->port;
			
			++numAddressOptions;
		}
	}
	
	if( IsntPGPError( err ) && numAddressOptions != 1 )
	{
		pgpDebugMsg( "PGPNewKeyServer: Invalid host specification" );
		err = kPGPError_BadParams;
	}
	
	return err;
}



	static PGPError
pgpNewX509KeyServer(
	PGPContextRef		context,
	PGPKeyServerClass	serverClass,
	KeyServerOptions *	options,
	PGPKeyServerRef 	*outKeyServerRef)
{
	PGPError						result = kPGPError_NoErr;
	
	try {
		if (options->url != 0) {
			*outKeyServerRef = (PGPKeyServerRef) CKeyServer::NewKeyServerFromURL(
																context,
																options->url,
																options->accessType,
																options->keySpace,
																serverClass);
		} else if (options->hostName != 0) {
			*outKeyServerRef = (PGPKeyServerRef) CKeyServer::NewKeyServerFromHostName(
																context,
																options->hostName,
																options->hostPort,
																0,
																options->protocol,
																options->accessType,
																options->keySpace,
																serverClass);
		} else {
			*outKeyServerRef = (PGPKeyServerRef) CKeyServer::NewKeyServerFromHostAddress(
																context,
																options->hostAddress,
																options->hostPort,
																options->protocol,
																options->accessType,
																options->keySpace,
																serverClass);
		}
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}

#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



static const PGPOptionType sNewKeyServerOptionSet[] =
{
	kPGPOptionType_URL,
	kPGPOptionType_HostName,
	kPGPOptionType_HostAddress,
	kPGPOptionType_KeyServerProtocol,
	kPGPOptionType_KeyServerKeySpace,
	kPGPOptionType_KeyServerAccessType
};

	static PGPError
pgpNewKeyServer(
	PGPContextRef 		context,
	PGPKeyServerClass	serverClass,
	PGPKeyServerRef 	*outKeyServerRef,
	PGPOptionListRef	optionList)
{
	PGPError			err;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	pgpAssert( IsntNull( outKeyServerRef ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sNewKeyServerOptionSet,
					elemsof( sNewKeyServerOptionSet ) );
		if( IsntPGPError( err ) )
		{
			KeyServerOptions	options;
			
			err = GatherKeyServerOptions( optionList, &options );
			if( IsntPGPError( err ) )
			{
				switch (serverClass) {
					case kPGPKeyServerClass_PGP:
					{
						if( IsntNull( options.url ) )
						{
							err = PGPNewKeyServerFromURL( context, options.url,
										options.accessType, options.keySpace,
										outKeyServerRef );
						}
						else if( IsntNull( options.hostName ) )
						{
							err = PGPNewKeyServerFromHostName( context, options.hostName,
										options.hostPort, options.protocol,
										options.accessType, options.keySpace,
										outKeyServerRef );
						}
						else
						{
							err = PGPNewKeyServerFromHostAddress( context,
										options.hostAddress, options.hostPort,
										options.protocol, options.accessType,
										options.keySpace, outKeyServerRef );
						}
					}
					break;
					
					
					case kPGPKeyServerClass_NetToolsCA:
					case kPGPKeyServerClass_Verisign:
					case kPGPKeyServerClass_Entrust:
					{
						err = pgpNewX509KeyServer(context, serverClass, &options, outKeyServerRef);
					}
					break;
					
					
					default:
					{
						pgpDebugMsg( "Server class unimplemented" );
						err = kPGPError_FeatureNotAvailable;
					}
					break;
				}
			}
		}
	}
	
	return( err );
}

	PGPError
PGPNewKeyServer(
	PGPContextRef 			context,
	PGPKeyServerClass		serverClass,
	PGPKeyServerRef 		*outKeyServerRef,
	PGPOptionListRef		firstOption,
	... )
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;

	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( serverClass >= kPGPKeyServerClass_PGP &&
				serverClass <= kPGPKeyServerClass_Entrust );
	pgpAssert( IsntNull( outKeyServerRef ) );
	
	if( IsntNull( outKeyServerRef ) )
		*outKeyServerRef = kInvalidPGPKeyServerRef;
		
	if( pgpContextIsValid( context ) &&
		( serverClass >= kPGPKeyServerClass_PGP &&
			serverClass <= kPGPKeyServerClass_Entrust ) &&
		IsntNull( outKeyServerRef ) )
	{
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs(context, FALSE, firstOption, args);
		va_end( args );
	
		err = pgpNewKeyServer( context, serverClass, outKeyServerRef, optionList );
	
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
PGPFreeKeyServer(
	PGPKeyServerRef	inKeyServerRef)
{
	PGPError		result = kPGPError_NoErr;
	PGPBoolean		freed = false;
	
	try {
		// Validation
		PGPValidatePtr(inKeyServerRef);
	
		if (((CKeyServer *) inKeyServerRef)->IsBusy()) {
			ThrowPGPError_(kPGPError_ServerInProgress);
	 	} else {
	 	 	((CKeyServer *) inKeyServerRef)->SetBusy(true);
	 	}
	 	
	 	try {
	 		freed = ((CKeyServer *) inKeyServerRef)->Free();
	 	}
	 	
	 	catch (...) {
#if PGP_WIN32
			if (! freed) {
				/* Server still refcounted. Clear busy flag */
		 	 	((CKeyServer *) inKeyServerRef)->SetBusy(false);
		 	}
			throw;
#endif
	 	}

		if (! freed) {
			/* Server still refcounted. Clear busy flag */
	 	 	((CKeyServer *) inKeyServerRef)->SetBusy(false);
	 	}
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}

#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPIncKeyServerRefCount(
	PGPKeyServerRef	inKeyServerRef)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(inKeyServerRef);
	
		((CKeyServer *) inKeyServerRef)->IncRefCount();
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}


	PGPError
PGPSetKeyServerEventHandler(
	PGPKeyServerRef			inKeyServerRef,
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue			inUserData)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(inKeyServerRef);
		
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->SetEventHandler(	inCallback,
															inUserData);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPGetKeyServerEventHandler(
	PGPKeyServerRef				inKeyServerRef,
	PGPEventHandlerProcPtr *	outCallback,
	PGPUserValue *				outUserData)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(inKeyServerRef)
		PGPValidatePtr(outCallback)
		*outCallback = 0;
		PGPValidatePtr(outUserData)
		*outUserData = 0;
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->GetEventHandler(	*outCallback,
															*outUserData);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPKeyServerOpen(
	PGPKeyServerRef		inKeyServerRef,
	PGPtlsSessionRef	inTLSSessionRef)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(inKeyServerRef);
		PGPValidateParam((inTLSSessionRef == kInvalidPGPtlsSessionRef)
			|| PGPtlsSessionRefIsValid(inTLSSessionRef));
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->Open(inTLSSessionRef);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPKeyServerClose(
	PGPKeyServerRef	inKeyServerRef)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef, true);

		((CKeyServer *) inKeyServerRef)->Close();
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPGetKeyServerTLSSession(
	PGPKeyServerRef		inKeyServerRef,
	PGPtlsSessionRef *	outTLSSessionRef)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outTLSSessionRef);
		*outTLSSessionRef = 0;
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetTLSSession(*outTLSSessionRef);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



#if PGP_DEPRECATED	/* [ */

	PGPError
PGPGetKeyServerType(
	PGPKeyServerRef		inKeyServerRef,
	PGPKeyServerType *	outType)
{
	return( PGPGetKeyServerProtocol( inKeyServerRef, outType ) );
}

#endif	/* ] PGP_DEPRECATED */

	PGPError
PGPGetKeyServerProtocol(
	PGPKeyServerRef			inKeyServerRef,
	PGPKeyServerProtocol *	outProtocol)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outProtocol);
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetProtocol(*outProtocol);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPGetKeyServerAccessType(
	PGPKeyServerRef				inKeyServerRef,
	PGPKeyServerAccessType *	outAccessType)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outAccessType);
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetAccessType(*outAccessType);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPGetKeyServerKeySpace(
	PGPKeyServerRef			inKeyServerRef,
	PGPKeyServerKeySpace *	outKeySpace)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outKeySpace);
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetKeySpace(*outKeySpace);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPGetKeyServerPort(
	PGPKeyServerRef	inKeyServerRef,
	PGPUInt16 *		outPort)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outPort);
		*outPort = 0;
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetPort(*outPort);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}


	PGPError
PGPGetKeyServerHostName(
	PGPKeyServerRef	inKeyServerRef,
	char **			outHostName)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outHostName);
		*outHostName = 0;
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetHostName(outHostName);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPGetKeyServerAddress(
	PGPKeyServerRef	inKeyServerRef,
	PGPUInt32 *		outAddress)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outAddress);
		*outAddress = 0;
		PGPValidatePtr(inKeyServerRef);

		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetAddress(*outAddress);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPGetKeyServerPath(
	PGPKeyServerRef	inKeyServerRef,
	char **			outPath)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outPath);
		*outPath = 0;
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetPath(outPath);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPGetLastKeyServerErrorString(
	PGPKeyServerRef	inKeyServerRef,
	char **			outErrorString)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outErrorString);
		*outErrorString = 0;
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->GetErrorString(outErrorString);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPQueryKeyServer(
	PGPKeyServerRef	inKeyServerRef, 
	PGPFilterRef	inFilterRef, 
	PGPKeySetRef *	outFoundKeys)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outFoundKeys);
		*outFoundKeys = 0;
		PGPValidatePtr(inKeyServerRef);
		PGPValidateParam(PGPFilterRefIsValid(inFilterRef));
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->Query(	inFilterRef,
												outFoundKeys);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPUploadToKeyServer(
	PGPKeyServerRef	inKeyServerRef, 
	PGPKeySetRef	inKeysToUpload, 
	PGPKeySetRef *	outKeysThatFailed)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outKeysThatFailed);
		*outKeysThatFailed = 0;
		PGPValidatePtr(inKeyServerRef);
		PGPValidateParam(PGPKeySetRefIsValid(inKeysToUpload));
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->Upload(	kPGPPublicKeyOnly,
													inKeysToUpload,
													outKeysThatFailed);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}

	PGPError
PGPDeleteFromKeyServer(
	PGPKeyServerRef	inKeyServerRef,
	PGPKeySetRef	inKeysToDelete,
	PGPKeySetRef *	outKeysThatFailed)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outKeysThatFailed);
		*outKeysThatFailed = 0;
		PGPValidatePtr(inKeyServerRef);
		PGPValidateParam(PGPKeySetRefIsValid(inKeysToDelete));
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->Delete(	inKeysToDelete,
													outKeysThatFailed);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPDisableFromKeyServer(
	PGPKeyServerRef	inKeyServerRef,
	PGPKeySetRef	inKeysToDisable,
	PGPKeySetRef *	outKeysThatFailed)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outKeysThatFailed);
		*outKeysThatFailed = 0;
		PGPValidatePtr(inKeyServerRef);
		PGPValidateParam(PGPKeySetRefIsValid(inKeysToDisable));
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->Disable(	inKeysToDisable,
													outKeysThatFailed);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPSendGroupsToServer(
	PGPKeyServerRef	inKeyServerRef,
	PGPGroupSetRef	inGroupSetRef)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(inKeyServerRef);
		PGPValidateParam(PGPGroupSetRefIsValid(inGroupSetRef));
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->SendGroups(inGroupSetRef);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}
						
	PGPError
PGPRetrieveGroupsFromServer(
	PGPKeyServerRef		inKeyServerRef,
	PGPGroupSetRef *	outGroupSetRef)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outGroupSetRef);
		*outGroupSetRef = 0;
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->RetrieveGroups(outGroupSetRef);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}



	PGPError
PGPNewServerMonitor(
	PGPKeyServerRef			inKeyServerRef,
	PGPKeyServerMonitor **	outMonitor)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outMonitor);
		*outMonitor = 0;
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->NewMonitor(outMonitor);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}

						

	PGPError
PGPFreeServerMonitor(
	PGPKeyServerMonitor *	inMonitor)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(inMonitor);
		PGPValidatePtr(inMonitor->keyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inMonitor->keyServerRef);
		
		((CKeyServer *) inMonitor->keyServerRef)->FreeMonitor(inMonitor);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}




	PGPError
PGPCancelKeyServerCall(
	PGPKeyServerRef	inKeyServerRef)
{
	PGPError	result = kPGPError_NoErr;
	
	try {
#if PGP_WIN32 || PGP_UNIX
		PGPRMWOLockStartWriting(&sCancelLock);
#endif

		// Validation
		PGPValidatePtr(inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->Cancel();

#if PGP_WIN32 || PGP_UNIX
		PGPRMWOLockStopWriting(&sCancelLock);
#endif
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}

	PGPContextRef
PGPGetKeyServerContext(PGPKeyServerRef inKeyServerRef)
{
	PGPContextRef context = kInvalidPGPContextRef;
	
	if( PGPKeyServerRefIsValid( inKeyServerRef ) )
	{
		context = ((CKeyServer *) inKeyServerRef)->GetContext();
	}

	return( context );
}		


	static PGPError
SaveDataToFile(
	PFLFileSpecRef	fileSpecRef,
	const void		*buffer,
	PGPSize			bufferSize,
	PGPBoolean		appendOutput)
{
	PGPIORef	fileIORef = 0;
	PGPError	err;
	
	if( ! appendOutput )
		(void) PFLFileSpecDelete( fileSpecRef );
		
	err = PGPOpenFileSpec( fileSpecRef, kPFLFileOpenFlags_ReadWrite,
				(PGPFileIORef *) &fileIORef );
	if( IsntPGPError( err ) )
	{
		if( appendOutput )
		{
			PGPFileOffset	eof;
			
			err = PGPIOGetEOF( fileIORef, &eof );
			if( IsntPGPError( err ) )
			{
				err = PGPIOSetPos( fileIORef, eof );		
			}
		}

		if( IsntPGPError( err ) )
		{
			err = PGPIOWrite( fileIORef, bufferSize, buffer );
		}
		
		PGPFreeIO( fileIORef );
	}
	
	return( err );
}

typedef struct SendCertRequestOptions
{
	PFLFileSpecRef	inFileSpecRef;
	PFLFileSpecRef	outFileSpecRef;
	
	PGPKeyRef		caKey;
	PGPKeyRef		requestKey;
		
	void			*inBuffer;
	PGPSize			inBufferSize;

	void			*outBuffer;
	PGPSize			maxOutBufferSize;
	PGPSize			*outBufferSize;
	
	void			**allocatedOutBuffer;
	
	PGPBoolean		appendOutput;
	
} SendCertRequestOptions;


	static PGPError
GatherSendCertRequestOptions(
	PGPOptionListRef		optionList,
	SendCertRequestOptions	*options)
{
	PGPError	err 				= kPGPError_NoErr;
	PGPUInt32	numInputOptions		= 0;
	PGPUInt32	numOutputOptions	= 0;
	PGPInt32	tempInt;
	
	pgpAssert( pgpOptionListIsValid( optionList ) );
	pgpAssert( IsntNull( options ) );
	
	pgpClearMemory( options, sizeof( *options ) );
	
	/* Find inputs */
	
	err = pgpFindOptionArgs( optionList,
				kPGPOptionType_InputFileRef, FALSE, "%f",
				&options->inFileSpecRef );
	if( PFLFileSpecRefIsValid( options->inFileSpecRef ) )
	{
		++numInputOptions;
	}
	
	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_InputBuffer, FALSE, "%p%l",
					&options->inBuffer, &options->inBufferSize );
		if( IsntNull( options->inBuffer ) )
		{
			++numInputOptions;
		}
	}
	
	/* Find CA key */
	if ( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_KeyServerCAKey, FALSE, "%p",
					&options->caKey);
	}

	/* Find requesting key */
	if ( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_KeyServerRequestKey, FALSE, "%p",
					&options->requestKey);
	}

	/* Find outputs */
	
	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_OutputFileRef, FALSE, "%f",
					&options->outFileSpecRef );
		if( PFLFileSpecRefIsValid( options->outFileSpecRef ) )
		{
			++numOutputOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPOOutputBufferDesc	*desc;
		
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_OutputBuffer, FALSE, "%p", &desc );
		if( IsntNull( options->outBuffer ) )
		{
			options->outBuffer 			= desc->buffer;
			options->maxOutBufferSize 	= desc->bufferSize;
			options->outBufferSize 		= desc->outputDataLength;
			
			*(options->outBufferSize) = 0;

			++numOutputOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPOAllocatedOutputBufferDesc	*desc;
		
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_OutputAllocatedBuffer, FALSE, "%p", &desc );
		if( IsntNull( desc ) )
		{
			options->allocatedOutBuffer = desc->buffer;
			options->maxOutBufferSize 	= desc->maximumBufferSize;
			options->outBufferSize 		= desc->actualBufferSize;
			
			*(options->allocatedOutBuffer) 	= 0;
			*(options->outBufferSize) 		= 0;
			
			++numOutputOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_AppendOutput, FALSE, "%d", &tempInt );
		if( tempInt != 0 )
		{
			options->appendOutput = TRUE;
			
			++numOutputOptions;
		}
	}

	/* Check for exactly one input and at most one output */
	
	if( IsntPGPError( err ) && 
		( numInputOptions != 1 || numOutputOptions > 1 ) )
	{
		pgpDebugMsg( "Invalid input or output specification" );
		err = kPGPError_BadParams;
	}
	
	return err;
}

static const PGPOptionType sSendCertificateReqOptionSet[] =
{
	kPGPOptionType_InputFileRef,
	kPGPOptionType_InputBuffer,
	kPGPOptionType_KeyServerCAKey,
	kPGPOptionType_KeyServerRequestKey,
	kPGPOptionType_OutputFileRef,
	kPGPOptionType_OutputBuffer,
	kPGPOptionType_OutputAllocatedBuffer,
	kPGPOptionType_AppendOutput,
	kPGPOptionType_DiscardOutput
};

	static PGPError
pgpSendCertificateRequestInternal(
	PGPKeyServerRef 	keyServerRef,
	PGPKeyRef			caKey,
	PGPKeyRef			requestKey,
	const void			*inBuffer,
	PGPSize				inBufferSize,
	void				**outBuffer,
	PGPSize				*outBufferSize)
{
	PGPError		result = kPGPError_NoErr;
	
	(void) requestKey;
	
	try {
		*outBuffer 		= 0;
		*outBufferSize	= 0;
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) keyServerRef);
		
		((CKeyServer *) keyServerRef)->SendCertificateRequest(	caKey,
																requestKey,
																inBuffer,
																inBufferSize,
																outBuffer,
																outBufferSize);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}

	static PGPError
pgpSendCertificateRequest(
	PGPContextRef		context,
	PGPKeyServerRef 	keyServerRef,
	PGPOptionListRef	optionList)
{
	PGPError		err;
	PGPMemoryMgrRef	memoryMgr = PGPGetContextMemoryMgr( context );
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( PGPKeyServerRefIsValid( keyServerRef ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sSendCertificateReqOptionSet,
					elemsof( sSendCertificateReqOptionSet ) );
		if( IsntPGPError( err ) )
		{
			SendCertRequestOptions	options;
			
			err = GatherSendCertRequestOptions( optionList, &options );
			if( IsntPGPError( err ) )
			{
				void		*inBuffer 			= 0;
				PGPSize		inBufferSize		= 0;
				void		*outBuffer 			= 0;
				PGPSize		outBufferSize		= 0;
				PGPBoolean	freeInputBuffer		= TRUE;
				PGPBoolean	freeOutputBuffer	= TRUE;
				
				if( PFLFileSpecRefIsValid( options.inFileSpecRef ) )
				{
					PGPIORef	fileIORef;
					
					err = PGPOpenFileSpec( options.inFileSpecRef,
								kPFLFileOpenFlags_ReadOnly,
								(PGPFileIORef *) &fileIORef );
					if( IsntPGPError( err ) )
					{
						PGPFileOffset	fileSize;
						
						err = PGPIOGetEOF( fileIORef, &fileSize );
						if( IsntPGPError( err ) )
						{
							inBuffer = PGPNewData( memoryMgr, (PGPSize) fileSize,
											0 );
							if( IsntNull( inBuffer ) )
							{
								err = PGPIORead( fileIORef, (PGPSize) fileSize,
											inBuffer, &inBufferSize );
							}
							else
							{
								err = kPGPError_OutOfMemory;
							}
						}
					
						PGPFreeIO( fileIORef );
					}
				}
				else if( IsntNull( options.inBuffer ) )
				{
					inBuffer 		= options.inBuffer;
					inBufferSize	= options.inBufferSize;
					freeInputBuffer	= FALSE;
				}
				
				err = pgpSendCertificateRequestInternal( keyServerRef, options.caKey,
							options.requestKey, inBuffer, inBufferSize, &outBuffer,
							&outBufferSize );
			
				if( freeInputBuffer && IsntNull( inBuffer ) )
					PGPFreeData( inBuffer );
					
				if( IsntPGPError( err ) && IsntNull( outBuffer ) )
				{
					if( PFLFileSpecRefIsValid( options.outFileSpecRef ) )
					{
						err = SaveDataToFile( options.outFileSpecRef,
									outBuffer, outBufferSize,
									options.appendOutput );
					}
					else if( IsntNull( options.outBuffer ) )
					{
						if( outBufferSize <= options.maxOutBufferSize )
						{
							pgpCopyMemory( outBuffer, options.outBuffer,
									outBufferSize );
									
							*options.outBufferSize = outBufferSize;
						}
						else
						{
							err = kPGPError_BufferTooSmall;
						}
					}
					else if( IsntNull( options.allocatedOutBuffer ) )
					{
						if( outBufferSize <= options.maxOutBufferSize )
						{
							*options.allocatedOutBuffer = outBuffer;
							*options.outBufferSize		= outBufferSize;
							
							freeOutputBuffer = FALSE;
						}
						else
						{
							err = kPGPError_BufferTooSmall;
						}
					}
					else
					{
						/* Discard output */
					}
				}
				
				if( freeOutputBuffer && IsntNull( outBuffer ) )
					PGPFreeData( outBuffer );
			}
		}
	}
	
	return( err );
}

	PGPError
PGPSendCertificateRequest( 
	PGPKeyServerRef 	inKeyServerRef,
	PGPOptionListRef	firstOption,
	... )
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;

	pgpAssert( PGPKeyServerRefIsValid( inKeyServerRef ) );
		
	if( PGPKeyServerRefIsValid( inKeyServerRef )  )
	{
		PGPOptionListRef	optionList;
		PGPContextRef		context;
		
		context = PGPGetKeyServerContext( inKeyServerRef );
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context, FALSE, firstOption, args);
		va_end( args );
	
		err = pgpSendCertificateRequest( context, inKeyServerRef, optionList );
	
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

typedef struct RetrieveCertificateOptions
{
	PGPKeyRef		caKey;
	PFLFileSpecRef	outFileSpecRef;
		
	void			*outBuffer;
	PGPSize			maxOutBufferSize;
	PGPSize			*outBufferSize;
	
	void			**allocatedOutBuffer;
	
	PGPBoolean		appendOutput;
	
	PGPFilterRef	searchFilter;
	PGPKeyRef		searchKey;
	PGPKeySetRef	keySet;
	
	PGPKeyRef		signingKey;
	PGPByte *		passPhrase;
	PGPSize			passLength;
	PGPBoolean		isPassphrase;
} RetrieveCertificateOptions;


	static PGPError
GatherRetrieveCertOptions(
	PGPOptionListRef			optionList,
	RetrieveCertificateOptions	*options)
{
	PGPError	err 				= kPGPError_NoErr;
	PGPUInt32	numOutputOptions	= 0;
	PGPInt32	tempInt;
	
	pgpAssert( pgpOptionListIsValid( optionList ) );
	pgpAssert( IsntNull( options ) );
	
	pgpClearMemory( options, sizeof( *options ) );
	
	/* Find ca key */
	if ( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_KeyServerCAKey, FALSE, "%p",
					&options->caKey);
	}
	
	/* Find ca keyset */
	if ( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_KeySetRef, FALSE, "%p",
					&options->keySet);
	}

	/* Find search key */
	if ( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_KeyServerSearchKey, FALSE, "%p",
					&options->searchKey);
	}
	
	/* Find search filter */
	if ( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_KeyServerSearchFilter, FALSE, "%p",
					&options->searchFilter);
	}
	
	/* Find signing key and passphrase*/
	if ( IsntPGPError( err ) )
	{
		PGPOption	signOp;
		
		err = pgpSearchOptionSingle(optionList, kPGPOptionType_SignWithKey, &signOp);
		if (IsntPGPError(err) && IsOp(signOp)) {
			err = pgpOptionPtr(&signOp, (void **) &options->signingKey);
			if (IsntPGPError(err)) {
				err = pgpFindOptionArgs(signOp.subOptions, kPGPOptionType_Passphrase, FALSE,
						"%p%l", &options->passPhrase, &options->passLength);
			}
			if (IsntPGPError(err) && IsntNull(options->passPhrase)) {
				options->isPassphrase = true;
			} else if (IsntPGPError(err)) {
				err = pgpFindOptionArgs(signOp.subOptions, kPGPOptionType_Passkey, FALSE,
						"%p%l", &options->passPhrase, &options->passLength);
				options->isPassphrase = false;
			}
		}
	}

	/* Find outputs */
	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_OutputFileRef, FALSE, "%f",
					&options->outFileSpecRef );
		if( PFLFileSpecRefIsValid( options->outFileSpecRef ) )
		{
			++numOutputOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPOOutputBufferDesc	*desc;
		
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_OutputBuffer, FALSE, "%p", &desc );
		if( IsntNull( options->outBuffer ) )
		{
			options->outBuffer 			= desc->buffer;
			options->maxOutBufferSize 	= desc->bufferSize;
			options->outBufferSize 		= desc->outputDataLength;
			
			*(options->outBufferSize) = 0;

			++numOutputOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		PGPOAllocatedOutputBufferDesc	*desc;
		
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_OutputAllocatedBuffer, FALSE, "%p", &desc );
		if( IsntNull( desc ) )
		{
			options->allocatedOutBuffer = desc->buffer;
			options->maxOutBufferSize 	= desc->maximumBufferSize;
			options->outBufferSize 		= desc->actualBufferSize;
			
			*(options->allocatedOutBuffer) 	= 0;
			*(options->outBufferSize) 		= 0;
			
			++numOutputOptions;
		}
	}

	if( IsntPGPError( err ) )
	{
		err = pgpFindOptionArgs( optionList,
					kPGPOptionType_AppendOutput, FALSE, "%d", &tempInt );
		if( tempInt != 0 )
		{
			options->appendOutput = TRUE;
			
			++numOutputOptions;
		}
	}

	/* Check for at most one output */
	
	if( IsntPGPError( err ) && 
		( numOutputOptions > 1 ) )
	{
		pgpDebugMsg( "Invalid input or output specification" );
		err = kPGPError_BadParams;
	}
	
	return err;
}

static const PGPOptionType sRetrieveCertOptionSet[] =
{
	kPGPOptionType_KeyServerCAKey,
	kPGPOptionType_KeyServerSearchKey,
	kPGPOptionType_KeyServerSearchFilter,
	kPGPOptionType_SignWithKey,
	kPGPOptionType_OutputFileRef,
	kPGPOptionType_OutputBuffer,
	kPGPOptionType_OutputAllocatedBuffer,
	kPGPOptionType_AppendOutput,
	kPGPOptionType_DiscardOutput,
	kPGPOptionType_KeySetRef
};

	static PGPError
pgpRetrieveCertificateInternal(
	PGPKeyServerRef 	keyServerRef,
	PGPFilterRef		searchFilter,
	PGPKeyRef			searchKey,
	PGPKeyRef			signingKey,
	PGPByte *			passPhrase,
	PGPSize				passLength,
	PGPBoolean			isPassphrase,
	void				**outBuffer,
	PGPSize				*outBufferSize)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		*outBuffer 		= 0;
		*outBufferSize	= 0;
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) keyServerRef);
		
		((CKeyServer *) keyServerRef)->RetrieveCertificate(	searchFilter,
															searchKey,
															signingKey,
															passPhrase,
															passLength,
															isPassphrase,
															outBuffer,
															outBufferSize);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}

	static PGPError
pgpRetrieveCertificate(
	PGPContextRef		context,
	PGPKeyServerRef 	keyServerRef,
	PGPOptionListRef	optionList)
{
	PGPError		err;
	
	(void) context;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( PGPKeyServerRefIsValid( keyServerRef ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpCheckOptionsInSet( optionList, sRetrieveCertOptionSet,
					elemsof( sRetrieveCertOptionSet ) );
		if( IsntPGPError( err ) )
		{
			RetrieveCertificateOptions	options;
			
			err = GatherRetrieveCertOptions( optionList, &options );
			if( IsntPGPError( err ) )
			{
				void		*outBuffer 			= 0;
				PGPSize		outBufferSize		= 0;
				PGPBoolean	freeOutputBuffer	= TRUE;
								
				err = pgpRetrieveCertificateInternal( keyServerRef, options.searchFilter,
							options.searchKey, options.signingKey, options.passPhrase,
							options.passLength, options.isPassphrase, &outBuffer, &outBufferSize );
			
				if( IsntPGPError( err ) )
				{
					if( PFLFileSpecRefIsValid( options.outFileSpecRef ) )
					{
						err = SaveDataToFile( options.outFileSpecRef,
									outBuffer, outBufferSize,
									options.appendOutput );
					}
					else if( IsntNull( options.outBuffer ) )
					{
						if( outBufferSize <= options.maxOutBufferSize )
						{
							pgpCopyMemory( outBuffer, options.outBuffer,
									outBufferSize );
									
							*options.outBufferSize = outBufferSize;
						}
						else
						{
							err = kPGPError_BufferTooSmall;
						}
					}
					else if( IsntNull( options.allocatedOutBuffer ) )
					{
						if( outBufferSize <= options.maxOutBufferSize )
						{
							*options.allocatedOutBuffer = outBuffer;
							*options.outBufferSize		= outBufferSize;
							
							freeOutputBuffer = FALSE;
						}
						else
						{
							err = kPGPError_BufferTooSmall;
						}
					}
					else
					{
						/* Discard output */
					}
				}
				
				if( freeOutputBuffer && IsntNull( outBuffer ) )
					PGPFreeData( outBuffer );
			}
		}
	}
	
	return( err );
}

	PGPError
PGPRetrieveCertificate( 
	PGPKeyServerRef 	inKeyServerRef,
	PGPOptionListRef	firstOption,
	... )
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;

	pgpAssert( PGPKeyServerRefIsValid( inKeyServerRef ) );
		
	if( PGPKeyServerRefIsValid( inKeyServerRef ) )
	{
		PGPOptionListRef	optionList;
		PGPContextRef		context;
		
		context = PGPGetKeyServerContext( inKeyServerRef );
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context, FALSE, firstOption, args);
		va_end( args );
	
		err = pgpRetrieveCertificate( context, inKeyServerRef, optionList );
	
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
pgpRetrieveCRLInternal(
	PGPKeyServerRef 	keyServerRef,
	PGPKeyRef			certifyingKey,
	PGPKeySetRef		searchKeySet,
	PGPKeyRef			signingKey,
	PGPByte *			passPhrase,
	PGPSize				passLength,
	PGPBoolean			isPassphrase,
	void				**outBuffer,
	PGPSize				*outBufferSize)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		*outBuffer 		= 0;
		*outBufferSize	= 0;
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) keyServerRef);
		
		((CKeyServer *) keyServerRef)->RetrieveCRL(	certifyingKey,
													searchKeySet,
													signingKey,
													passPhrase,
													passLength,
													isPassphrase,
													outBuffer,
													outBufferSize);
	}
	
	catch (PGPError exception) {
		result = MapErrors(exception);
	}
	
#if !PGP_WIN32
	catch (...) {
		result = kPGPError_UnknownError;
	}
#endif
	
	return result;
}

	static PGPError
pgpRetrieveCRL(
	PGPContextRef		context,
	PGPKeyServerRef 	keyServerRef,
	PGPOptionListRef	optionList)
{
	PGPError		err;
	
	(void) context;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( PGPKeyServerRefIsValid( keyServerRef ) );
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		/* This call uses the same options as PGPRetrieveCertificate() */
		
		err = pgpCheckOptionsInSet( optionList, sRetrieveCertOptionSet,
					elemsof( sRetrieveCertOptionSet ) );
		if( IsntPGPError( err ) )
		{
			RetrieveCertificateOptions	options;
			
			err = GatherRetrieveCertOptions( optionList, &options );
			if( IsntPGPError( err ) )
			{
				void		*outBuffer 			= 0;
				PGPSize		outBufferSize		= 0;
				PGPBoolean	freeOutputBuffer	= TRUE;
								
				err = pgpRetrieveCRLInternal( keyServerRef,
							options.caKey, options.keySet, options.signingKey, options.passPhrase,
							options.passLength, options.isPassphrase, &outBuffer, &outBufferSize );
			
				if( IsntPGPError( err ) )
				{
					if( PFLFileSpecRefIsValid( options.outFileSpecRef ) )
					{
						err = SaveDataToFile( options.outFileSpecRef,
									outBuffer, outBufferSize,
									options.appendOutput );
					}
					else if( IsntNull( options.outBuffer ) )
					{
						if( outBufferSize <= options.maxOutBufferSize )
						{
							pgpCopyMemory( outBuffer, options.outBuffer,
									outBufferSize );
									
							*options.outBufferSize = outBufferSize;
						}
						else
						{
							err = kPGPError_BufferTooSmall;
						}
					}
					else if( IsntNull( options.allocatedOutBuffer ) )
					{
						if( outBufferSize <= options.maxOutBufferSize )
						{
							*options.allocatedOutBuffer = outBuffer;
							*options.outBufferSize		= outBufferSize;
							
							freeOutputBuffer = FALSE;
						}
						else
						{
							err = kPGPError_BufferTooSmall;
						}
					}
					else
					{
						/* Discard output */
					}
				}
				
				if( freeOutputBuffer && IsntNull( outBuffer ) )
					PGPFreeData( outBuffer );
			}
		}
	}
	
	return( err );
}

	PGPError
PGPRetrieveCertificateRevocationList( 
	PGPKeyServerRef 	inKeyServerRef,
	PGPOptionListRef	firstOption,
	... )
{
	PGPError	err = kPGPError_NoErr;
	va_list		args;

	pgpAssert( PGPKeyServerRefIsValid( inKeyServerRef ) );
		
	if( PGPKeyServerRefIsValid( inKeyServerRef ) )
	{
		PGPOptionListRef	optionList;
		PGPContextRef		context;
		
		context = PGPGetKeyServerContext( inKeyServerRef );
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context, FALSE, firstOption, args);
		va_end( args );
	
		err = pgpRetrieveCRL( context, inKeyServerRef, optionList );
	
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

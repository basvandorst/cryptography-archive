/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: pgpKeyServer.cpp,v 1.20.10.1 1998/11/12 03:22:19 heller Exp $
____________________________________________________________________________*/

#include "pgpContext.h"


#include "pgpSockets.h"

#include "CKeyServer.h"
#include "pgpKeyServer.h"

#if PGP_WIN32
#include "PGPsdkNetworkLibDLLMain.h"
#endif

#if PGP_WIN32 || PGP_UNIX
#include "pgpRMWOLock.h"

static PGPRMWOLock			sBusyLock;
static PGPRMWOLock			sCancelLock;
#endif

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
		// Close locks for error -wjb
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
		if (outPreviousStorage == NULL) {
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
	PGPKeyServerType		inType,
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
		PGPValidateParam((inType >= kPGPKeyServerType_LDAP) &&
			(inType <= kPGPKeyServerType_HTTPS));
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
	PGPKeyServerType		inType,
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
		PGPValidateParam((inType >= kPGPKeyServerType_LDAP) &&
			(inType <= kPGPKeyServerType_HTTPS));
		PGPValidateParam((inAccessType >= kPGPKeyServerAccessType_Default) &&
			(inAccessType <= kPGPKeyServerAccessType_Administrator));
		PGPValidateParam((inKeySpace >= kPGPKeyServerKeySpace_Default) &&
			(inKeySpace <= kPGPKeyServerKeySpace_Pending));
	
		*outKeyServerRef =
				(PGPKeyServerRef) CKeyServer::NewKeyServerFromHostName(
															inContext,
															inHostName,
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
		*outCallback = NULL;
		PGPValidatePtr(outUserData)
		*outUserData = NULL;
	
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
		*outTLSSessionRef = NULL;
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



	PGPError
PGPGetKeyServerType(
	PGPKeyServerRef		inKeyServerRef,
	PGPKeyServerType *	outType)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outType);
		PGPValidatePtr(inKeyServerRef);
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);

		((CKeyServer *) inKeyServerRef)->GetType(*outType);
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
		*outPort = NULL;
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
		*outHostName = NULL;
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
		*outAddress = NULL;
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
PGPGetLastKeyServerErrorString(
	PGPKeyServerRef	inKeyServerRef,
	char **			outErrorString)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outErrorString);
		*outErrorString = NULL;
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
		*outFoundKeys = NULL;
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
		*outKeysThatFailed = NULL;
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
PGPUploadPrivateKeysToKeyServer(
	PGPKeyServerRef	inKeyServerRef, 
	PGPKeySetRef	inKeysToUpload, 
	PGPKeySetRef *	outKeysThatFailed)
{
	PGPError		result = kPGPError_NoErr;
	
	try {
		// Validation
		PGPValidatePtr(outKeysThatFailed);
		*outKeysThatFailed = NULL;
		PGPValidatePtr(inKeyServerRef);
		PGPValidateParam(PGPKeySetRefIsValid(inKeysToUpload));
	
		StKeyServerBusy	busyKeyServer((CKeyServer *) inKeyServerRef);
		
		((CKeyServer *) inKeyServerRef)->Upload(	kPGPPrivateKeyAllowed,
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
		*outKeysThatFailed = NULL;
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
		*outKeysThatFailed = NULL;
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
		*outGroupSetRef = NULL;
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
		*outMonitor = NULL;
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

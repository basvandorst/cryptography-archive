/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: CKeyServer.cpp,v 1.36.10.1 1998/11/12 03:22:16 heller Exp $
____________________________________________________________________________*/

#if PGP_MACINTOSH
#include <Threads.h>
#include <map>

#elif PGP_WIN32
#include <windows.h>
#endif

#if PGP_WIN32 || PGP_UNIX
#include "pgpRMWOLock.h"
#endif

#include <string.h>

#include "pgpStrings.h"
#include "pgpMemoryMgr.h"
#include "pgpEncode.h"
#include "pgpContext.h"
#include "pgpMem.h"

#include "pgpSockets.h"

#include "CLDAPKeyServer.h"
#include "CHTTPKeyServer.h"
#include "CKeyServer.h"


PGPBoolean				CKeyServer::sIsInitialized = FALSE;

struct SThreadContext {
	PGPEventHandlerProcPtr	idleEventHandler;
	PGPUserValue			idleEventHandlerData;
};


#if PGP_WIN32
PGPRMWOLock				CKeyServer::sThreadLock;
PGPInt32				CKeyServer::sIdleEventHandlerIndex = 0;
PGPInt32				CKeyServer::sIdleEventHandlerDataIndex = 0;
#endif

#if PGP_MACINTOSH
static map<ThreadID, SThreadContext>	sThreadContextMap;
extern ThreadID							gMainThreadID;
#endif

static const PGPUInt16			kPGPKeyServerPortLDAP = 389;
static const PGPUInt16			kPGPKeyServerPortLDAPS = 636;
static const PGPUInt16			kPGPKeyServerPortHTTP = 11371;
static const PGPUInt16			kPGPKeyServerPortHTTPS = 443;
static const char *				kProtocolSeparator = "://";
static const char *				kProtocolTag[] = {	"LDAPS:", "HTTPS:",
													"LDAP:", "HTTP:",
													NULL};
static const PGPKeyServerType	kProtocolType[] = {	kPGPKeyServerType_LDAPS,
													kPGPKeyServerType_HTTPS,
													kPGPKeyServerType_LDAP,
													kPGPKeyServerType_HTTP};


#if PGP_UNIX	// No need to save the idle event handler
StPreserveSocketsEventHandler::StPreserveSocketsEventHandler(
	CKeyServer * inKeyServer)
{
}



StPreserveSocketsEventHandler::~StPreserveSocketsEventHandler()
{
}

#else
StPreserveSocketsEventHandler::StPreserveSocketsEventHandler(
	CKeyServer * inKeyServer)
{
	PGPError				pgpError = kPGPError_NoErr;
	PGPEventHandlerProcPtr	eventHandler;
	PGPUserValue			eventHandlerData;
	
	CKeyServer::GetThreadIdleEventHandler(&eventHandler, &eventHandlerData);
	
	if (eventHandler != NULL) {
		pgpError = PGPSetSocketsIdleEventHandler(
										CKeyServer::KeyServerIdleEventHandler,
										inKeyServer);
	}
	
}



StPreserveSocketsEventHandler::~StPreserveSocketsEventHandler()
{
}
#endif	// idle event handler restoration

	void
CKeyServer::CheckInitialization()
{
	if (! sIsInitialized) {
		ThrowPGPError_(kPGPError_ServerNotInitialized);
	}
}

	void
CKeyServer::Initialize()
{
	PGPError	pgpError;
	
#if PGP_WIN32
	InitializePGPRMWOLock(&sThreadLock);
	sIdleEventHandlerIndex = TlsAlloc();
	if (! TlsSetValue(sIdleEventHandlerIndex, NULL)) {
		ThrowPGPError_(kPGPError_UnknownError);
	}
	sIdleEventHandlerDataIndex = TlsAlloc();
	if (! TlsSetValue(sIdleEventHandlerDataIndex, NULL)) {
		ThrowPGPError_(kPGPError_UnknownError);
	}
#endif

	pgpError = PGPSocketsInit();
	ThrowIfPGPError_(pgpError);
	
	sIsInitialized = TRUE;
}



	void
CKeyServer::Cleanup()
{
#if PGP_WIN32
	TlsFree(sIdleEventHandlerIndex);
	sIdleEventHandlerIndex = 0;
	TlsFree(sIdleEventHandlerDataIndex);
	sIdleEventHandlerDataIndex = 0;
	DeletePGPRMWOLock(&sThreadLock);
#endif

#if PGP_MACINTOSH
	sThreadContextMap.erase(	sThreadContextMap.begin(),
								sThreadContextMap.end());
#endif

	PGPSocketsCleanup();
	sIsInitialized = FALSE;
}
	


	CKeyServer *
CKeyServer::NewKeyServerFromURL(
	PGPContextRef			inContext,
	const char *			inURL,
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace)
{
	CKeyServer *					result = NULL;
	char *							url = NULL;
	
	CheckInitialization();
	
	try {
		PGPKeyServerType	keyServerType = (PGPKeyServerType) 0;
		char *				host;
		PGPUInt16			port;
		char *				currentItem = NULL;
		char *				nextItem = NULL;
		
		url = new char[strlen(inURL) + 1];
		if (url == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		strcpy(url, inURL);
		
		// Parse the URL
		currentItem = url;
		nextItem = strchr(currentItem, ':');
		
		// Determine the protocol
		if (nextItem != NULL) {
			// Determine if we are at the port separator or protocol
			// separator
			if (pgpCompareStringsIgnoreCaseN(nextItem, kProtocolSeparator,
			strlen(kProtocolSeparator)) == 0) {
				// Determine protocol
				for (PGPUInt16 i = 0; kProtocolTag[i] != NULL; i++) {
					if (pgpCompareStringsIgnoreCaseN(currentItem,
					kProtocolTag[i], strlen(kProtocolTag[i])) == 0) {
						keyServerType = kProtocolType[i];
						break;
					}
				}
				if (keyServerType == 0) {
					ThrowPGPError_(kPGPError_ServerInvalidProtocol);
				} else {
					nextItem += strlen(kProtocolSeparator);
				}
			} else {
				// Default to HTTP, colon must be for port
				keyServerType = kPGPKeyServerType_HTTP;
				nextItem = url;
			}
		} else {
			// Default to HTTP
			keyServerType = kPGPKeyServerType_HTTP;
			nextItem = url;
		}
		
		// Determine the host name and port
		host = currentItem = nextItem;
		nextItem = strchr(currentItem, ':');
		if (nextItem != NULL) {
			// Found port
			port = atoi(nextItem + 1);
			*nextItem = '\0';
		} else {
			switch (keyServerType) {
				case kPGPKeyServerType_LDAP:
				{
					port = kPGPKeyServerPortLDAP;
				}
				break;
				
				
				case kPGPKeyServerType_LDAPS:
				{
					port = kPGPKeyServerPortLDAPS;
				}
				break;
				
				
				case kPGPKeyServerType_HTTP:
				{
					port = kPGPKeyServerPortHTTP;
				}
				break;
				
				
				case kPGPKeyServerType_HTTPS:
				{
					port = kPGPKeyServerPortHTTPS;
				}
				break;
				
				default:
				{
					pgpDebugMsg("NewKeyServerFromURL() Unknown server type");
					port = 0;
				}
				break;
			}
		}
		
		// Trim paths
		nextItem = strchr(currentItem, '/');
		if (nextItem != NULL) {
			*nextItem = '\0';
		}

		result = NewKeyServerFromHostName(	inContext,
											host,
											port,
											keyServerType,
											inAccessType,
											inKeySpace);
	}
	
	catch (...) {
		delete[] url;
		throw;
	}
	
	delete[] url;
	
	return result;
}



	CKeyServer *
CKeyServer::NewKeyServerFromHostAddress(
	PGPContextRef			inContext,
	PGPUInt32				inHostAddress,
	PGPUInt16				inHostPort,
	PGPKeyServerType		inType,
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace)
{
	CKeyServer *	result = NULL;

	CheckInitialization();
	
	try {
		if (inHostPort == 0) {
			switch (inType) {
				case kPGPKeyServerType_LDAP:
				{
					inHostPort = kPGPKeyServerPortLDAP;
				}
				break;
				
				
				case kPGPKeyServerType_LDAPS:
				{
					inHostPort = kPGPKeyServerPortLDAPS;
				}
				break;
				
				
				case kPGPKeyServerType_HTTP:
				{
					inHostPort = kPGPKeyServerPortHTTP;
				}
				break;
				
				
				case kPGPKeyServerType_HTTPS:
				{
					inHostPort = kPGPKeyServerPortHTTPS;
				}
				break;
				
				default:
				{
					pgpDebugMsg(
					"NewKeyServerFromHostAddress(): Unknown server type" );
					inHostPort = 0;
				}
				break;
			}
		}
		if ((inType == kPGPKeyServerType_LDAP)
		|| (inType == kPGPKeyServerType_LDAPS)) {
			result = new CLDAPKeyServer(	inContext,
											NULL,
											inHostAddress,
											inHostPort,
											inType,
											inAccessType,
											inKeySpace);
		} else {
			result = new CHTTPKeyServer(	inContext,
											NULL,
											inHostAddress,
											inHostPort,
											inType);
		}
		if (result == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
	}
	
	catch (...) {
		delete result;
		throw;
	}
	
	return result;
}



	CKeyServer *
CKeyServer::NewKeyServerFromHostName(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt16				inHostPort,
	PGPKeyServerType		inType,
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace)
{
	CKeyServer *	result = NULL;

	CheckInitialization();
	
	try {
		if (inHostPort == 0) {
			switch (inType) {
				case kPGPKeyServerType_LDAP:
				{
					inHostPort = kPGPKeyServerPortLDAP;
				}
				break;
				
				
				case kPGPKeyServerType_LDAPS:
				{
					inHostPort = kPGPKeyServerPortLDAPS;
				}
				break;
				
				
				case kPGPKeyServerType_HTTP:
				{
					inHostPort = kPGPKeyServerPortHTTP;
				}
				break;
				
				
				case kPGPKeyServerType_HTTPS:
				{
					inHostPort = kPGPKeyServerPortHTTPS;
				}
				break;
				
				default:
				{
					pgpDebugMsg(
					"NewKeyServerFromHostName(): Unknown server type" );
					inHostPort = 0;
				}
				break;
			}
		}
		if ((inType == kPGPKeyServerType_LDAP)
		|| (inType == kPGPKeyServerType_LDAPS)) {
			result = new CLDAPKeyServer(	inContext,
											inHostName,
											0,
											inHostPort,
											inType,
											inAccessType,
											inKeySpace);
		} else {
			result = new CHTTPKeyServer(	inContext,
											inHostName,
											0,
											inHostPort,
											inType);
		}
		if (result == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
	}
	
	catch (...) {
		delete result;
		throw;
	}
	
	return result;
}



	PGPBoolean
CKeyServer::Free()
{
	PGPBoolean	freedServer = FALSE;
	
	mRefCount--;
	
	if (mRefCount == 0) {
		if (mIsOpen) {
			ThrowPGPError_(kPGPError_ServerOpen);
		}

		delete this;
		freedServer = TRUE;
	}
	
	return( freedServer );
}


CKeyServer::CKeyServer(
	PGPContextRef		inContext,
	const char *		inHostName,
	PGPUInt32			inHostAddress,
	PGPUInt16			inHostPort,
	PGPKeyServerType	inType)
	: mContext(inContext), mHostName(NULL), mHostPort(inHostPort),
	  mHostAddress(inHostAddress), mType(inType), mEventHandler(NULL),
	  mEventHandlerData(NULL),  mTLSSession(kInvalidPGPtlsSessionRef),
	  mErrorString(NULL), mSecured(false), mIsBusy(false), mIsOpen(false),
	  mCanceled(false), mRefCount(1)
#if PGP_WIN32
	  , mAsyncHostEntryRef(kPGPInvalidAsyncHostEntryRef)
#endif
{
	if (inHostName != NULL) {
		mHostName = new char[strlen(inHostName) + 1];
		if (mHostName == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		strcpy(mHostName, inHostName);
	}
}



CKeyServer::~CKeyServer()
{
	delete[] mHostName;
	delete[] mErrorString;
}



	void
CKeyServer::SetIdleEventHandler(
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue			inUserData)
{
	SetThreadIdleEventHandler(inCallback, inUserData);
}



	void
CKeyServer::GetIdleEventHandler(
	PGPEventHandlerProcPtr *	outCallback,
	PGPUserValue *				outUserData)
{
	GetThreadIdleEventHandler(outCallback, outUserData);
}



	PGPError
CKeyServer::KeyServerIdleEventHandler(
	PGPContextRef	inContext,
	PGPEvent *		inEvent,
	PGPUserValue	callBackArg)
{
	PGPEventHandlerProcPtr	eventHandler;
	PGPUserValue			eventHandlerData;
	PGPError				result;
	
	(void) inContext;
	(void) inEvent;
	
	GetThreadIdleEventHandler(&eventHandler, &eventHandlerData);
	
	if (eventHandler != NULL) {
		PGPContextRef	context = NULL;
		PGPEvent		theEvent;
		
		pgpClearMemory(&theEvent, sizeof(theEvent));
		theEvent.type = kPGPEvent_KeyServerIdleEvent;
		theEvent.job = NULL;
		theEvent.data.keyServerIdleData.keyServerRef =
									(PGPKeyServerRef) callBackArg;
		if (callBackArg != NULL) {
			context = ((CKeyServer *) callBackArg)->mContext;
		}
		result = eventHandler(context, &theEvent, eventHandlerData);
		if (callBackArg != NULL) {
			if (result != kPGPError_NoErr) {
				((CKeyServer *) callBackArg)->mCanceled = true;
			} else if (((CKeyServer *) callBackArg)->mCanceled) {
				result = kPGPError_UserAbort;
			}
		}
	} else {
		result = kPGPError_NoErr;
	}
	
	return result;
}



	void
CKeyServer::Open(
	PGPtlsSessionRef	inTLSSession)
{
	if (mIsOpen) {
		ThrowPGPError_(kPGPError_ServerOpen);
	}
	mTLSSession = inTLSSession;
	InitializeHostNameAndAddress();
	mIsOpen = true;
}



	void
CKeyServer::Close()
{
	mTLSSession = kInvalidPGPtlsSessionRef;
	mIsOpen = false;
	mSecured = false;
	mCanceled = false;
}



	void
CKeyServer::Cancel()
{
	StPreserveSocketsEventHandler	preserve(this);

	mCanceled = true;
#if PGP_WIN32
	if (PGPAsyncHostEntryRefIsValid(mAsyncHostEntryRef)) {
		PGPCancelAsyncHostEntryRef(mAsyncHostEntryRef);
		mAsyncHostEntryRef = kPGPInvalidAsyncHostEntryRef;
	}
#endif
}



	void
CKeyServer::GetHostName(
	char **	outHostName)
{
	InitializeHostNameAndAddress();
	*outHostName = (char *) PGPNewData(	PGPGetContextMemoryMgr(mContext),
										strlen(mHostName) + 1,
										kPGPMemoryMgrFlags_None);
	if (*outHostName == NULL) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	strcpy(*outHostName, mHostName);
}



	void
CKeyServer::GetAddress(
	PGPUInt32 &	outAddress)
{
	InitializeHostNameAndAddress();
	outAddress = mHostAddress;
}



	void
CKeyServer::GetErrorString(
	char **	outErrorString)
{
	if (mErrorString != NULL) {
		*outErrorString = (char *) PGPNewData(
										PGPGetContextMemoryMgr(mContext),
										strlen(mErrorString) + 1,
										kPGPMemoryMgrFlags_None);
		if (*outErrorString == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		strcpy(*outErrorString, mErrorString);
	}
}



	void
CKeyServer::SetErrorString(
	const char *	inErrorString)
{
	delete[] mErrorString;
	
	if (inErrorString == NULL) {
		mErrorString = NULL;
	} else {
		mErrorString = new char[strlen(inErrorString) + 1];
		if (mErrorString != NULL) {
			strcpy(mErrorString, inErrorString);
		}
	}
}


	void
CKeyServer::InitializeHostNameAndAddress()
{
	StPreserveSocketsEventHandler	preserve(this);
	PGPHostEntry *					hostEntry = NULL;
	
	if (mHostName != NULL) {
		PGPUInt32	tempAddress = PGPDottedToInternetAddress(mHostName);
		
		if (tempAddress != kPGPSockets_Error) {
			mHostAddress = PGPNetToHostLong(tempAddress);
			delete mHostName;
			mHostName = NULL;
		}
	}
	
#if PGP_WIN32
		PGPError				err;
		char					h_name[256];
#endif
		
	if (mHostName == NULL) {
		PGPInternetAddress	theAddress;
		char *				theHostName;
		
		theAddress.s_addr = PGPHostToNetLong(mHostAddress);
#if PGP_WIN32
		err = PGPStartAsyncGetHostByAddress(theAddress.s_addr,
				&mAsyncHostEntryRef);
		ThrowIfPGPError_(err);
		err = PGPWaitForGetHostByAddress(mAsyncHostEntryRef,
				h_name, 256);
		mAsyncHostEntryRef = kPGPInvalidAsyncHostEntryRef;
		if (IsPGPError(err)) {
#else
		hostEntry = PGPGetHostByAddress((const char *) &theAddress,
										sizeof(PGPInternetAddress),
										kPGPProtocolFamilyInternet);
		if (hostEntry == NULL) {
#endif
			PGPInternetAddress	address;
			
			address.s_addr = PGPHostToNetLong(mHostAddress);
			theHostName = PGPInternetAddressToDottedString(address);
			if (theHostName == NULL) {
				ThrowPGPError_(kPGPError_UnknownError);
			}
		} else {
#if PGP_WIN32
			theHostName = h_name;
#else
			theHostName = hostEntry->h_name;
#endif
		}
		mHostName = new char[strlen(theHostName) + 1];
		if (mHostName == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		strcpy(mHostName, theHostName);
	} else if (mHostAddress == 0) {
#if PGP_WIN32
		err = PGPStartAsyncGetHostByName(mHostName, &mAsyncHostEntryRef);
		ThrowIfPGPError_(err);
		err = PGPWaitForGetHostByName(mAsyncHostEntryRef, &mHostAddress);
		mAsyncHostEntryRef = kPGPInvalidAsyncHostEntryRef;
		ThrowIfPGPError_(err);
		mHostAddress = PGPNetToHostLong(mHostAddress);
#else
		hostEntry = PGPGetHostByName(mHostName);
		if (hostEntry == NULL) {
			ThrowPGPError_(PGPGetLastSocketsError());
		}
		mHostAddress = PGPNetToHostLong(
							*((PGPUInt32 *)hostEntry->h_addr));
#endif
	}
}



#if PGP_MACINTOSH
	void
CKeyServer::CreateThreadStorage(
	SThreadContext **	outContext)
{
	OSStatus									err;
	ThreadID									theThreadID;
	map<ThreadID, SThreadContext>::iterator		theIterator;
	SThreadContext *							theResult = nil;

	try {
		err = ::GetCurrentThread(&theThreadID);
		if (err != noErr) {
			ThrowPGPError_(err);
		}
		theIterator = sThreadContextMap.find(theThreadID);
		if (theIterator != sThreadContextMap.end()) {
			theResult = new SThreadContext;
			if (theResult == NULL) {
				ThrowPGPError_(kPGPError_OutOfMemory);
			}
			*theResult = (*theIterator).second;
		} else {
			SThreadContext	tempContext = {0, };

			pgpLeaksSuspend();
			sThreadContextMap[theThreadID] = tempContext;
			pgpLeaksResume();
			
			theIterator = sThreadContextMap.find(theThreadID);
			if (theIterator == sThreadContextMap.end()) {
				ThrowPGPError_(kPGPError_OutOfMemory);
			}
		}
		(*theIterator).second.idleEventHandler = NULL;
		(*theIterator).second.idleEventHandlerData = NULL;
	}
	
	catch (...) {
		delete theResult;
		throw;
	}
	
	*outContext = theResult;
}



	void
CKeyServer::DisposeThreadStorage(
	SThreadContext *	inContext)
{
	ThreadID	theThreadID;
	OSStatus	err;									
	
	err = ::GetCurrentThread(&theThreadID);
	if (err != noErr) {
		ThrowPGPError_(err);
	}
	if (inContext != NULL) {
		sThreadContextMap[theThreadID] = *inContext;
		delete inContext;
	} else {
		map<ThreadID, SThreadContext>::iterator	theIterator;

		theIterator = sThreadContextMap.find(theThreadID);
		if (theIterator != sThreadContextMap.end()) {
			sThreadContextMap.erase(theThreadID);
		}
	}
		
}
	static SThreadContext *
GetThreadContext()
{
	OSStatus									err;
	ThreadID									theThreadID;
	map<ThreadID, SThreadContext>::iterator		theIterator;
	SThreadContext *							theResult = nil;
	
	try {
		err = ::GetCurrentThread(&theThreadID);
		if (err != noErr) {
			ThrowPGPError_(err);
		}
		theIterator = sThreadContextMap.find(theThreadID);
		if (theIterator == sThreadContextMap.end()) {
			ThrowPGPError_(kPGPError_SocketsNoStaticStorage);
		}

		theResult = &(*theIterator).second;
	}
	
	catch (...) {
	}
	
	return theResult;
}



	void
CKeyServer::GetThreadIdleEventHandler(
	PGPEventHandlerProcPtr *	outCallback,
	PGPUserValue *				outUserData)
{
	SThreadContext *	theContext = GetThreadContext();
	
	if (theContext != NULL) {
		*outCallback = theContext->idleEventHandler;
		*outUserData = theContext->idleEventHandlerData;
	} else {
		*outCallback = NULL;
		*outUserData = NULL;
	}
}



	void
CKeyServer::SetThreadIdleEventHandler(
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue			inUserData)
{
	SThreadContext *	theContext = GetThreadContext();
	
	if (theContext != NULL) {
		theContext->idleEventHandler = inCallback;
		theContext->idleEventHandlerData = inUserData;
	}
}



#elif PGP_WIN32
	void
CKeyServer::GetThreadIdleEventHandler(
	PGPEventHandlerProcPtr *	outCallback,
	PGPUserValue *				outUserData)
{
	PGPRMWOLockStartReading(&sThreadLock);
	*outCallback = (PGPEventHandlerProcPtr)
					TlsGetValue(sIdleEventHandlerIndex);
	*outUserData = (PGPUserValue) TlsGetValue(sIdleEventHandlerDataIndex);
	PGPRMWOLockStopReading(&sThreadLock);
}



	void
CKeyServer::SetThreadIdleEventHandler(
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue			inUserData)
{
	PGPRMWOLockStartWriting(&sThreadLock);
	if (TlsSetValue(sIdleEventHandlerIndex, inCallback)) {
		if (! TlsSetValue(sIdleEventHandlerDataIndex, inUserData)) {
			TlsSetValue(sIdleEventHandlerIndex, NULL);
		}
	}
	PGPRMWOLockStopWriting(&sThreadLock);
}


	void
CKeyServer::CreateThreadStorage(
	SThreadContext **	outContext)
{
	*outContext = new SThreadContext;
	if (*outContext == NULL) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	GetThreadIdleEventHandler(	&(*outContext)->idleEventHandler,
								&(*outContext)->idleEventHandlerData);
}



	void
CKeyServer::DisposeThreadStorage(
	SThreadContext *	inContext)
{
	if (inContext != NULL) {
		SetThreadIdleEventHandler(	inContext->idleEventHandler,
									inContext->idleEventHandlerData);
		delete inContext;
	}
}

#elif PGP_UNIX

	void
CKeyServer::GetThreadIdleEventHandler(
	PGPEventHandlerProcPtr *	outCallback,
	PGPUserValue *				outUserData)
{
	(void) outCallback;
	(void) outUserData;
	
	ThrowPGPError_(kPGPError_ServerOperationNotSupported);
}



	void
CKeyServer::SetThreadIdleEventHandler(
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue			inUserData)
{
	(void) inCallback;
	(void) inUserData;
	
	ThrowPGPError_(kPGPError_ServerOperationNotSupported);
}


		void
CKeyServer::CreateThreadStorage(
	SThreadContext **	outContext)
{
	*outContext = NULL;
}



	void
CKeyServer::DisposeThreadStorage(
	SThreadContext *	inContext)
{
}

#endif
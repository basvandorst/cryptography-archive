/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CKeyServer.cpp,v 1.46.6.2 1999/06/04 01:12:05 heller Exp $
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
#include "CCRSEntrustServer.h"
#include "CCRSVerisignServer.h"
#include "CHTTPPGPKeyServer.h"
#include "CHTTPXcertServer.h"
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
static const PGPUInt16			kPGPKeyServerPortHTTPPGP = 11371;
static const PGPUInt16			kPGPKeyServerPortHTTP = 80;
static const PGPUInt16			kPGPKeyServerPortHTTPS = 443;
static const char *				kProtocolSeparator = "://";
static const char *				kProtocolTag[] = {	"LDAPS:", "HTTPS:",
													"LDAP:", "HTTP:",
													0};
static const PGPKeyServerProtocol	kProtocolType[] = {	kPGPKeyServerProtocol_LDAPS,
													kPGPKeyServerProtocol_HTTPS,
													kPGPKeyServerProtocol_LDAP,
													kPGPKeyServerProtocol_HTTP};


#if PGP_UNIX	// No need to save the idle event handler
StPreserveSocketsEventHandler::StPreserveSocketsEventHandler(
	CKeyServer * inKeyServer)
{
	(void) inKeyServer;
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
	
	if (eventHandler != 0) {
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
	if (! TlsSetValue(sIdleEventHandlerIndex, 0)) {
		ThrowPGPError_(kPGPError_UnknownError);
	}
	sIdleEventHandlerDataIndex = TlsAlloc();
	if (! TlsSetValue(sIdleEventHandlerDataIndex, 0)) {
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
	PGPKeyServerKeySpace	inKeySpace,
	PGPKeyServerClass		inClass)
{
	CKeyServer *					result = 0;
	char *							url = 0;
	
	CheckInitialization();
	
	try {
		PGPKeyServerProtocol	keyServerType = (PGPKeyServerProtocol) 0;
		char *				host;
		PGPUInt16			port;
		char *				currentItem = 0;
		char *				nextItem = 0;
		
		url = new char[strlen(inURL) + 1];
		if (url == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		strcpy(url, inURL);
		
		// Parse the URL
		currentItem = url;
		nextItem = strchr(currentItem, ':');
		
		// Determine the protocol
		if (nextItem != 0) {
			// Determine if we are at the port separator or protocol
			// separator
			if (pgpCompareStringsIgnoreCaseN(nextItem, kProtocolSeparator,
			strlen(kProtocolSeparator)) == 0) {
				// Determine protocol
				for (PGPUInt16 i = 0; kProtocolTag[i] != 0; i++) {
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
				keyServerType = kPGPKeyServerProtocol_HTTP;
				nextItem = url;
			}
		} else {
			// Default to HTTP
			keyServerType = kPGPKeyServerProtocol_HTTP;
			nextItem = url;
		}
		
		// Determine the host name and port
		host = currentItem = nextItem;
		nextItem = strchr(currentItem, ':');
		if (nextItem != 0) {
			// Found port
			currentItem = nextItem + 1;
			port = atoi(currentItem);
			*nextItem = '\0';
		} else {
			switch (keyServerType) {
				case kPGPKeyServerProtocol_LDAP:
				{
					port = kPGPKeyServerPortLDAP;
				}
				break;
				
				
				case kPGPKeyServerProtocol_LDAPS:
				{
					port = kPGPKeyServerPortLDAPS;
				}
				break;
				
				
				case kPGPKeyServerProtocol_HTTP:
				{
					if (inClass == kPGPKeyServerClass_PGP) {
						port = kPGPKeyServerPortHTTPPGP;
					} else {
						port = kPGPKeyServerPortHTTP;
					}
				}
				break;
				
				
				case kPGPKeyServerProtocol_HTTPS:
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
		
		// Find path
		nextItem = strchr(currentItem, '/');
		if (nextItem != 0) {
			currentItem = nextItem + 1;
			*nextItem = '\0';
		} else {
			currentItem = 0;
		}

		result = NewKeyServerFromHostName(	inContext,
											host,
											port,
											currentItem,
											keyServerType,
											inAccessType,
											inKeySpace,
											inClass);
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
	PGPKeyServerProtocol	inType,
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace,
	PGPKeyServerClass		inClass)
{
	CKeyServer *	result = 0;

	CheckInitialization();
	
	try {
		if (inHostPort == 0) {
			switch (inType) {
				case kPGPKeyServerProtocol_LDAP:
				{
					inHostPort = kPGPKeyServerPortLDAP;
				}
				break;
				
				
				case kPGPKeyServerProtocol_LDAPS:
				{
					inHostPort = kPGPKeyServerPortLDAPS;
				}
				break;
				
				
				case kPGPKeyServerProtocol_HTTP:
				{
					if (inClass == kPGPKeyServerClass_PGP) {
						inHostPort = kPGPKeyServerPortHTTPPGP;
					} else {
						inHostPort = kPGPKeyServerPortHTTP;
					}
				}
				break;
				
				
				case kPGPKeyServerProtocol_HTTPS:
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
		if ((inType == kPGPKeyServerProtocol_LDAP)
		|| (inType == kPGPKeyServerProtocol_LDAPS)) {
			if (inClass != kPGPKeyServerClass_PGP) {
				ThrowPGPError_(kPGPError_FeatureNotAvailable);
			}
			result = new CLDAPKeyServer(	inContext,
											0,
											inHostAddress,
											inHostPort,
											inType,
											inAccessType,
											inKeySpace);
		} else {
			switch (inClass) {
				case kPGPKeyServerClass_PGP:
				{
					result = new CHTTPPGPKeyServer(	inContext,
													0,
													inHostAddress,
													inHostPort,
													inType);
				}
				break;
				
				
				case kPGPKeyServerClass_Verisign:
				{
					result = new CCRSVerisignServer(	inContext,
														0,
														inHostAddress,
														inHostPort,
														0,
														inType);
				}
				break;
				
				
				case kPGPKeyServerClass_Entrust:
				{
					result = new CCRSEntrustServer(	inContext,
													0,
													inHostAddress,
													inHostPort,
													0,
													inType);
				}
				break;
				
				
				case kPGPKeyServerClass_NetToolsCA:
				{
					result = new CHTTPXcertServer(	inContext,
													0,
													inHostAddress,
													inHostPort,
													0,
													inType);
				}
				break;
				
				default:
					break;
			}
		}
		if (result == 0) {
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
	const char *			inPath,
	PGPKeyServerProtocol	inType,
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace,
	PGPKeyServerClass		inClass)
{
	CKeyServer *	result = 0;

	CheckInitialization();
	
	try {
		if (inHostPort == 0) {
			switch (inType) {
				case kPGPKeyServerProtocol_LDAP:
				{
					inHostPort = kPGPKeyServerPortLDAP;
				}
				break;
				
				
				case kPGPKeyServerProtocol_LDAPS:
				{
					inHostPort = kPGPKeyServerPortLDAPS;
				}
				break;
				
				
				case kPGPKeyServerProtocol_HTTP:
				{
					if (inClass == kPGPKeyServerClass_PGP) {
						inHostPort = kPGPKeyServerPortHTTPPGP;
					} else {
						inHostPort = kPGPKeyServerPortHTTP;
					}
				}
				break;
				
				
				case kPGPKeyServerProtocol_HTTPS:
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
		if ((inType == kPGPKeyServerProtocol_LDAP)
		|| (inType == kPGPKeyServerProtocol_LDAPS)) {
			if (inClass != kPGPKeyServerClass_PGP) {
				ThrowPGPError_(kPGPError_FeatureNotAvailable);
			}
			result = new CLDAPKeyServer(	inContext,
											inHostName,
											0,
											inHostPort,
											inType,
											inAccessType,
											inKeySpace);
		} else {
			switch (inClass) {
				case kPGPKeyServerClass_PGP:
				{
					result = new CHTTPPGPKeyServer(	inContext,
													inHostName,
													0,
													inHostPort,
													inType);
				}
				break;
				
				
				case kPGPKeyServerClass_Verisign:
				{
					result = new CCRSVerisignServer(	inContext,
														inHostName,
														0,
														inHostPort,
														inPath,
														inType);
				}
				break;
				
				
				case kPGPKeyServerClass_Entrust:
				{
					result = new CCRSEntrustServer(	inContext,
													inHostName,
													0,
													inHostPort,
													inPath,
													inType);
				}
				break;
				
				
				case kPGPKeyServerClass_NetToolsCA:
				{
					result = new CHTTPXcertServer(	inContext,
													inHostName,
													0,
													inHostPort,
													inPath,
													inType);
				}
				break;
				
				default:
					break;
			}
		}
		if (result == 0) {
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
	const char *		inPath,
	PGPKeyServerProtocol	inType)
	: mContext(inContext), mHostName(0), mHostPort(inHostPort),
	  mHostAddress(inHostAddress), mPath(0), mType(inType), mEventHandler(0),
	  mEventHandlerData(0),  mTLSSession(kInvalidPGPtlsSessionRef),
	  mErrorString(0), mSecured(false), mIsBusy(false), mIsOpen(false),
	  mCanceled(false), mRefCount(1)
#if PGP_WIN32
	  , mAsyncHostEntryRef(kPGPInvalidAsyncHostEntryRef)
#endif
{
	if (inHostName != 0) {
		mHostName = new char[strlen(inHostName) + 1];
		if (mHostName == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		strcpy(mHostName, inHostName);
	}
	if (inPath != 0) {
		mPath = new char[strlen(inPath) + 2];
		if (mPath == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		*mPath = '/';
		strcpy(mPath + 1, inPath);
	}
}



CKeyServer::~CKeyServer()
{
	delete[] mHostName;
	delete[] mErrorString;
	delete[] mPath;
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
	
	if (eventHandler != 0) {
		PGPContextRef	context = kInvalidPGPContextRef;
		PGPEvent		theEvent;
		
		pgpClearMemory(&theEvent, sizeof(theEvent));
		theEvent.type = kPGPEvent_KeyServerIdleEvent;
		theEvent.job = 0;
		theEvent.data.keyServerIdleData.keyServerRef =
									(PGPKeyServerRef) callBackArg;
		if (callBackArg != 0) {
			context = ((CKeyServer *) callBackArg)->mContext;
		}
		result = eventHandler(context, &theEvent, eventHandlerData);
		if (callBackArg != 0) {
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
	InitializeHostNameAndAddress(&mHostName, &mHostAddress);
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
	InitializeHostNameAndAddress(&mHostName, &mHostAddress);
	*outHostName = (char *) PGPNewData(	PGPGetContextMemoryMgr(mContext),
										strlen(mHostName) + 1,
										kPGPMemoryMgrFlags_None);
	if (*outHostName == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	strcpy(*outHostName, mHostName);
}



	void
CKeyServer::GetAddress(
	PGPUInt32 &	outAddress)
{
	InitializeHostNameAndAddress(&mHostName, &mHostAddress);
	outAddress = mHostAddress;
}



	void
CKeyServer::GetPath(
	char **	outPath)
{
	*outPath = (char *) PGPNewData(	PGPGetContextMemoryMgr(mContext),
									strlen(mPath) + 1,
									kPGPMemoryMgrFlags_None);
	if (*outPath == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	strcpy(*outPath, mPath);
}



	void
CKeyServer::GetErrorString(
	char **	outErrorString)
{
	if (mErrorString != 0) {
		*outErrorString = (char *) PGPNewData(
										PGPGetContextMemoryMgr(mContext),
										strlen(mErrorString) + 1,
										kPGPMemoryMgrFlags_None);
		if (*outErrorString == 0) {
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
	
	if (inErrorString == 0) {
		mErrorString = 0;
	} else {
		mErrorString = new char[strlen(inErrorString) + 1];
		if (mErrorString != 0) {
			strcpy(mErrorString, inErrorString);
		}
	}
}



	void
CKeyServer::InitOperation()
{
	SetErrorString(0);
	if (! mIsOpen) {
		ThrowPGPError_(kPGPError_ServerNotOpen);
	}
}



	void
CKeyServer::InitializeHostNameAndAddress(
	char **		inHostName,
	PGPUInt32 *	inHostAddress)
{
	StPreserveSocketsEventHandler	preserve(this);
	PGPHostEntry *					hostEntry = 0;
	
	if (*inHostName != 0) {
		PGPUInt32	tempAddress = PGPDottedToInternetAddress(*inHostName);
		
		if ((PGPInt32) tempAddress != kPGPSockets_Error) {
			*inHostAddress = PGPNetToHostLong(tempAddress);
			delete *inHostName;
			*inHostName = 0;
		}
	}
	
#if PGP_WIN32
		PGPError				err;
		char					h_name[256];
#endif
		
	if (*inHostName == 0) {
		PGPInternetAddress	theAddress;
		char *				theHostName;
		
		theAddress.s_addr = PGPHostToNetLong(*inHostAddress);
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
		if (hostEntry == 0) {
#endif
			PGPInternetAddress	address;
			
			address.s_addr = PGPHostToNetLong(*inHostAddress);
			theHostName = PGPInternetAddressToDottedString(address);
			if (theHostName == 0) {
				ThrowPGPError_(kPGPError_UnknownError);
			}
		} else {
#if PGP_WIN32
			theHostName = h_name;
#else
			theHostName = hostEntry->h_name;
#endif
		}
		*inHostName = new char[strlen(theHostName) + 1];
		if (*inHostName == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		strcpy(*inHostName, theHostName);
	} else if (*inHostAddress == 0) {
#if PGP_WIN32
		err = PGPStartAsyncGetHostByName(*inHostName, &mAsyncHostEntryRef);
		ThrowIfPGPError_(err);
		err = PGPWaitForGetHostByName(mAsyncHostEntryRef, inHostAddress);
		mAsyncHostEntryRef = kPGPInvalidAsyncHostEntryRef;
		ThrowIfPGPError_(err);
		*inHostAddress = PGPNetToHostLong(*inHostAddress);
#else
		hostEntry = PGPGetHostByName(*inHostName);
		if (hostEntry == 0) {
			ThrowPGPError_(PGPGetLastSocketsError());
		}
		*inHostAddress = PGPNetToHostLong(
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
			if (theResult == 0) {
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
		(*theIterator).second.idleEventHandler = 0;
		(*theIterator).second.idleEventHandlerData = 0;
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
	if (inContext != 0) {
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
	
	if (theContext != 0) {
		*outCallback = theContext->idleEventHandler;
		*outUserData = theContext->idleEventHandlerData;
	} else {
		*outCallback = 0;
		*outUserData = 0;
	}
}



	void
CKeyServer::SetThreadIdleEventHandler(
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue			inUserData)
{
	SThreadContext *	theContext = GetThreadContext();
	
	if (theContext != 0) {
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
			TlsSetValue(sIdleEventHandlerIndex, 0);
		}
	}
	PGPRMWOLockStopWriting(&sThreadLock);
}


	void
CKeyServer::CreateThreadStorage(
	SThreadContext **	outContext)
{
	*outContext = new SThreadContext;
	if (*outContext == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	GetThreadIdleEventHandler(	&(*outContext)->idleEventHandler,
								&(*outContext)->idleEventHandlerData);
}



	void
CKeyServer::DisposeThreadStorage(
	SThreadContext *	inContext)
{
	if (inContext != 0) {
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
	*outContext = 0;
}



	void
CKeyServer::DisposeThreadStorage(
	SThreadContext *	inContext)
{
	(void) inContext;
}

#endif
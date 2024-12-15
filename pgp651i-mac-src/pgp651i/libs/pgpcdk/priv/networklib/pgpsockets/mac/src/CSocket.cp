/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: CSocket.cp,v 1.34 1999/03/10 02:39:58 heller Exp $____________________________________________________________________________*/#include <Gestalt.h>#include "pgpMem.h"#include "pgpEncode.h"#include "COTInternetUtilities.h"#include "COTUDPSocket.h"#include "COTTCPSocket.h"#include "CMacTCPInternetUtilities.h"#include "CMacTCPUDPSocket.h"#include "CMacTCPTCPSocket.h"#include "CSocket.h"Boolean							CSocket::sInitialized = false;Boolean							CSocket::sUseOT = false;CInternetUtilities *			CSocket::sInternetUtilities = nil;set<CSocket *>					CSocket::sSocketsSet;map<ThreadID, SThreadContext>	CSocket::sThreadContextMap;const UInt32					kMinimumOTVersion = 0x01300000;extern ThreadID					gMainThreadID;	voidCSocket::Initialize(){	if (! sInitialized) {		SInt32		theGestaltResult;		OSStatus			err = noErr;		// Determine whether or not to use MacTCP or OT		if ((((UniversalProcPtr) InitOpenTransport) !=		(UniversalProcPtr) kUnresolvedCFragSymbolAddress)		&& (::Gestalt(gestaltOpenTpt, &theGestaltResult) == noErr)		&& (theGestaltResult & gestaltOpenTptTCPPresentMask)		&& (::Gestalt(gestaltOpenTptVersions, &theGestaltResult) == noErr)		&& (((NumVersionVariant *) &theGestaltResult)->whole >=		kMinimumOTVersion)) {			sUseOT = true;		} else {			sUseOT = false;		}		// Perform initialization		if (sUseOT) {			err = ::InitOpenTransport();			if (err != kOTNoError) {				ThrowPGPError_(err);			}			sInternetUtilities = new CMacTCPInternetUtilities;//			sInternetUtilities = new COTInternetUtilities;		} else {			if (::Gestalt('mtcp', &theGestaltResult) != noErr) {				ThrowPGPError_(kPGPError_SocketsNetworkDown);			}			sInternetUtilities = new CMacTCPInternetUtilities;		}		if (sInternetUtilities == nil) {			ThrowPGPError_(kPGPError_OutOfMemory);		}		sInitialized = true;	}}		voidCSocket::CleanupSockets(){	sSocketsSet.erase(sSocketsSet.begin(), sSocketsSet.end());	sThreadContextMap.erase(	sThreadContextMap.begin(),								sThreadContextMap.end());	delete sInternetUtilities;	sInternetUtilities = 0;		if (sUseOT) {		::CloseOpenTransport();	}	sInitialized = false;}	BooleanCSocket::VerifyPGPSocketRef(	CSocket *	inSocketRef){	return (sSocketsSet.find(inSocketRef) != sSocketsSet.end());}	voidCSocket::CreateThreadStorage(	SThreadContext **	outContext){	OSStatus									err;	ThreadID									theThreadID;	map<ThreadID, SThreadContext>::iterator		theIterator;	SThreadContext *							theResult = nil;	try {		err = ::GetCurrentThread(&theThreadID);		if (err != noErr) {			ThrowPGPError_(err);		}		theIterator = sThreadContextMap.find(theThreadID);		if (theIterator != sThreadContextMap.end()) {			theResult = new SThreadContext;			if (theResult == 0) {				ThrowPGPError_(kPGPError_OutOfMemory);			}			*theResult = (*theIterator).second;		} else {			SThreadContext	tempContext = {0, };			pgpLeaksSuspend();			sThreadContextMap[theThreadID] = tempContext;			pgpLeaksResume();						theIterator = sThreadContextMap.find(theThreadID);			if (theIterator == sThreadContextMap.end()) {				ThrowPGPError_(kPGPError_OutOfMemory);			}		}		(*theIterator).second.isBusy = false;		(*theIterator).second.lastError = kPGPError_NoErr;		(*theIterator).second.hNameBuffer[0] = 0;		(*theIterator).second.hAliasesBuffer = 0;		(*theIterator).second.hostEntry.h_name =				(*theIterator).second.hNameBuffer;		(*theIterator).second.hostEntry.unused =				(*theIterator).second.hAliasesBuffer;		(*theIterator).second.hostEntry.h_addrtype = kPGPAddressFamilyInternet;		(*theIterator).second.hostEntry.h_length = sizeof(PGPUInt32);		(*theIterator).second.hostEntry.h_addr_list = 				(char **) (*theIterator).second.hAddressesListBuffer;		(*theIterator).second.idleEventHandler = 0;		(*theIterator).second.idleEventHandlerData = 0;	}		catch (...) {		delete theResult;		throw;	}		*outContext = theResult;}	voidCSocket::DisposeThreadStorage(	SThreadContext *	inContext){	ThreadID	theThreadID;	OSStatus	err;											err = ::GetCurrentThread(&theThreadID);	if (err != noErr) {		ThrowPGPError_(err);	}	if (inContext != 0) {		sThreadContextMap[theThreadID] = *inContext;		delete inContext;	} else {		map<ThreadID, SThreadContext>::iterator	theIterator;		theIterator = sThreadContextMap.find(theThreadID);		if (theIterator != sThreadContextMap.end()) {			sThreadContextMap.erase(theThreadID);		}	}		}	SThreadContext *CSocket::GetThreadContext(){	OSStatus									err;	ThreadID									theThreadID;	map<ThreadID, SThreadContext>::iterator		theIterator;	SThreadContext *							theResult = nil;		try {		err = ::GetCurrentThread(&theThreadID);		if (err != noErr) {			ThrowPGPError_(err);		}		theIterator = sThreadContextMap.find(theThreadID);		if (theIterator == sThreadContextMap.end()) {			ThrowPGPError_(kPGPError_SocketsNoStaticStorage);		}		theResult = &(*theIterator).second;	}		catch (...) {	}		return theResult;}	CSocket *CSocket::CreateSocket(	SInt32	inType){	CSocket *	theSocket = (CSocket *) kInvalidPGPSocketRef;		// Create a socket of the correct type	switch (inType) {		case kPGPSocketTypeStream:		{			if (sUseOT) {				theSocket = new COTTCPSocket;			} else {				theSocket = new CMacTCPTCPSocket;			}			if (theSocket == nil) {				ThrowPGPError_(kPGPError_OutOfMemory);			}		}		break;						case kPGPSocketTypeDatagram:		{			if (sUseOT) {				theSocket = new COTUDPSocket;			} else {				theSocket = new CMacTCPUDPSocket;			}			if (theSocket == nil) {				ThrowPGPError_(kPGPError_OutOfMemory);			}		}		break;						default:		{			ThrowPGPError_(kPGPError_SocketsProtocolNotSupported);		}		break;	}		return theSocket;}CSocket::CSocket()	: mTLSSession(kInvalidPGPtlsSessionRef), mInCallback(false){	pgpLeaksSuspend();	sSocketsSet.insert(this);	pgpLeaksResume();}CSocket::~CSocket(){	sSocketsSet.erase(this);}	SInt32CSocket::Select(	PGPSocketSet *				ioReadSet,	PGPSocketSet *				ioWriteSet,	PGPSocketSet *				ioErrorSet,	const PGPSocketsTimeValue *	inTimeout){	UInt32				ticks;	SInt32				result = 0;	UInt16				i;	PGPSocketSet *		readSet = nil;	PGPSocketSet *		writeSet = nil;	PGPSocketSet *		errorSet = nil;		try {		readSet = new PGPSocketSet;		writeSet = new PGPSocketSet;		errorSet = new PGPSocketSet;		if ((readSet == nil) || (writeSet == nil) || (errorSet == nil)) {			ThrowPGPError_(kPGPError_OutOfMemory);		}				PGPSOCKETSET_ZERO(readSet);		PGPSOCKETSET_ZERO(writeSet);		PGPSOCKETSET_ZERO(errorSet);		// Set the timeout		if (inTimeout == 0) {			ticks = 0xFFFFFFFF;		} else {			ticks = ::TickCount() + (60 * inTimeout->tv_sec)						+ ((1000 * inTimeout->tv_usec) / 60);		}				// Check the sets		Boolean	socketClosed = false;		do {			if (ioReadSet != 0) {				for (i = 0; i < ioReadSet->fd_count; i++) {					if (VerifyPGPSocketRef(					(CSocket *) ioReadSet->fd_array[i])) {						if (((CSocket *) ioReadSet->fd_array[i])->						IsReadable()) {							PGPSOCKETSET_SET(	ioReadSet->fd_array[i],												readSet);							result++;						}					} else {						socketClosed = true;					}				}			}			if (ioWriteSet != 0) {				for (i = 0; i < ioWriteSet->fd_count; i++) {					if (VerifyPGPSocketRef(					(CSocket *) ioWriteSet->fd_array[i])) {						if (((CSocket *) ioWriteSet->fd_array[i])->							IsWriteable()) {								PGPSOCKETSET_SET(ioWriteSet->fd_array[i],									writeSet);								result++;							}					} else {						socketClosed = true;					}				}			}			if (ioErrorSet != 0) {				for (i = 0; i < ioErrorSet->fd_count; i++) {					if (VerifyPGPSocketRef(					(CSocket *) ioErrorSet->fd_array[i])) {						if (((CSocket *) ioErrorSet->fd_array[i])->IsError()) {							PGPSOCKETSET_SET(ioErrorSet->fd_array[i], errorSet);							result++;						}					} else {						socketClosed = true;					}				}			}						if ((result > 0) || socketClosed) {				break;			}						// Give time to callback			ThrowIfPGPError_(CallIdleEventHandler());		} while (::TickCount() < ticks);				if (ioReadSet != 0) {			*ioReadSet = *readSet;		}		if (ioWriteSet != 0) {			*ioWriteSet = *writeSet;		}		if (ioErrorSet != 0) {			*ioErrorSet = *errorSet;		}				delete readSet;		delete writeSet;		delete errorSet;	}		catch (...) {		delete readSet;		delete writeSet;		delete errorSet;		throw;	}		return result;}		voidCSocket::GetSocketOptions(	SInt32		inOptionName,	SInt32 *	outOptionValue){	switch (inOptionName) {		case kPGPSocketOptionAcceptingConnections:		{			*outOptionValue = IsListening();		}		break;						case kPGPSocketOptionType:		{			*outOptionValue = mSocketType;		}		break;	}}	voidCSocket::SetIdleEventHandler(	PGPEventHandlerProcPtr	inCallback,	PGPUserValue			inUserData){	SThreadContext *	theContext = CSocket::GetThreadContext();		if (theContext == nil) {		ThrowPGPError_(kPGPError_UnknownError);	} else {		theContext->idleEventHandler = inCallback;		theContext->idleEventHandlerData = inUserData;	}}	voidCSocket::GetIdleEventHandler(	PGPEventHandlerProcPtr *	outCallback,	PGPUserValue *				outUserData){	SThreadContext *	theContext = CSocket::GetThreadContext();	if (theContext == nil) {		ThrowPGPError_(kPGPError_UnknownError);	} else {		*outCallback = theContext->idleEventHandler;		*outUserData = theContext->idleEventHandlerData;	}}	PGPErrorCSocket::CallIdleEventHandler(){	SThreadContext *	theContext = CSocket::GetThreadContext();	PGPError	result;		if ((theContext != nil) && (theContext->idleEventHandler != nil)) {		PGPEvent			theEvent;		pgpClearMemory(&theEvent, sizeof(theEvent));		theEvent.type = kPGPEvent_SocketsIdleEvent;		result = theContext->idleEventHandler(									nil,									&theEvent,									theContext->idleEventHandlerData);	} else {		result = kPGPError_NoErr;	}		return result;}
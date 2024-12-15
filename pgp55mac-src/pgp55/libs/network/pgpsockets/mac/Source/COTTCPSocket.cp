/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COTTCPSocket.cp,v 1.8.10.1 1997/12/07 04:26:48 mhw Exp $
____________________________________________________________________________*/


#include <string.h>

#include "COTTCPSocket.h"

#include "pgpDebug.h"


COTTCPSocket::COTTCPSocket()
{
	OSStatus	err;
	
	// Create the endpoint
	mEndpointRef = ::OTOpenEndpoint(	::OTCreateConfiguration(kTCPName),
										0,
										NULL,
										&err);
	ThrowIfOTError(err);
	err = ::OTSetBlocking(mEndpointRef);
	ThrowIfOTError(err);
	err = ::OTInstallNotifier(	mEndpointRef,
								NotifyProc,
								this);
	ThrowIfOTError(err);
	err = ::OTUseSyncIdleEvents(	mEndpointRef,
									true);
	ThrowIfOTError(err);
}



COTTCPSocket::~COTTCPSocket()
{
}


	void
COTTCPSocket::Connect(
	const PGPSocketAddressInternet *	inAddress)
{
	OSStatus	err;
	TCall		sndCall;
	InetAddress	theAddress;
	
	// Bind the endpoint
	DoBind(0);
	
	// Create the TCall
	theAddress.fAddressType = AF_INET;
	theAddress.fPort = inAddress->sin_port;
	theAddress.fHost = inAddress->sin_addr.s_addr;
	sndCall.addr.len = sizeof(theAddress);
	sndCall.addr.buf = (UInt8 *) &theAddress;
	sndCall.addr.maxlen = sizeof(theAddress);
	sndCall.opt.len = 0;
	sndCall.opt.buf = nil;
	sndCall.opt.maxlen = 0;
	sndCall.udata.len = 0;
	sndCall.udata.buf = nil;
	sndCall.udata.maxlen = 0;
	sndCall.sequence = 0;

	err = ::OTSetAsynchronous(mEndpointRef);
	ThrowIfOTError(err);
	
	// Connect
	err = ::OTConnect(	mEndpointRef,
						&sndCall,
						nil);
	mInCallback = true;
	while (err == kOTNoDataErr) {
		err = ::OTRcvConnect(mEndpointRef, nil);
		if (mCallback != nil) {
			mCallback(mCallbackData);
		}
		if (mAsyncCancel) {
			err = kOTCanceledErr;
			mAsyncCancel = false;
		}
	};
	mInCallback = false;
	ThrowIfOTError(::OTSetSynchronous(mEndpointRef));
		
	// Handle disconnects and listens
	if (err == kOTLookErr) {
		OTResult	event;
		
		event = ::OTLook(mEndpointRef);
		
		switch (event) {
			case T_LISTEN:
			{
				TEndpointInfo	info;
				TCall			call;
				InetAddress		address;
				UInt8 *			options = (UInt8 *)::NewPtr(info.options);
				
				err = ::OTGetEndpointInfo(mEndpointRef, &info);
				if (err == kOTNoError) {
					options = (UInt8 *) ::NewPtr(info.options);
					call.addr.buf = (UInt8 *) &address;
					call.addr.maxlen = sizeof(address);
					call.opt.buf = options;
					call.opt.maxlen = (options == nil) ? 0 : info.options;
					call.udata.buf = nil;
					call.udata.maxlen = 0;
					
					err = ::OTListen(mEndpointRef, &call);
					if (err == kOTNoError) {
						::OTSndDisconnect(mEndpointRef, &call);
					}
					::DisposePtr((Ptr) options);
					err = ::OTRcvConnect(mEndpointRef, nil);
				}
			}
			break;
			
			
			default:
			{
				err = kOTLookErr;
			}
			break;
		}
	}
	ThrowIfOTError(err);
	
	// Connection was accepted
	SetSetInfo(triState_NoChange, triState_True, triState_False);
}



/*	This is the correct way to do it, unfortunately there is a bug
	in versions of OT < 1.3
	
	void
COTTCPSocket::Connect(
	const PGPSocketAddressInternet *	inAddress)
{
	OSStatus	err;
	TCall		sndCall;
	InetAddress	theAddress;
	
	// Bind the endpoint
	DoBind(0);
	
	// Create the TCall
	theAddress.fAddressType = AF_INET;
	theAddress.fPort = inAddress->sin_port;
	theAddress.fHost = inAddress->sin_addr.s_addr;
	sndCall.addr.len = sizeof(theAddress);
	sndCall.addr.buf = (UInt8 *) &theAddress;
	sndCall.addr.maxlen = sizeof(theAddress);
	sndCall.opt.len = 0;
	sndCall.opt.buf = nil;
	sndCall.opt.maxlen = 0;
	sndCall.udata.len = 0;
	sndCall.udata.buf = nil;
	sndCall.udata.maxlen = 0;
	sndCall.sequence = 0;
		
	// Connect
	err = ::OTConnect(	mEndpointRef,
						&sndCall,
						nil);
	
	// Handle disconnects and listens
	if (err == kOTLookErr) {
		OTResult	event;
		
		event = ::OTLook(mEndpointRef);
		
		switch (event) {
			case T_LISTEN:
			{
				TEndpointInfo	info;
				TCall			call;
				InetAddress		address;
				UInt8 *			options = (UInt8 *)::NewPtr(info.options);
				
				err = ::OTGetEndpointInfo(mEndpointRef, &info);
				if (err == kOTNoError) {
					options = (UInt8 *) ::NewPtr(info.options);
					call.addr.buf = (UInt8 *) &address;
					call.addr.maxlen = sizeof(address);
					call.opt.buf = options;
					call.opt.maxlen = (options == nil) ? 0 : info.options;
					call.udata.buf = nil;
					call.udata.maxlen = 0;
					
					err = ::OTListen(mEndpointRef, &call);
					if (err == kOTNoError) {
						::OTSndDisconnect(mEndpointRef, &call);
					}
					::DisposePtr((Ptr) options);
					err = ::OTRcvConnect
				}
			}
			break;
			
			
			default:
			{
				err = kOTLookErr;
			}
			break;
		}
	}
	ThrowIfOTError(err);
	
	// Connection was accepted
	SetSetInfo(triState_NoChange, triState_True, triState_False);
}
*/

	SInt16
COTTCPSocket::Send(
	const void *	inBuffer,
	SInt16			inLength,
	UInt16			inFlags)
{
	OSStatus		result;
	OTFlags			flags = 0;

	if (inFlags & kPGPSendFlagOOB) {
		flags |= T_EXPEDITED;
	}
	result = ::OTSnd(	mEndpointRef,
						(void *) inBuffer,
						inLength,
						flags);
	pgpAssert( result != kOTOutStateErr );
	ThrowIfOTError(result);

	SetSetInfo(triState_NoChange, triState_NoChange, triState_False);
	return result;
}



	SInt16
COTTCPSocket::SendTo(
	const void *						inBuffer,
	SInt16								inLength,
	const PGPSocketAddressInternet *	inAddress)
{
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
	
	return -1;
}



	SInt16
COTTCPSocket::Receive(
	void *	outBuffer,
	SInt16	inLength,
	UInt16	inFlags)
{
	OSStatus		result;
	OTFlags			flags = 0;
	
	result = ::OTRcv(	mEndpointRef,
						outBuffer,
						inLength,
						&flags);
	pgpAssert( result != kOTOutStateErr );
	
	// Handle disconnects
	if (result == kOTLookErr) {
		OTResult	event;
		
		event = ::OTLook(mEndpointRef);
		
		switch (event) {
			case T_DISCONNECT:
			{
				::OTRcvDisconnect(mEndpointRef, nil);
				result = 0;
			}
			break;


			case T_ORDREL:
			{
				::OTRcvOrderlyDisconnect(mEndpointRef);
				result = 0;
			}
			break;
		}
	}
	ThrowIfOTError(result);

	SetSetInfo(triState_NoChange, triState_NoChange, triState_False);
	return result;
}



	SInt16
COTTCPSocket::ReceiveFrom(
	void *						outBuffer,
	SInt16						inSize,
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	if (ioAddressLength != NULL) {
		*ioAddressLength = 0;
	}

	return Receive(outBuffer, inSize, kPGPReceiveFlagNone);	
}



	void
COTTCPSocket::Listen(
	SInt16	inMaxBacklog)
{
	DoBind(inMaxBacklog);
}



	CSocket *
COTTCPSocket::Accept(
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	OSStatus	result;
	TCall		theCall;
	InetAddress	theAddress;
	
	// Create the TCall
	theCall.addr.buf = (UInt8 *) &theAddress;
	theCall.addr.maxlen = sizeof(theAddress);
	theCall.opt.buf = nil;
	theCall.opt.maxlen = 0;
	theCall.udata.buf = nil;
	theCall.udata.maxlen = 0;

	result = ::OTListen(	mEndpointRef,
							&theCall);
	pgpAssert( result != kOTOutStateErr );
	ThrowIfOTError(result);
	
	// Accept the connection
	COTTCPSocket *	theNewSocket = new COTTCPSocket;
	
	if (theNewSocket == nil) {
		SetSetInfo(triState_False, triState_False, triState_True);
		throw((PGPSocketsError) kPGPSocketsErrorMFile);
	}
	
	result = ::OTAccept(	mEndpointRef, 
							theNewSocket->mEndpointRef,
							&theCall);
	pgpAssert( result != kOTOutStateErr );
	if (result != kOTNoError) {
		delete theNewSocket;
		ThrowIfOTError(result);
	}
	
	// Fill in the parms
	*ioAddressLength = sizeof(PGPSocketAddressInternet);
	outAddress->sin_family = kPGPAddressFamilyInternet;
	outAddress->sin_port = theAddress.fPort;
	outAddress->sin_addr.s_addr = theAddress.fHost;
	
	theNewSocket->SetSetInfo(	triState_NoChange,
								triState_True,
								triState_NoChange);
	
	SetSetInfo(triState_NoChange, triState_NoChange, triState_False);
	return theNewSocket;
}




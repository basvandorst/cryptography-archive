/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COTInternetSocket.cp,v 1.3.10.1 1997/12/07 04:26:45 mhw Exp $
____________________________________________________________________________*/

#include <stdarg.h>

#include "CPGPSocketsArray.h"

#include "COTInternetSocket.h"


COTInternetSocket::COTInternetSocket()
	: mEndpointRef(nil), mInCallback(false), mAsyncCancel(false)
{
	// Set up the binding address to default
	mBoundAddress.fAddressType = AF_INET;
	mBoundAddress.fPort = 0;
	mBoundAddress.fHost = 0;
}



COTInternetSocket::~COTInternetSocket()
{
}



	void
COTInternetSocket::Close()
{
	if (mInCallback) {
		if (::OTIsSynchronous(mEndpointRef)) {
			::OTCancelSynchronousCalls(mEndpointRef, kOTCanceledErr);
		} else {
			mAsyncCancel = true;
		}
	} else {
		::OTSetNonBlocking(mEndpointRef);
		Cleanup();
		::OTCloseProvider(mEndpointRef);
		delete this;
	}
}



	void
COTInternetSocket::Bind(
	const PGPSocketAddressInternet *	inAddress)
{
	mBoundAddress.fPort = inAddress->sin_port;
	mBoundAddress.fHost = inAddress->sin_addr.s_addr;
}



	void
COTInternetSocket::DoBind(
	OTQLen	inQLen)
{
	OSStatus	err;
	TBind		theBind;
	
	theBind.addr.len = sizeof(mBoundAddress);
	theBind.addr.buf = (UInt8 *) &mBoundAddress;
	theBind.addr.maxlen = sizeof(mBoundAddress);
	theBind.qlen = inQLen;
	
	err = ::OTBind(	mEndpointRef,
					&theBind,
					&theBind);
	ThrowIfOTError(err);
	SetSetInfo(triState_NoChange, triState_NoChange, triState_False);
}



	void
COTInternetSocket::Cleanup()
{
}



	Boolean
COTInternetSocket::IsReadable()
{
	size_t			count = 0;
	
	::OTCountDataBytes(mEndpointRef, &count);	
	if ((count > 0) || (::OTLook(mEndpointRef) == T_LISTEN)) {
		SetSetInfo(triState_True, triState_NoChange, triState_NoChange);
	}

	return mIsReadable;
}





	pascal void
COTInternetSocket::NotifyProc(
	void *		contextPtr,
	OTEventCode	code, 
	OTResult	result,
	void *		cookie)
{
	if (code == kOTSyncIdleEvent) {
		((COTInternetSocket *) contextPtr)->mInCallback = true;
		if (((COTInternetSocket *) contextPtr)->mCallback != nil) {
			((COTInternetSocket *) contextPtr)->
				mCallback(((COTInternetSocket *) contextPtr)->mCallbackData);
		}
		((COTInternetSocket *) contextPtr)->mInCallback = false;
	}
}



	void
COTInternetSocket::ThrowIfOTError(
	OSStatus	inErr)
{
	if (inErr < 0) {
		if (inErr == kOTCanceledErr) {
			Close();
		} else if (inErr == kOTLookErr) {
			// Clean up on an unexpected look
			OTResult	result;
			
			result = ::OTLook(mEndpointRef);
			switch (result) {
				case T_DISCONNECT:
				{
					::OTRcvDisconnect(mEndpointRef, nil);
					throw((PGPSocketsError) kPGPSocketsErrorNotConn);
				}
				break;
				
				
				case T_LISTEN:
				{
					TEndpointInfo	info;
					TCall			call;
					InetAddress		address;
					UInt8 *			options = (UInt8 *)::NewPtr(info.options);
					
					inErr = ::OTGetEndpointInfo(mEndpointRef, &info);
					if (inErr == kOTNoError) {
						options = (UInt8 *) ::NewPtr(info.options);
						call.addr.buf = (UInt8 *) &address;
						call.addr.maxlen = sizeof(address);
						call.opt.buf = options;
						call.opt.maxlen = (options == nil) ? 0 : info.options;
						call.udata.buf = nil;
						call.udata.maxlen = 0;
						
						inErr = ::OTListen(mEndpointRef, &call);
						if (inErr == kOTNoError) {
							::OTSndDisconnect(mEndpointRef, &call);
						}
						::DisposePtr((Ptr) options);
					}
					
					throw((PGPSocketsError) kPGPSocketsErrorUnknown);
				}
				break;
				
				
				case T_ORDREL:
				{
					::OTRcvOrderlyDisconnect(mEndpointRef);
					throw((PGPSocketsError) kPGPSocketsErrorNotConn);
				}
				break;
				
				
				case T_UDERR:
				{
					inErr = ::OTRcvUDErr(mEndpointRef, nil);
				}
				break;
			}
		}
		
		SetSetInfo(triState_False, triState_False, triState_True);
		throw(inErr);
	}
}

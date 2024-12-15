/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: COTInternetSocket.cp,v 1.13 1999/03/10 02:35:15 heller Exp $____________________________________________________________________________*/#include <stdarg.h>#include "pgpMem.h"#include "COTInternetSocket.h"COTInternetSocket::COTInternetSocket()	: mEndpointRef(nil), mCanceled(false){	// Set up the binding address to default	mBoundAddress.fAddressType = AF_INET;	mBoundAddress.fPort = 0;	mBoundAddress.fHost = 0;}COTInternetSocket::~COTInternetSocket(){}	voidCOTInternetSocket::Close(){	if (mInCallback) {		::OTCancelSynchronousCalls(mEndpointRef, kOTCanceledErr);		mCanceled = true;	} else {		Cleanup();				::OTUnbind(mEndpointRef);		::OTCloseProvider(mEndpointRef);		delete this;	}}	voidCOTInternetSocket::Bind(	const PGPSocketAddressInternet *	inAddress){	mBoundAddress.fPort = inAddress->sin_port;	mBoundAddress.fHost = inAddress->sin_addr.s_addr;}	voidCOTInternetSocket::DoBind(	OTQLen	inQLen){	OSStatus	err;	TBind		theBind;		theBind.addr.len = sizeof(mBoundAddress);	theBind.addr.buf = (UInt8 *) &mBoundAddress;	theBind.addr.maxlen = sizeof(mBoundAddress);	theBind.qlen = inQLen;		err = ::OTBind(	mEndpointRef,					&theBind,					&theBind);	ThrowIfOTError(err);}	voidCOTInternetSocket::Cleanup(){}	BooleanCOTInternetSocket::IsReadable(){	Boolean		isReadable = false;	size_t		numBytes;	OTResult	result;	result = ::OTCountDataBytes(mEndpointRef, &numBytes);	if ((result == kOTNoError) && (numBytes > 0)) {		isReadable = true;	} else {		result = ::OTLook(mEndpointRef);				switch (result) {			case T_DISCONNECT:			case T_ORDREL:			{				isReadable = true;			}			break;									case T_LISTEN:			{				if (IsListening()) {					isReadable = true;				}			}			break;		}	}		return isReadable;}	BooleanCOTInternetSocket::IsWriteable(){	OTResult	result = ::OTGetEndpointState(mEndpointRef);	Boolean		isWriteable = false;		switch (mSocketType) {		case kPGPSocketTypeStream:		{			if (result == T_DATAXFER) {				isWriteable = true;			}		}		break;						case kPGPSocketTypeDatagram:		{			if (result == T_IDLE) {				isWriteable = true;			}		}	}		return isWriteable;}	BooleanCOTInternetSocket::IsError(){	if (::OTLook(mEndpointRef) == T_DISCONNECT) {		return true;	} else {		return false;	}}	pascal voidCOTInternetSocket::NotifyProc(	void *		contextPtr,	OTEventCode	code, 	OTResult	result,	void *		cookie){	(void) result;	(void) cookie;		if (code == kOTSyncIdleEvent) {		((COTInternetSocket *) contextPtr)->mInCallback = true;		if (CallIdleEventHandler() != kPGPError_NoErr) {			((COTInternetSocket *) contextPtr)->Close();		}		((COTInternetSocket *) contextPtr)->mInCallback = false;	}}	voidCOTInternetSocket::ThrowIfOTError(	OSStatus	inErr){	if (inErr < 0) {		if (inErr == kOTCanceledErr) {			Close();		} else if (inErr == kOTLookErr) {			// Clean up on an unexpected look			OTResult	result;						result = ::OTLook(mEndpointRef);			switch (result) {				case T_DISCONNECT:				{					::OTRcvDisconnect(mEndpointRef, nil);					ThrowPGPError_(kPGPError_SocketsNotConnected);				}				break;												case T_LISTEN:				{					TEndpointInfo	info;					TCall			call;					InetAddress		address;					UInt8 *			options;										inErr = ::OTGetEndpointInfo(mEndpointRef, &info);					if (inErr == kOTNoError) {						options = (UInt8 *) ::NewPtr(info.options);						call.addr.buf = (UInt8 *) &address;						call.addr.maxlen = sizeof(address);						call.opt.buf = options;						call.opt.maxlen = (options == nil) ? 0 : info.options;						call.udata.buf = nil;						call.udata.maxlen = 0;												inErr = ::OTListen(mEndpointRef, &call);						if (inErr == kOTNoError) {							::OTSndDisconnect(mEndpointRef, &call);						}						::DisposePtr((Ptr) options);					}										ThrowPGPError_(kPGPError_UnknownError);				}				break;												case T_ORDREL:				{					::OTRcvOrderlyDisconnect(mEndpointRef);					ThrowPGPError_(kPGPError_SocketsNotConnected);				}				break;												case T_UDERR:				{					inErr = ::OTRcvUDErr(mEndpointRef, nil);				}				break;			}		}				ThrowPGPError_(inErr);	} else if (mCanceled) {		Close();		ThrowPGPError_(kOTCanceledErr);	}}	voidCOTInternetSocket::GetSocketName(	PGPSocketAddressInternet *	outName){	OSStatus	err;	TBind		theBind;	InetAddress	theBoundAddress;		pgpClearMemory(&theBind, sizeof(theBind));	theBind.addr.buf = (UInt8 *) &theBoundAddress;	theBind.addr.maxlen = sizeof(theBoundAddress);		err = ::OTGetProtAddress(	mEndpointRef,								&theBind,								nil);	ThrowIfOTError(err);	if (theBind.addr.len == 0) {		ThrowPGPError_(kPGPError_SocketsNotBound);	}	pgpClearMemory(outName, sizeof(PGPSocketAddressInternet));	outName->sin_family = kPGPAddressFamilyInternet;	outName->sin_port = theBoundAddress.fPort;	outName->sin_addr.s_addr = theBoundAddress.fHost;}	voidCOTInternetSocket::IOControlSocket(	SInt32		inCommand,	UInt32 *	ioParam){	switch (inCommand) {		case kPGPSocketCommandGetUnreadData:		{			::OTCountDataBytes(mEndpointRef, ioParam);			}		break;	}}
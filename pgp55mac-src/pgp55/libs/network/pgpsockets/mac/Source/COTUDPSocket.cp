/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COTUDPSocket.cp,v 1.1.10.1 1997/12/07 04:26:49 mhw Exp $
____________________________________________________________________________*/

#include "COTUDPSocket.h"



COTUDPSocket::COTUDPSocket()
	: mConnected(false), mBound(false)
{
	OSStatus	err;
	
	// Create the endpoint
	mEndpointRef = ::OTOpenEndpoint(	::OTCreateConfiguration(kUDPName),
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



COTUDPSocket::~COTUDPSocket()
{
}



	void
COTUDPSocket::Bind(
	const PGPSocketAddressInternet *	inAddress)
{
	inherited::Bind(inAddress);
	mBound = true;
	SetSetInfo(triState_NoChange, triState_True, triState_NoChange);
}



	void
COTUDPSocket::Connect(
	const PGPSocketAddressInternet *	inAddress)
{
	mConnected = true;
	::BlockMoveData(inAddress, &mConnectAddress, sizeof(PGPSocketAddressInternet));
}



	SInt16
COTUDPSocket::Send(
	const void *	inBuffer,
	SInt16			inLength,
	UInt16			inFlags)
{
	SInt16	result = 0;
	
	if (mConnected) {
		SendTo(inBuffer, inLength, &mConnectAddress);
	} else {
		throw((PGPSocketsError) kPGPSocketsErrorNotConn);
	}
	
	return result;
}



	SInt16
COTUDPSocket::SendTo(
	const void *						inBuffer,
	SInt16								inLength,
	const PGPSocketAddressInternet *	inAddress)
{
	OSStatus		err;
	TUnitData		theDataUnit;
	InetAddress		theAddress;
	
	// Fill out the data unit structure
	theAddress.fAddressType = AF_INET;
	theAddress.fPort = inAddress->sin_port;
	theAddress.fHost = inAddress->sin_addr.s_addr;
	theDataUnit.addr.len = sizeof(theAddress);
	theDataUnit.addr.buf = (UInt8 *) &theAddress;
	theDataUnit.opt.len = 0;
	theDataUnit.opt.buf = nil;
	theDataUnit.udata.len = inLength;
	theDataUnit.udata.buf = (UInt8 *) inBuffer;
	
	// Bind the thing
	DoBind(0);

	err = ::OTSndUData(		mEndpointRef,
							&theDataUnit);
	ThrowIfOTError(err);
	return inLength;
}




	SInt16
COTUDPSocket::Receive(
	void *	outBuffer,
	SInt16	inLength,
	UInt16	inFlags)
{
	SInt16	result = 0;
	
	if (mBound) {
		ReceiveFrom(outBuffer, inLength, nil, nil);
	} else {
		throw((PGPSocketsError) kPGPSocketsErrorInval);
	}
	
	return result;
}



	SInt16
COTUDPSocket::ReceiveFrom(
	void *						outBuffer,
	SInt16						inSize,
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	OSStatus		err;
	TUnitData		theDataUnit;
	InetAddress		theAddress;
	OTFlags			flags;

	// Make sure that we have been bound
	if (!mBound) {
		throw((PGPSocketsError) kPGPSocketsErrorInval);
	}
	
	// Fill out the data unit structure
	theDataUnit.addr.maxlen = sizeof(theAddress);
	theDataUnit.addr.buf = (UInt8 *) &theAddress;
	theDataUnit.opt.maxlen = 0;
	theDataUnit.opt.buf = nil;
	theDataUnit.udata.maxlen = inSize;
	theDataUnit.udata.buf = (UInt8 *) outBuffer;
	
	// Bind the thing
	DoBind(0);

	err = ::OTRcvUData(	mEndpointRef,
						&theDataUnit,
						&flags);
	ThrowIfOTError(err);
	if (outAddress != nil) {
		outAddress->sin_family = kPGPAddressFamilyInternet;
		outAddress->sin_port = theAddress.fPort;
		outAddress->sin_addr.s_addr = theAddress.fHost;
	}
	
	if (ioAddressLength != nil) {
		*ioAddressLength = sizeof(PGPSocketAddressInternet);
	}
	
	return theDataUnit.udata.len;
}


	void
COTUDPSocket::Listen(
	SInt16	inMaxBacklog)
{
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
}



	CSocket *
COTUDPSocket::Accept(
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	*ioAddressLength = 0;
	
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
	
	return (CSocket *) kPGPInvalidSocket;
}

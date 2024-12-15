/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMacTCPUDPSocket.cp,v 1.1.10.1 1997/12/07 04:26:44 mhw Exp $
____________________________________________________________________________*/

#include "pgpMem.h"

#include "CMacTCPUDPSocket.h"


const SInt32	kReceiveBufferSize	=	16*1024;


CMacTCPUDPSocket::CMacTCPUDPSocket()
	: mConnected(false), mBound(false), mCreated(false), mReceiveBuffer(nil),
	  mSending(false)
{
	mExcessReceiveHandle = ::NewHandle(0);
	if (mExcessReceiveHandle == nil) {
		throw(kPGPSocketsErrorNoBufs);
	}
}



CMacTCPUDPSocket::~CMacTCPUDPSocket()
{
	::DisposeHandle(mExcessReceiveHandle);
	if (mCreated) {
		UDPiopb	paramBlock;
		
		while (mSending) {
			if (mCallback != nil) {
				mCallback(mCallbackData);
			}
		}
		
		pgpClearMemory(&paramBlock, sizeof(paramBlock));
		paramBlock.ioCRefNum = sRefNum;
		paramBlock.csCode = UDPRelease;
		paramBlock.udpStream = mStream;
		::PBControlSync((ParmBlkPtr) &paramBlock);
	}
	delete [] mReceiveBuffer;
}



	void
CMacTCPUDPSocket::Bind(
	const PGPSocketAddressInternet *	inAddress)
{
	inherited::Bind(inAddress);
	mBound = true;
	SetSetInfo(triState_NoChange, triState_True, triState_NoChange);
}



	void
CMacTCPUDPSocket::Connect(
	const PGPSocketAddressInternet *	inAddress)
{
	mConnected = true;
	::BlockMoveData(inAddress, &mConnectAddress, sizeof(PGPSocketAddressInternet));
}



	SInt16
CMacTCPUDPSocket::Send(
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
CMacTCPUDPSocket::SendTo(
	const void *						inBuffer,
	SInt16								inLength,
	const PGPSocketAddressInternet *	inAddress)
{
	UDPiopb			paramBlock;
	wdsEntry		wds[2] = {	{inLength, (char *) inBuffer},
								{0, 0}};
	
	CreateUDPStream((mBound) ? mBoundPort : 0);
	
	pgpClearMemory(&paramBlock, sizeof(paramBlock));
	paramBlock.udpStream = mStream;
	paramBlock.csParam.send.remoteHost = inAddress->sin_addr.s_addr;
	paramBlock.csParam.send.remotePort = inAddress->sin_port;
	paramBlock.csParam.send.wdsPtr = (Ptr) wds;
	mSending = true;
	try {
		ProcessCommand(UDPWrite, (IPParamBlock *) &paramBlock);
		mSending = false;
	}
	
	catch (...) {
		mSending = false;
		throw;
	}
	
	return inLength;
}




	SInt16
CMacTCPUDPSocket::Receive(
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
CMacTCPUDPSocket::ReceiveFrom(
	void *						outBuffer,
	SInt16						inSize,
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	UDPiopb		paramBlock;
	Byte *		theBuffer = (Byte *) outBuffer;
	SInt16		theSize = inSize;
	SInt16		excessReceiveHandleSize =
						::GetHandleSize(mExcessReceiveHandle);
	SInt16		amountToCopy;
	
	if (! mBound) {
		throw(kPGPSocketsErrorInval);
	}
	CreateUDPStream(mBoundPort);
	
	if (excessReceiveHandleSize > 0) {
		amountToCopy = (theSize > excessReceiveHandleSize) ?
								excessReceiveHandleSize : theSize;
										
		::BlockMoveData(	*mExcessReceiveHandle,
							theBuffer,
							amountToCopy);
		theBuffer += amountToCopy;
		theSize -= amountToCopy;
		excessReceiveHandleSize -= amountToCopy;
		::BlockMoveData(	*mExcessReceiveHandle + amountToCopy,
							*mExcessReceiveHandle,
							excessReceiveHandleSize);
		::SetHandleSize(	mExcessReceiveHandle,
							excessReceiveHandleSize);
	}
	
	if (theSize > 0) {
		pgpClearMemory(&paramBlock, sizeof(paramBlock));
		paramBlock.udpStream = mStream;
		ProcessCommand(UDPRead, (IPParamBlock *) &paramBlock);
		if (outAddress != nil) {
			outAddress->sin_family = kPGPAddressFamilyInternet;
			outAddress->sin_port = paramBlock.csParam.receive.remotePort;
			outAddress->sin_addr.s_addr = 
					paramBlock.csParam.receive.remoteHost;
		}
		if (ioAddressLength != nil) {
			*ioAddressLength = sizeof(PGPSocketAddressInternet);
		}
		amountToCopy = (theSize > paramBlock.csParam.receive.rcvBuffLen) ?
								paramBlock.csParam.receive.rcvBuffLen
								: theSize;
		::BlockMoveData(	paramBlock.csParam.receive.rcvBuff,
							theBuffer,
							amountToCopy);
		::SetHandleSize(	mExcessReceiveHandle,
							excessReceiveHandleSize
								+ (paramBlock.csParam.receive.rcvBuffLen
								- amountToCopy));
		if (MemError() != noErr) {
			throw(kPGPSocketsErrorNoBufs);
		}
		::BlockMoveData(	paramBlock.csParam.receive.rcvBuff + amountToCopy,
							mExcessReceiveHandle + excessReceiveHandleSize,
							paramBlock.csParam.receive.rcvBuffLen
								- amountToCopy);
		theSize -= amountToCopy;
	}
		
	return inSize - theSize;
}


	void
CMacTCPUDPSocket::Listen(
	SInt16	inMaxBacklog)
{
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
}



	CSocket *
CMacTCPUDPSocket::Accept(
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	*ioAddressLength = 0;
	
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
	
	return (CSocket *) kPGPInvalidSocket;
}



	void
CMacTCPUDPSocket::CreateUDPStream(
	SInt16	inPort)
{
	if (! mCreated) {
		UDPiopb	paramBlock;
		
		pgpClearMemory(&paramBlock, sizeof(paramBlock));
		paramBlock.csParam.create.rcvBuff = 
				mReceiveBuffer = new char[kReceiveBufferSize];
		if (mReceiveBuffer == nil) {
			throw(kPGPSocketsErrorNoBufs);
		}
		paramBlock.csParam.create.rcvBuffLen = kReceiveBufferSize;
		paramBlock.csParam.create.localPort = inPort;
		ProcessCommand(UDPCreate, (IPParamBlock *) &paramBlock);
		mStream = paramBlock.udpStream;
		mCreated = true;
	}
}


/* This will probably need to be done with an async notification routine
	Boolean
*/
	Boolean
CMacTCPUDPSocket::IsReadable()
{
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
	return false;
}
